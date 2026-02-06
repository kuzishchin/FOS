/**************************************************************************//**
 * @file      file_sys.h
 * @brief     Abstraction layer between the file system and OS. Header file.
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

#ifndef APPLICATION_FOS_FILE_SYS_FILE_SYS_H_
#define APPLICATION_FOS_FILE_SYS_FILE_SYS_H_


#include "File/file_types.h"


// структура для регистрации смонтированных FS
typedef struct
{
	fos_sw_t      mnt_sw;       // выключатель монтирования
	fmount_flag_t mnt_fl;       // состояние монтирования

} fmount_state_t;


// смотнитровать файловую систему
void File_Mount(uint8_t dev_num);

// размонтировать файловую систему
void File_Unmount(uint8_t dev_num);

// открываем файл для записи писалки
file_err_t Private_FWriter_Open(fsys_t* fsys, fsys_path_t* path);

// закрываем файл писалики
file_err_t Private_FWriter_Close(fsys_t* fsys);

// пишем в файл писалкой
file_err_t Private_FWriter_Write(fsys_t* fsys, uint8_t* data, uint32_t data_len);

// синхронизуем файл писалкой
file_err_t Private_FWriter_Sync(fsys_t* fsys);

// ожидание монтирования
file_err_t File_WaitingMount(uint8_t dev, uint32_t timeout_ms);

// обработчик состояния всех устройств
void File_MountProc();






#endif /* APPLICATION_FOS_FILE_SYS_FILE_SYS_H_ */









