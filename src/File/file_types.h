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


// mounting flags
typedef enum
{
	FILE__NOT_MOUNT = 0,      // not mounted
	FILE__MOUNTED,            // mounted
	FILE__MOUNT_ERROR,        // mounting error

} fmount_flag_t;


// writer modes
typedef enum
{
	FILE_MODE__OFF = 0,    // off: all user defined write operations are blocked, file is closed, appending writing unavailable
	FILE_MODE__IDDLE,      // idle: all user defined write operations are blocked, file is open, appending writing switches to 'OFF' state after completion
	FILE_MODE__ON,         // on: everything available (unblocked)
	FILE_MODE__TEST,       // test operations: all user defined write operations are blocked

} file_mode_t;


// states
typedef enum
{
	FILE_STATE__NO_INIT = 0, // not initialized, only 'FILE_MODE__OFF' mode available
	FILE_STATE__CLOSED,      // file is closed: state is switched here from 'FILE_MODE__OFF' mode
	FILE_STATE__OPENED,      // file is open: state is switched here from all modes except 'FILE_MODE__OFF'

} file_state_t;


// errors
typedef enum
{
	FILE_ERR__NO = 0,              // no errors
	FILE_ERR__NO_DEF_FS = 0x81,    // file system is not specified
	FILE_ERR__WRONG_DEV,           // wrong device
	FILE_ERR__MOUNT,               // file system mounting error
	FILE_ERR__UNMOUNT,             // file system unmounting error
	FILE_ERR__OPEN_DEV,            // device opening error
	FILE_ERR__MAKE_DIR,            // directory creation error
	FILE_ERR__OPEN_DIR,            // directory opening error
	FILE_ERR__CREATE_FILE,         // file creation error

	FILE_ERR__OPEN_FILE,           // file opening error
	FILE_ERR__WRITE,               // file write error
	FILE_ERR__WRONG_CPY_LEN,       // wrong copied length
	FILE_ERR__DISC_OVF,            // file storage overflow
	FILE_ERR__SYNC,                // synchronization error
	FILE_ERR__LSEEK,               // positioning error
	FILE_ERR__TRUNC,               // truncation error
	FILE_ERR__CLOSE,               // file close error

} file_err_t;


// initialization
typedef struct
{
	circ_buf_init_t cbuf_init;     // circular buffer of write queue

} file_init_t;


// debugging
typedef struct
{
	uint32_t period;               // number of current test period
	uint32_t last_period;          // number of previous test period (1s earlier)
	uint32_t ts;                   // time stamp, milliseconds
	uint32_t test_wr_s;            // periods per second rate

} file_dbg_t;


// file system descriptors
typedef struct
{
#ifdef FOS_USE_FATFS
	FIL     file;                  // file
	FRESULT fres;                  // status
#else
	uint8_t t;
#endif
} fsys_t;


// settings
// full file path looks like:   dev_num:/folder_1/folder_2/folder_3/folder_num/subdir_name/file.txt
// dev_num - number of storage
// folder_num - numbered folder that consists of a number in the range from 1 to 10000
typedef struct
{
	const char* file_path;        // file path without dev_num, folder_num and subdir_name   /folder_1/folder_2/folder_3/file.txt
	const char* subdir_name;      // subdirectory name (not used if NULL)

	uint8_t  dev_num;             // device number
	uint8_t  folder_num;          // number of numbered folder
	fos_sw_t use_fnum;            // use folder with number
	fos_sw_t gen_fnum;            // generate folder with unique number in file_path

} fsys_path_t;


// variables
typedef struct
{
	volatile fos_sw_t     fsync_flag; // file synchronization flag
	volatile file_err_t   err;        // errors

	circ_buf_t            cbuf;       // circular buffer for written data
	fsys_t                fsys;       // descriptors of the file system

} fwriter_var_t;


// data write structure
typedef struct
{
	volatile file_mode_t   mode;     // write mode
	volatile file_state_t  state;    // write state

	fwriter_var_t var;      // variables
	file_dbg_t    dbg;      // debugging
	fsys_path_t   path;     // file path

} fwriter_t;


typedef fwriter_t* fwriter_ptr;


#endif /* APPLICATION_FOS_FILE_FILE_TYPES_H_ */






