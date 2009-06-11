
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "ants_def.h"


static char *consume (char *s, const char *s1) 
{
/*	const char *s2 = s; 
	const char *s3 = s1;  */

	while (*s == *s1) { s++; s1++; }
	if (*s1) {
/*		fprintf (stderr, "Failed to consume %s in %s\n", s2, s3); */
		return NULL;
	}
	return s;
}
	

struct ant *parse_ant (struct world *world, char *s, char ** s2, int color, struct position *p)
{
	int id; 

	s = consume (s, "ant of id ");
	id=strtol (s, &s, 10);

	world->ants[id].colony = color; 
	world->ants[id].id = id; 
	world->ants[id].pos = *p; 

	s = consume (s, ", dir ");
	world->ants[id].direction = strtol (s, &s, 10);

	s = consume (s, ", food ");
	world->ants[id].carries_food = strtol (s, &s, 10);

	s = consume (s, ", state ");
	world->ants[id].state = strtol (s, &s, 10);

	s = consume (s, ", resting ");
	world->ants[id].resting = strtol (s, &s, 10);

	*s2 = s;
	return &world->ants[id];
}

#define MAX_LINE 255

int read_world_as_icfp_dump (struct world ** worldp, FILE *f, int sizex, int sizey, int num_ants)
{
	size_t cell_mem;
	size_t ants_mem;
	struct position p;
	struct cell *cell;

	char *s;
	int cells_read;
	char buf[MAX_LINE];

	*worldp = (struct world *) malloc (sizeof (struct world));
	memset (*worldp, 0, sizeof (struct world));
	(*worldp)->size_x = sizex;
	(*worldp)->size_y = sizey;

	cell_mem = sizeof (struct cell) * (*worldp)->size_x * (*worldp)->size_y;
	(*worldp)->cells = (struct cell *) malloc (cell_mem);
	memset ((*worldp)->cells, 0, cell_mem);

	ants_mem = sizeof (struct ant) * num_ants;
	(*worldp)->ants = (struct ant *) malloc (ants_mem);
	memset ((*worldp)->ants, 0, ants_mem);

	(*worldp)->total_ants = num_ants;

	cells_read = 0;
	while (cells_read < sizex*sizey && !feof (f)) {
		if (fgets (buf, MAX_LINE, f) == NULL) {
			return -1;
		}

		s=buf;

		if ((s=consume (s, "cell (")) == NULL) continue;
		p.x=strtol (s, &s, 10);
		if ((s=consume (s, ", ")) == NULL) continue;
		p.y=strtol (s, &s, 10);
		if ((s=consume (s, "):")) == NULL) continue;
	
		cell = cell_at (*worldp, &p);
		cell->home_of_colony = -1;
		
		while (*s && *s != '\n') {
			while (*s == ' ' || *s == ';') s++;

			if (*s == 'r' && s[1] == 'o') {
				if ((s=consume (s, "rock")) == NULL) continue;
				cell->rocky = 1; 
			} else if (*s == 'r' && s[1] == 'e') {
				s=consume (s, "red ");
				switch (*s) {
				case 'h':
					s=consume (s, "hill");
					cell->home_of_colony = 0;
					break;
				
				case 'a':
					cell->ant=parse_ant (*worldp, s, &s, 0, &p);
					break;

				case 'm':
					s=consume (s, "marks: ");
					while (isdigit (*s)) {
						cell->markers[0]|=1<<(*s-'0');
						s++;
					}
					break;
				}
			} else if (*s == 'b') {
				s=consume (s, "black ");
				switch (*s) {
				case 'h':
					s=consume (s, "hill");
					cell->home_of_colony = 1;
					break;
				
				case 'a':
					cell->ant=parse_ant (*worldp, s, &s, 1, &p);
					break;

				case 'm':
					s=consume (s, "marks: ");
					while (isdigit (*s)) {
						cell->markers[1]|=1<<(*s-'0');
						s++;
					}
					break;
				}
			} else if (*s >= '0' && *s <= '9') {
				cell->food = strtol (s, &s, 10);
				s = consume (s, " food;");
			}
		}
		cells_read++;
	}
	if (cells_read < sizex*sizey) return -1;

	return 0;
}
	


	

