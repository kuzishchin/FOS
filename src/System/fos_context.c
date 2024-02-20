/**************************************************************************//**
 * @file      fos_context.c
 * @brief     Low level functional for context switch. Source file.
 * @version   V1.0.00
 * @date      14.02.2024
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


#include "System/fos_context.h"
#include "Platform/fos_tim_platform.h"

fos_mgv_t fos_mgv;                            // основные глобальные переменные

#pragma data_alignment = 8
uint32_t core_stack[FOS_CORE_STACK_SIZE / 4]; // стек ядра
//#pragma data_alignment = 1


// подготовить второй аппаратный стек
void FOS_Core_PreparePSP()
{
	static uint32_t core_sp = 0;
	if(core_sp != 0)
		return;

	core_sp = (uint32_t)core_stack;
	core_sp += FOS_CORE_STACK_SIZE - 1;
	core_sp /= 8;
	core_sp *= 8;

	SET_PSP(core_sp);
//	SET_CONTROL(0x2);     // включаем второй стек
//	SET_CONTROL(0x6);     // включае второй стек и принудительно FPU
}

// перейти в режим ядра
void FOS_Core_GoToCoreMode(fos_sw_t swithed_by_tim)
{
	if(fos_mgv.mode != FOS__CORE_WORK_MODE)   // если не режим ядра
	{
		fos_mgv.swithed_by_tim = swithed_by_tim;
		CallPendSV();                         // переключаемся в него
	}
}


// перейти в режим пользователя
void FOS_Core_GoToUserMode()
{
	if(fos_mgv.mode != FOS__USER_WORK_MODE)   // если не режим пользователя
		CallPendSV();                         // переключаемся в него
}


// обработчик прерывания PendSV
void PendSV_Handler()
{
	static __istate_t stat;

	ENTER_CRITICAL(stat);                        // обязательно входим в критическую секцию кода

	FOS_Core_SaveContext();                      // сохраняем контекст

	/*
	 * В зависисости от режима
	 */
	switch(fos_mgv.mode)
	{
	case FOS__CORE_WORK_MODE:                    // если был режим ядра

		fos_mgv.mode = FOS__USER_WORK_MODE;      // переключаем флаг режима в пользовательский

		GET_PSP(fos_mgv.core_sp);                // сохраняем указатель стека ядра
		SET_PSP(fos_mgv.user_sp);                // загружаем указатель стека пользователя

		FOS_Platform_MainTim_Enable();           // запускаем таймер на переключение контекста

		break;

	case FOS__USER_WORK_MODE:                    // если был режим пользоваетля

		FOS_Platform_MainTim_Disable();          // выключаем таймер на переключение контекста

		// запомниаем время затраченное прерванным процессом
		if(fos_mgv.swithed_by_tim)
			fos_mgv.thr_dt_us = 1000;
		else
			fos_mgv.thr_dt_us = FOS_Platform_MainTim_GetCounter();

		FOS_Platform_MainTim_SetCounter(0);      // и обнуляем таймер

		fos_mgv.mode = FOS__CORE_WORK_MODE;      // переключаем флаг режима в ядро

		GET_PSP(fos_mgv.user_sp );               // сохраняем указатель стека пользователя
		SET_PSP(fos_mgv.core_sp);                // загружаем указатель стека ядра

		break;
	}

	FOS_Core_LoadContext();                      // восстанавливаем конекст

	LEAVE_CRITICAL(stat);                        // выходим из критической секции
}


// сохранить конектс
static void FOS_Core_SaveContext()
{
	/*
	 * TO DO:
	 * Сохранить контекст
	 * Пишем non-volatile регистры в стек PSP
	 * Регистры volatile уже сохранены в стеке PSP аппаратно автоматически при входе в PendSV_Handler
	 */

	__asm volatile
	(
	"	mrs r0, psp                  \n"

	"   sub r1, r0, #16              \n"
	"   stmdb r1, {r4-r11}           \n"

	"	sub r2, r0, #64              \n"
	"   vstmDB r2!, {S16-S31}         \n"
	);
}


// восстановить контекст
static void FOS_Core_LoadContext()
{
	/*
	 * TO DO:
	 * Восстановить контекст
	 * Читаем non-volatile регистры из стека PSP
	 * Регистры volatile востановятся из стека PSP аппаратно автоматически при выходе из PendSV_Handler
	 */

	__asm volatile
	(
	"	mrs r0, psp                 \n"

	"   sub r1, r0, #48             \n"
	"   ldmfd r1, {r4-r11}          \n"

	"   sub r2, r0, #128            \n"
	"   vldmIA r2!, {S16-S31}        \n"
	);
}






