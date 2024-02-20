/**************************************************************************//**
 * @file      fos_context.h
 * @brief     Low level functional for context switch. Header file.
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

#ifndef APPLICATION_FOS_SYSTEM_CONTEXT_H_
#define APPLICATION_FOS_SYSTEM_CONTEXT_H_


#include "fos_types.h"


// подготовить второй аппаратный стек
void FOS_Core_PreparePSP();

// перейти в режим ядра
void FOS_Core_GoToCoreMode(fos_sw_t swithed_by_tim);

// перейти в режим пользователя
void FOS_Core_GoToUserMode();

// обработчик прерывания PendSV
void PendSV_Handler();

// сохранить конектс
static void FOS_Core_SaveContext();

// восстановить контекст
static void FOS_Core_LoadContext();


#endif /* APPLICATION_FOS_SYSTEM_CONTEXT_H_ */




