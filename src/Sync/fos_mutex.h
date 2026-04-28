/**************************************************************************//**
 * @file      fos_mutex.h
 * @brief     AMutex. Header file.
 * @version   V1.0.02
 * @date      27.04.2026
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

#ifndef SYNC_FOS_MUTEX_H_
#define SYNC_FOS_MUTEX_H_


#include "Sync/fos_semb.h"


// инициализация
void FOS_Mutex_Init(fos_mutex_t *p, fos_mutex_type_t type, uint8_t pcp_priority);

// деинициализация
void FOS_Mutex_DeInit(fos_mutex_t *p);

// set semb descriptor
fos_ret_t FOS_Mutex_SetSemaphorePtr(fos_mutex_t *p, fos_semaphore_binary_ptr semb_ptr);

// установить пользовательский дескриптор
fos_ret_t FOS_Mutex_SetUserDesc(fos_mutex_t *p, user_desc_t user_desc);

// взять
fos_ret_t FOS_Mutex_Take(fos_mutex_t *p, uint8_t thr_id);

// установить владельца и получить статус взятия мьютекса
// FOS__OK - нормальное взятие мьютекса, FOS__FAIL - взятие по таймауту
fos_ret_t FOS_Mutex_SetOwnerAndTakeStat(fos_mutex_t *p, uint8_t thr_id);

// дать
fos_ret_t FOS_Mutex_Give(fos_mutex_t *p, uint8_t thr_id);

// отсоединить поток
fos_ret_t FOS_Mutex_UnlinkThread(fos_mutex_t *p, uint8_t thr_id);


#endif /* SYNC_FOS_MUTEX_H_ */










