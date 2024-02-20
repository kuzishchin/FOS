# Fly Operating System (FOS)

### Information
FOS is a Real Time Operation System (RTOS) for robotic systems built at MCU. It supports ARM M7 and tested at stm32f722.


### OS parts
1. API - API for user threads
2. Core - the kerne realization of OS
3. File - users API and file objects of file system
4. Mem - the system and threads heap realization
5. Platform - the prototypes of platform dependent functions
6. Run - the starting OS fucntions
7. Sync - the thread synchronization objects
8. System - the low level OS functions
9. Thread - system threads and the scheduler

All OS parts can be found in the folfer `/src`. All paths are relative to the folfer `/src`.

### OS dependencies
To work this OS needs support libs that can be found the link below.
You may be using [FosSupportLibs](https://github.com/kuzishchin/FosSupportLibs).