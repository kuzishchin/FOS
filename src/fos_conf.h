/**************************************************************************//**
 * @file      fos_conf.h
 * @brief     Configuration file of OS. Header file.
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

#ifndef APPLICATION_FOS_FOS_CONF_H_
#define APPLICATION_FOS_FOS_CONF_H_



#define FOS_MAX_THR_CNT        32          // максимальное число потоков
#define FOS_SEM_BIN_CNT        32          // максимальное число бинарных светофоров
#define FOS_FWRITER_CNT        32          // максимальное число объктов записи
#define FOS_SYS_CALL_CNT       32          // максимальное число системных вызовов
#define FOS_PRIORITY_CNT       8           // максимальное число приоритетов (0 - наивысший)
#define FOS_THR_NAME_LEN       16          // длина имени потока
#define FOS_MAX_STR_ERR_LEN    32          // максимальная длина строки описания ошибки

#define FOS_USE_FATFS                      // использовать FatFs
#define FOS_MAX_FS_DEV         2           // максимальное число устройств
#define FOS_FILEWR_MAX_BUF_LEN 2048        // максимальный размер буфера на запись, байт
#define FOS_FS_TIMEOUT_MS      2000        // таймаут при работе с файловой системой, мс
#define FOS_FSDEV_TIMEOUT_MS   5000        // таймаут при монитровании устройсва, мс

#define FOS_HARD_FAULT_CALL_ID 0xFFFF      // номер функции вызова hard fault

#define FOS_CORE_STACK_SIZE    0x800       // размер стека ядра (минимум нужно 0х500)
#define FOS_CORE_HEAP_SIZE     0x2000      // размер кучи ядра
#define FOS_THREADS_HEAP_SIZE  0x18000     // размер кучи для всех процессов

#define FOS_DEF_THR_STACK_SIZE 0x400       // размер стека потока по умолчанию
#define FOS_DEF_THR_HEAP_SIZE  0x400       // размер кучи потока по умолчанию

#define FOS_ERROR_STACK_WML    40.0f       // генрация ошибки при превышении этого уровня заполнености стека в %



#endif /* APPLICATION_FOS_FOS_CONF_H_ */



