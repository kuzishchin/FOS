/**************************************************************************//**
 * @file      fwriter_sys.h
 * @brief     The driver of file writing handling. Header file.
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

#ifndef APPLICATION_FOS_FILE_SYS_FWRITER_SYS_H_
#define APPLICATION_FOS_FILE_SYS_FWRITER_SYS_H_


#include "File/Sys/file_sys.h"


// инициализация
fos_ret_t FWriter_Init(fwriter_t *p, file_init_t *init);

// системный обработчик
void FWriter_SysProc(fwriter_t *p);

// получить длину данных которые надо записать
uint16_t FWriter_GetDataToWrite(fwriter_t *p);

// обработчик ошибок
static void Private_FWriter_ErrorProc(fwriter_t *p);

// обработчик режима OFF
static void Private_FWriter_ProcOffMode(fwriter_t *p);

// обработчик режима IDDLE и режима ON
static void Private_FWriter_IddleAndOnMode(fwriter_t *p);

// обработчик режима TEST
static void Private_FWriter_TestMode(fwriter_t *p);

// записать данные из буфера
static uint16_t Private_FWriter_BufWrite(fwriter_t *p);

// тестовая запись в файл
static void Private_FWriter_TestWrite(fwriter_t *p);



#endif /* APPLICATION_FOS_FILE_SYS_FWRITER_SYS_H_ */




