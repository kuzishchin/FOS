/**************************************************************************//**
 * @file      fos_api.c
 * @brief     API of OS for queue32. Source file.
 * @version   V1.0.02
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



#include "Data/fos_queue32.h"


// initialization
fos_ret_t FOS_Queue32_Init(fos_queue32_t* p, uint32_t* buf_ptr, uint16_t buf_size)
{
	if((p == NULL) || (buf_ptr == NULL))
		return FOS__FAIL;

	msg32_ret_t ret = Msg32_Initialize(&p->msg, buf_ptr, buf_size);
	if(ret != MSG32__OK)
		return FOS__FAIL;

	return FOS__OK;
}


// set user descriptor
fos_ret_t FOS_Queue32_SetUserDesc(fos_queue32_t *p, user_desc_t user_desc)
{
	if(p == NULL)
		return FOS__FAIL;

	p->user_desc = user_desc;

	return FOS__OK;
}


// set user descriptor
fos_ret_t FOS_Queue32_SetSemaphorePtr(fos_queue32_t *p, fos_semaphore_cnt_ptr semc_ptr)
{
	if(p == NULL)
		return FOS__FAIL;

	p->semc_ptr = semc_ptr;

	return FOS__OK;
}


// write data
fos_ret_t FOS_Queue32_WriteData(fos_queue32_t* p, uint32_t data)
{
	if(p == NULL)
		return FOS__FAIL;

	uint32_t s;
	ENTER_CRITICAL(s);

	msg32_ret_t ret = Msg32_WriteData(&p->msg, data);
	if((ret == MSG32__OK) && (p->semc_ptr))
		FOS_SemaphoreCnt_Give(p->semc_ptr);

	LEAVE_CRITICAL(s);

	if(ret != MSG32__OK)
		return FOS__FAIL;

	return FOS__OK;
}


// ask data
// if thr_id == FOS_SPECIAL_ID semafore is taken but thread is not blocked
fos_ret_t FOS_Queue32_AskData(fos_queue32_t* p, uint8_t thr_id)
{
	if(p == NULL)
		return FOS__FAIL;

	if(p->semc_ptr)
		return FOS_SemaphoreCnt_Take(p->semc_ptr, thr_id);

	return FOS__OK;
}


// read data
// one must ask data before read every times
fos_ret_t FOS_Queue32_ReadData(fos_queue32_t* p, uint32_t* data_ptr)
{
	if(p == NULL)
		return FOS__FAIL;

	msg32_ret_t ret = Msg32_ReadData(&p->msg, data_ptr);
	if(ret != MSG32__OK)
		return FOS__FAIL;

	return FOS__OK;
}




















