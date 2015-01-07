/* 
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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "ndmp_common.h"

void usage(void);
void parse_cmd_line(int argc, char *argv[]);
void set_opt_defaults(void);

#define MAX_HOST_NAME_LEN 128
#define MAX_DIR_LEN 256
#define MAX_AUTH_USER_LEN 20
#define MAX_AUTH_PASSWORD_LEN 20

#define OPT_DHOST                "-dhost"
#define OPT_SRC_AUTH             "-sa"
#define OPT_DEST_AUTH            "-da"
#define OPT_NDMP_SRC_PORT        "-sport"
#define OPT_NDMP_DEST_PORT       "-dport"
#define OPT_LEVEL                "-level"
#define OPT_HELP                 "-h"
#define OPT_DEBUG                "-d"
#define OPT_VERBOSE              "-v"  
#define OPT_QUIET                "-q"  
#define OPT_DIFFERENT_PASSWORDS  "-dpass"

#define LOG_VERBOSE_LEVEL	1
#define DEBUG_VERBOSE_LEVEL	3

#define DEF_SRC_HOST             "localhost"
#define DEF_DEST_HOST            "localhost"

#define DEF_SRC_AUTH_TYPE        NDMP_AUTH_TEXT
#define DEF_SRC_AUTH_USER        "root"
#define DEF_SRC_AUTH_PASSWORD    ""

#define DEF_DEST_AUTH_TYPE       NDMP_AUTH_TEXT
#define DEF_DEST_AUTH_USER       "root"
#define DEF_DEST_AUTH_PASSWORD   ""

#define DEF_DHOST                ""

#define DEF_SRC_DIR              "/"
#define DEF_DEST_DIR             "/"
#define DEF_NDMP_SRC_PORT        0
#define DEF_NDMP_DEST_PORT       0
#define DEF_LEVEL                "0"
#define DEF_DEBUG                0
#define DEF_VERBOSE              1
#define DEF_DIFFERENT_PASSWORDS  0
#define DEF_EXTRACT				"y"

struct auth_info {
    ndmp_auth_type type;
    char user[MAX_AUTH_USER_LEN];
    char password[MAX_AUTH_PASSWORD_LEN];
};

struct cmd_line_opts {
    char			src_host[MAX_HOST_NAME_LEN+1];	/* Host name to start backup on				*/
    char			dest_host[MAX_HOST_NAME_LEN+1];	/* Host name to start recover on			*/
    char			dhost[MAX_HOST_NAME_LEN+1];		/* Override default mover address			*/
    struct			auth_info src_auth;				/* Authorization info for source			*/
    struct			auth_info dest_auth;			/* Authorization info for dest				*/
    char			src_dir[MAX_DIR_LEN+2];			/* Directory on src to back up				*/
    char			dest_dir[MAX_DIR_LEN+2];		/* Directory on dest to recover to			*/
    ushort			ndmp_src_port;					/* NDMP port for connections to src			*/
    ushort			ndmp_dest_port;					/* NDMP port for connections to dest		*/
    char			level[2];						/* Level of dump to perform (1 digit)		*/
    char			extract[2];						/* Extract or incrementally restore files	*/
    unsigned int	debug;							/* Display debugging messages?				*/
    int				verbose;						/* Display log messages?					*/
    int				different_passwords; 				/* src and dest have different passwords?       */

};

#endif /* __MAIN_H__ */

/*
 * End of file	:	main.h
 */
