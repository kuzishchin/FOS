/**************************************************************************//**
 * @file      fos_gates.c
 * @brief     Gates for system call handling. Source file.
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

// set binary semaphore timeout
static void GATE_FOS_SemBinarySetTimeout(void* data);

// взять счётный семафор
static void  GATE_FOS_SemCntTake(void* data);

// дать счётный семафор
static void GATE_FOS_SemCntGive(void* data);

// создать счётный семафор
static void GATE_FOS_CreateSemCnt(void* data);

// удалить счётный семафор
static void GATE_FOS_DeleteSemCnt(void* data);

// set counting semaphore timeout
static void GATE_FOS_SemCntSetTimeout(void* data);

// create queue32
static void GATE_FOS_CreateQueue32(void* data);

// delete queue32
static void GATE_FOS_DeleteQueue32(void* data);

// ask data from queue32
static void GATE_FOS_AskDataQueue32(void* data);

// read data from queue32
// one must ask data before read every times
static void GATE_FOS_ReadDataQueue32(void* data);

// write data to queue32
static void GATE_FOS_WriteDataQueue32(void* data);

// get taking status of binary semaphore
static void  GATE_FOS_SemBinaryTakeStat(void* data);

// get taking status of counting semaphore
static void  GATE_FOS_SemCntTakeStat(void* data);


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

	system_reg_call(GATE_FOS_SemBinarySetTimeout, FOS_SYSCALL_FOS_SEMB_SET_TIMEOUT);

	system_reg_call(GATE_FOS_SemCntTake, FOS_SYSCALL_FOS_SEMC_TAKE);
	system_reg_call(GATE_FOS_SemCntGive, FOS_SYSCALL_FOS_SEMC_GIVE);
	system_reg_call(GATE_FOS_CreateSemCnt, FOS_SYSCALL_FOS_CREATE_SEMC);
	system_reg_call(GATE_FOS_DeleteSemCnt, FOS_SYSCALL_FOS_DELETE_SEMC);
	system_reg_call(GATE_FOS_SemCntSetTimeout, FOS_SYSCALL_FOS_SEMC_SET_TIMEOUT);

	system_reg_call(GATE_FOS_CreateQueue32, FOS_SYSCALL_FOS_QUEUE_32_CREATE);
	system_reg_call(GATE_FOS_DeleteQueue32, FOS_SYSCALL_FOS_QUEUE_32_DELETE);
	system_reg_call(GATE_FOS_ReadDataQueue32, FOS_SYSCALL_FOS_QUEUE_32_READ);
	system_reg_call(GATE_FOS_WriteDataQueue32, FOS_SYSCALL_FOS_QUEUE_32_WRITE);
	system_reg_call(GATE_FOS_AskDataQueue32, FOS_SYSCALL_FOS_QUEUE_32_ASK);

	system_reg_call(GATE_FOS_SemBinaryTakeStat, FOS_SYSCALL_FOS_SEMB_TAKE_STAT);
	system_reg_call(GATE_FOS_SemCntTakeStat, FOS_SYSCALL_FOS_SEMC_TAKE_STAT);
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


// дать бинарный семафор
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


// set binary semaphore timeout
static void GATE_FOS_SemBinarySetTimeout(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemBinarySetTimeout((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
}


// взять счётный семафор
static void  GATE_FOS_SemCntTake(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemCntTake((user_desc_t)buf_ptr[1]);
}


// дать счётный семафор
static void GATE_FOS_SemCntGive(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemCntGive((user_desc_t)buf_ptr[1]);
}


// создать счётный семафор
static void GATE_FOS_CreateSemCnt(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_CreateSemCnt((uint32_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
}


// удалить счётный семафор
static void GATE_FOS_DeleteSemCnt(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_DeleteSemCnt((user_desc_t)buf_ptr[1]);
}


// set counting semaphore timeout
static void GATE_FOS_SemCntSetTimeout(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemCntSetTimeout((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
}


// create queue32
static void GATE_FOS_CreateQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_CreateQueue32((uint16_t)buf_ptr[1], (fos_queue_mode_t)buf_ptr[2], (uint32_t)buf_ptr[3]);
}


// delete queue32
static void GATE_FOS_DeleteQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_DeleteQueue32((user_desc_t)buf_ptr[1]);
}


// ask data from queue32
static void GATE_FOS_AskDataQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_Queue32AskData((user_desc_t)buf_ptr[1], (fos_queue_sw_t)buf_ptr[2]);
}


// read data from queue32
// one must ask data before read every times
static void GATE_FOS_ReadDataQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_Queue32ReadData((user_desc_t)buf_ptr[2], (uint32_t*)&buf_ptr[1]);
}


// write data to queue32
static void GATE_FOS_WriteDataQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_Queue32WriteData((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
}


// get taking status of binary semaphore
static void  GATE_FOS_SemBinaryTakeStat(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemBinaryTakeStat((user_desc_t)buf_ptr[1]);
}


// get taking status of counting semaphore
static void  GATE_FOS_SemCntTakeStat(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)USER_FOS_SemCntTakeStat((user_desc_t)buf_ptr[1]);
}





























