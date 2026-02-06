/**************************************************************************//**
 * @file      fos_lock.c
 * @brief     Object for locking threads. Source file.
 * @version   V1.2.01
 * @date      23.01.2026
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


// заглушка на блокировку потока с id
// реализация через функцию ядра
__weak void FOS_Lock_LockThread(uint8_t thr_id)
{

}


// заглушка на разблокировку потока с id
// реализация через функцию ядра
__weak void FOS_Lock_UnlockThread(uint8_t thr_id)
{

}



// инициализация
void FOS_Lock_Init(fos_lock_t *p)
{
	if(p == NULL)
		return;

	memset(p, 0, sizeof(fos_lock_t));
	memset((void*)p->lock_thr_is_list, FOS_WRONG_THREAD_ID, FOS_MAX_THR_CNT);   // массив заполняем FOS_WRONG_THREAD_ID
}


// взять блокировку; блокирует поток с id = thr_id
fos_ret_t FOS_Lock_Take(fos_lock_t *p, uint8_t thr_id)
{
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

	FOS_Lock_LockThread(thr_id);    // блокируем поток

//	if(p->timeout_flag)             // индикация что был таймаут
//		return FOS__FAIL;

	return FOS__OK;
}


// отдать блокировку; разблокирует заблокированные потоки в порядке очереди их блокировки
fos_ret_t FOS_Lock_Give(fos_lock_t *p, fos_sw_t timeout_flag)
{
	if(p == NULL)
		return FOS__FAIL;

	if(p->lock_thr_cnt)                   // если есть заблокированные потоки
	{
		uint8_t thr_id = p->lock_thr_is_list[p->first_lock_thr];         // получаем id первого заблокированного потока
		p->lock_thr_is_list[p->first_lock_thr] = FOS_WRONG_THREAD_ID;    // удаляем id этого потока из массива
		p->first_lock_thr = Private_FOS_Lock_IncInd(p->first_lock_thr);  // инкремент индекса первого заблокированного потока

		p->lock_thr_cnt--;                // декремент счётчика заблокированных потоков

		// обработка таймаута
//		p->timeout_flag = timeout_flag;
		if(timeout_flag)
			p->timeout_cnt++;

		FOS_Lock_UnlockThread(thr_id);    // разблокируем поток

	}

	return FOS__OK;
}


// вернуть число заблокированных потоков
uint8_t FOS_Lock_GetLockedThreadsCount(fos_lock_t *p)
{
	if(p == NULL)
		return 0;
	return p->lock_thr_cnt;
}


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
}







// инкремент индекса
static uint8_t Private_FOS_Lock_IncInd(uint8_t ind)
{
	ind++;
	if(ind >= FOS_MAX_THR_CNT)
		ind = 0;
	return ind;
}











