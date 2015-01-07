/*
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

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "main.h"
#include "dump.h"
#include "queue.h"
#include "ndmp_common.h"

extern NdmpMsgHandler		ndmp_msg_handler_tbl[];
extern struct cmd_line_opts	opts;

static NdmpConnection		rstr_connection;
NdmpConnection		dump_connection;
static ndmp_pval		environment[9];
static MsgQueue			backend_queue;

int check_notification(NdmpConnection, ndmp_message);
int data_get_state(NdmpConnection, ndmp_data_get_state_reply *);
/* establish_connection():
 *   Description:
 *      Connects the NDMP channels "dump_connection" and
 *      "rstr_connection" (globals) to the source and destination
 *      hosts specified in the "opts" struct (global, filled in by
 *      main()).  NDMP authentication information is also taken from
 *      the opts structure.
 *
 *   Inputs:
 *
 *   Outputs:
 *      ndmp_addr *mover_address -  address returned by recover
 *                                        operation
 *
 *   Returns:
 *      ndmp_error
 */

int establish_connection()
{
    ndmp_mover_listen_request	mover_request;
    ndmp_mover_listen_reply	*mover_reply;
    int				r;
    if (ndmp_open_connection(&dump_connection, opts.src_host, 
				    opts.ndmp_src_port, opts.src_auth) != 0)
	return -1;

    if (ndmp_open_connection(&rstr_connection, opts.dest_host,
				    opts.ndmp_dest_port, opts.dest_auth) != 0)
	return -1;
    ndmpFreeMessage(rstr_connection);
    return 0;
}

/* shutdown_connection():
 *   Description:
 *      waits for any outstanding messages, then closes the
 *      NDMP channels "dump_connection" and  "rstr_connection"
 *      (globals), and finally destroys the channels.
 *
 *   Inputs:
 *      NONE
 *
 *   Outputs:
 *      NONE
 *
 *   Returns:
 *      0 on success, -1 on failure
 */

int shutdown_connection()
{
    int r;

    /* flush ndmp messages on both connections */

    while (ndmpPoll(dump_connection, FALSE) ||
	   ndmpPoll(rstr_connection, FALSE))
        ; /* Do nothing */


    /* Send close connection requests to both connections */

    r = ndmpSendRequest(rstr_connection, NDMP_CONNECT_CLOSE, NDMP_NO_ERR,
								NULL, NULL);
    if (r != 0) {
	fprintf(stderr, "Error closing restore connection: %s\n",
							    ndmpErrToStr(r));
	return -1;
    }
    ndmpFreeMessage(rstr_connection);

    r = ndmpSendRequest(dump_connection, NDMP_CONNECT_CLOSE, NDMP_NO_ERR,
								NULL, NULL);
    if (r != 0) {
	fprintf(stderr, "Error closing dump connection: %s\n",
							    ndmpErrToStr(r));
	return -1;
    }
    ndmpFreeMessage(dump_connection);

    /* close and destroy connections */

    ndmpDestroyConnection(rstr_connection);
    ndmpDestroyConnection(dump_connection);

    return 0;
}

/* ndmp_dump():
 *   Description:
 *      Sets up the environment and sends an NDMP_START_BACKUP
 *      request to the source machine (dump_connection) using the
 *      mover address of the destination machine returned by 
 *      establish_connection().  
 *
 *   Inputs:
 *      ndmp_addr *mover_address -  address to send dump data to
 *                        (usually obtained from establish_connection())
 *
 *   Outputs:
 *
 *   Returns:
 *      0 on success, -1 on error.
 */

int ndmp_dump()
{
    int	r;
    ndmp_data_start_backup_request dump_request;
    ndmp_data_start_backup_reply *dump_reply = 0;

    environment[0].name = "UPDATE";
    environment[0].value = "Y";
    environment[1].name = "HIST";
    environment[1].value = "Y";
    environment[2].name = "FILESYSTEM";
    environment[2].value = opts.src_dir;
    environment[3].name = "LEVEL";
    environment[3].value = opts.level;
    environment[4].name = "DIRECT";
    environment[4].value = "Y";

    dump_request.env.env_val = environment;
    dump_request.env.env_len = 5;
    dump_request.butype_name = "dump";       /* XXX may make this an option later */
    /* allow destination mover address to be overridden on the command line */
    r = ndmpSendRequest(dump_connection, NDMP_DATA_START_BACKUP, NDMP_NO_ERR,
				    (void *)&dump_request, (void *)&dump_reply);
    if (r != 0) {
	fprintf(stderr, "Error starting backup, error: %s\n", ndmpErrToStr(r));
	return -1;
    }
    
    if (opts.verbose >= DEBUG_VERBOSE_LEVEL)
		fprintf(stderr, "DUMP: Done! returning.\n");
    check_notification(rstr_connection, NDMP_NOTIFY_DATA_HALTED);
    return (0);
}

/* ndmp_restore():
 *   Description:
 *      Sends an NDMP_START_RECOVER request to the destination machine.
 *      It assumes that the connections have been established with the
 *      establish_connection() call, and the appropriate command line
 *      arguments and defaults have been processed in the global opts
 *      structure.
 *      
 *   Inputs:
 *      None.
 *
 *   Outputs:
 *      None.
 *
 *   Returns:
 *      0 on success, -1 on error.
 */

int ndmp_restore()
{
        ndmp_data_start_recover_request restore_request;
        ndmp_data_start_recover_reply *restore_reply = 0;
        int r;
        static ndmp_name file_list[5];

        environment[0].name = "UPDATE";
        environment[0].value = "Y";
        environment[1].name = "HIST";
        environment[1].value = "Y";
        environment[2].name = "FILESYSTEM";
        environment[2].value = opts.src_dir;
        environment[3].name = "LEVEL";
        environment[3].value = opts.level;
        environment[4].name = "DIRECT";
        environment[4].value = "N";
        environment[5].name = "EXTRACT";
        environment[5].value = "Y";
        environment[6].name = "EXTRACT_ACL";
        environment[6].value = "Y";
        environment[7].name = "RECURSIVE";
        environment[7].value = "Y";
        environment[8].name = "DIRECT";
        environment[8].value = "N";

        file_list[0].name = "/";
        file_list[0].destination_dir = opts.dest_dir;
        file_list[0].fh_info = longLongToQuad(0LL);

        /* Because the connection was already established with the mover_listen,
         * we don't need to supply a mover address, but we DO need to tell it
         * that it's a TCP connection and not to read from tape.
         */

        restore_request.env.env_val = environment;
        restore_request.env.env_len = 7;
        restore_request.butype_name = "dump";
        restore_request.nlist.nlist_val = file_list;
        restore_request.nlist.nlist_len = 1;

        if (opts.verbose >= DEBUG_VERBOSE_LEVEL)
                fprintf(stderr, "\nRESTORE: Sending recover request.\n");

        r = ndmpSendRequest(rstr_connection, NDMP_DATA_START_RECOVER, NDMP_NO_ERR,
            (void *) & restore_request, (void *) & restore_reply);
        if (r != 0) {
                fprintf(stderr, "Error sending restore request, error: %s\n",
                    ndmpErrToStr(r));
                return -1;
        }


        if (opts.verbose >= DEBUG_VERBOSE_LEVEL)
                fprintf(stderr, "RESTORE: Done! returning.\n");

	ndmpFreeMessage(rstr_connection);
        /* should wait for NOTIFY_DATA_READ */
        check_notification(rstr_connection, NDMP_NOTIFY_DATA_READ);
        return 0;
}

/* ndmp_open_connection():
 *   Description:
 *      Establishes an NDMP connection to "dest_host" on port "port"
 *      using the provided authentication information.  That this
 *      should be used to establish the NDMP protocol connection only,
 *      the data connection is established by the MOVER_LISTEN and
 *      DATA_START_BACKUP messages sent once the NDMP channels have
 *      been opened.
 *      
 *   Inputs:
 *      char *dest_host   -  The name or IP address of the destination host
 *      int port          -  The network port to connect to on the destination
 *      struct auth_info ainfo - the authentication information needed
 *                               to open the connection.
 *
 *   Outputs:
 *      NdmpConnection *ndmp_connection - The handle of the connection
 *                                        established by this routine.
 *   Returns:
 *      0 on success, -1 on error.
 */

int ndmp_open_connection(NdmpConnection *ndmp_connection, char *dest_host, int port, struct auth_info ainfo)
{
    ndmp_connect_open_request			open_request;
    ndmp_connect_open_reply				open_reply;
    ndmp_connect_client_auth_request	auth_request;
    ndmp_connect_client_auth_reply		auth_reply;
    int									r;

    if (opts.verbose >= DEBUG_VERBOSE_LEVEL)
	fprintf(stderr, "Creating NDMP connection.\n");

    /* Open Connection */
    if ((*ndmp_connection = ndmpCreateConnection(ndmp_msg_handler_tbl)) == 0) {
        fprintf(stderr, "Could not create NDMP connection!\n");
	return -1;
    }

    ndmpSetClientData(*ndmp_connection, &backend_queue);

#ifdef DEBUG
    fprintf(stderr, "Connecting to %s.\n", dest_host);
#endif

    if (ndmpConnect(*ndmp_connection, dest_host, port) != 0) {
        fprintf(stderr, "Could not open NDMP connection to host %s.\n",
								dest_host);
	return -1;
    }

    /* open NDMP connection */

    open_request.protocol_version = NDMPVER; /* should be at least 2 */
    r = ndmpSendRequest(*ndmp_connection, NDMP_CONNECT_OPEN, NDMP_NO_ERR,
				(void *) &open_request, (void *) &open_reply);
    if (r != 0) {
	fprintf(stderr,"Error opening NDMP connection to host %s, error: %s.\n",
						    dest_host, ndmpErrToStr(r));
	return -1;
    }
    ndmpSetVersion(*ndmp_connection, open_request.protocol_version);
    ndmpFreeMessage(*ndmp_connection);

    /* Authorize connection */

    if (opts.verbose >= DEBUG_VERBOSE_LEVEL) {
	fprintf(stderr, "Authorizing connection with %s authorization.\n",
			    ainfo.type == NDMP_AUTH_TEXT ? "TEXT" : "NONE");
	if (ainfo.type == NDMP_AUTH_TEXT) 
	    fprintf(stderr, "    user = %s, password = %s.\n",
						    ainfo.user, ainfo.password);
    }

    if (ainfo.type == NDMP_AUTH_TEXT) {
	auth_request.auth_data.auth_type = NDMP_AUTH_TEXT;
	auth_request.auth_data.ndmp_auth_data_u.auth_text.auth_id = ainfo.user;
	auth_request.auth_data.ndmp_auth_data_u.auth_text.auth_password = ainfo.password;
	r = ndmpSendRequest(*ndmp_connection, NDMP_CONNECT_CLIENT_AUTH, NDMP_NO_ERR,
				(void *)&auth_request, (void *) &auth_reply);
	if (r != 0) {
	    fprintf(stderr, "Error sending text authentication: %s.\n",
							    ndmpErrToStr(r));
	    return -1;
	}

	if (opts.verbose >= DEBUG_VERBOSE_LEVEL)
	    fprintf(stderr, "Authenticated root with NDMP_AUTH_TEXT.\n");

    }
    else if (ainfo.type == NDMP_AUTH_NONE) {
	auth_request.auth_data.auth_type = NDMP_AUTH_NONE;
	r = ndmpSendRequest(*ndmp_connection, NDMP_CONNECT_CLIENT_AUTH, NDMP_NO_ERR,
			    (void *) &auth_request, (void *) &auth_reply);
	if (r != 0) {
	    fprintf(stderr, "Error sending NONE authentication, %s.\n", ndmpErrToStr(r));
	    return -1;
	}

	if (opts.verbose >= DEBUG_VERBOSE_LEVEL)
	    fprintf(stderr, "Authenticated with NDMP_AUTH_NONE.\n");
    }
    else {
	fprintf(stderr, "Invalid authentication type: %d.\n", ainfo.type);
	return -1;
    }

    ndmpSetAuthorized(*ndmp_connection, TRUE);
    ndmpFreeMessage(*ndmp_connection);

    return 0;
}

/* process_requests():
 *   Description:
 *      Listens on rstr_connection and dump_connection for incoming
 *      messages and handles them appropriately.  The primary messages
 *      that it attends to are halted messages, indicating that one
 *      side is either done or encountered an error.  It communicates
 *      with the backend protocol through a queue system.  The
 *      backend processing is all done through the switch table passed
 *      to ndmpCreateConnection().
 *      
 *   Inputs:
 *      starttime	-	Time copy was started.
 *
 *   Outputs:
 *      None.
 *
 *   Returns:
 *      -1 on error, 0 on success.
 */

int process_requests(time_t starttime)
{
    fd_set		conn_fds;
    int got_data_halted = 0;    /* everything is done when data and */
    int got_mover_halted = 0;   /* mover have both halted           */
    MsgData	msg;
    struct timeval timeout;
	time_t got_mover_time = 0; /* when got_mover_halted in hung server case */

	while (1) {

	/* work-around NetApp bug id 7134 so we don't wait forever,
	 * three minutes should be long enough in most cases... */
	if (got_mover_halted && got_mover_time &&
		(time(NULL) > got_mover_time + (time_t) 180)) {
		fprintf(stderr, "Data server never halted, closing connection.\n");
		shutdown_connection();
		return 0;
	}

	/* reinitialize timeout each time it is used because Linux modifies
	 * timeout to reflect the amount of time not slept */
	timeout.tv_sec = 1L;
	timeout.tv_usec = 0L;

	FD_ZERO(&conn_fds);
	FD_SET(ndmpGetFd(rstr_connection), &conn_fds);
	FD_SET(ndmpGetFd(dump_connection), &conn_fds);

	if (select(MAX(ndmpGetFd(rstr_connection),ndmpGetFd(dump_connection))+1,
				&conn_fds, NULL, NULL, &timeout) < 0) {

	    fprintf(stderr, "process_requests: error selecting on connection sockets: %s\n", strerror(errno));
	    return -1;
	}

	if (FD_ISSET(ndmpGetFd(dump_connection), &conn_fds) &&
			opts.verbose >= LOG_VERBOSE_LEVEL) {
	    if (ndmpPoll(dump_connection, FALSE) < 0) {
		fprintf(stderr, "Error processing NDMP requests.\n");
		return -1;
	    }
	}

	if (FD_ISSET(ndmpGetFd(rstr_connection), &conn_fds) &&
			opts.verbose >= LOG_VERBOSE_LEVEL) {
	    if (ndmpPoll(rstr_connection, FALSE) < 0) {
		fprintf(stderr, "Error processing NDMP requests.\n");
		return -1;
	    }
	}

	/* process any messages that came in */
	if (dequeue((MsgQueue *)ndmpGetClientData(rstr_connection), &msg) || 
	   (dequeue((MsgQueue *)ndmpGetClientData(dump_connection), &msg))) {

	    switch(msg.message) {
	    case NDMP_NOTIFY_DATA_HALTED:

			if ((msg.connection == dump_connection) &&
				(msg.reason == NDMP_DATA_HALT_SUCCESSFUL)) {
				got_data_halted = 1;
				if (opts.verbose >= LOG_VERBOSE_LEVEL)
					fprintf(stderr,"%s: %s\n", opts.src_host,msg.text);
				if (!got_mover_halted && opts.verbose >= LOG_VERBOSE_LEVEL)
					fprintf(stderr, "Waiting for %s to halt too.\n",
										opts.dest_host);
				} else if ((msg.connection == rstr_connection) && 
							(msg.reason == NDMP_DATA_HALT_SUCCESSFUL)) {
					got_mover_halted = 1;
					if (opts.verbose >= LOG_VERBOSE_LEVEL)
						fprintf(stderr,"%s: %s\n", opts.dest_host,msg.text);
					if (!got_data_halted && opts.verbose >= LOG_VERBOSE_LEVEL)
					{
						fprintf(stderr, "Waiting for %s to halt too.\n"
							"The transfer was successful, but the source "
							"filer hasn't halted yet.\n", opts.src_host);
						got_mover_time = time(NULL);

						if (opts.verbose >= LOG_VERBOSE_LEVEL)
						{
							time_t endtime	= time(NULL);
							time_t difftime	= endtime - starttime;

							printf("Elapsed time: %ld hours, "
								"%ld minutes, %ld seconds.\n\n", difftime / 3600,
								(difftime % 3600) / 60, (difftime % 60));
						}
					}
			} else {
				fprintf(stderr, "%s: Connection halted: %s\n", msg.connection == rstr_connection ? opts.dest_host : opts.src_host, msg.text);
				shutdown_connection();
				return -1;
			}
			break;
	    case NDMP_NOTIFY_MOVER_PAUSED:
			fprintf(stderr, "PAUSE: %s: %s\n", msg.connection == rstr_connection ? opts.dest_host : opts.src_host, msg.text);
			exit(1);
			break;
	    case NDMP_NOTIFY_MOVER_HALTED:
			if ((msg.connection == rstr_connection) &&
				(msg.reason == NDMP_MOVER_HALT_CONNECT_CLOSED))
				got_mover_halted = 1;
			break;
	    case NDMP_LOG_NORMAL:
			fprintf(stderr, "%s: LOG: %s", msg.connection == rstr_connection ? opts.dest_host : opts.src_host, msg.text);
			break;
	    default:
			fprintf(stderr, "%s: %s\n", msg.connection == rstr_connection ? opts.dest_host : opts.src_host, msg.text);
	    }
	}

	if (got_data_halted && got_mover_halted) {
	    if (opts.verbose >= DEBUG_VERBOSE_LEVEL)
		fprintf(stderr, "Mover and data server halted, closing connection.\n");
	    shutdown_connection();
	    return 0;
	}

    }

    return 0;
}

/* ndmpErrToStr():
 *   Description:
 *      Translates an NDMP error code into a human readable string.
 *      This function was taken from the NDMP API toolkit.
 *      
 *   Inputs:
 *      ndmp_error err    -  An NDMP error code generated by an API call
 *
 *   Outputs:
 *      None.
 *
 *   Returns:
 *      String describing the error code passed in.
 */

char*
ndmpErrToStr(ndmp_error	err)
{
	switch (err)
	{
		case NDMP_NO_ERR:
			return "No error";
		case NDMP_NOT_SUPPORTED_ERR:
			return "Error: Call is not supported";
		case NDMP_DEVICE_BUSY_ERR:
			return "Error: The device is in use";
		case NDMP_DEVICE_OPENED_ERR:
			return "Error: Another tape or scsi device is already open";
		case NDMP_NOT_AUTHORIZED_ERR:
			return "Error: Connection has not been authorized";
		case NDMP_PERMISSION_ERR:
			return "Error: Some sort of permission problem";
		case NDMP_DEV_NOT_OPEN_ERR:
			return "Error: Device is not open";
		case NDMP_IO_ERR:
			return "Error: I/O error";
		case NDMP_TIMEOUT_ERR:
			return "Error: Command timed out";
		case NDMP_ILLEGAL_ARGS_ERR:
			return "Error: Illegal arguments in request";
		case NDMP_NO_TAPE_LOADED_ERR:
			return "Error: Cannot open because there is";
		case NDMP_WRITE_PROTECT_ERR:
			return "Error: Tape cannot be open for write";
		case NDMP_EOF_ERR:
			return "Error: Command encountered EOF";
		case NDMP_EOM_ERR:
			return "Error: Command encountered EOM";
		case NDMP_FILE_NOT_FOUND_ERR:
			return "Error: File not found during restore";
		case NDMP_BAD_FILE_ERR:
			return "Error: Bad file";
		case NDMP_NO_DEVICE_ERR:
			return "Error: Invalid device";
		case NDMP_NO_BUS_ERR:
			return "Error: Invalid bus";
		case NDMP_XDR_DECODE_ERR:
			return "Error: Decoding arguments";
		case NDMP_ILLEGAL_STATE_ERR:
			return "Error: Invalid state to process request";
		case NDMP_UNDEFINED_ERR:
			return "Error: Undefined";
		case NDMP_XDR_ENCODE_ERR:
			return "Error: Encoding arguments";
		case NDMP_NO_MEM_ERR:
			return "Error: Allocating memory";
		default:
			return "Error: unknown error";
	}
}

/*
 * estb_data_conn() :
 *  This method is used establish a data connection between the NDMP servers.
 * The NDMP copy data is transferred on this connection channel.
 *
 * Return :
 * 0 - Failure
 * 1 - Success
 */
int estb_data_conn()
{
        ndmp_addr *mover_address;
	int r;
        /* Data Listen start */
        void *request_mem, *reply_mem;
        ndmp_data_listen_request *request = (ndmp_data_listen_request *)
            malloc(sizeof (ndmp_data_listen_request));
        request->addr_type = NDMP_ADDR_TCP;
        request_mem = (void *) request;

        r = ndmpSendRequest(rstr_connection, NDMP_DATA_LISTEN, NDMP_NO_ERR,
            (void *) request_mem, (void *) & reply_mem);
        if (r != 0) {
                fprintf(stderr,
		    "init: error sending NDMP_DATA_LISTEN message: %s\n",
		    ndmpErrToStr(r));
                return -1;
        }

        ndmp_data_listen_reply *reply_dl = (ndmp_data_listen_reply*) reply_mem;
        mover_address = &(reply_dl->connect_addr);
        /* Data Listen end */

        /* Creating the data connect object and sending request start */

        ndmp_data_connect_request *request_dc;
        request_dc = (ndmp_data_connect_request*) malloc
            (sizeof (ndmp_data_connect_request));
        request_mem = (void *) request_dc;
        ndmp_tcp_addr *tcp_addr =
	    (ndmp_tcp_addr *) malloc(sizeof (ndmp_tcp_addr));
        if (mover_address->ndmp_addr_u.tcp_addr.tcp_addr_val != NULL) {
                tcp_addr->port =
		    mover_address->ndmp_addr_u.tcp_addr.tcp_addr_val->port;
                tcp_addr->ip_addr =
		    mover_address->ndmp_addr_u.tcp_addr.tcp_addr_val->ip_addr;
                request_dc->addr.ndmp_addr_u.tcp_addr.tcp_addr_val = tcp_addr;
                request_dc->addr.ndmp_addr_u.tcp_addr.tcp_addr_len =
                    mover_address->ndmp_addr_u.tcp_addr.tcp_addr_len;
        }
        request_dc->addr.addr_type = mover_address->addr_type;

        r = ndmpSendRequest(dump_connection, NDMP_DATA_CONNECT, NDMP_NO_ERR,
            (void *) request_dc, (void *) & reply_mem);

        if (r != 0) {
                fprintf(stderr, "Error starting data connect, error: %s\n",
                    ndmpErrToStr(r));
                return -1;
        }
        /* Creating the data connect object end */
	ndmpFreeMessage(rstr_connection);
	ndmpFreeMessage(dump_connection);
}
/* 
 * ndmp_stop() :
 * Stop both the data servers once the copy is done.
 */
int ndmp_stop()
{
	int r;
        /* Connection close code start */
        void *request_mem, *reply_mem = NULL;
        request_mem = NULL;
	
        /* test code start */
        r = ndmpSendRequest(dump_connection, NDMP_DATA_GET_STATE, NDMP_NO_ERR,
            (void *) request_mem, (void *) & reply_mem);
        if (r != 0) {
                fprintf(stderr, "Error starting NDMP_DATA_GET_STATE, error: %d\n",
                    r);
                return -1;
        }
        ndmpFreeMessage(dump_connection);
	
        r = ndmpSendRequest(rstr_connection, NDMP_DATA_GET_STATE, NDMP_NO_ERR,
            (void *) request_mem, (void *) & reply_mem);
        if (r != 0) {
                fprintf(stderr, "Error starting NDMP_DATA_GET_STATE, error: %d\n",
                    r);
                return -1;
        }
        ndmpFreeMessage(dump_connection);
        /* test code end */
	
        r = ndmpSendRequest(dump_connection, NDMP_DATA_STOP, NDMP_NO_ERR,
            (void *) request_mem, (void *) & reply_mem);
        if (r != 0) {
                fprintf(stderr, "Error starting NDMP_DATA_STOP, error: %s\n",
                    ndmpErrToStr(r));
                return -1;
        }
	
        r = ndmpSendRequest(rstr_connection, NDMP_DATA_STOP, NDMP_NO_ERR,
            (void *) request_mem, (void *) & reply_mem);
        if (r != 0) {
                fprintf(stderr, "Error starting NDMP_DATA_STOP, error: %s\n",
                    ndmpErrToStr(r));
                return -1;
        }
        /* Connection close code end */
	ndmpFreeMessage(rstr_connection);
	ndmpFreeMessage(dump_connection);
	/* test code start */
        r = ndmpSendRequest(dump_connection, NDMP_DATA_GET_STATE, NDMP_NO_ERR,
            (void *) request_mem, (void *) & reply_mem);
        if (r != 0) {
                fprintf(stderr, "Error starting NDMP_DATA_GET_STATE, error: %d\n",
                    r);
                return -1;
        }
        ndmpFreeMessage(dump_connection);
	
        r = ndmpSendRequest(rstr_connection, NDMP_DATA_GET_STATE, NDMP_NO_ERR,
            (void *) request_mem, (void *) & reply_mem);
        if (r != 0) {
                fprintf(stderr, "Error starting NDMP_DATA_GET_STATE, error: %d\n",
                    r);
                return -1;
        }
        ndmpFreeMessage(dump_connection);
        /* test code end */
        return (1);
}

/*
 * data_get_state() :
 * Finds out the state of the data server.
 *
 *  Returns :
 *	State of the data server.
 */
int data_get_state(NdmpConnection conn, ndmp_data_get_state_reply *reply)
{
	int r, state = 1;
        void *request_mem, *reply_mem = NULL;
        request_mem = NULL;
	r = ndmpSendRequest(conn, NDMP_DATA_GET_STATE, NDMP_NO_ERR,
            (void *) request_mem, (void *) &reply_mem);
        if (r != 0) {
                fprintf(stderr, "Error starting NDMP_DATA_GET_STATE, error: %s error # %d\n",
                    ndmpErrToStr(r), r);
                return -1;
        }
	memcpy(reply, reply_mem, sizeof (ndmp_data_get_state_reply));
	state = reply->state;
	ndmpFreeMessage(conn);
	return (state);
}

/*
 * check_notification() :
 *  Check if the notification / post message specified in the argument is
 *  received. Do the processing beased on the notification.
 *
 *  Returns : void
 */
int check_notification(NdmpConnection connection, ndmp_message message)
{
        MsgData msg;
        fd_set set;
	int time_out = 0;
        struct timeval timeout;
        /* Inititalize the file descriptor set */
        FD_ZERO(&set);
	FD_SET(ndmpGetFd(connection), &set);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
	/* loops inifinately till the message arrives. The time out can be set here */
        while (1) {
		if (select(FD_SETSIZE, &set, NULL, NULL, &timeout)
		    < 0) {
			fprintf(stderr, "check_notification: timeout with error\n");
			return (0);
		}
		
		/* Checks if post has arrived and puts in message queue */
		if (ndmpProcessRequests(connection) < 0) {
			fprintf(stderr, "check_notification: processing error\n");
			return (0);
		}
                if (dequeue((MsgQueue *) ndmpGetClientData(connection), &msg)) {
                        switch (msg.message) {
                                case NDMP_NOTIFY_DATA_HALTED:
                                        fprintf(stdout,
                                            "NDMP_NOTIFY_DATA_HALTED\n");
                                        if (message == NDMP_NOTIFY_DATA_HALTED)
                                                return (1);
                                case NDMP_NOTIFY_DATA_READ:
                                        fprintf(stdout,
                                            "NDMP_NOTIFY_DATA_READ\n");
                                        if (message == NDMP_NOTIFY_DATA_READ)
                                                return (1);
                                        return (1);
                                default:
                                        fprintf(stderr, "%s: %s\n",
                                            msg.connection == rstr_connection
                                            ? opts.dest_host : opts.src_host,
                                            msg.text);
                        }
                }
        }
}

/* Dummy handler */
void common_handler(NdmpConnection connection, void *body)
{
}
/*
 * End of file	:	dump.c
 */
