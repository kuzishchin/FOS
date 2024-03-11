/**************************************************************************//**
 * @file      fos.h
 * @brief     Kernel libs. Header file.
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

#ifndef APPLICATION_FOS_CORE_FOS_H_
#define APPLICATION_FOS_CORE_FOS_H_


#include "System/fos_context.h"
#include "Thread/fos_scheduler.h"
#include "Sync/fos_semb.h"
#include "File/fwriter.h"

/*
 * Поток описывается индексом и дескриптотром
 * Дескриптор потока - описатель потока fos_thread_t
 * Индекс потока - номер потока в таблице дескрипторов потока
 */

extern fos_mgv_t fos_mgv;             // основные глобальные переменные


// переменные ОС
typedef struct
{
	volatile fos_sw_t       fos_sw;                                    // основной выключатель ОС

	volatile uint8_t        current_thr;                               // индекс текущего потока
	volatile uint8_t        thread_max_ind;                            // максимальный индекс зарегистрированного потока
	volatile fos_thread_ptr thread_desc_list[FOS_MAX_THR_CNT];         // список дескрипторов потоков

	volatile uint8_t                  semb_max_ind;                    // максимальный индекс зарегистрированного бинарного семафора
	volatile fos_semaphore_binary_ptr semb_desc_list[FOS_SEM_BIN_CNT]; // список дескрипторов семафоров

	volatile uint8_t     fwriter_max_id;                               // максимальный индекс зарегистированного объекта записи
	volatile fwriter_ptr fwriter_desc_list[FOS_FWRITER_CNT];           // список дескрипторов объектов записи

	volatile fos_err_t   error;                                        // зафиксированная ошибка

	volatile user_desc_t next_user_desc;                               // следующий свободный пользовательский дескриптор

} fos_var_t;

// основная структура ОС
typedef struct
{
	fos_var_t        var;               // переменные
	fos_scheduler_t  sheduler;          // планировщик
	fos_thread_dbg_t sys_stack_dbg;     // отладка системного стека

} fos_t;


// инициализация ОС
void FOS_Init(fos_t *p);

// запус ОС
fos_ret_t FOS_Start(fos_t *p);

// получение id потока по пользовательскопу дескриптору
uint8_t FOS_GetUdThreadId(fos_t *p, user_desc_t user_desc);

// регистрация потока
fos_ret_t FOS_ThreadReg(fos_t *p, fos_thread_t *thr);

// запустить поток с id
fos_ret_t FOS_RunId(fos_t *p, uint8_t id);

// завершить поток с id
fos_ret_t FOS_TerminateId(fos_t *p, uint8_t id, int32_t terminate_code);

// завершить текущий поток
fos_ret_t FOS_Terminate(fos_t *p, int32_t terminate_code);

// уступить другому процессу
void FOS_Yield();

// успыпить поток с id
fos_ret_t FOS_SleepId(fos_t *p, uint8_t id, uint32_t time);

// усыпить текущий поток
fos_ret_t FOS_Sleep(fos_t *p, uint32_t time);

// установить блокировку на поток с id
fos_ret_t FOS_LockId(fos_t *p, uint8_t id, uint32_t lock);

// снять блокировку с потока с id
fos_ret_t FOS_UnlockId(fos_t *p, uint8_t id, uint32_t lock);

// регистрация бинарного семафора
fos_ret_t FOS_SemaphoreBinaryReg(fos_t *p, fos_semaphore_binary_t *semb);

// взять бинарный семафор
fos_ret_t FOS_SemBinaryTake(fos_t *p, user_desc_t semb);

// дать бинарный семафор
fos_ret_t FOS_SemBinaryGive(fos_t *p, user_desc_t semb);

// получение дескриптора объекта записи по id
fwriter_t* FOS_GetFWriterDesc(fos_t *p, uint8_t id);

// регистрация объёкта записи
fos_ret_t FOS_FWriterReg(fos_t *p, fwriter_t *fw);

// зафиксировать ошибку
void FOS_ErrorSet(fos_t *p, fos_err_t *err);

// обработчик основного цикла
void FOS_MainLoopProc(fos_t *p);


/*
 * Пока не пригодилось
 */

// получить id текущего потока
//uint8_t FOS_GetCurrentThreadId(fos_t *p);

// разбудить поток
//fos_ret_t FOS_WeakUpId(fos_t *p, uint8_t id);

// установить блокировку на текущий поток
//fos_ret_t FOS_Lock(fos_t *p, uint32_t lock);


#endif /* APPLICATION_FOS_CORE_FOS_H_ */










