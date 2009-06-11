
#include <stdio.h>
#include <stdlib.h>

#include "ants_def.h"
#include "visualizer.h"

static int handle_dead_ants_around (struct position *pos)
{
/*	fprintf (stderr, "dead ants not yet implemented.\n"); */
	return 0;
}

static enum ant_action reverse_action[NUM_ACTIONS] = { 
        MOVE_BACKWARD,
        MOVE_FORWARD,
        TURN_RIGHT,
        TURN_LEFT,
        DROP_FOOD,
        PICKUP_FOOD,
        CLEAR_MARKER,
        SET_MARKER 
};

int skip_round_markers (struct world *world, int move_forward)
{
	if (move_forward) {
		while (world->cursor && world->cursor->next && !world->cursor->ant) { 
			world->cursor = world->cursor->next;
			world->round++;
		}
	} else {
		while (world->cursor && world->cursor->prev && !world->cursor->ant) {
			world->cursor = world->cursor->prev;
			world->round--;
		}
	}

	return 0;
}

int execute_step (struct world *world, int display, int move_forward)
{
	struct position new_pos;
	struct cell *cell;
	int ret;
	enum ant_action action;
	int new_direction, direction;
	int new_marker;

	if (!world->cursor) {
		fprintf (stderr, "Internal error: cursor is NULL\n");
		return -1;
	}

	if (move_forward) {
		skip_round_markers (world, 1);

		if (!world->cursor->next) {
			fprintf (stderr, "Already at the end of simulation\n");
			return -1;
		}
	} else {
		if (!world->cursor->prev) {
			fprintf (stderr, "Already at the beginning of simulation\n");
			return -1;
		}
		world->cursor = world->cursor->prev;
		skip_round_markers (world, 0);

		if (!world->cursor->ant) {
			fprintf (stderr, "Already at the beginning of simulation (file starts with round marker)\n");
			world->round--;
			return -1;
		}
		world->step--;
	}

	if (!world->cursor) {
		fprintf (stderr, "Internal error: cursor is NULL\n");
		return -1;
	}

	cell = cell_at (world, &world->cursor->ant->pos);
	if (world->cursor->ant != cell->ant) {
		fprintf (stderr, "Error in internal data structure: Ant not at cell.\n");
		return -1;
	}

	if (move_forward) {
		world->cursor->old_ant_state = cell->ant->state;
		cell->ant->state = world->cursor->new_ant_state;
	} else {
		cell->ant->state = world->cursor->old_ant_state;
	}
		
	action = world->cursor->action;
	if (!move_forward) {
		if (world->cursor->action_had_no_side_effects) {
			if (warnings) {
				fprintf (stderr, "Stepping back an action without side effects.\n");
			}
			return 0;
		}	
		action=reverse_action[action];  
	}	

	switch (action) {
	case MOVE_FORWARD: 
	case MOVE_BACKWARD: 
		new_pos = world->cursor->ant->pos;
		direction = world->cursor->ant->direction;	
		if (action == MOVE_BACKWARD) direction+=3;
		position_move (&new_pos, direction);

		if ((ret = verify_ant_position (world, &new_pos)) != 0) {
			if (warnings) {
				fprintf (stderr, "Ant crashing into another ant or rock!\n");
			}
			if (move_forward) {
				world->cursor->action_had_no_side_effects = 1;	
			}
			break;  /* this is a valid condition. */
		}

		if (display) {
			if ((ret=v_move_ant (world->cursor->ant, &new_pos, world->cursor->ant->direction)) != 0) {
				return ret;
			}
		}
		cell->ant = NULL;
		if (!display) cell->ant_needs_update = 1;

		world->cursor->ant->pos = new_pos;
		cell = cell_at (world, &new_pos);
		cell->ant = world->cursor->ant;
		if (!display) cell->ant_needs_update = 1;

		if ((ret=handle_dead_ants_around (&new_pos))!=0) return ret;
		break;

	case TURN_LEFT: 
	case TURN_RIGHT:
		new_direction = world->cursor->ant->direction;

		if (action == TURN_RIGHT) {
			new_direction++;
		} else {
			new_direction--;
		}
		if (new_direction < 0) new_direction+=6;
		if (new_direction > 5) new_direction-=6;
		
		if (display) {
			if ((ret=v_move_ant (world->cursor->ant, &world->cursor->ant->pos, new_direction)) != 0) {
				return ret;
			}
		}
		world->cursor->ant->direction = new_direction; 
		if (!display) cell->ant_needs_update = 1;

		break;

	case PICKUP_FOOD: 
		if (world->cursor->ant->carries_food) {
			if (warnings) {
				fprintf (stderr, "Ant carriing food attempts to pick up food.\n");
			}
			if (move_forward) {
				world->cursor->action_had_no_side_effects = 1;	
			}
			break;
		}
		if (cell->food == 0) {
			if (warnings) {
				fprintf (stderr, "Ant attepmts to pick up food but there is none.\n");
			}
			if (move_forward) {
				world->cursor->action_had_no_side_effects = 1;	
			}
			world->cursor->action_had_no_side_effects = 1;	
			break;
		}
			
		if (display) {
			if ((ret=v_ant_picks_up_food (world->cursor->ant, cell->food, cell->food-1)) != 0) {
				return ret;
			}
		}
		cell->food--;
		if (!display) cell->food_needs_update = 1;
		world->cursor->ant->carries_food++;
		break;
			
	case DROP_FOOD: 
		if (!world->cursor->ant->carries_food) {
			if (warnings) {
				fprintf (stderr, "Ant carriing no food attempts to drop food.\n");
			}
			if (move_forward) {
				world->cursor->action_had_no_side_effects = 1;	
			}
			world->cursor->action_had_no_side_effects = 1;	
			break;
		}
		cell = cell_at (world, &world->cursor->ant->pos);
			
		if (display) {
			if ((ret=v_ant_drops_food (world->cursor->ant, cell->food, cell->food+1)) != 0) {
				return ret;
			}
		}
		cell->food++;
		if (!display) cell->food_needs_update = 1;

		world->cursor->ant->carries_food--;
		break;

	case SET_MARKER: 
	case CLEAR_MARKER: 
		new_marker = cell->markers[world->cursor->ant->colony];

		if (action == SET_MARKER) {
			new_marker |= (1<<world->cursor->marker);
		} else {
			new_marker &= ~(1<<world->cursor->marker);
		} 
			
		if (new_marker == cell->markers[world->cursor->ant->colony]) { 
			if (warnings) {
				fprintf (stderr, "Ant setting/clearing marker but it is already set/cleared.\n");
			}
			if (move_forward) {
				world->cursor->action_had_no_side_effects = 1;	
			}
			break;
		}

		if (display) {
			if ((ret=v_ant_changes_marker (world->cursor->ant, cell->markers[world->cursor->ant->colony], new_marker)) != 0) {
				return ret;
			}
		}
			
		cell->markers[world->cursor->ant->colony] = new_marker;
		if (!display) cell->markers_need_update = 1;

		break;

	default:
		fprintf (stderr, "Illegal ant action\n");
		return -1;
	}
			

	if (move_forward) {
		if (world->cursor->next) world->cursor = world->cursor->next;
		world->step++;
	}  /* when moving backwards this already has been done at the beginning. */ 
	return 0;
}


int update_everything (struct world *world)
{
	struct position p;
	struct cell *cell;
	int team;
	int need_display;

	for (p.x=0; p.x<world->size_x; p.x++) {
		for (p.y=0; p.y<world->size_y; p.y++) {
			cell = cell_at (world, &p);
			
			need_display = 0;
			if (cell->ant_needs_update) {
				if (cell->ant) {
					v_set_ant (cell->ant);
				} else {
					v_remove_ant (&p);
				}
				cell->ant_needs_update = 0;		
				need_display = 1;
			}
			if (cell->food_needs_update) {
				v_set_food (&p, cell->food);
				cell->food_needs_update = 0;
				need_display = 1;
			}
			if (cell->markers_need_update) {
				for (team=0; team<NUM_COLONIES; team++) {	
					v_set_marker (&p, team, cell->markers[team]);
				}
				cell->markers_need_update = 0;
				need_display = 1;
			}
			if (need_display) v_display_cell (&p);
		}
	}
			
	return 0;
}

int step_forward (struct world *world, int display)
{
	return execute_step (world, display, 1);
}

int step_backward (struct world *world, int display)
{
	return execute_step (world, display, 0);
}

/* A round forward. If display is non-zero the display is updated. */
int round_forward (struct world *world, int display)
{
	int ret; 

	if ((ret=skip_round_markers (world, 1))!=0) return ret;
	while (world->cursor && world->cursor->ant) {
		if ((ret=step_forward (world, display))!=0) return ret;
	}
	return 0;
}
		

/* A round backward. If display is non-zero the display is updated. */
int round_backward (struct world *world, int display)
{
	int ret; 

	if ((ret=skip_round_markers (world, 0)) != 0) return ret;
	while (world->cursor && world->cursor->ant) {
		if ((ret=step_backward (world, display)) != 0) return ret;
	}
	return 0;
}

int n_rounds (struct world *world, int delta)
{
	int i; 
	int display_incrementally;
	int ret, ret2;

	if (abs (delta) < 10) {
		display_incrementally = 1; 
	} else {
		display_incrementally = 0;
	}

	if (delta<0) {
		for (i=0;i>delta;i--) {
			if ((ret=round_backward (world, display_incrementally)) != 0) break;
		}
	} else {
		for (i=0;i<delta;i++) {
			if ((ret=round_forward (world, display_incrementally)) != 0) break;
		}
	}

	if (!display_incrementally) {  
		if ((ret2=update_everything (world))!=0) return ret2;
	}
	
	return ret;
}
		

int n_steps (struct world *world, int delta)
{
	int i; 
	int display_incrementally;
	int ret, ret2;

	if (abs (delta) < 10) {
		display_incrementally = 1; 
	} else {
		display_incrementally = 0;
	}

	if (delta<0) {
		for (i=0;i>delta;i--) {
			if ((ret=step_backward (world, display_incrementally)) != 0) break;
		}
	} else {
		for (i=0;i<delta;i++) {
			if ((ret=step_forward (world, display_incrementally)) != 0) break;
		}
	}

	if (!display_incrementally) {  
		if ((ret2=update_everything (world))!=0) return ret2;
	}
	
	return ret;
}
		

/* Go to last position. If display is non-zero the display is updated. */
int goto_last (struct world *world, int display)
{
	while (step_forward (world, display) == 0) ;
	return 0;
}

/* Go to first position. If display is non-zero the display is updated. */
int goto_first (struct world *world, int display)
{
	while (step_backward (world, display) == 0) ;
	return 0;
}




