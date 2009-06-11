
#include "ants_def.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "pools.h"

struct world_trace *world_trace;

#define MAX_LINE 255


int parse_comment (const char *s)
{
	fprintf (stderr, "Comment: %s\n", s);

	return 0;
}

int parse_world_trace (struct world *world, FILE *f)
{
	char buf[MAX_LINE];
	char *s;
	int id, colony;
	struct execution_step **e, *new_e, *last_e;
	static pools_t step_pool;
	
	free_world_trace (world);

	e = &world->world_trace; 
	last_e = NULL;
		    
	init_pools(&step_pool);

#if 0
	new_e = pools_alloc (&step_pool,
	    sizeof (struct execution_step));
	new_e->prev = last_e;
	last_e = new_e;
	*e = new_e;
	e = &((*e)->next);
#endif

	while (fgets (buf, MAX_LINE, f) == buf) {
		if (buf[0] == '#' && buf[1] != '\n') {
			parse_comment (buf+1);
			continue;
		}

		new_e = pools_alloc (&step_pool, 
		    sizeof (struct execution_step));

		if (buf[0] == '#' && buf[1] == '\n') {
			/* nothing. */
		} else {
			s=buf;
    	    	    	
			while (*s==' ') s++;
			if (*s == '!') {
    	    	    	    new_e->failed_action = 1;
			    s++;
    	    	    	}

			colony = strtol (s, &s, 10);
			assert (s);
			id = strtol (s, &s, 10);
			assert (s);
	
			if (id >= world->total_ants || id < 0 ||
			    colony >= NUM_COLONIES || colony < 0) {
				fprintf (stderr, "Trace file format error, id/colony out of range.\n");
				exit (-1);
			}

			new_e->ant = &world->ants[id]; 
			new_e->old_ant_state = -1;

			while (*s==' ') s++;
			switch (*s) {
			case 'm': 
				assert (s[1] == 'f');
				new_e->action = MOVE_FORWARD;
				break;

			case 't': 
				if (s[1] == 'l') 
				    new_e->action = TURN_LEFT;
				else if (s[1] == 'r') 
				    new_e->action = TURN_RIGHT;
				else {
					fprintf (stderr, "Trace file format error, unknown action.\n");
					fprintf (stderr, "Line: %s\n", buf);
					exit (-1);
				}
				break;

			case 'p': 
				assert (s[1] == 'f');
				new_e->action = PICKUP_FOOD;
				break;

			case 'd': 
			    	if (s[1] == 'f') {
				    	new_e->action = DROP_FOOD;
				} else if (s[1] == 't') {
				    	new_e->action = KILL_ANT;
				} else {
					fprintf (stderr, "Trace file format error, unknown action.\n");
					fprintf (stderr, "Line: %s\n", buf);
					exit (-1);
				}
				break;

			case 's': 
				if (isdigit (s[1])) {
					new_e->action = SET_MARKER;
					new_e->marker = s[1] - '0';
				} else {
					fprintf (stderr, "Trace file format error, wrong marker number.\n");
					fprintf (stderr, "Line: %s\n", buf);
					exit (-1);
				}
				break;

			case 'l': 
				switch (s[1]) {
				    case 'h':
			    	    	new_e->action = SENSE_HERE;
					break;
				    case 'l':
			    	    	new_e->action = SENSE_LEFT;
					break;
				    case 'a':
			    	    	new_e->action = SENSE_AHEAD;
					break;
				    case 'r':
			    	    	new_e->action = SENSE_RIGHT;
					break;
				    default:
					fprintf (stderr, "Trace file format error, wrong marker number.\n");
					fprintf (stderr, "Line: %s\n", buf);
					exit (-1);
				}
				break;

			case 'c': 
				if (isdigit (s[1])) {
					new_e->action = CLEAR_MARKER;
					new_e->marker = s[1] - '0';
				} else {
					fprintf (stderr, "Trace file format error, wrong marker number.\n");
					fprintf (stderr, "Line: %s\n", buf);
					exit (-1);
				}
				break;

			default:
				fprintf (stderr, "Trace file format error, unknown action.\n");
				fprintf (stderr, "Line: %s (parsing at %s)\n", buf, s);
				exit (-1);
			}
/* ?? state */
			s+=2;	
			while (*s==' ') s++;

			new_e->new_ant_state = strtol (s, &s, 10);
			while (*s==' ') s++;
			assert (*s == '\n');
		}

		new_e->prev = last_e;
		last_e = new_e;
		*e = new_e; 
		e = &((*e)->next);
	}
	world->cursor = world->world_trace;

	return 0;
}

			
