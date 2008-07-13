#include "alex.h"
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>

SDL_Surface *screen;
TTF_Font *font;

struct rect {
	struct rect *next;
	double x, y, h, w;
	double speed, theta, moveto_x, moveto_y;
	double lasttime, timelimit;
	double top, bottom, left, right, middle_x, middle_y;
	Uint32 color;
};

int alexsdl_init (int width, int height, Uint32 flags);

int alexttf_init (char *setfont, double fontsize);

void draw_text (char *string, TTF_Font *font, double x, double y, Uint32 color);

void rect_def (struct rect *rect, double x, double y, double h, double w, Uint32 color);
