/**************************************************************************//**
 * @file      fos_thread.h
 * @brief     Thread object. Header file.
 * @version   V1.2.01
 * @date      01.04.2026
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

#ifndef APPLICATION_FOS_THREAD_THREAD_H_
#define APPLICATION_FOS_THREAD_THREAD_H_

#include "fos_types.h"


typedef void (*stack_err_cbk_t)(user_desc_t user_desc);  // прототип функции ошибки стека

// отладка
typedef struct
{
	uint32_t low_sp;                    // нижний адрес стека
	uint32_t high_sp;                   // верхний адрес стека
	uint32_t stack_size;                // размер стека

	uint32_t stack_watermark;           // алрес отметки максимальной заполненности стека
	uint32_t max_stack_usage_b;         // максимальное использование стека в байтах
	float max_stack_usage_p;            // максимальное использование стека в процентах

	uint32_t ts;                        // метка времени

	stack_err_cbk_t stack_err_cbk;      // callback ошибки стека

} fos_thread_dbg_t;


// описание переменных потока
typedef struct
{
	volatile uint32_t init_sp;           // начальный указатель стека
	volatile uint32_t sp;                // текущий указатель стека
	volatile uint32_t wake_up_time;      // время пробуждения потока (из соятояния BLOCKED в READY)
	volatile uint32_t lock_flag;         // флаг блокировки потока
	volatile int32_t  terminate_code;    // код завершения потока
	volatile user_desc_t parent;         // дескриптор родидельского потока
	volatile fos_thread_state_t state;   // состояние потока
	volatile fos_thread_mode_t  mode;    // режим потока
	volatile fos_sw_t static_flag;       // static thread flag
	volatile fos_sw_t heap_sw;           // state of the thread local heap
	volatile uint8_t* arg_ptr;           // argmunt pointer in the local thread heap
	volatile uint32_t arg_len;           // argument len
	volatile fos_thr_note_t* note_ptr;   // thread note pointer

} fos_thread_var_t;


// описание потока
typedef struct
{
	char name[FOS_THR_NAME_LEN];  // имя потока
	user_desc_t user_desc;        // пользовательский дескриптор потока

	fos_thread_cset_t cset;  // константные настройки
	fos_thread_set_t  set;   // настройки
	fos_thread_var_t  var;   // переменные
	fos_thread_dbg_t  dbg;   // отладка

} fos_thread_t;


typedef fos_thread_t* fos_thread_ptr;


// инициализация потока
void FOS_ThreadInit(fos_thread_t *p, fos_thread_init_t *init);

// установить пользовательский дескриптор
fos_ret_t FOS_Thread_SetUserDesc(fos_thread_t *p, user_desc_t user_desc, user_desc_t parent);

// установка флага регистриции потока
fos_ret_t FOS_Thread_SetRegFlag(fos_thread_t *p);

// установка флага запуска потока
fos_ret_t FOS_Thread_SetRunFlag(fos_thread_t *p);

// завершить потока
fos_ret_t FOS_Thread_SetTerminateFlag(fos_thread_t *p, int32_t terminate_code);

// усыпить поток
void FOS_ThreadSleep(fos_thread_t *p, uint32_t time);

// разбудить поток
void FOS_ThreadWeakUp(fos_thread_t *p);

// установить блокировку на поток
void FOS_ThreadLock(fos_thread_t *p, uint32_t lock);

// снять блокировку с потока
void FOS_ThreadUnlock(fos_thread_t *p, uint32_t lock);

// is thread run
fos_ret_t FOS_IsThreadRun(fos_thread_t *p);

// обработать состояния всех потоков
void FOS_AllThreadProcState(volatile fos_thread_ptr *thr_desc_list, uint8_t thr_max_ind);

// обработать отладку потока
void FOS_ThreadProcDbg(fos_thread_dbg_t *d, user_desc_t user_desc);

// set local thread state
fos_ret_t FOS_Thread_SetHeapState(fos_thread_t *p, fos_ret_t state);

// add arg to the thread
fos_ret_t FOS_Thread_AddArg(fos_thread_t *p, uint8_t* arg_ptr, uint32_t arg_size);

// get thread argument pointer
uint8_t* FOS_Thread_GetArgPtr(fos_thread_t *p);

// get thread argument len
uint32_t FOS_Thread_GetArgLen(fos_thread_t *p);

// add note pointer
fos_ret_t FOS_Thread_AddNotePtr(fos_thread_t *p, fos_thr_note_t *note_ptr);

// set note
fos_ret_t FOS_Thread_SetNote(fos_thread_t *p, fos_note_type_t type, uint32_t note);

// get note pointer
fos_thr_note_t* FOS_Thread_GetNotePtr(fos_thread_t *p);




#endif /* APPLICATION_FOS_THREAD_THREAD_H_ */





