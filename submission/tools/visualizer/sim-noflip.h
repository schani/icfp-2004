#ifndef	_SIM_H_
#define	_SIM_H_


#ifndef M_PI
#define M_PI 3.14159265
#endif /* !M_PI */

#define	M_DEG	(M_PI/180.0)


#include <SDL/SDL.h>


extern SDL_Color cols[256];

extern int	offsetx;
extern int	offsety;

extern unsigned int	width;
extern unsigned int	height;


typedef	signed char cidx;

typedef
struct	_hex {
	cidx cols[6];
	cidx middle;
	cidx type;
	signed char head;
	char label;
} hex;


typedef
struct	_mask {
	enum {
	    MASK_RED,
	    MASK_BLACK,
	    MASK_STATE,
	} type;
	int shift;
	int mask;
	int offset;
} mask;



void	setup_hex(int base);
void	draw_hex(int ix, int iy, hex *h, int do_update_rect);


#define     MAKE_COLOR(i,rv,gv,bv,a)		\
		do {				\
		    cols[i].r = rv;	\
		    cols[i].g = gv;	\
		    cols[i].b = bv;	\
		    cols[i].unused = a;	\
		} while (0)

#define     MAKE_MASK(i,j,t,s,m,o)		\
		do {				\
		    masks[i][j].type = t;	\
		    masks[i][j].shift = s;	\
		    masks[i][j].mask = m;	\
		    masks[i][j].offset = o;	\
		} while (0)



#endif
