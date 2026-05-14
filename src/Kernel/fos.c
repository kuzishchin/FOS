/**************************************************************************//**
 * @file      fos.c
 * @brief     Kernel libs. Source file.
 * @version   V1.6.05
 * @date      08.05.2026
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

// generate unique user descriptor
static user_desc_t FOS_GenUserDesc(fos_t *p, fos_obj_types_t obj_type, uint8_t ind);

// get object identifier by user defined descriptor
static uint8_t FOS_GetObjectIdByUd(user_desc_t user_desc);

// get object type
static fos_obj_types_t FOS_GetObjectTypeByUd(user_desc_t user_desc);

//***********************************************************

// check thread ud
static fos_ret_t FOS_CheckThreadUd(fos_thread_t *thr, user_desc_t user_desc);

// check binary semaphore ud
static fos_ret_t FOS_CheckSemaphoreBinaryUd(fos_semaphore_binary_t *semb, user_desc_t user_desc);

// check counting semaphore ud
static fos_ret_t FOS_CheckSemaphoreCntUd(fos_semaphore_cnt_t *semc, user_desc_t user_desc);

// check queue32 ud
static fos_ret_t FOS_CheckQueue32Ud(fos_queue32_t *que, user_desc_t user_desc);

// check mutex ud
static fos_ret_t FOS_CheckMutexUd(fos_mutex_t *mut, user_desc_t user_desc);

//************************************************

// get thread descriptor by identifier
static fos_thread_t* FOS_GetThreadDesc(fos_t *p, uint8_t id);

// get binary semaphore descriptor by its identifier
static fos_semaphore_binary_t* FOS_GetSemaphoreBinaryDesc(fos_t *p, uint8_t id);

// get semaphore descriptor by its identifier
static fos_semaphore_cnt_t* FOS_GetSemaphoreCntDesc(fos_t *p, uint8_t id);

// get queue32 descriptor by its identifier
static fos_queue32_t* FOS_GetQueue32Desc(fos_t *p, uint8_t id);

// get mutex descriptor by its identifier
static fos_mutex_t* FOS_GetMutexDesc(fos_t *p, uint8_t id);

//************************************************

// get thread identifier by its descriptor
static uint8_t FOS_GetThreadId(fos_t *p, fos_thread_t *thr);

// get semaphore identifier by its descriptor
static uint8_t FOS_GetSemaphoreBinaryId(fos_t *p, fos_semaphore_binary_t *semb);

// get semaphore identifier by its descriptor
static uint8_t FOS_GetSemaphoreCntId(fos_t *p, fos_semaphore_cnt_t *semc);

// get queue32 identifier by its descriptor
static uint8_t FOS_GetQueue32Id(fos_t *p, fos_queue32_t *que);

// get mutex identifier by its descriptor
static uint8_t FOS_GetMutexId(fos_t *p, fos_mutex_t *mut);

// get writer object identifier by its descriptor
static uint8_t FOS_GetFWriterId(fos_t *p, fwriter_t *fw);

//*************************************************

// update maximum index of thread descriptor table
static void Private_FOS_UpdThreadMaxInd(fos_t *p);

// update maximum index of binary semaphore descriptor table
static void Private_FOS_UpdSemBinaryMaxInd(fos_t *p);

// update maximum index of counting semaphore descriptor table
static void Private_FOS_UpdSemCntMaxInd(fos_t *p);

// update maximum index of mutex descriptor table
static void Private_FOS_UpdMutexMaxInd(fos_t *p);

// update maximum index of queue32 descriptor table
static void Private_FOS_UpdQueue32MaxInd(fos_t *p);

// update maximum index of writer object descriptor table
static void Private_FOS_UpdFWriterMaxInd(fos_t *p);

//**************************************************

// get current thread user descriptor
static user_desc_t FOS_GetCurrentThreadUd(fos_t *p);

// send thread with identifier to sleep
static fos_ret_t FOS_SleepId(fos_t *p, uint8_t id, uint32_t time, fos_sw_t is_waiting);

//**************************************************

// unlink thread from the binary semaphore
static fos_ret_t Private_FOS_UnlinkThreadFromSemb(fos_t *p, user_desc_t semb, uint8_t thr_id);

// unlink thread from the counting semaphore
static fos_ret_t Private_FOS_UnlinkThreadFromSemc(fos_t *p, user_desc_t semc, uint8_t thr_id);

//**************************************************

// OS kernel initialization
static void Private_FOS_Core_Init(fos_t *p);

// thread scheduler
static int16_t Private_FOS_Sheduler(fos_t *p);

// save user thread stack
static void Private_FOS_SaveUserSP(fos_t *p);

// load user thread stack
static void Private_FOS_LoadUserSP(fos_t *p);

// set unpriv mode
static void Private_FOS_SetUnprivMode(fos_t *p);

// terminating thread procedure
static void Private_FOS_TerminatingThreadProc(fos_t *p);

// unlink thread from all locking objects
static void Private_FOS_UnlinkThread(fos_t *p, uint8_t thr_id);

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

	if(Private_FOS_Sheduler(p) < 0)          // thread scheduler
		return;
	FOS_System_GoToUserMode();                 // switch to user mode
}

//*********************************

// generate unique user descriptor
static user_desc_t FOS_GenUserDesc(fos_t *p, fos_obj_types_t obj_type, uint8_t ind)
{
	p->var.last_user_desc++;
	p->var.last_user_desc &= FOS_UD_NUM_MASK;
	while((p->var.last_user_desc == FOS_KERNEL_USER_DESC) || (p->var.last_user_desc == FOS_WRONG_USER_DESC))
		p->var.last_user_desc++;
	return (p->var.last_user_desc & FOS_UD_NUM_MASK) | ((obj_type << FOS_UD_TYPE_SHIFT) & FOS_UD_TYPE_MASK) | ((ind << FOS_UD_IND_SHIFT) & FOS_UD_IND_MASK);
}


// get object identifier by user defined descriptor
static uint8_t FOS_GetObjectIdByUd(user_desc_t user_desc)
{
    if((user_desc == FOS_WRONG_USER_DESC) || (user_desc == FOS_KERNEL_USER_DESC))
        return FOS_WRONG_THREAD_ID;

    return (((uint32_t)user_desc) & FOS_UD_IND_MASK) >> FOS_UD_IND_SHIFT;
}

// get object type
static fos_obj_types_t FOS_GetObjectTypeByUd(user_desc_t user_desc)
{
    if((user_desc == FOS_WRONG_USER_DESC) || (user_desc == FOS_KERNEL_USER_DESC))
        return FOS_OBJ_TYPE__UNKNOWN;

    return (fos_obj_types_t)((((uint32_t)user_desc) & FOS_UD_TYPE_MASK) >> FOS_UD_TYPE_SHIFT);
}

//**********************************

// check thread ud
static fos_ret_t FOS_CheckThreadUd(fos_thread_t *thr, user_desc_t user_desc)
{
    if(thr == NULL)
        return FOS__FAIL;
    if(thr->user_desc != user_desc)
        return FOS__FAIL;
    return FOS__OK;
}


// check binary semaphore ud
static fos_ret_t FOS_CheckSemaphoreBinaryUd(fos_semaphore_binary_t *semb, user_desc_t user_desc)
{
    if(semb == NULL)
        return FOS__FAIL;
    if(semb->user_desc != user_desc)
        return FOS__FAIL;
    return FOS__OK;
}


// check counting semaphore ud
static fos_ret_t FOS_CheckSemaphoreCntUd(fos_semaphore_cnt_t *semc, user_desc_t user_desc)
{
    if(semc == NULL)
        return FOS__FAIL;
    if(semc->user_desc != user_desc)
        return FOS__FAIL;
    return FOS__OK;
}


// check queue32 ud
static fos_ret_t FOS_CheckQueue32Ud(fos_queue32_t *que, user_desc_t user_desc)
{
    if(que == NULL)
        return FOS__FAIL;
    if(que->user_desc != user_desc)
        return FOS__FAIL;
    return FOS__OK;
}


// check mutex ud
static fos_ret_t FOS_CheckMutexUd(fos_mutex_t *mut, user_desc_t user_desc)
{
    if(mut == NULL)
        return FOS__FAIL;
    if(mut->user_desc != user_desc)
        return FOS__FAIL;
    return FOS__OK;
}

//**************************************************

// get thread id by used descriptor
uint8_t FOS_GetThreadIdByUd(fos_t *p, user_desc_t user_desc)
{
    uint8_t id = FOS_GetObjectIdByUd(user_desc);
    fos_thread_t* thr_ptr = FOS_GetThreadDesc(p, id);
    if(FOS_CheckThreadUd(thr_ptr, user_desc) != FOS__OK)
        return FOS_WRONG_THREAD_ID;
    return id;
}

//**********************************************

// get thread descriptor by identifier
static fos_thread_t* FOS_GetThreadDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.thread_max_ind)
		return NULL;

	return p->var.thread_desc_list[id];
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


// get semaphore descriptor by its identifier
static fos_semaphore_cnt_t* FOS_GetSemaphoreCntDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.semc_max_ind)
		return NULL;

	return p->var.semc_desc_list[id];
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


// get mutex descriptor by its identifier
static fos_mutex_t* FOS_GetMutexDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.mutex_max_ind)
		return NULL;

	return p->var.mutex_desc_list[id];
}

//***********************************************

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


// get mutex identifier by its descriptor
static uint8_t FOS_GetMutexId(fos_t *p, fos_mutex_t *mut)
{
	if(p == NULL)
		return FOS_WRONG_MUTEX_ID;

	for(uint8_t i = 0; i < FOS_MUTEX_CNT; i++)
		if(p->var.mutex_desc_list[i] == mut)
			return i;

	return FOS_WRONG_MUTEX_ID;
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

//*************************************

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


// update maximum index of mutex descriptor table
static void Private_FOS_UpdMutexMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// calculate maximum index
	for(uint8_t i = 0; i < FOS_MUTEX_CNT; i++)
		if(p->var.mutex_desc_list[i] != NULL)
			ind = i;

	p->var.mutex_max_ind = ind;                // record the maximum index into a variable
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

//*************************************

// thread registration
fos_ret_t FOS_ThreadReg(fos_t *p, fos_thread_t *thr, uint8_t *id_ptr)
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
	if(FOS_Thread_SetUserDesc(thr, FOS_GenUserDesc(p, FOS_OBJ_TYPE__THREAD, ind), FOS_GetThreadParentUd(p)) != FOS__OK)
		return FOS__FAIL;

	// set thread registration flag
	if(FOS_Thread_SetRegFlag(thr) != FOS__OK)
		return FOS__FAIL;

	v->thread_desc_list[ind] = thr;        // insert the pointer to an available section

	Private_FOS_UpdThreadMaxInd(p);        // update maximum index

	if(id_ptr)                             // return id
		*id_ptr = ind;

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


// start thread with identifier and with arg
fos_ret_t FOS_RunIdWithArg(fos_t *p, uint8_t id, uint8_t* arg_ptr, uint32_t arg_len)
{
	if((p == NULL) || (arg_ptr == NULL))
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	fos_ret_t ret = FOS_Thread_AddArg(thr, arg_ptr, arg_len);
	if(ret != FOS__OK)
		return ret;

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


// get thread arg pointer
uint8_t* FOS_GetThreadArgPtr(fos_t *p)
{
	if(p == NULL)
		return NULL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, p->var.current_thr);
	if(thr == NULL)
		return NULL;

	return FOS_Thread_GetArgPtr(thr);
}


// get thread arg len
uint32_t FOS_GetThreadArgLen(fos_t *p)
{
	if(p == NULL)
		return 0;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, p->var.current_thr);
	if(thr == NULL)
		return 0;

	return FOS_Thread_GetArgLen(thr);
}


// set note to thread by id
fos_ret_t FOS_SetNoteId(fos_t *p, uint8_t id, fos_note_type_t type, uint32_t note)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	uint32_t x;
	ENTER_CRITICAL(x);
	fos_ret_t ret = FOS_Thread_SetNote(thr, type, note);
	LEAVE_CRITICAL(x);

	return ret;
}


// get thread note pointer
fos_thr_note_t* FOS_GetThreadNotePtr(fos_t *p)
{
	if(p == NULL)
		return NULL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, p->var.current_thr);
	if(thr == NULL)
		return NULL;

	return FOS_Thread_GetNotePtr(thr);
}


// get ep_wa
uint32_t FOS_GetThreadEpA(fos_t *p)
{
	if(p == NULL)
		return 0;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, p->var.current_thr);
	if(thr == NULL)
		return 0;

	// get ep_wa
	return FOS_Thread_GetEpA(thr);
}


// get returned values from the thread
fos_ret_val_t* FOS_GetThreadRValPtr(fos_t *p)
{
	if(p == NULL)
		return NULL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, p->var.current_thr);
	if(thr == NULL)
		return NULL;

	return FOS_Thread_GetRValPtr(thr);
}


// get id of current thread
uint8_t FOS_GetCurrentThreadId(fos_t *p)
{
	if(p == NULL)
		return FOS_WRONG_THREAD_ID;

	return p->var.current_thr;
}


// get current thread user descriptor
static user_desc_t FOS_GetCurrentThreadUd(fos_t *p)
{
	fos_var_t *v = &p->var;
	fos_thread_ptr thr = v->thread_desc_list[v->current_thr];
	if(thr == NULL)
		return FOS_WRONG_USER_DESC;

	return thr->user_desc;
}


// get user descriptor of parent thread
user_desc_t FOS_GetThreadParentUd(fos_t *p)
{
	if(FOS_System_GetWorkMode() == FOS__KERNEL_WORK_MODE)
		return FOS_KERNEL_USER_DESC;
	return FOS_GetCurrentThreadUd(p);
}


// is thread run
fos_ret_t FOS_IsThreadAlive(fos_t *p, user_desc_t desc)
{
	return FOS_IsThreadRun(FOS_GetThreadDesc(p, FOS_GetThreadIdByUd(p, desc)));
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


// get thread user descriptor by thread ID
user_desc_t FOS_GetUdThreadById(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return FOS_WRONG_USER_DESC;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS_WRONG_USER_DESC;

	return thr->user_desc;
}


// send thread with identifier to sleep
static fos_ret_t FOS_SleepId(fos_t *p, uint8_t id, uint32_t time, fos_sw_t is_waiting)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadSleep(thr, time, is_waiting);     // send the thread to sleep

	if(id == p->var.current_thr)                // if current thread is being sent to sleep
		FOS_System_GoToKernelMode(FOS__DISABLE);    // switch to kernel mode

	return FOS__OK;
}


// send current thread to sleep
fos_ret_t FOS_Sleep(fos_t *p, uint32_t time, fos_sw_t is_waiting)
{
	return FOS_SleepId(p, p->var.current_thr, time, is_waiting);
}


// set blocking to thread with identifier
fos_ret_t FOS_LockId(fos_t *p, uint8_t id, uint32_t lock, user_desc_t lock_obj_ud, uint32_t timeout_ms)
{
	if(p == NULL)
		return FOS__FAIL;

	// get thread descriptor by identifier
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadLock(thr, lock, lock_obj_ud, timeout_ms);  // block the thread

	if(id == p->var.current_thr)                         // if current thread is being blocked
		FOS_System_GoToKernelMode(FOS__DISABLE);         // switch to kernel mode

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


// yield to another process
fos_ret_t FOS_Yield()
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;

	FOS_System_GoToKernelMode(FOS__DISABLE);       // switch to kernel mode

	return FOS__OK;
}

//************************************************

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
	if(FOS_SemaphoreBinary_SetUserDesc(semb, FOS_GenUserDesc(p, FOS_OBJ_TYPE__BINARY_SEM, ind)) != FOS__OK)
		return FOS__FAIL;

	v->semb_desc_list[ind] = semb;        // insert the pointer into the available section

	Private_FOS_UpdSemBinaryMaxInd(p);    // update the maximum index

	return FOS__OK;
}


// delete binary semaphore
fos_ret_t FOS_SemBinaryDelete(fos_t *p, user_desc_t semb)
{
	uint8_t id = FOS_GetObjectIdByUd(semb);
	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(FOS_CheckSemaphoreBinaryUd(ptr, semb) != FOS__OK)
		return FOS__FAIL;

	if(Private_FOS_AddOjectToDelList(p, (uint32_t)ptr, FOS_KERNEL_HEAP_ID) != FOS__OK)
		return FOS__FAIL;

	FOS_SemaphoreBinary_UnlockAll(ptr);
	p->var.semb_desc_list[id] = NULL;

	Private_FOS_UpdSemBinaryMaxInd(p);    // update the maximum index

	return FOS__OK;
}


// acquire binary semaphore
fos_ret_t FOS_SemBinaryTake(fos_t *p, user_desc_t semb, uint32_t timeout_ms)
{
	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, FOS_GetObjectIdByUd(semb));
	if(FOS_CheckSemaphoreBinaryUd(ptr, semb) != FOS__OK)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_Take(ptr, p->var.current_thr, timeout_ms);
}


// release binary semaphore
fos_ret_t FOS_SemBinaryGive(fos_t *p, user_desc_t semb)
{
	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, FOS_GetObjectIdByUd(semb));
	if(FOS_CheckSemaphoreBinaryUd(ptr, semb) != FOS__OK)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_Give(ptr);
}

//***************************************

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
	if(FOS_SemaphoreCnt_SetUserDesc(semc, FOS_GenUserDesc(p, FOS_OBJ_TYPE__CNT_SEM, ind)) != FOS__OK)
		return FOS__FAIL;

	v->semc_desc_list[ind] = semc;        // insert the pointer into the available section

	Private_FOS_UpdSemCntMaxInd(p);       // update the maximum index

	return FOS__OK;
}


// delete counting semaphore
fos_ret_t FOS_SemCntDelete(fos_t *p, user_desc_t semc)
{
	uint8_t id = FOS_GetObjectIdByUd(semc);
	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, id);
	if(FOS_CheckSemaphoreCntUd(ptr, semc) != FOS__OK)
		return FOS__FAIL;

	if(Private_FOS_AddOjectToDelList(p, (uint32_t)ptr, FOS_KERNEL_HEAP_ID) != FOS__OK)
		return FOS__FAIL;

	FOS_SemaphoreCnt_UnlockAll(ptr);
	p->var.semc_desc_list[id] = NULL;

	Private_FOS_UpdSemCntMaxInd(p);    // update the maximum index

	return FOS__OK;
}


// acquire counting semaphore
fos_ret_t FOS_SemCntTake(fos_t *p, user_desc_t semc, uint32_t timeout_ms)
{
	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, FOS_GetObjectIdByUd(semc));
	if(FOS_CheckSemaphoreCntUd(ptr, semc) != FOS__OK)
		return FOS__FAIL;

	return FOS_SemaphoreCnt_Take(ptr, p->var.current_thr, timeout_ms);
}


// release counting semaphore
fos_ret_t FOS_SemCntGive(fos_t *p, user_desc_t semc)
{
	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, FOS_GetObjectIdByUd(semc));
	if(FOS_CheckSemaphoreCntUd(ptr, semc) != FOS__OK)
		return FOS__FAIL;

	return FOS_SemaphoreCnt_Give(ptr);
}

//******************************************

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
	if(FOS_Queue32_SetUserDesc(que, FOS_GenUserDesc(p, FOS_OBJ_TYPE__QUEUE_32, ind)) != FOS__OK)
		return FOS__FAIL;

	v->queue32_desc_list[ind] = que;      // insert the pointer into the available section

	Private_FOS_UpdQueue32MaxInd(p);      // update the maximum index

	return FOS__OK;
}


// join counting semaphore to queue32
fos_ret_t FOS_Queue32JoinToSemCnt(fos_t *p, fos_queue32_t *que, user_desc_t semc)
{
	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, FOS_GetObjectIdByUd(semc));
	if(FOS_CheckSemaphoreCntUd(ptr, semc) != FOS__OK)
		return FOS__FAIL;

	return FOS_Queue32_SetSemaphorePtr(que, ptr);
}


// delete queue32
fos_ret_t FOS_Queue32Delete(fos_t *p, user_desc_t que)
{
	uint8_t id = FOS_GetObjectIdByUd(que);
	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, id);
	if(FOS_CheckQueue32Ud(ptr, que) != FOS__OK)
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
fos_ret_t FOS_Queue32AskData(fos_t *p, user_desc_t que, uint32_t timeout_ms)
{
	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, FOS_GetObjectIdByUd(que));
	if(FOS_CheckQueue32Ud(ptr, que) != FOS__OK)
		return FOS__FAIL;

	uint8_t block_thr_id = FOS_SPECIAL_ID;

	if(timeout_ms)
		if(FOS_System_GetWorkMode() == FOS__USER_WORK_MODE)
			block_thr_id = p->var.current_thr;

	return FOS_Queue32_AskData(ptr, block_thr_id, timeout_ms);
}


// read data
// one must ask data before read every times
fos_ret_t FOS_Queue32ReadData(fos_t *p, user_desc_t que, uint32_t* data_ptr)
{
	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, FOS_GetObjectIdByUd(que));
	if(FOS_CheckQueue32Ud(ptr, que) != FOS__OK)
		return FOS__FAIL;

	return FOS_Queue32_ReadData(ptr, data_ptr);
}


// write data
fos_ret_t FOS_Queue32WriteData(fos_t *p, user_desc_t que, uint32_t data)
{
	fos_queue32_t *ptr = FOS_GetQueue32Desc(p, FOS_GetObjectIdByUd(que));
	if(FOS_CheckQueue32Ud(ptr, que) != FOS__OK)
		return FOS__FAIL;

	return FOS_Queue32_WriteData(ptr, data);
}

//**********************************************

// register mutex
fos_ret_t FOS_MutexReg(fos_t *p, fos_mutex_t *mut)
{
	if((p == NULL) || (mut == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// search for duplicated semaphores
	if(FOS_GetMutexId(p, mut) != FOS_WRONG_MUTEX_ID)
		return FOS__FAIL;

	// search for available section
	ind = FOS_GetMutexId(p, NULL);
	if(ind == FOS_WRONG_MUTEX_ID)
		return FOS__FAIL;

	// assign unique user-defined descriptor to the mutex
	if(FOS_Mutex_SetUserDesc(mut, FOS_GenUserDesc(p, FOS_OBJ_TYPE__MUTEX, ind)) != FOS__OK)
		return FOS__FAIL;

	v->mutex_desc_list[ind] = mut;        // insert the pointer into the available section

	Private_FOS_UpdMutexMaxInd(p);        // update the maximum index

	return FOS__OK;
}


// join binary semaphore to mutex
fos_ret_t FOS_MutexJoinToSemBinary(fos_t *p, fos_mutex_t *mut, user_desc_t semb)
{
	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, FOS_GetObjectIdByUd(semb));
	if(FOS_CheckSemaphoreBinaryUd(ptr, semb) != FOS__OK)
		return FOS__FAIL;

	return FOS_Mutex_SetSemaphorePtr(mut, ptr);
}


// delete mutex
fos_ret_t FOS_MutexDelete(fos_t *p, user_desc_t mutex)
{
	uint8_t id = FOS_GetObjectIdByUd(mutex);
	fos_mutex_t *ptr = FOS_GetMutexDesc(p, id);
	if(FOS_CheckMutexUd(ptr, mutex) != FOS__OK)
		return FOS__FAIL;

	if(Private_FOS_AddOjectToDelList(p, (uint32_t)ptr, FOS_KERNEL_HEAP_ID) != FOS__OK)
		return FOS__FAIL;

	if(ptr->semb_ptr)
		FOS_SemBinaryDelete(p, ptr->semb_ptr->user_desc);

	FOS_Mutex_DeInit(ptr);

	p->var.mutex_desc_list[id] = NULL;

	Private_FOS_UpdMutexMaxInd(p);    // update the maximum index

	return FOS__OK;
}


// acquire mutex
fos_ret_t FOS_MutexTake(fos_t *p, user_desc_t mutex, uint32_t timeout_ms)
{
	fos_mutex_t *ptr = FOS_GetMutexDesc(p, FOS_GetObjectIdByUd(mutex));
	if(FOS_CheckMutexUd(ptr, mutex) != FOS__OK)
		return FOS__FAIL;

	return FOS_Mutex_Take(ptr, p->var.current_thr, timeout_ms);
}


// set owner
fos_ret_t FOS_MutexSetOwner(fos_t *p, user_desc_t mutex)
{
	fos_mutex_t *ptr = FOS_GetMutexDesc(p, FOS_GetObjectIdByUd(mutex));
	if(FOS_CheckMutexUd(ptr, mutex) != FOS__OK)
		return FOS__FAIL;

	return FOS_Mutex_SetOwner(ptr, p->var.current_thr);
}


// release mutex
fos_ret_t FOS_MutexGive(fos_t *p, user_desc_t mutex)
{
	fos_mutex_t *ptr = FOS_GetMutexDesc(p, FOS_GetObjectIdByUd(mutex));
	if(FOS_CheckMutexUd(ptr, mutex) != FOS__OK)
		return FOS__FAIL;

	return FOS_Mutex_Give(ptr, p->var.current_thr);
}

//*****************************************************

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

//*********************************************


// identify error
void FOS_ErrorSet(fos_t *p, fos_err_t *err)
{
	if((p == NULL) || (err == NULL))
		return;

	memcpy((void*)&p->var.error, err, sizeof(fos_err_t));

	if(err->ext_str_ptr != NULL)
		strncpy((void*)p->var.error.str, err->ext_str_ptr, FOS_MAX_STR_ERR_LEN - 1);
}


// write in thread safe mode
fos_ret_t FOS_LogData(fos_t* p, char *str, fos_log_src_t src, fos_log_type_t type)
{
	return FOS_Log_WriteDataProtected((fos_log_t*)&p->var.log, str, src, type, FOS_GetThreadParentUd(p));
}


// read index
fos_ret_t FOS_LogRead(fos_t* p, fos_log_node_t* node_ptr)
{
	return FOS_Log_ReadDataProtected((fos_log_t*)&p->var.log, node_ptr);
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

//*******************************


// unlink thread from the semaphore
fos_ret_t FOS_UnlinkThreadFromSem(fos_t *p, user_desc_t sem, user_desc_t thr_ud)
{
	fos_obj_types_t type = FOS_GetObjectTypeByUd(sem);
	uint8_t thr_id = FOS_GetThreadIdByUd(p, thr_ud);
	if(thr_id == FOS_WRONG_THREAD_ID)
		return FOS__FAIL;

	if(type == FOS_OBJ_TYPE__BINARY_SEM)
		return Private_FOS_UnlinkThreadFromSemb(p, sem, thr_id);
	if(type == FOS_OBJ_TYPE__CNT_SEM)
		return Private_FOS_UnlinkThreadFromSemc(p, sem, thr_id);

	return FOS__FAIL;
}


// unlink thread from the binary semaphore
static fos_ret_t Private_FOS_UnlinkThreadFromSemb(fos_t *p, user_desc_t semb, uint8_t thr_id)
{
	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, FOS_GetObjectIdByUd(semb));
	if(FOS_CheckSemaphoreBinaryUd(ptr, semb) != FOS__OK)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_UnlinkThread(ptr, thr_id);
}


// unlink thread from the counting semaphore
static fos_ret_t Private_FOS_UnlinkThreadFromSemc(fos_t *p, user_desc_t semc, uint8_t thr_id)
{
	fos_semaphore_cnt_t *ptr = FOS_GetSemaphoreCntDesc(p, FOS_GetObjectIdByUd(semc));
	if(FOS_CheckSemaphoreCntUd(ptr, semc) != FOS__OK)
		return FOS__FAIL;

	return FOS_SemaphoreCnt_UnlinkThread(ptr, thr_id);
}

//*******************************


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
	Private_FOS_SetUnprivMode(p);                   // set unpriv mode

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


// set unpriv mode
static void Private_FOS_SetUnprivMode(fos_t *p)
{
	fos_thread_t* tp = p->var.thread_desc_list[p->var.current_thr];
	if(tp->set.unpriv_sw == FOS__ENABLE)
		fos_mgv.unpriv_mode = 1;
	else
		fos_mgv.unpriv_mode = 0;
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
	fos_semaphore_cnt_t    *semc;
	fos_mutex_t            *mut;

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

	/*
	 * Unlink thread from all counting semaphores
	 */
	for(uint8_t i = 0; i <= p->var.semc_max_ind; i++)
	{
		semc = FOS_GetSemaphoreCntDesc(p, i);
		if(semc)
		{
			FOS_SemaphoreCnt_UnlinkThread(semc, thr_id);
		}
	}

	/*
	 * Unlink thread from all mutexes
	 */
	for(uint8_t i = 0; i <= p->var.mutex_max_ind; i++)
	{
		mut = FOS_GetMutexDesc(p, i);
		if(mut)
		{
			FOS_Mutex_UnlinkThread(mut, thr_id);
		}
	}
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













