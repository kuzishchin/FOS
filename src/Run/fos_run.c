/**************************************************************************//**
 * @file      fos_run.c
 * @brief     OS start up functions. Source file.
 * @version   V1.0.01
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



#include "Run/fos_run.h"
#include "Platform/fos_tim_platform.h"


/*
 * Prototype of the user defined initialization function
 * Redefine in a suitable location when using
 */
__weak void USER_FOS_InitAndRun()
{

}


/*
 * Initialization and kernel start
 * Call before entering the main loop
 */
void RUN_FOS_InitAndRun()
{
	FOS_Platform_MainTim_Start();        // timer start
	FOS_Platform_MainTim_Disable();      // and instant timer pause

	GATE_FOS_Init();                     // gate inititalization
	USER_FOS_Init();                     // kernel variables initialization

	USER_FOS_InitAndRun();               // user defined initialization

	USER_FOS_Start();                    // OS start
}


/*
 * Main loop handler
 * Call from the main loop
 */
void RUN_FOS_MainLoopProc()
{
	USER_FOS_MainLoopProc();
}


/*
 * Main timer handler
 * Place to the main timer interrupt handler
 */
void RUN_FOS_TimHandler()
{
	FOS_System_GoToKernelMode(FOS__ENABLE);        // switch to kernel mode
}







