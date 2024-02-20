/**************************************************************************//**
 * @file      fos_gates.h
 * @brief     Gates for system call handling. Header file.
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

#ifndef APPLICATION_FOS_CORE_FOS_GATES_H_
#define APPLICATION_FOS_CORE_FOS_GATES_H_


#include "Core/user_fos.h"


// инициализировать шлюзы системных вызовов
void GATE_FOS_Init();


// уступить другому процессу
static void GATE_FOS_Yield(void* data);

// усыпить текущий поток
static void GATE_FOS_Sleep(void* data);

// взять бинарный семафор
static void  GATE_FOS_SemBinaryTake(void* data);

// дать бинарный свнтофор
static void GATE_FOS_SemBinaryGive(void* data);

// создать поток
static void GATE_FOS_CreateThread(void* data);

// создать бинарный семафор
static void GATE_FOS_CreateSemBinary(void* data);

// запустить поток с дескриптором
static void GATE_FOS_RunDesc(void* data);

// завершить текущий поток
static void GATE_FOS_Terminate(void* data);

// завершить поток с дескрипттором
static void GATE_FOS_TerminateDesc(void* data);

// зафиксировать ошибку
static void GATE_FOS_ErrorSet(void* data);

// смонитровать файловую систему
static void GATE_File_Mount(void* data);

// размонитровать файловую систему
static void GATE_File_Unmount(void* data);



#endif /* APPLICATION_FOS_CORE_FOS_GATES_H_ */









