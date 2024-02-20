/**************************************************************************//**
 * @file      fos_api.c
 * @brief     API of OS for user applications. Source file.
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


#include "API/fos_api.h"


/*
 * Уступить другому процессу
 * Потокобезопасно, вызывать из процесса который будет уступать
 * Не вызывать вне потоков
 */
void API_FOS_Yield()
{
	SYS_FOS_Yield();
}


/*
 * Усыпить текущий процесс
 * Потокобезопасно, вызывать из усыпляемого потока
 * Не вызывать вне потоков
 * time - время усыпления в мс, если FOS_INF_TIME - то бессрочно
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_Sleep(uint32_t time)
{
	return SYS_FOS_Sleep(time);
}


/*
 * Взять бинарный семафор
 * Потокобезопасно, вызывать из потока берущего семафор
 * Не вызывать вне потоков
 * semb - дескриптор бинарного семафора
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_SemBinaryTake(user_desc_t semb)
{
	return SYS_FOS_SemBinaryTake(semb);
}


/*
 * Дать бинарный семафор
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * semb - указатель на бинарный семафор
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_SemBinaryGive(user_desc_t semb)
{
	return SYS_FOS_SemBinaryGive(semb);
}


/*
 * Создать поток
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * user_init - настройки создаваемого потока
 * Возвращает пользовательский дескриптор созданного потока или FOS_WRONG_USER_DESC в случае ошибки
 */
user_desc_t API_FOS_CreateThread(fos_thread_user_init_t *user_init)
{
	return SYS_FOS_CreateThread(user_init);
}


/*
 * Создать поток с настройками размеров кучи и стека по умолчанию
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * name_ptr - имя потока
 * ep       - точка входа
 * priotity - приоритет
 * Возвращает пользовательский дескриптор созданного потока или FOS_WRONG_USER_DESC в случае ошибки
 */
user_desc_t API_FOS_CreateThreadDef(char* name_ptr, user_thread_ep_t ep, uint8_t priotity)
{
	fos_thread_user_init_t user_init = {0};
	user_init.user_thread_ep = ep;
	user_init.priotity   = priotity;		                       // LOW
	user_init.stack_size = FOS_DEF_THR_STACK_SIZE;
	user_init.heap_size  = FOS_DEF_THR_HEAP_SIZE;
	user_init.name_ptr   = name_ptr;
	return API_FOS_CreateThread(&user_init);
}


/*
 * Создать бинарный семафор
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * init_state - начальное состояние семафора
 * Возвращает пользовательский дескриптор созданного потока или FOS_WRONG_USER_DESC в случае ошибки
 */
user_desc_t API_FOS_CreateSemBinary(fos_semb_state_t init_state)
{
	return SYS_FOS_CreateSemBinary(init_state);
}


/**
 * Запустить поток с дескирпотором
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * desc - дескриптор запускаемого потока
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_RunDesc(user_desc_t desc)
{
	return SYS_FOS_RunDesc(desc);
}


/**
 * Завершить текущий поток
 * Потокобезопасно, вызывать из потока требующего завершение
 * Не вызывать вне потоков
 * terminate_code - код завершения (0 - штатное завершение, >0 - пользовательский любой код ошибки)
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_Terminate(uint8_t terminate_code)
{
	return SYS_FOS_Terminate((int32_t)terminate_code);
}


/**
 * Завершить поток с указанным дескриптором
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * desc - дескриптор завершаемого потока
 * terminate_code - код завершения (0 - штатное завершение, >0 - пользовательский любой код ошибки)
 * Возвращает статус выполнения
 */
fos_ret_t API_FOS_TerminateDesc(user_desc_t desc, uint8_t terminate_code)
{
	return SYS_FOS_TerminateDesc(desc, -terminate_code);
}


/**
 * Зафиксировать ошибку
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * err_code - код ошибки
 * err_string - строка с описанием ошибки
 */
void API_FOS_ErrorSet(uint32_t err_code, char* err_string)
{
	fos_err_t err = {0};
	err.err_code = err_code;
	err.ext_str_ptr = err_string;
	SYS_FOS_ErrorSet(&err);
}


/*
 * Смотировать файловую систему
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * dev_num - номер устройсвта для монитрования
 * Возвращает статус выполнения
 */
file_err_t API_File_Mount(uint8_t dev_num)
{
	SYS_File_Mount(dev_num);
	return File_WaitingMount(dev_num, FOS_FSDEV_TIMEOUT_MS);
}


/*
 * Размонитровать файловую систему
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * dev_num - номер устройсвта для размонтирования
 * Возвращает статус выполнения
 */
file_err_t API_File_Unmount(uint8_t dev_num)
{
	SYS_File_Unmount(dev_num);
	return FILE_ERR__NO;
}


// прототип создания объект записи
// использует функцию ядра, в заголовочный файн не выведена
__weak fwriter_t* USER_CreateFWriter(uint16_t write_buf_len)
{
	return NULL;
}


/*
 * Создать объект записи
 * Потокобезопасно, вызывать из потока или основного цикла
 * Не вызывать из прерывания
 * write_buf_len - размер буфера на запись
 * Возвращает указатель на объект записи
 */
fwriter_t* API_File_CreateFWriter(uint16_t write_buf_len)
{
	fwriter_t* p = NULL;
	uint32_t s;

	ENTER_CRITICAL(s);
	p = USER_CreateFWriter(write_buf_len);
	LEAVE_CRITICAL(s);

	return p;
}








