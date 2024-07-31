/**************************************************************************//**
 * @file      user_fos.c
 * @brief     Kernel. Source file.
 * @version   V1.1.00
 * @date      04.04.2024
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


#include "Kernel/user_fos.h"
#include "Mem/fos_heap.h"
#include "Platform/sl_platform.h"


fos_t fos;                                                 // ОС

extern uint32_t kernel_stack[FOS_KERNEL_STACK_SIZE / 4];   // стек ядра



// основной цикл потока бездействия системы
static void Iddle_Main_thr();

// поток обработки файловой системы
static void FProc_Main_thr();

// создать объект потока
static fos_thread_t* Private_USER_FOS_CreateThreadObj();

// создать объект семафора
static fos_semaphore_binary_t* Private_USER_FOS_CreateSemBinaryObj();

// создать объект записи
static fwriter_t* Private_USER_FOS_CreateFWriterObj();

// инициализация потока
static void USER_FOS_ThreadInit(fos_thread_t *p, fos_thread_init_t *init);

// регистрация потока
static fos_ret_t USER_FOS_ThreadReg(fos_thread_t *thr);

// инициализация и регистрация семафора
static fos_ret_t USER_FOS_SemBinaryInitAndReg(fos_semaphore_binary_t *semb, fos_semb_state_t init_state);

// инициализация и регистрация объекта записи
static fos_ret_t USER_FOS_FWriterInitAndReg(fwriter_t *fw, file_init_t *init);

// бработчик callback ошибки стека
static void FOS_Proc_StackErrorCallback(user_desc_t user_desc);

// OS garbage collection
static void USER_FOS_GarbageCollection();


// прототип перехватчика ошибок
__weak void SYS_FOS_ErrorSet(fos_err_t *err)
{

}


// инициализация ОС
void USER_FOS_Init()
{
	user_desc_t       thr;

	FOS_Init(&fos);
	FOS_Heap_Init();                     // инициализация куч ОС

	/*
	 * Для отладки стека ядра
	 */
	fos.sys_stack_dbg.low_sp = (uint32_t)kernel_stack;
	fos.sys_stack_dbg.high_sp = (uint32_t)kernel_stack + FOS_KERNEL_STACK_SIZE;
	fos.sys_stack_dbg.stack_size = FOS_KERNEL_STACK_SIZE;
	fos.sys_stack_dbg.stack_err_cbk = FOS_Proc_StackErrorCallback;


	/*
	 * Настройки и регистрация системных потоков
	 */

	// поток бездействия системы
	fos_thread_user_init_t user_init = {0};
	user_init.user_thread_ep = Iddle_Main_thr;
	user_init.priotity = FOS_PRIORITY_CNT - 1;		// IDDLE
	user_init.stack_size = STACK_SIZE_IDDLE_THR;
	user_init.heap_size  = STACK_SIZE_IDDLE_THR;    // кучу размером со стек
	user_init.alloc_type = FOS__THREAD_ALLOC_AUTO;
	user_init.name_ptr = "Iddle\0";
	thr = USER_FOS_CreateThread(&user_init);
	USER_FOS_RunDesc(thr);


	// поток обработки файловой системы
	user_init.user_thread_ep = FProc_Main_thr;
	user_init.priotity = FOS_PRIORITY_CNT - 2;		// VERY LOW
	user_init.stack_size = STACK_SIZE_FPROC_THR;
	user_init.heap_size  = FOS_DEF_THR_HEAP_SIZE;
	user_init.alloc_type = FOS__THREAD_ALLOC_AUTO;
	user_init.name_ptr = "FProc\0";
	thr = USER_FOS_CreateThread(&user_init);
	USER_FOS_RunDesc(thr);
}


// запус ОС
fos_ret_t USER_FOS_Start()
{
	return FOS_Start(&fos);
}


// создать поток
user_desc_t USER_FOS_CreateThread(fos_thread_user_init_t *user_init)
{
	if(user_init == NULL)
		return FOS_WRONG_USER_DESC;

	/*
	 * Выделяем память под поток
	 */
	uint32_t thread_mem_size = user_init->stack_size + user_init->heap_size;    // вычисляем объем памяти под поток
	void* thread_mem_ptr = FOS_Heap_ThreadsHeap_Alloc(thread_mem_size);         // выделяем память под поток
	if(thread_mem_ptr == NULL)                                                  // проверяем что выделелась
		return FOS_WRONG_USER_DESC;

	/*
	 * Создаем поток
	 */
	fos_thread_t *thr_ptr = Private_USER_FOS_CreateThreadObj();                 // создаем объект потока
	if(thr_ptr == NULL)                                                         // проверяем что создался
	{
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);                              // освобождаем память
		return FOS_WRONG_USER_DESC;
	}


	/*
	 * Создаём бирнарный семафор потока
	 */
	user_desc_t semb = USER_FOS_CreateSemBinary(FOS_SEMB_STATE__LOCK);
	if(semb == FOS_WRONG_USER_DESC)
	{
		FOS_Heap_KernelHeap_Free(thr_ptr);
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);
		return FOS_WRONG_USER_DESC;
	}

	/*
	 * Инициализируем поток
	 */
	fos_thread_init_t init = {0};
	init.set.priotity = user_init->priotity;
	init.cset.base_sp = (uint32_t)thread_mem_ptr;
	init.cset.stack_size = user_init->stack_size;
	init.cset.ep = (uint32_t)user_init->user_thread_ep;
	init.cset.alloc_type = user_init->alloc_type;
	init.cset.semb = semb;
	init.name_ptr = user_init->name_ptr;
	USER_FOS_ThreadInit(thr_ptr, &init);

	/*
	 * Регистрируем поток
	 */
	if(USER_FOS_ThreadReg(thr_ptr) != FOS__OK)
	{
		// если ошибка регистрации, освобождаем выделенную память
		FOS_Heap_KernelHeap_Free(thr_ptr);
		FOS_Heap_ThreadsHeap_Free(thread_mem_ptr);
		USER_FOS_DeleteSemBinary(semb);
		return FOS_WRONG_USER_DESC;
	}

	return thr_ptr->user_desc;
}


// запустить поток с дескриптором
fos_ret_t USER_FOS_RunDesc(user_desc_t desc)
{
	return FOS_RunId(&fos, FOS_GetUdThreadId(&fos, desc));
}


// завершить поток с дескрипттором
fos_ret_t USER_FOS_TerminateDesc(user_desc_t desc, int32_t terminate_code)
{
	return FOS_TerminateId(&fos, FOS_GetUdThreadId(&fos, desc), terminate_code);
}


// завершить текущий поток
fos_ret_t USER_FOS_Terminate(int32_t terminate_code)
{
	return FOS_Terminate(&fos, terminate_code);
}


// жив ли поток
fos_ret_t USER_FOS_IsThreadAlive(user_desc_t desc)
{
	if(FOS_GetUdThreadId(&fos, desc) == FOS_WRONG_THREAD_ID)
		return FOS__FAIL;
	return FOS__OK;
}


// усыпить текущий поток
fos_ret_t USER_FOS_Sleep(uint32_t time)
{
	return FOS_Sleep(&fos, time);
}


// создать бинарный семафор
user_desc_t USER_FOS_CreateSemBinary(fos_semb_state_t init_state)
{
	fos_semaphore_binary_t* sb_ptr = Private_USER_FOS_CreateSemBinaryObj();
	if(sb_ptr == NULL)
		return FOS_WRONG_USER_DESC;

	// инициализируем и регистрируем
	if(USER_FOS_SemBinaryInitAndReg(sb_ptr, init_state) != FOS__OK)
	{
		// обработка ошибки
		FOS_Heap_KernelHeap_Free(sb_ptr);
		return FOS_WRONG_USER_DESC;
	}

	return sb_ptr->user_desc;
}


// удалить бинарный семафор
fos_ret_t USER_FOS_DeleteSemBinary(user_desc_t semb)
{
	return FOS_SemBinaryDelete(&fos, semb);
}


// взять бинарный светофор
fos_ret_t USER_FOS_SemBinaryTake(user_desc_t semb)
{
	return FOS_SemBinaryTake(&fos, semb);
}


// дать бинарный свнтофор
fos_ret_t USER_FOS_SemBinaryGive(user_desc_t semb)
{
	return FOS_SemBinaryGive(&fos, semb);
}


// get semaphore binary user descriptor by thread user descriptor
user_desc_t USER_FOS_GetThreadSembDesc(user_desc_t desc)
{
	return FOS_GetThreadSembId(&fos, FOS_GetUdThreadId(&fos, desc));
}


// создать объект записи
// используется в слабом подтягивании
fwriter_t* USER_CreateFWriter(uint16_t write_buf_len)
{
	if(write_buf_len == 0)
		return NULL;

	/*
	 * Ограничиваем размер буфера на запись
	 */
	if(write_buf_len > FOS_FILEWR_MAX_BUF_LEN)
		write_buf_len = FOS_FILEWR_MAX_BUF_LEN;

	/*
	 * Выделяем память под буфер записи
	 */
	void* write_buf_ptr = FOS_Heap_ThreadsHeap_Alloc(write_buf_len);            // выделяем память под буфер записи
	if(write_buf_ptr == NULL)                                                  // проверяем что выделелась
		return NULL;

	/*
	 * Создаем объект записи
	 */
	fwriter_t *fwriter_ptr = Private_USER_FOS_CreateFWriterObj();               // создаем объект записи
	if(fwriter_ptr == NULL)                                                     // проверяем что создался
	{
		FOS_Heap_ThreadsHeap_Free(write_buf_ptr);                               // освобождаем память
		return NULL;
	}

	/*
	 * Инициализируе и регистрируем
	 */
	file_init_t file_init = {0};
	file_init.cbuf_init.buf_len = write_buf_len;
	file_init.cbuf_init.buf_ptr = write_buf_ptr;
	if(USER_FOS_FWriterInitAndReg(fwriter_ptr, &file_init) != FOS__OK)
	{
		// если ошибка регистрации, освобождаем выделенную память
		FOS_Heap_KernelHeap_Free(fwriter_ptr);
		FOS_Heap_ThreadsHeap_Free(write_buf_ptr);
		return NULL;
	}

	return fwriter_ptr;
}


// зафиксировать ошибку
void USER_FOS_ErrorSet(fos_err_t *err)
{
	FOS_ErrorSet(&fos, err);

	while(1){};
}


// обработчик основного цикла
void USER_FOS_MainLoopProc()
{
	USER_FOS_GarbageCollection();              // сборка мусора
	FOS_Heap_MainLoopProc();                   // отладка куч
	FOS_MainLoopProc(&fos);                    // переходим к ядру
}


/*
 * Callback
 */

// callback на блокировку потока с id
// используется в слабом подтягивании
void FOS_Lock_LockThread(uint8_t thr_id)
{
	FOS_LockId(&fos, thr_id, FOS_LOCK_OBJ_FLAG);
}


// callback на разблокировку потока с id
// используется в слабом подтягивании
void FOS_Lock_UnlockThread(uint8_t thr_id)
{
	FOS_UnlockId(&fos, thr_id, FOS_LOCK_OBJ_FLAG);
}



/*
 * Системные сервисы
 */

// основной цикл потока бездействия системы
static void Iddle_Main_thr()
{
	while(1)
	{
		SL_Delay(10);      // обычная задержка блокирующая поток
	}
}


// поток обработки файловой системы
static void FProc_Main_thr()
{
	const uint32_t SLEEP_TIME_MS = 10;

	fos_t* f = &fos;                // указатель на FOS
	fwriter_t* fwriter = NULL;      // указатель на FWriter
	uint8_t    isDataToWrite;       // флаг наличия данных на запись

	while(1)
	{
		isDataToWrite = 0;          // сброс флага

		// перебираем все FWriter id
		for(uint8_t i = 0; i <= f->var.fwriter_max_id; i++)
		{
			fwriter = FOS_GetFWriterDesc(f, i);              // получаем дескриптор очередного FWriter
			if(fwriter != NULL)
			{
				FWriter_SysProc(fwriter);                    // обрабатываем его

				if(FWriter_GetDataToWrite(fwriter) != 0)     // если ещё есть данные на запись
					isDataToWrite = 1;                       // ставим флаг
			}
		}

		if(!isDataToWrite)                                   // если данных на запись нет
			FOS_Sleep(f, SLEEP_TIME_MS);                     // усыпляем поток

		File_MountProc();                                    // обработка состояния устройств
	}
}


/*
 * Private
 */

// создать объект потока
static fos_thread_t* Private_USER_FOS_CreateThreadObj()
{
	return (fos_thread_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fos_thread_t));
}


// создать объект семафора
static fos_semaphore_binary_t* Private_USER_FOS_CreateSemBinaryObj()
{
	return (fos_semaphore_binary_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fos_semaphore_binary_t));
}


// создать объект записи
static fwriter_t* Private_USER_FOS_CreateFWriterObj()
{
	return (fwriter_t*)FOS_Heap_KernelHeap_Alloc(sizeof(fwriter_t));
}


// инициализация потока
static void USER_FOS_ThreadInit(fos_thread_t *p, fos_thread_init_t *init)
{
	FOS_ThreadInit(p, init);
}


// регистрация потока
static fos_ret_t USER_FOS_ThreadReg(fos_thread_t *thr)
{
	return FOS_ThreadReg(&fos, thr);
}


// инициализация и регистрация семафора
static fos_ret_t USER_FOS_SemBinaryInitAndReg(fos_semaphore_binary_t *semb, fos_semb_state_t init_state)
{
	FOS_SemaphoreBinary_Init(semb, init_state);
	return FOS_SemBinaryReg(&fos, semb);
}


// инициализация и регистрация объекта записи
static fos_ret_t USER_FOS_FWriterInitAndReg(fwriter_t *fw, file_init_t *init)
{
	FWriter_Init(fw, init);
	return FOS_FWriterReg(&fos, fw);
}


// бработчик callback ошибки стека
static void FOS_Proc_StackErrorCallback(user_desc_t user_desc)
{
	fos_err_t err = {0};
	err.err_code = FOS_ERROR_KERNEL_STACK;
	err.ext_str_ptr = "Kernel stack overflow\0";
	SYS_FOS_ErrorSet(&err);
}


// OS garbage collection
static void USER_FOS_GarbageCollection()
{
	uint32_t adr;
	uint8_t type;

	for(uint8_t i = 0; i < FOS_MAX_OBJ_TO_DEL; i++)
	{
		if(fos.var.obj_to_del_cnt == 0)
			return;

		adr  = fos.var.obj_to_del[i].adr;
		type = fos.var.obj_to_del[i].heap_type;

		if(adr)
		{
			fos.var.obj_to_del[i].adr = 0;
			fos.var.obj_to_del[i].heap_type = 0;
			fos.var.obj_to_del_cnt--;

			switch(type)
			{
			case FOS_KERNEL_HEAP_ID:
				FOS_Heap_KernelHeap_Free((void*)adr);
			break;

			case FOS_THREADS_HEAP_ID:
				FOS_Heap_ThreadsHeap_Free((void*)adr);
			break;
			}
		}
	}
}



/*
 * Пока не пригодилось
 */

// получить id текущего потока
/*uint8_t USER_FOS_GetCurrentThreadId()
{
	return FOS_GetCurrentThreadId(&fos);
}*/


// получить дескриптор текущего потока
/*fos_thread_t* USER_FOS_GetCurrentThreadDesc()
{
	return FOS_GetThreadDesc(&fos, FOS_GetCurrentThreadId(&fos));
}*/


// получить id потока по его дескриптору
/*uint8_t USER_FOS_GetThreadId(fos_thread_t *thr)
{
	return FOS_GetThreadId(&fos, thr);
}*/


// получение дескриптора потока по id
/*fos_thread_t* USER_FOS_GetThreadDesc(uint8_t id)
{
	return FOS_GetThreadDesc(&fos, id);
}*/


// уступить другому потоку
/*void USER_FOS_Yield()
{
	FOS_Yield();
}*/


// успыпить поток с id
/*fos_ret_t USER_FOS_SleepId(uint8_t id, uint32_t time)
{
	return FOS_SleepId(&fos, id, time);
}*/


// усыпить поток с дескриптором
/*fos_ret_t USER_FOS_SleepDesc(fos_thread_ptr desc, uint32_t time)
{
	return FOS_SleepId(&fos, FOS_GetThreadId(&fos, desc), time);
}*/


// запустить поток с id
/*fos_ret_t USER_FOS_RunId(uint8_t id)
{
	return FOS_RunId(&fos, id);
}*/


// разбудить поток с id
/*fos_ret_t USER_FOS_WeakUpId(uint8_t id)
{
	return FOS_WeakUpId(&fos, id);
}*/


// разбудить поток с дескриптором
/*fos_ret_t USER_FOS_WeakUpDesc(fos_thread_ptr desc)
{
	return FOS_WeakUpId(&fos, FOS_GetThreadId(&fos, desc));
}*/


// установить блокировку на поток с id
/*fos_ret_t USER_FOS_LockId(uint8_t id, uint32_t lock)
{
	return FOS_LockId(&fos, id, lock & FOS_USER_LOCK_MASK);
}*/


// установить блокировку на поток с дескриптором
/*fos_ret_t USER_FOS_LockDesc(fos_thread_ptr desc, uint32_t lock)
{
	return FOS_LockId(&fos, FOS_GetThreadId(&fos, desc), lock & FOS_USER_LOCK_MASK);
}*/


// установить блокировку на текущий поток
/*fos_ret_t USER_FOS_Lock(uint32_t lock)
{
	return FOS_Lock(&fos, lock & FOS_USER_LOCK_MASK);
}*/


// снять блокировку с потока с id
/*fos_ret_t USER_FOS_UnlockId(uint8_t id, uint32_t lock)
{
	return FOS_UnlockId(&fos, id, lock & FOS_USER_LOCK_MASK);
}*/


// снять блокировку с потока с дескриптором
/*fos_ret_t USER_FOS_UnlockDesc(fos_thread_ptr desc, uint32_t lock)
{
	return FOS_UnlockId(&fos, FOS_GetThreadId(&fos, desc), lock & FOS_USER_LOCK_MASK);
}*/



















