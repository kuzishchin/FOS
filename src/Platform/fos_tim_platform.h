/**************************************************************************//**
 * @file      fos_tim_platform.h
 * @brief     Abstraction layer for timer. Header file.
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

#ifndef APPLICATION_FOS_PLATFORM_FOS_TIM_PLATFORM_H_
#define APPLICATION_FOS_PLATFORM_FOS_TIM_PLATFORM_H_


#include <stdint.h>


/*
 * Prototype of main timer start function
 */
__weak void FOS_Platform_MainTim_Start();


/*
 * Prototype of main timer enable function
 */
__weak void FOS_Platform_MainTim_Enable();


/*
 * Prototype of main timer disable function
 */
__weak void FOS_Platform_MainTim_Disable();


/*
 * Prototype of main timer set counter function
 */
__weak void FOS_Platform_MainTim_SetCounter(uint32_t val);


/*
 * Prototype of main timer get counter function
 */
__weak uint32_t FOS_Platform_MainTim_GetCounter();


/*
 * Prototype of call pending interrupt function by main timer
 */
__weak void CallPendSV();




#endif /* APPLICATION_FOS_PLATFORM_FOS_TIM_PLATFORM_H_ */






