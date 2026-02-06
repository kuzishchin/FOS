/**************************************************************************//**
 * @file      fos_svcall.h
 * @brief     Low level functional for system calls. Header file.
 * @version   V1.0.05
 * @date      06.02.2026
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

#ifndef APPLICATION_FOS_SYSTEM_FOS_SVCALL_H_
#define APPLICATION_FOS_SYSTEM_FOS_SVCALL_H_


#include "fos_types.h"


// описание системных вызовов
typedef struct
{
	uint32_t reg_list[FOS_SYS_CALL_CNT];      // лист регистрации системных вызовов (тут дареса систенмых вызовов)

} sys_call_t;


// зарегистировать системную функцию
void system_reg_call(svcall_t func, uint16_t func_id);

// систенмый вызов функции с номером func_id и аргументами args
void system_call(uint32_t func_id, void *args);

// обработчик прерывания системного вызова
void SVC_Handler(void);




#endif /* APPLICATION_FOS_SYSTEM_FOS_SVCALL_H_ */










