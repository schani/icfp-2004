
#include <stdlib.h>

#include <SDL/SDL.h>
#include <sim-noflip.h>


// static	unsigned int frame = 0;


SDL_Surface *screen;

#ifdef	TRUE_COLOR
#define	CDEPTH	24
#else
#define	CDEPTH	8
#endif



SDL_Surface *sdl_screen(int width, int height)
{
	SDL_Surface *screen;


	screen = SDL_SetVideoMode(width, height, CDEPTH, 
		SDL_HWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE);
	if ( screen == 0 ) {
	    fprintf(stderr, "Couldn't set video mode: %s\n",
		    SDL_GetError());
	    return NULL;
	}
	return screen;
}

void	sdl_init(void)
{
	screen = sdl_screen(width, height);

	/* Initialize defaults, Video and Audio */
	if ((SDL_Init(SDL_INIT_VIDEO)==-1))
	  {
	    printf("Could not initialize SDL: %s.\n", SDL_GetError());
	    exit(-1);
	  }

	/* Clean up on exit */
	atexit(SDL_Quit);
}


void	sdl_clear(void)
{
	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 100, 100, 100));
}


void	sdl_resize(void)
{
	screen = sdl_screen(width, height);
	sdl_clear();
}

void	sdl_flip(void)
{
/*	SDL_Flip(screen); */ 
}

void	sdl_setcols(SDL_Color *colors, int first, int ncolors)
{
	SDL_SetColors(screen, colors, first, ncolors);
}


#include "sdl.h"

/* hex setup */

#include "sdl.h"
#include "sim.h"


static	int hex_pos[6][2];
static	int hex_base = 8;


void	setup_hex(int base)
{
	hex_base = base;
	
	hex_pos[0][1] = hex_pos[4][0] = hex_pos[5][0] = -base*2;
	hex_pos[1][0] = hex_pos[2][0] = hex_pos[3][1] = base*2;
	hex_pos[1][1] = hex_pos[5][1] = -base;
	hex_pos[2][1] = hex_pos[4][1] = base;
}

int mouse_to_hex(int x, int y, int *iv, int *jv)
{
	int i,j;
	
	j = (y - ( hex_base + offsety))/(3*hex_base);
	i = (x - ( hex_base*2 + offsetx) - (j%2)*hex_base)/(2*hex_base*2);
	
	*iv = i; *jv = j;

	return 0;
}

#ifdef	TRUE_COLOR
#define	INDEX_COL(i)	cols[i].r, cols[i].g, cols[i].b, cols[i].unused
#define	CMODE(c)	c ## RGBA
#else
#define	INDEX_COL(i)	(i)
#define	CMODE(c)	c ## Color
#endif

#define	max(a,b)	((a>b)?(a):(b))
#define	min(a,b)	((a<b)?(a):(b))


void	draw_hex(int ix, int iy, hex *h, int do_update_rect)
{
	static Sint16 vx[3] = {0,0,0};
	static Sint16 vy[3] = {0,0,0};
	SDL_Surface *dst = screen;
	int i,x,y;
	int minx, miny, maxx, maxy;

	x = 2*hex_base*(2*ix + (iy%2)) + ix*0 + hex_base*2 + offsetx;
	y = 3*hex_base*iy + hex_base*2 + iy*0 + offsety;
	minx = maxx = x;
	miny = maxy = y;
	
	vx[0] = x; vy[0] = y;
	vx[1] = x; vy[1] = y+hex_pos[0][1];
	for (i=0; i<7; i++) {
   	    vx[2] = x + hex_pos[i%6][0];
   	    vy[2] = y + hex_pos[i%6][1];

	    CMODE(filledPolygon) (dst, vx, vy, 3, INDEX_COL(h->cols[i%6]));
	    CMODE(line) (dst, vx[1], vy[1], vx[2], vy[2], INDEX_COL(FRAME_INDEX));
	    vx[1] = vx[2]; vy[1] = vy[2];

	    if (vx[1] > maxx) maxx = vx[1];
	    if (vx[1] < minx) minx = vx[1];

	    if (vy[1] > maxy) maxy = vy[1];
	    if (vy[1] < miny) miny = vy[1];
   	}
	if (h->type >= 0) {
    	    for (i= 0; i<=hex_base*0.8; i++) {
    		int v = i*2, w = min(hex_base*2-v, hex_base*1.5);

    		CMODE(vline) (dst, x-v, y-w, y+w, INDEX_COL(h->type));
    		if (i)
		    CMODE(vline) (dst, x+v, y-w, y+w, INDEX_COL(h->type));
    	    }
	}
	if (h->middle >= 0) {
	    CMODE(filledCircle) (dst, x, y, hex_base+1, INDEX_COL(h->middle));
	}
	CMODE(circle) (dst, x, y, hex_base+1, INDEX_COL(FRAME_INDEX));
	if (h->head >= 0) {
	    vx[0] = x + cos((h->head)*60*M_DEG)*(hex_base+1);
	    vy[0] = y + sin((h->head)*60*M_DEG)*(hex_base+1);
	    
	    CMODE(filledCircle) (dst, vx[0], vy[0], hex_base/2-1, INDEX_COL(HEAD_INDEX));
#if 0
	    CMODE(pixel) (dst, vx[0]+1, vy[0], INDEX_COL(HEAD_INDEX));
	    CMODE(pixel) (dst, vx[0]-1, vy[0], INDEX_COL(HEAD_INDEX));
	    CMODE(pixel) (dst, vx[0], vy[0]+1, INDEX_COL(HEAD_INDEX));
	    CMODE(pixel) (dst, vx[0], vy[0]-1, INDEX_COL(HEAD_INDEX));
	    CMODE(pixel) (dst, vx[0], vy[0], INDEX_COL(HEAD_INDEX));
#endif
	}

	CMODE(character) (dst, x-3, y-3, h->label, INDEX_COL(TEXT_INDEX));

	if (do_update_rect) {
		SDL_UpdateRect (screen, minx, miny, maxx-minx, maxy-miny);
	}
}
