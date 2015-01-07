/*                               -*- Mode: C -*- 
 * log.h
 * 
 * Description     : NMDP common error and debug logging
 *                   function interfaces.
 * 
 * Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _ndmp_log_h
#define _ndmp_log_h

#include <sys/types.h>
#include <syslog.h>

typedef void			LogFunc_t(u_long, char*, ...);
typedef LogFunc_t*		LogFuncPtr_t;

LogFuncPtr_t	LogSaveFileAndLine(char*, u_long, LogFuncPtr_t);
LogFunc_t		LogDebug;
LogFunc_t		LogError;

#define Debug	(*LogSaveFileAndLine)(__FILE__, __LINE__, LogDebug)
#define Error	(*LogSaveFileAndLine)(__FILE__, __LINE__, LogError)

void			Log(char*, ...);
u_long			SetDebugLevel(u_long	level);
u_long			GetDebugLevel(void);

/*
 * Debug categories.
 */
#define	DBG_CAT_CONN      0x00010	/* Connection management */
#define	DBG_CAT_COMM      0x00020	/* Communication library */
#define DBG_CAT_CONNECT   0x00040	/* Connection request handling */
#define DBG_CAT_CONFIG    0x00080	/* Configuration request handling */
#define DBG_CAT_TAPE      0x00100	/* Tape request handling */
#define DBG_CAT_SCSI      0x00200	/* SCSI request handling */
#define DBG_CAT_DATA      0x00400	/* Data request handling */
#define DBG_CAT_NOTIFY    0x00800	/* Notify request handling */
#define DBG_CAT_HISTORY   0x01000	/* File history request handling */
#define DBG_CAT_LOG       0x02000	/* Log request handling */
#define DBG_CAT_MOVER     0x04000	/* Data mover */
#define DBG_CAT_BACKUP    0x08000	/* Dump/restore module */

/*
 * Debug focus.
 */
#define DBG_FOC_ALWAYS    0
#define DBG_FOC_FLOW      1
#define DBG_FOC_DETAIL    2

#endif /* ifndef _ndmp_log_h */
