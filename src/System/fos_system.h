/**************************************************************************//**
 * @file      fos_system.h
 * @brief     System calls. Header file.
 * @version   V1.4.07
 * @date      08.06.2026
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

#ifndef APPLICATION_FOS_SYSTEM_FOS_SYSTEM_H_
#define APPLICATION_FOS_SYSTEM_FOS_SYSTEM_H_


#include "Thread/fos_thread.h"
#include "File/Sys/file_sys.h"


// уступить другому процессу
fos_ret_t SYS_FOS_Yield();

// взять бинарный семафор
fos_ret_t SYS_FOS_SemBinaryTake(user_desc_t semb, uint32_t timeout_ms);

// дать бинарный свнтофор
fos_ret_t SYS_FOS_SemBinaryGive(user_desc_t semb);

// получить дескриптор бинарного семафора потока
user_desc_t SYS_FOS_GetThreadSembDesc(user_desc_t desc);

// создать поток
user_desc_t SYS_FOS_CreateThread(fos_thread_user_init_t *user_init);

// создать бинарный семафор
user_desc_t SYS_FOS_CreateSemBinary(fos_semb_state_t init_state);

// удалить бинарный семафор
fos_ret_t SYS_FOS_DeleteSemBinary(user_desc_t semb);

// запустить поток с дескриптором
fos_ret_t SYS_FOS_RunDesc(user_desc_t desc);

// завершить поток с дескрипттором
fos_ret_t SYS_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code);

// вызвать Hard Fault
fos_ret_t SYS_FOS_HardFaultCall();

// смотнитровать файловую систему
fos_ret_t SYS_File_Mount(uint8_t dev_num);

// размонтировать файловую систему
fos_ret_t SYS_File_Unmount(uint8_t dev_num);

// взять счётный семафор
fos_ret_t SYS_FOS_SemCntTake(user_desc_t semc, uint32_t timeout_ms);

// дать счётный семафор
fos_ret_t SYS_FOS_SemCntGive(user_desc_t semc);

// создать счётный семафор
user_desc_t SYS_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt);

// удалить счётный семафор
fos_ret_t SYS_FOS_DeleteSemCnt(user_desc_t semc);

// create queue32
user_desc_t SYS_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode);

// delete queue32
fos_ret_t SYS_FOS_DeleteQueue32(user_desc_t que);

// ask data from queue32
fos_ret_t SYS_FOS_Queue32AskData(user_desc_t que, uint32_t timeout_ms);

// read data from queue32
// one must ask data before read every times
fos_ret_t SYS_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr);

// write data to queue32
fos_ret_t SYS_FOS_Queue32WriteData(user_desc_t que, uint32_t data);

// is the thhread alive
fos_ret_t SYS_FOS_IsThreadAlive(user_desc_t desc);

/*
 * **************************************************************
 */

// усыпить текущий поток
// используется в слабом подтягивании в file_sys.c, fwriter.c
fos_ret_t SYS_FOS_Sleep(uint32_t time);

// зафиксировать ошибку
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c, fos_heap.c, fos_thread.c
fos_ret_t SYS_FOS_ErrorSet(fos_err_t *err);

// завершить текущий поток
// используется в слабом подтягивании
// used via weak callback in the fos_thread.c
fos_ret_t SYS_FOS_Terminate(int32_t terminate_code);

// get thread arg pointer
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c
uint8_t* SYS_FOS_GetThreadArgPtr();

// get thread arg len
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c
uint32_t SYS_FOS_GetThreadArgLen();

// get ep_wa
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c
user_thread_ep_wa_t SYS_FOS_GetThreadEpA();

/*
 * **************************************************************
 */

// get user descriptor of the current thread
user_desc_t SYS_FOS_GetCurrentThreadUd();

// create the mutex
user_desc_t SYS_FOS_CreateMutex(fos_mutex_type_t type, uint8_t pcp_priority);

// delete the mutex
fos_ret_t SYS_FOS_DeleteMutex(user_desc_t mutex);

// take the mutex with picked descriptor
fos_ret_t SYS_FOS_MutexTake(user_desc_t mutex, uint32_t timeout_ms);

// set owner
fos_ret_t SYS_FOS_MutexSetOwner(user_desc_t mutex);

// release mutex
fos_ret_t SYS_FOS_MutexGive(user_desc_t mutex);

// allocate thread memory
void* SYS_FOS_LocalAlloc(uint32_t size_bytes);

// free thread memory
fos_ret_t SYS_FOS_LocalFree(void* ptr);

// start the thread with the picked descriptor with argument
fos_ret_t SYS_FOS_RunDescWithArg(user_desc_t desc, uint8_t* arg_ptr, uint32_t arg_len);

// set note to thread by user descriptor
fos_ret_t SYS_FOS_SetNoteDesc(user_desc_t desc, fos_note_type_t type, uint32_t note);

// get thread note pointer
fos_thr_note_t* SYS_FOS_GetThreadNotePtr();

// усыпить текущий поток с ожиданием сигнала
fos_ret_t SYS_FOS_Wait(uint32_t time);

// залогировать событие
fos_ret_t SYS_FOS_LogData(char *str, fos_log_type_t type);



#endif /* APPLICATION_FOS_SYSTEM_FOS_SYSTEM_H_ */



















