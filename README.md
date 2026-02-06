# Fly Operating System (FOS)

## Information
FOS is a Real Time Operating System (RTOS) for MCU-controlled robotic systems.
Current version features support for ARM Cortex-M4 and Cortex-M7 cores.

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

### Test information
OS has been tested with stm32f4, stm32f7 and stm32h7 MCUs, and IAR and GCC compillers.

### Detail information
Detail information can be found at the [WiKI](https://github.com/kuzishchin/FOS/wiki)

### Versions history
[Versions](Versions.md)