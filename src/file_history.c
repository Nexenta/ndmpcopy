/*                               -*- Mode: C -*- 
 * file_history.c
 * 
 * Description     : File history request handlers.
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
 * We don't really use the file history information.  These callbacks
 * are included for completeness and really just log the event if the
 * user wants verbose output.
 */

#include "ndmp_common.h"
#include "queue.h"
#include "main.h"


extern struct cmd_line_opts             opts;

/*
 * fileHistoryAddUnix
 *   fh_add_unix request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
fileHistoryAddUnix(NdmpConnection connection, void *body)
{
    ndmp_fh_add_file_post *request =
				(ndmp_fh_add_file_post *)body;

    MsgQueue    *backend_queue = ndmpGetClientData(connection);
    MsgData             backend_msg;


    if (opts.verbose >= DEBUG_VERBOSE_LEVEL) {



	backend_msg.message = NDMP_FH_ADD_FILE;
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);
    }

}


/*
 * fileHistoryAddUnixDir
 *   fh_add_unix_dir request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
fileHistoryAddUnixDir(NdmpConnection connection, void *body)
{
    ndmp_fh_add_dir_post*	request =
		(ndmp_fh_add_dir_post *)body;
    MsgQueue    *backend_queue = ndmpGetClientData(connection);
    MsgData             backend_msg;


    if (opts.verbose >= DEBUG_VERBOSE_LEVEL) {
	sprintf(backend_msg.text, "Received fh_add_unix_dir request: %lu "
			  "entries.\n", (unsigned long) request->dirs.dirs_len);

	backend_msg.message = NDMP_FH_ADD_DIR;
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);
    }

}


/*
 * fileHistoryAddUnixNode
 *   fh_add_unix_node request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
fileHistoryAddUnixNode(NdmpConnection connection, void *body) 
{

    ndmp_fh_add_node_post*	request =
	    (ndmp_fh_add_node_post *)body;

    MsgQueue    *backend_queue = ndmpGetClientData(connection);
    MsgData             backend_msg;

    if (opts.verbose >= DEBUG_VERBOSE_LEVEL) {
	sprintf(backend_msg.text, "Received fh_add_unix_node request: %lu "
		      "entries.\n", (unsigned long) request->nodes.nodes_len);

	backend_msg.message = NDMP_FH_ADD_NODE;
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);
    }
}

