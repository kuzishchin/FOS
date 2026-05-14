/**************************************************************************//**
 * @file      fos_sem.c
 * @brief     Counting named strong semaphore. Source file.
 * @version   V1.2.02
 * @date      08.05.2026
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


#include "Sync/fos_sem.h"
#include "Sync/fos_lock.h"
#include "Platform/sl_platform.h"
#include <string.h>


// инициализация
void FOS_SemaphoreCnt_Init(fos_semaphore_cnt_t *p, uint32_t max_cnt, uint32_t init_cnt)
{
	if(p == NULL)
		return;

	if(init_cnt > max_cnt)
		init_cnt = max_cnt;

	memset(p, 0, sizeof(fos_semaphore_cnt_t));
	p->max_cnt = max_cnt;
	p->cnt = init_cnt;
	FOS_Lock_Init(&p->fos_lock);
}


// установить пользовательский дескриптор
fos_ret_t FOS_SemaphoreCnt_SetUserDesc(fos_semaphore_cnt_t *p, user_desc_t user_desc)
{
	if(p == NULL)
		return FOS__FAIL;

	p->user_desc = user_desc;
	FOS_Lock_SetOwnerUd(&p->fos_lock, user_desc);

	return FOS__OK;
}


// взять
// поток с FOS_SPECIAL_ID уменьшает счётчик но не блоирует
fos_ret_t FOS_SemaphoreCnt_Take(fos_semaphore_cnt_t *p, uint8_t thr_id, uint32_t timeout_ms)
{
	if(p == NULL)
		return FOS__FAIL;

	if((thr_id >= FOS_MAX_THR_CNT) && (thr_id != FOS_SPECIAL_ID))
		return FOS__FAIL;

	if(p->cnt)          // если счётчик не пуст
	{
		p->cnt--;       // декремент
	}else               // если счётчик пуст
	{
		if(thr_id != FOS_SPECIAL_ID)
			return FOS_Lock_Take(&p->fos_lock, thr_id, timeout_ms);    // блокируем поток его берущий
	}

	return FOS__OK;
}


// дать
fos_ret_t FOS_SemaphoreCnt_Give(fos_semaphore_cnt_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	fos_ret_t ret = FOS__OK;
	uint32_t s;
	ENTER_CRITICAL(s);

	if(p->cnt)          // если счётчик не пуст
	{
		p->cnt++;       // инкремент
	}else               // если счётчик пуст
	{
		if(FOS_Lock_GetLockedThreadsCount(&p->fos_lock))    // если есть заблокированные потоки
			ret = FOS_Lock_Give(&p->fos_lock);              // разблокируем очередной поток и выходим
		else
			p->cnt = 1;                                     // если заблокированных полтокв нет, ставим счётчик в 1
	}

	if(p->cnt > p->max_cnt)                                 // ограничиваем счёт
		p->cnt = p->max_cnt;

	LEAVE_CRITICAL(s);

	return ret;
}


// отсоединить поток
fos_ret_t FOS_SemaphoreCnt_UnlinkThread(fos_semaphore_cnt_t *p, uint8_t thr_id)
{
	if(p == NULL)
		return FOS__FAIL;

	return FOS_Lock_UnlinkThread(&p->fos_lock, thr_id);
}


// освободить все потоки
fos_ret_t FOS_SemaphoreCnt_UnlockAll(fos_semaphore_cnt_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	// разблокируем все заблокированные потоки
	while(FOS_Lock_GetLockedThreadsCount(&p->fos_lock))
		FOS_Lock_Give(&p->fos_lock);

	return FOS__OK;
}













