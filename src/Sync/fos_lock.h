/**************************************************************************//**
 * @file      fos_lock.h
 * @brief     Object for locking threads. Header file.
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

#ifndef APPLICATION_FOS_SYNC_FOS_LOCK_H_
#define APPLICATION_FOS_SYNC_FOS_LOCK_H_


#include "fos_types.h"


// инициализация
void FOS_Lock_Init(fos_lock_t *p);

// взять блокировку; блокирует поток с id = thr_id
fos_ret_t FOS_Lock_Take(fos_lock_t *p, uint8_t thr_id);

// отдать блокировку; разблокирует заблокированные потоки в порядке очереди их блокировки
fos_ret_t FOS_Lock_Give(fos_lock_t *p);

// вернуть число заблокированных потоков
uint8_t FOS_Lock_GetLockedThreadsCount(fos_lock_t *p);




#endif /* APPLICATION_FOS_SYNC_FOS_LOCK_H_ */









