/**************************************************************************//**
 * @file      fos_lof.c
 * @brief     Logger. Source file.
 * @version   V1.0.03
 * @date      29.04.2026
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


#include "fos_log.h"
#include "Platform/sl_platform.h"
#include <string.h>
#include <stdio.h>

// inc index
static uint16_t Private_FOS_Log_IncIndex(uint16_t buf_size, uint16_t ind);

// write data
fos_ret_t FOS_Log_WriteData(fos_log_t* p, char *str, fos_log_src_t src, fos_log_type_t type, user_desc_t ud)
{
	if((p == NULL) || (str == NULL))
		return FOS__FAIL;

	fos_log_node_t* node = &p->log_list[p->end_ind];
	strncpy((void*)node->str, str, FOS_MAX_STR_LOG_LEN);
	node->src = src;
	node->type = type;
	node->user_desc = ud;
	node->ts = SL_GetTick();
	node->num = p->log_num;
	p->log_num++;
	if(p->cnt < FOS_MAX_LOG_LEN)
		p->cnt++;
	p->end_ind = Private_FOS_Log_IncIndex(FOS_MAX_LOG_LEN, p->end_ind);

	fos_log_evt_cnt_t* cnt = NULL;
	if(src == FOS_LOG_SRC__SYS)
		cnt = &p->sys;
	else
		cnt = &p->user;

	switch(type)
	{
	case FOS_LOG_TYPE__ERROR:
		cnt->error_cnt++;
	break;

	case FOS_LOG_TYPE__WARNING:
		cnt->warning_cnt++;
	break;

	case FOS_LOG_TYPE__INFO:
		cnt->info_cnt++;
	break;
	}

	return FOS__OK;
}


// read index
fos_ret_t FOS_Log_ReadData(fos_log_t* p, fos_log_node_t* node_ptr)
{
	if((p == NULL) || (node_ptr == NULL))
		return FOS__FAIL;

	if(p->cnt == 0)
		return FOS__FAIL;

	fos_log_node_t* node = &p->log_list[p->start_ind];
	memcpy(node_ptr, node, sizeof(fos_log_node_t));
	p->cnt--;
	p->start_ind = Private_FOS_Log_IncIndex(FOS_MAX_LOG_LEN, p->start_ind);

	return FOS__OK;
}


// write in thread safe mode
fos_ret_t FOS_Log_WriteDataProtected(fos_log_t* p, char *str, fos_log_src_t src, fos_log_type_t type, user_desc_t ud)
{
	uint32_t s;
	ENTER_CRITICAL(s);
	fos_ret_t ret = FOS_Log_WriteData(p, str, src, type, ud);
	LEAVE_CRITICAL(s);
	return ret;
}


// read in thread safe mode
fos_ret_t FOS_Log_ReadDataProtected(fos_log_t* p, fos_log_node_t* node_ptr)
{
	uint32_t s;
	ENTER_CRITICAL(s);
	fos_ret_t ret = FOS_Log_ReadData(p, node_ptr);
	LEAVE_CRITICAL(s);
	return ret;
}


// get string
uint32_t FOS_LogNode_GetString(fos_log_node_t* p, char *str, uint32_t len)
{
	if((p == NULL) || (str == NULL))
		return 0;

	str[0] = 0;

	char src[16];
	src[0] = 0;
	switch(p->src)
	{
	case FOS_LOG_SRC__SYS:
		strncpy(src, "SYS", 16);
	break;

	case FOS_LOG_SRC__USER:
		strncpy(src, "USER", 16);
	break;

	case FOS_LOG_SRC__ISR:
		strncpy(src, "ISR", 16);
	break;
	}

	char type[16];
	type[0] = 0;
	switch(p->type)
	{
	case FOS_LOG_TYPE__ERROR:
		strncpy(type, "ERROR", 16);
	break;

	case FOS_LOG_TYPE__WARNING:
		strncpy(type, "WARNING", 16);
	break;

	case FOS_LOG_TYPE__INFO:
		strncpy(type, "INFO", 16);
	break;
	}

	return snprintf(str, len, "%i %s_%s at %ims from ud=0x%08X: %s\r\n", (int)p->num, src, type, (int)p->ts, (int)p->user_desc, p->str);
}


// inc index
static uint16_t Private_FOS_Log_IncIndex(uint16_t buf_size, uint16_t ind)
{
	ind++;
	if(ind >= buf_size)
		ind = 0;
	return ind;
}




















