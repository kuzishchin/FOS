/**************************************************************************//**
 * @file      fos_context.c
 * @brief     Low level functional for context switch. Source file.
 * @version   V1.0.01
 * @date      27.02.2024
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

//#pragma data_alignment = 8
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
	/*
	 * Входим в критическую секцию кода
	 */
	__asm volatile ("ldr r0, =1");            // r0 = 1
	__asm volatile ("msr primask, r0");       // basepri = r0 = 1, блокируем прерывания

	/*
	 * Восстанвливаем значение регистра r7
	 * Нужно только для GCC компилятора
	 */
#if defined (GCC_COMPILER)
	__asm volatile ("mov r3, r7");               // r3 = r7
	__asm volatile ("mrs r0, msp");              // r0 = msp
	__asm volatile ("add r1, r0, #16");          // отступаем назад на 16 байт к r7
	__asm volatile ("ldmfd r1, {r7}");           // восстанавливаем r7
#endif

	/*
	 * Сохранить контекст
	 * Пишем non-volatile регистры в стек PSP
	 * r4-r11, S16-S31
	 *
	 * Регистры volatile уже сохранены в стеке PSP аппаратно автоматически при входе в PendSV_Handler
	 * r0-r3, r12, r14(LR), retAdr, xPSR, S0-S15, FPSCR
	 *
	 * r13(SP) и r15(PC) - сохранять не нужно
	 */
	__asm volatile ("mrs r0, psp");              // r0 = psp

	// пишем регистры в стек с общим смещение 32+16=48 байт
	__asm volatile ("sub r1, r0, #16");          // r1 = r0 - 16 = psp - 16, отступаем вверх по стеку на 16 байт
	__asm volatile ("stmdb r1, {r4-r11}");       // сохраняем регитсры r4-r11 отступая вверх по стеку на 8*4=32 байта

	// пишем регистры в стек с общим смещение 64+64=128 байт
	__asm volatile ("sub r2, r0, #64");          // r2 = r0 - 64 = psp - 64, отступаем вверх по стеку на 64 байта
	__asm volatile ("vstmDB r2!, {S16-S31}");    // сохраняем регитсры S16-S31 отступая вверх по стеку на 16*4=64 байта

	/*
	 * Возвращаем значение регистар r7
	 * Нужно только для GCC компилятора
	 */
#if defined (GCC_COMPILER)
	__asm volatile ("mov r7, r3");               // r7 = r3
#endif

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


	/*
	 * Загружаем контекст
	 * non-volatile
	 * r4-r11, S16-S31
	 *
	 * volatile
	 * r0-r3, r12, r14(LR), retAdr, xPSR, S0-S15, FPSCR
	 *
	 * r13(SP) и r15(PC) - трогать не нужно
	 */
	__asm volatile ("mrs r0, psp");               // r0 = psp

	// читаем регитсры из стека со смещением 48 байт
	__asm volatile ("sub r1, r0, #48");           // r1 = r0 - 48 = psp - 48, отступаем вверх по стеку на 44 байт
	__asm volatile ("ldmfd r1, {r4-r11}");        // читаем регистры сверху вниз

	// читаем регитсры из стека со смещением 128 байт
	__asm volatile ("sub r2, r0, #128");         // r2 = r0 - 128 = psp - 128, отступаем вверх по стеку на 128 байт
	__asm volatile ("vldmIA r2!, {S16-S31}");    // читаем регистры сверху вниз



	/*
	 * Выходим из критической секции кода
	 */
	__asm volatile ("ldr r0, =0");            // r0 = 0
	__asm volatile ("msr primask, r0");       // basepri = r0 = 0, разрешаем прерывания

	/*
	 * Выходим сами
	 * Нужно только для GCC компилятора
	 */
#if defined (GCC_COMPILER)
	__asm volatile ("add sp, #16");           // очищаем стек
	__asm volatile ("pop {r0, lr}");
	__asm volatile ("bx lr");                 // выходим
#endif
}








