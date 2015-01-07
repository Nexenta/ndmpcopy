/*                               -*- Mode: C -*- 
 * ndmpc.h
 * 
 * Description     : NDMP client application commons.
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

/*
 * The majority of this code was taken from the NDMP SDK client toolkit.
 */

#ifndef ndmpc_h_
#define ndmpc_h_

#include "ndmp_common.h"

char *ndmpErrToStr(ndmp_error err);

int getCdb(int num, char **cdb, int *cdb_len, char **data, int *data_len);
void displayCdbReply(char status, u_long dataout_len, char *data, int data_len, char *sense, int sense_len);

NdmpMsgHandlerFunc	notifyConnected;
NdmpMsgHandlerFunc	notifyDataHalted;
NdmpMsgHandlerFunc	notifyMoverHalted;
NdmpMsgHandlerFunc	notifyMoverPaused;
NdmpMsgHandlerFunc	notifyDataRead;
NdmpMsgHandlerFunc	logLog;
NdmpMsgHandlerFunc	logDebug;
NdmpMsgHandlerFunc	logFile;
NdmpMsgHandlerFunc	fileHistoryAddUnix;
NdmpMsgHandlerFunc	fileHistoryAddUnixNode;
NdmpMsgHandlerFunc	fileHistoryAddUnixDir;

#endif /* ifndef ndmpc_h_ */

/*
 * End of file	:	ndmpc.h
 */
