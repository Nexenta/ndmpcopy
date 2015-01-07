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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include "main.h"
#include "dump.h"
#include "ndmp_common.h"

struct cmd_line_opts opts;
void process_ndmpcopy();

int main(int argc, char *argv[])
{
    ndmp_addr		mover_address;
    time_t		starttime, endtime, difftime;

    set_opt_defaults();
    parse_cmd_line(argc, argv);

    starttime = time(NULL);
    process_ndmpcopy();

    endtime = time(NULL);
    difftime = endtime - starttime;

    if (opts.verbose >= LOG_VERBOSE_LEVEL)
	printf("Elapsed time: %ld hours, "
	       "%ld minutes, %ld seconds.\n\n", difftime / 3600,
	       (difftime % 3600) / 60, (difftime % 60));
    return 0;
}

void usage()
{
    fprintf(stderr,
	"\n"
	"Usage:  ndmpcopy src_filer:/src/dir dest_filer:/dest/dir\n"
	"                 [%s none | user:password] [%s none | user:password]\n"
	"                 [%s ndmp_src_port] [%s ndmp_dest_port]\n"
	"                 [%s ndmp_dest_ip_addr] [%s ndmp_dump_level]\n"
	"                 [%s] [%s] [%s] [%s]\n\n",
		OPT_SRC_AUTH, OPT_DEST_AUTH,
		OPT_NDMP_SRC_PORT, OPT_NDMP_DEST_PORT,
		OPT_DHOST, OPT_LEVEL,
		OPT_VERBOSE, OPT_QUIET, OPT_DIFFERENT_PASSWORDS, OPT_HELP);

    fprintf(stderr,
	"Defaults:\n"
	"   src_auth_type       = %s\n"
	"   src_auth_user       = %s\n"
	"   src_auth_password   = %s\n"
	"   dest_auth_type      = %s\n"
	"   dest_auth_user      = %s\n"
	"   dest_auth_password  = %s\n"
	"   ndmp_src_port       = %d  (0 means NDMP default, usually 10000)\n"
	"   ndmp_dest_port      = %d  (0 means NDMP default, usually 10000)\n"
	"   ndmp_dump_level     = %s  (valid range: 0 - 9)\n"
	"   ndmp_dest_ip_addr   = %s  (no default: user needs to override dest_filer value)\n"
	"   verbosity           = %s\n"
	"   different_passwords = %s\n\n",

	    DEF_SRC_AUTH_TYPE == NDMP_AUTH_TEXT ? "text" : "none",
	    DEF_SRC_AUTH_USER,  DEF_SRC_AUTH_PASSWORD,

	    DEF_DEST_AUTH_TYPE == NDMP_AUTH_TEXT ? "text" : "none",
	    DEF_DEST_AUTH_USER, DEF_DEST_AUTH_PASSWORD,

	    DEF_NDMP_SRC_PORT, DEF_NDMP_DEST_PORT,DEF_LEVEL,DEF_DHOST,
	    (DEF_VERBOSE == 0) ? "quiet" : "noisy",
	    (DEF_DIFFERENT_PASSWORDS == 0) ? "no" : "yes");

    exit(1);
}

void set_opt_defaults(void)
{
    strcpy(opts.src_host, DEF_SRC_HOST);
    strcpy(opts.dest_host, DEF_DEST_HOST);

    opts.src_auth.type = DEF_DEST_AUTH_TYPE;
    strcpy(opts.src_auth.user, DEF_SRC_AUTH_USER);
    strcpy(opts.src_auth.password, DEF_SRC_AUTH_PASSWORD);

    opts.dest_auth.type = DEF_DEST_AUTH_TYPE;
    strcpy(opts.dest_auth.user, DEF_DEST_AUTH_USER);
    strcpy(opts.dest_auth.password, DEF_DEST_AUTH_PASSWORD);

    strcpy(opts.dhost, DEF_DHOST);

    strcpy(opts.src_dir, DEF_SRC_DIR);
    strcpy(opts.dest_dir, DEF_DEST_DIR);

    opts.ndmp_src_port = DEF_NDMP_SRC_PORT;
    opts.ndmp_dest_port = DEF_NDMP_DEST_PORT;

    strcpy(opts.level,  DEF_LEVEL);
    strcpy(opts.extract,  DEF_EXTRACT);

    opts.verbose = DEF_VERBOSE;

    opts.different_passwords = DEF_DIFFERENT_PASSWORDS;
}

void parse_cmd_line(int argc, char *argv[])
{
    char *tmp_char;
    char **curarg;
    int src_host_encountered = 0, dest_host_encountered = 0;

    for (curarg = ++argv; *curarg; curarg++) {

	if (strcmp(*curarg, OPT_HELP) == 0) {
	    usage();
	}

	/* 
	 * By default the filer currently doesn't write
	 * a restore_symboltable file when restoring it
	 * just simply extracts all the files.
	 *
	 * Incremental dumps require at least 4.3 to be
	 * running on the destination filer.
	 */
	else if (strcmp(*curarg, OPT_LEVEL) == 0) {
	    if (*(++curarg) == NULL)
			usage();
		opts.level[0] = **curarg; /* only recognize first digit */
	    opts.level[1] = '\0';

		if (opts.level[0] < '0' || opts.level[0] > '9')
		{
			fprintf(stderr,"Error: Invalid level %s.\n",opts.level);
			usage();
		}
		strcpy(opts.extract,"n");
	}
	else if (strcmp(*curarg, OPT_VERBOSE) == 0) {
	    opts.verbose++;
	}
	else if (strcmp(*curarg, OPT_QUIET) == 0) {
	    opts.verbose--;
	}
	else if (strcmp(*curarg, OPT_DIFFERENT_PASSWORDS) == 0) {
	    opts.different_passwords = 1;
	}
	else if (strcmp(*curarg, OPT_DEBUG) == 0) {
	    if (*(++curarg) == NULL)
		usage();
	    opts.debug = atoi(*curarg);
	    SetDebugLevel(opts.debug);
	}
	else if (strcmp(*curarg, OPT_SRC_AUTH) == 0) {
	    /* auth info is either "none" or "user:password" */
	    if (*(++curarg) == NULL)
		usage();
	    if (strcmp("none", *curarg) == 0) {
		opts.src_auth.type = NDMP_AUTH_NONE;
		continue;
	    }
	    else if ((tmp_char = strchr(*curarg, ':')) != NULL) {
		*tmp_char = '\0'; tmp_char++; /* end username and skip null */
		opts.src_auth.type = NDMP_AUTH_TEXT;

		strncpy(opts.src_auth.user, *curarg, strlen(*curarg));
		strncpy(opts.src_auth.password, tmp_char, strlen(tmp_char));


		continue;
	    }
	    else {
		fprintf(stderr,"Error: Bad source authentication information.\n");
		usage();
	    }
	}


	else if (strcmp(*curarg, OPT_DEST_AUTH) == 0) {
	    /* auth info is either "none" or "user:password" */
	    if (*(++curarg) == NULL)
		usage();
	    if (strcmp("none", *curarg) == 0) {
		opts.dest_auth.type = NDMP_AUTH_NONE;
		continue;
	    }
	    else if ((tmp_char = strchr(*curarg, ':')) != NULL) {
		*tmp_char = '\0'; tmp_char++; /* end username and skip null */
		opts.dest_auth.type = NDMP_AUTH_TEXT;

		strncpy(opts.dest_auth.user, *curarg, strlen(*curarg));
		strncpy(opts.dest_auth.password, tmp_char, strlen(tmp_char));


		continue;
	    }
	    else {
		fprintf(stderr, "Error: Bad destination authentication "
							    "information.\n");
		usage();
	    }
	}
	else if (strcmp(*curarg, OPT_NDMP_SRC_PORT) == 0) {
	    if (*(++curarg) == NULL)
		usage();
	    opts.ndmp_src_port = atoi(*curarg);
	    if (opts.ndmp_src_port > 65535) {
		fprintf(stderr, "Error: NDMP source port can not exceed "
								    "65535.\n");
		usage();
	    }

	    /* XXX Check uid if port < 1024 */

	    continue;
	}

	else if (strcmp(*curarg, OPT_DHOST) == 0) {
	    if (*(++curarg) == NULL)
		usage();
	    strncpy(opts.dhost, *curarg, MAX_HOST_NAME_LEN);
	}

	else if (strcmp(*curarg, OPT_NDMP_DEST_PORT) == 0) {
	    if (*(++curarg) == NULL)
		usage();
	    opts.ndmp_dest_port = atoi(*curarg);
	    if (opts.ndmp_dest_port > 65535) {
		fprintf(stderr, "Error: NDMP destination port can not exceed "
								    "65535.\n");
		usage();
	    }

	    /* XXX Check uid if port < 1024 */

	    continue;
	}
	else if (!src_host_encountered) {
	    
	    char *shost, *sdir;
	    
	    shost = *curarg;
	    if ((sdir = strchr(*curarg, ':')) != NULL) {
		*sdir = '\0'; sdir++;   /* end source host and skip null */


		if (strlen(shost) > MAX_HOST_NAME_LEN) {
		    fprintf(stderr, "Error: Source hostname too long, max %d "
					    "characters.\n", MAX_HOST_NAME_LEN);
		    usage();
		}
		if (strlen(sdir) > MAX_DIR_LEN) {
		    fprintf(stderr, "Error: Source path name too long, max %d "
						"characters.\n", MAX_DIR_LEN);
		    usage();
		}
		strcpy(opts.src_host, shost);
		strcpy(opts.src_dir, sdir);


		/* make sure directories end with a slash.  NDMP complains
		 * if the last character of the src and dest directories
		 * are not the same.
		 */
		 if (*(opts.src_dir + strlen(opts.src_dir)) != '/')
		    strcat(opts.src_dir, "/");

		src_host_encountered = !0;
	    } else {
		fprintf(stderr, "The source must contain a source "
				"host, a colon, and\nthen the source "
				"pathname.\n");
		usage();
	    }
	    continue;
	}
	else if (src_host_encountered && !dest_host_encountered) {
	    char *dhost, *ddir;
	    
	    dhost = *curarg;
	    if ((ddir = strchr(*curarg, ':')) != NULL) {
		*ddir = '\0'; ddir++;   /* end dest host and skip null */

		if (strlen(dhost) > MAX_HOST_NAME_LEN) {
		    fprintf(stderr, "Error: Source hostname too long, max %d "
					    "characters.\n", MAX_HOST_NAME_LEN);
		    usage();
		}
		if (strlen(ddir) > MAX_DIR_LEN) {
		    fprintf(stderr, "Error: Source path name too long, max %d "
						"characters.\n", MAX_DIR_LEN);
		    usage();
		}
		strcpy(opts.dest_host, dhost);
		strcpy(opts.dest_dir, ddir);

		/* make sure directories end with a slash.  NDMP complains
		 * if the last character of the src and dest directories
		 * are not the same.
		 */
		 if (*(opts.dest_dir + strlen(opts.dest_dir)) != '/')
		    strcat(opts.dest_dir, "/");
		dest_host_encountered = !0;
	    } else {
		fprintf(stderr, "The destination must contain a destination "
				"host, a colon, and\nthen the destination "
				"pathname.\n");
		usage();
	    }
	    continue;
	}
    }

    if (!src_host_encountered  || !dest_host_encountered) {
	fprintf(stderr, "Source and destination filer names and paths are required.\n");
	usage();
    }

    if (opts.different_passwords) {
        if (opts.src_auth.type != NDMP_AUTH_NONE
           && !strcmp(opts.src_auth.password, DEF_SRC_AUTH_PASSWORD)) {
           strcpy(opts.src_auth.password, (char *) getpass("Source password:"));
        }

        if (opts.dest_auth.type != NDMP_AUTH_NONE
           && !strcmp(opts.dest_auth.password, DEF_DEST_AUTH_PASSWORD)) {
           strcpy(opts.dest_auth.password, (char *) getpass("Destination password:"));
        }
    }
    else {
	if (!strcmp(opts.dest_auth.password, DEF_DEST_AUTH_PASSWORD)
	   || !strcmp(opts.src_auth.password, DEF_SRC_AUTH_PASSWORD)) {
	   char *p;

	   p = (char *) getpass("Password:");
	   if (opts.src_auth.type != NDMP_AUTH_NONE) {
	       strcpy(opts.src_auth.password, p);
	   }
	   if (opts.dest_auth.type != NDMP_AUTH_NONE) {
	       strcpy(opts.dest_auth.password, p);
	   }
	}
    }
}

/*
 *  process_ndmpcopy() :
 *  The main processing of ndmpcopy program happens here.
 *  First a data channel is established between the two data servers.
 *  Then the copy is done using back and recover.
 *
 *  Returns : void
 */
void process_ndmpcopy()
{
	
    if (establish_connection() != 0)
	exit (1);
    if (estb_data_conn() == 0)
	exit(1);
    if (ndmp_restore() != 0)
	exit (1);
    if (ndmp_dump() != 0)
	exit (1);
    if (ndmp_stop() != 0)
	exit (1);
    shutdown_connection();
}

/*
 * End of file	: main.c
 */
