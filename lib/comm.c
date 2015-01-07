/*                               -*- Mode: C -*- 
 * comm.c
 * 
 * Description     : NDMP common communication functions.
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
 * 
 */


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>

#include "ndmp_common.h"

/* try to use select() since it is already used in src/dump.c */
#if !defined(USE_POLL) && defined(FD_SET) && defined(FD_ZERO)
#define USE_SELECT
#endif


static int
ndmp_readit(void*		connectionHandle,
			caddr_t		buf,
			int			len);

static int
ndmp_writeit(void*		connectionHandle,
			 caddr_t	buf,
			 int		len);

static int
ndmp_recv_msg(Connection*	connection);

static int
ndmp_process_messages(Connection*	connection,
					  bool_t		reply_expected);

NdmpMsgHandler*
ndmp_get_handler(Connection*	connection,
				 u_short		procedure);

type_handlePostMessage *p_handlePostMessage = NULL;

int
sethandlePostMessage(type_handlePostMessage *);

/*
 * ndmpCreateConnection
 *
 * Allocate and initialize a connection structure.
 *
 * Parameters:
 *   handler_tbl (input) - message handlers.
 *
 * Returns:
 *   0 - error
 *  connection pointer
 *
 * Notes:
 *   The returned connection should be destroyed using
 *   ndmpDestroyConnection().
 */
NdmpConnection
ndmpCreateConnection(NdmpMsgHandler*	msgHandlerTbl)
{
	Connection*		connection;
	
	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpCreateConnection\n");
	
	connection = (Connection*)malloc(sizeof(Connection));
	if (connection == 0)
	{
		Error(LOG_ERR, "system call (malloc): %s.\n",
			  strerror(errno));
		return(0);
	}

	connection->sock          = -1;
	connection->mySequence    = 0;
	connection->authorized    = FALSE;
	connection->eof           = FALSE;
	connection->msginfo.body  = 0;
	connection->msgHandlerTbl = msgHandlerTbl;
	connection->version       = NDMPVER;
	connection->clientData    = 0;
	
	connection->xdrs.x_ops    = 0;
	
	xdrrec_create(&connection->xdrs, 0, 0,
				  (caddr_t)connection,
				  ndmp_readit, ndmp_writeit);

	if (connection->xdrs.x_ops == 0)
	{
		Error(LOG_ERR, "xdrrec_create failed.\n");
		close(connection->sock);
		return(0);
	}

	return((NdmpConnection)connection);
}

/*
 * ndmpDestroyConnection
 *
 * Shutdown a connection and release allocated resources.
 *
 * Parameters:
 *   connectionHandle (Input) - connection handle.
 *
 * Returns:
 *   void
 */
void
ndmpDestroyConnection(NdmpConnection	connectionHandle)
{
	Connection*	connection = (Connection*)connectionHandle;

	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpDestroyConnection\n");

	if (connection->sock >= 0)
		(void)close(connection->sock);

	xdr_destroy(&connection->xdrs);
	free((void*)connection);
}

/*
 * ndmpClose
 *   Close a connection.
 *
 * Parameters:
 *   connectionHandle (Input) - connection handle.
 *
 * Returns:
 *   void
 */
void
ndmpClose(NdmpConnection	connectionHandle)
{
	Connection*	connection = (Connection*)connectionHandle;

	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpClose\n");

	if (connection->sock != -1)
	{
		close(connection->sock);
		connection->sock = -1;
	}
	Error(LOG_ERR, "ndmpClose: connection->eof = TRUE\n");
	connection->eof = TRUE;
}

/*
 * ndmpRun
 *
 * Creates a socket for listening and accepting connections
 * from NDMP clients.
 * Accepts connections and passes each connection to the connection
 * handler.
 * If the dofork parameter is TRUE, a child process is forked and the
 * connection handler is run in the context of the child process.
 *
 * Parameters:
 *   port (input)   -  NDMP server port.
 *                     If 0, the port number will be retrieved from
 *                     the network service database. If not found there,
 *                     the default NDMP port number (from ndmp.x)
 *                     will be used.
 *   dofork (input)  - if TRUE, connection handler is run in the context
 *                     of a child process.
 *   handler (input) - connection handler function.
 *
 * Returns:
 *   void
 *
 * Notes:
 *   This function does not return unless encountering an error
 *   related to the listen socket.
 */
void
ndmpRun(u_long				port,
		bool_t				dofork,
		NdmpMsgHandler*		msgHandlerTbl,
		NdmpConHandlerFunc*	conHandlerFunc)
{
    struct sockaddr_in	sin;
	struct servent		*sp;
	int					server_socket;
    int 				on = 1;
	
	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpRun\n");

	/*
	 * If no port number was specified, get
	 * the port number from the network services database.
	 */
	if (port == 0)
	{
		if ((sp = getservbyname("ndmp", "tcp")) == 0)
			port = NDMPPORT;
		else
			port = sp->s_port;
	}
	
	sin.sin_family 		= AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port 		= htons(port);

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		Error(LOG_ERR, "system call (socket): %s.\n",
			  strerror(errno));
		return;
	}
		
	(void) setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
					  (char *)&on, sizeof(on));

	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		Error(LOG_ERR, "system call (bind): %s.\n", strerror(errno));
		close(server_socket);
		return;
	}
		
	if (listen(server_socket, 5) < 0)
	{
		Error(LOG_ERR, "system call (listen): %s.\n",
			  strerror(errno));
		close(server_socket);
		return;
	}
		
	Debug(DBG_CAT_CONN|DBG_FOC_FLOW,
		  "ndmpd running\n");

	for (;;)
	{
		struct sockaddr_in	from;
		int					from_len = sizeof(from);
		int					ns;
		pid_t				pid;
		
		if ((ns = accept(server_socket, (struct sockaddr *)&from,
						 &from_len)) < 0)
		{
			Error(LOG_ERR, "system call (accept): %s.\n", strerror(errno));
			continue;
		}

		Debug(DBG_CAT_CONN|DBG_FOC_FLOW,
			  "Received connection from %s\n",
			  inet_ntoa(from.sin_addr));
	
		if (dofork == FALSE)
		{
			NdmpConnection	connection;
			
			if ((connection = ndmpCreateConnection(msgHandlerTbl)) == 0)
			{
				close(ns);
				continue;
			}
			
			((Connection*)connection)->sock = ns;
			
			(*conHandlerFunc)(connection);

			ndmpDestroyConnection(connection);
			continue;
		}
		
		pid = fork();
		
		if (pid < 0)
		{
			Error(LOG_ERR, "system call (fork): %s.\n", strerror(errno));
			(void)close(ns);
			continue;
		}

		if (pid == 0)
		{
			/* Child process. */

			NdmpConnection	connection;
			
			(void)close(server_socket);
			
			if ((connection = ndmpCreateConnection(msgHandlerTbl)) == 0)
			{
				(void)close(ns);
				exit(1);
			}
			
			((Connection *)connection)->sock = ns;
			(*conHandlerFunc)(connection);
			ndmpDestroyConnection(connection);
			exit(0);
		}

		(void)close(ns);

		Debug(DBG_CAT_CONN|DBG_FOC_FLOW,
			  "forked %d\n", pid);
	}
}

/*
 * ndmpConnect
 *
 * Establish a connection to an NDMP server.
 *
 * Parameters:
 *   connection (input) - connection handle.
 *   host (input) - NDMP server host name.
 *   port (input) - NDMP server port number.
 *                  if 0, the port number is retrieved from the
 *                  network service database. If not found there,
 *                  the default NDMP port number (from ndmp.x)
 *                  will be used.
 *
 * Returns:
 *   0 - successful connection.
 *  -1 - error.
 */
int
ndmpConnect(NdmpConnection	connection,
			char*			host,
			u_long			port)
{
	struct sockaddr_in	sin;
	struct hostent		*hp;
	struct servent		*sp;
	int					sock;
  
	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpConnect: host:%s port:%d\n",
		  host, port);

	if (port == 0)
	{
		if ((sp = getservbyname("ndmp", "tcp")) == 0)
			port = NDMPPORT;
		else
			port = sp->s_port;
	}
	
	if ((hp = gethostbyname(host)) == 0)
	{
		Error(LOG_ERR, "unknown host: %s\n", host);
		return(-1);
	}

	memset((void*)&sin, 0, sizeof(sin));
	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;
	sin.sin_port        = htons(port);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		Error(LOG_ERR, "system call (socket): %s\n", strerror(errno));
		return(-1);
	}

	if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		Error(LOG_ERR, "system call (connect): %s\n", strerror(errno));
		close(sock);
		return(-1);
	}
	
	((Connection*)connection)->sock       = sock;

	return(0);
}

/*
 * ndmpPoll
 *
 * Poll for message data on the connection socket and
 * process received messages.
 *
 * Parameters:
 *   connection (input) - connection handle
 *   block (input)      - If TRUE, wait for data.
 *
 * Returns:
 *   1 - messages received and processed.
 *   0 - block FALSE and no data available.
 *  -1 - Error; connection no longer established.
 */
int
ndmpPoll(NdmpConnection	connectionHandle,
		 bool_t			block)
{
	Connection		*connection = (Connection *)connectionHandle;
#ifdef USE_SELECT
   fd_set          readfds;
   struct timeval  *timeout;
#else
	struct pollfd	fds[1];
	int				timeout;
#endif
	int				ret;
	
/*	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpPoll: block:%s\n",
		  block == TRUE ? "TRUE" : "FALSE");
*/

#ifdef USE_SELECT
	if (block)
		timeout = NULL;
	else
	{
		timeout = (struct timeval *) malloc(sizeof(struct timeval));
		timeout->tv_sec = timeout->tv_usec = 0;
	}

	FD_ZERO(&readfds);
	FD_SET(connection->sock, &readfds);

	ret = select(connection->sock + 1, &readfds, NULL, NULL, timeout);

	if (timeout != NULL)
		free(timeout);
#else

	if (block)
		timeout = -1;
	else
		timeout = 0;
	
	fds[0].fd      = connection->sock;
	fds[0].events  = POLLIN|POLLRDNORM|POLLRDBAND|POLLPRI;

	ret = poll(fds, 1, timeout);
#endif

	if (ret < 0)
	{
		Error(LOG_ERR, "ndmpPoll: ret < 0, connection->eof = TRUE\n");
		connection->eof = TRUE;
		(void)close(connection->sock);
		connection->sock = -1;
		return(-1);
	}

#ifdef USE_SELECT
	if (FD_ISSET(connection->sock, &readfds))
		return(ndmpProcessRequests(connectionHandle));
	else
		return(0);
#else
	if (ret > 0)
		return(ndmpProcessRequests(connectionHandle));
	else
		return(0);
#endif
}

/*
 * ndmpProcessRequests
 *
 * Reads the next request message into the stream buffer.
 * Processes messages until the stream buffer is empty.
 *
 * Parameters:
 *   connectionHandle (input) - connection handle.
 *
 * Returns:
 *   0 - 1 or more messages successfully processed.
 *  -1 - error; connection no longer established.
 */
int
ndmpProcessRequests(NdmpConnection	connectionHandle)
{
	Connection		*connection = (Connection *)connectionHandle;
	
	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpProcessRequests\n");

	if (ndmp_process_messages(connection, 0) < 0)
		return(-1);

	return(0);
}
	
/*
 * ndmpSendRequest
 *
 * Send an NDMP request message.
 *
 * Parameters:
 *   connectionHandle  (input)  - connection pointer.
 *   message           (input)  - message number.
 *   err               (input)  - error code to place in header.
 *   request_data      (input)  - message body.
 *   reply_msg         (output) - reply message. If 0, reply will be
 *                                discarded.
 *
 * Returns:
 *   0        - successful send.
 *  -1        - error.
 *  otherwise - error from reply header.
 *
 * Notes:
 *   - The reply body is only returned if the error code is NDMP_NO_ERR.
 */
int
ndmpSendRequest(NdmpConnection	connectionHandle,
				ndmp_message	message,
				ndmp_error		err,
				void*			request_data,
				void**			reply)
{
	Connection		*connection = (Connection *)connectionHandle;
	ndmp_header		header;
	NdmpMsgHandler	*handler;
	struct timeval	time;

	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpSendRequest: message:0x%x\n",
		  message);

	/* Lookup info necessary for processing this request. */
	if (!(handler = ndmp_get_handler(connection, message)))
	{
		Error(LOG_ERR,"sending message 0x%x: not supported.\n", message);
		return(-1);
	}
  	
	gettimeofday(&time, 0);
	
	header.sequence 		= ++(connection->mySequence);
	header.time_stamp		= time.tv_sec;
	header.message_type		= NDMP_MESSAGE_REQUEST;
	header.message_code		    = message;
	header.reply_sequence	= 0;
	header.error_code			= err;

	connection->xdrs.x_op = XDR_ENCODE;
	if (!xdr_ndmp_header(&connection->xdrs, &header))
	{
		Error(LOG_ERR, "sending message 0x%x: encoding request header.\n",
			  message);
		xdrrec_endofrecord(&connection->xdrs, 1);
		return(-1);
	}
	
	if (err == NDMP_NO_ERR &&
		handler->xdr_request &&
		request_data)
	{
		if (!(*handler->xdr_request)(&connection->xdrs, request_data))
		{
			Error(LOG_ERR, "sending message 0x%x: encoding request body.\n",
				  message);
			xdrrec_endofrecord(&connection->xdrs, 1);
			return(-1);
		}
	}
	
	xdrrec_endofrecord(&connection->xdrs, 1);

	if (handler->xdr_reply == 0)
		return(0);

/*
	printf("Khushal: nsr message 0x%x\n", message);
*/
	/*
	 * Process messages until the reply to this request has been
	 * processed.
	 */
	for (;;)
	{
		int			r;

		r = ndmp_process_messages(connection, TRUE);
		/* connection error? */
		if (r < 0)
			return(-1);

		/* no reply received? */
		if (r == 0)
			continue;

		/* reply received? */
		if (r == 1)
		{
			if (message != connection->msginfo.hdr.message_code)
			{
				Error(LOG_ERR, "received unexpected reply 0x%x.\n",
					  connection->msginfo.hdr.message_code);
				ndmpFreeMessage(connectionHandle);
				return(-1);
			}

			if (reply != 0)
				*reply = connection->msginfo.body;
			else
				ndmpFreeMessage(connectionHandle);
			
			return(connection->msginfo.hdr.error_code);
		}

		/* error handling reply */
		return(-1);
	}
	
	return(0);
}

/*
 * ndmpSendReply
 *   Send an NDMP reply message.
 *
 * Parameters:
 *   connectionHandle  (input)  - connection pointer.
 *   err               (input)  - error code to place in header.
 *   reply             (input)  - reply message body.
 *
 * Returns:
 *   0 - successful send.
 *  -1 - error.
 *
 * Notes:
 *   - The body is only sent if the error code is NDMP_NO_ERR.
 */
int
ndmpSendReply(NdmpConnection	connectionHandle,
			  ndmp_error		err,
			  void*				reply)
{
	Connection		*connection = (Connection *)connectionHandle;
	ndmp_header		header;
	struct timeval	time;
	gettimeofday(&time, 0);
	
	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpSendReply: message:0x%x\n",
		  connection->msginfo.hdr.message_code);

	header.sequence 		= ++(connection->mySequence);
	header.time_stamp		= time.tv_sec;
	header.message_type		= NDMP_MESSAGE_REPLY;
	header.message_code  		= connection->msginfo.hdr.message_code;
	header.reply_sequence	= connection->msginfo.hdr.sequence;
	header.error_code			= err;

	connection->xdrs.x_op = XDR_ENCODE;
	if (!xdr_ndmp_header(&connection->xdrs, &header))
	{
		Error(LOG_ERR,
			  "sending message 0x%x: encoding reply header.\n",
			  header.message_code);
		xdrrec_endofrecord(&connection->xdrs, 1);
		return(-1);
	}

	if (err == NDMP_NO_ERR &&
		connection->msginfo.handler->xdr_reply &&
		reply)
	{
		if (!(*connection->msginfo.handler->xdr_reply)(&connection->xdrs,
													   reply))
		{
			Error(LOG_ERR,
				  "sending message 0x%x: encoding reply body.\n",
				  header.message_code);
			xdrrec_endofrecord(&connection->xdrs, 1);
			return(-1);
		}
	}
	
	xdrrec_endofrecord(&connection->xdrs, 1);
	return(0);
}

void
ndmpFreeMessage(NdmpConnection	connectionHandle)
{
	Connection*	connection = (Connection *)connectionHandle;
	
	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmpFreeMessage: message:0x%x\n",
		  connection->msginfo.hdr.message_code);

	if (connection->msginfo.handler == 0 ||
		connection->msginfo.body == 0)
		return;

	connection->xdrs.x_op = XDR_FREE;
	if (connection->msginfo.hdr.message_type == NDMP_MESSAGE_REQUEST)
	{
		if (connection->msginfo.handler->xdr_request)
			(*connection->msginfo.handler->xdr_request)(&connection->xdrs,
														connection->msginfo.body);
	}
	else
	{
		if (connection->msginfo.handler->xdr_reply)
			(*connection->msginfo.handler->xdr_reply)(&connection->xdrs,
													  connection->msginfo.body);
	}
	
	connection->msginfo.body = 0;
}

/*
 * ndmpGetFd
 *
 * Returns the connection file descriptor.
 *
 * Parameters:
 *   connectionHandle (input) - connection handle
 *
 * Returns:
 *   >=0 - file descriptor.
 *   -1  - connection not open.
 */
int
ndmpGetFd(NdmpConnection	connectionHandle)
{
	return(((Connection *)connectionHandle)->sock);
}

/*
 * ndmpSetClientData
 *
 * This function provides a means for the library client to provide
 * a pointer to some user data structure that is retrievable by
 * each message handler via ndmpGetClientData.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   clientData        (input) - user data pointer.
 *
 * Returns:
 *   void
 */
void
ndmpSetClientData(NdmpConnection	connectionHandle,
				  void*				clientData)
{
	((Connection *)connectionHandle)->clientData = clientData;
}

/*
 * ndmpGetClientData
 *
 * This function provides a means for the library client to provide
 * a pointer to some user data structure that is retrievable by
 * each message handler via ndmpGetClientData.
 *
 * Parameters:
 *   connectionHandle (input) - connection handle.
 *
 * Returns:
 *   client data pointer.
 */
void*
ndmpGetClientData(NdmpConnection	connectionHandle)
{
	return(((Connection *)connectionHandle)->clientData);
}

/*
 * ndmpSetVersion
 *   Sets the NDMP protocol version to be used on the connection.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   version           (input) - protocol version.
 *
 * Returns:
 *   void
 */
void
ndmpSetVersion(NdmpConnection	connectionHandle,
			   u_short			version)
{
	((Connection *)connectionHandle)->version = version;
}

/*
 * ndmpGetVersion
 *   Gets the NDMP protocol version in use on the connection.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   version           (input) - protocol version.
 *
 * Returns:
 *   void
 */
u_short
ndmpGetVersion(NdmpConnection	connectionHandle)
{
	return(((Connection *)connectionHandle)->version);
}

/*
 * ndmpSetAuthorized
 *   Mark the connection as either having been authorized or not.
 *
 * Parameters:
 *   connectionHandle  (input) - connection handle.
 *   authorized        (input) - TRUE or FALSE.
 *
 * Returns:
 *   void
 */
void
ndmpSetAuthorized(NdmpConnection	connectionHandle,
				  bool_t			authorized)
{
	((Connection *)connectionHandle)->authorized = authorized;
}

u_longlong_t
quadToLongLong(ndmp_u_quad	quad)
{
	u_longlong_t	ull;
	ull = ((u_longlong_t)quad.high << 32) +	quad.low;
	return(ull);
}

ndmp_u_quad
longLongToQuad(u_longlong_t	ull)
{
	ndmp_u_quad	quad;

	quad.high = (u_long)(ull >> 32);
	quad.low  = (u_long)ull;
	return(quad);
}

/************** private functions ****************************************/

/*
 * ndmp_readit
 *
 * Low level read routine called by the xdrrec library.
 *
 * Parameters:
 *   connection (input) - connection pointer.
 *   buf        (input) - location to store received data.
 *   len        (input) - max number of bytes to read.
 *
 * Returns:
 *   >0 - number of bytes received.
 *   -1 - error.
 */
static int
ndmp_readit(void*		connectionHandle,
			caddr_t		buf,
			int			len)
{
	Connection	*connection = (Connection *)connectionHandle;

	Debug(DBG_CAT_COMM|DBG_FOC_DETAIL,
		  "ndmp_readit: len:%d\n", len);

	len = read(connection->sock, buf, len);
/*
	printf("Khushal : ndmp_readit: len %d\n", len);
*/
	if (len <= 0)
	{
		/* Connection has been closed. */
		Error(LOG_ERR, "ndmp_readit: connection->eof = TRUE\n");
		connection->eof = TRUE;
		return(-1);
	}

	return(len);
}

/*
 * ndmp_writeit
 *
 * Low level write routine called by the xdrrec library.
 *
 * Parameters:
 *   connection (input) - connection pointer.
 *   buf        (input) - location to store received data.
 *   len        (input) - max number of bytes to read.
 *
 * Returns:
 *   >0 - number of bytes sent.
 *   -1 - error.
 */
static int
ndmp_writeit(void*		connectionHandle,
			 caddr_t	buf,
			 int		len)
{
	Connection		*connection = (Connection *)connectionHandle;
	register int	n;
	register int	cnt;

	Debug(DBG_CAT_COMM|DBG_FOC_DETAIL,
		  "ndmp_writeit: len:%d\n", len);

	for(cnt = len; cnt > 0; cnt -= n, buf += n)
	{
		if ((n = write(connection->sock, buf, cnt)) < 0)
		{
			Error(LOG_ERR, "ndmp_writeit: connection->eof = TRUE\n");
			connection->eof = TRUE;
			return (-1);
		}
	}

	return(len);
}

/*
 * ndmp_recv_msg
 *
 * Read the next message.
 *
 * Parameters:
 *   connection (input)  - connection pointer.
 *   msg        (output) - received message.
 *
 * Returns:
 *   0 - Message successfully received.
 *   error number - Message related error.
 *  -1 - Error decoding the message header.
 */
static int
ndmp_recv_msg(Connection*	connection)
{
	bool_t	(*xdr_func)(XDR*, ...) = 0;

	Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
		  "ndmp_recv_msg\n");

	/* Decode the header. */
	connection->xdrs.x_op = XDR_DECODE;
	xdrrec_skiprecord(&connection->xdrs);
	if (!xdr_ndmp_header(&connection->xdrs, &connection->msginfo.hdr))
	{
		return(-1);
	}

	/* Lookup info necessary for processing this message. */
	if ((connection->msginfo.handler =
		  ndmp_get_handler(connection,
						   connection->msginfo.hdr.message_code)) == 0)
	{
		Error(LOG_ERR, "message 0x%x not supported.\n",
			  connection->msginfo.hdr.message_code);
		return(NDMP_NOT_SUPPORTED_ERR);
	}
  
	connection->msginfo.body		= 0;

	if (connection->msginfo.hdr.error_code != NDMP_NO_ERR)
		return(0);
	
	/* Determine body type */
	if (connection->msginfo.hdr.message_type == NDMP_MESSAGE_REQUEST)
	{
		if (connection->msginfo.handler->auth_required &&
			!connection->authorized)
		{
			Error(LOG_ERR,
				  "processing request 0x%x: connection not authorized.\n",
				  connection->msginfo.hdr.message_code);
			return(NDMP_NOT_AUTHORIZED_ERR);
		}
		
		if (connection->msginfo.handler->sizeof_request > 0)
		{
			xdr_func		= connection->msginfo.handler->xdr_request;
			if (xdr_func == 0)
			{
				Error(LOG_ERR,
					  "processing request 0x%x: no xdr function in handler table.\n",
					  connection->msginfo.hdr.message_code);
				return(NDMP_NOT_SUPPORTED_ERR);
			}
			
			connection->msginfo.body =
				(void *)malloc(connection->msginfo.handler->sizeof_request);
			if (connection->msginfo.body == 0)
			{
				Error(LOG_ERR,
					  "processing request 0x%x: system call (malloc): %s\n",
					  connection->msginfo.hdr.message_code, strerror(errno));
				return(NDMP_NO_MEM_ERR);
			}
			memset(connection->msginfo.body, 0,
				   connection->msginfo.handler->sizeof_request);
		}
	}
	else
	{
		if (connection->msginfo.handler->sizeof_reply > 0)
		{
			xdr_func = connection->msginfo.handler->xdr_reply;
			if (xdr_func == 0)
			{
				Error(LOG_ERR,
					  "processing reply 0x%x: no xdr function in handler table.\n",
					  connection->msginfo.hdr.message_code);
				return(NDMP_NOT_SUPPORTED_ERR);
			}

			connection->msginfo.body =
				(void *)malloc(connection->msginfo.handler->sizeof_reply);
			if (connection->msginfo.body == 0)
			{
				Error(LOG_ERR,
					  "processing reply 0x%x: system call (malloc): %s\n",
					  connection->msginfo.hdr.message_code, strerror(errno));
				return(NDMP_NO_MEM_ERR);
			}
			memset(connection->msginfo.body, 0,
				   connection->msginfo.handler->sizeof_reply);
		}
	}

	/* Decode message arguments if needed */
	if (xdr_func)
	{
		if (!(*xdr_func)(&connection->xdrs,
						 connection->msginfo.body))
		{
			Error(LOG_ERR,
				  "processing message 0x%x: error decoding arguments.\n",
				  connection->msginfo.hdr.message_code);
			free(connection->msginfo.body);
			connection->msginfo.body = 0;
			return(NDMP_XDR_DECODE_ERR);
		}
	}
/*
	printf("Khushal: ndmp_recv_msg: msg type is 0x%x\n",
	    connection->msginfo.hdr.message_code);
*/
	if (connection->msginfo.hdr.message_code == NDMP_LOG_MESSAGE ) {
/*
		printf("Khushal: ndmp_recv_msg: message is %s\n",
		    ((ndmp_log_message_post *)(connection->msginfo.body))->entry);
*/
	}
	if (connection->msginfo.hdr.message_code == NDMP_LOG_FILE ) {
/*
		printf("Khushal: ndmp_recv_msg: message is %s\n",
		    ((ndmp_log_file_post *)(connection->msginfo.body))->name);
*/
	}
	return(0);
}

/*
 * ndmp_process_messages
 *
 * Reads the next message into the stream buffer.
 * Processes messages until the stream buffer is empty.
 *
 * This function processes all data in the stream buffer before returning.
 * This allows functions like poll() to be used to determine when new 
 * messages have arrived. If only some of the messages in the stream buffer
 * were processed and then poll was called, poll() could block waiting for
 * a message that had already been received and read into the stream buffer.
 *
 * This function processes both request and reply messages.
 * Request messages are dispatched using the appropriate function from the
 * message handling table.
 * Only one reply messages may be pending receipt at a time.
 * A reply message, if received, is placed in connection->msginfo
 * before returning to the caller.
 * Errors are reported if a reply is received but not expected or if
 * more than one reply message is received
 *
 * Parameters:
 *   connection     (input)  - connection pointer.
 *   reply_expected (output) - TRUE  - a reply message is expected.
 *                             FALSE - no reply message is expected and
 *                             an error will be reported if a reply
 *                             is received.
 *
 * Returns:
 *   2 - 1 or messages successfully processed, error processing reply message.
 *   1 - 1 or messages successfully processed, reply seen.
 *   0 - 1 or more messages successfully processed, no reply seen.
 *  -1 - error; connection no longer established.
 *
 * Notes:
 *   If the peer is generating a large number of requests, a caller
 *   looking for a reply will be blocked while the requests are handled.
 *   This is because this function does not return until the stream
 *   buffer is empty.
 *   Code needs to be added to allow a return if the stream buffer
 *   is not empty but there is data available on the socket. This will
 *   prevent poll() from blocking and prevent a caller looking for a reply
 *   from getting blocked by a bunch of requests.
 */
static int
ndmp_process_messages(Connection*	connection,
					  bool_t		reply_expected)
{
	MsgInfo		reply_msginfo;
	bool_t		reply_read = FALSE;
	bool_t		reply_error = FALSE;
	int			err;
	
	Debug(DBG_CAT_COMM|DBG_FOC_DETAIL,
		  "ndmp_process_messages: reply_expected:%s\n",
		  reply_expected == TRUE ? "TRUE" : "FALSE");

	do
	{
		memset((void *)&connection->msginfo, 0, sizeof(MsgInfo));
	
		if ((err = ndmp_recv_msg(connection)) != NDMP_NO_ERR)
		{
			if (connection->eof)
			{
				Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
					  "ndmp_process_messages: detected eof\n");
				Error(LOG_ERR, "ndmp_process_messages: detected eof\n");
				return(-1);
			}
			
			if (err < 1)
			{
				Debug(DBG_CAT_COMM|DBG_FOC_DETAIL,
					  "ndmp_process_messages: error decoding header\n");

				/*
				 * Error occurred decoding the header.
				 * Don't send a reply since we don't know the message
				 * or if the message was even a request message.
				 *
				 * To be safe, assume that the message was a reply
				 * if a reply was expected. Need to do this to prevent
				 * hanging ndmpSendRequest() waiting for a reply. 
				 * Don't set reply_read so that the reply will be
				 * processed if it received later.
				 */
				if (reply_read == FALSE)
					reply_error = TRUE;
				continue;
			}
			
			if (connection->msginfo.hdr.message_type != NDMP_MESSAGE_REQUEST)
			{
				Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
					  "ndmp_process_messages: received reply: 0x%x\n",
					  connection->msginfo.hdr.message_code);

				if (reply_expected == 0 || reply_read == TRUE)
					Error(LOG_ERR, "unexpected reply message: 0x%x.\n",
						  connection->msginfo.hdr.message_code);

				ndmpFreeMessage((NdmpConnection)connection);

				if (reply_read == FALSE)
				{
					reply_read = TRUE;
					reply_error = TRUE;
				}
				
				continue;
			} else {
				if (p_handlePostMessage != NULL)
					p_handlePostMessage(connection);
			}
			
			Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
				  "ndmp_process_messages: received request: 0x%x\n",
				  connection->msginfo.hdr.message_code);

			ndmpSendReply((NdmpConnection)connection, err, 0);
			ndmpFreeMessage((NdmpConnection)connection);
			continue;
		}
			
		if (connection->msginfo.hdr.message_type != NDMP_MESSAGE_REQUEST)
		{
			Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
				  "ndmp_process_messages: received reply: 0x%x\n",
				  connection->msginfo.hdr.message_code);

			if (reply_expected == 0 || reply_read == TRUE)
			{
				Error(LOG_ERR, "unexpected reply message: 0x%x.\n",
					  connection->msginfo.hdr.message_code);
				ndmpFreeMessage((NdmpConnection)connection);
				continue;
			}
			
			reply_read = TRUE;
			reply_msginfo = connection->msginfo;
			continue;
		} else {
			if (p_handlePostMessage != NULL)
				p_handlePostMessage(connection);
		}

		Debug(DBG_CAT_COMM|DBG_FOC_FLOW,
			  "ndmp_process_messages: received request: 0x%x\n",
			  connection->msginfo.hdr.message_code);

		/*
		 * The following needed to catch an improperly constructed
		 * handler table or to deal with an NDMP client that is not
		 * conforming to the negotiated protocol version.
		 */
		if (connection->msginfo.handler->func == 0)
		{
			Error(LOG_ERR,
				  "No handler for message 0x%x\n",
				  connection->msginfo.hdr.message_code);

			ndmpSendReply((NdmpConnection)connection, NDMP_NOT_SUPPORTED_ERR, 0);
			ndmpFreeMessage((NdmpConnection)connection);
			continue;
		}

		/*
		 * Call the handler function.
		 * The handler will send any necessary reply.
		 */
		(*connection->msginfo.handler->func)(connection,
											 connection->msginfo.body);

		ndmpFreeMessage((NdmpConnection)connection);

	} while (xdrrec_eof(&connection->xdrs) == FALSE &&
			 connection->eof == FALSE);

	Debug(DBG_CAT_COMM|DBG_FOC_DETAIL,
		  "ndmp_process_messages: no more messages in stream buffer\n");

	if (connection->eof == TRUE)
		return(-1);
	
	if (reply_error)
		return(2);

	if (reply_read)
	{
		connection->msginfo = reply_msginfo;
		return(1);
	}
	
	return(0);
}

/*
 * ndmp_get_handler_info
 *
 * Return the handler info for the specified NDMP procedure.
 *
 * Parameters:
 *   procedure (input) - procedure number.
 *
 * Returns:
 *   0 - procedure not found.
 *   pointer to handler info.
 */
NdmpMsgHandler*
ndmp_get_handler(Connection*	connection,
				 u_short		procedure)
{
	NdmpMsgHandler		*handler;

	for (handler = connection->msgHandlerTbl;
		 handler->message != 0;
		 handler++)
	{
		if (handler->message == procedure &&
			handler->version == connection->version)
			return(handler);
    }

	return(0);
}

/*
 * sethandlePostMessage() :
 *	Sets the pointer handlePostMessage() function.
 *  This will help ndmp_process_message() call it. handlepostmessage()
 *  is the one who handles the post messages.
 *
 *  Returns :
 *	0 - Success
 *	1 - Failure
 */
int
sethandlePostMessage(type_handlePostMessage *fp)
{
	if (fp != NULL) {
		p_handlePostMessage = fp;
		return (0);
	} else {
		return (1);
	}
}

/*
 * End of file	:	comm.c
 */
