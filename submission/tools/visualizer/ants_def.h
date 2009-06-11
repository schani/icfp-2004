
/** 
 * Contains general definitions and data structures for the ICFP 2004's
 * contest solution of the Vienna based task force "FunktionImKopfDerMensch".
 * (Not translateable into any other language, including English, C and
 * Java).
 *
 * Also public C-Functions go in here.
 * 
 * Currently not includeable from C++ (fixme??)
 */ 

#ifndef _ANTS_DEF_H 
#define _ANTS_DEF_H


#include <stdio.h>


/* Number of ant populations/opponents. In the contest, only 2 populations
   challenge each other. 
 */
#define NUM_COLONIES 2

#define ROCK_CELL '#'
#define EMPTY_CELL '.' 

#define COLONY_HOMES "+-"

extern char *colony_home_chars; 


extern int warnings;


/* Maximum number of distinct markers that can go on a cell per 
   colony. */
#define MARKERS_PER_CELL 6 

/**
 * The definitions are in kind' a' Object-oriented C. You'll have to 
 * to specify the object explizitly as first parameter. Behaviour is
 * undefined if you manipulate the object's contents directly. 
 *
 * If not specified otherwise all functions returning int return 
 * zero on success, non-zero otherwise. 
 */



/** 
 * A position in the world.
 */


struct position {
	int x, y;
};


struct ant {
	struct position pos; 
	int direction;   /* Where the ant is heading to. 
                          0 - right, 1-down/right, 2-down/left
                          3 - left, 4-up/left, 5-up/right */
	int id;        /* unique id for this ant. Also unique between colonies */

	short state; /* state the ant is currently in. */
	char resting;  /* Moves to rest before I can move again. */
	char dead;  /* Nonzero if ant has RIP. */
	
	unsigned char colony;    /* id of colony. */
	char carries_food; /* Nonzero if ant carries food. */
};


#
/** 
 * Represents a cell in the world. A cell can be rocky, be home 
 * of an ant colony, contain markers and/or food.
 */

struct cell {
	int rocky;  /* Non-zero if the cell is impassable. */
	int home_of_colony; /* Number of colony this cell is 
                      home to. -1 if not. */ 
	int food;        /* Number of food particles on cell. */	
	struct ant *ant; /* Pointer to ant if there is one on cell. */
/* 	char markers[NUM_COLONIES][MARKERS_PER_CELL];  */
	int markers[NUM_COLONIES]; /* Markers on the cell. Non-zero if set. 
                     This is a bit field. */

	int ant_needs_update:1;
	int food_needs_update:1;
	int markers_need_update:1;
};
	
	


enum ant_action {
	MOVE_FORWARD, 
	MOVE_BACKWARD, 
	TURN_LEFT, 
	TURN_RIGHT, 
	PICKUP_FOOD, 
	DROP_FOOD, 
	SET_MARKER, 
	CLEAR_MARKER,
	KILL_ANT,
	RESURRECT_ANT,
	SENSE_HERE,
	SENSE_LEFT,
	SENSE_AHEAD,
	SENSE_RIGHT,
	NUM_ACTIONS
};


struct execution_step {
	struct execution_step *next, *prev;
	struct ant *ant;   /* The ant. If this is NULL, then this is 
			      a turn delimiter. 
                            */

	short new_ant_state; /* new state of ant, from trace file */
	short old_ant_state; /* old state of ant, computed in step_forward () */

	char action;
	char failed_action;
	char marker;
			    /* Non-zero if the action, for whatever reason failed. 
                               Could be an ant running into another ant or rock, 
                               ant dropping food and having none or ant setting
                               marker that is set already. 
                               This is computed when stepping forward and needed when 
                               stepping backward.
                            */
//    	char food;
};

		
struct world {
	int size_x, size_y;  /* Size of world. */
	struct cell *cells;  /* The cells ... */ 
	struct ant *ants;    /* The ants ... */  
	struct execution_step *world_trace;  /* The world's evolution. */
	struct execution_step *cursor;  /* The next step to execute. */

	int total_ants;      /* Total # of ants. */
	int total_food;   /* Food left. counting food carried by ants? */

	int step;         /* Current step in execution. */
	int round;        /* Current round in execution. */
};


/** Constructor for the world object. 
    Reads world from text file as specified by ICFP. */

extern int read_world (struct world ** world, FILE *f);
 
/** Reads from dump file in ICFP format (those on the webpages ...) */
int read_world_as_icfp_dump (struct world ** worldp, FILE *f, int sizex, int sizey, int num_ants);

/** (Re-)displays everything in world. This happens at the beginning or 
    when a world is restored. */
extern int display_world (struct world *world);

/** Parses evolution of world in world trace format (.wtf). This is
    documented in the Documentation directory. */
extern int parse_world_trace (struct world *world, FILE *f);

/** Frees the world trace if non-NULL. */
extern void free_world_trace (struct world *world);

/** Frees the world if non-NULL. */
extern void free_world (struct world *world);

#if 0
/** Constructor for position object. */
extern int make_position (struct position *pos, int x, int y);

/** Queries about the world. */

/** Non-zero if there is rock at postion. */
int rocky (struct world *world, struct position *pos);

/** Non-zero if there is an ant at position. */ 
int some_ant_is_at (struct world *world, struct position *pos);

/** ... */ 


#endif 

/* Stepping through the execution trace. */

/* A step forward. If display is non-zero the display is updated. */
// extern int step_forward (struct world *world, int display);

/* A step backward. If display is non-zero the display is updated. */
// extern int step_backward (struct world *world, int display);

/* A round forward. If display is non-zero the display is updated. */
extern int round_forward (struct world *world, int display);

/* A round forward. If display is non-zero the display is updated. */
extern int round_backward (struct world *world, int display);

/* Go to last position. If display is non-zero the display is updated. */
extern int goto_last (struct world *world, int display);

/* Go to first position. If display is non-zero the display is updated. */
extern int goto_first (struct world *world, int display);

/* Go n steps. n maybe negative. */
extern int n_steps (struct world *world, int delta);

/* Go n rounds. n maybe negative. */
extern int n_rounds (struct world *world, int delta);



/* Convenience functions.  */

/** Returns cell at position by reference. */
extern struct cell* cell_at (struct world *world, struct position *pos);

/** Updates position one step towards direction. */
extern int position_move (struct position *pos, int direction);

/** Returns non-zero if an ant cannot go onto pos (rock, another ant ..) */
extern int verify_ant_position (struct world *world, struct position *pos);


#ifdef __cplusplus
extern "C" {
#endif 


#ifdef __cplusplus
}
#endif 

#endif /* ANTS_DEF_H */
