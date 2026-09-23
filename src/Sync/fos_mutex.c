/**************************************************************************//**
 * @file      fos_mutex.c
 * @brief     Mutex. Source file.
 * @version   V1.2.05
 * @date      18.09.2026
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


// заглушка на получение приоритета потока c id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak uint8_t Kernel_FOS_GetThreadCurrPriorityById(uint8_t thr_id)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return 0;
}


// заглушка на установку приоритета потока с id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak fos_ret_t Kernel_FOS_SetThreadCurrPriorityById(uint8_t thr_id, uint8_t priority)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}


// заглушка на сброс приоритета потока с id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak fos_ret_t Kernel_FOS_ResetThreadCurrPriorityById(uint8_t thr_id)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}




// инициализация
void FOS_Mutex_Init(fos_mutex_t *p, uint8_t pcp_priority, char* name_ptr, uint32_t attr_bits)
{
	if(p == NULL)
		return;

	memset(p, 0, sizeof(fos_mutex_t));
	p->pcp_priority = pcp_priority;
	p->owner_thr_id = FOS_WRONG_THREAD_ID;
	if(name_ptr)
		strncpy(p->name, name_ptr, FOS_MUTEX_NAME_LEN - 1);

	p->type = FOS_MUTEX_TYPE__SIMPLE;
	if(attr_bits & FOS_MUTEX_PIP)
		p->type = FOS_MUTEX_TYPE__PIP;
	if(attr_bits & FOS_MUTEX_PCP)
		p->type = FOS_MUTEX_TYPE__PCP;

	if(attr_bits & FOS_MUTEX_RECURSIVE)
		p->recursive_sw = FOS__ENABLE;

	if(attr_bits & FOS_MUTEX_ROBUST)
		p->robust_sw = FOS__ENABLE;
}


// деинициализация
void FOS_Mutex_DeInit(fos_mutex_t *p)
{
	if(p == NULL)
		return;

	if(p->owner_thr_id != FOS_WRONG_THREAD_ID)
		Kernel_FOS_ResetThreadCurrPriorityById(p->owner_thr_id);
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

	if(thr_id == p->owner_thr_id)              // обработка повторного захвата тем же потоком
	{
		if(p->recursive_sw == FOS__ENABLE)     // если мьютекс рекурентный
		{
			p->recursive_cnt++;                // считаем число повторных взятий
			return FOS__OK;
		}else                                  // если мьютекс обычный
			return FOS__ERR_RESOURCE;          // возвращаем ошибку
	}

	if(p->owner_thr_id != FOS_WRONG_THREAD_ID)
	{
		uint8_t thread_priority;
		switch(p->type)
		{
		case FOS_MUTEX_TYPE__SIMPLE:
		break;

		case FOS_MUTEX_TYPE__PIP:
			thread_priority = Kernel_FOS_GetThreadCurrPriorityById(thr_id);
			if(Kernel_FOS_GetThreadCurrPriorityById(p->owner_thr_id) > thread_priority)
				Kernel_FOS_SetThreadCurrPriorityById(p->owner_thr_id, thread_priority);
		break;

		case FOS_MUTEX_TYPE__PCP:
			if(Kernel_FOS_GetThreadCurrPriorityById(p->owner_thr_id) > p->pcp_priority)
				Kernel_FOS_SetThreadCurrPriorityById(p->owner_thr_id, p->pcp_priority);
		break;
		}
	}

	return FOS_SemaphoreBinary_Take(p->semb_ptr, thr_id, timeout_ms, lock_flag);      // берем бинарный семафор
}


// установить владельца
fos_ret_t FOS_Mutex_SetOwner(fos_mutex_t *p, uint8_t thr_id)
{
	if((p == NULL) || (thr_id >= FOS_MAX_THR_CNT))
		return FOS__FAIL;

	/*
	 * Если добролись сюда значит мьютекс был свободный
	 * но на всякий случай проверим условие ниже
	 */
	if(p->owner_thr_id != FOS_WRONG_THREAD_ID)
		Kernel_FOS_ResetThreadCurrPriorityById(p->owner_thr_id);

	p->owner_thr_id = thr_id;                               // устанавливаем нового владельца
	return FOS__OK;
}


// дать
fos_ret_t FOS_Mutex_Give(fos_mutex_t *p, uint8_t thr_id)
{
	if((p == NULL) || (thr_id >= FOS_MAX_THR_CNT))
		return FOS__FAIL;

	if(thr_id != p->owner_thr_id)      // проверяем что выдача осуществляется владельцем
		return FOS__ERR_RESOURCE;

	if(p->recursive_cnt)               // если счётчик повторных взятий не пуст
	{
		p->recursive_cnt--;            // уменьшаем его
		return FOS__OK;                // и выходим без освобождения мьютекса
	}

	Kernel_FOS_ResetThreadCurrPriorityById(p->owner_thr_id);

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
		if(p->robust_sw == FOS__ENABLE)                    // если мьютекс робастный
		{
			p->owner_thr_id = FOS_WRONG_THREAD_ID;         // сброс владельца
			return FOS_SemaphoreBinary_Give(p->semb_ptr);  // выдача
		}
	}else{                                                                 // если нет
		return FOS_SemaphoreBinary_UnlinkThread(p->semb_ptr, thr_id);      // исключаем этот поток из очерди на получение если он там есть
	}

	return FOS__OK;
}

















