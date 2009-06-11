
#include <stdio.h>
#include <stdlib.h>

#include "ants_def.h"
#include "visualizer.h"


extern int do_animation, do_one_step;

void halt_simulation(void)
{
    	do_one_step = 1;
     	do_animation = 0;
}



static int do_kill_ant (struct world *world, struct ant *ant, int display)
{
    	struct cell *cell;
    	int value = 3;

	cell = cell_at (world, &ant->pos);
	ant->dead = 1;
	cell->ant = NULL;
	
	if (ant->carries_food)
	    value++;
	cell->food += value;

	if (display) {
    	    	v_ant_is_killed (ant, cell);
	} else {
    	    	cell->food_needs_update = 1;
    	    	cell->ant_needs_update = 1;
	}
	return 0;
}

static int do_resurrect_ant (struct world *world, struct ant *ant, int display)
{
    	struct cell *cell;
    	int value = 3;

	cell = cell_at (world, &ant->pos);
	ant->dead = 0;
	cell->ant = ant;
	
	if (ant->carries_food)
	    value++;
	cell->food -= value;
	
	if (display) {
    	    	v_ant_is_resurrected (ant, cell);
	} else {
    	    	cell->food_needs_update = 1;
    	    	cell->ant_needs_update = 1;
    	}
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
        SET_MARKER,
	RESURRECT_ANT,
	KILL_ANT,
	SENSE_HERE,
	SENSE_LEFT,
	SENSE_AHEAD,
	SENSE_RIGHT,
};

int skip_round_markers (struct world *world, int forward)
{
	if (forward) {
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

int execute_step (struct world *world, int display, int forward)
{
	struct position old_pos, new_pos;
	struct cell *cell;
	int ret;
	enum ant_action action;
	int team, direction, old_direction;
	int old_markers[NUM_COLONIES];

	if (!world->cursor) {
		fprintf (stderr, "Internal error: cursor is NULL\n");
		return -1;
	}

	if (forward) {
		skip_round_markers (world, 1);

		if (!world->cursor->next) {
			fprintf (stderr, "Already at the end of simulation\n");
		    	halt_simulation();
			return -1;
		}
	} else {
		if (!world->cursor->prev) {
			fprintf (stderr, "Already at the beginning of simulation\n");
		    	halt_simulation();
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
	if (world->cursor->ant != cell->ant &&
	    !world->cursor->ant->dead) {
		fprintf (stderr, "Error in internal data structure: Ant not at cell.\n");
		halt_simulation();
		return -1;
	}

		
	action = world->cursor->action;
	
	if (world->cursor->failed_action)
	    goto skip_action;
	    
	if (!forward) {
		action = reverse_action[action];  
	}	

	switch (action) {
	case MOVE_FORWARD: 
	case MOVE_BACKWARD: 
	
		old_pos = world->cursor->ant->pos;
		direction = world->cursor->ant->direction;	
		if (action == MOVE_BACKWARD) 
		    direction = (direction + 3) % 6;
		new_pos = old_pos;
		position_move (&new_pos, direction);

		if ((ret = verify_ant_position (world, &new_pos)) != 0) {
			if (warnings) 
				fprintf (stderr, "Ant crashing into another ant or rock!\n");
			halt_simulation();
			// break;  /* this is a valid condition. */
		}

		cell->ant = NULL;
		cell->ant_needs_update = 1;
		cell = cell_at (world, &new_pos);
		world->cursor->ant->pos = new_pos;
		cell->ant = world->cursor->ant;
		
		if (display)
		    v_move_ant (world->cursor->ant, cell, &old_pos);
		else
		    cell->ant_needs_update = 1;
		break;

	case TURN_LEFT: 
	case TURN_RIGHT:
		direction = world->cursor->ant->direction;
    	    	old_direction = direction;

		if (action == TURN_RIGHT) {
		    	if (direction == 5)
			    direction = 0;
			else
		    	    direction++;
		} else { 
		    	if (direction == 0)
			    direction = 5;
			else
		    	    direction--;
		}
				
		world->cursor->ant->direction = direction; 
		if (display)
		    v_turn_ant (world->cursor->ant, old_direction);
		else 
		    cell->ant_needs_update = 1;

		break;

	case PICKUP_FOOD: 
		if (world->cursor->ant->carries_food) {
			if (warnings)
			    fprintf (stderr, 
		    	    	"Ant %d at [%d,%d] carrying food attempts to pick up food.\n",
				world->cursor->ant->id, 
				world->cursor->ant->pos.x, 
				world->cursor->ant->pos.y);
			halt_simulation();
			break;
		}
		if (cell->food == 0) {
			if (warnings)
			    fprintf (stderr, 
		    	    	"Ant %d at [%d,%d] attempts to pick up food but there is none.\n",
				world->cursor->ant->id, 
				world->cursor->ant->pos.x, 
				world->cursor->ant->pos.y);
			halt_simulation();
			break;
		}
			
		cell->food--;
		world->cursor->ant->carries_food++;
		
		if (display)
		    v_ant_picks_up_food (world->cursor->ant, cell);
		else 
		    cell->food_needs_update = 1;
		break;
			
	case DROP_FOOD: 
		if (!world->cursor->ant->carries_food) {
			if (warnings)
			    fprintf (stderr, 
		    	    	"Ant %d at [%d,%d] carrying no food attempts to drop food.\n",
				world->cursor->ant->id, 
				world->cursor->ant->pos.x, 
				world->cursor->ant->pos.y);
			halt_simulation();
			break;
		}

		cell->food++;
		world->cursor->ant->carries_food--;

		if (display)
		    v_ant_drops_food (world->cursor->ant, cell);
    	    	else 
		    cell->food_needs_update = 1;
		break;

	case SET_MARKER: 
	case CLEAR_MARKER: 
	
		old_markers[0] = cell->markers[0];
		old_markers[1] = cell->markers[1];
		team = world->cursor->ant->colony;

		if (action == SET_MARKER) {
			cell->markers[team%2] |= (1 << world->cursor->marker);
		} else {
			cell->markers[team%2] &= ~(1 << world->cursor->marker);
		} 
			
		if (cell->markers[team%2] == old_markers[team%2]) { 
			if (warnings)
				fprintf (stderr, "Ant setting/clearing marker but it is already set/cleared.\n");
			if (forward)
				world->cursor->failed_action = 1;	
			break;
		}

		if (display)
		    v_ant_changes_marker (world->cursor->ant, cell, old_markers);
		else 
		    cell->markers_need_update = 1;

		break;

	case KILL_ANT: 
	    	do_kill_ant(world, world->cursor->ant, display);
	    	break;
		
	case RESURRECT_ANT:	
	    	do_resurrect_ant(world, world->cursor->ant, display);
	    	break;
	 
	case SENSE_HERE:
	case SENSE_LEFT:
	case SENSE_AHEAD:
	case SENSE_RIGHT:
	    	break;

	default:
		fprintf (stderr, "Illegal ant action\n");
		return -1;
	}

    	if (cell->ant) {
	    if (forward) {
	    	    world->cursor->old_ant_state = cell->ant->state;
	    	    cell->ant->state = world->cursor->new_ant_state;
	    } else {
	    	    cell->ant->state = world->cursor->old_ant_state;
	    }
	}
			
skip_action:
	if (forward) {
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

	v_disable_update();
	for (p.x=0; p.x<world->size_x; p.x++) {
		for (p.y=0; p.y<world->size_y; p.y++) {
			cell = cell_at (world, &p);
			
			need_display = 0;
			if (cell->ant_needs_update) {
				if (cell->ant) {
					v_set_ant (cell->ant);
				} else {
					v_remove_ant (&p, cell);
				}
				cell->ant_needs_update = 0;		
				need_display = 1;
			}
			if (cell->food_needs_update) {
				v_set_food (&p, cell);
				cell->food_needs_update = 0;
				need_display = 1;
			}
			if (cell->markers_need_update) {
				for (team=0; team<NUM_COLONIES; team++) {	
					v_set_marker (&p, cell->markers);
				}
				cell->markers_need_update = 0;
				need_display = 1;
			}
			if (need_display) 
				v_display_cell (&p);
		}
	}
	v_enable_update();
	return 0;
}

static inline
int step_forward (struct world *world, int display)
{
	return execute_step (world, display, 1);
}

static inline
int step_backward (struct world *world, int display)
{
	return execute_step (world, display, 0);
}

/* A round forward. If display is non-zero the display is updated. */
int round_forward (struct world *world, int display)
{
	int ret; 

	if ((ret=skip_round_markers (world, 1)) != 0) 
	    return ret;
	while (world->cursor && world->cursor->ant) {
		if ((ret=step_forward (world, display)) != 0) 
		    return ret;
	}
	return 0;
}
		

/* A round backward. If display is non-zero the display is updated. */
int round_backward (struct world *world, int display)
{
	int ret; 

    	step_backward (world, display);
	if ((ret=skip_round_markers (world, 0)) != 0) 
	    return ret;
	while (world->cursor && world->cursor->next && 
	    world->cursor->next->ant) {
		if ((ret=step_backward (world, display)) != 0) 
		    return ret;
	}
	return 0;
}

int n_rounds (struct world *world, int delta)
{
	int i; 
	int display_incrementally;
	int ret, ret2;

	if (delta*delta < 100) {
		display_incrementally = 1; 
	} else {
		display_incrementally = 0;
	}

	if (delta < 0) {
		for (i=0; i>delta; i--) {
 			if ((ret = round_backward (world, display_incrementally)) != 0) 
			    break;
		}
	} else {
		for (i=0; i<delta; i++) {
			if ((ret = round_forward (world, display_incrementally)) != 0) 
			    break;
		}
	}

	if (!display_incrementally) {  
		if ((ret2=update_everything (world))!=0) 
		    return ret2;
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




