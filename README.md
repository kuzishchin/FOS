# Fly Operating System (FOS)

## Information
FOS is a Real Time Operating System (RTOS) for MCU-controlled robotic systems.
Current version features support for ARM Cortex-M4 and Cortex-M7 cores.

## Key features
* FOS uses hardware PSP for core stack and threads stack. So thread stack does`n store context in a case of interrupts that decrease necessary thread stack size.
* FOS works int hte main loop and does`t block it. So it is easy to add FOS to already existing project.
* FOS can use extremely low RAM and FLASH down to 16 kbytes code and 16 kbytes data.
* To create a thread in is needed to call only one function without preliminary set of the compilator.
* FOS support statically allocated rum by compiler and automatically distributes it between system objects and threads memory.
* Memory for system objects are separated from threads memory
* All the threads have its own stack and local heap protected from the leak.
* All OS memory for threads and system objects are protected by CRC
* FOS includes file system and objects for writing to the files via the system thread. Writing is allowded from the threeads, main loop and ISR.
* Interaction between threads and OS is supported by API via protected system calls.
* FOS support thread synchronization by: strong binary semaphores, strong counting semaphores, mutexes, critical code sections.
* FOS support protected data transferring via: thread notes, queues.

## OS supports
* ARM Cortex-M4 with FPU
* ARM Cortex-M7 with FPU (ICache can be Enabled)
* IAR ARM (with maximum speed optimization exept "Common subexpression elimination" and "Instruction scheduling")
* ARM GCC (with optimization O3)

### Test information
* OS has been tested on some heavy brojects and next platforms:
* STM32H745ZIT6 dual core at NUCLEO-H745ZI board (two separeted FOS at M7 and M4 cores)
* STM32H743ZIT6 at NUCLEO-H743ZI board
* STM32F722RET6 at custom board
* STM32F401CCU6 at BlackPill board

## OS parts
1. API - API for user threads
2. Kernel - implementation of the OS kernel 
3. File - user API and file objects of the file system
4. Mem - heap implementation for system and threads 
5. Platform - prototype platform-specific functions
6. Run - OS startup functions
7. Sync - objects for thread synchronization
8. System - low level OS functions
9. Thread - system threads and scheduler
10. Data - objects for transferring data between threads 

The file `fos_conf.h ` in the root folder contains all OS configuration.

All OS parts are located in the `/src` folder. All paths are relative to the `/src` folder.

## OS dependencies
FOS works with support libs [FosSupportLibs](https://github.com/kuzishchin/FosSupportLibs).



### Detail information
Detail information can be found at the [WiKI](https://github.com/kuzishchin/FOS/wiki)

### Versions history
[Versions](Versions.md)