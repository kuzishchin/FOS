/**************************************************************************//**
 * @file      fos_semb.c
 * @brief     Binary named strong semaphore. Source file.
 * @version   V1.1.00
 * @date      04.04.2024
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
#include <string.h>


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


// дать
fos_ret_t FOS_SemaphoreBinary_Give(fos_semaphore_binary_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	switch(p->state)
	{
	case FOS_SEMB_STATE__UNLOCK:                            // если семафор был разблокирован
                                                            // ничего не делаем
	break;

	case FOS_SEMB_STATE__LOCK:                              // если семафор был заблокирован

		if(FOS_Lock_GetLockedThreadsCount(&p->fos_lock))    // если есть заблокированные потоки
			return FOS_Lock_Give(&p->fos_lock);             // разблокируем очередной поток и выходим

		p->state = FOS_SEMB_STATE__UNLOCK;                  // если заблокированных полтокв нет, разблокируем семафор
	break;

	}

	return FOS__OK;
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
		FOS_Lock_Give(&p->fos_lock);

	return FOS__OK;
}





