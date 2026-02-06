/**************************************************************************//**
 * @file      file_sys.c
 * @brief     Abstraction layer between the file system and OS. Source file.
 * @version   V1.0.01
 * @date      06.02.2026
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


#include "File/Sys/file_sys.h"
#include "Libs/sl_string.h"
#include "Platform/sl_platform.h"


#ifdef FOS_USE_FATFS
static FATFS fatfs[FOS_MAX_FS_DEV];
#endif

static fmount_state_t fmount_states[FOS_MAX_FS_DEV];    // состояния устройств


// обработка состояния устройства
static void Private_File_MountProc(uint8_t dev);

// смотнитровать файловую систему
static file_err_t Private_Mount(uint8_t dev_num);

// размонтировать файловую систему
static file_err_t Private_Unmount(uint8_t dev_num);


// уступить другому процессу
// реализация через системный вызов
__weak fos_ret_t SYS_FOS_Sleep(uint32_t time)
{
	return FOS__OK;
}


// смотнитровать файловую систему
void File_Mount(uint8_t dev_num)
{
	if(dev_num >= FOS_MAX_FS_DEV)
		return;

	fmount_states[dev_num].mnt_sw = FOS__ENABLE;
}


// размонтировать файловую систему
void File_Unmount(uint8_t dev_num)
{
	if(dev_num >= FOS_MAX_FS_DEV)
		return;

	fmount_states[dev_num].mnt_sw = FOS__DISABLE;
}


// открываем файл для записи писалки
file_err_t Private_FWriter_Open(fsys_t* fsys, fsys_path_t* path)
{
#ifdef FOS_USE_FATFS

	uint8_t ret;

	char file_path[255] = {0};                      // путь к файлу
	char dev[10] = {0};                             // имя устройства
	char num_dir[10] = {0};                         // имя номерной папки
	char dir_name[255] = {0};                       // имя директории/файла

	strcpy(file_path, path->file_path);             // копируем путь к файлу
	user_itoa(path->dev_num, 10, 1, dev, 0);        // преобразуем номер в строку
	strcat(dev, ":/\0");



	// выбираем файловую систему
	fsys->fres = f_chdrive((TCHAR*)dev);
	if(fsys->fres != FR_OK)
		return FILE_ERR__OPEN_DEV;

	// сброс директории
	fsys->fres = f_chdir((TCHAR*)dev);
	if(fsys->fres != FR_OK)
		return FILE_ERR__OPEN_DIR;

	// создаём директории по папкам
	while(1)
	{
		// выделяем из пути очередное имя
		ret = parse_first_node_name((char*)file_path, dir_name, (char*)file_path, 255);
		if(ret != 3)          // если больше имен нет
			break;            // выходим

		// создаем директорию (пропускаем шаг, если она сушествует)
		fsys->fres = f_mkdir((TCHAR*)dir_name);
		if((fsys->fres != FR_OK) && (fsys->fres != FR_EXIST))
			return FILE_ERR__MAKE_DIR;

		// заходим в неё
		fsys->fres = f_chdir((TCHAR*)dir_name);
		if(fsys->fres != FR_OK)
			return FILE_ERR__OPEN_DIR;
	}

	// проверяем на ошибки
	if((ret == 0) || (ret == 4))
		return FILE_ERR__MAKE_DIR;

	/*
	 * Создаем номерную папку
	 */
	if(path->use_fnum == FOS__ENABLE)                // если она используется
	{
		if(path->gen_fnum != FOS__ENABLE)            // если создавать уникальный номер не тнужно
		{
			user_itoa(path->folder_num, 10, 1, num_dir, 0);        // преобразуем номер в строку
			fsys->fres = f_mkdir((TCHAR*)num_dir);                 // создаём нумерную папку
			if((fsys->fres != FR_OK) && (fsys->fres != FR_EXIST))
				return FILE_ERR__MAKE_DIR;
		}else{                                       // если нудна папка с уникальным номер

			for(uint16_t i = 1; i < 10000; i++)      // перебираем номера папок
			{
				path->folder_num = i;                // сохраняем номер
				user_itoa(path->folder_num, 10, 1, num_dir, 0);    // преобразуем номер в строку

				// создаем измерительную директорию
				fsys->fres = f_mkdir((TCHAR*)num_dir);
				if(fsys->fres == FR_EXIST){          // если существет
					continue;                        // повторяем цикл заново с новым номером
				}else{
					if(fsys->fres != FR_OK)          // если ошибка
						return FILE_ERR__MAKE_DIR;   // выходим из функции
					else                             // иначе
						break;                       // выходим из программы
				}
			}
		}

		// заходим в неё
		fsys->fres = f_chdir((TCHAR*)num_dir);
		if(fsys->fres != FR_OK)
			return FILE_ERR__OPEN_DIR;
	}

	// создаем директорию subdir_name
	if(path->subdir_name != NULL)         // если она используется
	{
		// создаем директорию (пропускаем шаг, если она сушествует)
		fsys->fres = f_mkdir((TCHAR*)path->subdir_name);
		if((fsys->fres != FR_OK) && (fsys->fres != FR_EXIST))
			return FILE_ERR__MAKE_DIR;

		// заходим в неё
		fsys->fres = f_chdir((TCHAR*)path->subdir_name);
		if(fsys->fres != FR_OK)
			return FILE_ERR__OPEN_DIR;
	}

	// если имени файла в пути нет
	if(ret == 2)
		return FILE_ERR__NO;

	// создаем файл
	fsys->fres = f_open(&fsys->file, (TCHAR*)dir_name, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
	if(fsys->fres != FR_OK)
		return FILE_ERR__CREATE_FILE;

	return FILE_ERR__NO;

#else
	return FILE_ERR__NO_DEF_FS;
#endif
}


// закрываем файл писалики
file_err_t Private_FWriter_Close(fsys_t* fsys)
{

#ifdef FOS_USE_FATFS

	fsys->fres = f_close(&fsys->file);
	if(fsys->fres != FR_OK)
		return FILE_ERR__CLOSE;

	return FILE_ERR__NO;

#else
	return FILE_ERR__NO_DEF_FS;
#endif
}


// пишем в файл писалкой
file_err_t Private_FWriter_Write(fsys_t* fsys, uint8_t* data, uint32_t data_len)
{
#ifdef FOS_USE_FATFS

	uint32_t fwriten;

	fsys->fres = f_write(&fsys->file, data, data_len, (UINT*)&fwriten);
	if(fsys->fres != FR_OK)
		return FILE_ERR__WRITE;

	if(data_len != fwriten)
		return FILE_ERR__DISC_OVF;

	return FILE_ERR__NO;

#else
	return FILE_ERR__NO_DEF_FS;
#endif
}


// синхронизуем файл писалкой
file_err_t Private_FWriter_Sync(fsys_t* fsys)
{
#ifdef FOS_USE_FATFS

	fsys->fres = f_sync(&fsys->file);
	if(fsys->fres != FR_OK)
		return FILE_ERR__SYNC;

	return FILE_ERR__NO;

#else
	return FILE_ERR__NO_DEF_FS;
#endif
}


// ожидание монтирования
file_err_t File_WaitingMount(uint8_t dev, uint32_t timeout_ms)
{
	const uint32_t sleep = 10;

	if(dev >= FOS_MAX_FS_DEV)
		return FILE_ERR__MOUNT;

	uint32_t timeout  = SL_GetTick() + timeout_ms;
	fmount_state_t *s = &fmount_states[dev];

	while(s->mnt_fl != FILE__MOUNTED)
	{
		if(SL_GetTick() > timeout)
			return FILE_ERR__MOUNT;

		SYS_FOS_Sleep(sleep);
	}

	return FILE_ERR__NO;
}


// обработчик состояния всех устройств
void File_MountProc()
{
	for(uint8_t i = 0; i < FOS_MAX_FS_DEV; i++)
		Private_File_MountProc(i);
}





// обработка состояния устройства
static void Private_File_MountProc(uint8_t dev)
{
	fmount_state_t *s = &fmount_states[dev];

	switch(s->mnt_sw)
	{
	case FOS__DISABLE:

		if(s->mnt_fl == FILE__MOUNTED)
		{
			if(Private_Unmount(dev) != FILE_ERR__NO)
				s->mnt_fl = FILE__MOUNT_ERROR;
			else
				s->mnt_fl = FILE__NOT_MOUNT;
		}

		break;

	case FOS__ENABLE:

		if(s->mnt_fl != FILE__MOUNTED)
		{
			if(Private_Mount(dev) != FILE_ERR__NO){
				s->mnt_fl = FILE__MOUNT_ERROR;
				s->mnt_sw = FOS__DISABLE;
			} else
				s->mnt_fl = FILE__MOUNTED;
		}

		break;
	}
}


// смотнитровать файловую систему
static file_err_t Private_Mount(uint8_t dev_num)
{
#ifdef FOS_USE_FATFS

	if(dev_num >= FOS_MAX_FS_DEV)
		return FILE_ERR__WRONG_DEV;

	char dev[10] = {0};

	user_itoa(dev_num, 10, 1, dev, 0);        // преобразуем номер в строку
	strcat(dev, ":/\0");

	// монтируем файловую систему
	FRESULT fres = f_mount(&fatfs[dev_num], dev, 1);
	if(fres != FR_OK)
		return FILE_ERR__MOUNT;

	return FILE_ERR__NO;

#else
	return FILE_ERR__NO_DEF_FS;
#endif
}


// размонтировать файловую систему
static file_err_t Private_Unmount(uint8_t dev_num)
{
#ifdef FOS_USE_FATFS

	if(dev_num >= FOS_MAX_FS_DEV)
		return FILE_ERR__WRONG_DEV;

	char dev[10] = {0};

	user_itoa(dev_num, 10, 1, dev, 0);        // преобразуем номер в строку
	strcat(dev, ":/\0");

	// размонтируем файловую систему
	FRESULT fres  = f_mount(NULL, dev, 1);
	if(fres != FR_OK)
		return FILE_ERR__UNMOUNT;

	return FILE_ERR__NO;

#else
	return FILE_ERR__NO_DEF_FS;
#endif
}













