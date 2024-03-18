/**************************************************************************//**
 * @file      fos_system.h
 * @brief     System calls. Header file.
 * @version   V1.0.00
 * @date      14.02.2024
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

// дать бинарный свнтофор
fos_ret_t SYS_FOS_SemBinaryGive(user_desc_t semb);

// создать поток
user_desc_t SYS_FOS_CreateThread(fos_thread_user_init_t *user_init);

// создать бинарный семафор
user_desc_t SYS_FOS_CreateSemBinary(fos_semb_state_t init_state);

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



#endif /* APPLICATION_FOS_SYSTEM_FOS_SYSTEM_H_ */




