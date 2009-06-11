/*
 * (c) 2004 Herbert Poetzl
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <linux/errno.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include "visualizer.h"
#include "ants_def.h"

#include "sim-noflip.h"
#include "sdl-noflip.h"





static	hex map[100][100];

static	int map_sizex, map_sizey;

int 	mask_sel = 0;
int	hex_size = 10;


int	offsetx = 0;
int	offsety = 0;

unsigned int width = 600;
unsigned int height = 500;

int	mousex = 0;
int	mousey = 0;



SDL_Color cols[256];

mask masks[16][6];



int v_initialize (void)
{
	sdl_init();
	setup_hex( hex_size );

	{
	    int i;

	    for (i=0; i<256; i++) {
		cols[i].r = i*4;
		cols[i].g = i*4;
		cols[i].b = i*4;
		cols[i].unused = 255;
	    }
	}
	

	MAKE_COLOR(TEXT_INDEX		,  64,  64,  64, 255);
	MAKE_COLOR(ROCK_INDEX		, 128, 128, 128, 255);
	MAKE_COLOR(ROCK2_INDEX		,  64,  64, 128, 255);
	MAKE_COLOR(EMPTY_INDEX		, 240, 240, 240, 255);
	MAKE_COLOR(FRAME_INDEX		, 200, 200, 200, 255);
	MAKE_COLOR(HEAD_INDEX		,   0,   0,   0, 255);

	MAKE_COLOR(RED_INDEX		, 240,   0,   0, 255);
	MAKE_COLOR(BLACK_INDEX		,   0, 128,   0, 255);
	MAKE_COLOR(RED_HOME_INDEX	, 240,   0,   0, 128);
	MAKE_COLOR(BLACK_HOME_INDEX	,   0, 128,   0, 128);

	MAKE_COLOR(RED_ANT_INDEX	, 240,   0,   0, 255);
	MAKE_COLOR(BLACK_ANT_INDEX	,   0, 160,   0, 255);

	MAKE_COLOR(RED_FOOD_INDEX	, 240, 128, 128, 255);
	MAKE_COLOR(BLACK_FOOD_INDEX	, 128, 200, 128, 255);


	MAKE_MASK(  0, 0,	MASK_RED,	0, 0x3, 32);
	MAKE_MASK(  0, 1,	MASK_RED,	2, 0x3, 32);
	MAKE_MASK(  0, 2,	MASK_RED,	4, 0x3, 32);
	MAKE_MASK(  0, 3,	MASK_BLACK,	0, 0x3, 32);
	MAKE_MASK(  0, 4,	MASK_BLACK,	2, 0x3, 32);
	MAKE_MASK(  0, 5,	MASK_BLACK,	4, 0x3, 32);

	MAKE_MASK(  1, 0,	MASK_RED,	0, 0x1, 64);
	MAKE_MASK(  1, 1,	MASK_RED,	1, 0x1, 64);
	MAKE_MASK(  1, 2,	MASK_RED,	2, 0x1, 64);
	MAKE_MASK(  1, 3,	MASK_RED,	3, 0x1, 64);
	MAKE_MASK(  1, 4,	MASK_RED,	4, 0x1, 64);
	MAKE_MASK(  1, 5,	MASK_RED,	5, 0x1, 64);

	MAKE_MASK(  2, 0,	MASK_BLACK,	0, 0x1, 64);
	MAKE_MASK(  2, 1,	MASK_BLACK,	1, 0x1, 64);
	MAKE_MASK(  2, 2,	MASK_BLACK,	2, 0x1, 64);
	MAKE_MASK(  2, 3,	MASK_BLACK,	3, 0x1, 64);
	MAKE_MASK(  2, 4,	MASK_BLACK,	4, 0x1, 64);
	MAKE_MASK(  2, 5,	MASK_BLACK,	5, 0x1, 64);

	sdl_setcols(&cols[64], 64, 16); 

	return 0;
}


int v_run_main_loop (void)
{
    while (v_poll_event (0));
	
    return 0;
}


int handle_key(SDLKey key)
{
	switch(key) {
	    case SDLK_KP0:
	    	mask_sel = 0;
		break;
	    case SDLK_KP1:
	    	mask_sel = 1;
		break;
	    case SDLK_KP2:
	    	mask_sel = 2;
		break;
	    case SDLK_KP3:
	    	mask_sel = 3;
		break;
	    case SDLK_KP4:
	    	mask_sel = 4;
		break;
	    case SDLK_KP5:
	    	mask_sel = 5;
		break;
	    case SDLK_KP6:
	    	mask_sel = 6;
		break;
	    case SDLK_KP7:
	    	mask_sel = 7;
		break;
	    case SDLK_KP8:
	    	mask_sel = 8;
		break;
	    case SDLK_KP9:
	    	mask_sel = 9;
		break;
	    
	    case SDLK_KP_PLUS:
	    	hex_size++;
		setup_hex( hex_size );
	    	break;
	    case SDLK_KP_MINUS:
	    	if (--hex_size < 2)
		    hex_size = 2;
		setup_hex( hex_size );
	    	break;
		
	    case SDLK_KP_DIVIDE:
	    	hex_size = 3;
		setup_hex( hex_size );
	    	break;
	    case SDLK_KP_MULTIPLY:
	    	hex_size = 12;
		setup_hex( hex_size );
	    	break;
		
 	    case SDLK_UP:
	    	offsety -= 10;
	    	break;
 	    case SDLK_DOWN:
	    	offsety += 10;
	    	break;
 	    case SDLK_RIGHT:
	    	offsetx += 10;
	    	break;
 	    case SDLK_LEFT:
	    	offsetx -= 10;
	    	break;

	    case SDLK_SPACE: return EVENT_TOGGLE_ANIMATION;
	    case SDLK_RETURN: return EVENT_START_ANIMATION;
	    case SDLK_q: return EVENT_STEP_FORWARD;
	    case SDLK_a: return EVENT_STEP_BACKWARD;
	    case SDLK_w: return EVENT_ROUND_FORWARD;
	    case SDLK_s: return EVENT_ROUND_BACKWARD;
	    case SDLK_e: return EVENT_10_ROUNDS_FORWARD;
	    case SDLK_d: return EVENT_10_ROUNDS_BACKWARD;
	    case SDLK_r: return EVENT_100_ROUNDS_FORWARD;
	    case SDLK_f: return EVENT_100_ROUNDS_BACKWARD;
	    case SDLK_t: return EVENT_1000_ROUNDS_FORWARD;
	    case SDLK_g: return EVENT_1000_ROUNDS_BACKWARD;
	    case SDLK_y: return EVENT_10000_ROUNDS_FORWARD;
	    case SDLK_h: return EVENT_10000_ROUNDS_BACKWARD;
		
	    case SDLK_1: return EVENT_STOP_ANIMATION;
	    case SDLK_2: return EVENT_ANIMATE_SLOWER;
	    case SDLK_3: return EVENT_ANIMATE_FASTER;
	    case SDLK_4: return EVENT_ANIMATE_FASTEST;

	    case SDLK_ESCAPE: return EVENT_QUIT;

	    default:
		printf("Key %d hit!\n", (int)key);
	    
	}
	return EVENT_REDISPLAY;
}

	
enum events v_poll_event (int timeout)
{
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
	switch( event.type ) {
	    case SDL_QUIT:
		printf("Got quit event!\n");
		return EVENT_QUIT;
		break;
	    case SDL_KEYDOWN:
	    	return handle_key(event.key.keysym.sym);
		break;
	    case SDL_MOUSEMOTION:
		mouse_to_hex(event.motion.x, event.motion.y, 
		    &mousex , &mousey);
	    	
	    
/*		printf("mouse %d,%d moved!\n", event.motion.x, event.motion.y); */
		break;
	    case SDL_ACTIVEEVENT:
		return EVENT_REDISPLAY;
		break;
	    case SDL_VIDEORESIZE:
	    	width = event.resize.w;
	    	height = event.resize.h;
		sdl_resize();
		return EVENT_REDISPLAY;
		break;
	}
    }
	
    return EVENT_NONE;
}


int v_set_geometry (int x, int y, int h)
{
        width = x;
        height = y;
        hex_size = h;

        return 0;
}



int v_query_pointer (struct position *pos)
{
	pos->x = mousex;
	pos->y = mousey;   /* ?? needs more */
	
	return 0;
}

int v_display_status_line (const char *line)
{
	fprintf (stderr, "%s\e[K\r", line);  /* ?? in main window. */

	return 0;
}


int v_load_map (int sizex, int sizey, char *charmap)
{
	char c;
	hex cell;
	int index,i,j,k;
	
//	assert(sizex <= 100);
//	assert(sizey <= 100);
	map_sizex = sizex;
	map_sizey = sizey;
	
	cell.head = -1;
	cell.middle = -1;
	cell.label = ' ';
	
	for (j=0; j<sizey; j++) {
	    for (i=0; i<sizex; i++) {
		c = charmap[sizex*j+i];
		
		cell.type = -1;
		index = 0;
		switch (c) {
		    case ROCK_CELL:
			index = ROCK_INDEX;
			cell.type = ROCK2_INDEX;
			break;
		    case EMPTY_CELL:
			index = EMPTY_INDEX;
			cell.type = -1;
			break;
		    case '+':
			index = EMPTY_INDEX;
			cell.type = RED_HOME_INDEX;
			break;
		    case '-':
			index = EMPTY_INDEX;
			cell.type = BLACK_HOME_INDEX;
			break;
		}	
		for (k=0; k<6; k++)
		    cell.cols[k] = index;
		map[i][j] = cell;
	    }
	}
	return 0;
}

int sim_food_update (struct position *pos, int count)
{
	static char vals[] = " 123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	map[pos->x][pos->y].label = vals[count%36];
	return 0;
}


int v_set_food (struct position *pos, int count)
{
	sim_food_update (pos, count);
	return 0;
}


int sim_ant_update (struct ant *ant, struct position *pos, int add)
{
	int index;
	
	if (add) {
	    if (ant->carries_food) {
	    	if (ant->colony % 2)
	    	    index = BLACK_FOOD_INDEX;
	    	else
	    	    index = RED_FOOD_INDEX;
	    } else {
	    	if (ant->colony % 2)
	    	    index = BLACK_ANT_INDEX;
	    	else
	    	    index = RED_ANT_INDEX;
	    }
	    map[pos->x][pos->y].head = ant->direction;
	} else {
	    index = -1;
	    map[pos->x][pos->y].head = -1;
	}
	map[pos->x][pos->y].middle = index;
	return 0;
}

int v_set_ant (struct ant *ant)
{
	sim_ant_update (ant, &ant->pos, 1);
	return 0;
}

int v_remove_ant (struct position *pos)
{
	sim_ant_update (NULL, pos, 0);
	return 0;
}

int sim_marker_update (struct position *pos, int team, int value)
{
	int i, o;
	
	o = (team%2)?MASK_RED:MASK_BLACK;
	for (i=0; i<6; i++) {
	    if (masks[mask_sel][i].type == o)
	    	map[pos->x][pos->y].cols[i] = 
	    	    ((value >> masks[mask_sel][i].shift) &
		    masks[mask_sel][i].mask) + masks[mask_sel][i].offset;
	}

	return 0;
}

int v_set_marker (struct position *pos, int team, int value)
{
	sim_marker_update (pos, team, value);
	return 0;
}

int v_redisplay_everything (void)
{
	int i,j;

	sdl_clear();

	for (i=0; i<map_sizex; i++) {
	    for (j=0; j<map_sizey; j++) {
		draw_hex (i, j, &map[i][j], 0);
	    }
	}
	SDL_UpdateRect (screen, 0, 0, width, height);

	return 0;
}


/** Called whenever ant moves. ant contains old position, to is new
    position. If ant just changes direction, to is equal to pos in ant. 
  */
int v_move_ant (struct ant *ant, struct position *to, int new_direction)
{
	int i1,j1,i2,j2;
	
	i1 = ant->pos.x; j1 = ant->pos.y;
	sim_ant_update (ant, &ant->pos, 0);
	draw_hex (i1, j1, &map[i1][j1], 1);

	i2 = to->x; j2 = to->y;
	sim_ant_update (ant, to, 1);
	draw_hex (i2, j2, &map[i2][j2], 1);
	
	return 0;
}

/** Called when ant picks up food. Position is in struct ant. */
int v_ant_picks_up_food (struct ant *ant, int old_food_amount, int new_food_amount)
{
	int i,j;
	
	sim_food_update (&ant->pos, new_food_amount);
	i = ant->pos.x; j = ant->pos.y;
	draw_hex (i, j, &map[i][j], 1);

	return 0;
}

/** Called when ant picks up food. Position is in struct ant. */
int v_ant_drops_food (struct ant *ant, int old_food_amount, int new_food_amount)
{
	int i,j;
	
	sim_food_update (&ant->pos, new_food_amount);
	i = ant->pos.x; j = ant->pos.y;
	draw_hex (i, j, &map[i][j], 1);

	return 0;
}

/** Ant dies! It becomes food afterwards. */
int v_ant_is_killed (struct ant *ant, int old_food_amount, int new_food_amount)
{
	int i,j;
	
	i = ant->pos.x; j = ant->pos.y;
	sim_ant_update (ant, &ant->pos, 0);
	sim_food_update (&ant->pos, new_food_amount);
	draw_hex (i, j, &map[i][j], 1);

	return 0;
}

/** Marker changes. Old marker and new marker are encoded bitwise. Normally, 
    only one bit should change.  
?? review this for schani's suggestion. 
  */ 
int v_ant_changes_marker (struct ant *ant, int old_marker, int new_marker)
{
	int i,j;

	i = ant->pos.x; j = ant->pos.y;
	sim_marker_update (&ant->pos, (ant->colony%2), new_marker);
	draw_hex (i, j, &map[i][j], 1);

	return 0;
}


int v_display_cell (struct position *pos)
{
	int i,j;

	i = pos->x; j = pos->y;
	draw_hex (i, j, &map[i][j], 1);
	return 0;
}



