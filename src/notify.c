/*                               -*- Mode: C -*- 
 * notify.c
 * 
 * Description     : Notify request handlers.
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


/*
 * The majority of this code is taken from the NDMP SDK client toolkit.
 */

#include "ndmp_common.h"
#include "queue.h"
#include "main.h"

extern struct cmd_line_opts		opts;

/*
 * notifyConnected
 *   notify_connected request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyConnected(NdmpConnection connection, void *body)
{
    ndmp_notify_connection_status_post*	request =
	    (ndmp_notify_connection_status_post *)body;
    MsgQueue	*backend_queue = ndmpGetClientData(connection);
    MsgData		backend_msg;

    if (opts.verbose >= DEBUG_VERBOSE_LEVEL) {
	backend_msg.message = NDMP_NOTIFY_CONNECTION_STATUS;
	backend_msg.reason  = request->reason;
	sprintf(backend_msg.text, "Received notify_connect request: "
				  "version: %ld, %s\n",
				   (long)request->protocol_version,
				    request->text_reason);
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);
    }

    if (opts.verbose >= LOG_VERBOSE_LEVEL) {
	switch (request->reason)
	{
	    case NDMP_CONNECTED:
		strcpy(backend_msg.text, "CONNECT: Connection established.");
		break;

	    case NDMP_SHUTDOWN:
		strcpy(backend_msg.text, "CONNECT: Connection shutdown.");
		break;

	    case NDMP_REFUSED:
		strcpy(backend_msg.text, "CONNECT: Connection refused.");
		break;

	    default:
		strcpy(backend_msg.text, "CONNECT: Unknown error.");
		break;
	}

	backend_msg.message = NDMP_NOTIFY_CONNECTION_STATUS;
	backend_msg.reason  = request->reason;
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);

    }
}

/*
 * notifyDataHalted
 *   notify_data_halted request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyDataHalted(NdmpConnection connection, void *body)
{
    ndmp_notify_data_halted_post*	request =
	    (ndmp_notify_data_halted_post *)body;
    
    MsgQueue	*backend_queue = ndmpGetClientData(connection);
    MsgData		backend_msg;

    switch (request->reason)
    {
	case NDMP_DATA_HALT_NA:
	    sprintf(backend_msg.text,"NA");
	    break;

	case NDMP_DATA_HALT_SUCCESSFUL:
	    sprintf(backend_msg.text,"HALT: The operation was successful!");
	    break;

	case NDMP_DATA_HALT_ABORTED:
	    sprintf(backend_msg.text,"HALT: The operation was aborted!");
	    break;

	case NDMP_DATA_HALT_INTERNAL_ERROR:
	    sprintf(backend_msg.text,"HALT: Internal error!");
	    break;

	case NDMP_DATA_HALT_CONNECT_ERROR:
	    sprintf(backend_msg.text,"HALT: Connect error!");
	    break;

	default:
	    sprintf(backend_msg.text,"HALT: Unknown error.");
	    break;
    }

    backend_msg.message = NDMP_NOTIFY_DATA_HALTED;
    backend_msg.reason  = request->reason;
    backend_msg.connection  = connection;
    enqueue(backend_queue, &backend_msg);

    return;
}

/*
 * notifyMoverHalted
 *   notify_mover_halted request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyMoverHalted(NdmpConnection	connection,
				  void*				body)
{
    ndmp_notify_mover_halted_post*	request =
	    (ndmp_notify_mover_halted_post *)body;
    
    MsgData backend_msg;
    MsgQueue	*backend_queue = ndmpGetClientData(connection);

    if (opts.verbose >= DEBUG_VERBOSE_LEVEL) {


	backend_msg.message = NDMP_NOTIFY_MOVER_HALTED;
	backend_msg.reason  = request->reason;
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);
    }
	
    switch (request->reason)
    {
	case NDMP_MOVER_HALT_NA:
	    sprintf(backend_msg.text, "MOVER_HALT: NA.");
	    break;

	case NDMP_MOVER_HALT_CONNECT_CLOSED:
	    sprintf(backend_msg.text, "MOVER_HALT: Connection closed.");
	    break;

	case NDMP_MOVER_HALT_ABORTED:
	    sprintf(backend_msg.text, "MOVER_HALT: Aborted."); 
	    break;

	case NDMP_MOVER_HALT_INTERNAL_ERROR:
	    sprintf(backend_msg.text, "MOVER_HALT: Internal error."); 
	    break;

	case NDMP_MOVER_HALT_CONNECT_ERROR:
	    sprintf(backend_msg.text, "MOVER_HALT: Connection error."); 
	    break;

	default:
	    sprintf(backend_msg.text, "MOVER_HALT: Undefined mover error."); 
	    break;
    }
			
    backend_msg.message = NDMP_NOTIFY_MOVER_HALTED;
    backend_msg.reason  = request->reason;
    backend_msg.connection  = connection;
    enqueue(backend_queue, &backend_msg);

    return;
}

/*
 * notifyMoverPaused
 *   notify_mover_paused request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyMoverPaused(NdmpConnection connection, void *body)
{
    ndmp_notify_mover_paused_post*	request =
	    (ndmp_notify_mover_paused_post *)body;
    MsgData backend_msg;
    MsgQueue	*backend_queue = ndmpGetClientData(connection);

    switch (request->reason)
    {
	case NDMP_MOVER_PAUSE_NA:
	    sprintf(backend_msg.text, "MOVER_PAUSE: NA.");
	    break;

	case NDMP_MOVER_PAUSE_EOM:
	    sprintf(backend_msg.text, "MOVER_PAUSE: End of Media.");
	    break;

	case NDMP_MOVER_PAUSE_EOF:
	    sprintf(backend_msg.text, "MOVER_PAUSE: End of File.");
	    break;

	case NDMP_MOVER_PAUSE_SEEK:
	    sprintf(backend_msg.text, "MOVER_PAUSE: Seek Error.");
	    break;

	case NDMP_MOVER_PAUSE_EOW:
	    sprintf(backend_msg.text, "MOVER_PAUSE: Media Error.");
	    break;

	default:
	    sprintf(backend_msg.text, "MOVER_PAUSE: Unknown Error.");
	    break;
    }

    backend_msg.message = NDMP_NOTIFY_MOVER_PAUSED;
    backend_msg.reason  = request->reason;
    backend_msg.connection  = connection;
    enqueue(backend_queue, &backend_msg);

    return;
}
/*
 * notifyDataRead
 *   notify_data_read request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyDataRead(NdmpConnection connection, void *body)
{
        ndmp_notify_data_read_post *request =
            (ndmp_notify_data_read_post *) body;

        MsgData backend_msg;
        MsgQueue *backend_queue = ndmpGetClientData(connection);

        if (opts.verbose >= DEBUG_VERBOSE_LEVEL) {
                sprintf(backend_msg.text, "Received notify_data_read: "
                    "offset:%lld len:%lld\n",
                    (unsigned long long) quadToLongLong(request->offset),
                    (unsigned long long) quadToLongLong(request->length));
        }

        backend_msg.message = NDMP_NOTIFY_DATA_READ;
        backend_msg.connection = connection;
        enqueue(backend_queue, &backend_msg);

}

/*
 * End of file	:	notify.c
 */
