/**************************************************************************//**
 * @file      fos_system.c
 * @brief     System calls. Source file.
 * @version   V1.4.03
 * @date      06.05.2026
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


// prototype of kernel function
// kernel function is used, not indicated in the header file
// defined in the fos_kernel.c
__weak fos_ret_t Kernel_FOS_SetNoteDesc(user_desc_t desc, fos_note_type_t type, uint32_t note)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}


// prototype of kernel function
// kernel function is used, not indicated in the header file
// defined in the fos_kernel.c
__weak fos_ret_t Kernel_FOS_SemBinaryGive(user_desc_t semb)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}


// prototype of kernel function
// kernel function is used, not indicated in the header file
// defined in the fos_kernel.c
__weak fos_ret_t Kernel_FOS_SemCntGive(user_desc_t semb)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}


// prototype of kernel function
// kernel function is used, not indicated in the header file
// defined in the fos_kernel.c
__weak fos_ret_t Kernel_FOS_Queue32WriteData(user_desc_t que, uint32_t data)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}


// prototype of logging user data
// kernel function is used, not indicated in the header file
// defined in the fos_kernel.c
__weak fos_ret_t Kernel_FOS_LogUserData(char *str, fos_log_type_t type)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}



// уступить другому процессу
fos_ret_t SYS_FOS_Yield()
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[1];

	system_call(FOS_SYSCALL_FOS_YIELD, buf);

	return (fos_ret_t)buf[0];
}


// взять бинарный семафор
fos_ret_t SYS_FOS_SemBinaryTake(user_desc_t semb, uint32_t timeout_ms)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[4];
	buf[1] = (uint32_t)semb;
	buf[2] = (uint32_t)timeout_ms;

	system_call(FOS_SYSCALL_FOS_SEMB_TAKE, buf);

	fos_ret_t ret = (fos_ret_t)buf[0];
	if(ret != FOS__OK)
		return FOS__FAIL;

	if(timeout_ms == 0)
		return FOS__OK;

	fos_ret_val_t* rv_ptr = (fos_ret_val_t*)buf[3];
	if(rv_ptr == NULL)
		return FOS__FAIL;
	if(rv_ptr->sign != FOS_NOTE_SIGN)
		return FOS__FAIL;

	if(rv_ptr->timeout_flag == FOS__DISABLE)
		return FOS__OK;
	return FOS__FAIL;
}


// дать бинарный семафор
fos_ret_t SYS_FOS_SemBinaryGive(user_desc_t semb)
{
	if(__get_IPSR() != 0)
		return Kernel_FOS_SemBinaryGive(semb);

	uint32_t buf[2];
	buf[1] = (uint32_t)semb;

	system_call(FOS_SYSCALL_FOS_SEMB_GIVE, buf);

	return (fos_ret_t)buf[0];
}


// получить дескриптор бинарного семафора потока
user_desc_t SYS_FOS_GetThreadSembDesc(user_desc_t desc)
{
	if(__get_IPSR() != 0)
		return FOS_WRONG_USER_DESC;

	uint32_t buf[2];
	buf[1] = (uint32_t)desc;

	system_call(FOS_SYSCALL_FOS_GET_THREAD_SEMB_D, buf);

	return (user_desc_t)buf[0];
}


// создать поток
user_desc_t SYS_FOS_CreateThread(fos_thread_user_init_t *user_init)
{
	if(__get_IPSR() != 0)
		return FOS_WRONG_USER_DESC;

	uint32_t buf[2];
	buf[1] = (uint32_t)user_init;

	system_call(FOS_SYSCALL_FOS_CREATE_THREAD, buf);

	return (user_desc_t)buf[0];
}


// создать бинарный семафор
user_desc_t SYS_FOS_CreateSemBinary(fos_semb_state_t init_state)
{
	if(__get_IPSR() != 0)
		return FOS_WRONG_USER_DESC;

	uint32_t buf[2];
	buf[1] = (uint32_t)init_state;

	system_call(FOS_SYSCALL_FOS_CREATE_SEMB, buf);

	return (user_desc_t)buf[0];
}


// удалить бинарный семафор
fos_ret_t SYS_FOS_DeleteSemBinary(user_desc_t semb)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)semb;

	system_call(FOS_SYSCALL_FOS_DELETE_SEMB, buf);

	return (fos_ret_t)buf[0];
}


// запустить поток с дескриптором
fos_ret_t SYS_FOS_RunDesc(user_desc_t desc)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)desc;

	system_call(FOS_SYSCALL_FOS_THREAD_RUN, buf);

	return (fos_ret_t)buf[0];
}


// завершить поток с дескрипттором
fos_ret_t SYS_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[3];
	buf[1] = (uint32_t)desc;
	buf[2] = (uint32_t)terminate_code;

	system_call(FOS_SYSCALL_FOS_THREAD_TERMINATE_D, buf);

	return (fos_ret_t)buf[0];
}


// вызвать Hard Fault
fos_ret_t SYS_FOS_HardFaultCall()
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	system_call(FOS_HARD_FAULT_CALL_ID, NULL);

	return FOS__OK;
}


// смотнитровать файловую систему
fos_ret_t SYS_File_Mount(uint8_t dev_num)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[1];
	buf[0] = (uint32_t)dev_num;

	system_call(FOS_SYSCALL_FILE_MOUNT, buf);

	return FOS__OK;
}


// размонтировать файловую систему
fos_ret_t SYS_File_Unmount(uint8_t dev_num)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[1];
	buf[0] = (uint32_t)dev_num;

	system_call(FOS_SYSCALL_FILE_UNMOUNT, buf);

	return FOS__OK;
}


// взять счётный семафор
fos_ret_t SYS_FOS_SemCntTake(user_desc_t semc, uint32_t timeout_ms)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[4];
	buf[1] = (uint32_t)semc;
	buf[2] = (uint32_t)timeout_ms;

	system_call(FOS_SYSCALL_FOS_SEMC_TAKE, buf);

	fos_ret_t ret = (fos_ret_t)buf[0];
	if(ret != FOS__OK)
		return FOS__FAIL;

	if(timeout_ms == 0)
		return FOS__OK;

	fos_ret_val_t* rv_ptr = (fos_ret_val_t*)buf[3];
	if(rv_ptr == NULL)
		return FOS__FAIL;
	if(rv_ptr->sign != FOS_NOTE_SIGN)
		return FOS__FAIL;

	if(rv_ptr->timeout_flag == FOS__DISABLE)
		return FOS__OK;
	return FOS__FAIL;
}


// дать счётный семафор
fos_ret_t SYS_FOS_SemCntGive(user_desc_t semc)
{
	if(__get_IPSR() != 0)
		return Kernel_FOS_SemCntGive(semc);

	uint32_t buf[2];
	buf[1] = (uint32_t)semc;

	system_call(FOS_SYSCALL_FOS_SEMC_GIVE, buf);

	return (fos_ret_t)buf[0];
}


// создать счётный семафор
user_desc_t SYS_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt)
{
	if(__get_IPSR() != 0)
		return FOS_WRONG_USER_DESC;

	uint32_t buf[3];
	buf[1] = (uint32_t)max_cnt;
	buf[2] = (uint32_t)init_cnt;

	system_call(FOS_SYSCALL_FOS_CREATE_SEMC, buf);

	return (user_desc_t)buf[0];
}


// удалить счётный семафор
fos_ret_t SYS_FOS_DeleteSemCnt(user_desc_t semc)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)semc;

	system_call(FOS_SYSCALL_FOS_DELETE_SEMC, buf);

	return (fos_ret_t)buf[0];
}


// create queue32
user_desc_t SYS_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode)
{
	if(__get_IPSR() != 0)
		return FOS_WRONG_USER_DESC;

	uint32_t buf[3];
	buf[1] = (uint32_t)size;
	buf[2] = (uint32_t)mode;

	system_call(FOS_SYSCALL_FOS_QUEUE_32_CREATE, buf);

	return (user_desc_t)buf[0];
}


// delete queue32
fos_ret_t SYS_FOS_DeleteQueue32(user_desc_t que)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)que;

	system_call(FOS_SYSCALL_FOS_QUEUE_32_DELETE, buf);

	return (fos_ret_t)buf[0];
}


// ask data from queue32
fos_ret_t SYS_FOS_Queue32AskData(user_desc_t que, uint32_t timeout_ms)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[4];
	buf[1] = (uint32_t)que;
	buf[2] = (uint32_t)timeout_ms;

	system_call(FOS_SYSCALL_FOS_QUEUE_32_ASK, buf);

	fos_ret_t ret = (fos_ret_t)buf[0];
	if(ret != FOS__OK)
		return FOS__FAIL;

	if(timeout_ms == 0)
		return FOS__OK;

	fos_ret_val_t* rv_ptr = (fos_ret_val_t*)buf[3];
	if(rv_ptr == NULL)
		return FOS__FAIL;
	if(rv_ptr->sign != FOS_NOTE_SIGN)
		return FOS__FAIL;

	if(rv_ptr->timeout_flag == FOS__DISABLE)
		return FOS__OK;
	return FOS__FAIL;
}


// read data from queue32
// one must ask data before read every times
fos_ret_t SYS_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[3];
	buf[2] = (uint32_t)que;

	system_call(FOS_SYSCALL_FOS_QUEUE_32_READ, buf);

	fos_ret_t ret = (fos_ret_t)buf[0];
	if(ret == FOS__OK)
		*data_ptr = buf[1];

	return ret;
}


// write data to queue32
fos_ret_t SYS_FOS_Queue32WriteData(user_desc_t que, uint32_t data)
{
	if(__get_IPSR() != 0)
		return Kernel_FOS_Queue32WriteData(que, data);

	uint32_t buf[3];
	buf[1] = (uint32_t)que;
	buf[2] = (uint32_t)data;

	system_call(FOS_SYSCALL_FOS_QUEUE_32_WRITE, buf);

	return (fos_ret_t)buf[0];
}


// is the thhread alive
fos_ret_t SYS_FOS_IsThreadAlive(user_desc_t desc)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)desc;

	system_call(FOS_SYSCALL_FOS_IS_THREAD_ALIVE, buf);

	return (fos_ret_t)buf[0];
}

/*
 * **************************************************************
 */

// усыпить текущий поток
// используется в слабом подтягивании в file_sys.c, fwriter.c
fos_ret_t SYS_FOS_Sleep(uint32_t time)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[4];         // buf[3] - тут не ипользуется но под него обязательнор нужно выделять место
	buf[1] = time;
	buf[2] = (uint32_t)FOS__DISABLE;
	// buf[3];

	system_call(FOS_SYSCALL_FOS_SLEEP, buf);

	return (fos_ret_t)buf[0];
}


// зафиксировать ошибку
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c, fos_heap.c, fos_thread.c
fos_ret_t SYS_FOS_ErrorSet(fos_err_t *err)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[1];
	buf[0] = (uint32_t)err;

	system_call(FOS_SYSCALL_FOS_ERROR_SET, buf);

	return FOS__OK;
}


// завершить текущий поток
// используется в слабом подтягивании
// used via weak callback in the fos_thread.c
fos_ret_t SYS_FOS_Terminate(int32_t terminate_code)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)terminate_code;

	system_call(FOS_SYSCALL_FOS_THREAD_TERMINATE, buf);

	return (fos_ret_t)buf[0];
}


// get thread arg pointer
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c
uint8_t* SYS_FOS_GetThreadArgPtr()
{
	if(__get_IPSR() != 0)
		return NULL;

	uint32_t buf[1];

	system_call(FOS_SYSCALL_FOS_THREAD_ARG_GET_PTR, buf);

	return (uint8_t*)buf[0];
}

// get thread arg len
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c
uint32_t SYS_FOS_GetThreadArgLen()
{
	if(__get_IPSR() != 0)
		return 0;

	uint32_t buf[1];

	system_call(FOS_SYSCALL_FOS_THREAD_ARG_GET_LEN, buf);

	return (uint32_t)buf[0];
}


// get ep_wa
// используется в слабом подтягивании
// used via weak callback in the fos_kernel.c
user_thread_ep_wa_t SYS_FOS_GetThreadEpA()
{
	if(__get_IPSR() != 0)
		return NULL;

	uint32_t buf[1];

	system_call(FOS_SYSCALL_FOS_THREAD_GET_EP_WA, buf);

	return (user_thread_ep_wa_t)buf[0];
}

/*
 * **************************************************************
 */

// get user descriptor of the current thread
user_desc_t SYS_FOS_GetCurrentThreadUd()
{
	if(__get_IPSR() != 0)
		return FOS_WRONG_USER_DESC;

	uint32_t buf[1];

	system_call(FOS_SYSCALL_FOS_GET_CURRENT_THR_UD, buf);

	return (user_desc_t)buf[0];
}


// create the mutex
user_desc_t SYS_FOS_CreateMutex(fos_mutex_type_t type, uint8_t pcp_priority)
{
	if(__get_IPSR() != 0)
		return FOS_WRONG_USER_DESC;

	uint32_t buf[2];
	buf[1] = ((uint32_t)type) | ((uint32_t)pcp_priority) << 8;

	system_call(FOS_SYSCALL_FOS_CREATE_MUTEX, buf);

	return (user_desc_t)buf[0];
}


// delete the mutex
fos_ret_t SYS_FOS_DeleteMutex(user_desc_t mutex)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)mutex;

	system_call(FOS_SYSCALL_FOS_DELETE_MUTEX, buf);

	return (fos_ret_t)buf[0];
}


// take the mutex with picked descriptor
fos_ret_t SYS_FOS_MutexTake(user_desc_t mutex, uint32_t timeout_ms)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[4];
	buf[1] = (uint32_t)mutex;
	buf[2] = (uint32_t)timeout_ms;

	system_call(FOS_SYSCALL_FOS_MUTEX_TAKE, buf);

	fos_ret_t ret = (fos_ret_t)buf[0];
	if(ret != FOS__OK)
		return FOS__FAIL;

	fos_ret_val_t* rv_ptr = (fos_ret_val_t*)buf[3];
	if(rv_ptr == NULL)
		return FOS__FAIL;
	if(rv_ptr->sign != FOS_NOTE_SIGN)
		return FOS__FAIL;

	if(rv_ptr->timeout_flag == FOS__DISABLE)
		return FOS__OK;
	return FOS__FAIL;
}


// set owner
fos_ret_t SYS_FOS_MutexSetOwner(user_desc_t mutex)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)mutex;

	system_call(FOS_SYSCALL_FOS_MUTEX_SET_OWNER, buf);

	return (fos_ret_t)buf[0];
}


// release mutex
fos_ret_t SYS_FOS_MutexGive(user_desc_t mutex)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)mutex;

	system_call(FOS_SYSCALL_FOS_MUTEX_GIVE, buf);

	return (fos_ret_t)buf[0];
}


// allocate thread local memory
void* SYS_FOS_LocalAlloc(uint32_t size_bytes)
{
	if(__get_IPSR() != 0)
		return NULL;

	uint32_t buf[2];
	buf[1] = (uint32_t)size_bytes;

	system_call(FOS_SYSCALL_FOS_LOCAL_ALLOC, buf);

	return (void*)buf[0];
}


// free thread local memory
fos_ret_t SYS_FOS_LocalFree(void* ptr)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[2];
	buf[1] = (uint32_t)ptr;

	system_call(FOS_SYSCALL_FOS_LOCAL_FREE, buf);

	return (fos_ret_t)buf[0];
}


// start the thread with the picked descriptor with argument
fos_ret_t SYS_FOS_RunDescWithArg(user_desc_t desc, uint8_t* arg_ptr, uint32_t arg_len)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[4];
	buf[1] = (uint32_t)desc;
	buf[2] = (uint32_t)arg_ptr;
	buf[3] = (uint32_t)arg_len;

	system_call(FOS_SYSCALL_FOS_THREAD_RUN_WITH_ARG, buf);

	return (fos_ret_t)buf[0];
}


// set note to thread by user descriptor
fos_ret_t SYS_FOS_SetNoteDesc(user_desc_t desc, fos_note_type_t type, uint32_t note)
{
	if(__get_IPSR() != 0)
		return Kernel_FOS_SetNoteDesc(desc, type, note);

	uint32_t buf[4];
	buf[1] = (uint32_t)desc;
	buf[2] = (uint32_t)type;
	buf[3] = (uint32_t)note;

	system_call(FOS_SYSCALL_FOS_THREAD_SET_NOTE, buf);

	return (fos_ret_t)buf[0];
}


// get thread note pointer
fos_thr_note_t* SYS_FOS_GetThreadNotePtr()
{
	if(__get_IPSR() != 0)
		return NULL;

	uint32_t buf[1];

	system_call(FOS_SYSCALL_FOS_THREAD_NOTE_GET_PTR, buf);

	return (fos_thr_note_t*)buf[0];
}


// усыпить текущий поток с ожиданием сигнала
fos_ret_t SYS_FOS_Wait(uint32_t time)
{
	if(__get_IPSR() != 0)
		return FOS__FAIL;

	uint32_t buf[4];
	buf[1] = time;
	buf[2] = (uint32_t)FOS__ENABLE;

	system_call(FOS_SYSCALL_FOS_SLEEP, buf);

	fos_ret_t ret = (fos_ret_t)buf[0];
	if(ret != FOS__OK)
		return FOS__FAIL;

	fos_thr_note_t* note_ptr = (fos_thr_note_t*)buf[3];
	if(note_ptr == NULL)
		return FOS__FAIL;
	if(note_ptr->sign != FOS_NOTE_SIGN)
		return FOS__FAIL;

	if(note_ptr->stat)
		return FOS__OK;
	return FOS__FAIL;
}


// залогировать событие
fos_ret_t SYS_FOS_LogData(char *str, fos_log_type_t type)
{
	if(__get_IPSR() != 0)
		return Kernel_FOS_LogUserData(str, (fos_log_type_t)(type | FOS_LOG_FROM_ISR_BIT));

	uint32_t buf[3];
	buf[1] = (uint32_t)str;
	buf[2] = (uint32_t)type;

	system_call(FOS_SYSCALL_FOS_LOG_USER_DATA, buf);

	return (fos_ret_t)buf[0];
}

















