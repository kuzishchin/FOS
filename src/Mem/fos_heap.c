/**************************************************************************//**
 * @file      fos_heap.c
 * @brief     Abstraction layer for heap. Source file.
 * @version   V1.0.05
 * @date      31.03.2026
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



#include "Mem/fos_heap.h"
#include "DMem/dmem.h"
#include <string.h>


static uint8_t kernel_heap_array[FOS_KERNEL_HEAP_SIZE];       // массив для кучи ядра
static uint8_t threads_heap_array[FOS_THREADS_HEAP_SIZE];     // массив для кучи процессов

dmem_heap_t kernel_heap;      // куча ядра
dmem_heap_t threads_heap;     // куча процессов

static dmem_heap_t* local_heaps_list[FOS_MAX_THR_CNT];


// обработчик ошибки кучи ядра
static void Private_FOS_Heap_KernelHeap_ErrCbk();

// обработчик ошибки кучи процессов
static void Private_FOS_Heap_ThreadsHeap_ErrCbk();


// прототип перехватчика ошибок
// реализация через системный вызов
// defined in the fos_system.c
__weak void SYS_FOS_ErrorSet(fos_err_t *err)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
}


// инициализация куч
void FOS_Heap_Init()
{
	dmem_heap_init_t init = {0};

	/*
	 * Инициализируем кучу ядра
	 */
	init.array_ptr = kernel_heap_array;
	init.array_size_byte = FOS_KERNEL_HEAP_SIZE;
	init.dmem_err_cbk_t  = Private_FOS_Heap_KernelHeap_ErrCbk;
	DMem_HeapInit(&kernel_heap, &init);
#ifdef FOS_HEAP_CHECK_PERIOD_MS
	DMem_SetProcPeriod(&kernel_heap, FOS_HEAP_CHECK_PERIOD_MS);
#endif

	/*
	 * Инициализируем кучу процессов
	 */
	init.array_ptr = threads_heap_array;
	init.array_size_byte = FOS_THREADS_HEAP_SIZE;
	init.dmem_err_cbk_t  = Private_FOS_Heap_ThreadsHeap_ErrCbk;
	DMem_HeapInit(&threads_heap, &init);
#ifdef FOS_HEAP_CHECK_PERIOD_MS
	DMem_SetProcPeriod(&threads_heap, FOS_HEAP_CHECK_PERIOD_MS);
#endif
}


// обработчик основного цикла
void FOS_Heap_MainLoopProc()
{
#ifdef FOS_HEAP_CHECK_PERIOD_MS
	DMem_MainLoopProc(&kernel_heap);
	DMem_MainLoopProc(&threads_heap);
#endif
}


// выделить память в куче ядра
void* FOS_Heap_KernelHeap_Alloc(uint32_t size_bytes)
{
	return DMem_Alloc(&kernel_heap, size_bytes);
}


// выделить память в куче процессов
void* FOS_Heap_ThreadsHeap_Alloc(uint32_t size_bytes)
{
	return DMem_Alloc(&threads_heap, size_bytes);
}


// освободить память в куче ядра
void FOS_Heap_KernelHeap_Free(void* ptr)
{
	DMem_Free(&kernel_heap, ptr);
}


// освободить память в куче процессов
void FOS_Heap_ThreadsHeap_Free(void* ptr)
{
	DMem_Free(&threads_heap, ptr);
}


// create the local heap of the thread
fos_ret_t FOS_Heap_LocalHeap_Create(uint8_t thr_id, uint8_t* mem_ptr, uint32_t mem_size)
{
	if((thr_id >= FOS_MAX_THR_CNT) || (mem_ptr == NULL) || (mem_size == 0))
		return FOS__FAIL;

	dmem_heap_t* ptr = local_heaps_list[thr_id];      // если объект уже создан, переиспользуем его
	if(ptr == NULL)                                   // иначе, создаём
	{
		ptr = (dmem_heap_t*)FOS_Heap_KernelHeap_Alloc(sizeof(dmem_heap_t));
		local_heaps_list[thr_id] = ptr;
	}
	if(ptr == NULL)
		return FOS__FAIL;

	// сбрасываем его
	memset(ptr, 0, sizeof(dmem_heap_t));

	// затем инициализируем
	dmem_heap_init_t init = {0};
	init.array_ptr = mem_ptr;
	init.array_size_byte = mem_size;
	init.dmem_err_cbk_t = NULL;
	dmem_ret_t ret = DMem_HeapInit(ptr, &init);
	if(ret != DMEM_OK)
		return FOS__FAIL;

	DMem_CheckHeap(ptr);

	return FOS__OK;
}


// allocate local heap
void* FOS_Heap_LocalHeap_Alloc(uint32_t size_bytes, uint8_t thr_id)
{
	if(thr_id >= FOS_MAX_THR_CNT)
		return NULL;
	void* ptr = DMem_Alloc(local_heaps_list[thr_id], size_bytes);
	DMem_CheckHeap(local_heaps_list[thr_id]);
	return ptr;
}


// free local heap
void FOS_Heap_LocalHeap_Free(void* ptr, uint8_t thr_id)
{
	if(thr_id >= FOS_MAX_THR_CNT)
		return;
	DMem_Free(local_heaps_list[thr_id], ptr);
	DMem_CheckHeap(local_heaps_list[thr_id]);
}


// обработчик ошибки кучи ядра
static void Private_FOS_Heap_KernelHeap_ErrCbk()
{
	fos_err_t err = {0};
	err.err_code = FOS_ERROR_KERNEL_HEAP;
	err.ext_str_ptr = "Wrong CRC in kernel heap\0";
	SYS_FOS_ErrorSet(&err);
}


// обработчик ошибки кучи процессов
static void Private_FOS_Heap_ThreadsHeap_ErrCbk()
{
	fos_err_t err = {0};
	err.err_code = FOS_ERROR_THREADS_HEAP;
	err.ext_str_ptr = "Wrong CRC in threads heap\0";
	SYS_FOS_ErrorSet(&err);
}









