/**************************************************************************//**
 * @file      fwriter.c
 * @brief     API for the file writer object. Source file.
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


#include "File/fwriter.h"
#include "Platform/sl_platform.h"
#include <string.h>


// change writer work mode
static fos_ret_t FWriter_SetMode(fwriter_t *p, file_mode_t mode);

// pending file state
static fos_ret_t FWriter_WaitingState(fwriter_t *p, file_state_t state, uint32_t timeout_ms);

// set path
static fos_ret_t FWriter_SetPath(fwriter_t *p, fsys_path_t *path);


// yield to another process
// implemented with system call, not indicated in the header file
__weak fos_ret_t SYS_FOS_Sleep(uint32_t time)
{
	return FOS__OK;
}


/*
 * Open file
 * Call from the owner thread
 * p    - pointer to the writer object
 * path - path to the file
 * Returns execution result
 */
fos_ret_t API_FWriter_Open(fwriter_t *p, fsys_path_t *path)
{
	if((p == NULL) || (path == NULL))
		return FOS__FAIL;

	if(p->state == FILE_STATE__NO_INIT)
		return FOS__FAIL;

	// reset errors
	p->var.err = FILE_ERR__NO;

	// specify file path
	if(FWriter_SetPath(p, path) != FOS__OK)
		return FOS__FAIL;

	// set mode
	if(FWriter_SetMode(p, FILE_MODE__ON) != FOS__OK)
		return FOS__FAIL;

	if(FWriter_WaitingState(p, FILE_STATE__OPENED, FOS_FS_TIMEOUT_MS) != FOS__OK)
		return FOS__FAIL;

	return FOS__OK;
}


/*
 * Close file
 * Call from the owner thread
 * p - pointer to the writer object
 * Returns execution result
 */
fos_ret_t API_FWriter_Close(fwriter_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	if(p->state == FILE_STATE__NO_INIT)
		return FOS__FAIL;

	// set mode
	if(FWriter_SetMode(p, FILE_MODE__IDDLE) != FOS__OK)
		return FOS__FAIL;

	if(FWriter_WaitingState(p, FILE_STATE__CLOSED, FOS_FS_TIMEOUT_MS) != FOS__OK)
	{
		FWriter_SetMode(p, FILE_MODE__OFF);
		return FOS__FAIL;
	}

	return FOS__OK;
}


/*
 * Begin test writing
 * Call from the owner thread
 * p - pointer to the writer object
 * Returns execution result
 */
fos_ret_t API_FWriter_StartWriteTest(fwriter_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	if(p->state == FILE_STATE__NO_INIT)
		return FOS__FAIL;

	// set writer work mode
	if(FWriter_SetMode(p, FILE_MODE__TEST) != FOS__OK)
		return FOS__FAIL;

	if(FWriter_WaitingState(p, FILE_STATE__OPENED, FOS_FS_TIMEOUT_MS) != FOS__OK)
		return FOS__FAIL;

	return FOS__OK;
}


/*
 * Write data
 * Thread-safe. Call from any thread, main loop or interrupt
 * p - pointer to the writer object
 * data - pointer to the written data
 * data_len - length of the written data
 * Note: written data is copied in one piece
 * Returns execution result
 */
fos_ret_t API_FWriter_Write(fwriter_t *p, uint8_t* data, uint32_t data_len)
{
	if((p == NULL) || (data == NULL))
		return FOS__FAIL;

	if(p->mode != FILE_MODE__ON)
		return FOS__FAIL;

	if(CircBuf_AddDataProtected(&p->var.cbuf, data, data_len) != CIRC_BUF__OK)
		return FOS__FAIL;

	return FOS__OK;
}


/*
 * Synchronize data
 * Call from the owner thread
 * p - pointer to the writer object
 * Returns execution result
 */
fos_ret_t API_FWriter_FSync(fwriter_t *p)
{
	if(p == NULL)
		return FOS__FAIL;

	p->var.fsync_flag = FOS__ENABLE;

	return FOS__OK;
}


/*
 * Receive file state
 * p - pointer to the writer object
 * Returns file state
 */
file_state_t API_FWriter_GetFileState(fwriter_t *p)
{
	if(p == NULL)
		return FILE_STATE__NO_INIT;

	return p->state;
}


// change writer work mode
static fos_ret_t FWriter_SetMode(fwriter_t *p, file_mode_t mode)
{
	if(p == NULL)
		return FOS__FAIL;

	if(p->state == FILE_STATE__NO_INIT)
		return FOS__FAIL;

	if((p->mode == FILE_MODE__OFF) && (mode == FILE_MODE__IDDLE))
		return FOS__FAIL;

	if(p->mode == FILE_MODE__IDDLE)
		return FOS__FAIL;

	if((p->mode == FILE_MODE__ON) && (mode == FILE_MODE__TEST))
		return FOS__FAIL;

	if((p->mode == FILE_MODE__TEST) && (mode == FILE_MODE__ON))
		return FOS__FAIL;

	p->mode = mode;

	return FOS__OK;
}


// pending file state
static fos_ret_t FWriter_WaitingState(fwriter_t *p, file_state_t state, uint32_t timeout_ms)
{
	const uint32_t sleep = 10;

	if(p == NULL)
		return FOS__FAIL;

	if(p->state == FILE_STATE__NO_INIT)
		return FOS__FAIL;

	uint32_t timeout = SL_GetTick() + timeout_ms;

	while(p->state != state)
	{
		if(p->var.err & FILE_CRITICAL_ERR_MASK)
			return FOS__FAIL;

		if(SL_GetTick() > timeout)
			return FOS__FAIL;

		SYS_FOS_Sleep(sleep);
	}

	return FOS__OK;
}


// set path
static fos_ret_t FWriter_SetPath(fwriter_t *p, fsys_path_t *path)
{
	if((p == NULL) || (path == NULL))
		return FOS__FAIL;

	memcpy(&p->path, path, sizeof(fsys_path_t));

	return FOS__OK;
}







