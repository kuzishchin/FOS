/**************************************************************************//**
 * @file      fos_api.h
 * @brief     API of OS for user applications. Header file.
 * @version   V1.3.03
 * @date      05.02.2026
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

#ifndef APPLICATION_FOS_API_FOS_API_H_
#define APPLICATION_FOS_API_FOS_API_H_


#include "System/fos_system.h"
#include "FIle/file_types.h"

/*
 * Yield to another process
 * Thread-safe, call from the process that yields to another one
 * Do not call from outside the threads (it has no effect)
 */
void API_FOS_Yield();


/*
 * Send current process to sleep
 * Thread-safe, call from the thread that is sent to sleep
 * Do not call from outside the threads (calling outside the thread cause blocking last active thread)
 * time - sleep timeout in milliseconds, if 'FOS_INF_TIME' - infinite, no timeout
 * Returns execution status
 * Always returns FOS__OK under normal operation
 */
fos_ret_t API_FOS_Sleep(uint32_t time);


/*
 * Acquire binary semaphore
 * Thread-safe, call from the thread that is acquiring semaphore
 * Do not call from outside the threads (calling outside the thread cause acquiring semaphore by last active thread and it returns unpredictable result)
 * semb - binary semaphore user descriptor
 * Returns execution status
 * FOS__FAIL - if semb is wrong or timeout is occurred
 */
fos_ret_t API_FOS_SemBinaryTake(user_desc_t semb);


/*
 * Release binary semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * semb - binary semaphore user descriptor
 * Returns execution status
 * FOS__FAIL - if semb is wrong
 */
fos_ret_t API_FOS_SemBinaryGive(user_desc_t semb);


/*
 * Release binary semaphore for ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * semb - binary semaphore user descriptor
 * Returns execution status
 * FOS__FAIL - if semb is wrong
 */
fos_ret_t API_FOS_SemBinaryGiveFromISR(user_desc_t semb);


/*
 * Create a new thread
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * user_init - settings for created thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 */
user_desc_t API_FOS_CreateThread(fos_thread_user_init_t *user_init);


/*
 * Create a new thread with default heap, stack settings and auto allocation
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * name_ptr - name of the thread
 * ep       - entry point
 * priority - priority of the thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * See def heap and stack size in fos_conf.h
 */
user_desc_t API_FOS_CreateThreadDef(char* name_ptr, user_thread_ep_t ep, uint8_t priority);


/*
 * Create a new thread with default heap, stack settings and dynamic allocation
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * name_ptr - name of the thread
 * ep       - entry point
 * priority - priority of the thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * See def heap and stack size in fos_conf.h
 */
user_desc_t API_FOS_CreateThreadDyn(char* name_ptr, user_thread_ep_t ep, uint8_t priority);


/*
 * Create a binary semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * init_state - initial state of the semaphore
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 */
user_desc_t API_FOS_CreateSemBinary(fos_semb_state_t init_state);


/*
 * Delete a binary semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * semb - a binary semaphore to be deleted
 * Returns execution status
 * FOS__FAIL - if semb is wrong or error while deleting is occurred
 */
fos_ret_t API_FOS_DeleteSemBinary(user_desc_t semb);


/**
 * Start the thread with descriptor
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * desc - user descriptor of the started thread
 * Returns execution status
 * FOS__FAIL - if desc is wrong or the thread is not ready to run
 */
fos_ret_t API_FOS_RunDesc(user_desc_t desc);


/**
 * Terminate the current thread
 * Thread-safe, call from the thread intended for termination
 * Do not call from outside the threads (it terminates last active thread)
 * terminate_code - termination code, describes the termination result (0 - successful termination, >0 - user defined any error code)
 * Returns execution status
 * Always returns FOS__OK under normal operation
 */
fos_ret_t API_FOS_Terminate(uint8_t terminate_code);


/**
 * Terminate the thread with the specified descriptor
 * Thread-safe, call from the thread intended for termination
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * desc - descriptor of the thread being terminated
 * terminate_code - termination code, describes the termination result (0 - successful termination, >0 - user defined any error code)
 * Returns execution status
 * FOS__FAIL - if desc is wrong or the thread is not redy to terminate
 */
fos_ret_t API_FOS_TerminateDesc(user_desc_t desc, uint8_t terminate_code);


/*
 * This method check if the thread is alive
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * desc - descriptor of the checked thread
 * Returns thread status
 * FOS__OK - is alive
 */
fos_ret_t API_FOS_IsThreadAlive(user_desc_t desc);


/*
 * Blocking current thread till desc thread is terminated
 * Thread-safe, call from the thread
 * Do not call from outside the threads (call outside the thread cause joining to last active thread)
 * desc - descriptor to the being terminated thread
 * Returns execution status
 * FOS__FAIL - if desc is wrong
 */
fos_ret_t API_FOS_Join(user_desc_t desc);


/**
 * Detect an error
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * err_code - error code
 * err_string - error descriptive string
 */
void API_FOS_ErrorSet(uint32_t err_code, char* err_string);


/*
 * Mount the file system
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * dev_num - mounted device number
 * Returns execution status
 */
file_err_t API_File_Mount(uint8_t dev_num);


/*
 * Unmount the file system
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * dev_num - unmounted device number
 * Returns execution status
 */
file_err_t API_File_Unmount(uint8_t dev_num);


/*
 * Create the writer object
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * write_buf_len - written buffer size
 * Returns the pointer to the writer object
 *
 * write_buf_len must be equel or less then FOS_FILEWR_MAX_BUF_LEN in fos_conf.h
 */
fwriter_t* API_File_CreateFWriter(uint16_t write_buf_len);


/*
 * Set binary semaphore timeout in ms
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * semb - a binaty semaphore
 * timeout_ms - timeout in ms, to disable set into 0 or FOS_INF_TIME
 * Returns execution status
 * FOS__FAIL - if semb is wrong
 */
fos_ret_t API_FOS_SemBinarySetTimeout(user_desc_t semb, uint32_t timeout_ms);


/*
 * Acquire counting semaphore
 * Thread-safe, call from the thread that is acquiring semaphore
 * Do not call from outside the threads  (call outside the thread cause acquiring semaphore by last active thread and it returns unpredictable result)
 * semc - binary semaphore user descriptor
 * Returns execution status
 * FOS__FAIL - if semc is wrong or timeout is occurred
 */
fos_ret_t API_FOS_SemCntTake(user_desc_t semc);


/*
 * Release counting semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * semc - binary semaphore user descriptor
 * Returns execution status
 * FOS__FAIL - if semc is wrong
 */
fos_ret_t API_FOS_SemCntGive(user_desc_t semc);


/*
 * Release binary semaphore for ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * semc - binary semaphore user descriptor
 * Returns execution status
 * FOS__FAIL - if semc is wrong
 */
fos_ret_t API_FOS_SemCntGiveFromISR(user_desc_t semc);


/*
 * Create a counting semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * max_cnt  - max count
 * init_cnt - initial state of the semaphore (if init_cnt > max_cnt Then init_cnt = max_cnt)
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 */
user_desc_t API_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt);


/*
 * Delete a counting semaphore
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * semc - a semaphore to be deleted
 * Returns execution status
 * FOS__FAIL - if semc is wrong or error while deleting is occurred
 */
fos_ret_t API_FOS_DeleteSemCnt(user_desc_t semc);


/*
 * Set counting semaphore timeout in ms
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * semc - a semaphore
 * timeout_ms - timeout in ms, to disable set into 0 or FOS_INF_TIME
 * Returns execution status
 * FOS__FAIL - if semc is wrong
 */
fos_ret_t API_FOS_SemCntSetTimeout(user_desc_t semc, uint32_t timeout_ms);


/*
 * Create a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * size  - max data count in uint32_t pithes
 * mode  - queue mode
 * timeout_ms - timeout in ms, to disable set into 0 or FOS_INF_TIME
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 */
user_desc_t API_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode, uint32_t timeout_ms);


/*
 * Delete a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * que - a queue32 to be deleted
 * Returns execution status
 * FOS__FAIL - if que is wrong or error while deleting is occurred
 */
fos_ret_t API_FOS_DeleteQueue32(user_desc_t que);


/*
 * Read data from a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * que - a queue32
 * data_ptr - pointer for read data
 * blocking_mode_sw - poll or block switch, blocking mode works only in the thread and queue mode is FOS_QUEUE_MODE__POLL_AND_BLOCK
 * Returns execution status
 * FOS__FAIL - if no data is read from the queue
 */
fos_ret_t API_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr, fos_queue_sw_t blocking_mode_sw);


/*
 * Write data to a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * que - a queue32
 * data - data do write in the queue
 * Returns execution status
 * FOS__FAIL - if no data is written to the queue
 */
fos_ret_t API_FOS_Queue32WriteData(user_desc_t que, uint32_t data);


/*
 * Write data to a queue32 for uint32_t data from ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * que - a queue32
 * data - data do write in the queue
 * Returns execution status
 * FOS__FAIL - if no data is written to the queue
 */
fos_ret_t API_FOS_Queue32WriteDataFromISR(user_desc_t que, uint32_t data);


#endif /* APPLICATION_FOS_API_FOS_API_H_ */





















