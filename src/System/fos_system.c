/**************************************************************************//**
 * @file      fos_system.c
 * @brief     System calls. Source file.
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


#include "System/fos_system.h"
#include "System/fos_svcall.h"
#include "System/fos_svc_id.h"


// уступить другому процессу
void SYS_FOS_Yield()
{
	system_call(FOS_SYSCALL_FOS_YIELD, NULL);
}


// усыпить текущий поток
// используется в слабом подтягивании
fos_ret_t SYS_FOS_Sleep(uint32_t time)
{
	uint32_t buf[2];
	buf[1] = time;

	system_call(FOS_SYSCALL_FOS_SLEEP, buf);

	return (fos_ret_t)buf[0];
}


// взять бинарный светофор
fos_ret_t SYS_FOS_SemBinaryTake(user_desc_t semb)
{
	uint32_t buf[2];
	buf[1] = (uint32_t)semb;

	system_call(FOS_SYSCALL_FOS_SEMB_TAKE, buf);

	return (fos_ret_t)buf[0];
}


// дать бинарный свнтофор
fos_ret_t SYS_FOS_SemBinaryGive(user_desc_t semb)
{
	uint32_t buf[2];
	buf[1] = (uint32_t)semb;

	system_call(FOS_SYSCALL_FOS_SEMB_GIVE, buf);

	return (fos_ret_t)buf[0];
}


// создать поток
user_desc_t SYS_FOS_CreateThread(fos_thread_user_init_t *user_init)
{
	uint32_t buf[2];
	buf[1] = (uint32_t)user_init;

	system_call(FOS_SYSCALL_FOS_CREATE_THREAD, buf);

	return (user_desc_t)buf[0];
}


// создать бинарный семафор
user_desc_t SYS_FOS_CreateSemBinary(fos_semb_state_t init_state)
{
	uint32_t buf[2];
	buf[1] = (uint32_t)init_state;

	system_call(FOS_SYSCALL_FOS_CREATE_SEMB, buf);

	return (user_desc_t)buf[0];
}


// запустить поток с дескриптором
fos_ret_t SYS_FOS_RunDesc(user_desc_t desc)
{
	uint32_t buf[2];
	buf[1] = (uint32_t)desc;

	system_call(FOS_SYSCALL_FOS_THREAD_RUN, buf);

	return (fos_ret_t)buf[0];
}


// завершить текущий поток
// используется в слабом подтягивании
fos_ret_t SYS_FOS_Terminate(int32_t terminate_code)
{
	uint32_t buf[2];
	buf[1] = (uint32_t)terminate_code;

	system_call(FOS_SYSCALL_FOS_THREAD_TERMINATE, buf);

	return (fos_ret_t)buf[0];
}


// завершить поток с дескрипттором
fos_ret_t SYS_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code)
{
	uint32_t buf[3];
	buf[1] = (uint32_t)desc;
	buf[2] = (uint32_t)terminate_code;

	system_call(FOS_SYSCALL_FOS_THREAD_TERMINATE_D, buf);

	return (fos_ret_t)buf[0];
}


// вызвать Hard Fault
void SYS_FOS_HardFaultCall()
{
	system_call(FOS_HARD_FAULT_CALL_ID, NULL);
}


// зафиксировать ошибку
// используется в слабом подтягивании
void SYS_FOS_ErrorSet(fos_err_t *err)
{
	uint32_t buf[1];
	buf[0] = (uint32_t)err;

	system_call(FOS_SYSCALL_FOS_ERROR_SET, buf);
}


// смотнитровать файловую систему
void SYS_File_Mount(uint8_t dev_num)
{
	uint32_t buf[1];
	buf[0] = (uint32_t)dev_num;

	system_call(FOS_SYSCALL_FILE_MOUNT, buf);
}


// размонтировать файловую систему
void SYS_File_Unmount(uint8_t dev_num)
{
	uint32_t buf[1];
	buf[0] = (uint32_t)dev_num;

	system_call(FOS_SYSCALL_FILE_UNMOUNT, buf);
}



















