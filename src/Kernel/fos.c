/**************************************************************************//**
 * @file      fos.c
 * @brief     Kernel libs. Source file.
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


#include "Kernel/fos.h"
#include <string.h>

// получить id потока по его дескриптору
static uint8_t FOS_GetThreadId(fos_t *p, fos_thread_t *thr);

// получение дескриптора потока по id
static fos_thread_t* FOS_GetThreadDesc(fos_t *p, uint8_t id);

// получение id семафора по пользовательскопу дескриптору
static uint8_t FOS_GetUdSemaphoreBinaryId(fos_t *p, user_desc_t user_desc);

// получить id семафора по его дескриптору
static uint8_t FOS_GetSemaphoreBinaryId(fos_t *p, fos_semaphore_binary_t *semb);

// получение дескриптора бинарного семафора по id
static fos_semaphore_binary_t* FOS_GetSemaphoreBinaryDesc(fos_t *p, uint8_t id);

// получить id объекта записи по его дескриптору
static uint8_t FOS_GetFWriterId(fos_t *p, fwriter_t *fw);

// инициализация ядра ОС
static void Private_FOS_Core_Init(fos_t *p);

// планировщик потоков
static int16_t Private_FOS_Sheduler(fos_t *p);

// сохранить стек пользовательского потока
static void Private_FOS_SaveUserSP(fos_t *p);

// загрузить стек пользовательского потока
static void Private_FOS_LoadUserSP(fos_t *p);

// сгенирировать уникальный пользовательский дескриптор
static user_desc_t Private_FOS_GenUserDesc(fos_t *p);

// обновить максимальный индекс таблицы дескрипторов потоков
static void Private_FOS_UpdThreadMaxInd(fos_t *p);

// обновить максимальный индекс таблицы дескрипторов бинарных семафоров
static void Private_FOS_UpdSemBinaryMaxInd(fos_t *p);

// обновить максимальный индекс таблицы дескрипторов объектов записи
static void Private_FOS_UpdFWriterMaxInd(fos_t *p);


// инициализация ОС
void FOS_Init(fos_t *p)
{
	if(p == NULL)
		return;

	Private_FOS_Core_Init(p);          // инициализация ядра ОС
}


// запус ОС
fos_ret_t FOS_Start(fos_t *p)
{
	if(p == NULL)
		return FOS__FAIL;
	if(p->var.fos_sw == FOS__ENABLE)
		return FOS__FAIL;

	Private_FOS_LoadUserSP(p);             // загрузить стек первого процесса
	p->var.fos_sw = FOS__ENABLE;           // включаем ОС

	return FOS__OK;
}


// получение id потока по пользовательскопу дескриптору
uint8_t FOS_GetUdThreadId(fos_t *p, user_desc_t user_desc)
{
	if((p == NULL) || (user_desc == FOS_WRONG_USER_DESC))
		return FOS_WRONG_THREAD_ID;

	for(uint8_t i = 0; i <= p->var.thread_max_ind; i++)
		if(p->var.thread_desc_list[i])
			if(p->var.thread_desc_list[i]->user_desc == user_desc)
				return i;

	return FOS_WRONG_THREAD_ID;
}


// получить id потока по его дескриптору
static uint8_t FOS_GetThreadId(fos_t *p, fos_thread_t *thr)
{
	if(p == NULL)
		return FOS_WRONG_THREAD_ID;

	for(uint8_t i = 0; i < FOS_MAX_THR_CNT; i++)
		if(p->var.thread_desc_list[i] == thr)
			return i;

	return FOS_WRONG_THREAD_ID;
}


// получение дескриптора потока по id
static fos_thread_t* FOS_GetThreadDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.thread_max_ind)
		return NULL;

	return p->var.thread_desc_list[id];
}


// регистрация потока
fos_ret_t FOS_ThreadReg(fos_t *p, fos_thread_t *thr)
{
	if((p == NULL) || (thr == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// поиск дубликата
	if(FOS_GetThreadId(p, thr) != FOS_WRONG_THREAD_ID)
		return FOS__FAIL;

	// поиск свободной ячейки
	ind = FOS_GetThreadId(p, NULL);
	if(ind == FOS_WRONG_THREAD_ID)
		return FOS__FAIL;

	// присваиваем потоку уникальный пользоваетльский дескриптор
	if(FOS_Thread_SetUserDesc(thr, Private_FOS_GenUserDesc(p)) != FOS__OK)
		return FOS__FAIL;

	// поднимаем флаг регистрации потока
	if(FOS_Thread_SetRegFlag(thr) != FOS__OK)
		return FOS__FAIL;

	v->thread_desc_list[ind] = thr;        // вставляем указатель в свободную ячейку

	Private_FOS_UpdThreadMaxInd(p);        // обновить максимальный индекс

	return FOS__OK;
}


// запустить поток с id
fos_ret_t FOS_RunId(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return FOS__FAIL;

	// получаем дескриптор потока по id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	// поднимае флаг запуска потока
	return FOS_Thread_SetRunFlag(thr);
}


// завершить поток с id
fos_ret_t FOS_TerminateId(fos_t *p, uint8_t id, int32_t terminate_code)
{
	if(p == NULL)
		return FOS__FAIL;

	// получаем дескриптор потока по id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	// ставим флаги завершения потока
	if(FOS_Thread_SetTerminateFlag(thr, terminate_code) != FOS__OK)
		return FOS__FAIL;

	if(id == p->var.current_thr)                // если текущий поток явялется завершаемым
		FOS_System_GoToKernelMode(FOS__DISABLE);    // переходим в режим ядра

	return FOS__OK;
}


// завершить текущий поток
fos_ret_t FOS_Terminate(fos_t *p, int32_t terminate_code)
{
	return FOS_TerminateId(p, p->var.current_thr, terminate_code);
}


// уступить другому процессу
void FOS_Yield()
{
	FOS_System_GoToKernelMode(FOS__DISABLE);       // переключаемся в режим ядра
}


// успыпить поток с id
fos_ret_t FOS_SleepId(fos_t *p, uint8_t id, uint32_t time)
{
	if(p == NULL)
		return FOS__FAIL;

	// получаем дескриптор потока по id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadSleep(thr, time);     // усыпляем поток

	if(id == p->var.current_thr)                // если текущий поток явялется усыпляемым
		FOS_System_GoToKernelMode(FOS__DISABLE);    // переходим в режим ядра

	return FOS__OK;
}


// усыпить текущий поток
fos_ret_t FOS_Sleep(fos_t *p, uint32_t time)
{
	return FOS_SleepId(p, p->var.current_thr, time);
}


// установить блокировку на поток с id
fos_ret_t FOS_LockId(fos_t *p, uint8_t id, uint32_t lock)
{
	if(p == NULL)
		return FOS__FAIL;

	// получаем дескриптор потока по id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadLock(thr, lock);       // блокируем поток

	if(id == p->var.current_thr)                 // если блокируемый поток является текущим
		FOS_System_GoToKernelMode(FOS__DISABLE);     // переходим в режим ядра

	return FOS__OK;
}


// снять блокировку с потока с id
fos_ret_t FOS_UnlockId(fos_t *p, uint8_t id, uint32_t lock)
{
	if(p == NULL)
		return FOS__FAIL;

	// получаем дескриптор потока по id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadUnlock(thr, lock);

	return FOS__OK;
}


// получение id семафора по пользовательскопу дескриптору
static uint8_t FOS_GetUdSemaphoreBinaryId(fos_t *p, user_desc_t user_desc)
{
	if((p == NULL) || (user_desc == FOS_WRONG_USER_DESC))
		return FOS_WRONG_SEM_BIN_ID;

	for(uint8_t i = 0; i <= p->var.semb_max_ind; i++)
		if(p->var.semb_desc_list[i])
			if(p->var.semb_desc_list[i]->user_desc == user_desc)
				return i;

	return FOS_WRONG_SEM_BIN_ID;
}


// получить id семафора по его дескриптору
static uint8_t FOS_GetSemaphoreBinaryId(fos_t *p, fos_semaphore_binary_t *semb)
{
	if(p == NULL)
		return FOS_WRONG_SEM_BIN_ID;

	for(uint8_t i = 0; i < FOS_SEM_BIN_CNT; i++)
		if(p->var.semb_desc_list[i] == semb)
			return i;

	return FOS_WRONG_SEM_BIN_ID;
}


// получение дескриптора бинарного семафора по id
static fos_semaphore_binary_t* FOS_GetSemaphoreBinaryDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.semb_max_ind)
		return NULL;

	return p->var.semb_desc_list[id];
}


// регистрация бинарного семафора
fos_ret_t FOS_SemaphoreBinaryReg(fos_t *p, fos_semaphore_binary_t *semb)
{
	if((p == NULL) || (semb == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// поиск дубликата
	if(FOS_GetSemaphoreBinaryId(p, semb) != FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	// поиск свободной ячейки
	ind = FOS_GetSemaphoreBinaryId(p, NULL);
	if(ind == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	// присваиваем семафору уникальный пользоваетльский дескриптор
	if(FOS_SemaphoreBinary_SetUserDesc(semb, Private_FOS_GenUserDesc(p)) != FOS__OK)
		return FOS__FAIL;

	v->semb_desc_list[ind] = semb;        // вставляем указатель в свободную ячейку

	Private_FOS_UpdSemBinaryMaxInd(p);    // обновить максимальный индекс

	return FOS__OK;
}


// взять бинарный семафор
fos_ret_t FOS_SemBinaryTake(fos_t *p, user_desc_t semb)
{
	if((p == NULL) || (semb == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreBinaryId(p, semb);
	if(id == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_Take(ptr, p->var.current_thr);
}


// дать бинарный семафор
fos_ret_t FOS_SemBinaryGive(fos_t *p, user_desc_t semb)
{
	if((p == NULL) || (semb == FOS_WRONG_USER_DESC))
		return FOS__FAIL;

	uint8_t id = FOS_GetUdSemaphoreBinaryId(p, semb);
	if(id == FOS_WRONG_SEM_BIN_ID)
		return FOS__FAIL;

	fos_semaphore_binary_t *ptr = FOS_GetSemaphoreBinaryDesc(p, id);
	if(ptr == NULL)
		return FOS__FAIL;

	return FOS_SemaphoreBinary_Give(ptr);
}


// получить id объекта записи по его дескриптору
static uint8_t FOS_GetFWriterId(fos_t *p, fwriter_t *fw)
{
	if(p == NULL)
		return FOS_WRONG_FWRITER_ID;

	for(uint8_t i = 0; i < FOS_FWRITER_CNT; i++)
		if(p->var.fwriter_desc_list[i] == fw)
			return i;

	return FOS_WRONG_FWRITER_ID;
}


// получение дескриптора объекта записи по id
fwriter_t* FOS_GetFWriterDesc(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return NULL;

	if(id > p->var.fwriter_max_id)
		return NULL;

	return p->var.fwriter_desc_list[id];
}


// регистрация объекта записи
fos_ret_t FOS_FWriterReg(fos_t *p, fwriter_t *fw)
{
	if((p == NULL) || (fw == NULL))
		return FOS__FAIL;

	uint8_t ind = 0;
	fos_var_t *v = &p->var;

	// поиск дубликата
	if(FOS_GetFWriterId(p, fw) != FOS_WRONG_FWRITER_ID)
		return FOS__FAIL;

	// поиск свободной ячейки
	ind = FOS_GetFWriterId(p, NULL);
	if(ind == FOS_WRONG_FWRITER_ID)
		return FOS__FAIL;

	v->fwriter_desc_list[ind] = fw;        // вставляем указатель в свободную ячейку

	Private_FOS_UpdFWriterMaxInd(p);       // обновить максимальный индекс

	return FOS__OK;
}


// зафиксировать ошибку
void FOS_ErrorSet(fos_t *p, fos_err_t *err)
{
	if((p == NULL) || (err == NULL))
		return;

	memcpy((void*)&p->var.error, err, sizeof(fos_err_t));

	if(err->ext_str_ptr != NULL)
		strncpy((void*)p->var.error.str, err->ext_str_ptr, FOS_MAX_STR_ERR_LEN);
}


// обработчик основного цикла
void FOS_MainLoopProc(fos_t *p)
{
	if(p == NULL)
		return;

	if(p->var.fos_sw == FOS__DISABLE)
		return;

	FOS_ThreadProcDbg(&p->sys_stack_dbg, 0);    // отладка стека ядра

	/*
	 * Обработать сосояния всех потоков
	 */
	FOS_AllThreadProcState(p->var.thread_desc_list, p->var.thread_max_ind);

	if(Private_FOS_Sheduler(p) < 0)          // планировщик потоков
		return;
	FOS_System_GoToUserMode();                 // переключится в режим пользователя
}


// инициализация ядра ОС
static void Private_FOS_Core_Init(fos_t *p)
{
	memset(&p->var, 0, sizeof(fos_var_t));
	memset(&p->sheduler, 0, sizeof(fos_scheduler_t));
	memset(&p->sys_stack_dbg, 0 , sizeof(fos_thread_dbg_t));
}


// планировщик потоков
static int16_t Private_FOS_Sheduler(fos_t *p)
{
	fos_var_t* v = &p->var;

	// сбор статистики для отладки текущего потока
	uint8_t  current_thr = v->current_thr;
	uint32_t thr_dt_us   = fos_mgv.thr_dt_us;
	FOS_ScheduleDbg(&p->sheduler, v->thread_max_ind, current_thr, thr_dt_us);

	/*
	 * Выбираем следующий поток
	 */
	int16_t next_thr = FOS_Schedule(&p->sheduler, v->thread_desc_list, v->thread_max_ind);
	if(next_thr < 0)
		return next_thr;

	Private_FOS_SaveUserSP(p);                      // сохранить стек пользовательтского потока

	/*
	 * Переключение потоков
	 */
	fos_thread_ptr thr = FOS_GetThreadDesc(p, v->current_thr);    // получаем дескриптор текущего потока
	if(thr == NULL)                                               // проверяем что он есть (избыточно!!!)
		return next_thr;

	if(thr->var.state == FOS__THREAD_RUNNING)       // если старый поток был RUNNING (можеть быть и BLOCKED)
		thr->var.state = FOS__THREAD_READY;         // говорим что он READY

	thr = FOS_GetThreadDesc(p, (uint8_t)next_thr);  // получаем дескриптор следующего потока
	if(thr == NULL)                                 // проверяем что он есть (избыточно!!!)
		return next_thr;

	v->current_thr = (uint8_t)next_thr;             // запоминаем индекс следующего потока как текущего
	thr->var.state = FOS__THREAD_RUNNING;           // говорим что он RUNNING

	Private_FOS_LoadUserSP(p);                      // загрузить стек пользовательтского потока

	return next_thr;
}


// сохранить стек пользовательского потока
static void Private_FOS_SaveUserSP(fos_t *p)
{
	fos_thread_t* tp = p->var.thread_desc_list[p->var.current_thr];
	tp->var.sp = fos_mgv.user_sp;
}


// загрузить стек пользовательского потока
static void Private_FOS_LoadUserSP(fos_t *p)
{
	fos_thread_t* tp = p->var.thread_desc_list[p->var.current_thr];
	fos_mgv.user_sp = tp->var.sp;
}


// сгенирировать уникальный пользовательский дескриптор
static user_desc_t Private_FOS_GenUserDesc(fos_t *p)
{
	p->var.next_user_desc++;
	return p->var.next_user_desc;
}


// обновить максимальный индекс таблицы дескрипторов потоков
static void Private_FOS_UpdThreadMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// вычисляем максимальный индекс зарегистрированного потока
	for(uint8_t i = 0; i < FOS_MAX_THR_CNT; i++)
		if(p->var.thread_desc_list[i] != NULL)
			ind = i;

	p->var.thread_max_ind = ind;               // запоминаем максимальный индекс
}


// обновить максимальный индекс таблицы дескрипторов бинарных семафоров
static void Private_FOS_UpdSemBinaryMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// вычисляем максимальный индекс зарегистрированного потока
	for(uint8_t i = 0; i < FOS_SEM_BIN_CNT; i++)
		if(p->var.semb_desc_list[i] != NULL)
			ind = i;

	p->var.semb_max_ind = ind;                // запоминаем максимальный индекс
}


// обновить максимальный индекс таблицы дескрипторов объектов записи
static void Private_FOS_UpdFWriterMaxInd(fos_t *p)
{
	uint8_t ind = 0;

	// вычисляем максимальный индекс зарегистрированного потока
	for(uint8_t i = 0; i < FOS_FWRITER_CNT; i++)
		if(p->var.fwriter_desc_list[i] != NULL)
			ind = i;

	p->var.fwriter_max_id = ind;               // запоминаем максимальный индекс
}


/*
 * Пока не пригодилось
 */

// получить id текущего потока
/*uint8_t FOS_GetCurrentThreadId(fos_t *p)
{
	if(p == NULL)
		return FOS_WRONG_THREAD_ID;

	return p->var.current_thr;
}*/


// разбудить поток
/*fos_ret_t FOS_WeakUpId(fos_t *p, uint8_t id)
{
	if(p == NULL)
		return FOS__FAIL;

	// получаем дескриптор потока по id
	fos_thread_t *thr = FOS_GetThreadDesc(p, id);
	if(thr == NULL)
		return FOS__FAIL;

	FOS_ThreadWeakUp(thr);

	return FOS__OK;
}*/


// установить блокировку на текущий поток
/*fos_ret_t FOS_Lock(fos_t *p, uint32_t lock)
{
	return FOS_LockId(p, p->var.current_thr, lock);
}*/













