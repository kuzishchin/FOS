/**************************************************************************//**
 * @file      fos_mutex.c
 * @brief     Mutex. Source file.
 * @version   V1.1.03
 * @date      15.05.2026
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


#include "Sync/fos_mutex.h"
#include <string.h>

/*
// заглушка на получение приоритета потока c id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak uint8_t FOS_Mutex_GetThreadPriority(uint8_t thr_id)
{
//	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return 0;
}


// заглушка на установку приоритета потока с id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak void FOS_Mutex_SetThreadPriority(uint8_t thr_id, uint8_t priority)
{
//	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
}


// заглушка на сброс приоритета потока с id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak void FOS_Mutex_ResetThreadPriority(uint8_t thr_id)
{
//	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
}*/




// инициализация
void FOS_Mutex_Init(fos_mutex_t *p, fos_mutex_type_t type, uint8_t pcp_priority)
{
	if(p == NULL)
		return;

	memset(p, 0, sizeof(fos_mutex_t));
	p->type         = type;
	p->pcp_priority = pcp_priority;
	p->owner_thr_id = FOS_WRONG_THREAD_ID;
}


// деинициализация
void FOS_Mutex_DeInit(fos_mutex_t *p)
{
	if(p == NULL)
		return;
/*
	if(p->owner_thr_id != FOS_WRONG_THREAD_ID)
		FOS_Mutex_ResetThreadPriority(p->owner_thr_id);*/
}


// set semb descriptor
fos_ret_t FOS_Mutex_SetSemaphorePtr(fos_mutex_t *p, fos_semaphore_binary_ptr semb_ptr)
{
	if(p == NULL)
		return FOS__FAIL;

	p->semb_ptr = semb_ptr;

	return FOS__OK;
}


// установить пользовательский дескриптор
fos_ret_t FOS_Mutex_SetUserDesc(fos_mutex_t *p, user_desc_t user_desc)
{
	if(p == NULL)
		return FOS__FAIL;

	p->user_desc = user_desc;

	return FOS__OK;
}


// взять
fos_ret_t FOS_Mutex_Take(fos_mutex_t *p, uint8_t thr_id, uint32_t timeout_ms, fos_sw_t *lock_flag)
{
	if((p == NULL) || (thr_id >= FOS_MAX_THR_CNT))
		return FOS__FAIL;

	if(thr_id == p->owner_thr_id)       // защита от повторного захвата тем же потоком
		return FOS__FAIL;

//	if(timeout_ms == 0)
//		return FOS__FAIL;
/*
	if(p->owner_thr_id != FOS_WRONG_THREAD_ID)
	{
		uint8_t thread_priority;
		switch(p->type)
		{
		case FOS_MUTEX_TYPE__SIMPLE:
		break;

		case FOS_MUTEX_TYPE__PIP:
			thread_priority = FOS_Mutex_GetThreadPriority(thr_id);
			if(FOS_Mutex_GetThreadPriority(p->owner_thr_id) > thread_priority)
				FOS_Mutex_SetThreadPriority(p->owner_thr_id, thread_priority);
		break;

		case FOS_MUTEX_TYPE__PCP:
			if(FOS_Mutex_GetThreadPriority(p->owner_thr_id) > p->pcp_priority)
				FOS_Mutex_SetThreadPriority(p->owner_thr_id, p->pcp_priority);
		break;
		}
	}*/

	return FOS_SemaphoreBinary_Take(p->semb_ptr, thr_id, timeout_ms, lock_flag);      // берем бинарный семафор
}


// установить владельца
fos_ret_t FOS_Mutex_SetOwner(fos_mutex_t *p, uint8_t thr_id)
{
	if((p == NULL) || (thr_id >= FOS_MAX_THR_CNT))
		return FOS__FAIL;

	/*
	 * Если добролись сюда значит мьютекс был свободный
	 * или произошел таймаут
	 */
/*
	if(p->owner_thr_id != FOS_WRONG_THREAD_ID)
		FOS_Mutex_ResetThreadPriority(p->owner_thr_id);*/

	p->owner_thr_id = thr_id;                               // устанавливаем нового владельца
	return FOS__OK;
}


// дать
fos_ret_t FOS_Mutex_Give(fos_mutex_t *p, uint8_t thr_id)
{
	if((p == NULL) || (thr_id >= FOS_MAX_THR_CNT))
		return FOS__FAIL;

	if(thr_id != p->owner_thr_id)      // проверяем что выдача осуществляется владельцем
		return FOS__FAIL;

//	FOS_Mutex_ResetThreadPriority(p->owner_thr_id);

	p->owner_thr_id = FOS_WRONG_THREAD_ID;             // сброс владаельца
	return FOS_SemaphoreBinary_Give(p->semb_ptr);      // выдаем бинарный семафор
}


// отсоединить поток
fos_ret_t FOS_Mutex_UnlinkThread(fos_mutex_t *p, uint8_t thr_id)
{
	if(p == NULL)
		return FOS__FAIL;

	/*
	 * Вызывается при удадении потока с его thr_id
	 */

	if(thr_id == p->owner_thr_id){                         // если завершен поток захвативший мьютекс
		p->owner_thr_id = FOS_WRONG_THREAD_ID;             // сброс владельца
		return FOS_SemaphoreBinary_Give(p->semb_ptr);      // выдача
	}else{                                                                 // если нет
		return FOS_SemaphoreBinary_UnlinkThread(p->semb_ptr, thr_id);      // исключаем этот поток из очерди на получение если он там есть
	}

//	return FOS__FAIL;
}

















