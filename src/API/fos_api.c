/**************************************************************************//**
 * @file      fos_api.c
 * @brief     API of OS for user applications. Source file.
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


#include "API/fos_api.h"


// prototype of writer object creation
// kernel function is used, not indicated in the header file
__weak fwriter_t* USER_CreateFWriter(uint16_t write_buf_len)
{
	return NULL;
}


/*
 * Yield to another process
 * Thread-safe, call from the process that yields to another one
 * Do not call from outside the threads
 */
void API_FOS_Yield()
{
	SYS_FOS_Yield();
}


/*
 * Send current process to sleep
 * Thread-safe, call from the thread that is sent to sleep
 * Do not call from outside the threads
 * time - sleep timeout in milliseconds, if 'FOS_INF_TIME' - infinite, no timeout
 * Returns execution status
 */
fos_ret_t API_FOS_Sleep(uint32_t time)
{
	return SYS_FOS_Sleep(time);
}


/*
 * Acquire binary semaphore
 * Thread-safe, call from the thread that is acquiring semaphore
 * Do not call from outside the threads
 * semb - binary semaphore user descriptor
 * Returns execution status
 */
fos_ret_t API_FOS_SemBinaryTake(user_desc_t semb)
{
	return SYS_FOS_SemBinaryTake(semb);
}


/*
 * Release binary semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * semb - binary semaphore user descriptor
 * Returns execution status
 */
fos_ret_t API_FOS_SemBinaryGive(user_desc_t semb)
{
	return SYS_FOS_SemBinaryGive(semb);
}


/*
 * Create a new thread
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * user_init - settings for created thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 */
user_desc_t API_FOS_CreateThread(fos_thread_user_init_t *user_init)
{
	return SYS_FOS_CreateThread(user_init);
}


/*
 * Create a new thread with default heap and stack settings
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * name_ptr - name of the thread
 * ep       - entry point
 * priotity - priority of the thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * See def heap and stack size in fos_conf.h
 */
user_desc_t API_FOS_CreateThreadDef(char* name_ptr, user_thread_ep_t ep, uint8_t priotity)
{
	fos_thread_user_init_t user_init = {0};
	user_init.user_thread_ep = ep;
	user_init.priotity   = priotity;		                       // LOW
	user_init.stack_size = FOS_DEF_THR_STACK_SIZE;
	user_init.heap_size  = FOS_DEF_THR_HEAP_SIZE;
	user_init.name_ptr   = name_ptr;
	return API_FOS_CreateThread(&user_init);
}


/*
 * Create a binaty semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * init_state - initial state of the semaphore
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 */
user_desc_t API_FOS_CreateSemBinary(fos_semb_state_t init_state)
{
	return SYS_FOS_CreateSemBinary(init_state);
}


/**
 * Start the thread with descriptor
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * desc - user descriptor of the started thread
 * Returns execution status
 */
fos_ret_t API_FOS_RunDesc(user_desc_t desc)
{
	return SYS_FOS_RunDesc(desc);
}


/**
 * Terminate the current thread
 * Thread-safe, call from the thread intended for termination
 * Do not call from outside the threads
 * terminate_code - termination code, describes the termination result (0 - successful termination, >0 - user defined any error code)
 * Returns execution status
 */
fos_ret_t API_FOS_Terminate(uint8_t terminate_code)
{
	return SYS_FOS_Terminate((int32_t)terminate_code);
}


/**
 * Terminate the thread with the specified descriptor
 * Thread-safe, call from the thread intended for termination
 * Do not call from interrupts
 * desc - descriptor of the thread being terminated
 * terminate_code - termination code, describes the termination result (0 - successful termination, >0 - user defined any error code)
 * Returns execution status
 */
fos_ret_t API_FOS_TerminateDesc(user_desc_t desc, uint8_t terminate_code)
{
	return SYS_FOS_TerminateDesc(desc, -terminate_code);
}


/**
 * Detect an error
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * err_code - error code
 * err_string - error descriptive string
 */
void API_FOS_ErrorSet(uint32_t err_code, char* err_string)
{
	fos_err_t err = {0};
	err.err_code = err_code;
	err.ext_str_ptr = err_string;
	SYS_FOS_ErrorSet(&err);
}


/*
 * Mount the file system
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * dev_num - mounted device number
 * Returns execution status
 */
file_err_t API_File_Mount(uint8_t dev_num)
{
	SYS_File_Mount(dev_num);
	return File_WaitingMount(dev_num, FOS_FSDEV_TIMEOUT_MS);
}


/*
 * Unmount the file system
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * dev_num - unmounted device number
 * Returns execution status
 */
file_err_t API_File_Unmount(uint8_t dev_num)
{
	SYS_File_Unmount(dev_num);
	return FILE_ERR__NO;
}


/*
 * Create the writer object
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * write_buf_len - written buffer size
 * Returns the pointer to the writer object
 *
 * write_buf_len must be equel or less then FOS_FILEWR_MAX_BUF_LEN in fos_conf.h
 */
fwriter_t* API_File_CreateFWriter(uint16_t write_buf_len)
{
	fwriter_t* p = NULL;
	uint32_t s;

	ENTER_CRITICAL(s);
	p = USER_CreateFWriter(write_buf_len);
	LEAVE_CRITICAL(s);

	return p;
}








