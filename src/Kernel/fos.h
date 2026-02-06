/**************************************************************************//**
 * @file      fos.h
 * @brief     Kernel libs. Header file.
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

#ifndef APPLICATION_FOS_CORE_FOS_H_
#define APPLICATION_FOS_CORE_FOS_H_


#include "System/fos_context.h"
#include "Thread/fos_scheduler.h"
#include "Sync/fos_semb.h"
#include "Sync/fos_sem.h"
#include "File/fwriter.h"
#include "Data/fos_queue32.h"

/*
 * A thread is described by index and descriptor
 * Thread descriptor characterizes fos_thread_t
 * Thread index is the thread number in the table of thread descriptors
 */

extern fos_mgv_t fos_mgv;             // main global variables

// objects to delete node
typedef struct
{
	uint32_t adr;           // object addres
	uint8_t  heap_type;     // heap type where the object is located

} obj_to_del_t;


// OS variables
typedef struct
{
	volatile fos_sw_t       fos_sw;                                    // main OS switch

	volatile uint8_t        current_thr;                               // current thread index
	volatile uint8_t        thread_max_ind;                            // maximum index of registered thread
	volatile fos_thread_ptr thread_desc_list[FOS_MAX_THR_CNT];         // list of thread descriptors

	volatile uint8_t                  semb_max_ind;                    // maximum index of registered binary semaphore
	volatile fos_semaphore_binary_ptr semb_desc_list[FOS_SEM_BIN_CNT]; // list of binary semaphore descriptors

	volatile uint8_t               semc_max_ind;                         // maximum index of registered counting semaphore
	volatile fos_semaphore_cnt_ptr semc_desc_list[FOS_SEM_COUNTING_CNT]; // list of counting semaphore descriptors

	volatile uint8_t         queue32_max_ind;                         // maximum index of registered queue32
	volatile fos_queue32_ptr queue32_desc_list[FOS_SEM_QUEUE_32_CNT]; // list of queue32 descriptors

	volatile uint8_t     fwriter_max_id;                               // maximum index of registered writer object
	volatile fwriter_ptr fwriter_desc_list[FOS_FWRITER_CNT];           // list of writer object descriptors

	volatile fos_err_t   error;                                        // identified error

	volatile user_desc_t last_user_desc;                               // last used user desсriptor

	volatile uint8_t  obj_to_del_cnt;                                  // count objects to delete
	volatile obj_to_del_t obj_to_del[FOS_MAX_OBJ_TO_DEL];              // list of addres of objects to delete

} fos_var_t;

// OS basic structure
typedef struct
{
	fos_var_t        var;               // variables
	fos_scheduler_t  sheduler;          // scheduler
	fos_thread_dbg_t sys_stack_dbg;     // system stack debug

} fos_t;


// OS initialization
void FOS_Init(fos_t *p);

// OS startup
fos_ret_t FOS_Start(fos_t *p);

// get thread identifier by user defined descriptor
uint8_t FOS_GetUdThreadId(fos_t *p, user_desc_t user_desc);

// get semaphore binary user descriptor by thread ID
user_desc_t FOS_GetThreadSembId(fos_t *p, uint8_t id);

// thread registration
fos_ret_t FOS_ThreadReg(fos_t *p, fos_thread_t *thr);

// start thread with identifier
fos_ret_t FOS_RunId(fos_t *p, uint8_t id);

// terminate thread with identifier
fos_ret_t FOS_TerminateId(fos_t *p, uint8_t id, int32_t terminate_code);

// terminate current thread
fos_ret_t FOS_Terminate(fos_t *p, int32_t terminate_code);

// yield to another process
void FOS_Yield();

//send thread with identifier to sleep
//fos_ret_t FOS_SleepId(fos_t *p, uint8_t id, uint32_t time);

// send current thread to sleep
fos_ret_t FOS_Sleep(fos_t *p, uint32_t time);

// set blocking to thread with identifier
fos_ret_t FOS_LockId(fos_t *p, uint8_t id, uint32_t lock);

// unblock thread with identifier
fos_ret_t FOS_UnlockId(fos_t *p, uint8_t id, uint32_t lock);

// register binary semaphore
fos_ret_t FOS_SemBinaryReg(fos_t *p, fos_semaphore_binary_t *semb);

// delete binary semaphore
fos_ret_t FOS_SemBinaryDelete(fos_t *p, user_desc_t semb);

// acquire binary semaphore
fos_ret_t FOS_SemBinaryTake(fos_t *p, user_desc_t semb);

// get taking status of binary semaphore
// FOS__OK - normal taking, FOS__FAIL - taking with timeout
fos_ret_t FOS_SemBinaryTakeStat(fos_t *p, user_desc_t semb);

// release binary semaphore
fos_ret_t FOS_SemBinaryGive(fos_t *p, user_desc_t semb);

// set binary semaphore timeout
fos_ret_t FOS_SemBinarySetTimeout(fos_t *p, user_desc_t semb, uint32_t timeout_ms);

// get writer object descriptor by its identifier
fwriter_t* FOS_GetFWriterDesc(fos_t *p, uint8_t id);

// register writer object
fos_ret_t FOS_FWriterReg(fos_t *p, fwriter_t *fw);

// identify error
void FOS_ErrorSet(fos_t *p, fos_err_t *err);

// register counting semaphore
fos_ret_t FOS_SemCntReg(fos_t *p, fos_semaphore_cnt_t *semc);

// delete counting semaphore
fos_ret_t FOS_SemCntDelete(fos_t *p, user_desc_t semc);

// acquire counting semaphore
fos_ret_t FOS_SemCntTake(fos_t *p, user_desc_t semc);

// get status of acquire counting semaphore
fos_ret_t FOS_SemCntTakeStat(fos_t *p, user_desc_t semc);

// release counting semaphore
fos_ret_t FOS_SemCntGive(fos_t *p, user_desc_t semc);

// set counting semaphore timeout
fos_ret_t FOS_SemCntSetTimeout(fos_t *p, user_desc_t semc, uint32_t timeout_ms);

// register queue32
fos_ret_t FOS_Queue32Reg(fos_t *p, fos_queue32_t *que);

// join counting semaphore to queue32
fos_ret_t FOS_Queue32JoinToSemCnt(fos_t *p, fos_queue32_t *que, user_desc_t semc);

// delete queue32
fos_ret_t FOS_Queue32Delete(fos_t *p, user_desc_t que);

// ask data
fos_ret_t FOS_Queue32AskData(fos_t *p, user_desc_t que, fos_queue_sw_t blocking_mode_sw);

// read data
// one must ask data before read every times
fos_ret_t FOS_Queue32ReadData(fos_t *p, user_desc_t que, uint32_t* data_ptr);

// write data
fos_ret_t FOS_Queue32WriteData(fos_t *p, user_desc_t que, uint32_t data);

// get the system stack debug info
fos_thread_dbg_t* FOS_GetSysStackDbgInfo(fos_t *p);

// get the scheduler debug info
fos_scheduler_dbg_t* FOS_GetSchedulerDbgInfo(fos_t *p);

// main loop handler
void FOS_MainLoopProc(fos_t *p);


/*
 * Currently not used
 */

// get id of current thread
//uint8_t FOS_GetCurrentThreadId(fos_t *p);

// wake up thread
//fos_ret_t FOS_WeakUpId(fos_t *p, uint8_t id);

// block current thread
//fos_ret_t FOS_Lock(fos_t *p, uint32_t lock);


#endif /* APPLICATION_FOS_CORE_FOS_H_ */










