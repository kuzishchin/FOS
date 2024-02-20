/**************************************************************************//**
 * @file      fos_run.c
 * @brief     OS start up functions. Source file.
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



#include "Run/fos_run.h"
#include "Platform/fos_tim_platform.h"

/*
 * Инициализация и запуск ядра
 * Вызвать перед входом в основной цикл в main
 */
void RUN_FOS_InitAndRun()
{
	FOS_Platform_MainTim_Start();        // запуск таймера
	FOS_Platform_MainTim_Disable();      // и тут же ставим его на паузу

	GATE_FOS_Init();                     // инициализация шлюзов
	USER_FOS_Init();                     // инициализация переменных ядра

	USER_FOS_InitAndRun();               // пользовательская инициализация

	USER_FOS_Start();                    // запуск ОС
}


/*
 * Обработчик основного цикла
 * Вызвать из основного цикла
 */
void RUN_FOS_MainLoopProc()
{
	USER_FOS_MainLoopProc();
}


/*
 * Обработчик основного таймера
 * Положить в обработчик прерывания основного таймера
 */
void RUN_FOS_TimHandler()
{
	FOS_Core_GoToCoreMode(FOS__ENABLE);        // переключаемся в режим ядра
}


/*
 * Прототип функции пользовательской инициализации
 * Для использования переопределить в удобном месте
 */
__weak void USER_FOS_InitAndRun()
{

}





