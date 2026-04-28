/**************************************************************************//**
 * @file      fwriter.h
 * @brief     API for the file writer object. Header file.
 * @version   V1.0.03
 * @date      13.04.2026
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
 * 6.2.2 Open file
 * Call from the owner thread
 * p    - pointer to the writer object
 * path - path to the file
 * Returns execution result
 *
 * Actual from v0.10
 */
fos_ret_t API_FWriter_Open(fwriter_t *p, fsys_path_t *path);


/*
 * 6.2.3 Close file
 * Call from the owner thread
 * p - pointer to the writer object
 * Returns execution result
 *
 * Actual from v0.10
 */
fos_ret_t API_FWriter_Close(fwriter_t *p);


/*
 * 6.2.4 Begin test writing
 * Call from the owner thread
 * p - pointer to the writer object
 * Returns execution result
 *
 * Actual from v0.10
 */
fos_ret_t API_FWriter_StartWriteTest(fwriter_t *p);


/*
 * 6.2.5 Write data
 * Thread-safe. Call from any thread, main loop or interrupt
 * p - pointer to the writer object
 * data - pointer to the written data
 * data_len - length of the written data
 * Note: written data is copied in one piece
 * Returns execution result
 *
 * Actual from v0.10
 */
fos_ret_t API_FWriter_Write(fwriter_t *p, uint8_t* data, uint32_t data_len);


/*
 * 6.2.6 Synchronize data
 * Call from the owner thread
 * p - pointer to the writer object
 * Returns execution result
 *
 * Actual from v0.10
 */
fos_ret_t API_FWriter_FSync(fwriter_t *p);


/*
 * 6.2.7 Receive file state
 * p - pointer to the writer object
 * Returns file state
 *
 * Actual from v0.10
 */
file_state_t API_FWriter_GetFileState(fwriter_t *p);




#endif /* APPLICATION_FOS_FILE_FWRITER_H_ */









