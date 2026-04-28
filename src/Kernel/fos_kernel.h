/**************************************************************************//**
 * @file      fos_kernel.h
 * @brief     Kernel. Header file.
 * @version   V1.5.13
 * @date      27.04.2026
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

#ifndef APPLICATION_FOS_CORE_USER_FOS_H_
#define APPLICATION_FOS_CORE_USER_FOS_H_


#include "Kernel/fos.h"


// OS initialization
void Kernel_FOS_Init();

// OS start
fos_ret_t Kernel_FOS_Start();

// creat thread
user_desc_t Kernel_FOS_CreateThread(fos_thread_user_init_t *user_init);

// start the thread with the picked descriptor
fos_ret_t Kernel_FOS_RunDesc(user_desc_t desc);

// start the thread with the picked descriptor with argument
fos_ret_t Kernel_FOS_RunDescWithArg(user_desc_t desc, uint8_t* arg_ptr, uint32_t arg_len);

// get thread arg pointer
uint8_t* Kernel_FOS_GetThreadArgPtr();

// get thread arg len
uint32_t Kernel_FOS_GetThreadArgLen();

// terminate the thread with the picked descriptor
fos_ret_t Kernel_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code);

// terminate the current thread
fos_ret_t Kernel_FOS_Terminate(int32_t terminate_code);

// check if the thread with the picked descriptor is alive
fos_ret_t Kernel_FOS_IsThreadAlive(user_desc_t desc);

// sleep the current thread
fos_ret_t Kernel_FOS_Sleep(uint32_t time, fos_sw_t is_waiting);

// create the binary semaphore
user_desc_t Kernel_FOS_CreateSemBinary(fos_semb_state_t init_state);

// delete the binary semaphore with picked descriptor
fos_ret_t Kernel_FOS_DeleteSemBinary(user_desc_t semb);

// take the binary semaphore with picked descriptor
fos_ret_t Kernel_FOS_SemBinaryTake(user_desc_t semb);

// get the taking status of the binary semaphore
// FOS__OK - normal taking, FOS__FAIL - taking with timeout
fos_ret_t Kernel_FOS_SemBinaryTakeStat(user_desc_t semb);

// set the binary semaphore timeout
fos_ret_t Kernel_FOS_SemBinarySetTimeout(user_desc_t semb, uint32_t timeout_ms);

// get the semaphore binary user descriptor by the thread user descriptor
user_desc_t Kernel_FOS_GetThreadSembDesc(user_desc_t desc);

// set the error
void Kernel_FOS_ErrorSet(fos_err_t *err);

// creat the counting semaphore
user_desc_t Kernel_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt);

// delete the counting semaphore
fos_ret_t Kernel_FOS_DeleteSemCnt(user_desc_t semc);

// take the counting semaphore
fos_ret_t Kernel_FOS_SemCntTake(user_desc_t semc);

// get taking status of the counting semaphore
// FOS__OK - normal taking, FOS__FAIL - taking with timeout
fos_ret_t Kernel_FOS_SemCntTakeStat(user_desc_t semc);

// set the counting semaphore timeout
fos_ret_t Kernel_FOS_SemCntSetTimeout(user_desc_t semc, uint32_t timeout_ms);

// create the queue for uint32_t
user_desc_t Kernel_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode, uint32_t timeout_ms);

// delete the queue32
fos_ret_t Kernel_FOS_DeleteQueue32(user_desc_t que);

// ask data
fos_ret_t Kernel_FOS_Queue32AskData(user_desc_t que, fos_queue_sw_t blocking_mode_sw);

// read data
// one must ask data before read every times
fos_ret_t Kernel_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr);

// get the system stack debug info
fos_thread_dbg_t* Kernel_FOS_GetSysStackDbgInfo();

// get the scheduler debug info
fos_scheduler_dbg_t* Kernel_FOS_GetSchedulerDbgInfo();

/*
 * **************************************************************
 */

// get FOS version
// used via weak callback in the fos_api.c
char* Kernel_FOS_GetVersion();

// get log writer pointer
// used via weak callback in the fos_api.c
fwriter_t* Kernel_FOS_GetLogWriterPtr();

// create the file writer object
// used via weak callback in the fos_api.c
fwriter_t* Kernel_CreateFWriter(uint16_t write_buf_len);

// give the binary semaphore
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_SemBinaryGive(user_desc_t semb);

// give the counting semaphore
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_SemCntGive(user_desc_t semc);

// write data to queue32
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_Queue32WriteData(user_desc_t que, uint32_t data);

// set note to thread by user descriptor
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_SetNoteDesc(user_desc_t desc, fos_note_type_t type, uint32_t note);

// log user data
// used via weak callback in the fos_api.c
fos_ret_t Kernel_FOS_LogUserData(char *str, fos_log_type_t type);

/*
 * **************************************************************
 */

// get user descriptor of the current thread
user_desc_t Kernel_FOS_GetCurrentThreadUd();

// create the mutex
user_desc_t Kernel_FOS_CreateMutex(uint32_t timeout_ms, fos_mutex_type_t type, uint8_t pcp_priority);

// delete the mutex
fos_ret_t Kernel_FOS_DeleteMutex(user_desc_t mutex);

// take the mutex with picked descriptor
fos_ret_t Kernel_FOS_MutexTake(user_desc_t mutex);

// get taking status of the mutex and set owner
// FOS__OK - normal taking, FOS__FAIL - taking with timeout
fos_ret_t Kernel_FOS_MutexSetOwnerAndTakeStat(user_desc_t mutex);

// release mutex
fos_ret_t Kernel_FOS_MutexGive(user_desc_t mutex);

// allocate thread local memory
void* Kernel_FOS_LocalAlloc(uint32_t size_bytes);

// free thread local memory
fos_ret_t Kernel_FOS_LocalFree(void* ptr);

// get thread note pointer
fos_thr_note_t* Kernel_FOS_GetThreadNotePtr();

// get ep_wa
uint32_t Kernel_FOS_GetThreadEpA();

// read log
fos_ret_t Kernel_FOS_LogRead(fos_log_node_t* node_ptr);

// log system data
fos_ret_t Kernel_FOS_LogSysData(char *str, fos_log_type_t type);

// log system data
fos_ret_t Kernel_FOS_LogSysData2(char *str1, char *str2, fos_log_type_t type);

// log system data
fos_ret_t Kernel_FOS_LogSysData3(char *str1, char *str2, uint32_t val, fos_log_type_t type);

// OS main loop proc
void Kernel_FOS_MainLoopProc();



/*
 * Пока не пригодилось
 */

// получить id текущего потока
//uint8_t USER_FOS_GetCurrentThreadId();

// получить дескриптор текущего потока
//fos_thread_t* USER_FOS_GetCurrentThreadDesc();

// получить id потока по его дескриптору
//uint8_t USER_FOS_GetThreadId(fos_thread_t *thr);

// получение дескриптора потока по id
//fos_thread_t* USER_FOS_GetThreadDesc(uint8_t id);

// запустить поток с id
//fos_ret_t USER_FOS_RunId(uint8_t id);

// уступить другому потоку
//void USER_FOS_Yield();

// успыпить поток с id
//fos_ret_t USER_FOS_SleepId(uint8_t id, uint32_t time);

// усыпить поток с дескриптором
//fos_ret_t USER_FOS_SleepDesc(fos_thread_ptr desc, uint32_t time);

// разбудить поток с id
//fos_ret_t USER_FOS_WeakUpId(uint8_t id);

// разбудить поток с дескриптором
//fos_ret_t USER_FOS_WeakUpDesc(fos_thread_ptr desc);

// установить блокировку на поток с id
//fos_ret_t USER_FOS_LockId(uint8_t id, uint32_t lock);

// установить блокировку на поток с дескриптором
//fos_ret_t USER_FOS_LockDesc(fos_thread_ptr desc, uint32_t lock);

// установить блокировку на текущий поток
//fos_ret_t USER_FOS_Lock(uint32_t lock);

// снять блокировку с потока с id
//fos_ret_t USER_FOS_UnlockId(uint8_t id, uint32_t lock);

// снять блокировку с потока с дескриптором
//fos_ret_t USER_FOS_UnlockDesc(fos_thread_ptr desc, uint32_t lock);



#endif /* APPLICATION_FOS_CORE_USER_FOS_H_ */





