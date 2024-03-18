/**************************************************************************//**
 * @file      fos_scheduler.h
 * @brief     Scheduler. Header file.
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

#ifndef APPLICATION_FOS_THREAD_SCHEDULER_H_
#define APPLICATION_FOS_THREAD_SCHEDULER_H_


#include "Thread/fos_thread.h"


// переменные
typedef struct
{
	uint32_t curr_dt_us[FOS_MAX_THR_CNT];       // текущее время каждого потока
	uint32_t ts;                                // метка времени

} fos_scheduler_var_t;

// отладочаня информация
typedef struct
{
	uint32_t thr_active_per_1s[FOS_MAX_THR_CNT];// число переходов потоков в активное состояние в течении 1 сек

	uint32_t iddle_time_ms_per_1s;              // бездействие системы в течении 1 сек, мс
	uint32_t all_thr_time_ms_per_1s;            // время на все потоки в течении 1 сек, мс
	uint32_t sys_time_ms_per_1s;                // системное время в течении 1 сек, мс

} fos_scheduler_dbg_t;

// планировщик задач
typedef struct
{
	uint8_t thr_plist[FOS_MAX_THR_CNT];          // список потоков (для готовых и активных - номер приоритета, для остальных FOS_SUSPEND_BLOCKED_ID)
	uint8_t run_list[FOS_MAX_THR_CNT];           // список готовых к выполению потоков (включая активный)

	uint8_t priority_list[FOS_PRIORITY_CNT];     // число потоков для каждого приоритета

	uint8_t ready_thr_cnt;                       // число готовых потоков (включая активный)
	uint8_t run_list_cnt;                        // число записей в run_list

	fos_scheduler_var_t var;                     // переменные
	fos_scheduler_dbg_t dbg;                     // отладочная информация

} fos_scheduler_t;


// спланировать задачу (возвращает номре выбранной задачи или -1, если её нет)
int16_t FOS_Schedule(fos_scheduler_t *ptr, volatile fos_thread_ptr *thr_desc_list, uint8_t thr_max_id);

// отладка
void FOS_ScheduleDbg(fos_scheduler_t *ptr, uint8_t thr_max_id, uint8_t id, uint32_t thr_dt_us);


#endif /* APPLICATION_FOS_THREAD_SCHEDULER_H_ */





