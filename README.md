# Fly Operating System (FOS)

### Information
FOS is a Real Time Operating System (RTOS) for MCU-controlled robotic systems. 
Current version features support for ARM Cortex-M7 core.

### OS parts
1. API - API for user threads
2. Kernel - implementation of the OS kernel 
3. File - user API and file objects of the file system
4. Mem - heap implementation for system and threads 
5. Platform - prototype platform-specific functions
6. Run - OS startup functions
7. Sync - objects for thread synchronization
8. System - low level OS functions
9. Thread - system threads and scheduler

The file `fos_conf.h ` in the root folder contains all OS configuration.

All OS parts are located in the `/src` folder. All paths are relative to the `/src` folder.

### OS dependencies
FOS works with support libs [FosSupportLibs] which can be found at the link below.
(https://github.com/kuzishchin/FosSupportLibs).

### Test information
OS has been tested with STM32F722 MCU, compillers IAR ARM v8.22.1.15700 and GCC v7-2018-q2-update. Read `Version.txt` for more information.

### Detail information
Detail information can be found at the link below.
(https://github.com/kuzishchin/FOS/wiki)