/**************************************************************************//**
 * @file      fos_semb.h
 * @brief     Binary named strong semaphore. Header file.
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

#ifndef APPLICATION_FOS_SYNC_FOS_SEMB_H_
#define APPLICATION_FOS_SYNC_FOS_SEMB_H_


#include "fos_types.h"

// инициализация
void FOS_SemaphoreBinary_Init(fos_semaphore_binary_t *p, fos_semb_state_t init_state);

// установить пользовательский дескриптор
fos_ret_t FOS_SemaphoreBinary_SetUserDesc(fos_semaphore_binary_t *p, user_desc_t user_desc);

// взять
fos_ret_t FOS_SemaphoreBinary_Take(fos_semaphore_binary_t *p, uint8_t thr_id);

// дать
fos_ret_t FOS_SemaphoreBinary_Give(fos_semaphore_binary_t *p);



#endif /* APPLICATION_FOS_SYNC_FOS_SEMB_H_ */







