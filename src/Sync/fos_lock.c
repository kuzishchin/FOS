/**************************************************************************//**
 * @file      fos_lock.c
 * @brief     Object for locking threads. Source file.
 * @version   V1.3.04
 * @date      18.05.2026
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


#include "Sync/fos_lock.h"
#include <string.h>


// инкремент индекса
static uint8_t Private_FOS_Lock_IncInd(uint8_t ind);

// декремент индекса
static uint8_t Private_FOS_Lock_DecInd(uint8_t ind);

// поиск индеса массива с нужным id
static uint8_t Private_FOS_Lock_FindIndById(fos_lock_t *p, uint8_t thr_id);


// заглушка на блокировку потока с id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak fos_ret_t FOS_Lock_LockThread(uint8_t thr_id, user_desc_t lock_obj_ud, uint32_t timeout_ms)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}


// заглушка на разблокировку потока с id
// реализация через функцию ядра
// defined in the fos_kernel.c
__weak fos_ret_t FOS_Lock_UnlockThread(uint8_t thr_id)
{
	FOS_INTERNAL_ERROR_OF_THE_CALLBACK();
	return FOS__FAIL;
}


// инициализация
void FOS_Lock_Init(fos_lock_t *p)
{
	if(p == NULL)
		return;

	memset(p, 0, sizeof(fos_lock_t));
	memset((void*)p->lock_thr_is_list, FOS_WRONG_THREAD_ID, FOS_MAX_THR_CNT);   // массив заполняем FOS_WRONG_THREAD_ID
}


// set ud of the owner
void FOS_Lock_SetOwnerUd(fos_lock_t *p, user_desc_t owner_ud)
{
	if(p == NULL)
		return;
	p->owner_ud = owner_ud;
}


// взять блокировку; блокирует поток с id = thr_id
fos_ret_t FOS_Lock_Take(fos_lock_t *p, uint8_t thr_id, uint32_t timeout_ms)
{
	if(timeout_ms == 0)
		return FOS__FAIL;

	if((p == NULL) || (thr_id >= FOS_MAX_THR_CNT))
		return FOS__FAIL;

	if(p->lock_thr_cnt == 0)                   // если не было заблокированных потоков
	{
		p->first_lock_thr = 0;                 // обнуляем индексы
		p->last_lock_thr  = 0;                 // ...
		p->lock_thr_is_list[0] = thr_id;       // в массив с нулевым индексом записываем id первого блокируемого потока
	}else
	{                                                                           // если это не первый заблокированный поток
		p->last_lock_thr = Private_FOS_Lock_IncInd(p->last_lock_thr);           // инкермент индекса посденего заблокированного потоа
		p->lock_thr_is_list[p->last_lock_thr] = thr_id;                         // по этому индексу в массив запоминаем id очередного блокируемого потока
	}

	p->lock_thr_cnt++;              // инкермент счётчика заблокированных потоков

	return FOS_Lock_LockThread(thr_id, p->owner_ud, timeout_ms);    // блокируем поток
}


// отдать блокировку; разблокирует заблокированные потоки в порядке очереди их блокировки
fos_ret_t FOS_Lock_Give(fos_lock_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	uint8_t thr_id = FOS_WRONG_THREAD_ID;

	while((p->lock_thr_cnt) && (thr_id == FOS_WRONG_THREAD_ID))          // пока есть потоки на разблокировку и не раблокировали очередной существующий поток
	{
		thr_id = p->lock_thr_is_list[p->first_lock_thr];                 // получаем id первого заблокированного потока
		p->lock_thr_is_list[p->first_lock_thr] = FOS_WRONG_THREAD_ID;    // удаляем id этого потока из массива
		p->first_lock_thr = Private_FOS_Lock_IncInd(p->first_lock_thr);  // инкремент индекса первого заблокированного потока

		p->lock_thr_cnt--;                      // декремент счётчика заблокированных потоков

		if(thr_id != FOS_WRONG_THREAD_ID)            // если поток существующий
			return FOS_Lock_UnlockThread(thr_id);    // разблокируем поток
	}

	return FOS__FAIL;
}


// вернуть число заблокированных потоков
uint8_t FOS_Lock_GetLockedThreadsCount(fos_lock_t *p)
{
	if(p == NULL)
		return 0;
	return p->lock_thr_cnt;
}

/*
// отсоединить поток от блокиратора
fos_ret_t FOS_Lock_UnlinkThread(fos_lock_t *p, uint8_t thr_id)
{
	if(p == NULL)
		return FOS__FAIL;

	for(uint8_t i = 0; i < FOS_MAX_THR_CNT; i++)
	{
		if(thr_id == p->lock_thr_is_list[i])
		{
			p->lock_thr_is_list[i] = FOS_WRONG_THREAD_ID;
			return FOS__OK;
		}
	}

	return FOS__FAIL;
}*/



// отсоединить поток от блокиратора
fos_ret_t FOS_Lock_UnlinkThread(fos_lock_t *p, uint8_t thr_id)
{
	if(p == NULL)
		return FOS__FAIL;


	uint8_t i = Private_FOS_Lock_FindIndById(p, thr_id);
	if(i == FOS_WRONG_THREAD_ID)
		return FOS__FAIL;

	uint8_t j;
	uint8_t curr_id = p->lock_thr_is_list[i];
	while(curr_id != FOS_WRONG_THREAD_ID)
	{
		j = Private_FOS_Lock_IncInd(i);
		p->lock_thr_is_list[i] = p->lock_thr_is_list[j];
		curr_id = p->lock_thr_is_list[i];
		i = Private_FOS_Lock_IncInd(i);
	}

	p->lock_thr_cnt--;
	p->last_lock_thr = Private_FOS_Lock_DecInd(p->last_lock_thr);

	return FOS__OK;
}


// инкремент индекса
static uint8_t Private_FOS_Lock_IncInd(uint8_t ind)
{
	ind++;
	if(ind >= FOS_MAX_THR_CNT)
		ind = 0;
	return ind;
}


// декремент индекса
static uint8_t Private_FOS_Lock_DecInd(uint8_t ind)
{
	ind--;
	if(ind >= FOS_MAX_THR_CNT)
		ind = FOS_MAX_THR_CNT - 1;
	return ind;
}


// поиск индеса массива с нужным id
static uint8_t Private_FOS_Lock_FindIndById(fos_lock_t *p, uint8_t thr_id)
{
	for(uint8_t i = 0; i < FOS_MAX_THR_CNT; i++)
		if(thr_id == p->lock_thr_is_list[i])
			return i;
	return FOS_WRONG_THREAD_ID;
}








