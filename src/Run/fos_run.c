/**************************************************************************//**
 * @file      fos_run.c
 * @brief     OS start up functions. Source file.
 * @version   V1.0.07
 * @date      13.04.2026
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
#include "Platform/sl_platform.h"


/*
 * 1.5 Prototype of the user defined initialization function
 * Redefine in a suitable location when using
 *
 * Actual from v0.10
 */
__weak void USER_FOS_InitAndRun()
{
	ERROR_PLEASE_IMPLEMENT_THE_CALLBACK();
}


/*
 * 1.2 Initialization and kernel start
 * Call before entering the main loop
 *
 * Actual from v0.10
 */
void RUN_FOS_InitAndRun()
{
	FOS_Platform_MainTim_Start();        // timer start
	FOS_Platform_MainTim_Disable();      // and instant timer pause

	FOS_System_SetMainTimPeriod(FOS_SWITCH_CONTEXT_TIME_US);     // set switch context time
	SL_Delay(FOS_STAB_TIME_MS);                                  // stab time

	GATE_FOS_Init();                     // gate inititalization
	Kernel_FOS_Init();                     // kernel variables initialization

	USER_FOS_InitAndRun();               // user defined initialization

	Kernel_FOS_Start();                    // OS start

#if FOS_DEBUL_LEVEL >= 3
	Kernel_FOS_LogSysData("FOS is started", FOS_LOG_TYPE__INFO); // 14 symbols
#endif
}


/*
 * 1.3 Main loop handler
 * Call from the main loop
 *
 * Actual from v0.10
 */
void RUN_FOS_MainLoopProc()
{
	Kernel_FOS_MainLoopProc();
}


/*
 * 1.4 Main timer handler
 * Place to the main timer interrupt handler
 *
 * Actual from v0.10
 */
void RUN_FOS_TimHandler()
{
	FOS_System_GoToKernelMode(FOS__ENABLE);        // switch to kernel mode
}







