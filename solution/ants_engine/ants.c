
/* 

   Main file for ants visualizer. 

   Reads world from stdin, followed by traces. Remembers each step in 
   the world's evolvement, so one can analyze the solution. 

   Drives the visualizer. 

*/

#include "visualizer.h"
#include "ants_def.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


int warnings = 0;

static int display_map (struct world *world)
/* Helper function to genereate ASCII map from world. and feed it into 
   v_load_map. 
*/

{
	char *tmp_map;
	char c;
	struct position p;
	struct cell *cell;
	int ret; 

	tmp_map = (char *)malloc (world->size_x*world->size_y);
	memset (tmp_map, ' ', world->size_x*world->size_y);

	for (p.x=0;p.x<world->size_x;p.x++) {
		for (p.y=0;p.y<world->size_y;p.y++) {
			cell = cell_at (world, &p);
			if (cell->home_of_colony != -1) {
				c = colony_home_chars[cell->home_of_colony];		
			} else if (cell->rocky) {
				c = ROCK_CELL;
			} else c = EMPTY_CELL;
			tmp_map[p.y*world->size_x+p.x] = c;
		}
	}

	ret = v_load_map (world->size_x, world->size_y, tmp_map);
	free (tmp_map);
	return ret;
}
	

int display_world (struct world *world)
{
	int ret; 
	struct position p;
	struct cell *cell;
	int team;

	if ((ret=display_map (world)) != 0) return ret;

	for (p.x=0;p.x<world->size_x;p.x++) {
		for (p.y=0;p.y<world->size_y;p.y++) {
			cell = cell_at (world, &p);

			v_set_food (&p, cell->food);
			for (team=0; team<NUM_COLONIES; team++) {
				v_set_marker (&p, team, cell->markers[team]);
			}
			if (cell->ant) {
				v_set_ant (cell->ant);
			}
		}
	}

	return v_redisplay_everything ();
}
	
			
	
void usage_and_exit (char *prog)
{
	fprintf (stderr, "Usage: %s <mapfile> <worldtracefile>	for world trace format\n", prog); 
	fprintf (stderr, "       %s -d [-r resolution ] [-s skip_steps] [-h head_steps] [-t tail_steps] <dumpfile>	for ICFP format worlds\n", prog); 
	fprintf (stderr, "       -w display warnings (ant attempting to run into rock, for example)\n");
	fprintf (stderr, "       -p <step> run performance test to show that visualizer rules in brain.\n");
	fprintf (stderr, "       -x set window width.\n");
	fprintf (stderr, "       -y set window height.\n");
	fprintf (stderr, "       -c set cell size (3 is good for 100x100 maps, 12 for 10x10 map).\n");

	exit (-1);
}
	
int dump_format = 1;

FILE *open_next_file_from_argv (char ** argv)
{
	FILE *f;

	if (!argv[optind]) {
		usage_and_exit (argv[0]);
	}
		
	f = fopen (argv[optind], "r");
	if (!f) {
		fprintf (stderr, "Error, could not open %s\n", argv[optind]); 
		exit (-1);
	}
	optind++;
	
	return f;
}

int show_state_in_editor (struct world *world)
{
	struct position *pos;
	struct cell *cell;

	v_query_pointer (pos);

	cell = cell_at (world, pos);
	if (!cell) return -1;

	return 0;
}

const char *cell_info (struct world *world, struct position *pos)
{
	static char info[100];
	static char *team[NUM_COLONIES] = { "friendly", "enemy" };
	static char *direction[6] = { "left", "down left", "down right", "right", "up right", "up left" };
	char *s;
	struct cell *cell;

	cell = cell_at (world, pos);
	if (!cell) return "(pointer out of range)";

	s = info;
	s += sprintf (s, "cell (%d/%d)", pos->x, pos->y); 
	if (cell->rocky) s += sprintf (s, " (rock)"); 
	if (cell->home_of_colony != -1) s += sprintf (s, " (#%d %s homebase)", cell->home_of_colony, team [cell->home_of_colony]); 
	if (cell->food) s += sprintf (s, " (%d food)", cell->food); 
	if (cell->ant) {
		s += sprintf (s, " (%s ant", team [cell->ant->colony]); 
		s += sprintf (s, ", heading %s", direction [cell->ant->direction]); 
		s += sprintf (s, ", id %d, state %d", cell->ant->id, cell->ant->state); 
		s += sprintf (s, ")");
	}

	return info;
}
	
	

int update_status (struct world *world)
{
	static char line[100];
	struct position pos;

	v_query_pointer (&pos);
	sprintf (line, "Step %d, Round %d %s", world->step, world->round, cell_info (world, &pos)); 
	return v_display_status_line (line);
}

int run_main_loop (struct world *world)
{
	int rounds, steps, do_animation, do_one_step;
	int animation_speed;   /* In milliseconds. */
	static int round_intervals[6] = { 0, 1, 10, 100, 1000, 10000 };
	static int step_intervals[6] = { 1, 0, 0, 0, 0, 0 };

	enum events event; 	

	rounds = 1;  
	steps = 0;  

	do_animation = 0;
	do_one_step = 0;
	
	animation_speed = 500;

	while ((event = v_poll_event (animation_speed)) != EVENT_QUIT) {
		switch (event) {
		case EVENT_NONE: break;
		case EVENT_REDISPLAY: display_world (world); break;

		case EVENT_ANIMATE_FASTEST: animation_speed = 0; break;
		case EVENT_ANIMATE_FASTER: animation_speed -= 100; break;
		case EVENT_ANIMATE_SLOWER: animation_speed += 100; break;
		case EVENT_START_ANIMATION: do_animation=1; break;
		case EVENT_STOP_ANIMATION: do_animation=0; break;
		case EVENT_TOGGLE_ANIMATION: do_animation=!do_animation; break;

		case EVENT_UPDATE: break;   /* just causes status line to be updated. */
		case EVENT_SHOW_ANT_STATE: show_state_in_editor (world); 
		default: break;
		}

		if (event>=EVENT_STEP_FORWARD && event<EVENT_10000_ROUNDS_BACKWARD) {
			rounds = round_intervals [(event - EVENT_STEP_FORWARD) / 2];
			steps = step_intervals [(event - EVENT_STEP_FORWARD) / 2];
			if ((event - EVENT_STEP_FORWARD) & 1) {
				rounds = -rounds;
				steps = -steps;
			}

			do_one_step = 1;
		}

		update_status (world);
		if (do_animation || do_one_step) {
			if (steps) {
				n_steps (world, steps);
			} else {
				n_rounds (world, rounds);
			}
		}
		do_one_step = 0;
	}

	return 0;
}


int run_performance_test (struct world *world, int step)
{
	while (world->cursor->next) {
		n_rounds (world, step);
	}
	printf ("%d steps, %d rounds\n", world->step, world->round);

	return 0;
}
		



int main (int argc, char ** argv)
{
	struct world *world;
	FILE *f;
	int c, ret;
	int head_steps, tail_steps, skip_steps, next_step;
	int ants_count, resolution;
	int step;
	int performance_test;
	int x;
	int y;
	int hex_size;

	dump_format = 0;
	step = 0;
	head_steps = 0; 
	tail_steps = 0; 
	skip_steps = 1;
	performance_test = 0;

	x = 800;
	y = 600;
	hex_size = 3;

/* These are used for dump format, since it is difficult to figure them out. */
	resolution = 10;
	ants_count = 32;

	world = NULL;

	while ((c=getopt (argc, argv, "wdh:t:s:a:p:x:y:c:")) != EOF) {
		switch (c) {
		case 'd': dump_format = 1; break;
		case 't': 
			if (!optarg) usage_and_exit (argv[0]);
			tail_steps = atoi (optarg); break;
		case 's': 
			if (!optarg) usage_and_exit (argv[0]);
			skip_steps = atoi (optarg); break;
                case 'r':
                        if (!optarg) usage_and_exit (argv[0]);
                        resolution = atoi (optarg); break;
                case 'a':
                        if (!optarg) usage_and_exit (argv[0]);
                        ants_count = atoi (optarg); break;
		case 'h': 
			if (!optarg) usage_and_exit (argv[0]);
			head_steps = atoi (optarg); break;
		case 'w': warnings = 1; break;
		case 'p': 
			if (!optarg) usage_and_exit (argv[0]);
			performance_test = atoi (optarg); 
			break;
		case 'x': 
			if (!optarg) usage_and_exit (argv[0]);
			x = atoi (optarg); break;
		case 'y': 
			if (!optarg) usage_and_exit (argv[0]);
			y = atoi (optarg); break;
		case 'c': 
			if (!optarg) usage_and_exit (argv[0]);
			hex_size = atoi (optarg); break;
		
		case '?': 
		default: usage_and_exit (argv[0]);
		}
	}

	if (skip_steps < 1 || tail_steps < head_steps) {
		fprintf (stderr, "Illegal step setting.\n");
		exit (-1);
	}

	if (dump_format) {
		next_step = head_steps;
		f = open_next_file_from_argv (argv);

		v_set_geometry (x, y, hex_size);
		v_initialize ();
		while (!feof (f)) {
			if (read_world_as_icfp_dump (&world, f, resolution, resolution, ants_count) == 0) {
				if (step <= head_steps ||
                                    step >= tail_steps ||
                                    step == next_step) {
					printf ("Step %d ...\n", step);
					display_world (world);
					if ((ret = v_poll_event (0))) {
					    if (ret == -1) {
						break;
					    }
					}

					next_step = skip_steps+step;
				}
			}
			step++;
		}
		fclose (f);
	} else {
		f = open_next_file_from_argv (argv);
		if (read_world (&world, f) != 0) {
			fprintf (stderr, "Error in reading world map.\n");
			exit (-1);
		}
		fclose (f);

		f = open_next_file_from_argv (argv);
		if (parse_world_trace (world, f) != 0) {
			fprintf (stderr, "Error in reading world trace.\n");
			exit (-1);
		}
		fclose (f);

		v_set_geometry (x, y, hex_size);
		v_initialize ();
		display_world (world);
	}

	if (performance_test) {
		return run_performance_test (world, performance_test);
	} else {
		return run_main_loop (world);
	}
}


/** TODO:
       Open state of ant unter mouse pointer on click.
       Handle dead ants. 

   to test: 
       Backward. 

   done:
       Only update changed cells even when stepping over several rounds.
       Show state of cell under mouse pointer. 

   Bertl: get_pos ()
*/

