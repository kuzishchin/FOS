/**************************************************************************//**
 * @file      fos_svc_id.h
 * @brief     System call IDs. Header file.
 * @version   V1.2.02
 * @date      23.01.2026
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
#define FOS_SYSCALL_FOS_DELETE_SEMB         0x0C        // fos_ret_t USER_FOS_DeleteSemBinary(user_desc_t semb);
#define FOS_SYSCALL_FOS_GET_THREAD_SEMB_D   0x0D        // user_desc_t USER_FOS_GetThreadSembDesc(user_desc_t desc);
#define FOS_SYSCALL_FOS_SEMB_SET_TIMEOUT    0x0E        // fos_ret_t USER_FOS_SemBinarySetTimeout(user_desc_t semb, uint32_t timeout_ms);
#define FOS_SYSCALL_FOS_SEMC_TAKE           0x0F        // fos_ret_t USER_FOS_SemCntTake(user_desc_t semc);
#define FOS_SYSCALL_FOS_SEMC_GIVE           0x10        // fos_ret_t USER_FOS_SemCntGive(user_desc_t semc);
#define FOS_SYSCALL_FOS_CREATE_SEMC         0x11        // user_desc_t USER_FOS_CreateSemCnt(uint32_t max_cnt, uint32_t init_cnt);
#define FOS_SYSCALL_FOS_DELETE_SEMC         0x12        // fos_ret_t USER_FOS_DeleteSemCnt(user_desc_t semc);
#define FOS_SYSCALL_FOS_SEMC_SET_TIMEOUT    0x13        // fos_ret_t USER_FOS_SemCntSetTimeout(user_desc_t semc, uint32_t timeout_ms);
#define FOS_SYSCALL_FOS_QUEUE_32_CREATE     0x14        // user_desc_t USER_FOS_CreateQueue32(uint16_t size, fos_queue_mode_t mode, uint32_t timeout_ms);
#define FOS_SYSCALL_FOS_QUEUE_32_DELETE     0x15        // fos_ret_t USER_FOS_DeleteQueue32(user_desc_t que);
#define FOS_SYSCALL_FOS_QUEUE_32_READ       0x16        // fos_ret_t USER_FOS_Queue32ReadData(user_desc_t que, uint32_t* data_ptr, fos_queue_sw_t blocking_mode_sw);
#define FOS_SYSCALL_FOS_QUEUE_32_WRITE      0x17        // fos_ret_t USER_FOS_Queue32WriteData(user_desc_t que, uint32_t data);
#define FOS_SYSCALL_FOS_QUEUE_32_ASK        0x18        // fos_ret_t USER_FOS_Queue32AskData(user_desc_t que, fos_queue_sw_t blocking_mode_sw);
#define FOS_SYSCALL_FOS_SEMB_TAKE_STAT      0x19        // fos_ret_t USER_FOS_SemBinaryTakeStat(user_desc_t semb);
#define FOS_SYSCALL_FOS_SEMC_TAKE_STAT      0x1A        // fos_ret_t USER_FOS_SemCntTakeStat(user_desc_t semc);


#endif /* APPLICATION_FOS_FOS_SVC_ID_H_ */














