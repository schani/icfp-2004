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

#include "sim.h"
#include "sdl.h"





static	hex map[100][100];

int 	map_sizex;
int 	map_sizey;

int 	mask_sel = 0;
int	hex_size = 10;


int	offsetx = 0;
int	offsety = 0;

int	width = 600;
int	height = 500;

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
		cols[i].r = i*16;
		cols[i].g = i*16;
		cols[i].b = i*16;
		cols[i].unused = 255;
	    }
	}
	

	MAKE_COLOR(TEXT_INDEX		,  64,  64,  64, 255);
	MAKE_COLOR(STATUS_INDEX		, 255, 255, 255, 255);
	
	MAKE_COLOR(ROCK_INDEX		, 128, 128, 128, 255);
	MAKE_COLOR(ROCK2_INDEX		,  64,  64, 128, 255);
	MAKE_COLOR(EMPTY_INDEX		, 240, 240, 240, 255);
	MAKE_COLOR(FRAME_INDEX		, 200, 200, 200, 255);
	MAKE_COLOR(HEAD_INDEX		,   0,   0,   0, 255);

	MAKE_COLOR(RED_INDEX		, 240,   0,   0, 255);
	MAKE_COLOR(BLACK_INDEX		,   0, 128,   0, 255);
	MAKE_COLOR(RED_HOME_INDEX	, 200,   0,   0, 128);
	MAKE_COLOR(BLACK_HOME_INDEX	,   0, 200,   0, 128);

	MAKE_COLOR(RED_ANT_INDEX	, 240,   0,   0, 255);
	MAKE_COLOR(BLACK_ANT_INDEX	,   0, 160,   0, 255);

	MAKE_COLOR(RED_FOOD_INDEX	, 240, 128, 128, 255);
	MAKE_COLOR(BLACK_FOOD_INDEX	, 128, 200, 128, 255);

	MAKE_COLOR(S_BACK_INDEX		, 240, 240, 240, 255);
	MAKE_COLOR(S_DIR_INDEX		,   0,   0,   0, 255);
	MAKE_COLOR(S_BIT3_INDEX		, 255, 200, 200, 255);
	MAKE_COLOR(S_BIT4_INDEX		, 200, 255, 200, 255);
	MAKE_COLOR(S_BIT5_INDEX		, 200, 200, 255, 255);


	MAKE_MASK(  4, 0,	MASK_RED,	0, 0x3, 32);
	MAKE_MASK(  4, 1,	MASK_RED,	2, 0x3, 32);
	MAKE_MASK(  4, 2,	MASK_RED,	4, 0x3, 32);
	MAKE_MASK(  4, 3,	MASK_BLACK,	0, 0x3, 32);
	MAKE_MASK(  4, 4,	MASK_BLACK,	2, 0x3, 32);
	MAKE_MASK(  4, 5,	MASK_BLACK,	4, 0x3, 32);

	MAKE_MASK(  5, 0,	MASK_RED,	0, 0x1, 64);
	MAKE_MASK(  5, 1,	MASK_RED,	1, 0x1, 64);
	MAKE_MASK(  5, 2,	MASK_RED,	2, 0x1, 64);
	MAKE_MASK(  5, 3,	MASK_RED,	3, 0x1, 64);
	MAKE_MASK(  5, 4,	MASK_RED,	4, 0x1, 64);
	MAKE_MASK(  5, 5,	MASK_RED,	5, 0x1, 64);

	MAKE_MASK(  6, 0,	MASK_BLACK,	0, 0x1, 64);
	MAKE_MASK(  6, 1,	MASK_BLACK,	1, 0x1, 64);
	MAKE_MASK(  6, 2,	MASK_BLACK,	2, 0x1, 64);
	MAKE_MASK(  6, 3,	MASK_BLACK,	3, 0x1, 64);
	MAKE_MASK(  6, 4,	MASK_BLACK,	4, 0x1, 64);
	MAKE_MASK(  6, 5,	MASK_BLACK,	5, 0x1, 64);


	sdl_setcols(&cols[64], 64, 16); 

	return 0;
}


int v_run_main_loop (void)
{
    while (v_poll_event (0));
	
    return 0;
}


void v_disable_update (void)
{
    	sdl_disable_update();
}

void v_enable_update (void)
{
    	sdl_enable_update();
}

void v_setup_hex(int val)
{
        hex_size = max(min(val, 100), 2);
	setup_hex( hex_size );
}


int v_zoom_in (int x, int y)
{
	offsetx -= x - width/2;
	offsety -= y - height/2;
	hex_size++;
    	setup_hex( hex_size );
	return 0;
}

int v_zoom_out (int x, int y)
{
	offsetx -= x - width/2;
	offsety -= y - height/2;
	if (--hex_size < 2)
    	    hex_size = 2;
	setup_hex( hex_size );
	return 0;
}

int v_center_view (int x, int y)
{
	offsetx -= x - width/2;
	offsety -= y - height/2;
	return 0;
}

int v_recenter_view (void)
{
	offsetx = 0;
	offsety = 0;
	return 0;
}

int v_refit_view (void)
{
    	int fitx, fity;

	offsetx = 0;
	offsety = 0;
	
	fitx = (width-4)/(4*map_sizex+1);
	fity = (height-12)/(3*map_sizey+1);
	setup_hex( min(fitx, fity) );
	return 0;
}



int handle_key(SDLKey key)
{
	printf("Key %d hit!\n", (int)key);
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
	    	v_setup_hex(++hex_size);
	    	break;
	    case SDLK_KP_MINUS:
	    	v_setup_hex(--hex_size);
	    	break;
		
	    case SDLK_KP_DIVIDE:
		v_setup_hex(2);
	    	break;
	    case SDLK_KP_MULTIPLY:
		v_setup_hex(12);
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

    	    case SDLK_HOME:
	    	v_recenter_view();
	    	break;
		
    	    case SDLK_END:
	    	v_refit_view();
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
	    	break;
		mouse_to_hex(event.motion.x, event.motion.y, 
		    &mousex , &mousey);
		break;
    	    case SDL_MOUSEBUTTONDOWN:
	    	switch (event.button.button) {
		    case 1:
		    	v_zoom_in(event.button.x, event.button.y);
			break;
			
		    case 2:
		    	v_center_view(event.button.x, event.button.y);
			break;
			
		    case 3:
		    	v_zoom_out(event.button.x, event.button.y);
			break;
		}
		return EVENT_REDISPLAY;
	    	break;
    	    case SDL_MOUSEBUTTONUP:
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
	setup_hex( hex_size );
	
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
    	draw_status(line);
#ifdef	DBL_DRAW
	sdl_flip();
    	draw_status(line);
#endif
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
		    
		    default:
		    	printf("strange map char %d '%c'\n", c, c);
			break;
		}	
		for (k=0; k<6; k++)
		    cell.cols[k] = index;
		map[i][j] = cell;
	    }
	}
	return 0;
}

static inline 
int sim_food_update (int i, int j, struct cell *cell)
{
	static char vals[] = 
	    " 123456789"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "abcdefghijklmnopqrstuvwxyz"
	    "**************************";
    	int count;
    	
    	count = min(cell->food, 64);
	map[i][j].label = vals[count];
	return 0;
}

static inline 
int sim_ant_update (int i, int j, struct ant *ant, int add)
{
	int index;
	
	if (add && !ant->dead) {
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
	    map[i][j].head = ant->direction;
	} else {
	    index = -1;
	    map[i][j].head = -1;
	}
	map[i][j].middle = index;
	return 0;
}

int sim_marker_update (int i, int j, int team, int value)
{
	int k, o;
	
	o = (team%2)?MASK_RED:MASK_BLACK;
    	if (mask_sel < 4) {
	    int dir = (value & 0x07);
	    
	    k = 1;
	    if ((1 << (team%2)) & mask_sel) {
		map[i][j].cols[(k+dir+0)%6] = (dir)?S_DIR_INDEX:S_BACK_INDEX;
		map[i][j].cols[(k+dir-1)%6] = S_BACK_INDEX;
		map[i][j].cols[(k+dir+1)%6] = S_BACK_INDEX;
	
		map[i][j].cols[(k+dir+3)%6] =
		    ((value>>3)&1)?S_BIT3_INDEX:S_BACK_INDEX;
		map[i][j].cols[(k+dir+2)%6] =
		    ((value>>4)&1)?S_BIT4_INDEX:S_BACK_INDEX;
		map[i][j].cols[(k+dir+4)%6] =
		    ((value>>5)&1)?S_BIT5_INDEX:S_BACK_INDEX;
    	    }
	} else {
	    for (k=0; k<6; k++) {
	    	if (masks[mask_sel][k].type == o)
	    	    map[i][j].cols[k] =
	    		((value >> masks[mask_sel][k].shift) &
	    		masks[mask_sel][k].mask) + masks[mask_sel][k].offset;
	    }
    	}
	return 0;
}

/*

static inline 
int sim_ant_turn (struct ant *ant, int i, int j, int new_direction)
{
	map[i][j].head = new_direction;
	return 0;
}



*/

int v_redisplay_everything (void)
{
	int i,j;

	sdl_disable_update();
	sdl_clear();

	for (i=0; i<map_sizex; i++) {
	    for (j=0; j<map_sizey; j++) {
		draw_hex (i, j, &map[i][j]);
	    }
	}

	sdl_enable_update();
	// sdl_flip();
	return 0;
}


/** Called whenever ant moves. ant contains old position, to is new
    position. If ant just changes direction, to is equal to pos in ant. 
  */
int v_move_ant (struct ant *ant, struct cell *cell, struct position *old)
{
	int i1,j1,i2,j2;
	
	i1 = old->x; j1 = old->y;
	sim_ant_update (i1, j1, ant, 0);
	draw_hex (i1, j1, &map[i1][j1]);

	i2 = ant->pos.x; j2 = ant->pos.y;
	sim_ant_update (i2, j2, ant, 1);
	draw_hex (i2, j2, &map[i2][j2]);
	
#ifdef	DBL_DRAW
	sdl_flip();
	draw_hex (i1, j1, &map[i1][j1]);
	draw_hex (i2, j2, &map[i2][j2]);
#endif
	return 0;
}


int v_turn_ant (struct ant *ant, int old_direction)
{
	int i,j;
	
	i = ant->pos.x; j = ant->pos.y;	
	sim_ant_update (i, j, ant, 1);
	draw_hex (i, j, &map[i][j]);
	
#ifdef	DBL_DRAW
	sdl_flip();
	draw_hex (i, j, &map[i][j]);
#endif
	return 0;
}

/** Called when ant picks up food. Position is in struct ant. */
int v_ant_picks_up_food (struct ant *ant, struct cell *cell)
{
	int i,j;
	
	i = ant->pos.x; j = ant->pos.y;
	sim_food_update (i, j, cell);
	sim_ant_update (i, j, ant, 1);
	draw_hex (i, j, &map[i][j]);

#ifdef	DBL_DRAW
	sdl_flip();
	draw_hex (i, j, &map[i][j]);
#endif
	return 0;
}

/** Called when ant picks up food. Position is in struct ant. */
int v_ant_drops_food (struct ant *ant, struct cell *cell)
{
	int i,j;
	
	i = ant->pos.x; j = ant->pos.y;
	sim_food_update (i, j, cell);
	sim_ant_update (i, j, ant, 1);
	draw_hex (i, j, &map[i][j]);

#ifdef	DBL_DRAW
	sdl_flip();
	draw_hex (i, j, &map[i][j]);
#endif
	return 0;
}

/** Ant dies! It becomes food afterwards. */
int v_ant_is_killed (struct ant *ant, struct cell *cell)
{
	int i,j;
	
	i = ant->pos.x; j = ant->pos.y;
	sim_food_update (i, j, cell);
	sim_ant_update (i, j, ant, 0);
	draw_hex (i, j, &map[i][j]);

#ifdef	DBL_DRAW
	sdl_flip(); 
	draw_hex (i, j, &map[i][j]);
#endif
	return 0;
}

/** Ant is resurrected! needs food. */
int v_ant_is_resurrected (struct ant *ant, struct cell *cell)
{
	int i,j;
	
	i = ant->pos.x; j = ant->pos.y;
	sim_food_update (i, j, cell);
	sim_ant_update (i, j, ant, 1);
	draw_hex (i, j, &map[i][j]);

#ifdef	DBL_DRAW
	sdl_flip(); 
	draw_hex (i, j, &map[i][j]);
#endif
	return 0;
}

/** Marker changes. Old marker and new marker are encoded bitwise. Normally, 
    only one bit should change.  
?? review this for schani's suggestion. 
  */ 
int v_ant_changes_marker (struct ant *ant, struct cell *cell, int *old_markers)
{
	int i,j;

	i = ant->pos.x; j = ant->pos.y;
	if (old_markers[0] != cell->markers[0])
	    sim_marker_update (i, j, 0, cell->markers[0]);
	if (old_markers[1] != cell->markers[1])
	    sim_marker_update (i, j, 1, cell->markers[1]);
	draw_hex (i, j, &map[i][j]);

#ifdef	DBL_DRAW
	sdl_flip();
	draw_hex (i, j, &map[i][j]);
#endif
	return 0;
}


int v_display_cell (struct position *pos)
{
	int i,j;

	i = pos->x; j = pos->y;
//	printf("display_cell [%d,%d]\n", i,j);
	draw_hex (i, j, &map[i][j]);
#ifdef	DBL_DRAW
	sdl_flip(); 
	draw_hex (i, j, &map[i][j]);
#endif
	return 0;
}


int v_set_food (struct position *pos, struct cell *cell)
{
	sim_food_update (pos->x, pos->y, cell);
	return 0;
}

int v_set_ant (struct ant *ant)
{
	sim_ant_update (ant->pos.x, ant->pos.y, ant, 1);
	return 0;
}

int v_remove_ant (struct position *pos, struct cell *cell)
{
	sim_ant_update (pos->x, pos->y, NULL, 0);
	return 0;
}


int v_set_marker (struct position *pos, int *markers)
{
	sim_marker_update (pos->x, pos->y, 0, markers[0]);
	sim_marker_update (pos->x, pos->y, 1, markers[1]);
	return 0;
}
