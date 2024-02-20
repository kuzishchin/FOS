/**************************************************************************//**
 * @file      fwriter.h
 * @brief     API for the file writer object. Header file.
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

#ifndef APPLICATION_FOS_FILE_FWRITER_H_
#define APPLICATION_FOS_FILE_FWRITER_H_


#include "File/Sys/fwriter_sys.h"



/*
 * Открыть файл
 * Вызывать из потока владельца
 * p    - указатель на объект записи
 * path - путь к файлу
 * Возвращает результат выполнения
 */
fos_ret_t API_FWriter_Open(fwriter_t *p, fsys_path_t *path);


/*
 * Закрыть файл
 * Вызывать из потока владельца
 * p - указатель на объект записи
 * Возвращает результат выполнения
 */
fos_ret_t API_FWriter_Close(fwriter_t *p);


/*
 * Начать тестовую запись
 * Вызывать из потока владельца
 * p - указатель на объект записи
 * Возвращает результат выполнения
 */
fos_ret_t API_FWriter_StartWriteTest(fwriter_t *p);


/*
 * Записать данные
 * Потокобезопасно. Вызывать из любого потока, основного цикла или прерывания
 * p    - указатель на объект записи
 * data - указатель на записываемые данные
 * data_len - длина записываемых данных
 * Примесаение: записываемые данные копируются целиком
 * Возвращает результат выполнения
 */
fos_ret_t API_FWriter_Write(fwriter_t *p, uint8_t* data, uint32_t data_len);


/*
 * Синхронизовать данные
 * Вызывать из потока владельца
 * p - указатель на объект записи
 * Возвращает результат выполнения
 */
fos_ret_t API_FWriter_FSync(fwriter_t *p);


/*
 * Получить состояние файла
 * p - указатель на объект записи
 * Возвращает состояние файла
 */
file_state_t API_FWriter_GetFileState(fwriter_t *p);



// смена режима работы
static fos_ret_t FWriter_SetMode(fwriter_t *p, file_mode_t mode);

// ожидание состояния
static fos_ret_t FWriter_WaitingState(fwriter_t *p, file_state_t state, uint32_t timeout_ms);

// установить путь
static fos_ret_t FWriter_SetPath(fwriter_t *p, fsys_path_t *path);



#endif /* APPLICATION_FOS_FILE_FWRITER_H_ */









