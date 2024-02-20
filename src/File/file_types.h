/**************************************************************************//**
 * @file      file_types.h
 * @brief     File types declarations. Header file.
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

#ifndef APPLICATION_FOS_FILE_FILE_TYPES_H_
#define APPLICATION_FOS_FILE_FILE_TYPES_H_

#include "fos_types.h"

#include "CircBuf/CircBuf.h"

#ifdef FOS_USE_FATFS
#include "fatfs.h"
#endif

#define FILE_CRITICAL_ERR_MASK 0x80


// фалги монтирования
typedef enum
{
	FILE__NOT_MOUNT = 0,      // не смонтирована
	FILE__MOUNTED,            // смонтирована
	FILE__MOUNT_ERROR,        // ошибка монтирования

} fmount_flag_t;


// режимы
typedef enum
{
	FILE_MODE__OFF = 0,    // выключен: блокируются все пользовательские опреации записи, файл закрывается, дозаписи нет
	FILE_MODE__IDDLE,      // холостой ход: блокируются все пользовательские опреации записи, файлы открыт, происходит дозапись c автопереходом в OFF
	FILE_MODE__ON,         // включён: все разблокировано
	FILE_MODE__TEST,       // тестовые операции: блокируются все пользовательские опреации

} file_mode_t;


// состояния
typedef enum
{
	FILE_STATE__NO_INIT = 0, // не инициализировано, только режим FILE_MODE__OFF
	FILE_STATE__CLOSED,      // файл закрыт: стремится сиюда из FILE_MODE__OFF
	FILE_STATE__OPENED,      // файл открыт: стремится сюда из всех режимов кроме FILE_MODE__OFF

} file_state_t;


// ошибки
typedef enum
{
	FILE_ERR__NO = 0,              // нет ошибок
	FILE_ERR__NO_DEF_FS = 0x81,    // не выбрана файловая система
	FILE_ERR__WRONG_DEV,           // неверное устройство
	FILE_ERR__MOUNT,               // ошибка монитрования
	FILE_ERR__UNMOUNT,             // ошибка размонтирования
	FILE_ERR__OPEN_DEV,            // ошибка открытия устройства
	FILE_ERR__MAKE_DIR,            // ошибка создания директории
	FILE_ERR__OPEN_DIR,            // ошибка открытия директории
	FILE_ERR__CREATE_FILE,         // ошибка создания файла

	FILE_ERR__OPEN_FILE,           // ошибка открытия файла
	FILE_ERR__WRITE,               // ошибка записи
	FILE_ERR__WRONG_CPY_LEN,       // невреная длина при копировании
	FILE_ERR__DISC_OVF,            // ндиск переполнен
	FILE_ERR__SYNC,                // ошибка при синхронизации
	FILE_ERR__LSEEK,               // ошибка позиционирования
	FILE_ERR__TRUNC,               // ошибка обрезки
	FILE_ERR__CLOSE,               // ошибка закрытия

} file_err_t;


// инициализация
typedef struct
{
	circ_buf_init_t cbuf_init;     // кольцевой буфер очереди на запись

} file_init_t;


// отладка
typedef struct
{
	uint32_t period;               // номер текущего тестового перидла
	uint32_t last_period;          // номер прошлого тестовго периода (секунду назад)
	uint32_t ts;                   // метка времени, мс
	uint32_t test_wr_s;            // число периодов в секунду

} file_dbg_t;


// описатели файловой системы
typedef struct
{
#ifdef FOS_USE_FATFS
	FIL     file;                  // файл
	FRESULT fres;                  // статус
#else
	uint8_t t;
#endif
} fsys_t;


// настройки
// полный путь к файлу имееет вид: dev_num:/folder_1/folder_2/folder_3/folder_num/subdir_name/file.txt
// dev_num - номпер диска
// folder_num - номерная папка состоящая из числа от 1 до 10000
typedef struct
{
	const char* file_path;        // путь к файлу без  dev_num, folder_num и subdir_name   /folder_1/folder_2/folder_3/file.txt
	const char* subdir_name;      // ипя поддиректории (не используется если NULL)

	uint8_t  dev_num;             // номер устройства
	uint8_t  folder_num;          // номер номерной папки
	fos_sw_t use_fnum;            // использовать папку с номером
	fos_sw_t gen_fnum;            // сгенерировать папку с уникальным номером в пути file_path

} fsys_path_t;


// переменные
typedef struct
{
	volatile fos_sw_t     fsync_flag; // флаг синхронизации файла
	volatile file_err_t   err;        // ошибки

	circ_buf_t            cbuf;       // кольцевой буфер данных на запись
	fsys_t                fsys;       // описатели файловой системы

//	uint8_t write_buf[FILE_WRITE_BUF_LEN];   // буфер на запись

} fwriter_var_t;


// структура для записи данных
typedef struct
{
	volatile file_mode_t   mode;     // режим
	volatile file_state_t  state;    // состояние

	fwriter_var_t var;      // переменные
	file_dbg_t    dbg;      // отладка
	fsys_path_t   path;     // путь к файлу

} fwriter_t;


typedef fwriter_t* fwriter_ptr;


#endif /* APPLICATION_FOS_FILE_FILE_TYPES_H_ */






