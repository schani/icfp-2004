/** This is an event-based interface to the visualizer. Each event causes
    the screen to be updated. Note that the visualizer needs not contain
    state about the world. The visualizer gets all information it needs
    on every call (bertl, just tell me if you need something else), so
    there's no need to call back for more information. 

    The visualizer must not change any fields in structs passed to it. 

    All functions should return non-zero on error, zero on success. 
  */

#ifndef _VISUALIZER_H
#define _VISUALIZER_H

#include "ants_def.h"

enum events {
	EVENT_NONE = 0, 	/* No event happened. */
	EVENT_QUIT,     	/* Quit visualizer. */
	EVENT_REDISPLAY, 	/* Redisplay everything (for example, on scrolling) */

	EVENT_SHOW_ANT_STATE, 	/* A cell was clicked on with left button. */
	EVENT_UPDATE,           /* Mouse moved, display cell info. */

	EVENT_ANIMATE_FASTER = 16,   /* Do what they say .. */
	EVENT_ANIMATE_FASTEST,  /* No sleeping. */
	EVENT_ANIMATE_SLOWER, 
	EVENT_STOP_ANIMATION,   
	EVENT_START_ANIMATION,   
	EVENT_TOGGLE_ANIMATION, 

	EVENT_STEP_FORWARD = 64,    /* Navigation. All of these stop animation. */
	EVENT_STEP_BACKWARD,
	EVENT_ROUND_FORWARD,
	EVENT_ROUND_BACKWARD,
	EVENT_10_ROUNDS_FORWARD,
	EVENT_10_ROUNDS_BACKWARD,
	EVENT_100_ROUNDS_FORWARD,
	EVENT_100_ROUNDS_BACKWARD,
	EVENT_1000_ROUNDS_FORWARD,
	EVENT_1000_ROUNDS_BACKWARD,
	EVENT_10000_ROUNDS_FORWARD,
	EVENT_10000_ROUNDS_BACKWARD,

	EVENT_LAST
};
	

/** Initializes the visualization driver. */

extern int v_initialize (void);

/** Map is encoded like in input files except that there are no spaces and
    the sizes are ints. This is also the initialization
    function and is called only once.
  
    Also resets all ants, food and markers to zero, i.e. there are 
    not ants, food and markers. 
  */

extern void v_disable_update (void);
extern void v_enable_update (void);


extern int v_load_map (int sizex, int sizey, char *map);

/** Dumps one cell to screen. To be used after display_xxx methods for
    individual cells. */
extern int v_display_cell (struct position *pos);

/** Called when all the above are finished to 'stroke' the world to screen. 
  */
extern int v_redisplay_everything (void);


/** Called whenever ant moves. ant contains old position, to is new
    position. If ant just changes direction, to is equal to pos in ant. 
  */
extern int v_move_ant (struct ant *ant, struct cell *cell, struct position *old);

extern int v_turn_ant (struct ant *ant, int old_direction);

/** Called when ant picks up food. Position is in struct ant. */
extern int v_ant_picks_up_food (struct ant *ant, struct cell *cell);

/** Called when ant picks up food. Position is in struct ant. */
extern int v_ant_drops_food (struct ant *ant, struct cell *cell);

/** Ant dies! It becomes food afterwards. */
extern int v_ant_is_killed (struct ant *ant, struct cell *cell);

/** Ant is resurrected! It requires food. */
extern int v_ant_is_resurrected (struct ant *ant, struct cell *cell);

/** Marker changes. Old marker and new marker are encoded bitwise. Normally, 
    only one bit should change.  
?? review this for schani's suggestion. 
  */ 
extern int v_ant_changes_marker (struct ant *ant, struct cell *cell, int *old_markers);

/** NXRunAltertPanel ... */
extern int v_display_message (const char *msg);

/** Pass a comment line to the output driver. Comments may, for example, 
    define color maps for displaying different marker values. The 
    vizualiation driver does not touch these */

extern int v_comment (const char *comment);

/** Displays a status line. For example, step number and round number. */
extern int v_display_status_line (const char *line);

/** This is called when reading of world trace has been finished, so 
    that one can navigate through the trace. 

?? this function is obsolete.. 
*/

extern int v_run_main_loop (void);

/** Poll an event and return that event. If there is no event within timeout, 
    returns immeadiately with event EVENT_NONE. If timeout is NULL, wait
    forever until there is an event. 
  */
extern enum events v_poll_event (int timeout);

/** Used to tell some command line args. */
extern int v_set_geometry (int x, int y, int h);

/** Used to query the mouse pointer. */
extern int v_query_pointer (struct position *pos);


/** Called at the beginning and whenever amount of food changes. */
extern int v_set_food (struct position *pos, struct cell *cell);

/** Called at the beginning. Id is just for distinguishing ants and 
    may be ignored. 
  */
extern int v_set_ant (struct ant *ant);

/** Called when several steps are executed without updating each
    one individually.
  */
extern int v_remove_ant (struct position *pos, struct cell *cell);

/** Called at the beginning to set any markers. This is used when a state
    is restored from a file. */
extern int v_set_marker (struct position *pos, int *markers);

#endif /* _VISUALIZER_H */
