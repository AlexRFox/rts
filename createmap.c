#include "alexsdl.h"

enum {
	WIDTH = 640,
	HEIGHT = 480,
};

int mousebutton[10];
double mouse_x, mouse_y;

struct pathblock {
	struct pathblock *next;
	double x, y, h, w;
	double top, bottom, left, right;
	double canplace;
	Uint32 color;
};

struct pathblock *first_pathblock, *last_pathblock, placeblock, unit;

void
unit_def (struct pathblock *pp)
{
	pp->top = pp->y;
	pp->bottom = pp->y + pp->h;
	pp->left = pp->x;
	pp->right = pp->x + pp->w;
}

void
init_stuff (void)
{
	placeblock.h = 20;
	placeblock.w = 20;

	unit.w = 40;
	unit.h = 20;
	unit.x = WIDTH / 2 - unit.w / 2;
	unit.y = HEIGHT / 2 - unit.h / 2;
	unit.color = 0x00ff00ff;
	unit_def (&unit);
}

void
place_pathblock (void)
{
	struct pathblock *pp;
	pp = xcalloc (1, sizeof *pp);

	if (placeblock.canplace) {
		if (first_pathblock == NULL) {
			first_pathblock = pp;
		} else {
			last_pathblock->next = pp;
		}
		
		last_pathblock = pp;
		
		pp->w = 20;
		pp->h = 20;
		pp->x = mouse_x - pp->w / 2;
		pp->y = mouse_y - pp->h / 2;
		pp->color = 0x777777ff;
		unit_def (pp);
	}
}

void
check_space (void)
{
	struct pathblock *pp;

	placeblock.x = mouse_x - placeblock.w / 2;
	placeblock.y = mouse_y - placeblock.h / 2;
	unit_def (&placeblock);
	placeblock.canplace = 1;
	placeblock.color = 0x00ff0077;

	if (placeblock.left > unit.right || placeblock.right < unit.left
	    || placeblock.top > unit.bottom || placeblock.bottom < unit.top) {
		placeblock.color = 0x00ff0077;
		placeblock.canplace = 1;
	} else {
		placeblock.color = 0xff000077;
		placeblock.canplace = 0;
	}

	if (placeblock.canplace) {
		for (pp = first_pathblock; pp; pp = pp->next) {
			if (placeblock.left > pp->right
			    || placeblock.right < pp->left
			    || placeblock.top > pp->bottom
			    || placeblock.bottom < pp->top) {
				placeblock.color = 0x00ff0077;
				placeblock.canplace = 1;
			} else {
				placeblock.color = 0xff000077;
				placeblock.canplace = 0;
				return;
			}
		}
	}
}

void
draw (void)
{
	struct pathblock *pp;

	boxColor (screen, unit.x, unit.y, unit.x + unit.w,
			unit.y + unit.h, unit.color);
	for (pp = first_pathblock; pp; pp = pp->next) {
		boxColor (screen, pp->left, pp->top, pp->right, pp->bottom,
			  pp->color);
	}
	boxColor (screen, placeblock.left, placeblock.top, placeblock.right,
		  placeblock.bottom, placeblock.color);
}

void
process_input (void)
{
	SDL_Event event;
        int key;

        while (SDL_PollEvent (&event)) {
                key = event.key.keysym.sym;
                switch (event.type) {
                case SDL_QUIT:
                        exit (0);
                case SDL_KEYUP:
                        if (key == SDLK_ESCAPE || key == 'q') {
                                exit (0);
                        }
                case SDL_MOUSEBUTTONDOWN:
                        mousebutton[event.button.button] = 1;
			place_pathblock ();
                        break;
                case SDL_MOUSEBUTTONUP:
                        mousebutton[event.button.button] = 0;
                        break;
                case SDL_MOUSEMOTION:
                        mouse_x = event.button.x;
                        mouse_y = event.button.y;
                        break;
                }
        }
}

int
main (int argc, char **argv)
{
	alexsdl_init (WIDTH, HEIGHT, SDL_HWSURFACE | SDL_DOUBLEBUF);

	init_stuff ();

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, 0x000000);
		check_space ();
		draw ();
		SDL_Flip (screen);
	}

	return (0);
}
