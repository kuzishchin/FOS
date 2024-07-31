/**************************************************************************//**
 * @file      fos_gates.c
 * @brief     Gates for system call handling. Source file.
 * @version   V1.0.01
 * @date      04.04.2024
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


#include "Kernel/fos_gates.h"
#include "System/fos_svcall.h"
#include "System/fos_svc_id.h"

// уступить другому процессу
static void GATE_FOS_Yield(void* data);

// усыпить текущий поток
static void GATE_FOS_Sleep(void* data);

// взять бинарный семафор
static void  GATE_FOS_SemBinaryTake(void* data);

// дать бинарный свнтофор
static void GATE_FOS_SemBinaryGive(void* data);

// get semaphore binary user descriptor by thread user descriptor
static void GATE_FOS_GetThreadSembDesc(void* data);

// создать поток
static void GATE_FOS_CreateThread(void* data);

// создать бинарный семафор
static void GATE_FOS_CreateSemBinary(void* data);

// удалить бинарный семафор
static void GATE_FOS_DeleteSemBinary(void* data);

// запустить поток с дескриптором
static void GATE_FOS_RunDesc(void* data);

// завершить текущий поток
static void GATE_FOS_Terminate(void* data);

// завершить поток с дескрипттором
static void GATE_FOS_TerminateDesc(void* data);

// зафиксировать ошибку
static void GATE_FOS_ErrorSet(void* data);

// смонитровать файловую систему
static void GATE_File_Mount(void* data);

// размонитровать файловую систему
static void GATE_File_Unmount(void* data);


// инициализировать шлюзы системных вызовов
void GATE_FOS_Init()
{
	system_reg_call(GATE_FOS_Yield, FOS_SYSCALL_FOS_YIELD);
	system_reg_call(GATE_FOS_Sleep, FOS_SYSCALL_FOS_SLEEP);

	system_reg_call(GATE_FOS_SemBinaryTake, FOS_SYSCALL_FOS_SEMB_TAKE);
	system_reg_call(GATE_FOS_SemBinaryGive, FOS_SYSCALL_FOS_SEMB_GIVE);
	system_reg_call(GATE_FOS_GetThreadSembDesc, FOS_SYSCALL_FOS_GET_THREAD_SEMB_D);

	system_reg_call(GATE_FOS_CreateThread, FOS_SYSCALL_FOS_CREATE_THREAD);
	system_reg_call(GATE_FOS_CreateSemBinary, FOS_SYSCALL_FOS_CREATE_SEMB);
	system_reg_call(GATE_FOS_DeleteSemBinary, FOS_SYSCALL_FOS_DELETE_SEMB);

	system_reg_call(GATE_FOS_RunDesc, FOS_SYSCALL_FOS_THREAD_RUN);
	system_reg_call(GATE_FOS_Terminate, FOS_SYSCALL_FOS_THREAD_TERMINATE);
	system_reg_call(GATE_FOS_TerminateDesc, FOS_SYSCALL_FOS_THREAD_TERMINATE_D);

	system_reg_call(GATE_FOS_ErrorSet, FOS_SYSCALL_FOS_ERROR_SET);

	system_reg_call(GATE_File_Mount, FOS_SYSCALL_FILE_MOUNT);
	system_reg_call(GATE_File_Unmount, FOS_SYSCALL_FILE_UNMOUNT);
}


// уступить другому процессу
static void GATE_FOS_Yield(void* data)
{
	FOS_Yield();
}


// усыпить текущий поток
static void GATE_FOS_Sleep(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_Sleep(buf_ptr[1]);
}


// взять бинарный семафор
static void  GATE_FOS_SemBinaryTake(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemBinaryTake((user_desc_t)buf_ptr[1]);
}


// дать бинарный свнтофор
static void GATE_FOS_SemBinaryGive(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemBinaryGive((user_desc_t)buf_ptr[1]);
}


// get semaphore binary user descriptor by thread user descriptor
static void GATE_FOS_GetThreadSembDesc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_GetThreadSembDesc((user_desc_t)buf_ptr[1]);
}


// создать поток
static void GATE_FOS_CreateThread(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_CreateThread((fos_thread_user_init_t*)buf_ptr[1]);
}


// создать бинарный семафор
static void GATE_FOS_CreateSemBinary(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_CreateSemBinary((fos_semb_state_t)buf_ptr[1]);
}


// удалить бинарный семафор
static void GATE_FOS_DeleteSemBinary(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_DeleteSemBinary((user_desc_t)buf_ptr[1]);
}


// запустить поток с дескриптором
static void GATE_FOS_RunDesc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_RunDesc((user_desc_t)buf_ptr[1]);
}


// завершить текущий поток
static void GATE_FOS_Terminate(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_Terminate((int32_t)buf_ptr[1]);
}


// завершить поток с дескрипттором
static void GATE_FOS_TerminateDesc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_TerminateDesc((user_desc_t)buf_ptr[1], (int32_t)buf_ptr[2]);
}


// зафиксировать ошибку
static void GATE_FOS_ErrorSet(void* data)
{
	uint32_t *buf_ptr = data;
	USER_FOS_ErrorSet((fos_err_t*)buf_ptr[0]);
}


// смонитровать файловую систему
static void GATE_File_Mount(void* data)
{
	uint32_t *buf_ptr = data;
	File_Mount((uint8_t)buf_ptr[0]);
}


// размонитровать файловую систему
static void GATE_File_Unmount(void* data)
{
	uint32_t *buf_ptr = data;
	File_Unmount((uint8_t)buf_ptr[0]);
}













