CFLAGS = -g -Wall `sdl-config --cflags`
LIBS = -g -Wall `sdl-config --libs` -lm -lSDL_image -lSDL_gfx -lSDL_ttf

rts: rts.o alexsdl.o alex.o
	$(CC) $(CFLAGS) -o rts rts.o alexsdl.o alex.o $(LIBS)
clean:
	rm *~
	rm *.o
