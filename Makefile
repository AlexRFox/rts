CFLAGS = -g -Wall `sdl-config --cflags`
LIBS = -g -Wall `sdl-config --libs` -lm -lSDL_image -lSDL_gfx -lSDL_ttf

rts: rts.o alexsdl.o alex.o
	$(CC) $(CFLAGS) -o rts rts.o alexsdl.o alex.o $(LIBS)

baserts: baserts.o alexsdl.o alex.o
	$(CC) $(CFLAGS) -o baserts baserts.o alexsdl.o alex.o $(LIBS)

loadmap: loadmap.o alexsdl.o alex.o
	$(CC) $(CFLAGS) -o loadmap loadmap.o alexsdl.o alex.o $(LIBS)

createmap: createmap.o alexsdl.o alex.o
	$(CC) $(CFLAGS) -o createmap createmap.o alexsdl.o alex.o $(LIBS)

fightingrts: fightingrts.o alexsdl.o alex.o
	$(CC) $(CFLAGS) -o fightingrts fightingrts.o alexsdl.o alex.o $(LIBS)

clean:
	rm *~
	rm *.o
