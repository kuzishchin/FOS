# FOS versions

## FOS v1.0.2 build 03 06.02.2026 api-1
* Added release binary semaphore for ISR
* Added timeout event for binary semaphore
* Add version OS in the string
* Fixed some bugs
* Added the support of GCC compiller with optimization
* Added counting semaphore with functionality as binary semaphore
* Added queue for 32 bit data pithes

## FOS v0.14 built 04.04.2024

* Added dynamically allocated threads wich automatically delete after terminating 
* Added garbage collection for terminated threads 
* Added function wich deletes binary semaphore 
* Added Join thread method
* Added IsAlive thread method 

## FOS v0.13 built 18.03.24

* Added support of ARM Cortex-M4 core
* Added adjustable context swith time
* Fixed some bugs

## FOS v0.12 built 12.03.24

* System calls optimization
* OS config optimization

## FOS v0.11 built 27.02.24

* Added the support of GCC v7-2018-q2-update with no optimization

## FOS v0.10 built 14.02.24

* The first version of RTOS. It supports Arm Cotrex M7 only with FPU using.
* OS has been tested by IAR Compiller Arm 8.22.1.15700
* All optimization levels are supported exept "Common subexpression elimination" and "Instruction scheduling"
