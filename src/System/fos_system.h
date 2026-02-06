/**************************************************************************//**
 * @file      fos_system.h
 * @brief     System calls. Header file.
 * @version   V1.2.02
 * @date      23.01.2026
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
void SYS_FOS_Yield();

// усыпить текущий поток
// используется в слабом подтягивании
fos_ret_t SYS_FOS_Sleep(uint32_t time);

// взять бинарный светофор
fos_ret_t SYS_FOS_SemBinaryTake(user_desc_t semb);

// статус взятия бинарного семафор
fos_ret_t SYS_FOS_SemBinaryTakeStat(user_desc_t semb);

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

// завершить текущий поток
// используется в слабом подтягивании
fos_ret_t SYS_FOS_Terminate(int32_t terminate_code);

// завершить поток с дескрипттором
fos_ret_t SYS_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code);

// вызвать Hard Fault
void SYS_FOS_HardFaultCall();

// зафиксировать ошибку
// используется в слабом подтягивании
void SYS_FOS_ErrorSet(fos_err_t *err);

// смотнитровать файловую систему
void SYS_File_Mount(uint8_t dev_num);

// размонтировать файловую систему
void SYS_File_Unmount(uint8_t dev_num);

// set binary semaphore timeout
fos_ret_t SYS_FOS_SemBinarySetTimeout(user_desc_t semb, uint32_t timeout_ms);

// взять счётный семафор
fos_ret_t SYS_FOS_SemCntTake(user_desc_t semc);

// статус взятия счётного семафора
fos_ret_t SYS_FOS_SemCntTakeStat(user_desc_t semc);

// дать счётный семафор
fos_ret_t SYS_FOS_SemCntGive(user_desc_t semc);

// создать счётный семафор
user_desc_t SYS_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt);

// удалить счётный семафор
fos_ret_t SYS_FOS_DeleteSemCnt(user_desc_t semc);

// set counting semaphore timeout
fos_ret_t SYS_FOS_SemCntSetTimeout(user_desc_t semc, uint32_t timeout_ms);

// create queue32
user_desc_t SYS_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode, uint32_t timeout_ms);

// delete queue32
fos_ret_t SYS_FOS_DeleteQueue32(user_desc_t que);

// ask data from queue32
fos_ret_t SYS_FOS_Queue32AskData(user_desc_t que, fos_queue_sw_t blocking_mode_sw);

// read data from queue32
// one must ask data before read every times
fos_ret_t SYS_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr);

// write data to queue32
fos_ret_t SYS_FOS_Queue32WriteData(user_desc_t que, uint32_t data);


#endif /* APPLICATION_FOS_SYSTEM_FOS_SYSTEM_H_ */



















