#ifndef	_PRIV_SDL_H_
#define	_PRIV_SDL_H_

#include <SDL/SDL.h>
#include <SDL_gfxPrimitives.h>


void	sdl_init(void);
SDL_Surface *sdl_screen(int width, int height);

void	sdl_setcols(SDL_Color *colors, int first, int ncolors);
void	sdl_clear(void);
void	sdl_flip(void);
void	sdl_resize(void);

int 	mouse_to_hex(int x, int y, int *i, int *j);

SDL_Surface *screen;


enum {
    TEXT_INDEX=64,
    ROCK_INDEX,
    ROCK2_INDEX,
    EMPTY_INDEX,
    FRAME_INDEX,
    HEAD_INDEX,

    RED_INDEX,
    BLACK_INDEX,
    RED_HOME_INDEX,
    BLACK_HOME_INDEX,

    RED_ANT_INDEX,
    BLACK_ANT_INDEX,

    RED_FOOD_INDEX,
    BLACK_FOOD_INDEX,

};


#define TRUE_COLOR

#endif
