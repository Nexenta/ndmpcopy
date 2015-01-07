/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _NOTIFY_H
#define	_NOTIFY_H

#pragma ident	"@(#)notify.h	1.5	08/07/25 SMI"

/*
 * Function declarations for notify interface.
 */

#ifdef __cplusplus
extern "C" {
#endif

NdmpParamExtractFunc	ndmp_notify_data_halted_post_extract;
NdmpCompareParamFunc	ndmp_notify_data_halted_post_compare;
PrintNdmpMsgFunc	ndmp_notify_data_halted_post_print;

NdmpParamExtractFunc	ndmp_notify_connection_status_post_extract;
NdmpCompareParamFunc	ndmp_notify_connection_status_post_compare;
PrintNdmpMsgFunc	ndmp_notify_connection_status_post_print;

NdmpParamExtractFunc	ndmp_notify_mover_halted_post_extract;
NdmpCompareParamFunc	ndmp_notify_mover_halted_post_compare;
PrintNdmpMsgFunc	ndmp_notify_mover_halted_post_print;

NdmpParamExtractFunc	ndmp_notify_mover_paused_post_extract;
NdmpCompareParamFunc	ndmp_notify_mover_paused_post_compare;
PrintNdmpMsgFunc	ndmp_notify_mover_paused_post_print;

NdmpParamExtractFunc	ndmp_notify_data_read_post_extract;
NdmpCompareParamFunc	ndmp_notify_data_read_post_compare;
PrintNdmpMsgFunc	ndmp_notify_data_read_post_print;

#ifdef __cplusplus
}
#endif

#endif /* _NOTIFY_H */
