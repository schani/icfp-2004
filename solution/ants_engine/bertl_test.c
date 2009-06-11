#include "ants_def.h"
#include "visualizer.h"

extern int mask_sel;

int main ()
{
	struct ant ant, ant2;

	mask_sel = 1;

	v_set_geometry (300, 300, 10);
	v_initialize ();

	v_load_map (4, 4, "#####..##..#####");
	v_redisplay_everything ();

	memset (&ant, 0, sizeof (ant));
	ant.pos.x = 1;
	ant.pos.y = 1;

	memset (&ant2, 0, sizeof (ant2));
	ant2.colony = 1;
	ant2.pos.x = 1;
	ant2.pos.y = 1;

	v_ant_changes_marker (&ant, 0, 1);
	getchar ();

	v_ant_changes_marker (&ant2, 0, 0);
	getchar ();

	v_ant_changes_marker (&ant, 0, 0);
	getchar ();

	v_set_marker (&ant.pos, 0, 1);
	v_set_marker (&ant.pos, 1, 0);
	v_display_cell (&ant.pos);
	getchar ();

	v_set_marker (&ant.pos, 0, 0);
	v_display_cell (&ant.pos);
	getchar ();
}

	

	
