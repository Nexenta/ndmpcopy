/*                               -*- Mode: C -*- 
 * handler_tbl.c
 * 
 * Description     : NDMP request handler table.
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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * The majority of this code was taken from the NDMP SDK client toolkit.
 */

#include "ndmp_common.h"
#include "ndmpc.h"
void common_handler(NdmpConnection, void *);

NdmpMsgHandler ndmp_msg_handler_tbl[] =
{
	{
		NDMP_CONNECT_OPEN, 4,
		0,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_connect_open_request),
		XDR_AND_SIZE(ndmp_connect_open_reply)
	},
	{
		NDMP_CONNECT_CLIENT_AUTH, 4,
		0,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_connect_client_auth_request),
		XDR_AND_SIZE(ndmp_connect_client_auth_reply)
	},
	{
		NDMP_CONNECT_SERVER_AUTH, 4,
		0,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_connect_server_auth_request),
		XDR_AND_SIZE(ndmp_connect_server_auth_reply)
	},
	{
		NDMP_CONNECT_CLOSE, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		0, 0,
	},
	{
		NDMP_CONFIG_GET_HOST_INFO, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_host_info_reply)
	},
	{
		NDMP_CONFIG_GET_SERVER_INFO, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_server_info_reply)
	},
	{
		NDMP_CONFIG_GET_CONNECTION_TYPE, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_connection_type_reply)
	},
	{
		NDMP_CONFIG_GET_AUTH_ATTR, 4,
		0,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_config_get_auth_attr_request),
		XDR_AND_SIZE(ndmp_config_get_auth_attr_reply)
	},
	{
		NDMP_CONFIG_GET_BUTYPE_INFO, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_butype_attr_reply)
	},
	{
		NDMP_CONFIG_GET_FS_INFO, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_fs_info_reply)
	},
	{
		NDMP_CONFIG_GET_TAPE_INFO, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_tape_info_reply)
	},
	{
		NDMP_CONFIG_GET_SCSI_INFO, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_scsi_info_reply)
	},
	{
		NDMP_CONFIG_GET_EXT_LIST, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_ext_list_reply)
	},
	{
		NDMP_CONFIG_SET_EXT_LIST, 4,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_set_ext_list_reply)
	},
	{
		NDMP_SCSI_OPEN, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_scsi_open_request),
		XDR_AND_SIZE(ndmp_scsi_open_reply)
	},
	{
		NDMP_SCSI_CLOSE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_scsi_close_reply)
	},
	{
		NDMP_SCSI_GET_STATE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_scsi_get_state_reply)
	},
	{
		NDMP_SCSI_RESET_DEVICE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_scsi_reset_device_reply)
	},
	{
		NDMP_SCSI_EXECUTE_CDB, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_execute_cdb_request),
		XDR_AND_SIZE(ndmp_execute_cdb_reply)
	},
	{
		NDMP_TAPE_OPEN, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_open_request),
		XDR_AND_SIZE(ndmp_tape_open_reply)
	},
	{
		NDMP_TAPE_CLOSE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_tape_close_reply)
	},
	{
		NDMP_TAPE_GET_STATE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_tape_get_state_reply)
	},
	{
		NDMP_TAPE_MTIO, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_mtio_request),
		XDR_AND_SIZE(ndmp_tape_mtio_reply)
	},
	{
		NDMP_TAPE_WRITE, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_write_request),
		XDR_AND_SIZE(ndmp_tape_write_reply)
	},
	{
		NDMP_TAPE_READ, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_read_request),
		XDR_AND_SIZE(ndmp_tape_read_reply)
	},
	{
		NDMP_TAPE_EXECUTE_CDB, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_execute_cdb_request),
		XDR_AND_SIZE(ndmp_tape_execute_cdb_reply)
	},
	{
		NDMP_DATA_GET_STATE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_get_state_reply)
	},
	{
		NDMP_DATA_START_BACKUP, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_start_backup_request),
		XDR_AND_SIZE(ndmp_data_start_backup_reply)
	},
	{
		NDMP_DATA_START_RECOVER, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_start_recover_request),
		XDR_AND_SIZE(ndmp_data_start_recover_reply)
	},
	{
		NDMP_DATA_START_RECOVER_FILEHIST, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_start_recover_request),
		XDR_AND_SIZE(ndmp_data_start_recover_reply)
	},
	{
		NDMP_DATA_ABORT, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_abort_reply)
	},
	{
		NDMP_DATA_GET_ENV, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_get_env_reply)
	},
	{
		NDMP_DATA_STOP, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_stop_reply)
	},
	{
		NDMP_DATA_LISTEN, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_listen_request),
		XDR_AND_SIZE(ndmp_data_listen_reply)
	},
	{
		NDMP_DATA_CONNECT, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_connect_request),
		XDR_AND_SIZE(ndmp_data_connect_reply)
	},
	{
		NDMP_NOTIFY_DATA_HALTED, 4,
		notifyDataHalted,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_data_halted_post),
		0, 0,
	},
	{
		NDMP_NOTIFY_CONNECTION_STATUS, 4,
		common_handler,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_connection_status_post),
		0, 0,
	},
	{
		NDMP_NOTIFY_MOVER_HALTED, 4,
		common_handler,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_mover_halted_post),
		0, 0,
	},
	{
		NDMP_NOTIFY_MOVER_PAUSED, 4,
		common_handler,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_mover_paused_post),
		0, 0,
	},
	{
		NDMP_NOTIFY_DATA_READ, 4,
		notifyDataRead,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_data_read_post),
		0, 0,
	},
	{
		NDMP_MOVER_GET_STATE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_get_state_reply)
	},
	{
		NDMP_MOVER_LISTEN, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_listen_request),
		XDR_AND_SIZE(ndmp_mover_listen_reply)
	},
	{
		NDMP_MOVER_CONTINUE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_continue_reply)
	},
	{
		NDMP_MOVER_ABORT, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_abort_reply)
	},
	{
		NDMP_MOVER_STOP, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_stop_reply)
	},
	{
		NDMP_MOVER_SET_WINDOW, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_set_window_request),
		XDR_AND_SIZE(ndmp_mover_set_window_reply)
	},
	{
		NDMP_MOVER_READ, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_read_request),
		XDR_AND_SIZE(ndmp_mover_read_reply)
	},
	{
		NDMP_MOVER_CLOSE, 4,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_close_reply)
	},
	{
		NDMP_MOVER_SET_RECORD_SIZE, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_set_record_size_request),
		XDR_AND_SIZE(ndmp_mover_set_record_size_reply)
	},
	{
		NDMP_MOVER_CONNECT, 4,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_connect_request),
		XDR_AND_SIZE(ndmp_mover_connect_reply)
	},
	{
		NDMP_LOG_FILE, 4,
		common_handler,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_log_file_post),
		0, 0,
	},
	{
		NDMP_LOG_MESSAGE, 4,
		common_handler,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_log_message_post),
		0, 0,
	},
	{
		NDMP_FH_ADD_FILE, 4,
		common_handler,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_file_post),
		0, 0,
	},
	{
		NDMP_FH_ADD_DIR, 4,
		common_handler,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_dir_post),
		0, 0,
	},
	{
		NDMP_FH_ADD_NODE, 4,
		common_handler,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_node_post),
		0, 0,
	},
	{
		0,
		0,
		0,
		0, 0,
		0, 0,
	}
};
