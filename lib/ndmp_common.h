/*                               -*- Mode: C -*- 
 * ndmp_common.h
 * 
 * Description     : NDMP client/server common stuff.
 * 
 * Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
 *

 *
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
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

#ifndef _ndmp_common_h
#define _ndmp_common_h

#include "log.h"
#include "ndmp.h"

#ifdef __linux__
typedef long long          longlong_t;
typedef unsigned long long u_longlong_t;
#define USE_SELECT
#endif

#define XDR_AND_SIZE(func) (bool_t(*)(XDR*, ...))xdr_##func ,sizeof(func)
#define AUTH_REQUIRED		TRUE
#define AUTH_NOT_REQUIRED	FALSE

typedef void*	NdmpConnection;

typedef void NdmpConHandlerFunc(NdmpConnection);
	
typedef void NdmpMsgHandlerFunc(NdmpConnection	connection,
								void*			request);

typedef struct NdmpMsgHandler
{
	ndmp_message		message;
	u_short				version;
	NdmpMsgHandlerFunc*	func;
	bool_t				auth_required;
	bool_t				(*xdr_request)(XDR* xdrs, ...);
	int					sizeof_request;
	bool_t				(*xdr_reply)(XDR* xdrs, ...);
	int					sizeof_reply;
} NdmpMsgHandler;

typedef struct MsgInfo
{
	ndmp_header			hdr;
	NdmpMsgHandler*		handler;
	void*				body;
} MsgInfo;

/* Connection data structure. */
typedef struct Connection
{
	int					sock;
	XDR					xdrs;
	u_long				mySequence;
	bool_t				authorized;
	bool_t				eof;
	MsgInfo				msginfo;		/* received request or reply message */
	NdmpMsgHandler*		msgHandlerTbl;
	u_short				version;
	void*				clientData;
} Connection;

NdmpConnection
ndmpCreateConnection(NdmpMsgHandler*	msgHandlerTbl);

void
ndmpDestroyConnection(NdmpConnection	connection);

void
ndmpClose(NdmpConnection	connectionHandle);

int
ndmpConnect(NdmpConnection	connectionHandle,
			char*			host,
			u_long			port);

int
ndmpPoll(NdmpConnection	connectionHandle,
		 bool_t			block);

void
ndmpRun(u_long				port,
		bool_t				dofork,
		NdmpMsgHandler*		msgHandlerTbl,
		NdmpConHandlerFunc*	conHandlerFunc);

int
ndmpProcessRequests(NdmpConnection	connectionHandle);

int
ndmpSendReply(NdmpConnection	connectionHandle,
			  ndmp_error		err,
			  void*				data);

int
ndmpSendRequest(NdmpConnection	connectionHandle,
				ndmp_message	message,
				ndmp_error		err,
				void*			request_data,
				void**			reply_data);

void
ndmpFreeMessage(NdmpConnection	connectionHandle);

int
ndmpGetFd(NdmpConnection	connectionHandle);

void
ndmpSetClientData(NdmpConnection	connectionHandle,
				  void				*client_data);

void*
ndmpGetClientData(NdmpConnection	connectionHandle);

void
ndmpSetVersion(NdmpConnection	connectionHandle,
			   u_short			version);

u_short
ndmpGetVersion(NdmpConnection	connectionHandle);

void
ndmpSetAuthorized(NdmpConnection	connectionHandle,
				  bool_t			authorized);

u_longlong_t
quadToLongLong(ndmp_u_quad	quad);

ndmp_u_quad
longLongToQuad(u_longlong_t	ull);

typedef int type_handlePostMessage(Connection *);
extern int
sethandlePostMessage(type_handlePostMessage *);

#endif /* _ndmp_common_h */
