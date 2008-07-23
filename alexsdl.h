#include "alex.h"
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>

SDL_Surface *screen;
TTF_Font *font;

int alexsdl_init (int width, int height, Uint32 flags);

int alexttf_init (char *setfont, double fontsize);

void draw_text (char *string, TTF_Font *font, double x, double y, Uint32 color);
