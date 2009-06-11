#include <stdlib.h>
#include "ants_def.h"

void fatal (const char *s) 
{
	fprintf (stderr, s);
	exit (-1);
}

int verify_world_pos (struct world *world, struct position *pos)
{
	if (!world) fatal ("No world!");
	if (!world->cells) fatal ("No cells in world!");
	
	if (!pos) fatal ("No position");

	if (pos->x >= world->size_x || pos->y >= world->size_y ||
	    pos->x < 0 || pos->y < 0) {
		if (pos->x!=-1 || pos->y!=-1) {
			if (warnings) {
				fprintf (stderr, "Warning: Position out of range (%d/%d)\n", pos->x, pos->y);
			}
		}
		return -1;
	}
	return 0;
}

struct cell* cell_at (struct world *world, struct position *pos)
{
	if (verify_world_pos (world, pos) != 0) {
		return NULL;	
	}

	return world->cells+pos->x+pos->y*world->size_x;
}

int position_move (struct position *pos, int direction) 
{
	while (direction<0) direction+=6;
	while (direction>5) direction-=6;

	switch (direction) {
	case 0: pos->x++; break;
	case 1: pos->x+=pos->y&1; pos->y++; break;
	case 2: pos->x+=(pos->y&1)-1; pos->y++; break;
	case 3: pos->x--; break;
	case 4: pos->x+=(pos->y&1)-1; pos->y--; break;
	case 5: pos->x+=pos->y&1; pos->y--; break;
	default: 
		fprintf (stderr, "Wrong direction in position_move\n");
		return -1;
	}
	return 0;
}


int verify_ant_position (struct world *world, struct position *pos)
{
	struct cell *cell;

	cell = cell_at (world, pos);	
	if (cell->rocky) return -1;
	if (cell->ant) return -1;
	return 0;
}


	
void free_world_trace (struct world *world)
{
	struct execution_step *e, *next;

	if (!world) return;
	e = world->world_trace;

	while (e) {
		next = e->next;
		free (e);
		e = next;
	}
	world->world_trace = NULL;
	world->cursor = NULL;
}


void free_world (struct world *world) 
{
	if (!world) return;
	free_world_trace (world);

	if (world->ants) free (world->ants);
	if (world->cells) free (world->cells);
	free (world);
}	

