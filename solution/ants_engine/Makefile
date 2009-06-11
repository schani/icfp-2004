SDL_DRIVER_DIR = ../../Bertl/SDL

OBJS = ants.o utilities.o input_world.o input_world_dump.o parse_world_trace.o steps.o
TXT_OBJS = output_world_ascii.o

SDL_OBJS = $(SDL_DRIVER_DIR)/sdl.o $(SDL_DRIVER_DIR)/SDL_gfxPrimitives.o $(SDL_DRIVER_DIR)/sim.o 

SDL_OBJS_JO = $(SDL_DRIVER_DIR)/sdl-noflip.o $(SDL_DRIVER_DIR)/SDL_gfxPrimitives.o $(SDL_DRIVER_DIR)/sim-noflip.o 

BERTL_TEST_OBJS = bertl_test.o

CFLAGS += -I../include -Wall -g -I. -I/usr/include/SDL -I$(SDL_DRIVER_DIR) 
LDFLAGS += -lSDL

all: ants_ascii ants_sdl ants_sdl_jo bertl_test

ants_ascii: $(OBJS) $(TXT_OBJS)
	gcc $(LDFLAGS) $(OBJS) $(TXT_OBJS) -o ants_ascii

ants_sdl_jo: $(OBJS) $(SDL_OBJS_JO)
	gcc $(LDFLAGS) $(OBJS) $(SDL_OBJS_JO) -o ants_sdl_jo

ants_sdl: $(OBJS) $(SDL_OBJS)
	gcc $(LDFLAGS) $(OBJS) $(SDL_OBJS) -o ants_sdl

bertl_test: $(BERTL_TEST_OBJS) $(SDL_OBJS)
	gcc $(LDFLAGS) $(BERTL_TEST_OBJS) $(SDL_OBJS) -o bertl_test

clean:
	rm -f $(OBJS) $(TXT_OBJS) $(SDL_OBJS) $(SDL_OBJS_JO)
	rm -f ants_ascii ants_sdl ants_sdl_jo
