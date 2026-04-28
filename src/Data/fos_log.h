/**************************************************************************//**
 * @file      fos_lof.h
 * @brief     Logger. Header file.
 * @version   V1.0.02
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

#ifndef DATA_FOS_LOG_H_
#define DATA_FOS_LOG_H_

#include "fos_types.h"

// event counters
typedef struct
{
	uint32_t info_cnt;
	uint32_t warning_cnt;
	uint32_t error_cnt;

} fos_log_evt_cnt_t;

// log list
typedef struct
{
	fos_log_node_t log_list[FOS_MAX_LOG_LEN];      // log list

	uint16_t start_ind;     // index to read data
	uint16_t end_ind;       // index to write data
	uint16_t cnt;           // counter of elements

	uint16_t log_num;       // current log num

	fos_log_evt_cnt_t sys;
	fos_log_evt_cnt_t user;

} fos_log_t;

// write data
fos_ret_t FOS_Log_WriteData(fos_log_t* p, char *str, fos_log_src_t src, fos_log_type_t type, user_desc_t ud);

// read index
fos_ret_t FOS_Log_ReadData(fos_log_t* p, fos_log_node_t* node_ptr);

// write in thread safe mode
fos_ret_t FOS_Log_WriteDataProtected(fos_log_t* p, char *str, fos_log_src_t src, fos_log_type_t type, user_desc_t ud);

// read in thread safe mode
fos_ret_t FOS_Log_ReadDataProtected(fos_log_t* p, fos_log_node_t* node_ptr);

// get string
uint32_t FOS_LogNode_GetString(fos_log_node_t* p, char *str, uint32_t len);


#endif /* DATA_FOS_LOG_H_ */











