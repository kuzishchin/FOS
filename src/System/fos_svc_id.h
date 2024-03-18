/**************************************************************************//**
 * @file      fos_svc_id.h
 * @brief     System call IDs. Header file.
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

#ifndef APPLICATION_FOS_FOS_SVC_ID_H_
#define APPLICATION_FOS_FOS_SVC_ID_H_


#define FOS_SYSCALL_FOS_YIELD               0x00        // void FOS_Yield();
#define FOS_SYSCALL_FOS_SLEEP               0x01        // fos_ret_t USER_FOS_Sleep(uint32_t time);
#define FOS_SYSCALL_FOS_SEMB_TAKE           0x02        // fos_ret_t USER_FOS_SemBinaryTake(fos_semaphore_binary_t *semb);
#define FOS_SYSCALL_FOS_SEMB_GIVE           0x03        // fos_ret_t USER_FOS_SemBinaryGive(fos_semaphore_binary_t *semb);
#define FOS_SYSCALL_FOS_CREATE_THREAD       0x04        // fos_thread_t* USER_FOS_CreateThread(fos_thread_user_init_t *user_init);
#define FOS_SYSCALL_FOS_CREATE_SEMB         0x05        // fos_semaphore_binary_t* USER_FOS_CreateSemBinary(fos_semb_state_t init_state);
#define FOS_SYSCALL_FOS_THREAD_RUN          0x06        // fos_ret_t USER_FOS_RunDesc(fos_thread_ptr desc);
#define FOS_SYSCALL_FOS_THREAD_TERMINATE    0x07        // fos_ret_t USER_FOS_Terminate(int32_t terminate_code);
#define FOS_SYSCALL_FOS_THREAD_TERMINATE_D  0x08        // fos_ret_t USER_FOS_TerminateDesc(fos_thread_ptr desc, int32_t terminate_code);
#define FOS_SYSCALL_FOS_ERROR_SET           0x09        // USER_FOS_ErrorSet(fos_err_t *err);
#define FOS_SYSCALL_FILE_MOUNT              0x0A        // file_err_t File_Mount(uint8_t dev_num);
#define FOS_SYSCALL_FILE_UNMOUNT            0x0B        // file_err_t File_Unmount(uint8_t dev_num);


#endif /* APPLICATION_FOS_FOS_SVC_ID_H_ */






