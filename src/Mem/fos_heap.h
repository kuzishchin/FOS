/**************************************************************************//**
 * @file      fos_heap.h
 * @brief     Abstraction layer for heap. Header file.
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

#ifndef APPLICATION_FOS_MEM_FOS_HEAP_H_
#define APPLICATION_FOS_MEM_FOS_HEAP_H_


#include "fos_types.h"

// инициализация куч
void FOS_Heap_Init();

// обработчик основного цикла
void FOS_Heap_MainLoopProc();

// выделить память в куче ядра
void* FOS_Heap_CoreHeap_Alloc(uint32_t size_bytes);

// выделить память в куче процессов
void* FOS_Heap_ThreadsHeap_Alloc(uint32_t size_bytes);

// освободить память в куче ядра
void FOS_Heap_CoreHeap_Free(void* ptr);

// освободить память в куче процессов
void FOS_Heap_ThreadsHeap_Free(void* ptr);



// прототип перехватчика ошибок
// реализация через системный вызов
__weak void SYS_FOS_ErrorSet(fos_err_t *err);



// обработчик ошибки кучи ядра
static void Private_FOS_Heap_CoreHeap_ErrCbk();

// обработчик ошибки кучи процессов
static void Private_FOS_Heap_ThreadsHeap_ErrCbk();


#endif /* APPLICATION_FOS_MEM_FOS_HEAP_H_ */







