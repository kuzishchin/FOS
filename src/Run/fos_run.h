/**************************************************************************//**
 * @file      fos_run.h
 * @brief     OS start up functions. Header file.
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

#ifndef APPLICATION_FOS_RUN_FOS_RUN_H_
#define APPLICATION_FOS_RUN_FOS_RUN_H_


#include "Core/fos_gates.h"


/*
 * Активировать программный стек
 * Вызвать в самом начале main
 */
#define RUN_FOS_ACTIVATE_PSP_MACRO   \
		FOS_Core_PreparePSP();       \
        SET_CONTROL(0x06);


/*
 * Инициализация и запуск ядра
 * Вызвать перед входом в основной цикл в main
 */
void RUN_FOS_InitAndRun();

/*
 * Обработчик основного цикла
 * Вызвать из основного цикла
 */
void RUN_FOS_MainLoopProc();

/*
 * Обработчик основного таймера
 * Положить в обработчик прерывания основного таймера
 */
void RUN_FOS_TimHandler();

/*
 * Прототип функции пользовательской инициплизации
 * Для использования переопределить в удобном месте
 */
__weak void USER_FOS_InitAndRun();



#endif /* APPLICATION_FOS_RUN_FOS_RUN_H_ */












