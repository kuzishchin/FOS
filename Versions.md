# FOS versions

## FOS v1.0.7 build 10 08.05.2026 api-1.1

* User descriptor includes more information
* User descriptor is logged in HEX number format
* Some optimization is done
* Some code refactror is done
* Added context cheching while calling API function
* Timeout mechanical is improved
* API is changed:
1. Next function are deprecated because timeout mechanism has been changed
1.1. `API_FOS_SemBinarySetTimeout`
1.2. `API_FOS_SemCntSetTimeout`
2. Timeout is added to argument into below functions
2.1. `API_FOS_SemBinaryTake`
2.2. `API_FOS_SemCntTake`
2.3 `API_FOS_MutexTake`
2.4. `API_FOS_Queue32ReadData`
2.5. `API_FOS_Join`
3. Arguments of the functions below are changed
3.1. `API_FOS_CreateMutex`
3.2. `API_FOS_CreateQueue32`
4. Next function are deprecated because context watch is added
4.1. `API_FOS_SemBinaryGiveFromISR`
4.2. `API_FOS_SemCntGiveFromISR`
4.3. `API_FOS_Queue32WriteDataFromISR`
4.4. `API_FOS_SentNoteToThreadFromISR`
4.5. `API_FOS_LogDataFromISR`
5. In `API_FOS_Yield` adde returned status.

## FOS v1.0.6 build 08 28.04.2026 api-1

* Added the thread function with arguments
* Added `Wait` function
* Added `GetWaitStatus` for `Wait` function
* Added checking while creating threads
* Fixed some potentially bugs while context swith
* Fixed a bug while manuiually error set
* Fixed a bug while unsucsess creating queue32

## FOS v1.0.5 build 07 03.04.2026 api-1

* Added mutex
* Added possibility to allocate memory in the local thread heap
* Added possibility transferring arguments (data) to the thread before start it
* Added flag notes to the thread
* Added periodically run thread
* Added critical code section to API
* Fixed bug while creating queue32
* Some code refactoring is done

## FOS v1.0.4 build 03 18.03.2026 api-1

* Added check conditions in the some API functions
* Added protection in the weak callback mechanism
* IsThreadAlive moved out to the gate
* Fixed bug with false positive check at terminated static thread
* Some code refactoring is done

## FOS v1.0.3 build 02 03.03.2026 api-1

* Fix some bugs in the semaphore logic
* Done settings optimization
* Done some memory optimization
* Added ability to disable some runtime checks

## FOS v1.0.2 build 03 06.02.2026 api-1

* Added release binary semaphore for ISR
* Added timeout event for binary semaphore
* Add version OS in the string
* Fixed some bugs
* Added the support of GCC compiler with optimization (-O3 level)
* Added counting semaphore with functionality as binary semaphore
* Added queue for 32 bit data pieces

## FOS v0.14 built 04.04.2024

* Added dynamically allocated threads witch automatically delete after terminating
* Added garbage collection for terminated threads
* Added function witch deletes binary semaphore
* Added Join thread method
* Added IsAlive thread method

## FOS v0.13 built 18.03.2024

* Added support of ARM Cortex-M4 core
* Added adjustable context swith time
* Fixed some bugs

## FOS v0.12 built 12.03.2024

* System calls optimization
* OS config optimization

## FOS v0.11 built 27.02.2024

* Added the support of GCC v7-2018-q2-update with no optimization

## FOS v0.10 built 14.02.2024

* The first version of RTOS. It supports Arm Cortex M7 only with FPU using.
* OS has been tested by IAR Compiler Arm 8.22.1.15700
* All optimization levels are supported except "Common subexpression elimination" and "Instruction scheduling"

## FOS v0.10 beta built 14.02.2024

* The code is prepared for publications

## FOS v0.10 alpha built 14.02.2024

* 6 Feb - 14 Feb 2024
* The user id is created for threads and binary semaphores
* The file writers creation is dynamically now
* The file writers are improved

## FOS v0.09 alpha bult 05.02.2024

* 30 Jan - 5 Feb 2024
* API has been created
* Run code has been refactored
* Binary semaphores are created dynamically
* Thread`s mode has been created
* Thread termination has been created
* Automatically thread termination is performed if the thread leaves the infinity loop
* Thread termination by id has been created
* Error catcher has been created

## FOS v0.08 alpha bult 22.01.2024

* 15 Jan - 22 Jan 2024
* Thread manager is improved
* Stack error control is added
* System calls is added
* Binary semaphore is developed
* Heap is created
* Thread creation is dynamically now

## FOS v0.07 alpha bult 23.11.2023

* 22 Nov - 23 nov 2023
* The file writers are improved

## FOS v0.06 alpha bult 04.08.2023

* 4 Aug 2023
* The file writers are improved

## FOS v0.05 alpha bult 12.07.2023

* 7 Jul - 12 Jul 2023
* Bug fix
* The file writers are improved

## FOS v0.04 alpha bult 08.06.2023

* 26 May - 8 Jun 2023
* Thread lock an unlock have been created
* First version of the binary semaphore is created
* Thread list is created
* Descriptor system is created
* Idle thread is system now
* File system is added

## FOS v0.03 alpha bult 18.05.2023

* 15 May - 18 May 2023
* Thread priority and state are added
* Thread scheduler is developed

## FOS v0.02 alpha bult 02.05.2023

* 26 Apr - 2 May 2023
* Simple scheduler is created
* Switch context timer is added
* PSP is activated
* Context save and load are added
* FPU support is added

## FOS v0.01 alpha bult 25.04.2023

* 25 Apr 2023
* First context switch is developed

