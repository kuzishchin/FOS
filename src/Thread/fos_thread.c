/**************************************************************************//**
 * @file      fos_thread.c
 * @brief     Thread object. Source file.
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


#include "Thread/fos_thread.h"
#include "Platform/sl_platform.h"
#include <string.h>


// получить адрес максимальной отметки заполнения стека
static uint32_t FOS_ThreadGetAdrStackWatermark(uint32_t low_sp, uint32_t high_sp);

// обработать сосояние потока
static void FOS_ThreadProcState(fos_thread_t *p);

// добавить данные в стек потока
static void FOS_ThreadPushStack(fos_thread_t *p, uint32_t val);

// удалить данные из стека потока
//static void FOS_ThreadPopStack(fos_thread_t *p);

// инициализация стека потока
static void FOS_ThreadStackInit(fos_thread_t *p);

// функция ловушка для избежания выхода за пределы основного цикла потока
static void Private_FOS_InfLoop();

// установить флаг блокировки потока
static void FOS_ThreadSetLockFlag(fos_thread_t *p, uint32_t lock);

// сбросить флаг блокировки
static void FOS_ThreadReleaseLockFlag(fos_thread_t *p, uint32_t lock);

// вызов callback ошибки стека
static void FOS_Call_StackErrorCallback(fos_thread_dbg_t *p, user_desc_t user_desc);

// бработчик callback ошибки стека
static void FOS_Proc_StackErrorCallback(user_desc_t user_desc);


// прототип функции завершения потока
// реализация через системный вызов
__weak fos_ret_t SYS_FOS_Terminate(int32_t terminate_code);

// прототип перехватчика ошибок
// реализация через системный вызов
__weak void SYS_FOS_ErrorSet(fos_err_t *err);



// инициализация потока
void FOS_ThreadInit(fos_thread_t *p, fos_thread_init_t *init)
{
	if((p == NULL) || (init == NULL))
		return;

	// инициализируем только неинициализированный поток
	if(p->var.mode != FOS__THREAD_NO_INIT)
		return;

	strncpy(p->name, init->name_ptr, FOS_THR_NAME_LEN);
	memcpy(&p->cset, &init->cset, sizeof(fos_thread_cset_t));
	memcpy(&p->set, &init->set, sizeof(fos_thread_set_t));
	memset(&p->var, 0, sizeof(fos_thread_var_t));
	memset(&p->dbg, 0, sizeof(fos_thread_dbg_t));

	p->dbg.low_sp        = p->cset.base_sp;
	p->dbg.high_sp       = p->cset.base_sp + p->cset.stack_size;
	p->dbg.stack_size    = p->cset.stack_size;
	p->dbg.stack_err_cbk = FOS_Proc_StackErrorCallback;

	FOS_ThreadStackInit(p);

	p->var.state = FOS__THREAD_SUSPEND;
	p->var.mode  = FOS__THREAD_INIT;
}


// установить пользовательский дескриптор
fos_ret_t FOS_Thread_SetUserDesc(fos_thread_t *p, user_desc_t user_desc)
{
	if(p == NULL)
		return FOS__FAIL;

	p->user_desc = user_desc;

	return FOS__OK;
}


// установка флага регистриции потока
fos_ret_t FOS_Thread_SetRegFlag(fos_thread_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	// регистрируем только инициализированный поток
	if(p->var.mode != FOS__THREAD_INIT)
		return FOS__FAIL;

	p->var.mode = FOS__THREAD_READY_TO_RUN;

	return FOS__OK;
}


// установка флага запуска потока
fos_ret_t FOS_Thread_SetRunFlag(fos_thread_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	// запустить можно только зарегистрированный поток
	if(p->var.mode != FOS__THREAD_READY_TO_RUN)
		return FOS__FAIL;

	p->var.mode  = FOS__THREAD_RUN;
	p->var.state = FOS__THREAD_READY;

	return FOS__OK;
}


// завершить потока
fos_ret_t FOS_Thread_SetTerminateFlag(fos_thread_t *p, int32_t terminate_code)
{
	if(p == NULL)
		return FOS__FAIL;

	fos_thread_var_t *v = &p->var;

	// завершить можно только готовый к запуску или запущенный поток
	if((v->mode != FOS__THREAD_READY_TO_RUN) && (v->mode != FOS__THREAD_RUN))
		return FOS__FAIL;

	v->mode  = FOS__TERMINATED;
	v->state = FOS__THREAD_SUSPEND;
	v->terminate_code = terminate_code;

	return FOS__OK;
}


// усыпить поток
void FOS_ThreadSleep(fos_thread_t *p, uint32_t time)
{
	if(p == NULL)
		return;

	if(p->var.state == FOS__THREAD_SUSPEND)
		return;

	if(time == FOS_INF_TIME)
		p->var.wake_up_time = 0;
	else
		p->var.wake_up_time = SL_GetTick() + time;

	p->var.state = FOS__THREAD_BLOCKED;
}


// разбудить поток
void FOS_ThreadWeakUp(fos_thread_t *p)
{
	if(p == NULL)
		return;

	if(p->var.state == FOS__THREAD_SUSPEND)
		return;
	p->var.wake_up_time = SL_GetTick();
}


// установить блокировку на поток
void FOS_ThreadLock(fos_thread_t *p, uint32_t lock)
{
	if(p == NULL)
		return;

	if(p->var.state == FOS__THREAD_SUSPEND)
		return;

	FOS_ThreadSetLockFlag(p, lock);

	if(p->var.lock_flag)
		FOS_ThreadSleep(p, FOS_INF_TIME);
}


// снять блокировку с потока
void FOS_ThreadUnlock(fos_thread_t *p, uint32_t lock)
{
	if(p == NULL)
		return;

	if(p->var.state == FOS__THREAD_SUSPEND)
		return;

	FOS_ThreadReleaseLockFlag(p, lock);

	if(!p->var.lock_flag)
		FOS_ThreadWeakUp(p);
}


// обработать состояния всех потоков
void FOS_AllThreadProcState(volatile fos_thread_ptr *thr_desc_list, uint8_t thr_max_ind)
{
	if((thr_desc_list == NULL) || (thr_max_ind >= FOS_MAX_THR_CNT))
		return;

	for(uint8_t i = 0; i <= thr_max_ind; i++)
		FOS_ThreadProcState(thr_desc_list[i]);
}


// обработать отладку потока
void FOS_ThreadProcDbg(fos_thread_dbg_t *d, user_desc_t user_desc)
{
	if(d == NULL)
		return;

	/*
	 * Проверка заполненности стека
	 */
	if((SL_GetTick() - d->ts) >= FOS_STACK_CHECK_PERIOD_MS)
	{
		d->ts = SL_GetTick();
		d->stack_watermark = FOS_ThreadGetAdrStackWatermark(d->low_sp, d->high_sp);
		d->max_stack_usage_b = d->high_sp - d->stack_watermark;
		d->max_stack_usage_p = (float)d->max_stack_usage_b / (float)d->stack_size;
		d->max_stack_usage_p *= 100.0f;

		if(d->max_stack_usage_p > FOS_ERROR_STACK_WML)
			FOS_Call_StackErrorCallback(d, user_desc);
	}
}


// прототип функции завершения потока
// реализация через системный вызов
__weak fos_ret_t SYS_FOS_Terminate(int32_t terminate_code)
{
	return FOS__OK;
}


// прототип перехватчика ошибок
// реализация через системный вызов
__weak void SYS_FOS_ErrorSet(fos_err_t *err)
{

}


// получить адрес максимальной отметки заполнения стека
static uint32_t FOS_ThreadGetAdrStackWatermark(uint32_t low_sp, uint32_t high_sp)
{
	for(uint32_t i = low_sp; i < high_sp; i++)
		if(*(uint8_t*)i != 0)
			return i;

	return high_sp;
}


// обработать состояние потока
static void FOS_ThreadProcState(fos_thread_t *p)
{
	if(p == NULL)
		return;

	fos_thread_var_t *v = &p->var;

	// обрабатываем только поток в работе
	if(v->mode != FOS__THREAD_RUN)
		return;

	/*
	 * Проврека на условие автопробуждения по таймингу
	 */
	if((v->state == FOS__THREAD_BLOCKED) && (v->wake_up_time != 0) && (!v->lock_flag))
	{
		if(SL_GetTick() >= v->wake_up_time)
			v->state = FOS__THREAD_READY;
	}

	FOS_ThreadProcDbg(&p->dbg, p->user_desc);     // обработать отладку потока
}


// добавить данные в стек потока
static void FOS_ThreadPushStack(fos_thread_t *p, uint32_t val)
{
	p->var.sp -= 4;
	*((uint32_t*)p->var.sp) = val;
}


// удалить данные из стека потока
/*
static void FOS_ThreadPopStack(fos_thread_t *p)
{
	p->var.sp += 4;
}*/


// инициализация стека потока
static void FOS_ThreadStackInit(fos_thread_t *p)
{
	p->var.init_sp = p->cset.base_sp;            // берем базовый адрес стека

	p->var.init_sp += p->cset.stack_size - 1;    // переходи к его самому старшему адресу
	p->var.init_sp /= 8;                         // вырваниваем..
	p->var.init_sp *= 8;                         // ..на 8 байт

	p->var.sp = p->var.init_sp;                  // инициализируем указатель стека

	// 18 dword - float point reg
	uint32_t reserved = 0x00000000;
	uint32_t FPSCR    = 0x00000000;
	uint32_t S[16]    = {0};

	// 8 dword - common reg
	uint32_t xPSR     = 0x01000000;                       // тут важно такое значение
	uint32_t PC       = p->cset.ep;                       // точка входа
	uint32_t LR       = (uint32_t)Private_FOS_InfLoop;    // ловушка
	uint32_t R12      = 0x00000000;
	uint32_t R3       = 0x00000000;
	uint32_t R2       = 0x00000000;
	uint32_t R1       = 0x00000000;
	uint32_t R0       = 0x00000000;

	/*
	 * Заполняем стек
	 */
	FOS_ThreadPushStack(p, reserved);
	FOS_ThreadPushStack(p, FPSCR);
	for(uint8_t i = 0; i < 16; i++)
		FOS_ThreadPushStack(p, S[i]);

	FOS_ThreadPushStack(p, xPSR);
	FOS_ThreadPushStack(p, PC);
	FOS_ThreadPushStack(p, LR);
	FOS_ThreadPushStack(p, R12);
	FOS_ThreadPushStack(p, R3);
	FOS_ThreadPushStack(p, R2);
	FOS_ThreadPushStack(p, R1);
	FOS_ThreadPushStack(p, R0);
}


// функция ловушка для избежания выхода за пределы основного цикла потока
static void Private_FOS_InfLoop()
{
	while(1)
	{
		SYS_FOS_Terminate(0);
		SL_Delay(100);
	}
}


// установить флаг блокировки потока
static void FOS_ThreadSetLockFlag(fos_thread_t *p, uint32_t lock)
{
	p->var.lock_flag |= lock;
}


// сбросить флаг блокировки
static void FOS_ThreadReleaseLockFlag(fos_thread_t *p, uint32_t lock)
{
	p->var.lock_flag &= (~lock);
}


// вызов callback ошибки стека
static void FOS_Call_StackErrorCallback(fos_thread_dbg_t *p, user_desc_t user_desc)
{
	if(p->stack_err_cbk)
		p->stack_err_cbk(user_desc);
}


// бработчик callback ошибки стека
static void FOS_Proc_StackErrorCallback(user_desc_t user_desc)
{
	fos_err_t err = {0};
	err.err_code = FOS_ERROR_THREADS_STACK;
	err.ext_str_ptr = "Thread stack overflow\0";
	err.user_desc = user_desc;
	SYS_FOS_ErrorSet(&err);
}












