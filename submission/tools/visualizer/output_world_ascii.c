/* Outputs current state as ASCII. 

   Meant for testing visualizer interface. Hence we store the ASCII 
   representation of the map and react on updates. 
*/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "visualizer.h"
#include "ants_def.h"

/* Layer 0 is the map (contains info about rocks, empty or homebase)
   Layer 1 is the amount of food and the markers
   Layer 2 are the ants
*/

#define LAYERS 3

#define MAP_LAYER 0
#define FOOD_LAYER 1
#define ANT_LAYER 2

/* This is really a three-dimansional array [layer][y][x] */

static char * _map;
static int map_size_x, map_size_y;

static char direction_chars[6] = ">L\\<`/";
static char carries_food_chars[2] = " *";

#define CLEAR_SCREEN "\e[H\e[2J"

/* ASCII width of one cell. */ 
#define CELL_WIDTH 4

static char *map_at (int layer, int x, int y) 
{
	return _map+CELL_WIDTH*x+map_size_x*y+map_size_x*map_size_y*layer;
}

static char *map_at_pos (int layer, struct position *pos) 
{
	return map_at (layer, pos->x, pos->y);
}

int v_initialize (void)
{
	return 0;
}

int v_set_geometry (int x, int y, int h)
{
	return 0;
}

enum events v_poll_event (int timeout)
{
	printf ("Main loop, press enter to exit ...");
	getchar ();
	return 0;
}

int v_run_main_loop (void)
{
	v_poll_event (0);
	return 0;
}


int v_redisplay_everything (void)
{
	int x, y;
	int i, layer;
	char *s;

	printf ("%s", CLEAR_SCREEN);

	for (y=0;y<map_size_y;y++) {
		if (y&1) printf ("  ");

		for (x=0;x<map_size_x/CELL_WIDTH;x++) {
			for (i=0;i<CELL_WIDTH;i++) {
				for (layer=LAYERS-1;layer>0;layer--) {
					s = map_at (layer, x, y);
					if (s[i] != ' ') {
						printf ("%c", s[i]);
						goto next_char;
					}
				}
				s = map_at (0, x, y);
				printf ("%c", s[i]);
next_char:
				while (0) { } ;  /* avoid compiler warning */
			}
		}
		printf ("\n\n");
	}

#if 0
	printf ("Press <Enter> to continue ...");
	getchar ();
#endif 
	return 0;
}


/** Map is encoded like in input files. This is also the initialization
    function and is called only once.
  */

int v_load_map (int sizex, int sizey, char *map)
{
	int x, y, i; 
	char *s, c; 

	map_size_x = sizex * CELL_WIDTH;
	map_size_y = sizey;

	_map = (char*) malloc (map_size_x*map_size_y*LAYERS);
	memset (_map, ' ', map_size_x*map_size_y*LAYERS);

	for (x=0;x<sizex;x++) {
		for (y=0;y<sizey;y++) {
			s=map_at (MAP_LAYER, x, y);
			c=map[y*sizex + x]; 
			for (i=0;i<NUM_COLONIES;i++) {
				if (c==colony_home_chars[i]) {
					s[1] = s[2] = c;
					goto ok;
				}
			}
			switch (c) {
			case ROCK_CELL: strncpy (s, " ## ", 4); break;
			case ' ': 
			case EMPTY_CELL: strncpy (s, "    ", 4); break;
			default: fprintf (stderr, "Warning: Illegal character in internal map (%c)\n", c);
			}
ok:
			while (0) { } ;  /* avoid compiler warning */
		}
	}

	return v_redisplay_everything ();
}


int v_display_cell (struct position *pos)
{
	return -1;
}

/** Called at the beginning and whenever amount of food changes. */
int v_set_food (struct position *pos, int count)
{
	char *s; 

	s=map_at_pos (FOOD_LAYER, pos);
	if (count > 0) {
		s[0] = '0'+count; 
	} else s[0] = ' ';
	return v_redisplay_everything ();
}


/** Called at the beginning. Id is just for distinguishing ants and 
    may be ignored. 
  */
int v_set_ant (struct ant *ant)
{
	char *s; 

	s=map_at_pos (ANT_LAYER, &ant->pos);
	s[1] = colony_home_chars[ant->colony];
	s[2] = direction_chars[ant->direction];
	s[3] = carries_food_chars[ant->carries_food];
	
	return v_redisplay_everything ();
}


/** Called when several steps are executed without updating each
    one individually.
  */
int v_remove_ant (struct position *pos)
{
	char *s; 

	s=map_at_pos (ANT_LAYER, pos);
	s[1] = ' ';
	s[2] = ' ';
	s[3] = ' ';
	
	return v_redisplay_everything ();
}

/** Called whenever ant moves. ant contains old position, to is new
    position. If ant just changes direction, to is equal to pos in ant. 
  */

int v_move_ant (struct ant *ant, struct position *to, int new_direction)
{
	char *s;
	struct ant new_ant;

	s=map_at_pos (ANT_LAYER, &ant->pos);
	strncpy (s, "    ", 4);

	new_ant = *ant;
	new_ant.pos = *to; 
	new_ant.direction = new_direction; 
	return v_set_ant (&new_ant); 
}


/** Called when ant picks up food. Position is in struct ant. */
int v_ant_picks_up_food (struct ant *ant, int old_food_amount, int new_food_amount)
{
	return v_set_food (&ant->pos, new_food_amount);
}


/** Called when ant picks up food. Position is in struct ant. */
int v_ant_drops_food (struct ant *ant, int old_food_amount, int new_food_amount)
{
	return v_set_food (&ant->pos, new_food_amount);
}


/** Ant dies! It becomes food afterwards. */
int v_ant_is_killed (struct ant *ant, int old_food_amount, int new_food_amount)
{
	char *s;

	s=map_at_pos (ANT_LAYER, &ant->pos);
	strncpy (s, "    ", 4);

	return v_set_food (&ant->pos, new_food_amount);
}


/** Called at the beginning to set any markers. This is used when a state
    is restored from a file. */
int v_set_marker (struct position *pos, int team, int new_marker)
{
	char *s;

	s=map_at_pos (FOOD_LAYER, pos);
	if (new_marker == 0) {
		s[team+1] = ' ';
	} else {
		s[team+1] = '@'+new_marker;
	}

	return v_redisplay_everything ();

}

/** Marker changes. Old marker and new marker are encoded bitwise. Normally, 
    only one bit should change.  
  */ 
int v_ant_changes_marker (struct ant *ant, int old_marker, int new_marker)
{
	return v_set_marker (&ant->pos, ant->colony, new_marker);
}


/** NXRunAltertPanel ... */
int v_display_message (const char *msg)
{
	printf ("%s\n", msg);	
	getchar ();

	return v_redisplay_everything ();
}


int v_display_status_line (const char *line)
{
	printf ("%s\n", line);	

	return 0;
}
	

int v_query_pointer (struct position *pos)
{
	pos->x = pos->y = -1;
	return 0;
}



/** Pass a comment line to the output driver. Comments may, for example, 
    define color maps for displaying different marker values. The 
    vizualiation driver does not touch these */

int v_comment (const char *comment)
{
	printf ("Comment line:\n%s\n", comment);	
	getchar ();

	return v_redisplay_everything ();
}



void v_disable_update (void)
{
}

void v_enable_update (void)
{
}


