
#include <stdlib.h>
#include <string.h>

#include "ants_def.h"



char *colony_home_chars = COLONY_HOMES;

static int find_ants (struct world *world)
  /* Finds ants in world. 
     Expects total_ants and cell to be filled out correctly. 
     world->ants is filled out.
   */

{
	int i;
	size_t ants_mem;
	struct position p;
	int ant_pos;
	struct cell *cell;

	if (world->ants) free (world->ants);
	ants_mem = world->total_ants * sizeof (struct ant);
	world->ants = (struct ant *) malloc (ants_mem);
	memset (world->ants, 0, ants_mem);

	ant_pos = 0;

/* The order is important, so that ant numbering matches those of 
   OCAML simulator. */
	for (p.y=0; p.y<world->size_y; p.y++) {
		for (p.x=0; p.x<world->size_x; p.x++) {
			cell = cell_at (world, &p);
			if (cell->home_of_colony != -1) {
				i = cell->home_of_colony;
				world->ants[ant_pos].pos = p;
				world->ants[ant_pos].colony = i;
				world->ants[ant_pos].id = ant_pos;
				cell->ant = &world->ants[ant_pos];

				ant_pos++;
			}
		}
	}

	return 0;
}



int read_world (struct world ** worldp, FILE *f)
{
	size_t cell_mem;
	struct position p;
	struct cell *cell;
	char c;
	int i;
	int w; 

	w = 0;
	*worldp = (struct world *) malloc (sizeof (struct world));
	memset (*worldp, 0, sizeof (struct world));

	fscanf (f, "%d\n", &(*worldp)->size_x);
	fscanf (f, "%d\n", &(*worldp)->size_y);

	cell_mem = sizeof (struct cell) * (*worldp)->size_x * (*worldp)->size_y;
	
	(*worldp)->cells = (struct cell *) malloc (cell_mem);
	memset ((*worldp)->cells, 0, cell_mem);

	p.x = p.y = 0;
	
	while (!feof (f) && p.y < (*worldp)->size_y) {
		cell = cell_at (*worldp, &p);
		cell->home_of_colony = -1;
		while ((c = fgetc (f)) == ' ') ;

		if (c=='#') {
			cell->rocky = 1; 
			goto char_ok;
		}
		for (i=0; i<NUM_COLONIES; i++) {
			if (c==colony_home_chars[i]) {
				cell->home_of_colony = i;
				(*worldp)->total_ants++;
				goto char_ok;
			}
		}
		if (c>='0' && c<='9') {
			cell->food=c-'0';
			goto char_ok;
		}
		if (c=='.') goto char_ok;

		fprintf (stderr, "Warning: Unknown character in world (%c)\n", c); 
		w++;
char_ok: 
		p.x++;
		if (p.x >= (*worldp) -> size_x) {
			while ((c = fgetc (f)) == ' ') ;
			if (c != '\n') {
				fprintf (stderr, "Warning: Line length mismatch at position (%d/%d)\n", p.x, p.y); 
				w++;
			}
			
			p.y++;
			p.x=0;
		}
	}

	find_ants (*worldp);
	
	return w;
}
	


	

