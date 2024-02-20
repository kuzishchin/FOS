/**************************************************************************//**
 * @file      fos_scheduler.c
 * @brief     Scheduler. Source file.
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

#include "Thread/fos_scheduler.h"
#include "Platform/sl_platform.h"
#include <string.h>


// спланировать задачу (возвращает номре выбранной задачи или -1, если её нет)
int16_t FOS_Schedule(fos_scheduler_t *ptr, volatile fos_thread_ptr *thr_desc_list, uint8_t thr_max_id)
{
	if((ptr == NULL) || (thr_desc_list == NULL) || (thr_max_id >= FOS_MAX_THR_CNT))
		return -1;

	fos_thread_state_t thr_state;      // состояние потока
	fos_thread_ptr     thr_desc;       // дескриптор потока
	uint8_t thr_pr;                    // приоритет потока
	uint8_t ind;                       // индекс
	uint8_t isActiveThr = 0;           // флаг наличия активного потока
	uint8_t active_thr;                // индекс активного потока

	/*
	 * Сбрасываем все списки
	 */
	memset(ptr->thr_plist, FOS_EMPTY_ID, FOS_MAX_THR_CNT);
	memset(ptr->run_list, FOS_EMPTY_ID, FOS_MAX_THR_CNT);
	memset(ptr->priority_list, 0, FOS_PRIORITY_CNT);

	ptr->ready_thr_cnt = 0;
	ptr->run_list_cnt = 0;

	/*
	 * Строим список потоков
	 */
	for(uint8_t i = 0; i <= thr_max_id; i++)         // перебираем все потоки
	{
		thr_desc = thr_desc_list[i];                 // получаем дескриптор очередного потока
		if(thr_desc == NULL)                         // проверяем что он есть
			continue;

		thr_state = thr_desc->var.state;             // получаем состояние
		thr_pr = thr_desc->set.priotity;             // получаем приоритет
		if(thr_pr >= FOS_PRIORITY_CNT)               // проверяем приоритет
			thr_pr = FOS_PRIORITY_CNT - 1;

		// если поток запущен или готов к выполнению
		if((thr_state == FOS__THREAD_READY) || (thr_state == FOS__THREAD_RUNNING))
		{
			ptr->thr_plist[i] = thr_pr;         // ставим его приоритет в списке потоков
			ptr->priority_list[thr_pr]++;       // инкрементируем счётчик в списке приоритетов
			ptr->ready_thr_cnt++;               // инкрементируем счётчик готовых потоков
		}else
		{
			ptr->thr_plist[i] = FOS_SUSPEND_BLOCKED_ID;  // иначе, ставем индекс спащего/заблокированного потока
		}
	}

	// если готовых задач нет
	if(ptr->ready_thr_cnt == 0)
		return -1;                   // возвращаем -1

	/*
	 * Ищем максимальный приоритет потока
	 */
	thr_pr = 0;                                          // изначально приоритет наивысший
	for(uint8_t i = 0; i < FOS_PRIORITY_CNT; i++)        // перебираем список приоритетов
	{
		if(ptr->priority_list[thr_pr] != 0)              // если не 0
			break;                                       // выходим
		thr_pr++;                                        // иначе понижаем приоритет (увеличиваем значение приоритета)
	}

	/*
	 * Заполняем список задач на выполнение
	 */
	ind = 0;
	for(uint8_t i = 0; i <= thr_max_id; i++)             // перебираем все потоки
	{
		if(ptr->thr_plist[i] == thr_pr)                  // находим поток с целевым приоритетом (его получили на предыдущем шаге)
		{
			ptr->run_list[ind] = i;                      // запоминаем индекс потокв в списк выполнения
			ind++;                                       // увеличиваем индекс

			thr_desc = thr_desc_list[i];                 // получаем дескриптор поток
			if(thr_desc == NULL)                         // проверяем что он есть (избыточно!!!)
				continue;

			// обрабатываем ситуацию что это был активный поток
			if(thr_desc->var.state == FOS__THREAD_RUNNING)
			{
				isActiveThr = 1;                         // ставим флаг наличия активного потока
				active_thr = i;                          // запоминаем его индекс
			}
		}
	}
	ptr->run_list_cnt = ind;                             // запоминаем число потоков в списке выполнения

	/*
	 * Если на очереди одна задача
	 */
	if(ptr->run_list_cnt == 1)
		return ptr->run_list[0];        // возвращаем ее индекс

	/*
	 * Если на очерди более одной задачи и нет активной
	 */
	if(isActiveThr == 0)
		return ptr->run_list[0];        // возвращаем индекс первой в списке выполнения задачи

	/*
	 * Если на очерди более одной задачи и есть активная
	 */
	for(uint8_t i = 0; i < ptr->run_list_cnt; i++)       // перебираем все задачи в списке выполнения
	{
		// ищем активную задачу
		if(ptr->run_list[i] == active_thr)
		{
			ind = i;                                     // запоминаем индекс активной задачи в списке выполнения
			break;                                       // выходим
		}
	}

	// циклически переходим на следующую за активной задачей
	ind++;
	if(ind >=  ptr->run_list_cnt)      // кольцуем задачи
		ind = 0;

	// возвращаем индекс очередной задачи
	return ptr->run_list[ind];
}


// отладка
void FOS_ScheduleDbg(fos_scheduler_t *ptr, uint8_t thr_max_id, uint8_t id, uint32_t thr_dt_us)
{
	const uint32_t period_ms = 1000;

	if((ptr == NULL) || (thr_max_id >= FOS_MAX_THR_CNT) || (id >= FOS_MAX_THR_CNT))
		return;

	ptr->var.curr_dt_us[id] += thr_dt_us;

	if((SL_GetTick() - ptr->var.ts) < period_ms)
		return;
	ptr->var.ts = SL_GetTick();

	uint32_t all_dt_us = 0;

	for(uint8_t i = 0; i <= thr_max_id; i++)
	{
		ptr->dbg.thr_active_per_1s[i] = ptr->var.curr_dt_us[i] / 1000;
		ptr->var.curr_dt_us[i] = 0;

		all_dt_us += ptr->dbg.thr_active_per_1s[i];
	}

	ptr->dbg.iddle_time_ms_per_1s   = ptr->dbg.thr_active_per_1s[0];
	ptr->dbg.all_thr_time_ms_per_1s = all_dt_us - ptr->dbg.iddle_time_ms_per_1s;
	ptr->dbg.sys_time_ms_per_1s     = 1000 - all_dt_us;
}




