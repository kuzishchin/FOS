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


#include "Kernel/fos_gates.h"


/*
 * Prototype of the user defined initialization function
 * Redefine in a suitable location when using
 */
//__weak void USER_FOS_InitAndRun();


/*
 * Activate the program stack
 * Call in the very beginning of the 'main'
 */
#define RUN_FOS_ACTIVATE_PSP_MACRO   \
		FOS_System_PreparePSP();     \
        SET_CONTROL(0x06);


/*
 * Initialization and kernel start
 * Call before entering the main loop
 */
void RUN_FOS_InitAndRun();

/*
 * Main loop handler
 * Call from the main loop
 */
void RUN_FOS_MainLoopProc();

/*
 * Main timer handler
 * Place to the main timer interrupt handler
 */
void RUN_FOS_TimHandler();



#endif /* APPLICATION_FOS_RUN_FOS_RUN_H_ */












