/**************************************************************************//**
 * @file      fos_gates.c
 * @brief     Gates for system call handling. Source file.
 * @version   V1.2.15
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


#include "Kernel/fos_gates.h"
#include "System/fos_svcall.h"
#include "System/fos_svc_id.h"


static void GATE_FOS_Yield(void* data);

static void GATE_FOS_Sleep(void* data);

static void GATE_FOS_SemBinaryTake(void* data);

static void GATE_FOS_SemBinaryGive(void* data);

static void GATE_FOS_GetThreadSembDesc(void* data);

static void GATE_FOS_CreateThread(void* data);

static void GATE_FOS_CreateSemBinary(void* data);

static void GATE_FOS_DeleteSemBinary(void* data);

static void GATE_FOS_RunDesc(void* data);

static void GATE_FOS_Terminate(void* data);

static void GATE_FOS_TerminateDesc(void* data);

static void GATE_FOS_ErrorSet(void* data);

static void GATE_File_Mount(void* data);

static void GATE_File_Unmount(void* data);

static void GATE_FOS_SemCntTake(void* data);

static void GATE_FOS_SemCntGive(void* data);

static void GATE_FOS_CreateSemCnt(void* data);

static void GATE_FOS_DeleteSemCnt(void* data);

static void GATE_FOS_CreateQueue32(void* data);

static void GATE_FOS_DeleteQueue32(void* data);

static void GATE_FOS_AskDataQueue32(void* data);

static void GATE_FOS_ReadDataQueue32(void* data);

static void GATE_FOS_WriteDataQueue32(void* data);

static void GATE_FOS_IsThreadAlive(void* data);

static void GATE_FOS_GetCurrentThreadUd(void* data);

static void GATE_FOS_CreateMutex(void* data);

static void GATE_FOS_DeleteMutex(void* data);

static void GATE_FOS_MutexTake(void* data);

static void GATE_FOS_MutexSetOwner(void* data);

static void GATE_FOS_MutexGive(void* data);

static void GATE_FOS_LocalAlloc(void* data);

static void GATE_FOS_LocalFree(void* data);

static void GATE_FOS_RunDescWithArg(void* data);

static void GATE_FOS_GetThreadArgPtr(void* data);

static void GATE_FOS_GetThreadArgLen(void* data);

static void GATE_FOS_SetNoteDesc(void* data);

static void GATE_FOS_GetThreadNotePtr(void* data);

static void GATE_FOS_GetThreadEpA(void* data);

static void GATE_FOS_LogData(void* data);




// initialization of all system gates
void GATE_FOS_Init()
{
	system_reg_call(GATE_FOS_CreateThread, FOS_SYSCALL_FOS_CREATE_THREAD);
	system_reg_call(GATE_FOS_RunDesc, FOS_SYSCALL_FOS_THREAD_RUN);
	system_reg_call(GATE_FOS_RunDescWithArg, FOS_SYSCALL_FOS_THREAD_RUN_WITH_ARG);
	system_reg_call(GATE_FOS_GetThreadArgPtr, FOS_SYSCALL_FOS_THREAD_ARG_GET_PTR);
	system_reg_call(GATE_FOS_GetThreadArgLen, FOS_SYSCALL_FOS_THREAD_ARG_GET_LEN);
	system_reg_call(GATE_FOS_Terminate, FOS_SYSCALL_FOS_THREAD_TERMINATE);
	system_reg_call(GATE_FOS_TerminateDesc, FOS_SYSCALL_FOS_THREAD_TERMINATE_D);
	system_reg_call(GATE_FOS_IsThreadAlive, FOS_SYSCALL_FOS_IS_THREAD_ALIVE);
	system_reg_call(GATE_FOS_GetThreadSembDesc, FOS_SYSCALL_FOS_GET_THREAD_SEMB_D);

	system_reg_call(GATE_FOS_Yield, FOS_SYSCALL_FOS_YIELD);
	system_reg_call(GATE_FOS_Sleep, FOS_SYSCALL_FOS_SLEEP);

	system_reg_call(GATE_FOS_LocalAlloc, FOS_SYSCALL_FOS_LOCAL_ALLOC);
	system_reg_call(GATE_FOS_LocalFree, FOS_SYSCALL_FOS_LOCAL_FREE);

	system_reg_call(GATE_FOS_SetNoteDesc, FOS_SYSCALL_FOS_THREAD_SET_NOTE);
	system_reg_call(GATE_FOS_GetThreadNotePtr, FOS_SYSCALL_FOS_THREAD_NOTE_GET_PTR);

	system_reg_call(GATE_FOS_GetThreadEpA, FOS_SYSCALL_FOS_THREAD_GET_EP_WA);

	system_reg_call(GATE_FOS_CreateSemBinary, FOS_SYSCALL_FOS_CREATE_SEMB);
	system_reg_call(GATE_FOS_DeleteSemBinary, FOS_SYSCALL_FOS_DELETE_SEMB);
	system_reg_call(GATE_FOS_SemBinaryTake, FOS_SYSCALL_FOS_SEMB_TAKE);
	system_reg_call(GATE_FOS_SemBinaryGive, FOS_SYSCALL_FOS_SEMB_GIVE);

	system_reg_call(GATE_FOS_GetCurrentThreadUd, FOS_SYSCALL_FOS_GET_CURRENT_THR_UD);

	system_reg_call(GATE_FOS_CreateSemCnt, FOS_SYSCALL_FOS_CREATE_SEMC);
	system_reg_call(GATE_FOS_DeleteSemCnt, FOS_SYSCALL_FOS_DELETE_SEMC);
	system_reg_call(GATE_FOS_SemCntTake, FOS_SYSCALL_FOS_SEMC_TAKE);
	system_reg_call(GATE_FOS_SemCntGive, FOS_SYSCALL_FOS_SEMC_GIVE);

	system_reg_call(GATE_FOS_CreateMutex, FOS_SYSCALL_FOS_CREATE_MUTEX);
	system_reg_call(GATE_FOS_DeleteMutex, FOS_SYSCALL_FOS_DELETE_MUTEX);
	system_reg_call(GATE_FOS_MutexTake, FOS_SYSCALL_FOS_MUTEX_TAKE);
	system_reg_call(GATE_FOS_MutexSetOwner, FOS_SYSCALL_FOS_MUTEX_SET_OWNER);
	system_reg_call(GATE_FOS_MutexGive, FOS_SYSCALL_FOS_MUTEX_GIVE);

	system_reg_call(GATE_FOS_CreateQueue32, FOS_SYSCALL_FOS_QUEUE_32_CREATE);
	system_reg_call(GATE_FOS_DeleteQueue32, FOS_SYSCALL_FOS_QUEUE_32_DELETE);
	system_reg_call(GATE_FOS_ReadDataQueue32, FOS_SYSCALL_FOS_QUEUE_32_READ);
	system_reg_call(GATE_FOS_WriteDataQueue32, FOS_SYSCALL_FOS_QUEUE_32_WRITE);
	system_reg_call(GATE_FOS_AskDataQueue32, FOS_SYSCALL_FOS_QUEUE_32_ASK);

	system_reg_call(GATE_FOS_ErrorSet, FOS_SYSCALL_FOS_ERROR_SET);
	system_reg_call(GATE_FOS_LogData, FOS_SYSCALL_FOS_LOG_USER_DATA);

	system_reg_call(GATE_File_Mount, FOS_SYSCALL_FILE_MOUNT);
	system_reg_call(GATE_File_Unmount, FOS_SYSCALL_FILE_UNMOUNT);
}


static void GATE_FOS_Yield(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)FOS_Yield();
}


static void GATE_FOS_Sleep(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_Sleep(buf_ptr[1], (fos_sw_t)buf_ptr[2]);
	buf_ptr[3] = (uint32_t)Kernel_FOS_GetThreadNotePtr();
}


static void GATE_FOS_SemBinaryTake(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_SemBinaryTake((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
	buf_ptr[3] = (uint32_t)Kernel_FOS_GetThreadRValPtr();
}


static void GATE_FOS_SemBinaryGive(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_SemBinaryGive((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_GetThreadSembDesc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_GetThreadSembDesc((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_CreateThread(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_CreateThread((fos_thread_user_init_t*)buf_ptr[1]);
}


static void GATE_FOS_CreateSemBinary(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_CreateSemBinary((fos_semb_state_t)buf_ptr[1]);
}


static void GATE_FOS_DeleteSemBinary(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_DeleteSemBinary((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_RunDesc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_RunDesc((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_Terminate(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_Terminate((int32_t)buf_ptr[1]);
}


static void GATE_FOS_TerminateDesc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_TerminateDesc((user_desc_t)buf_ptr[1], (int32_t)buf_ptr[2]);
}


static void GATE_FOS_ErrorSet(void* data)
{
	uint32_t *buf_ptr = data;
	Kernel_FOS_ErrorSet((fos_err_t*)buf_ptr[0]);
}


static void GATE_File_Mount(void* data)
{
	uint32_t *buf_ptr = data;
	File_Mount((uint8_t)buf_ptr[0]);
}


static void GATE_File_Unmount(void* data)
{
	uint32_t *buf_ptr = data;
	File_Unmount((uint8_t)buf_ptr[0]);
}


static void GATE_FOS_SemCntTake(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_SemCntTake((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
	buf_ptr[3] = (uint32_t)Kernel_FOS_GetThreadRValPtr();
}


static void GATE_FOS_SemCntGive(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_SemCntGive((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_CreateSemCnt(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_CreateSemCnt((uint32_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
}


static void GATE_FOS_DeleteSemCnt(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_DeleteSemCnt((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_CreateQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_CreateQueue32((uint16_t)buf_ptr[1], (fos_queue_mode_t)buf_ptr[2]);
}


static void GATE_FOS_DeleteQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_DeleteQueue32((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_AskDataQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_Queue32AskData((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
	buf_ptr[3] = (uint32_t)Kernel_FOS_GetThreadRValPtr();
}


static void GATE_FOS_ReadDataQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_Queue32ReadData((user_desc_t)buf_ptr[2], (uint32_t*)&buf_ptr[1]);
}


static void GATE_FOS_WriteDataQueue32(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_Queue32WriteData((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
}


static void GATE_FOS_IsThreadAlive(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_IsThreadAlive((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_GetCurrentThreadUd(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_GetCurrentThreadUd();
}


static void GATE_FOS_CreateMutex(void* data)
{
	uint32_t *buf_ptr = data;
	fos_mutex_type_t type = (fos_mutex_type_t)((buf_ptr[1]) & 0xFF);
	uint8_t pcp_priority = (uint8_t)((buf_ptr[1] >> 8) & 0xFF);
	buf_ptr[0] = (uint32_t)Kernel_FOS_CreateMutex(type, pcp_priority);
}


static void GATE_FOS_DeleteMutex(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_DeleteMutex((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_MutexTake(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_MutexTake((user_desc_t)buf_ptr[1], (uint32_t)buf_ptr[2]);
	buf_ptr[3] = (uint32_t)Kernel_FOS_GetThreadRValPtr();
}


static void GATE_FOS_MutexSetOwner(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_MutexSetOwner((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_MutexGive(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_MutexGive((user_desc_t)buf_ptr[1]);
}


static void GATE_FOS_LocalAlloc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_LocalAlloc((uint32_t)buf_ptr[1]);
}


static void GATE_FOS_LocalFree(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_LocalFree((void*)buf_ptr[1]);
}


static void GATE_FOS_RunDescWithArg(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_RunDescWithArg((user_desc_t)buf_ptr[1], (uint8_t*)buf_ptr[2], (uint32_t)buf_ptr[3]);
}


static void GATE_FOS_GetThreadArgPtr(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_GetThreadArgPtr();
}


static void GATE_FOS_GetThreadArgLen(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_GetThreadArgLen();
}


static void GATE_FOS_SetNoteDesc(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_SetNoteDesc((user_desc_t)buf_ptr[1], (fos_note_type_t)buf_ptr[2], (uint32_t)buf_ptr[3]);
}


static void GATE_FOS_GetThreadNotePtr(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_GetThreadNotePtr();
}


static void GATE_FOS_GetThreadEpA(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_GetThreadEpA();
}


static void GATE_FOS_LogData(void* data)
{
	uint32_t *buf_ptr = data;
	buf_ptr[0] = (uint32_t)Kernel_FOS_LogUserData((char*)buf_ptr[1], (fos_log_type_t)buf_ptr[2]);
}
















