/**************************************************************************//**
 * @file      fos_api.h
 * @brief     API of OS for user applications. Header file.
 * @version   V1.5.05
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

#ifndef APPLICATION_FOS_API_FOS_API_H_
#define APPLICATION_FOS_API_FOS_API_H_


#include "System/fos_system.h"
#include "FIle/file_types.h"


/*
 * 2. Threads API
 */

/*
 * 2.1 Threads management  API
 */

/*
 * 2.1.1 Create a new thread
 * Thread-safe, call from the thread or from the main loop
 * user_init - settings for created thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * Actual from v0.10
 */
user_desc_t API_FOS_CreateThread(fos_thread_user_init_t *user_init);


/*
 * 2.1.2 Create a new thread with default heap, stack settings and auto allocation
 * Thread-safe, call from the thread or from the main loop
 * name_ptr - name of the thread
 * ep       - entry point
 * priority - priority of the thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * See def heap and stack size in fos_conf.h
 * Actual from v0.10
 */
user_desc_t API_FOS_CreateThreadDef(char* name_ptr, user_thread_ep_t ep, uint8_t priority);


/*
 * 2.1.3 Create a new thread with default heap, stack settings and dynamic allocation
 * Thread-safe, call from the thread or from the main loop
 * name_ptr - name of the thread
 * ep       - entry point
 * priority - priority of the thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * See def heap and stack size in fos_conf.h
 * Actual from v0.14
 */
user_desc_t API_FOS_CreateThreadDyn(char* name_ptr, user_thread_ep_t ep, uint8_t priority);


/**
 * 2.1.4 Start the thread with descriptor
 * Thread-safe, call from the thread or from the main loop
 * desc - user descriptor of the started thread
 * Returns execution status
 *
 * Actual from v0.10
 */
fos_ret_t API_FOS_RunDesc(user_desc_t desc);


/**
 * 2.1.5 Terminate the current thread
 * Thread-safe, call from the thread intended for termination
 * terminate_code - termination code, describes the termination result (0 - successful termination, >0 - user defined any error code)
 * Returns execution status
 *
 * Actual from v0.10
 */
fos_ret_t API_FOS_Terminate(uint8_t terminate_code);


/**
 * 2.1.6 Terminate the thread with the specified descriptor
 * Thread-safe, call from the thread or from the main loop
 * desc - descriptor of the thread being terminated
 * terminate_code - termination code, describes the termination result (0 - successful termination, >0 - user defined any error code)
 * Returns execution status
 *
 * Actual from v0.10
 */
fos_ret_t API_FOS_TerminateDesc(user_desc_t desc, uint8_t terminate_code);


/*
 * 2.1.7 This method check if the thread is alive
 * Thread-safe, call from the thread or from the main loop
 * desc - descriptor of the checked thread
 * Returns thread status
 * FOS__OK - is alive
 *
 * Actual from v0.14
 */
fos_ret_t API_FOS_IsThreadAlive(user_desc_t desc);


/*
 * 2.1.8a Blocking current thread till desc thread is terminated
 * Thread-safe, call from the thread
 * desc - descriptor to the being terminated thread
 * Returns execution status
 *
 * Actual from v0.14
 * Deprecated from v1.0.7: arguments are changed
 */
//fos_ret_t API_FOS_Join(user_desc_t desc);


/*
 * 2.1.8b Blocking current thread till desc thread is terminated
 * Thread-safe, call from the thread
 * desc - descriptor to the being terminated thread
 * timeout_ms - timeout in ms
 * Returns execution status
 *
 * Actual from v1.0.7
 */
fos_ret_t API_FOS_Join(user_desc_t desc, uint32_t timeout_ms);


/**
 * 2.1.9 Start the thread with descriptor and with the argument
 * Argument transfers to the thread via its local heap (via copy)
 * Thread-safe, call from the thread or from the main loop
 * desc - user descriptor of the started thread
 * arg_ptr - pointer to argument which send to the thread via local heap
 * arg_len - length of the argument
 * Returns execution status
 *
 * Actual from v1.0.5
 */
fos_ret_t API_FOS_RunDescWithArg(user_desc_t desc, uint8_t* arg_ptr, uint32_t arg_len);


/**
 * 2.1.10 Get the argument in the thread
 * Returns pointer to the argument witch has been transferred via API_FOS_RunDescWithArg
 * Returned pointer witch points to the memory in the local thread heap
 * Thread-safe, call from the thread where argument is needed
 * Returns pointer to the argument
 * Returns NULL if there is no argument or error is occurred
 *
 * Actual from v1.0.5
 */
uint8_t* API_FOS_GetThreadArgPtr();


/**
 * 2.1.11 Get the argument length in the thread
 * Returns length of the argument witch has been transferred via API_FOS_RunDescWithArg
 * Returned length of the memory in the local thread heap witch contains the argument
 * Thread-safe, call from the thread where argument is needed
 * Returns length of the argument
 * Returns 0 if there is no argument or error is occurred
 *
 * Actual from v1.0.5
 */
uint32_t API_FOS_GetThreadArgLen();


/*
 * 2.1.12 Create periodically run thread
 * Thread-safe, call from the thread or from the main loop
 * name_ptr  - name of the thread
 * poll_func - pointer to the polling function
 * priority  - priority of the thread
 * period_ms - poll period in ms, it considers the function code time (period is not dependent from code time)
 * cnt       - poll counter (if 0, counter is disabled)
 * start_delay_ms - delay before start in ms
 * desc      - pointer to save user descriptor (if NULL is not used)
 * Returns execution status
 *
 * Actual from v1.0.5
 */
fos_ret_t API_FOS_CreateAndRunPeriodicallyThread(char* name_ptr, user_thread_ep_t poll_func, uint8_t priority, uint32_t period_ms, uint32_t cnt, uint32_t start_delay_ms, user_desc_t *desc);


/*
 * 2.1.13 Create a new thread with default heap, stack settings, auto allocation and arguments
 * Thread-safe, call from the thread or from the main loop
 * name_ptr - name of the thread
 * ep_wa    - entry point at the function with arguments
 * priority - priority of the thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * See def heap and stack size in fos_conf.h
 * Actual from v1.0.6
 */
user_desc_t API_FOS_CreateThreadDefA(char* name_ptr, user_thread_ep_wa_t ep_wa, uint8_t priority);


/*
 * 2.1.14 Create a new thread with default heap, stack settings, dynamic allocation and arguments
 * Thread-safe, call from the thread or from the main loop
 * name_ptr - name of the thread
 * ep_wa    - entry point at the function with arguments
 * priority - priority of the thread
 * Returns the user descriptor of created thread or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * See def heap and stack size in fos_conf.h
 * Actual from v1.0.6
 */
user_desc_t API_FOS_CreateThreadDynA(char* name_ptr, user_thread_ep_wa_t ep_wa, uint8_t priority);


/*
 * 2.2 Time management API
 */

/*
 * 2.2.1a Yield to another process
 * Thread-safe, call from the process that yields to another one
 *
 * Actual from v0.10
 * Deprecated from v1.0.7: ret code is added
 */
//void API_FOS_Yield();


/*
 * 2.2.1b Yield to another process
 * Thread-safe, call from the process that yields to another one
 * Returns execution status
 *
 * Actual from v1.0.7
 */
fos_ret_t API_FOS_Yield();


/*
 * 2.2.2 Send current process to sleep
 * Thread-safe, call from the thread that is sent to sleep
 * time - sleep timeout in milliseconds, if 'FOS_INF_TIME' - infinite, no timeout
 * Returns execution status
 *
 * Actual from v0.10
 */
fos_ret_t API_FOS_Sleep(uint32_t time);


/*
 * 2.3 Memory management API
 */

/*
 * 2.3.1 Allocate memory in the local thread heap
 * Thread-safe, call from the thread where it is needed to allocate memory
 * size_bytes - memory length to allocate
 * Returns pointer to allocated memory
 * Returns NULL if an error is occurred
 *
 * Actual from v1.0.5
 */
void* API_FOS_Alloc(uint32_t size_bytes);


/*
 * 2.3.2 Free memory in the local thread heap
 * Thread-safe, call from the thread where it is needed to release memory
 * ptr - pointer to the memory to release
 * Returns execution status
 *
 * Actual from v1.0.5
 */
fos_ret_t API_FOS_Free(void* ptr);

/*
 * 2.4 Thread notification API
 */

/*
 * 2.4.1 Sent the note to the thread by user descriptor
 * Thread-safe, call from the thread, from the main loop and from the ISR
 * desc - user descriptor of the thread
 * type - note type
 * note - 32 flags in uint32_t to send to the thread
 * Returns execution status
 *
 * Actual from v1.0.5
 * Function has been changed at v1.0.7: the possibility calling from ISR is added
 */
fos_ret_t API_FOS_SentNoteToThread(user_desc_t desc, fos_note_type_t type, uint32_t note);


/*
 * 2.4.2 Sent the note to the thread by user descriptor from ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * desc - user descriptor of the thread
 * type - note type
 * note - 32 flags in uint32_t to send to the thread
 * Returns execution status
 *
 * Actual from v1.0.5
 * Deprecated from v1.0.7: function is replaced by API_FOS_SentNoteToThread
 */
//fos_ret_t API_FOS_SentNoteToThreadFromISR(user_desc_t desc, fos_note_type_t type, uint32_t note);


/*
 * 2.4.3 Get thread note pointer
 * Thread-safe, call from the thread where it is needed to get the note pointer
 * Returns note pointer that contains the flag massages
 * Each thread has one mailbox for the note
 * To get the pointer one must call this function in the corresponding thread
 * Returns NULL if an error is occurred
 *
 * Actual from v1.0.5
 *
 */
fos_thr_note_t* API_FOS_GetThreadNotePtr();


/*
 * 2.4.4 To proceed system note place this function in while loop conditions of the thread
 * Thread-safe, call from the thread where it is needed to proceed system note
 * p - pointer to the thread note
 * Returns 1 if there is no quit system message, otherwise 0
 *
 * Actual from v1.0.5
 */
uint32_t API_FOS_ProcSysNote(fos_thr_note_t* p);


/*
 * 2.4.5 Get user flag note as uint32_t
 * Thread-safe, call from the thread where it is needed to get user note
 * Returns note as uint32_t
 * This function automatically reset all user flags
 * p - pointer to the thread note
 * Returns flag note
 *
 * Actual from v1.0.5
 */
uint32_t API_FOS_GetAndResetUserNote(fos_thr_note_t* p);


/*
 * 2.4.6 Send current process to wait a note
 * time - wait timeout in milliseconds, if 'FOS_INF_TIME' - infinite, no timeout
 * Returns execution status
 *
 * Actual from v1.0.6
 */
fos_ret_t API_FOS_Wait(uint32_t time);


/**
 * 2.4.7 Get wait status
 * Thread-safe, call from the thread where it is needed to know the note status
 * The note status clears automatically when API_FOS_Wait is called
 * The note status is set if there is any note (status yells the type of note)
 * In the case of timeout the status is 0
 * So it is possible to differ timeout while API_FOS_Wait
 * Return note status
 *
 * Actual from v1.0.6
 */
uint8_t API_FOS_GetWaitingStatus(fos_thr_note_t* p);


/*
 * 3. Sync API
 */

/*
 * 3.1 Binary semaphores API
 */

/*
 * 3.1.1 Create a binary semaphore
 * Thread-safe, call from the thread or from the main loop
 * init_state - initial state of the semaphore
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * Actual from v0.10
 */
user_desc_t API_FOS_CreateSemBinary(fos_semb_state_t init_state);


/*
 * 3.1.2 Set binary semaphore timeout in ms
 * Thread-safe, call from the thread or from the main loop
 * semb - a binary semaphore
 * timeout_ms - timeout in ms, to disable set into 0 or FOS_INF_TIME
 * Returns execution status
 *
 * Actual from v1.0.2
 * Deprecated from v1.0.7: timeout of the semaphore is not global now
 */
//fos_ret_t API_FOS_SemBinarySetTimeout(user_desc_t semb, uint32_t timeout_ms);


/*
 * 3.1.3 Delete a binary semaphore
 * Thread-safe, call from the thread or from the main loop
 * semb - a binary semaphore to be deleted
 * Returns execution status
 *
 * Actual from v0.14
 */
fos_ret_t API_FOS_DeleteSemBinary(user_desc_t semb);


/*
 * 3.1.4a Acquire binary semaphore
 * Thread-safe, call from the thread that is acquiring semaphore
 * semb - binary semaphore user descriptor
 * Returns execution status
 *
 * Actual from v0.10
 * Deprecated from v1.0.7: arguments are changed
 */
//fos_ret_t API_FOS_SemBinaryTake(user_desc_t semb);


/*
 * 3.1.4b Acquire binary semaphore
 * Thread-safe, call from the thread, from the main loop and the ISR (main loop and ISR only in no blocking mode)
 * semb - binary semaphore user descriptor
 * timeout_ms - timeout in milliseconds, if 'FOS_INF_TIME' - infinite, no timeout, if '0' - no blocking mode
 * Returns execution status
 *
 * Actual from v1.0.7
 */
fos_ret_t API_FOS_SemBinaryTake(user_desc_t semb, uint32_t timeout_ms);


/*
 * 3.1.5 Release binary semaphore
 * Thread-safe, call from the thread, from the main loop and the ISR
 * semb - binary semaphore user descriptor
 * Returns execution status
 *
 * Actual from v0.10
 * Function has been changed at v1.0.7: the possibility calling from ISR is added
 */
fos_ret_t API_FOS_SemBinaryGive(user_desc_t semb);


/*
 * 3.1.6 Release binary semaphore for ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * semb - binary semaphore user descriptor
 * Returns execution status
 *
 * Actual from v1.0.2
 * Deprecated from v1.0.7: replaced by API_FOS_SemBinaryGive
 */
//fos_ret_t API_FOS_SemBinaryGiveFromISR(user_desc_t semb);

/*
 * 3.2 Counting semaphores API
 */

/*
 * 3.2.1 Create a counting semaphore
 * Thread-safe, call from the thread or from the main loop
 * max_cnt  - max count
 * init_cnt - initial state of the semaphore (if init_cnt > max_cnt Then init_cnt = max_cnt)
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * Actual from v1.0.2
 */
user_desc_t API_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt);


/*
 * 3.2.2 Set counting semaphore timeout in ms
 * Thread-safe, call from the thread or from the main loop
 * semc - a semaphore
 * timeout_ms - timeout in ms, to disable set into 0 or FOS_INF_TIME
 * Returns execution status
 *
 * Actual from v1.0.2
 * Deprecated from v1.0.7: timeout of the semaphore is not global now
 */
//fos_ret_t API_FOS_SemCntSetTimeout(user_desc_t semc, uint32_t timeout_ms);


/*
 * 3.2.3 Delete a counting semaphore
 * Thread-safe, call from the thread or from the main loop
 * semc - a semaphore to be deleted
 * Returns execution status
 *
 * Actual from v1.0.2
 */
fos_ret_t API_FOS_DeleteSemCnt(user_desc_t semc);


/*
 * 3.2.4a Acquire counting semaphore
 * Thread-safe, call from the thread that is acquiring semaphore
 * semc - binary semaphore user descriptor
 * Returns execution status
 *
 * Actual from v1.0.2
 * Deprecated from v1.0.7: arguments are changed
 */
//fos_ret_t API_FOS_SemCntTake(user_desc_t semc);


/*
 * 3.2.4b Acquire counting semaphore
 * Thread-safe, call from the thread, from the main loop and the ISR (main loop and ISR only in no blocking mode)
 * semc - binary semaphore user descriptor
 * timeout_ms - timeout in milliseconds, if 'FOS_INF_TIME' - infinite, no timeout, if '0' - no blocking mode
 * Returns execution status
 *
 * Actual from v1.0.7
 */
fos_ret_t API_FOS_SemCntTake(user_desc_t semc, uint32_t timeout_ms);


/*
 * 3.2.5 Release counting semaphore
 * Thread-safe, call from the thread, from the main loop and the ISR
 * semc - binary semaphore user descriptor
 * Returns execution status
 *
 * Actual from v1.0.2
 * Function has been changed at v1.0.7: the possibility calling from ISR is added
 */
fos_ret_t API_FOS_SemCntGive(user_desc_t semc);


/*
 * 3.2.6 Release binary semaphore for ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * semc - binary semaphore user descriptor
 * Returns execution status
 *
 * Actual from v1.0.2
 * Deprecated from v1.0.7: replaced by API_FOS_SemCntGive
 */
//fos_ret_t API_FOS_SemCntGiveFromISR(user_desc_t semc);

/*
 * 3.3 Mutex API
 */

/*
 * 3.3.1a Create a simple mutex
 * Thread-safe, call from the thread or from the main loop
 * timeout_ms -  - timeout in ms, to disable set into 0 or FOS_INF_TIME
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * Actual from v1.0.5
 * Deprecated from v1.0.7: arguments are changed
 */
//user_desc_t API_FOS_CreateMutex(uint32_t timeout_ms);


/*
 * 3.3.1b Create a simple mutex
 * Thread-safe, call from the thread or from the main loop
 * attr - attributes (reserved and should be NULL)
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * Actual from v1.0.7
 */
user_desc_t API_FOS_CreateMutex(fos_mutex_attr_t *attr);


/*
 * 3.3.2 Delete a mutex
 * Thread-safe, call from the thread or from the main loop
 * mutex - the mutex to be deleted
 * Returns execution status
 *
 * Actual from v1.0.5
 */
fos_ret_t API_FOS_DeleteMutex(user_desc_t mutex);


/*
 * 3.3.3a Acquire mutex
 * Thread-safe, call from the thread that is acquiring mutex
 * mutex - user descriptor of the mutex
 * Returns execution status
 *
 * Actual from v1.0.5
 * Deprecated from v1.0.7: arguments are changed
 */
//fos_ret_t API_FOS_MutexTake(user_desc_t mutex);


/*
 * 3.3.3b Acquire mutex
 * Thread-safe, call from the thread that is acquiring mutex
 * mutex - user descriptor of the mutex
 * timeout_ms - timeout in milliseconds, if 'FOS_INF_TIME' - infinite, no timeout, if '0' is forbidden
 * Returns execution status
 *
 * Actual from v1.0.7
 */
fos_ret_t API_FOS_MutexTake(user_desc_t mutex, uint32_t timeout_ms);


/*
 * 3.3.4 Release mutex
 * Thread-safe, call from the thread or from the main loop
 * mutex - user descriptor of the mutex
 * Returns execution status
 *
 * Actual from v1.0.5
 */
fos_ret_t API_FOS_MutexGive(user_desc_t mutex);

/*
 * 3.4 Critical sections
 */

/*
 * 3.4.1 Enter the critical section
 * Saves and return current interrupt state and then disable it
 * Use with careful
 * One must use the critical section only for short time operation. Not longer than 10 us.
 * For long time operation use the mutex.
 * Do not use any API function except ISR API function into the critical section.
 * It is allowed to use critical section recurrently
 * It is allowed to use critical section into ISR
 * Enters count must be equal leave counts!!!
 * Return interrupt state
 *
 * Actual from v1.0.5
 */
uint32_t API_FOS_EnterCritical();

/*
 * 3.4.2 Leave the critical section
 * Restore the interrupt state
 * x - state to restore
 *
 * Actual from v1.0.5
 */
void API_FOS_LeaveCritical(uint32_t x);

/*
 * 4 Data transfer API
 */

/*
 * 4.1 Queue32
 */

/*
 * 4.1.1a Create a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * size  - max data count in uint32_t pithes
 * mode  - queue mode
 * timeout_ms - timeout in ms, to disable set into 0 or FOS_INF_TIME
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * Actual from v1.0.2
 * Deprecated from v1.0.7: arguments are changed
 */
//user_desc_t API_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode, uint32_t timeout_ms);


/*
 * 4.1.1b Create a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * size  - max data count in uint32_t pithes
 * attr  - attributes pointer (if NULL then default parameters are used)
 * Returns the user descriptor of created object or 'FOS_WRONG_USER_DESC' in case of an error
 *
 * Actual from v1.0.7
 */
user_desc_t API_FOS_CreateQueue32(uint16_t size, fos_queue_attr_t *attr);


/*
 * 4.1.2 Delete a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * que - a queue32 to be deleted
 * Returns execution status
 *
 * Actual from v1.0.2
 */
fos_ret_t API_FOS_DeleteQueue32(user_desc_t que);


/*
 * 4.1.3a Read data from a queue32 for uint32_t data
 * Thread-safe, call from the thread or from the main loop
 * que - a queue32
 * data_ptr - pointer for read data
 * blocking_mode_sw - poll or block switch, blocking mode works only in the thread and queue mode is FOS_QUEUE_MODE__POLL_AND_BLOCK
 * Returns execution status
 *
 * Actual from v1.0.2
 * Deprecated from v1.0.7: arguments are changed
 */
//fos_ret_t API_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr, fos_queue_sw_t blocking_mode_sw);


/*
 * 4.1.3b Read data from a queue32 for uint32_t data
 * Thread-safe, call from the thread, from the main loop and the ISR (main loop and ISR only in no blocking mode)
 * que - a queue32
 * data_ptr - pointer for read data
 * timeout_ms - timeout im ms. If equal '0' then no blocking mode, if equal 'FOS_INF_TIME' then no timeout mode
 *
 * Actual from v1.0.7
 */
fos_ret_t API_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr, uint32_t timeout_ms);


/*
 * 4.1.4 Write data to a queue32 for uint32_t data
 * Thread-safe, call from the thread, from the main loop and the ISR
 * que - a queue32
 * data - data do write in the queue
 * Returns execution status
 *
 * Actual from v1.0.2
 * Function has been changed at v1.0.7: the possibility calling from ISR is added
 */
fos_ret_t API_FOS_Queue32WriteData(user_desc_t que, uint32_t data);


/*
 * 4.1.5 Write data to a queue32 for uint32_t data from ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * que - a queue32
 * data - data do write in the queue
 * Returns execution status
 * FOS__FAIL - if no data is written to the queue
 *
 * Actual from v1.0.2: replaced by API_FOS_Queue32WriteData
 */
//fos_ret_t API_FOS_Queue32WriteDataFromISR(user_desc_t que, uint32_t data);


/*
 * 5 Log API
 */

/**
 * 5.1 Detect an error
 * Thread-safe, call from the thread or from the main loop
 * err_code - error code
 * err_string - error descriptive string
 *
 * Actual from v0.10
 */
void API_FOS_ErrorSet(uint32_t err_code, char* err_string);


/**
 * 5.2 Log data
 * Thread-safe, call from the thread, from the main loop or ISR
 * str  - string to log
 * type - type log
 * Returns execution status
 *
 * Actual from v1.0.6
 * Function has been changed at v1.0.7: the possibility calling from ISR is added
 */
fos_ret_t API_FOS_LogData(char *str, fos_log_type_t type);


/**
 * 5.3 Log data from ISR
 * Call from interrupts only (call outside the interrupt has some limitations)
 * str  - string to log
 * type - type log
 * Returns execution status
 *
 * Actual from v1.0.6
 * Deprecated from v1.0.7: replaced by API_FOS_LogData
 */
//fos_ret_t API_FOS_LogDataFromISR(char *str, fos_log_type_t type);


/**
 * 5.4 Get the file writer pointer
 * Returns the file writer pointer in the thread that serves SD card and FS
 *
 * Actual from v1.0.6
 */
fwriter_t* API_FOS_GetLogWriterPtr();


/**
 * 5.5 Get FOS version
 *
 * Actual from v1.0.6
 */
char* API_FOS_GetVersion();

/*
 * 6 File system API
 */

/*
 * 6.1 General
 */

/*
 * 6.1.1 Mount the file system
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * dev_num - mounted device number
 * Returns execution status
 *
 * Actual from v0.10
 */
file_err_t API_File_Mount(uint8_t dev_num);


/*
 * 6.1.2 Unmount the file system
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts (it can lead to unpredictable behavior)
 * dev_num - unmounted device number
 * Returns execution status
 *
 * Actual from v0.10
 */
file_err_t API_File_Unmount(uint8_t dev_num);

/*
 * 6.2 File writer
 */

/*
 * 6.2.1 Create the writer object
 * Thread-safe, call from the thread or from the main loop
 * Do not call from interrupts
 * write_buf_len - written buffer size
 * Returns the pointer to the writer object
 *
 * write_buf_len must be equel or less then FOS_FILEWR_MAX_BUF_LEN in fos_conf.h
 * Actual from v0.10
 */
fwriter_t* API_File_CreateFWriter(uint16_t write_buf_len);





#endif /* APPLICATION_FOS_API_FOS_API_H_ */

























