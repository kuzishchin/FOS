/**************************************************************************//**
 * @file      fos_semb.c
 * @brief     Binary named strong semaphore. Source file.
 * @version   V1.2.04
 * @date      10.04.2026
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


#include "Sync/fos_semb.h"
#include "Sync/fos_lock.h"
#include "Platform/sl_platform.h"
#include <string.h>



// заглушка на логирование событий
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak fos_ret_t FOS_LogSysData(char *str1, char *str2, uint32_t val, fos_log_type_t type)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}



// инициализация
void FOS_SemaphoreBinary_Init(fos_semaphore_binary_t *p, fos_semb_state_t init_state)
{
	if(p == NULL)
		return;

	p->state = init_state;
	FOS_Lock_Init(&p->fos_lock);
}


// установить пользовательский дескриптор
fos_ret_t FOS_SemaphoreBinary_SetUserDesc(fos_semaphore_binary_t *p, user_desc_t user_desc)
{
	if(p == NULL)
		return FOS__FAIL;

	p->user_desc = user_desc;

	return FOS__OK;
}


// взять
fos_ret_t FOS_SemaphoreBinary_Take(fos_semaphore_binary_t *p, uint8_t thr_id)
{
	if((p == NULL) || (thr_id >= FOS_MAX_THR_CNT))
		return FOS__FAIL;

	switch(p->state)
	{
	case FOS_SEMB_STATE__UNLOCK:                     // если семафор был разблокирован
		p->state = FOS_SEMB_STATE__LOCK;             // блокируем его
	break;

	case FOS_SEMB_STATE__LOCK:                       // если семафор был заблокирован
		return FOS_Lock_Take(&p->fos_lock, thr_id);  // блокируем поток его берущий

	}

	return FOS__OK;
}


// получить статус взятия семафора
// FOS__OK - нормальное взятие семафора, FOS__FAIL - взятие по таймауту
fos_ret_t FOS_SemaphoreBinary_TakeStat(fos_semaphore_binary_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	if(p->timeout.timeout_flag)      // индикация что был таймаут
		return FOS__FAIL;

	return FOS__OK;
}


// дать
fos_ret_t FOS_SemaphoreBinary_Give(fos_semaphore_binary_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	fos_ret_t ret = FOS__OK;
	uint32_t s;
	ENTER_CRITICAL(s);

	switch(p->state)
	{
	case FOS_SEMB_STATE__UNLOCK:                            // если семафор был разблокирован
                                                            // ничего не делаем
	break;

	case FOS_SEMB_STATE__LOCK:                              // если семафор был заблокирован

		if(FOS_Lock_GetLockedThreadsCount(&p->fos_lock))    // если есть заблокированные потоки
			ret = FOS_Lock_Give(&p->fos_lock, FOS__DISABLE);// разблокируем очередной поток и выходим
		else
			p->state = FOS_SEMB_STATE__UNLOCK;              // если заблокированных полтокв нет, разблокируем семафор

	break;
	}

	p->timeout.timeout_flag  = FOS__DISABLE;                              // снимаем флаг таймату по выдаче
	p->timeout.timeout_ts_ms = SL_GetTick() + p->timeout.timeout_ms;      // обновляем метку времени наступления таймаута

	LEAVE_CRITICAL(s);

	return ret;
}


// обработка таймаута
static fos_ret_t FOS_SemaphoreBinary_ProcTimeout(fos_semaphore_binary_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	fos_ret_t ret = FOS__OK;
	uint32_t s;

	if(p->timeout.timeout_ms)                               // если таймауты включены
	{
		if(FOS_Lock_GetLockedThreadsCount(&p->fos_lock))    // если есть заблокированные потоки
		{
			if(SL_GetTick() >= p->timeout.timeout_ts_ms)
			{
#if FOS_DEBUL_LEVEL >= 2
				FOS_LogSysData("Semb is timeout.", "Semb with", p->user_desc, FOS_LOG_TYPE__WARNING); // 16+9+10+6=41 symbols
#endif

				p->timeout.timeout_ts_ms = SL_GetTick() + p->timeout.timeout_ms;

				ENTER_CRITICAL(s);
				p->timeout.timeout_flag = FOS__ENABLE;           // поднимаем флаг таймаута
				ret = FOS_Lock_Give(&p->fos_lock, FOS__ENABLE);  // разблокируем очередной поток и выходим
				LEAVE_CRITICAL(s);
			}

		}else{
			p->timeout.timeout_ts_ms = SL_GetTick() + p->timeout.timeout_ms;
		}
	}

	return ret;
}


// обработка таймаута всех семафоров
void FOS_AllSemaphoreBinary_ProcTimeout(volatile fos_semaphore_binary_ptr *semb_desc_list, uint8_t semb_max_ind)
{
	if((semb_desc_list == NULL) || (semb_max_ind >= FOS_SEM_BIN_CNT))
		return;

	for(uint8_t i = 0; i <= semb_max_ind; i++)
		FOS_SemaphoreBinary_ProcTimeout(semb_desc_list[i]);
}


// отсоединить поток
fos_ret_t FOS_SemaphoreBinary_UnlinkThread(fos_semaphore_binary_t *p, uint8_t thr_id)
{
	if(p == NULL)
		return FOS__FAIL;

	return FOS_Lock_UnlinkThread(&p->fos_lock, thr_id);
}


// освободить все потоки
fos_ret_t FOS_SemaphoreBinary_UnlockAll(fos_semaphore_binary_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	// разблокируем все заблокированные потоки
	while(FOS_Lock_GetLockedThreadsCount(&p->fos_lock))
		FOS_Lock_Give(&p->fos_lock, FOS__DISABLE);

	return FOS__OK;
}


// установить таймаут
fos_ret_t FOS_SemaphoreBinary_SetTimeout(fos_semaphore_binary_t *p, uint32_t timeout_ms)
{
	if(p == NULL)
		return FOS__FAIL;

	if(timeout_ms == FOS_INF_TIME)
		timeout_ms = 0;

	p->timeout.timeout_ms = timeout_ms;

	return FOS__OK;
}














