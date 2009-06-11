
#include <stdio.h>
#include <stdlib.h>
#include <linux/errno.h>
#include <unistd.h>

#include "visualizer.h"

#define VERSION "0.1"


static	char	err_flag = 0;

static	char	opt_tests = 0;
static	char	opt_sim = 0;
static	char	opt_map = 0;
static	char	opt_display = 1;

static	char *	map_name;
static	char *	cmd_name;


#define OPTIONS "htsdS:X:Y:B:M:D:"





int main (int argc, char ** argv) 
{
	extern int optind;
	extern char *optarg;
	int c, errflg = 0;
	int hex_ok = 0;

	cmd_name = argv[0];
	while ((c = getopt (argc, argv, OPTIONS)) != EOF) {
	    switch (c) {
	    case 'h':
		fprintf(stderr,
		    "This is %s V%s\n"
		    "options are:\n"
		    "-h        print this help message\n"
		    "-s        simulate\n"
		    "-d        display\n"
		    "-M <map>  read map file\n"
		   ,cmd_name, VERSION);
		exit(0);
		break;
	    case 't':
		opt_tests = 1;
		break;
	    case 's':
		opt_sim = 1;
		break;
	    case 'd':
		opt_display = 1;
		break;
	    case 'M':
		opt_map = 1;
		map_name = optarg;
		break;
	    case 'S':
		hex_ok = 1;
		setup_hex(atoi(optarg));
		break;
	    case '?':
	    default:
		errflg++;
		break;
	    }
	}
	if (errflg) {
	    fprintf (stderr, 
		"Usage: %s -[" OPTIONS "]\n"
		"%s -h for help.\n",
		cmd_name, cmd_name);
	    exit(2);
	}

	if (!hex_ok) {
	   setup_hex( 5 );
	}
		
	v_initialize ();
	
	v_redisplay_everything (); 

	return v_run_main_loop ();
}
	
