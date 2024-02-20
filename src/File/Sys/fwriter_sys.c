/**************************************************************************//**
 * @file      fwriter_sys.c
 * @brief     The driver of file writing handling. Source file.
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


#include "File/Sys/fwriter_sys.h"
#include "Platform/sl_platform.h"


static uint8_t write_buf[FOS_FILEWR_MAX_BUF_LEN];   // буфер на запись


// инициализация
fos_ret_t FWriter_Init(fwriter_t *p, file_init_t *init)
{
	if((p == NULL) || (init == NULL))
		return FOS__FAIL;

	if((init->cbuf_init.buf_len == 0) || (init->cbuf_init.buf_ptr == NULL))
		return FOS__FAIL;

	if(init->cbuf_init.buf_len > FOS_FILEWR_MAX_BUF_LEN)
		return FOS__FAIL;

	p->mode = FILE_MODE__OFF;
	memset(&p->var, 0, sizeof(fwriter_var_t));
	memset(&p->dbg, 0, sizeof(file_dbg_t));
	memset(&p->path, 0, sizeof(fsys_path_t));

	CircBuf_Init(&p->var.cbuf, &init->cbuf_init);

	p->state = FILE_STATE__CLOSED;

	return FOS__OK;
}


// системный обработчик
void FWriter_SysProc(fwriter_t *p)
{
	if(p == NULL)
		return;

	if(p->state == FILE_STATE__NO_INIT)        // проверям инициализацию
		return;

	if(p->var.err != FILE_ERR__NO)                 // проверяем ошибки
	{
		Private_FWriter_ErrorProc(p);              // обработчик ошибок
		return;
	}

	// обработка режимов
	switch(p->mode)
	{
	case FILE_MODE__OFF:
		Private_FWriter_ProcOffMode(p);
		break;

	case FILE_MODE__IDDLE:
		Private_FWriter_IddleAndOnMode(p);
		break;

	case FILE_MODE__ON:
		Private_FWriter_IddleAndOnMode(p);
		break;

	case FILE_MODE__TEST:
		Private_FWriter_TestMode(p);
		break;
	}
}


// получить длину данных которые надо записать
uint16_t FWriter_GetDataToWrite(fwriter_t *p)
{
	if(p == NULL)
		return 0;

	if(p->var.err & FILE_CRITICAL_ERR_MASK)
		return 0;

	if(p->mode == FILE_MODE__OFF)
		return 0;

	if(p->mode == FILE_MODE__TEST)
		return p->var.cbuf.buf_len;

	return CircBuf_GetDataLen(&p->var.cbuf);          // получем длину данных в буфере на запись
}


// обработчик ошибок
static void Private_FWriter_ErrorProc(fwriter_t *p)
{
	if(p->state == FILE_STATE__OPENED)           // если файл открыт
	{
		Private_FWriter_Close(&p->var.fsys);         // закрываем
		p->state = FILE_STATE__CLOSED;
		p->mode  = FILE_MODE__OFF;
	}
}


// обработчик режима OFF
static void Private_FWriter_ProcOffMode(fwriter_t *p)
{
	if(p->state == FILE_STATE__OPENED)           // если файл открыт
	{
		p->var.err = Private_FWriter_Close(&p->var.fsys);       // закрываем
		p->state = FILE_STATE__CLOSED;
	}
}


// обработчик режима IDDLE и режима ON
static void Private_FWriter_IddleAndOnMode(fwriter_t *p)
{
	if(p->state == FILE_STATE__CLOSED)           // если файл закрыт
	{
		p->var.err = Private_FWriter_Open(&p->var.fsys, &p->path);        // открываем с обработкой ошибок
		if(p->var.err != FILE_ERR__NO)
			return;

		p->state = FILE_STATE__OPENED;
	}

	if(p->state == FILE_STATE__OPENED)           // если файл открыт
	{
		uint16_t len = Private_FWriter_BufWrite(p);  // пишем в него данные из буфера и запрашиваем длину этих данных

		if((p->mode == FILE_MODE__IDDLE) && (len == 0))   // если буфер был пуст, а режим IDDLE, то переходим в OFF
			p->mode = FILE_MODE__OFF;
	}
}


// обработчик режима TEST
static void Private_FWriter_TestMode(fwriter_t *p)
{
	if(p->state == FILE_STATE__CLOSED)                     // если файл закрыт, открываем
	{
		file_err_t err = Private_FWriter_Open(&p->var.fsys, &p->path);
		if(err == FILE_ERR__NO)
			p->state = FILE_STATE__OPENED;

		for(uint32_t i = 0; i < p->var.cbuf.buf_len; i++)      // тестовые значения на запись
			p->var.cbuf.buf_ptr[i] = 'T';

		p->var.err = err;
	}

	if(p->state == FILE_STATE__OPENED)                     // если открыт
	{
		Private_FWriter_TestWrite(p);                          // осуществялем тестовую запись
	}
}


// записать данные из буфера
static uint16_t Private_FWriter_BufWrite(fwriter_t *p)
{
	circ_buf_t* cbuf = &p->var.cbuf;

	uint16_t data_len = CircBuf_GetDataLen(cbuf);              // получем длину данных в буфере на запись
	if(data_len)                                               // если данные есть
	{
		uint16_t si = CircBuf_GetStartIndex(cbuf);                                            // получаем стартовый индекс данных
		uint16_t ei = CircBuf_AddIndValue(cbuf, si, data_len);                                // получаем конечный индекс
		uint16_t part_len = CircBuf_DataCopyBetweenIndexes(cbuf, write_buf,si, ei);           // копируем данные в промежуточный буфер записи
		CircBuf_SetStartInd(cbuf, ei);                                                        // удаляем данные из приёмного буфера

		if(data_len != part_len){                                                             // контроь длины данных, избыточно
			p->var.err = FILE_ERR__WRONG_CPY_LEN;
			return data_len;
		}

		p->var.err = Private_FWriter_Write(&p->var.fsys, write_buf, data_len);                // пишем данные
	}

	/*
	 * Синхронизация файла
	 */
	if(p->var.fsync_flag == FOS__ENABLE)                      // если стоит флаг синхронизации
	{
		p->var.fsync_flag = FOS__DISABLE;                     // сбрасываем его
		p->var.err = Private_FWriter_Sync(&p->var.fsys);      // синхронизуем файл
	}

	return data_len;                                                                          // возвращем длину записанныз данных
}


// тестовая запись в файл
static void Private_FWriter_TestWrite(fwriter_t *p)
{
	const uint8_t SUB_PERIODS = 5;                         // число тестовых подпериодов

	file_dbg_t* dbg = &p->dbg;

	for(uint8_t i = 0; i < SUB_PERIODS; i++)               // пишем SUB_PERIODS раз весь входной буфер
	{
		p->var.err = Private_FWriter_Write(&p->var.fsys, p->var.cbuf.buf_ptr, p->var.cbuf.buf_len);
		if(p->var.err != FILE_ERR__NO)
			return;
	}

	dbg->period++;

	if((SL_GetTick() - dbg->ts) > 1000)
	{
		dbg->ts = SL_GetTick();
		dbg->test_wr_s = dbg->period - dbg->last_period;
		dbg->last_period = dbg->period;
	}
}


