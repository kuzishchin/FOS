/**************************************************************************//**
 * @file      fos_api.h
 * @brief     API of OS for user applications. Header file.
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

#ifndef APPLICATION_FOS_API_FOS_API_H_
#define APPLICATION_FOS_API_FOS_API_H_


#include "System/fos_system.h"
#include "FIle/file_types.h"

/*
 * Уступить другому процессу
 * Потокобезопасно, вызывать из процесса который будет уступать
 * Не вызывать вне потоков
 */
void API_FOS_Yield();

/*
 * Усыпить текущий процесс
 * Потокобезопасно, вызывать из усыпляемого потока
 * Не вызывать вне потоков
 * time - время усыпления в мс, если FOS_INF_TIME - то бессрочно
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_Sleep(uint32_t time);

/*
 * Взять бинарный семафор
 * Потокобезопасно, вызывать из потока берущего семафор
 * Не вызывать вне потоков
 * semb - дескриптор бинарного семафора
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_SemBinaryTake(user_desc_t semb);

/*
 * Дать бинарный семафор
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * semb - указатель на бинарный семафор
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_SemBinaryGive(user_desc_t semb);

/*
 * Создать поток
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * user_init - настройки создаваемого потока
 * Возвращает пользовательский дескриптор созданного потока или FOS_WRONG_USER_DESC в случае ошибки
 */
user_desc_t API_FOS_CreateThread(fos_thread_user_init_t *user_init);

/*
 * Создать поток с настройками размеров кучи и стека по умолчанию
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * name_ptr - имя потока
 * ep       - точка входа
 * priotity - приоритет
 * Возвращает пользовательский дескриптор созданного потока или FOS_WRONG_USER_DESC в случае ошибки
 */
user_desc_t API_FOS_CreateThreadDef(char* name_ptr, user_thread_ep_t ep, uint8_t priotity);

/*
 * Создать бинарный семафор
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * init_state - начальное состояние семафора
 * Возвращает пользовательский дескриптор созданного потока или FOS_WRONG_USER_DESC в случае ошибки
 */
user_desc_t API_FOS_CreateSemBinary(fos_semb_state_t init_state);

/**
 * Запустить поток с дескирпотором
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * desc - дескриптор запускаемого потока
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_RunDesc(user_desc_t desc);


/**
 * Завершить текущий поток
 * Потокобезопасно, вызывать из потока требующего завершение
 * Не вызывать вне потоков
 * terminate_code - код завершения (0 - штатное завершение, >0 - пользовательский любой код ошибки)
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_Terminate(uint8_t terminate_code);


/**
 * Завершить поток с указанным дескриптором
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * desc - дескриптор завершаемого потока
 * terminate_code - код завершения (0 - штатное завершение, >0 - пользовательский любой код ошибки)
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_TerminateDesc(user_desc_t desc, uint8_t terminate_code);


/**
 * Зафиксировать ошибку
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * err_code - код ошибки
 * err_string - строка с описанием ошибки
 */
void API_FOS_ErrorSet(uint32_t err_code, char* err_string);


/*
 * Смотировать файловую систему
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * dev_num - номер устройсвта для монитрования
 * Возвращает статус выполнения
 */
file_err_t API_File_Mount(uint8_t dev_num);


/*
 * Размонитровать файловую систему
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * dev_num - номер устройсвта для размонтирования
 * Возвращает статус выполнения
 */
file_err_t API_File_Unmount(uint8_t dev_num);


/*
 * Создать объект записи
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * write_buf_len - размер буфера на запись
 * Возвращает указатель на объект записи
 */
fwriter_t* API_File_CreateFWriter(uint16_t write_buf_len);



#endif /* APPLICATION_FOS_API_FOS_API_H_ */











