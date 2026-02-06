/**************************************************************************//**
 * @file      fos.c
 * @brief     Kernel libs. Source file.
 * @version   V1.4.03
 * @date      06.02.2026
 ******************************************************************************/
/*
* Copyright 2024 Yury A. Kuzishchin and Vitaly A. Kostarev. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include "Kernel/fos.h"
#include <string.h>

// get thread identifier by its descriptor
static uint8_t FOS_GetThreadId(fos_t *p, fos_thread_t *thr);

// get thread descriptor by its identifier
static fos_thread_t* FOS_GetThreadDesc(fos_t *p, uint8_t id);

// send thread with identifier to sleep
static fos_ret_t FOS_SleepId(fos_t *p, uint8_t id, uint32_t time);

// get binary semaphore identifier by user defined descriptor
static uint8_t FOS_GetUdSemaphoreBinaryId(fos_t *p, user_desc_t user_desc);

// get binary semaphore identifier its descriptor
static uint8_t FOS_GetSemaphoreBinaryId(fos_t *p, fos_semaphore_binary_t *semb);

// get binary semaphore descriptor by its identifier
static fos_semaphore_binary_t* FOS_GetSemaphoreBinaryDesc(fos_t *p, uint8_t id);

// get writer object identifier by its descriptor
static uint8_t FOS_GetFWriterId(fos_t *p, fwriter_t *fw);

// get semaphore identifier by user defined descriptor
static uint8_t FOS_GetUdSemaphoreCntId(fos_t *p, user_desc_t user_desc);

// get semaphore identifier by its descriptor
static uint8_t FOS_GetSemaphoreCntId(fos_t *p, fos_semaphore_cnt_t *semc);

// get binary semaphore descriptor by its identifier
static fos_semaphore_cnt_t* FOS_GetSemaphoreCntDesc(fos_t *p, uint8_t id);

// get queue32 identifier by its descriptor
static uint8_t FOS_GetQueue32Id(fos_t *p, fos_queue32_t *que);

// get queue32 identifier by user defined descriptor
static uint8_t FOS_GetUdQueue32Id(fos_t *p, user_desc_t user_desc);

// get queue32 descriptor by its identifier
static fos_queue32_t* FOS_GetQueue32Desc(fos_t *p, uint8_t id);

// OS kernel initialization
static void Private_FOS_Core_Init(fos_t *p);

// thread scheduler
static int16_t Private_FOS_Sheduler(fos_t *p);

// save user thread stack
static void Private_FOS_SaveUserSP(fos_t *p);

// load user thread stack
static void Private_FOS_LoadUserSP(fos_t *p);

// generate unique user descriptor
static user_desc_t Private_FOS_GenUserDesc(fos_t *p);

// update maximum index of thread descriptor table
static void Private_FOS_UpdThreadMaxInd(fos_t *p);

// update maximum index of binary semaphore descriptor table
static void Private_FOS_UpdSemBinaryMaxInd(fos_t *p);

// update maximum index of counting semaphore descriptor table
static void Private_FOS_UpdSemCntMaxInd(fos_t *p);

// update maximum index of queue32 descriptor table
static void Private_FOS_UpdQueue32MaxInd(fos_t *p);

// update maximum index of writer object descriptor table
static void Private_FOS_UpdFWriterMaxInd(fos_t *p);

// terminating thread procedure
static void Private_FOS_TerminatingThreadProc(fos_t *p);

// unlink thread from all locking objects
static void Private_FOS_UnlinkThread(fos_t *p, uint8_t thr_id);

// get current thread user descriptor
static user_desc_t Private_FOS_GetCurrentThreadUd(fos_t *p);

// get user descriptor of parent thread
static user_desc_t Private_FOS_GetThreadParentUd(fos_t *p);

// add object into turn to delete
static fos_ret_t Private_FOS_AddOjectToDelList(fos_t *p, uint32_t adr, uint8_t heap_type);


// OS initialization
void FOS_Init(fos_t *p)
{
	if(p == NULL)
		return;

	Private_FOS_Core_Init(p);          // OS kernel initialization
}


// OS startup
fos_ret_t FOS_Start(fos_t *p)
{
	if(p == NULL)
		return FOS__FAIL;
	if(p->var.fos_sw == FOS__ENABLE)
		return FOS__FAIL;

	Private_FOS_LoadUserSP(p);             // load stack of the first process
	p->var.fos_sw = FOS__ENABLE;           // enable the OS

	return FOS__OK;
}


// get thread identifier by user defined descriptor
uint8_t FOS_GetUdThreadId(fos_t *p, user_desc_t user_desc)
{
	if((p == NULL) || (user_desc == FOS_WRONG_USER_DESC))
		return FOS_WRONG_THREAD_ID;

	for(uint8_t i = 0; i <= p->var.thread_max_ind; i++)
		if(p->var.thread_desc_list[i])
			if(p->var.thread_desc_list[i]->user_desc == user_desc)
				return i;

	return FOS_WRONG_THREAD_ID;
}


// get thread identifier by its descriptor
static uint8_t FOS_GetThreadId(fos_t *p, fos_thread_t *thr)
{
	if(p == NULL)
		return FOS_WRONG_THREAD_ID;

	for(uint8_t i = 0; i < FOS_MAX_THR_CNT; i++)
		if(p->var.thread_desc_list[i] == thr)
			return i;

	return FOS_WRONG_THREAD_ID;
}


// get thread descriptor by identifier
static fos_thread_t* FOS_GetThreadDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.thread_max_ind)
		return NULL;

	return p->var.thread_desc_list[id];
}


// get semaphore binary user descriptor by thread ID
user_desc_t FOS_GetThreadSembId(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return FOS_WRONG_USER_DESC;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS_WRONG_USER_DESC;

	return thr->cset.semb;
}


// thread registration
fos_ret_t FOS_ThreadReg(fos_t *p, fos_thread_t *thr)
{
	if((p == NULL) || (thr == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// check for duplicated threads
	if(FOS_GetThreadId(p, thr) != FOS_WRONG_THREAD_ID)
		return FOS__FAIL;

	// search for available section
	ind = FOS_GetThreadId(p, NULL);
	if(ind == FOS_WRONG_THREAD_ID)
		return FOS__FAIL;

	// assign a unique user defined identifier to the thread
	if(FOS_Thread_SetUserDesc(thr, Private_FOS_GenUserDesc(p), Private_FOS_GetThreadParentUd(p)) != FOS__OK)
		return FOS__FAIL;

	// set thread registration flag
	if(FOS_Thread_SetRegFlag(thr) != FOS__OK)
		return FOS__FAIL;

	v->thread_desc_list[ind] = thr;        // insert the pointer to an available section

	Private_FOS_UpdThreadMaxInd(p);        // update maximum index

	return FOS__OK;
}


// start thread with identifier
fos_ret_t FOS_RunId(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	// set thread run flag
	return FOS_Thread_SetRunFlag(thr);
}


// terminate thread with identifier
fos_ret_t FOS_TerminateId(fos_t *p, uint8_t id, int32_t terminate_code)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	// set thread termination flags
	if(FOS_Thread_SetTerminateFlag(thr, terminate_code) != FOS__OK)
		return FOS__FAIL;

	if(id == p->var.current_thr)                // if current thread is being terminated
		FOS_System_GoToKernelMode(FOS__DISABLE);    // switch to kernel mode

	return FOS__OK;
}


// terminate current thread
fos_ret_t FOS_Terminate(fos_t *p, int32_t terminate_code)
{
	return FOS_TerminateId(p, p->var.current_thr, terminate_code);
}


// yield to another process
void FOS_Yield()
{
	FOS_System_GoToKernelMode(FOS__DISABLE);       // switch to kernel mode
}


// send thread with identifier to sleep
static fos_ret_t FOS_SleepId(fos_t *p, uint8_t id, uint32_t time)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadSleep(thr, time);     // send the thread to sleep

	if(id == p->var.current_thr)                // if current thread is being sent to sleep
		FOS_System_GoToKernelMode(FOS__DISABLE);    // switch to kernel mode

	return FOS__OK;
}


// send current thread to sleep
fos_ret_t FOS_Sleep(fos_t *p, uint32_t time)
{
	return FOS_SleepId(p, p->var.current_thr, time);
}


// set blocking to thread with identifier
fos_ret_t FOS_LockId(fos_t *p, uint8_t id, uint32_t lock)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadLock(thr, lock);       // block the thread

	if(id == p->var.current_thr)                 // if current thread is being blocked
		FOS_System_GoToKernelMode(FOS__DISABLE);     // switch to kernel mode

	return FOS__OK;
}


// unblock thread with identifier
fos_ret_t FOS_UnlockId(fos_t *p, uint8_t id, uint32_t lock)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadUnlock(thr, lock);

	return FOS__OK;
}


// get semaphore identifier by user defined descriptor
static uint8_t FOS_GetUdSemaphoreBinaryId(fos_t *p, user_desc_t user_desc)
{
	if((p == NULL) || (user_desc == FOS_WRONG_USER_DESC))
		return FOS_WRONG_SEM_BIN_ID;

	for(uint8_t i = 0; i <= p->var.semb_max_ind; i++)
		if(p->var.semb_desc_list[i])
			if(p->var.semb_desc_list[i]->user_desc == user_desc)
				return i;

	return FOS_WRONG_SEM_BIN_ID;
}


// get semaphore identifier by its descriptor
static uint8_t FOS_GetSemaphoreBinaryId(fos_t *p, fos_semaphore_binary_t *semb)
{
	if(p == NULL)
		return FOS_WRONG_SEM_BIN_ID;

	for(uint8_t i = 0; i < FOS_SEM_BIN_CNT; i++)
		if(p->var.semb_desc_list[i] == semb)
			return i;

	return FOS_WRONG_SEM_BIN_ID;
}


// get binary semaphore descriptor by its identifier
static fos_semaphore_binary_t* FOS_GetSemaphoreBinaryDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.semb_max_ind)
		return NULL;

	return p->var.semb_desc_list[id];
}


// register binary semaphore
fos_ret_t FOS_SemBinaryReg(fos_t *p, fos_semaphore_binary_t *semb)
{
	if((p == NULL) || (semb == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// search for duplicated semaphores
	if(FOS_GetSemaphoreBinaryId(p, semb) != FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	// search for available section
	ind = FOS_GetSemaphoreBinaryId(p, NULL);
	if(ind == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	// assign unique user-defined descriptor to the semaphore
	if(FOS_SemaphoreBinary_SetUserDesc(semb, Private_FOS_GenUserDesc(p)) != FOS__OK)
		return FOS__FAIL;

	v->semb_desc_list[ind] = semb;        // insert the pointer into the available section

	Private_FOS_UpdSemBinaryMaxInd(p);    // update the maximum index

	return FOS__OK;
}


// delete binary semaphore
fos_ret_t FOS_SemBinaryDelete(fos_t *p, user_desc_t semb)
{
	if((p == NULL) || (semb == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreBinaryId(p, semb);
	if(id == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	if(Private_FOS_AddOjectToDelList(p, (uint32_t)ptr, FOS_KERNEL_HEAP_ID) != FOS__OK)
		return FOS__FAIL;

	FOS_SemaphoreBinary_UnlockAll(ptr);
	p->var.semb_desc_list[id] = NULL;

	Private_FOS_UpdSemBinaryMaxInd(p);    // update the maximum index

	return FOS__OK;
}


// acquire binary semaphore
fos_ret_t FOS_SemBinaryTake(fos_t *p, user_desc_t semb)
{
	if((p == NULL) || (semb == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreBinaryId(p, semb);
	if(id == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_Take(ptr, p->var.current_thr);
}


// get taking status of binary semaphore
// FOS__OK - normal taking, FOS__FAIL - taking with timeout
fos_ret_t FOS_SemBinaryTakeStat(fos_t *p, user_desc_t semb)
{
	if((p == NULL) || (semb == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreBinaryId(p, semb);
	if(id == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_TakeStat(ptr);
}


// release binary semaphore
fos_ret_t FOS_SemBinaryGive(fos_t *p, user_desc_t semb)
{
	if((p == NULL) || (semb == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreBinaryId(p, semb);
	if(id == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_Give(ptr);
}


// set binary semaphore timeout
fos_ret_t FOS_SemBinarySetTimeout(fos_t *p, user_desc_t semb, uint32_t timeout_ms)
{
	if((p == NULL) || (semb == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreBinaryId(p, semb);
	if(id == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_SetTimeout(ptr, timeout_ms);
}


// get writer object identifier by its descriptor
static uint8_t FOS_GetFWriterId(fos_t *p, fwriter_t *fw)
{
	if(p == NULL)
		return FOS_WRONG_FWRITER_ID;

	for(uint8_t i = 0; i < FOS_FWRITER_CNT; i++)
		if(p->var.fwriter_desc_list[i] == fw)
			return i;

	return FOS_WRONG_FWRITER_ID;
}


// get writer object descriptor by its identifier
fwriter_t* FOS_GetFWriterDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.fwriter_max_id)
		return NULL;

	return p->var.fwriter_desc_list[id];
}


// register writer object
fos_ret_t FOS_FWriterReg(fos_t *p, fwriter_t *fw)
{
	if((p == NULL) || (fw == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// search for duplicated writer objects
	if(FOS_GetFWriterId(p, fw) != FOS_WRONG_FWRITER_ID)
		return FOS__FAIL;

	// search for available section
	ind = FOS_GetFWriterId(p, NULL);
	if(ind == FOS_WRONG_FWRITER_ID)
		return FOS__FAIL;

	v->fwriter_desc_list[ind] = fw;        // insert the pointer into the available section

	Private_FOS_UpdFWriterMaxInd(p);       // update the maximum index

	return FOS__OK;
}


// identify error
void FOS_ErrorSet(fos_t *p, fos_err_t *err)
{
	if((p == NULL) || (err == NULL))
		return;

	memcpy((void*)&p->var.error, err, sizeof(fos_err_t));

	if(err->ext_str_ptr != NULL)
		strncpy((void*)p->var.error.str, err->ext_str_ptr, FOS_MAX_STR_ERR_LEN);
}


// get semaphore identifier by user defined descriptor
static uint8_t FOS_GetUdSemaphoreCntId(fos_t *p, user_desc_t user_desc)
{
	if((p == NULL) || (user_desc == FOS_WRONG_USER_DESC))
		return FOS_WRONG_SEM_CNT_ID;

	for(uint8_t i = 0; i <= p->var.semc_max_ind; i++)
		if(p->var.semc_desc_list[i])
			if(p->var.semc_desc_list[i]->user_desc == user_desc)
				return i;

	return FOS_WRONG_SEM_CNT_ID;
}


// get semaphore identifier by its descriptor
static uint8_t FOS_GetSemaphoreCntId(fos_t *p, fos_semaphore_cnt_t *semc)
{
	if(p == NULL)
		return FOS_WRONG_SEM_CNT_ID;

	for(uint8_t i = 0; i < FOS_SEM_COUNTING_CNT; i++)
		if(p->var.semc_desc_list[i] == semc)
			return i;

	return FOS_WRONG_SEM_CNT_ID;
}


// get semaphore descriptor by its identifier
static fos_semaphore_cnt_t* FOS_GetSemaphoreCntDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.semc_max_ind)
		return NULL;

	return p->var.semc_desc_list[id];
}


// register counting semaphore
fos_ret_t FOS_SemCntReg(fos_t *p, fos_semaphore_cnt_t *semc)
{
	if((p == NULL) || (semc == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// search for duplicated semaphores
	if(FOS_GetSemaphoreCntId(p, semc) != FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	// search for available section
	ind = FOS_GetSemaphoreCntId(p, NULL);
	if(ind == FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	// assign unique user-defined descriptor to the semaphore
	if(FOS_SemaphoreCnt_SetUserDesc(semc, Private_FOS_GenUserDesc(p)) != FOS__OK)
		return FOS__FAIL;

	v->semc_desc_list[ind] = semc;        // insert the pointer into the available section

	Private_FOS_UpdSemCntMaxInd(p);       // update the maximum index

	return FOS__OK;
}


// delete counting semaphore
fos_ret_t FOS_SemCntDelete(fos_t *p, user_desc_t semc)
{
	if((p == NULL) || (semc == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreCntId(p, semc);
	if(id == FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	if(Private_FOS_AddOjectToDelList(p, (uint32_t)ptr, FOS_KERNEL_HEAP_ID) != FOS__OK)
		return FOS__FAIL;

	FOS_SemaphoreCnt_UnlockAll(ptr);
	p->var.semc_desc_list[id] = NULL;

	Private_FOS_UpdSemCntMaxInd(p);    // update the maximum index

	return FOS__OK;
}


// acquire counting semaphore
fos_ret_t FOS_SemCntTake(fos_t *p, user_desc_t semc)
{
	if((p == NULL) || (semc == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreCntId(p, semc);
	if(id == FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreCnt_Take(ptr, p->var.current_thr);
}


// get status of acquire counting semaphore
fos_ret_t FOS_SemCntTakeStat(fos_t *p, user_desc_t semc)
{
	if((p == NULL) || (semc == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreCntId(p, semc);
	if(id == FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreCnt_TakeStat(ptr);
}


// release counting semaphore
fos_ret_t FOS_SemCntGive(fos_t *p, user_desc_t semc)
{
	if((p == NULL) || (semc == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreCntId(p, semc);
	if(id == FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreCnt_Give(ptr);
}


// set counting semaphore timeout
fos_ret_t FOS_SemCntSetTimeout(fos_t *p, user_desc_t semc, uint32_t timeout_ms)
{
	if((p == NULL) || (semc == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreCntId(p, semc);
	if(id == FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreCnt_SetTimeout(ptr, timeout_ms);
}


// get queue32 identifier by its descriptor
static uint8_t FOS_GetQueue32Id(fos_t *p, fos_queue32_t *que)
{
	if(p == NULL)
		return FOS_WRONG_QUE_32_ID;

	for(uint8_t i = 0; i < FOS_SEM_QUEUE_32_CNT; i++)
		if(p->var.queue32_desc_list[i] == que)
			return i;

	return FOS_WRONG_QUE_32_ID;
}


// get queue32 identifier by user defined descriptor
static uint8_t FOS_GetUdQueue32Id(fos_t *p, user_desc_t user_desc)
{
	if((p == NULL) || (user_desc == FOS_WRONG_USER_DESC))
		return FOS_WRONG_QUE_32_ID;

	for(uint8_t i = 0; i <= p->var.queue32_max_ind; i++)
		if(p->var.queue32_desc_list[i])
			if(p->var.queue32_desc_list[i]->user_desc == user_desc)
				return i;

	return FOS_WRONG_QUE_32_ID;
}


// get queue32 descriptor by its identifier
static fos_queue32_t* FOS_GetQueue32Desc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.queue32_max_ind)
		return NULL;

	return p->var.queue32_desc_list[id];
}


// register queue32
fos_ret_t FOS_Queue32Reg(fos_t *p, fos_queue32_t *que)
{
	if((p == NULL) || (que == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// search for duplicated
	if(FOS_GetQueue32Id(p, que) != FOS_WRONG_QUE_32_ID)
		return FOS__FAIL;

	// search for available section
	ind = FOS_GetQueue32Id(p, NULL);
	if(ind == FOS_WRONG_QUE_32_ID)
		return FOS__FAIL;

	// assign unique user-defined descriptor to the queue32
	if(FOS_Queue32_SetUserDesc(que, Private_FOS_GenUserDesc(p)) != FOS__OK)
		return FOS__FAIL;

	v->queue32_desc_list[ind] = que;      // insert the pointer into the available section

	Private_FOS_UpdQueue32MaxInd(p);      // update the maximum index

	return FOS__OK;
}


// join counting semaphore to queue32
fos_ret_t FOS_Queue32JoinToSemCnt(fos_t *p, fos_queue32_t *que, user_desc_t semc)
{
	if((p == NULL) || (que == NULL) || (semc == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreCntId(p, semc);
	if(id == FOS_WRONG_SEM_CNT_ID)
		return FOS__FAIL;

	FOS_Queue32_SetSemaphorePtr(que, FOS_GetSemaphoreCntDesc(p, id));

	return FOS__OK;
}


// delete queue32
fos_ret_t FOS_Queue32Delete(fos_t *p, user_desc_t que)
{
	if((p == NULL) || (que == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdQueue32Id(p, que);
	if(id == FOS_WRONG_QUE_32_ID)
		return FOS__FAIL;

	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	if(Private_FOS_AddOjectToDelList(p, (uint32_t)ptr, FOS_KERNEL_HEAP_ID) != FOS__OK)
		return FOS__FAIL;

	if(Private_FOS_AddOjectToDelList(p, (uint32_t)ptr->msg.buf_ptr, FOS_THREADS_HEAP_ID) != FOS__OK)
		return FOS__FAIL;

	if(ptr->semc_ptr)
		FOS_SemCntDelete(p, ptr->semc_ptr->user_desc);

	p->var.queue32_desc_list[id] = NULL;

	Private_FOS_UpdQueue32MaxInd(p);   // update the maximum index

	return FOS__OK;
}


// ask data
fos_ret_t FOS_Queue32AskData(fos_t *p, user_desc_t que, fos_queue_sw_t blocking_mode_sw)
{
	if((p == NULL) || (que == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdQueue32Id(p, que);
	if(id == FOS_WRONG_QUE_32_ID)
		return FOS__FAIL;

	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	uint8_t block_thr_id = FOS_SPECIAL_ID;

	if(blocking_mode_sw == FOS_QUEUE_SW__BLOCK)
		if(FOS_System_GetWorkMode() == FOS__USER_WORK_MODE)
			block_thr_id = p->var.current_thr;

	return FOS_Queue32_AskData(ptr, block_thr_id);
}


// read data
// one must ask data before read every times
fos_ret_t FOS_Queue32ReadData(fos_t *p, user_desc_t que, uint32_t* data_ptr)
{
	if((p == NULL) || (que == FOS_WRONG_USER_DESC) || (data_ptr == NULL))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdQueue32Id(p, que);
	if(id == FOS_WRONG_QUE_32_ID)
		return FOS__FAIL;

	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_Queue32_ReadData(ptr, data_ptr);
}


// write data
fos_ret_t FOS_Queue32WriteData(fos_t *p, user_desc_t que, uint32_t data)
{
	if((p == NULL) || (que == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdQueue32Id(p, que);
	if(id == FOS_WRONG_QUE_32_ID)
		return FOS__FAIL;

	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_Queue32_WriteData(ptr, data);
}


// get the system stack debug info
fos_thread_dbg_t* FOS_GetSysStackDbgInfo(fos_t *p)
{
	if(p == NULL)
		return NULL;

	return &p->sys_stack_dbg;
}


// get the scheduler debug info
fos_scheduler_dbg_t* FOS_GetSchedulerDbgInfo(fos_t *p)
{
	if(p == NULL)
		return NULL;

	return &p->sheduler.dbg;
}


// main loop handler
void FOS_MainLoopProc(fos_t *p)
{
	if(p == NULL)
		return;

	if(p->var.fos_sw == FOS__DISABLE)
		return;

	Private_FOS_TerminatingThreadProc(p);       // terminating thread procedure

	FOS_ThreadProcDbg(&p->sys_stack_dbg, 0);    // kernel stack debug

	/*
	 * Handle states of all the threads
	 */
	FOS_AllThreadProcState(p->var.thread_desc_list, p->var.thread_max_ind);

	/*
	 * Handle states of all the sem
	 */
	FOS_AllSemaphoreBinary_ProcTimeout(p->var.semb_desc_list, p->var.semb_max_ind);
	FOS_AllSemaphoreCnt_ProcTimeout(p->var.semc_desc_list, p->var.semc_max_ind);

	if(Private_FOS_Sheduler(p) < 0)          // thread scheduler
		return;
	FOS_System_GoToUserMode();                 // switch to user mode
}


// OS kernel intialization
static void Private_FOS_Core_Init(fos_t *p)
{
	memset(&p->var, 0, sizeof(fos_var_t));
	memset(&p->sheduler, 0, sizeof(fos_scheduler_t));
	memset(&p->sys_stack_dbg, 0 , sizeof(fos_thread_dbg_t));
}


// thread scheduler
static int16_t Private_FOS_Sheduler(fos_t *p)
{
	fos_var_t* v = &p->var;

	// acquisition of current thread statistics
	uint8_t  current_thr = v->current_thr;
	uint32_t thr_dt_us   = fos_mgv.thr_dt_us;
	FOS_ScheduleDbg(&p->sheduler, v->thread_max_ind, current_thr, thr_dt_us);

	/*
	 * Choose next thread
	 */
	int16_t next_thr = FOS_Schedule(&p->sheduler, v->thread_desc_list, v->thread_max_ind);
	if(next_thr < 0)
		return next_thr;



	/*
	 * Switch threads
	 */
	fos_thread_ptr thr = FOS_GetThreadDesc(p, v->current_thr);    // get current thread descrpitor
	if(thr)                                                       // check for its existence
	{
		Private_FOS_SaveUserSP(p);                                // save stack of user defined thread

		if(thr->var.state == FOS__THREAD_RUNNING)                 // if current thread is RUNNING (it can be BLOCKED)
			thr->var.state = FOS__THREAD_READY;                   // assign this thread state READY
	}


	thr = FOS_GetThreadDesc(p, (uint8_t)next_thr);  // get next thread descriptor
	if(thr == NULL)                                 // check for its existence (redundant!!!)
		return next_thr;

	v->current_thr = (uint8_t)next_thr;             // assign the index of the next thread as the index of the running thread
	thr->var.state = FOS__THREAD_RUNNING;           // assign this thread state RUNNING

	Private_FOS_LoadUserSP(p);                      // load the stack of user defined thread

	return next_thr;
}


// save the stack of user defined thread
static void Private_FOS_SaveUserSP(fos_t *p)
{
	fos_thread_t* tp = p->var.thread_desc_list[p->var.current_thr];
	tp->var.sp = fos_mgv.user_sp;
}


// load the stack of user defined thread
static void Private_FOS_LoadUserSP(fos_t *p)
{
	fos_thread_t* tp = p->var.thread_desc_list[p->var.current_thr];
	fos_mgv.user_sp = tp->var.sp;
}


// generate unique user descriptor
static user_desc_t Private_FOS_GenUserDesc(fos_t *p)
{
	p->var.last_user_desc++;
	while((p->var.last_user_desc == FOS_KERNEL_USER_DESC) || (p->var.last_user_desc == FOS_WRONG_USER_DESC))
		p->var.last_user_desc++;
	return p->var.last_user_desc;
}


// update maximum index of thread descriptor table
static void Private_FOS_UpdThreadMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// calculate maximum index
	for(uint8_t i = 0; i < FOS_MAX_THR_CNT; i++)
		if(p->var.thread_desc_list[i] != NULL)
			ind = i;

	p->var.thread_max_ind = ind;               // record the maximum index into a variable
}


// update maximum index of binary semaphore descriptor table
static void Private_FOS_UpdSemBinaryMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// calculate maximum index
	for(uint8_t i = 0; i < FOS_SEM_BIN_CNT; i++)
		if(p->var.semb_desc_list[i] != NULL)
			ind = i;

	p->var.semb_max_ind = ind;                // record the maximum index into a variable
}


// update maximum index of counting semaphore descriptor table
static void Private_FOS_UpdSemCntMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// calculate maximum index
	for(uint8_t i = 0; i < FOS_SEM_COUNTING_CNT; i++)
		if(p->var.semc_desc_list[i] != NULL)
			ind = i;

	p->var.semc_max_ind = ind;                // record the maximum index into a variable
}


// update maximum index of queue32 descriptor table
static void Private_FOS_UpdQueue32MaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// calculate maximum index
	for(uint8_t i = 0; i < FOS_SEM_QUEUE_32_CNT; i++)
		if(p->var.queue32_desc_list[i] != NULL)
			ind = i;

	p->var.queue32_max_ind = ind;             // record the maximum index into a variable
}


// update maximum index of writer object descriptor table
static void Private_FOS_UpdFWriterMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// calculate maximum index
	for(uint8_t i = 0; i < FOS_FWRITER_CNT; i++)
		if(p->var.fwriter_desc_list[i] != NULL)
			ind = i;

	p->var.fwriter_max_id = ind;               // record the maximum index into a variable
}


// terminating thread procedure
static void Private_FOS_TerminatingThreadProc(fos_t *p)
{
	fos_thread_t     *thr;
	fos_thread_var_t *v;
	uint8_t max_upd_needed = 0;

	for(uint8_t i = 0; i <= p->var.thread_max_ind; i++)
	{
		// get thread descriptor by identifier
		thr = FOS_GetThreadDesc(p, i);
		if(thr)
		{
			v = &thr->var;

			if(v->mode == FOS__THREAD_TERMINATING)
			{
				FOS_SemBinaryDelete(p, thr->cset.semb);
				Private_FOS_UnlinkThread(p, i);
				v->mode = FOS__THREAD_TERMINATED;
			}

			if((v->mode == FOS__THREAD_TERMINATED) && (v->static_flag == FOS__DISABLE))
			{
				if(thr->cset.base_sp)
				{
					if(Private_FOS_AddOjectToDelList(p, thr->cset.base_sp, FOS_THREADS_HEAP_ID) == FOS__OK)
						thr->cset.base_sp = 0;
					else
						continue;
				}

				if(Private_FOS_AddOjectToDelList(p, (uint32_t)thr, FOS_KERNEL_HEAP_ID) == FOS__OK)
				{
					p->var.thread_desc_list[i] = NULL;
					max_upd_needed = 1;
				}
			}
		}
	}

	if(max_upd_needed)
		Private_FOS_UpdThreadMaxInd(p);        // update maximum index
}



// unlink thread from all locking objects
static void Private_FOS_UnlinkThread(fos_t *p, uint8_t thr_id)
{
	fos_semaphore_binary_t *semb;

	/*
	 * Unlink thread from all binary semaphores
	 */
	for(uint8_t i = 0; i <= p->var.semb_max_ind; i++)
	{
		semb = FOS_GetSemaphoreBinaryDesc(p, i);
		if(semb)
		{
			FOS_SemaphoreBinary_UnlinkThread(semb, thr_id);
		}
	}
}


// get current thread user descriptor
static user_desc_t Private_FOS_GetCurrentThreadUd(fos_t *p)
{
	fos_var_t *v = &p->var;
	fos_thread_ptr thr = v->thread_desc_list[v->current_thr];
	if(thr == NULL)
		return FOS_WRONG_USER_DESC;

	return thr->user_desc;
}


// get user descriptor of parent thread
static user_desc_t Private_FOS_GetThreadParentUd(fos_t *p)
{
	if(FOS_System_GetWorkMode() == FOS__KERNEL_WORK_MODE)
		return FOS_KERNEL_USER_DESC;
	return Private_FOS_GetCurrentThreadUd(p);
}


// add object into turn to delete
static fos_ret_t Private_FOS_AddOjectToDelList(fos_t *p, uint32_t adr, uint8_t heap_type)
{
	fos_var_t *v = &p->var;

	if(v->obj_to_del_cnt == FOS_MAX_OBJ_TO_DEL)
		return FOS__FAIL;

	for(uint8_t i = 0; i < FOS_MAX_OBJ_TO_DEL; i++)
	{
		if(v->obj_to_del[i].adr == 0)
		{
			v->obj_to_del[i].adr = adr;
			v->obj_to_del[i].heap_type = heap_type;
			v->obj_to_del_cnt++;
			return FOS__OK;
		}
	}

	return FOS__FAIL;
}


/*
 * Currently not used
 */

// get id of current thread
/*uint8_t FOS_GetCurrentThreadId(fos_t *p)
{
	if(p == NULL)
		return FOS_WRONG_THREAD_ID;

	return p->var.current_thr;
}*/


// wake up thread
/*fos_ret_t FOS_WeakUpId(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return FOS__FAIL;

	// получаем дескриптор потока по id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadWeakUp(thr);

	return FOS__OK;
}*/


// block current thread
/*fos_ret_t FOS_Lock(fos_t *p, uint32_t lock)
{
	return FOS_LockId(p, p->var.current_thr, lock);
}*/













