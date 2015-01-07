/*                               -*- Mode: C -*- 
 * log.c
 * 
 * Description     : Informational, error, and debug message logging functions.
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

#include <stdio.h>
#include <stdarg.h>

#include "log.h"

#if defined(_REENTRANT) || defined(_POSIX_PTHREAD)
static char*	SavedFileName;
static u_long	SavedLineNumber;
static char*	getFileName();
static u_long	getLineNumber();
#else
static char*	SavedFileName;
static u_long	SavedLineNumber;

#define getFileName()	SavedFileName	
#define getLineNumber()	SavedLineNumber	
#endif

/*
 * SetDebugLevel
 *   Sets the current debug level.
 *   A debug level contains a category mask and a focus value.
 *   The focus is contained in the lower 4 bits.
 *   The remaining bits comprise the category.
 *   A debug message is output if the category bit is set in the
 *   current debug level and the focus is greater than or equal to
 *   the focus in the current debug level.
 *
 * Parameters:
 *   level (input) - new debug level.
 *
 * Returns:
 *   old debug level.
 */

#if defined(_REENTRANT) || defined(_POSIX_PTHREAD)
u_long
SetDebugLevel(u_long	level)
{
	NOT IMPLEMENTED;
}
#else
static u_long	DebugLevel = 0;

u_long
SetDebugLevel(u_long	level)
{
	u_long	old = DebugLevel;
	DebugLevel = level;
	return(old);
}
#endif


/*
 * GetDebugLevel
 *   Returns the current debug level.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   debug level.
 */
#if defined(_REENTRANT) || defined(_POSIX_PTHREAD)
u_long
GetDebugLevel()
{
	NOT IMPLEMENTED;
	return();
}
#else
u_long
GetDebugLevel()
{
	return(DebugLevel);
}
#endif


void
Log(char*	format,
	...)
{
	va_list	ap;
	
	if (format == 0)
		return;
	
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

void
LogDebug(u_long	level,
		char*	format,
		...)
{
	va_list	ap;
	u_long	current_level = GetDebugLevel();
	
	if ((((level&current_level)&0xfffffff0) == 0) ||
		((level&0x0f) > (current_level&0x0f)))
		return;
	
	va_start(ap, format);
	fprintf(stderr, "DEBUG: %s:%lu ", getFileName(), getLineNumber());
	vfprintf(stderr, format, ap);
	va_end(ap);
}


static char*	PriorityStr[] =
{
	"EMERGENCY",
	"ALERT",
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
};


void
LogError(u_long	priority,
		char*	format,
		...)
{
	va_list	ap;

	if (priority > 7)
		priority = LOG_ERR;
	
	va_start(ap, format);
	fprintf(stderr, "%s: %s:%lu ", PriorityStr[priority],
			getFileName(), getLineNumber());
	vfprintf(stderr, format, ap);
	va_end(ap);
}

#if defined(_REENTRANT) || defined(_POSIX_PTHREAD)
/*
 * Thread safe implementation for storing the filename and
 * line number of the code location logging a debug or error message.
 */
LogFuncPtr_t
LogSaveFileAndLine(char			*filename,
				   u_long		linenum,
				   LogFuncPtr_t	func)
{
	NOT IMPLEMENTED;
	return(func);
}

static char*
getFileName()
{
	NOT IMPLEMENTED;
	return();
}

static u_long
getLineNumber()
{
	NOT IMPLEMENTED;
	return();
}
#else
LogFuncPtr_t
LogSaveFileAndLine(char			*filename,
				   u_long		linenum,
				   LogFuncPtr_t	func)
{
	SavedFileName   = filename;
	SavedLineNumber = linenum;
	return(func);
}
#endif /* if defined(_REENTRANT) || defined(_POSIX_PTHREAD) */


