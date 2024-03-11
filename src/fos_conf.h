/**************************************************************************//**
 * @file      fos_conf.h
 * @brief     Configuration file of OS. Header file.
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

#ifndef APPLICATION_FOS_FOS_CONF_H_
#define APPLICATION_FOS_FOS_CONF_H_



#define FOS_MAX_THR_CNT        32          // maximum thread count
#define FOS_SEM_BIN_CNT        32          // maximum binary semaphore count
#define FOS_FWRITER_CNT        32          // maximum writer objects count
#define FOS_SYS_CALL_CNT       32          // maximum system call count
#define FOS_PRIORITY_CNT       8           // maximum priorities count(0 is the highest, 1 - lower than 0, etc.)
#define FOS_THR_NAME_LEN       16          // thread name length
#define FOS_MAX_STR_ERR_LEN    32          // maximum length of error descriptive string

#define FOS_USE_FATFS                      // use FatFs
#define FOS_MAX_FS_DEV         2           // maximum number of devices
#define FOS_FILEWR_MAX_BUF_LEN 2048        // maximum written buffer size, bytes
#define FOS_FS_TIMEOUT_MS      2000        // timeout setting for file system, ms
#define FOS_FSDEV_TIMEOUT_MS   5000        // timeout setting for device mounting, ms

#define FOS_HARD_FAULT_CALL_ID 0xFFFF      // identifier of hard fault calling function

#define FOS_KERNEL_STACK_SIZE  0x800       // kernel stack size (minimum size 0х500)
#define FOS_KERNEL_HEAP_SIZE   0x2000      // kernel heap size
#define FOS_THREADS_HEAP_SIZE  0x18000     // heap size for all the threads

#define FOS_DEF_THR_STACK_SIZE 0x400       // default thread stack size
#define FOS_DEF_THR_HEAP_SIZE  0x400       // default thread heap size

#define FOS_ERROR_STACK_WML    40.0f       // maximum stack fill factor value %, (error event is triggered if exceeded)



#endif /* APPLICATION_FOS_FOS_CONF_H_ */



