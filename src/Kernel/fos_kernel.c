/**************************************************************************//**
 * @file      fos_kernel.c
 * @brief     Kernel. Source file.
 * @version   V1.5.19
 * @date      05.05.2026
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


#include "Kernel/fos_kernel.h"
#include "Mem/fos_heap.h"
#include "Platform/sl_platform.h"
#include "Libs/sl_string.h"
#include <stdio.h>


static fos_t fos;                                          // OS

extern uint32_t kernel_stack[FOS_KERNEL_STACK_SIZE / 4];   // kernel stack

static char *FOS_ver = "FOS version 1.0.7 build 10 08.05.2026 api-1.1\r\n\0";  // FOS version

static fwriter_t* fptr = NULL;


// the main loop of the iddle thread
static void Iddle_Main_thr();

#ifdef FOS_USE_FATFS
// the main loop of the file system thread
static void FProc_Main_thr();
// the main loop of the system logger
#ifdef FOS_USE_LOG_TO_FS
static void FOS_SysLogWriter_thr();
#endif
#endif

// create the thread object
static fos_thread_t* Private_Kernel_FOS_CreateThreadObj();

// create the binary semaphore
static fos_semaphore_binary_t* Private_Kernel_FOS_CreateSemBinaryObj();

// create the counting semaphore
static fos_semaphore_cnt_t* Private_Kernel_FOS_CreateSemCntObj();

// create the mutex
static fos_mutex_t* Private_Kernel_FOS_CreateMutexObj();

// create the file writer object
static fwriter_t* Private_Kernel_FOS_CreateFWriterObj();

// create the queue32 object
static fos_queue32_t* Private_Kernel_FOS_CreateQueue32Obj();

// create note
static fos_thr_note_t* Private_Kernel_FOS_CreateNoteObj(uint8_t thr_id);

// create ret val object
static fos_ret_val_t* Private_Kernel_FOS_CreateRValObj(uint8_t thr_id);

// initialize and register the binary semaphore
static fos_ret_t Private_Kernel_FOS_SemBinaryInitAndReg(fos_semaphore_binary_t *semb, fos_semb_state_t init_state);

// initialize and register the counting semaphore
static fos_ret_t Private_Kernel_FOS_SemCntInitAndReg(fos_semaphore_cnt_t *semc, uint32_t max_cnt, uint32_t init_cnt);

// initialize and register the mutex
static fos_ret_t Private_Kernel_FOS_MutexInitAndReg(fos_mutex_t *mut, user_desc_t semb, fos_mutex_type_t type, uint8_t pcp_priority);

// initialize and register the file writer object
static fos_ret_t Private_Kernel_FOS_FWriterInitAndReg(fwriter_t *fw, file_init_t *init);

// initialize and register the queue32 object
static fos_ret_t Private_Kernel_FOS_Queue32InitAndReg(fos_queue32_t *que, uint32_t* buf_ptr, uint16_t buf_size, user_desc_t semc);

// stack error callback procedure
static void Private_Kernel_FOS_Proc_StackErrorCallback(user_desc_t user_desc);

// OS garbage collection
static void Private_Kernel_FOS_GarbageCollection();

// system etry point to start the thread with arguments
static void Private_Kernel_FOS_StartThreadWithArgs();

// the prototype of error catch
// defined in the fos_system.c
__weak void SYS_FOS_ErrorSet(fos_err_t *err)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
}


// get thread arg pointer
// defined in the fos_system.c
__weak  uint8_t* SYS_FOS_GetThreadArgPtr()
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return NULL;
}


// get thread arg len
// defined in the fos_system.c
__weak uint32_t SYS_FOS_GetThreadArgLen()
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return 0;
}


// get ep_wa
// defined in the fos_system.c
__weak user_thread_ep_wa_t SYS_FOS_GetThreadEpA()
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return NULL;
}


// OS initialization
void Kernel_FOS_Init()
{
	user_desc_t       thr;

	FOS_Init(&fos);
	FOS_Heap_Init();

	/*
	 * Для отладки стека ядра
	 * For debugging of the stack
	 */
	fos.sys_stack_dbg.low_sp = (uint32_t)kernel_stack;
	fos.sys_stack_dbg.high_sp = (uint32_t)kernel_stack + FOS_KERNEL_STACK_SIZE;
	fos.sys_stack_dbg.stack_size = FOS_KERNEL_STACK_SIZE;
	fos.sys_stack_dbg.stack_err_cbk = Private_Kernel_FOS_Proc_StackErrorCallback;


	/*
	 * Initialization of the system threads
	 */

	// the iddle thread
	fos_thread_user_init_t user_init = {0};
	user_init.user_thread_ep = Iddle_Main_thr;
	user_init.priority = FOS_PRIORITY_CNT - 1;		// IDDLE
	user_init.stack_size = STACK_SIZE_IDDLE_THR;
	user_init.heap_size  = STACK_SIZE_IDDLE_THR;    // heap has the same size like stack
	user_init.alloc_type = FOS__THREAD_ALLOC_AUTO;
	user_init.name_ptr = "Iddle\0";
	thr = Kernel_FOS_CreateThread(&user_init);
	Kernel_FOS_RunDesc(thr);

#ifdef FOS_USE_FATFS
	// the file system thread
	user_init.user_thread_ep = FProc_Main_thr;
	user_init.priority = FOS_PRIORITY_CNT - 2;		// VERY LOW
	user_init.stack_size = STACK_SIZE_FPROC_THR;
	user_init.heap_size  = FOS_DEF_THR_HEAP_SIZE;
	user_init.alloc_type = FOS__THREAD_ALLOC_AUTO;
	user_init.name_ptr = "FProc\0";
	thr = Kernel_FOS_CreateThread(&user_init);
	Kernel_FOS_RunDesc(thr);

#ifdef FOS_USE_LOG_TO_FS
	fptr = Kernel_CreateFWriter(0x400);
	if(fptr)
	{
		// the logger writer
		user_init.user_thread_ep = FOS_SysLogWriter_thr;
		user_init.priority = FOS_PRIORITY_CNT - 2;		// VERY LOW
		user_init.stack_size = STACK_SIZE_LOGWR_THR;
		user_init.heap_size  = FOS_DEF_THR_HEAP_SIZE;
		user_init.alloc_type = FOS__THREAD_ALLOC_AUTO;
		user_init.name_ptr = "SysLogWriter\0";
		thr = Kernel_FOS_CreateThread(&user_init);
		Kernel_FOS_RunDesc(thr);
	}
#endif
#endif

#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData("FOS is initialized", FOS_LOG_TYPE__INFO);
	Kernel_FOS_LogSysData(FOS_ver, FOS_LOG_TYPE__INFO); // 18 symbols
#endif
}


// OS start
fos_ret_t Kernel_FOS_Start()
{
#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData("FOS is starting", FOS_LOG_TYPE__INFO); // 15 symbols
#endif
	return FOS_Start(&fos);
}


// OS main loop proc
void Kernel_FOS_MainLoopProc()
{
	Private_Kernel_FOS_GarbageCollection();    // garbage collection
	FOS_Heap_MainLoopProc();                   // heap debug
	FOS_MainLoopProc(&fos);                    // kernel proc
}


// creat thread
user_desc_t Kernel_FOS_CreateThread(fos_thread_user_init_t *user_init)
{
	const uint32_t MIN_LEN = 256;

	if(user_init == NULL)
		return FOS_WRONG_USER_DESC;

	if(user_init->name_ptr == NULL)
		return FOS_WRONG_USER_DESC;
	if((user_init->user_thread_ep == NULL) && (user_init->user_thread_ep_wa == NULL))
		return FOS_WRONG_USER_DESC;

	if(user_init->heap_size < MIN_LEN)
		user_init->heap_size = MIN_LEN;
	if(user_init->stack_size < MIN_LEN)
		user_init->stack_size = MIN_LEN;

	/*
	 * Allocate memory for the thread stack and heap
	 */
	uint32_t thread_mem_size = user_init->stack_size + user_init->heap_size;    // calculate thread memory size
	void* thread_mem_ptr = FOS_Heap_ThreadsHeap_Alloc(thread_mem_size);         // allocate it
	if(thread_mem_ptr == NULL)                                                  // error check
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData2("Creating thread error. Cannot allocate thread memory. Name=", user_init->name_ptr, FOS_LOG_TYPE__ERROR); // 59+16+1=76 symbols
#endif
		return FOS_WRONG_USER_DESC;
	}

	/*
	 * Create the thread
	 */
	fos_thread_t *thr_ptr = Private_Kernel_FOS_CreateThreadObj();
	if(thr_ptr == NULL)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData2("Creating thread error. Cannot create the thread object. Name=", user_init->name_ptr, FOS_LOG_TYPE__ERROR); // 61+16+1=78 symbols
#endif
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);
		return FOS_WRONG_USER_DESC;
	}


	/*
	 * Create the binary semaphore
	 */
	user_desc_t semb = Kernel_FOS_CreateSemBinary(FOS_SEMB_STATE__LOCK);
	if(semb == FOS_WRONG_USER_DESC)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData2("Creating thread error. Cannot create semaphore thread. Name=", user_init->name_ptr, FOS_LOG_TYPE__ERROR); // 61+16+1=78 symbols
#endif
		FOS_Heap_KernelHeap_Free(thr_ptr);
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);
		return FOS_WRONG_USER_DESC;
	}


	/*
	 * The thread initialization
	 */
	fos_thread_init_t init = {0};
	init.set.priority = user_init->priority;
	init.cset.priority_def = user_init->priority;
	init.cset.base_sp = (uint32_t)thread_mem_ptr;
	init.cset.stack_size = user_init->stack_size;
	init.cset.alloc_type = user_init->alloc_type;
	init.cset.semb = semb;
	init.name_ptr = user_init->name_ptr;
	if(user_init->user_thread_ep_wa){
		init.cset.ep    = (uint32_t)Private_Kernel_FOS_StartThreadWithArgs;
		init.cset.ep_wa = (uint32_t)user_init->user_thread_ep_wa;
	}else{
		init.cset.ep    = (uint32_t)user_init->user_thread_ep;
		init.cset.ep_wa = 0;
	}
	FOS_ThreadInit(thr_ptr, &init);

	/*
	 * The thread registration
	 */
	uint8_t thr_id = FOS_WRONG_THREAD_ID;
	if(FOS_ThreadReg(&fos, thr_ptr, &thr_id) != FOS__OK)
	{
		// if there is any error, then free all allocated memory
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData2("Creating thread error. Cannot register the thread. Name=", user_init->name_ptr, FOS_LOG_TYPE__ERROR); // 56+16+1=73 symbols
#endif
		FOS_Heap_KernelHeap_Free(thr_ptr);
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);
		Kernel_FOS_DeleteSemBinary(semb);
		return FOS_WRONG_USER_DESC;
	}

	/*
	 * Create the local heap of the thread
	 */
	uint32_t  base_local_heap = ((uint32_t)thread_mem_ptr) + user_init->stack_size;
	fos_ret_t heap_state = FOS_Heap_LocalHeap_Create(thr_id, (uint8_t*)base_local_heap, user_init->heap_size);
	FOS_Thread_SetHeapState(thr_ptr, heap_state);
#if FOS_DEBUL_LEVEL >= 1
	if(heap_state != FOS__OK)
		Kernel_FOS_LogSysData3("Creating thread error. Cannot create heap of the thread. Name=", user_init->name_ptr, thr_ptr->user_desc, FOS_LOG_TYPE__ERROR); // 62+16+10+6=94 symbols
#endif

	/*
	 * Allocate local memory for the note
	 */
	if(heap_state == FOS__OK)
	{
		fos_thr_note_t* note_ptr = Private_Kernel_FOS_CreateNoteObj(thr_id);
		FOS_Thread_AddNotePtr(thr_ptr, note_ptr);
		if(note_ptr)
			note_ptr->sign = FOS_NOTE_SIGN;
		else{
#if FOS_DEBUL_LEVEL >= 1
			Kernel_FOS_LogSysData3("Creating thread error. Cannot allocate memory for the note. Name=", user_init->name_ptr, thr_ptr->user_desc, FOS_LOG_TYPE__ERROR); // 65+16+10+6=97 symbols
#endif
		}

		fos_ret_val_t* rv_ptr = Private_Kernel_FOS_CreateRValObj(thr_id);
		FOS_Thread_AddRValPtr(thr_ptr, rv_ptr);
		if(rv_ptr)
			rv_ptr->sign = FOS_NOTE_SIGN;
		else{
#if FOS_DEBUL_LEVEL >= 1
			Kernel_FOS_LogSysData3("Creating thread error. Cannot allocate memory for the rv. Name=", user_init->name_ptr, thr_ptr->user_desc, FOS_LOG_TYPE__ERROR); // 63+16+10+6=95 symbols
#endif
		}
	}

#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData3("Thread is created. Name=", user_init->name_ptr, thr_ptr->user_desc, FOS_LOG_TYPE__INFO); // 24+16+10+6=56 symbols
#endif

	return thr_ptr->user_desc;
}


// start the thread with the picked descriptor
fos_ret_t Kernel_FOS_RunDesc(user_desc_t desc)
{
#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData3("Thread is running.", "Thread", desc, FOS_LOG_TYPE__INFO); // 18+6+10+6=40 symbols
#endif
	return FOS_RunId(&fos, FOS_GetThreadIdByUd(&fos, desc));
}


// start the thread with the picked descriptor with argument
fos_ret_t Kernel_FOS_RunDescWithArg(user_desc_t desc, uint8_t* arg_ptr, uint32_t arg_len)
{
#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData3("Thread is running with args.", "Thread", desc, FOS_LOG_TYPE__INFO); // 26+6+10+6=50 symbols
#endif
	if((arg_ptr == NULL) || (arg_len == 0))
		return FOS__FAIL;

	uint8_t* mem_ptr = (uint8_t*)FOS_Heap_LocalHeap_Alloc(arg_len, FOS_GetThreadIdByUd(&fos, desc));
	if(mem_ptr == NULL)
		return FOS__FAIL;

	memcpy(mem_ptr, arg_ptr, arg_len);

	return FOS_RunIdWithArg(&fos, FOS_GetThreadIdByUd(&fos, desc), (uint8_t*)mem_ptr, arg_len);
}


// get user descriptor of the current thread
user_desc_t Kernel_FOS_GetCurrentThreadUd()
{
	return FOS_GetThreadParentUd(&fos);      // a current thread is the same as a parrent
}


// get thread arg pointer
uint8_t* Kernel_FOS_GetThreadArgPtr()
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return NULL;
	return FOS_GetThreadArgPtr(&fos);
}


// get thread arg len
uint32_t Kernel_FOS_GetThreadArgLen()
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return 0;
	return FOS_GetThreadArgLen(&fos);
}


// terminate the thread with the picked descriptor
fos_ret_t Kernel_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code)
{
#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData3("Thread is terminating.", "Thread", desc, FOS_LOG_TYPE__INFO); // 22+6+10+6=44 symbols
#endif
	return FOS_TerminateId(&fos, FOS_GetThreadIdByUd(&fos, desc), terminate_code);
}


// terminate the current thread
fos_ret_t Kernel_FOS_Terminate(int32_t terminate_code)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData3("Thread is terminating.", "Thread", Kernel_FOS_GetCurrentThreadUd(), FOS_LOG_TYPE__INFO); // 22+6+10+6=44 symbols
#endif
	return FOS_Terminate(&fos, terminate_code);
}


// check if the thread with the picked descriptor is alive
fos_ret_t Kernel_FOS_IsThreadAlive(user_desc_t desc)
{
	return FOS_IsThreadAlive(&fos, desc);
}


// sleep the current thread
fos_ret_t Kernel_FOS_Sleep(uint32_t time, fos_sw_t is_waiting)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_Sleep(&fos, time, is_waiting);
}


// create the binary semaphore
user_desc_t Kernel_FOS_CreateSemBinary(fos_semb_state_t init_state)
{
	fos_semaphore_binary_t* sb_ptr = Private_Kernel_FOS_CreateSemBinaryObj();
	if(sb_ptr == NULL)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating binary semaphore error. Cannot create the binary semaphore object", FOS_LOG_TYPE__ERROR); // 74 symbols
#endif
		return FOS_WRONG_USER_DESC;
	}

	// initialization and registartion
	if(Private_Kernel_FOS_SemBinaryInitAndReg(sb_ptr, init_state) != FOS__OK)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating binary semaphore error. Cannot register the binary semaphore", FOS_LOG_TYPE__ERROR); // 69 symbols
#endif
		FOS_Heap_KernelHeap_Free(sb_ptr);
		return FOS_WRONG_USER_DESC;
	}

	return sb_ptr->user_desc;
}


// delete the binary semaphore with picked descriptor
fos_ret_t Kernel_FOS_DeleteSemBinary(user_desc_t semb)
{
	return FOS_SemBinaryDelete(&fos, semb);
}


// take the binary semaphore with picked descriptor
fos_ret_t Kernel_FOS_SemBinaryTake(user_desc_t semb, uint32_t timeout_ms)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_SemBinaryTake(&fos, semb, timeout_ms);
}


// get the semaphore binary user descriptor by the thread user descriptor
user_desc_t Kernel_FOS_GetThreadSembDesc(user_desc_t desc)
{
	return FOS_GetThreadSembId(&fos, FOS_GetThreadIdByUd(&fos, desc));
}


// set the error
void Kernel_FOS_ErrorSet(fos_err_t *err)
{
	FOS_ErrorSet(&fos, err);

	while(1){};
}


// creat the counting semaphore
user_desc_t Kernel_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt)
{
	fos_semaphore_cnt_t* sc_ptr = Private_Kernel_FOS_CreateSemCntObj();
	if(sc_ptr == NULL)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating counting semaphore error. Cannot create the counting semaphore object", FOS_LOG_TYPE__ERROR); // 78 symbols
#endif
		return FOS_WRONG_USER_DESC;
	}

	// initialization and registartion
	if(Private_Kernel_FOS_SemCntInitAndReg(sc_ptr, max_cnt, init_cnt) != FOS__OK)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating counting semaphore error. Cannot register the counting semaphore", FOS_LOG_TYPE__ERROR); // 73 symbols
#endif
		FOS_Heap_KernelHeap_Free(sc_ptr);
		return FOS_WRONG_USER_DESC;
	}

	return sc_ptr->user_desc;
}


// delete the counting semaphore
fos_ret_t Kernel_FOS_DeleteSemCnt(user_desc_t semc)
{
	return FOS_SemCntDelete(&fos, semc);
}


// take the counting semaphore
fos_ret_t Kernel_FOS_SemCntTake(user_desc_t semc, uint32_t timeout_ms)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_SemCntTake(&fos, semc, timeout_ms);
}


// create the queue for uint32_t
user_desc_t Kernel_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode)
{
	uint32_t queue_buf_len = size * sizeof(uint32_t);
	void* queue_buf_ptr = FOS_Heap_ThreadsHeap_Alloc(queue_buf_len);
	if(queue_buf_ptr == NULL)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating queue32 error. Cannot allocate queue32 memory", FOS_LOG_TYPE__ERROR); // 54 symbols
#endif
		return FOS_WRONG_USER_DESC;
	}

	fos_queue32_t* que_ptr = Private_Kernel_FOS_CreateQueue32Obj();
	if(que_ptr == NULL)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating queue32 error. Cannot create the queue32 object", FOS_LOG_TYPE__ERROR); // 56 symbols
#endif
		FOS_Heap_ThreadsHeap_Free(queue_buf_ptr);
		return FOS_WRONG_USER_DESC;
	}

	user_desc_t semc = FOS_WRONG_USER_DESC;
	if(mode == FOS_QUEUE_MODE__POLL_AND_BLOCK)
		semc = Kernel_FOS_CreateSemCnt(size, 0);

	// initialization and registartion
	if(Private_Kernel_FOS_Queue32InitAndReg(que_ptr, queue_buf_ptr, size, semc) != FOS__OK)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating queue32 error. Cannot register the queue32 object", FOS_LOG_TYPE__ERROR); // 58 symbols
#endif
		Kernel_FOS_DeleteSemCnt(semc);
		FOS_Heap_KernelHeap_Free(que_ptr);
		FOS_Heap_ThreadsHeap_Free(queue_buf_ptr);
		return FOS_WRONG_USER_DESC;
	}

	return que_ptr->user_desc;
}


// delete the queue32
fos_ret_t Kernel_FOS_DeleteQueue32(user_desc_t que)
{
	return FOS_Queue32Delete(&fos, que);
}


// ask data
fos_ret_t Kernel_FOS_Queue32AskData(user_desc_t que, uint32_t timeout_ms)
{
	return FOS_Queue32AskData(&fos, que, timeout_ms);
}


// read data
// one must ask data before read every times
fos_ret_t Kernel_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr)
{
	return FOS_Queue32ReadData(&fos, que, data_ptr);
}


// get the system stack debug info
fos_thread_dbg_t* Kernel_FOS_GetSysStackDbgInfo()
{
	return FOS_GetSysStackDbgInfo(&fos);
}


// get the scheduler debug info
fos_scheduler_dbg_t* Kernel_FOS_GetSchedulerDbgInfo()
{
	return FOS_GetSchedulerDbgInfo(&fos);
}


// create the mutex
user_desc_t Kernel_FOS_CreateMutex(fos_mutex_type_t type, uint8_t pcp_priority)
{
	fos_mutex_t* mutex_ptr = Private_Kernel_FOS_CreateMutexObj();
	if(mutex_ptr == NULL)
	{
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating mutex error. Cannot create the mutex object", FOS_LOG_TYPE__ERROR); // 52 symbols
#endif
		return FOS_WRONG_USER_DESC;
	}

	user_desc_t semb = Kernel_FOS_CreateSemBinary(FOS_SEMB_STATE__UNLOCK);
	if(semb == FOS_WRONG_USER_DESC)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating mutex error. Cannot create the semaphore of the mutex", FOS_LOG_TYPE__ERROR); // 62 symbols
#endif
		FOS_Heap_KernelHeap_Free(mutex_ptr);
		return FOS_WRONG_USER_DESC;
	}

	// initialization and registartion
	if(Private_Kernel_FOS_MutexInitAndReg(mutex_ptr, semb, type, pcp_priority) != FOS__OK)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating mutex error. Cannot register the mutex object", FOS_LOG_TYPE__ERROR); // 54 symbols
#endif
		Kernel_FOS_DeleteSemBinary(semb);
		FOS_Heap_KernelHeap_Free(mutex_ptr);
		return FOS_WRONG_USER_DESC;
	}

	return mutex_ptr->user_desc;
}


// delete the mutex
fos_ret_t Kernel_FOS_DeleteMutex(user_desc_t mutex)
{
	return FOS_MutexDelete(&fos, mutex);
}


// take the mutex with picked descriptor
fos_ret_t Kernel_FOS_MutexTake(user_desc_t mutex, uint32_t timeout_ms)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_MutexTake(&fos, mutex, timeout_ms);
}


// set owner
fos_ret_t Kernel_FOS_MutexSetOwner(user_desc_t mutex)
{
	return FOS_MutexSetOwner(&fos, mutex);
}


// release mutex
fos_ret_t Kernel_FOS_MutexGive(user_desc_t mutex)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_MutexGive(&fos, mutex);
}


// allocate thread local memory
void* Kernel_FOS_LocalAlloc(uint32_t size_bytes)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return NULL;
	return FOS_Heap_LocalHeap_Alloc(size_bytes, FOS_GetCurrentThreadId(&fos));
}


// free thread local memory
fos_ret_t Kernel_FOS_LocalFree(void* ptr)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	FOS_Heap_LocalHeap_Free(ptr, FOS_GetCurrentThreadId(&fos));
	return FOS__OK;
}


// get thread note pointer
fos_thr_note_t* Kernel_FOS_GetThreadNotePtr()
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return NULL;
	return FOS_GetThreadNotePtr(&fos);
}


// get ep_wa
uint32_t Kernel_FOS_GetThreadEpA()
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return 0;
	return FOS_GetThreadEpA(&fos);
}


// get returned values from the thread
fos_ret_val_t* Kernel_FOS_GetThreadRValPtr()
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return NULL;
	return FOS_GetThreadRValPtr(&fos);
}


// read log
fos_ret_t Kernel_FOS_LogRead(fos_log_node_t* node_ptr)
{
	return FOS_LogRead(&fos, node_ptr);
}


// log system data
fos_ret_t Kernel_FOS_LogSysData(char *str, fos_log_type_t type)
{
	return FOS_LogData(&fos, str, FOS_LOG_SRC__SYS, type);
}


// log system data
fos_ret_t Kernel_FOS_LogSysData2(char *str1, char *str2, fos_log_type_t type)
{
	char str[FOS_MAX_STR_LOG_LEN];
	snprintf(str, FOS_MAX_STR_LOG_LEN, "%s %s", str1, str2);
	return Kernel_FOS_LogSysData(str, type);
}


// log system data
fos_ret_t Kernel_FOS_LogSysData3(char *str1, char *str2, uint32_t val, fos_log_type_t type)
{
	char str[FOS_MAX_STR_LOG_LEN];
	snprintf(str, FOS_MAX_STR_LOG_LEN, "%s %s: ud=0x%08X", str1, str2, (int)val);
	return Kernel_FOS_LogSysData(str, type);
}


/*
 * **************************************************************
 */

// get FOS version
// used via weak callback in the fos_api.c
char* Kernel_FOS_GetVersion()
{
	return FOS_ver;
}


// get log writer pointer
// used via weak callback in the fos_api.c
fwriter_t* Kernel_FOS_GetLogWriterPtr()
{
	return fptr;
}


// create the file writer object
// used via weak callback in the fos_api.c
fwriter_t* Kernel_CreateFWriter(uint16_t write_buf_len)
{
	if(write_buf_len == 0)
		return NULL;

	/*
	 * Limit the buffer len
	 */
	if(write_buf_len > FOS_FILEWR_MAX_BUF_LEN)
		write_buf_len = FOS_FILEWR_MAX_BUF_LEN;

	/*
	 * Allocate the memory at the buffer to write
	 */
	void* write_buf_ptr = FOS_Heap_ThreadsHeap_Alloc(write_buf_len);
	if(write_buf_ptr == NULL)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating file writer error. Cannot allocate file writer memory", FOS_LOG_TYPE__ERROR);  // 62 symbols
#endif
		return NULL;
	}

	/*
	 * Create the file writer object
	 */
	fwriter_t *fwriter_ptr = Private_Kernel_FOS_CreateFWriterObj();
	if(fwriter_ptr == NULL)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating file writer error. Cannot create the file writer object", FOS_LOG_TYPE__ERROR); // 64 symbols
#endif
		FOS_Heap_ThreadsHeap_Free(write_buf_ptr);
		return NULL;
	}

	/*
	 * initialization and registartion
	 */
	file_init_t file_init = {0};
	file_init.cbuf_init.buf_len = write_buf_len;
	file_init.cbuf_init.buf_ptr = write_buf_ptr;
	if(Private_Kernel_FOS_FWriterInitAndReg(fwriter_ptr, &file_init) != FOS__OK)
	{
		// error proc
#if FOS_DEBUL_LEVEL >= 1
	Kernel_FOS_LogSysData("Creating file writer error. Cannot register the file writer object", FOS_LOG_TYPE__ERROR); // 66 symbols
#endif
		FOS_Heap_KernelHeap_Free(fwriter_ptr);
		FOS_Heap_ThreadsHeap_Free(write_buf_ptr);
		return NULL;
	}

	return fwriter_ptr;
}


// give the binary semaphore
// used via weak callback in the fos_system.c
fos_ret_t Kernel_FOS_SemBinaryGive(user_desc_t semb)
{
	return FOS_SemBinaryGive(&fos, semb);
}


// give the counting semaphore
// used via weak callback in the fos_system.c
fos_ret_t Kernel_FOS_SemCntGive(user_desc_t semc)
{
	return FOS_SemCntGive(&fos, semc);
}


// write data to queue32
// used via weak callback in the fos_system.c
fos_ret_t Kernel_FOS_Queue32WriteData(user_desc_t que, uint32_t data)
{
	return FOS_Queue32WriteData(&fos, que, data);
}


// set note to thread by user descriptor
// used via weak callback in the fos_system.c
fos_ret_t Kernel_FOS_SetNoteDesc(user_desc_t desc, fos_note_type_t type, uint32_t note)
{
	return FOS_SetNoteId(&fos, FOS_GetThreadIdByUd(&fos, desc), type, note);
}


// log user data
// used via weak callback in the fos_system.c
fos_ret_t Kernel_FOS_LogUserData(char *str, fos_log_type_t type)
{
	if(type & FOS_LOG_FROM_ISR_BIT)
		return FOS_LogData(&fos, str, FOS_LOG_SRC__ISR, (fos_log_type_t)(type & FOS_LOG_TYPE_MASK));
	else
		return FOS_LogData(&fos, str, FOS_LOG_SRC__USER, (fos_log_type_t)(type & FOS_LOG_TYPE_MASK));
}


// unlink thread from the semaphore
// used via weak callback in the fos_thread.c
fos_ret_t Kernel_FOS_UnlinkThreadFromSem(user_desc_t sem, user_desc_t thr_ud)
{
#if FOS_DEBUL_LEVEL >= 2
	Kernel_FOS_LogSysData3("Sem is timeout.", "Sem with", sem, FOS_LOG_TYPE__WARNING); // 16+9+10+6=41 symbols
	Kernel_FOS_LogSysData3("=>", "Thread unlock with", thr_ud, FOS_LOG_TYPE__WARNING); // 2+18+10+6=36 symbols
#endif
	return FOS_UnlinkThreadFromSem(&fos, sem, thr_ud);
}



/*
 * Callback
 */

// callback to thread lock with picked id
// used via weak callback in the fos_lock.c
void FOS_Lock_LockThread(uint8_t thr_id, user_desc_t lock_obj_ud, uint32_t timeout_ms)
{
	FOS_LockId(&fos, thr_id, FOS_LOCK_OBJ_FLAG, lock_obj_ud, timeout_ms);
}


// callback to thread unlock with picked id
// used via weak callback in the fos_lock.c
void FOS_Lock_UnlockThread(uint8_t thr_id)
{
	FOS_UnlockId(&fos, thr_id, FOS_LOCK_OBJ_FLAG);
}


/*
 * System threads
 */

// the main loop of the iddle thread
static void Iddle_Main_thr()
{
	while(1)
	{
		SL_Delay(10);      // simple blocking dealy
	}
}



#ifdef FOS_USE_FATFS
// the main loop of the file system thread
static void FProc_Main_thr()
{
	fos_t* f = &fos;                // pointer at FOS
	fwriter_t* fwriter = NULL;      // pointer at FWriter
	uint8_t    isDataToWrite;       // the flag is set when there is data to write

	while(1)
	{
		isDataToWrite = 0;          // clear flag

		// goes through the all FWriter id
		for(uint8_t i = 0; i <= f->var.fwriter_max_id; i++)
		{
			fwriter = FOS_GetFWriterDesc(f, i);              // get descriptor of the next FWriter
			if(fwriter != NULL)
			{
				FWriter_SysProc(fwriter);                    // process it

				if(FWriter_GetDataToWrite(fwriter) != 0)     // if there is any data to write
					isDataToWrite = 1;                       // set flag
			}
		}

		if(!isDataToWrite)                                   // if flag is cleared
			FOS_Sleep(f, FOS_WRITE_PERIOD_MS, FOS__DISABLE); // goes to sleep

		File_MountProc();                                    // process device state
	}
}

#ifdef FOS_USE_LOG_TO_FS
// the main loop of the system logger
static void FOS_SysLogWriter_thr()
{
	enum {BUF_LEN = 256};
	fos_ret_t      ret;
	fos_log_node_t node;
	uint32_t       data_len = 0;
	char str[BUF_LEN];

	while(1)
	{
		if(API_FWriter_GetFileState(fptr) == FILE_STATE__OPENED)
		{
			ret = Kernel_FOS_LogRead(&node);
			if(ret == FOS__OK)
			{
				data_len = FOS_LogNode_GetString(&node, str, BUF_LEN);
				ret = API_FWriter_Write(fptr, (uint8_t*)str, data_len);
				while(ret != FOS__OK)
				{
					Kernel_FOS_Sleep(FOS_LOG_PERIOD_MS, FOS__DISABLE);
					ret = API_FWriter_Write(fptr, (uint8_t*)str, data_len);
				}
			}
		}

		Kernel_FOS_Sleep(FOS_LOG_PERIOD_MS, FOS__DISABLE);
	}
}
#endif
#endif

/*
 * Private
 */

// create the thread object
static fos_thread_t* Private_Kernel_FOS_CreateThreadObj()
{
	return (fos_thread_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fos_thread_t));
}


// create the binary semaphore
static fos_semaphore_binary_t* Private_Kernel_FOS_CreateSemBinaryObj()
{
	return (fos_semaphore_binary_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fos_semaphore_binary_t));
}


// create the counting semaphore
static fos_semaphore_cnt_t* Private_Kernel_FOS_CreateSemCntObj()
{
	return (fos_semaphore_cnt_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fos_semaphore_cnt_t));
}


// create the mutex
static fos_mutex_t* Private_Kernel_FOS_CreateMutexObj()
{
	return (fos_mutex_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fos_mutex_t));
}


// create the file writer object
static fwriter_t* Private_Kernel_FOS_CreateFWriterObj()
{
	return (fwriter_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fwriter_t));
}


// create the queue32 object
static fos_queue32_t* Private_Kernel_FOS_CreateQueue32Obj()
{
	return (fos_queue32_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fos_queue32_t));
}


// create note object
static fos_thr_note_t* Private_Kernel_FOS_CreateNoteObj(uint8_t thr_id)
{
	return (fos_thr_note_t*)FOS_Heap_LocalHeap_Alloc(sizeof(fos_thr_note_t), thr_id);
}


// create ret val object
static fos_ret_val_t* Private_Kernel_FOS_CreateRValObj(uint8_t thr_id)
{
	return (fos_ret_val_t*)FOS_Heap_LocalHeap_Alloc(sizeof(fos_ret_val_t), thr_id);
}


// initialize and register the binary semaphore
static fos_ret_t Private_Kernel_FOS_SemBinaryInitAndReg(fos_semaphore_binary_t *semb, fos_semb_state_t init_state)
{
	FOS_SemaphoreBinary_Init(semb, init_state);
	return FOS_SemBinaryReg(&fos, semb);
}


// initialize and register the counting semaphore
static fos_ret_t Private_Kernel_FOS_SemCntInitAndReg(fos_semaphore_cnt_t *semc, uint32_t max_cnt, uint32_t init_cnt)
{
	FOS_SemaphoreCnt_Init(semc, max_cnt, init_cnt);
	return FOS_SemCntReg(&fos, semc);
}


// initialize and register the mutex
static fos_ret_t Private_Kernel_FOS_MutexInitAndReg(fos_mutex_t *mut, user_desc_t semb, fos_mutex_type_t type, uint8_t pcp_priority)
{
	FOS_Mutex_Init(mut, type, pcp_priority);
	FOS_MutexJoinToSemBinary(&fos, mut, semb);
	return FOS_MutexReg(&fos, mut);
}


// initialize and register the file writer object
static fos_ret_t Private_Kernel_FOS_FWriterInitAndReg(fwriter_t *fw, file_init_t *init)
{
	FWriter_Init(fw, init);
	return FOS_FWriterReg(&fos, fw);
}


// initialize and register the queue32 object
static fos_ret_t Private_Kernel_FOS_Queue32InitAndReg(fos_queue32_t *que, uint32_t* buf_ptr, uint16_t buf_size, user_desc_t semc)
{
	FOS_Queue32_Init(que, buf_ptr, buf_size);
	FOS_Queue32JoinToSemCnt(&fos, que, semc);
	return FOS_Queue32Reg(&fos, que);
}


// stack error callback procedure
static void Private_Kernel_FOS_Proc_StackErrorCallback(user_desc_t user_desc)
{
	fos_err_t err = {0};
	err.err_code = FOS_ERROR_KERNEL_STACK;
	err.ext_str_ptr = "Kernel stack overflow\0";
	SYS_FOS_ErrorSet(&err);
}


// OS garbage collection
static void Private_Kernel_FOS_GarbageCollection()
{
	uint32_t adr;
	uint8_t type;

	for(uint8_t i = 0; i < FOS_MAX_OBJ_TO_DEL; i++)
	{
		if(fos.var.obj_to_del_cnt == 0)
			return;

		adr  = fos.var.obj_to_del[i].adr;
		type = fos.var.obj_to_del[i].heap_type;

		if(adr)
		{
			fos.var.obj_to_del[i].adr = 0;
			fos.var.obj_to_del[i].heap_type = 0;
			fos.var.obj_to_del_cnt--;

			switch(type)
			{
			case FOS_KERNEL_HEAP_ID:
				FOS_Heap_KernelHeap_Free((void*)adr);
			break;

			case FOS_THREADS_HEAP_ID:
				FOS_Heap_ThreadsHeap_Free((void*)adr);
			break;
			}
		}
	}
}


// system etry point to start the thread with arguments
static void Private_Kernel_FOS_StartThreadWithArgs()
{
	uint8_t* arg_ptr = SYS_FOS_GetThreadArgPtr();
	uint32_t arg_len = SYS_FOS_GetThreadArgLen();

	user_thread_ep_wa_t ep_wa = SYS_FOS_GetThreadEpA();
	if(ep_wa)
		ep_wa(arg_ptr, arg_len);
}




















