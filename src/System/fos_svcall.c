/**************************************************************************//**
 * @file      fos_svcall.c
 * @brief     Low level functional for system calls. Source file.
 * @version   V1.0.03
 * @date      11.03.2024
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


#include "System/fos_svcall.h"

sys_call_t sys_call;                // системные вызовы


// зарегистировать системную функцию
static void _system_reg_call(sys_call_t *p, svcall_t func, uint16_t func_id);

// обработчик системного вызова
static void system_handler(uint32_t func_id, uint32_t args_adr);



// зарегистировать системную функцию
void system_reg_call(svcall_t func, uint16_t func_id)
{
	_system_reg_call(&sys_call, func, func_id);
}


// систенмый вызов функции с номером func_id и аргументами args
void system_call(uint32_t func_id, void *args)
{
	__asm volatile("push {%0}" ::"r"(func_id));
	__asm volatile("push {%0}" ::"r"(args));
	__asm volatile("svc 0");
	__asm volatile("pop {%0}" ::"r"(func_id));
	__asm volatile("pop {%0}" ::"r"(func_id));
}


// обработчик прерывания системного вызова
void SVC_Handler(void)
{
#if defined(IAR_COMPILER)

	static uint32_t func_id  = 0;
	static uint32_t args_adr = 0;

#elif defined(GCC_COMPILER)

	register uint32_t func_id  __asm("r2");
	register uint32_t args_adr __asm("r1");

#else
	#error Unknown!
#endif

	__asm volatile("mrs r0, psp");
	__asm volatile("add r0, r0, #32");

	__asm volatile
		(
				"tst r14, #0x10    \n"
				"it eq             \n"
				"addeq r0, r0, #72 \n"
		);

	__asm volatile("ldmfd r0, {r1-r2}");

#if defined(IAR_COMPILER)

	__asm volatile("str r2, [%0]" ::"r"(&func_id));
	__asm volatile("str r1, [%0]" ::"r"(&args_adr));

#elif defined(GCC_COMPILER)

#else
	#error Unknown!
#endif

	system_handler(func_id, args_adr);
}


// зарегистировать системную функцию
static void _system_reg_call(sys_call_t *p, svcall_t func, uint16_t func_id)
{
	if((p == NULL) || (func == NULL) || func_id >= FOS_SYS_CALL_CNT)
		return;

	if(p->reg_list[func_id] == 0)
		p->reg_list[func_id] = (uint32_t)func;
}


// обработчик системного вызова
static void system_handler(uint32_t func_id, uint32_t args_adr)
{
	svcall_t func = NULL;
	void*    args = NULL;

	if(func_id == FOS_HARD_FAULT_CALL_ID)
		func(args);

	if(func_id >= FOS_SYS_CALL_CNT)
		return;

	func = (svcall_t)sys_call.reg_list[func_id];
	args = (void*)args_adr;

	if(func)
		func(args);
}





