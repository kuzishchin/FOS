/**************************************************************************//**
 * @file      fos_sem.h
 * @brief     Counting named strong semaphore. Header file.
 * @version   V1.1.02
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

#ifndef SYNC_FOS_SEM_H_
#define SYNC_FOS_SEM_H_


#include "fos_types.h"


// инициализация
void FOS_SemaphoreCnt_Init(fos_semaphore_cnt_t *p, uint32_t max_cnt, uint32_t init_cnt);

// установить пользовательский дескриптор
fos_ret_t FOS_SemaphoreCnt_SetUserDesc(fos_semaphore_cnt_t *p, user_desc_t user_desc);

// взять
// поток с FOS_SPECIAL_ID уменьшает счётчик но не блоирует
fos_ret_t FOS_SemaphoreCnt_Take(fos_semaphore_cnt_t *p, uint8_t thr_id);

// получить статус взятия семафора
// FOS__OK - нормальное взятие семафора, FOS__FAIL - взятие по таймауту
fos_ret_t FOS_SemaphoreCnt_TakeStat(fos_semaphore_cnt_t *p);

// дать
fos_ret_t FOS_SemaphoreCnt_Give(fos_semaphore_cnt_t *p);

// обработка таймаута всех семафоров
void FOS_AllSemaphoreCnt_ProcTimeout(volatile fos_semaphore_cnt_ptr *sem_desc_list, uint8_t sem_max_ind);

// отсоединить поток
fos_ret_t FOS_SemaphoreCnt_UnlinkThread(fos_semaphore_cnt_t *p, uint8_t thr_id);

// освободить все потоки
fos_ret_t FOS_SemaphoreCnt_UnlockAll(fos_semaphore_cnt_t *p);

// установить таймаут
fos_ret_t FOS_SemaphoreCnt_SetTimeout(fos_semaphore_cnt_t *p, uint32_t timeout_ms);



#endif /* SYNC_FOS_SEM_H_ */




















