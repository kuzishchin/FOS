/**************************************************************************//**
 * @file      fos_api.h
 * @brief     API of OS for queue32. Header file.
 * @version   V1.0.02
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

#ifndef DATA_FOS_QUEUE32_H_
#define DATA_FOS_QUEUE32_H_


#include "CircBuf/msg32.h"
#include "Sync/fos_sem.h"


// queue of uint32_t elements
typedef struct
{
	msg32_t msg;                         // circ buf of uint32_t elements
	fos_semaphore_cnt_ptr semc_ptr;      // counting semaphore for sync operations, if NULL then do not use
	user_desc_t user_desc;               // used defined semaphore descriptor

} fos_queue32_t;

typedef fos_queue32_t* fos_queue32_ptr;


// initialization
fos_ret_t FOS_Queue32_Init(fos_queue32_t* p, uint32_t* buf_ptr, uint16_t buf_size);

// set user descriptor
fos_ret_t FOS_Queue32_SetUserDesc(fos_queue32_t *p, user_desc_t user_desc);

// set user descriptor
fos_ret_t FOS_Queue32_SetSemaphorePtr(fos_queue32_t *p, fos_semaphore_cnt_ptr semc_ptr);

// write data
fos_ret_t FOS_Queue32_WriteData(fos_queue32_t* p, uint32_t data);

// ask data
// if thr_id == FOS_SPECIAL_ID semafore is taken but thread is not blocked
fos_ret_t FOS_Queue32_AskData(fos_queue32_t* p, uint8_t thr_id);

// read data
// one must ask data before read every times
fos_ret_t FOS_Queue32_ReadData(fos_queue32_t* p, uint32_t* data_ptr);


#endif /* DATA_FOS_QUEUE32_H_ */







