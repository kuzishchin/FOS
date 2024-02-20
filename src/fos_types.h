/**************************************************************************//**
 * @file      fos_types.h
 * @brief     OS types declarations. Header file.
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

#ifndef APPLICATION_FOS_FOS_TYPES_H_
#define APPLICATION_FOS_FOS_TYPES_H_


#include "fos_conf.h"
#include "compiler_macros.h"
#include <stdint.h>
#include <stddef.h>


#define FOS_SUSPEND_BLOCKED_ID 250           // идентификатор остановленных и заблокированных задач
#define FOS_EMPTY_ID           255           // идентификатор отсутсвия задачи
#define FOS_INF_TIME           0xFFFFFFFF    // бесконечное время
#define FOS_USER_LOCK_MASK     0xFFFF        // маска пользовательских блокировок
#define FOS_LOCK_OBJ_FLAG      0x10000       // флаг блокировки для объекта блокировки
#define FOS_WRONG_THREAD_ID    0xFF          // id неверного дескриптора потока
#define FOS_WRONG_SEM_BIN_ID   0xFF          // id неверного дескриптора бинарного семафора
#define FOS_WRONG_FWRITER_ID   0xFF          // id неверного дескриптора объекта записи
#define FOS_WRONG_USER_DESC    0             // неверный пользовательский дескриптор


// выключатель
typedef enum
{
	FOS__DISABLE = 0,
	FOS__ENABLE,

} fos_sw_t;


// коды возврата
typedef enum
{
	FOS__OK = 0,
	FOS__FAIL,

} fos_ret_t;


// состояния семафора
typedef enum
{
	FOS_SEMB_STATE__LOCK = 0,       // заблокирован
	FOS_SEMB_STATE__UNLOCK,         // разблокирован

} fos_semb_state_t;


// режим работы ОС
typedef enum
{
	FOS__CORE_WORK_MODE = 0,   // режим ядра
	FOS__USER_WORK_MODE,       // режим пользователя

} fos_work_mode_t;


// сосотяния процессов
typedef enum
{
	FOS__THREAD_SUSPEND = 0,   // спящий поток
	FOS__THREAD_BLOCKED,       // заблокированный поток
	FOS__THREAD_READY,         // готовый к запуску поток
	FOS__THREAD_RUNNING,       // аткивный поток

} fos_thread_state_t;


// режим работы потока
typedef enum
{
	FOS__THREAD_NO_INIT = 0,   // не инициализирован
	FOS__THREAD_INIT,          // инициализирован
	FOS__THREAD_READY_TO_RUN,  // готов к запуску
	FOS__THREAD_RUN,           // запущен
	FOS__TERMINATED,           // завершён
	FOS__THREAD_DEINIT,        // деинициализирован

} fos_thread_mode_t;


// пользовательский дескриптор
typedef uint32_t user_desc_t;


// основные глобальные переменные
typedef struct
{
	volatile uint32_t core_sp;           // указатель на стек ядра
	volatile uint32_t user_sp;           // указатель на стек пользовательского процесса

	volatile fos_work_mode_t mode;       // режим работы

	volatile fos_sw_t swithed_by_tim;    // флаг переключения контекста по таймеру
	volatile uint32_t thr_dt_us;         // затрачено на работу текущего процесса в мкс

} fos_mgv_t;


// описание константных настроек потока
typedef struct
{
	uint32_t base_sp;          // базовый адрес стека
	uint32_t ep;               // адрес точки входа
	uint32_t stack_size;       // размер стека

} fos_thread_сset_t;


// настройки потока
typedef struct
{
	volatile uint8_t priotity;          // приоритет потока (0 - наивысший)

} fos_thread_set_t;


// инициализация потока
typedef struct
{
	char *name_ptr;            // указатель на имя потока
	fos_thread_сset_t cset;    // константные настройки
	fos_thread_set_t  set;     // начальные настройки

} fos_thread_init_t;


typedef void (*user_thread_ep_t)();       // точка входа в пользовательский поток


// пользовательская инициализация потока
typedef struct
{
	char            *name_ptr;         // указатель на имя потока
	user_thread_ep_t user_thread_ep;   // точка входа потока
	uint32_t         stack_size;       // размер стека потока
	uint32_t         heap_size;        // размер кучи потока
	uint8_t          priotity;         // приоритет потока (0 - наивысший)

} fos_thread_user_init_t;



typedef void (*svcall_t)(void*);  // прототип функции системного вызоыв


// объект блокировки
typedef struct
{
	volatile uint8_t first_lock_thr;                       // индекс элемента lock_thr_is_list первого заблокированного потока
	volatile uint8_t last_lock_thr;                        // индекс элемента lock_thr_is_list последнего заблокированного потока
	volatile uint8_t lock_thr_cnt;                         // число заблокированных потоков
	volatile uint8_t lock_thr_is_list[FOS_MAX_THR_CNT];    // список id заблокированных потокв

} fos_lock_t;


// бинарный семафор
typedef struct
{
	volatile fos_semb_state_t state;   // состояние семафора
	fos_lock_t  fos_lock;              // объект блокировки
	user_desc_t user_desc;             // пользовательский дескриптор семафора

} fos_semaphore_binary_t;


typedef fos_semaphore_binary_t* fos_semaphore_binary_ptr;


// описание ошибки
typedef struct
{
	uint32_t    err_code;
	user_desc_t user_desc;
	char *ext_str_ptr;
	char str[FOS_MAX_STR_ERR_LEN];

} fos_err_t;


// стандартные перехваченные ошибки
typedef enum
{
	FOS_ERROR_CORE_HEAP = 1,
	FOS_ERROR_THREADS_HEAP,
	FOS_ERROR_THREADS_STACK,
	FOS_ERROR_CORE_STACK,

} fos_err_enum;


#endif /* APPLICATION_FOS_FOS_TYPES_H_ */









