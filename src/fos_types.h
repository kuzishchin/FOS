/**************************************************************************//**
 * @file      fos_types.h
 * @brief     OS types declarations. Header file.
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

#ifndef APPLICATION_FOS_FOS_TYPES_H_
#define APPLICATION_FOS_FOS_TYPES_H_


#include "fos_conf.h"
#include "Platform/compiler_macros.h"
#include <stdint.h>
#include <stddef.h>


#define FOS_SUSPEND_BLOCKED_ID 250           // identifier of suspended and blocked tasks
#define FOS_EMPTY_ID           255           // identifier of empty (non present) task
#define FOS_INF_TIME           0xFFFFFFFF    // infinite time
#define FOS_USER_LOCK_MASK     0xFFFF        // user defined mask for blocking
#define FOS_LOCK_OBJ_FLAG      0x10000       // blocking flag for blocker object
#define FOS_WRONG_THREAD_ID    0xFF          // identifier of a wrong thread descriptor
#define FOS_WRONG_SEM_BIN_ID   0xFF          // identifier of a wrong binary semphore descriptor
#define FOS_WRONG_FWRITER_ID   0xFF          // identifier of a wrong writer object descriptor
#define FOS_WRONG_USER_DESC    0             // wrong user defined descriptor


// on-off switch
typedef enum
{
	FOS__DISABLE = 0,
	FOS__ENABLE,

} fos_sw_t;


// return codes
typedef enum
{
	FOS__OK = 0,
	FOS__FAIL,

} fos_ret_t;


// semaphore states
typedef enum
{
	FOS_SEMB_STATE__LOCK = 0,       // locked
	FOS_SEMB_STATE__UNLOCK,         // unlocked

} fos_semb_state_t;


// OS work mode
typedef enum
{
	FOS__KERNEL_WORK_MODE = 0, // kernel mode
	FOS__USER_WORK_MODE,       // user mode

} fos_work_mode_t;


// process states
typedef enum
{
	FOS__THREAD_SUSPEND = 0,   // sleeping thread
	FOS__THREAD_BLOCKED,       // blocked thread
	FOS__THREAD_READY,         // launch ready thread
	FOS__THREAD_RUNNING,       // running thread

} fos_thread_state_t;


// режим работы потока
typedef enum
{
	FOS__THREAD_NO_INIT = 0,   // not initialized
	FOS__THREAD_INIT,          // initialized
	FOS__THREAD_READY_TO_RUN,  // thread is ready for launch
	FOS__THREAD_RUN,           // thread is running
	FOS__TERMINATED,           // thread is terminated
	FOS__THREAD_DEINIT,        // thread is deinitialized

} fos_thread_mode_t;


// user descriptor
typedef uint32_t user_desc_t;


// major global variables
typedef struct
{
	volatile uint32_t kernel_sp;         // pointer to kernel stack
	volatile uint32_t user_sp;           // pointer to process stack

	volatile fos_work_mode_t mode;       // OS work mode

	volatile fos_sw_t swithed_by_tim;    // context switch flag
	volatile uint32_t thr_dt_us;         // time spent for the running process, microseconds

} fos_mgv_t;


// description of the constant thread settings
typedef struct
{
	uint32_t base_sp;          // stack starting address
	uint32_t ep;               // entry point address
	uint32_t stack_size;       // stack size

} fos_thread_cset_t;


// thread settings
typedef struct
{
	volatile uint8_t priotity;          // thread priority (0 - the highest, 1 - lower than 0, etc.)

} fos_thread_set_t;


// thread initialization
typedef struct
{
	char *name_ptr;            // pointer to the name of the thread
	fos_thread_cset_t cset;    // constant settings
	fos_thread_set_t  set;     // initial settings

} fos_thread_init_t;


typedef void (*user_thread_ep_t)();       // entry point to user thread


// user defined thread initialization
typedef struct
{
	char            *name_ptr;         // pointer to the name of the thread
	user_thread_ep_t user_thread_ep;   // thread entry point
	uint32_t         stack_size;       // thread stack size
	uint32_t         heap_size;        // thread heap size
	uint8_t          priotity;         // thread priority (0 - the highest, 1 - lower than 0, etc.)

} fos_thread_user_init_t;



typedef void (*svcall_t)(void*);  // system call function prototype


// blocker object
typedef struct
{
	volatile uint8_t first_lock_thr;                       // index of the element of 'lock_thr_is_list' of the first blocked thread
	volatile uint8_t last_lock_thr;                        // index of the element of 'lock_thr_is_list' of the last blocked thread
	volatile uint8_t lock_thr_cnt;                         // blocked threads count
	volatile uint8_t lock_thr_is_list[FOS_MAX_THR_CNT];    // identifier list of the blocked threads

} fos_lock_t;


// binary semaphore
typedef struct
{
	volatile fos_semb_state_t state;   // semaphore state
	fos_lock_t  fos_lock;              // blocker object
	user_desc_t user_desc;             // used defined semaphore descriptor

} fos_semaphore_binary_t;


typedef fos_semaphore_binary_t* fos_semaphore_binary_ptr;


// error description
typedef struct
{
	uint32_t    err_code;
	user_desc_t user_desc;
	char *ext_str_ptr;
	char str[FOS_MAX_STR_ERR_LEN];

} fos_err_t;


// standard intercepted errors
typedef enum
{
	FOS_ERROR_KERNEL_HEAP = 1,
	FOS_ERROR_THREADS_HEAP,
	FOS_ERROR_THREADS_STACK,
	FOS_ERROR_KERNEL_STACK,

} fos_err_enum;


#endif /* APPLICATION_FOS_FOS_TYPES_H_ */









