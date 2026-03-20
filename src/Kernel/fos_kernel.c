/**************************************************************************//**
 * @file      fos_kernel.c
 * @brief     Kernel. Source file.
 * @version   V1.5.00
 * @date      18.03.2026
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


static fos_t fos;                                          // OS

extern uint32_t kernel_stack[FOS_KERNEL_STACK_SIZE / 4];   // kernel stack

static char *FOS_ver = "FOS version 1.0.4 build 03 18.03.2026 api-1\r\n\0";  // FOS version


// the main loop of the iddle thread
static void Iddle_Main_thr();

// the main loop of the file system thread
#ifdef FOS_USE_FATFS
static void FProc_Main_thr();
#endif

// create the thread object
static fos_thread_t* Private_Kernel_FOS_CreateThreadObj();

// create the binary semaphore
static fos_semaphore_binary_t* Private_Kernel_FOS_CreateSemBinaryObj();

// create the counting semaphore
static fos_semaphore_cnt_t* Private_Kernel_FOS_CreateSemCntObj();

// create the file writer object
static fwriter_t* Private_Kernel_FOS_CreateFWriterObj();

// create the queue32 object
static fos_queue32_t* Private_Kernel_FOS_CreateQueue32Obj();

// initialize and register the binary semaphore
static fos_ret_t Private_Kernel_FOS_SemBinaryInitAndReg(fos_semaphore_binary_t *semb, fos_semb_state_t init_state);

// initialize and register the counting semaphore
static fos_ret_t Private_Kernel_FOS_SemCntInitAndReg(fos_semaphore_cnt_t *semc, uint32_t max_cnt, uint32_t init_cnt);

// initialize and register the file writer object
static fos_ret_t Private_Kernel_FOS_FWriterInitAndReg(fwriter_t *fw, file_init_t *init);

// initialize and register the queue32 object
static fos_ret_t Private_Kernel_FOS_Queue32InitAndReg(fos_queue32_t *que, uint32_t* buf_ptr, uint16_t buf_size, user_desc_t semc);

// stack error callback procedure
static void Private_Kernel_FOS_Proc_StackErrorCallback(user_desc_t user_desc);

// OS garbage collection
static void Private_Kernel_FOS_GarbageCollection();


// the prototype of error catch
// defined in the fos_system.c
__weak void SYS_FOS_ErrorSet(fos_err_t *err)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
}


// get FOS version
char* Kernel_FOS_GetVersion()
{
	return FOS_ver;
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
	user_init.priotity = FOS_PRIORITY_CNT - 1;		// IDDLE
	user_init.stack_size = STACK_SIZE_IDDLE_THR;
	user_init.heap_size  = STACK_SIZE_IDDLE_THR;    // heap has the same size like stack
	user_init.alloc_type = FOS__THREAD_ALLOC_AUTO;
	user_init.name_ptr = "Iddle\0";
	thr = Kernel_FOS_CreateThread(&user_init);
	Kernel_FOS_RunDesc(thr);

#ifdef FOS_USE_FATFS
	// the file system thread
	user_init.user_thread_ep = FProc_Main_thr;
	user_init.priotity = FOS_PRIORITY_CNT - 2;		// VERY LOW
	user_init.stack_size = STACK_SIZE_FPROC_THR;
	user_init.heap_size  = FOS_DEF_THR_HEAP_SIZE;
	user_init.alloc_type = FOS__THREAD_ALLOC_AUTO;
	user_init.name_ptr = "FProc\0";
	thr = Kernel_FOS_CreateThread(&user_init);
	Kernel_FOS_RunDesc(thr);
#endif
}


// OS start
fos_ret_t Kernel_FOS_Start()
{
	return FOS_Start(&fos);
}


// creat thread
user_desc_t Kernel_FOS_CreateThread(fos_thread_user_init_t *user_init)
{
	if(user_init == NULL)
		return FOS_WRONG_USER_DESC;

	/*
	 * Выделяем память под поток
	 * Allocate memory for the thread stack and heap
	 */
	uint32_t thread_mem_size = user_init->stack_size + user_init->heap_size;    // calculate thread memory size
	void* thread_mem_ptr = FOS_Heap_ThreadsHeap_Alloc(thread_mem_size);         // allocate it
	if(thread_mem_ptr == NULL)                                                  // error check
		return FOS_WRONG_USER_DESC;

	/*
	 * Create the thread
	 */
	fos_thread_t *thr_ptr = Private_Kernel_FOS_CreateThreadObj();
	if(thr_ptr == NULL)
	{
		// error proc
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
		FOS_Heap_KernelHeap_Free(thr_ptr);
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);
		return FOS_WRONG_USER_DESC;
	}

	/*
	 * The thread initialization
	 */
	fos_thread_init_t init = {0};
	init.set.priotity = user_init->priotity;
	init.cset.base_sp = (uint32_t)thread_mem_ptr;
	init.cset.stack_size = user_init->stack_size;
	init.cset.ep = (uint32_t)user_init->user_thread_ep;
	init.cset.alloc_type = user_init->alloc_type;
	init.cset.semb = semb;
	init.name_ptr = user_init->name_ptr;
	FOS_ThreadInit(thr_ptr, &init);

	/*
	 * The thread registration
	 */
	if(FOS_ThreadReg(&fos, thr_ptr) != FOS__OK)
	{
		// if there is any error, then free all allocated memory
		FOS_Heap_KernelHeap_Free(thr_ptr);
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);
		Kernel_FOS_DeleteSemBinary(semb);
		return FOS_WRONG_USER_DESC;
	}

	return thr_ptr->user_desc;
}


// start the thread with the picked descriptor
fos_ret_t Kernel_FOS_RunDesc(user_desc_t desc)
{
	return FOS_RunId(&fos, FOS_GetUdThreadId(&fos, desc));
}


// terminate the thread with the picked descriptor
fos_ret_t Kernel_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code)
{
	return FOS_TerminateId(&fos, FOS_GetUdThreadId(&fos, desc), terminate_code);
}


// terminate the current thread
fos_ret_t Kernel_FOS_Terminate(int32_t terminate_code)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_Terminate(&fos, terminate_code);
}


// check if the thread with the picked descriptor is alive
fos_ret_t Kernel_FOS_IsThreadAlive(user_desc_t desc)
{
	return FOS_IsThreadAlive(&fos, desc);
}


// sleep the current thread
fos_ret_t Kernel_FOS_Sleep(uint32_t time)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_Sleep(&fos, time);
}


// create the binary semaphore
user_desc_t Kernel_FOS_CreateSemBinary(fos_semb_state_t init_state)
{
	fos_semaphore_binary_t* sb_ptr = Private_Kernel_FOS_CreateSemBinaryObj();
	if(sb_ptr == NULL)
		return FOS_WRONG_USER_DESC;

	// initialization and registartion
	if(Private_Kernel_FOS_SemBinaryInitAndReg(sb_ptr, init_state) != FOS__OK)
	{
		// error proc
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
fos_ret_t Kernel_FOS_SemBinaryTake(user_desc_t semb)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_SemBinaryTake(&fos, semb);
}


// get the taking status of the binary semaphore
// FOS__OK - normal taking, FOS__FAIL - taking with timeout
fos_ret_t Kernel_FOS_SemBinaryTakeStat(user_desc_t semb)
{
	return FOS_SemBinaryTakeStat(&fos, semb);
}


// set the binary semaphore timeout
fos_ret_t Kernel_FOS_SemBinarySetTimeout(user_desc_t semb, uint32_t timeout_ms)
{
	return FOS_SemBinarySetTimeout(&fos, semb, timeout_ms);
}


// get the semaphore binary user descriptor by the thread user descriptor
user_desc_t Kernel_FOS_GetThreadSembDesc(user_desc_t desc)
{
	return FOS_GetThreadSembId(&fos, FOS_GetUdThreadId(&fos, desc));
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
		return FOS_WRONG_USER_DESC;

	// initialization and registartion
	if(Private_Kernel_FOS_SemCntInitAndReg(sc_ptr, max_cnt, init_cnt) != FOS__OK)
	{
		// error proc
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
fos_ret_t Kernel_FOS_SemCntTake(user_desc_t semc)
{
	if(FOS_System_GetWorkMode() != FOS__USER_WORK_MODE)
		return FOS__FAIL;
	return FOS_SemCntTake(&fos, semc);
}


// get taking status of the counting semaphore
// FOS__OK - normal taking, FOS__FAIL - taking with timeout
fos_ret_t Kernel_FOS_SemCntTakeStat(user_desc_t semc)
{
	return FOS_SemCntTakeStat(&fos, semc);
}


// set the counting semaphore timeout
fos_ret_t Kernel_FOS_SemCntSetTimeout(user_desc_t semc, uint32_t timeout_ms)
{
	return FOS_SemCntSetTimeout(&fos, semc, timeout_ms);
}


// create the queue for uint32_t
user_desc_t Kernel_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode, uint32_t timeout_ms)
{
	uint32_t queue_buf_len = size * sizeof(uint32_t);
	void* queue_buf_ptr = FOS_Heap_ThreadsHeap_Alloc(queue_buf_len);
	if(queue_buf_ptr == NULL)
		return FOS_WRONG_USER_DESC;

	fos_queue32_t* que_ptr = Private_Kernel_FOS_CreateQueue32Obj();
	if(que_ptr == NULL)
		return FOS_WRONG_USER_DESC;

	user_desc_t semc = FOS_WRONG_USER_DESC;
	if(mode == FOS_QUEUE_MODE__POLL_AND_BLOCK)
	{
		semc = Kernel_FOS_CreateSemCnt(size, 0);
		Kernel_FOS_SemCntSetTimeout(semc, timeout_ms);
	}

	// initialization and registartion
	if(Private_Kernel_FOS_Queue32InitAndReg(que_ptr, queue_buf_ptr, size, semc) != FOS__OK)
	{
		// error proc
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
fos_ret_t Kernel_FOS_Queue32AskData(user_desc_t que, fos_queue_sw_t blocking_mode_sw)
{
	return FOS_Queue32AskData(&fos, que, blocking_mode_sw);
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
		return NULL;

	/*
	 * Create the file writer object
	 */
	fwriter_t *fwriter_ptr = Private_Kernel_FOS_CreateFWriterObj();
	if(fwriter_ptr == NULL)
	{
		// error proc
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
		FOS_Heap_KernelHeap_Free(fwriter_ptr);
		FOS_Heap_ThreadsHeap_Free(write_buf_ptr);
		return NULL;
	}

	return fwriter_ptr;
}


// give the binary semaphore
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_SemBinaryGive(user_desc_t semb)
{
	return FOS_SemBinaryGive(&fos, semb);
}


// give the counting semaphore
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_SemCntGive(user_desc_t semc)
{
	return FOS_SemCntGive(&fos, semc);
}


// write data to queue32
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_Queue32WriteData(user_desc_t que, uint32_t data)
{
	return FOS_Queue32WriteData(&fos, que, data);
}


// OS main loop proc
void Kernel_FOS_MainLoopProc()
{
	Private_Kernel_FOS_GarbageCollection();    // garbage collection
	FOS_Heap_MainLoopProc();                   // heap debug
	FOS_MainLoopProc(&fos);                    // kernel proc
}


/*
 * Callback
 */

// callback to thread lock with picked id
// used via weak callback in the fos_locl.c
void FOS_Lock_LockThread(uint8_t thr_id)
{
	FOS_LockId(&fos, thr_id, FOS_LOCK_OBJ_FLAG);
}


// callback to thread unlock with picked id
// used via weak callback in the fos_locl.c
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


// the main loop of the file system thread
#ifdef FOS_USE_FATFS
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
			FOS_Sleep(f, FOS_WRITE_PERIOD_MS);               // goes to sleep

		File_MountProc();                                    // process device state
	}
}
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



/*
 * Пока не пригодилось
 */

// получить id текущего потока
/*uint8_t USER_FOS_GetCurrentThreadId()
{
	return FOS_GetCurrentThreadId(&fos);
}*/


// получить дескриптор текущего потока
/*fos_thread_t* USER_FOS_GetCurrentThreadDesc()
{
	return FOS_GetThreadDesc(&fos, FOS_GetCurrentThreadId(&fos));
}*/


// получить id потока по его дескриптору
/*uint8_t USER_FOS_GetThreadId(fos_thread_t *thr)
{
	return FOS_GetThreadId(&fos, thr);
}*/


// получение дескриптора потока по id
/*fos_thread_t* USER_FOS_GetThreadDesc(uint8_t id)
{
	return FOS_GetThreadDesc(&fos, id);
}*/


// уступить другому потоку
/*void USER_FOS_Yield()
{
	FOS_Yield();
}*/


// успыпить поток с id
/*fos_ret_t USER_FOS_SleepId(uint8_t id, uint32_t time)
{
	return FOS_SleepId(&fos, id, time);
}*/


// усыпить поток с дескриптором
/*fos_ret_t USER_FOS_SleepDesc(fos_thread_ptr desc, uint32_t time)
{
	return FOS_SleepId(&fos, FOS_GetThreadId(&fos, desc), time);
}*/


// запустить поток с id
/*fos_ret_t USER_FOS_RunId(uint8_t id)
{
	return FOS_RunId(&fos, id);
}*/


// разбудить поток с id
/*fos_ret_t USER_FOS_WeakUpId(uint8_t id)
{
	return FOS_WeakUpId(&fos, id);
}*/


// разбудить поток с дескриптором
/*fos_ret_t USER_FOS_WeakUpDesc(fos_thread_ptr desc)
{
	return FOS_WeakUpId(&fos, FOS_GetThreadId(&fos, desc));
}*/


// установить блокировку на поток с id
/*fos_ret_t USER_FOS_LockId(uint8_t id, uint32_t lock)
{
	return FOS_LockId(&fos, id, lock & FOS_USER_LOCK_MASK);
}*/


// установить блокировку на поток с дескриптором
/*fos_ret_t USER_FOS_LockDesc(fos_thread_ptr desc, uint32_t lock)
{
	return FOS_LockId(&fos, FOS_GetThreadId(&fos, desc), lock & FOS_USER_LOCK_MASK);
}*/


// установить блокировку на текущий поток
/*fos_ret_t USER_FOS_Lock(uint32_t lock)
{
	return FOS_Lock(&fos, lock & FOS_USER_LOCK_MASK);
}*/


// снять блокировку с потока с id
/*fos_ret_t USER_FOS_UnlockId(uint8_t id, uint32_t lock)
{
	return FOS_UnlockId(&fos, id, lock & FOS_USER_LOCK_MASK);
}*/


// снять блокировку с потока с дескриптором
/*fos_ret_t USER_FOS_UnlockDesc(fos_thread_ptr desc, uint32_t lock)
{
	return FOS_UnlockId(&fos, FOS_GetThreadId(&fos, desc), lock & FOS_USER_LOCK_MASK);
}*/



















