
OBJS = ants.o utilities.o input_world.o input_world_dump.o parse_world_trace.o steps.o
TXT_OBJS = output_world_ascii.o

SDL_OBJS = sdl.o SDL_gfxPrimitives.o sim.o 
POOL_OBJS = pools.o
 
HFILES = ants_def.h visualizer.h

CFLAGS += -I../include -Wall -g -I. -I/usr/include/SDL 
LDFLAGS += -lSDL

# CFLAGS += -pg
# LDFLAGS += -pg

all: ants_sdl

ants_ascii: $(OBJS) $(TXT_OBJS) $(POOL_OBJS)
	gcc $(LDFLAGS) $(OBJS) $(TXT_OBJS) $(POOL_OBJS) -o ants_ascii

ants_sdl: $(OBJS) $(SDL_OBJS) $(HFILES) $(POOL_OBJS)
	gcc $(LDFLAGS) $(OBJS) $(SDL_OBJS) $(POOL_OBJS) -o ants_sdl

clean:
	rm -f *.o
	rm -f ants_ascii ants_sdl ants_sdl_jo


sim.o: sdl.h sim.h sim.c

sdl.o: sdl.h sim.h sdl.c

pools.o: pools.c pools.h

