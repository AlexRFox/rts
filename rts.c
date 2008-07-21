#include "alexsdl.h"

enum {
	WIDTH = 640,
	HEIGHT = 480,
};

enum {
	RIGHT = 1,
	LEFT = 2,

	TOP = 3,
	BOTTOM = 4,
};

int mousebutton[10];
double mouse_x, mouse_y;

struct unit {
	struct unit *next;
	double x, y, h, w, center_x, center_y;
	double top, bottom, left, right;
	double speed, theta, moveto_x, moveto_y;
	double lasttime, timelimit;
	Uint32 color;
	int side_hit_h, side_hit_v;
};

struct unit *selection, *first_unit, *last_unit;

void
unit_def (struct unit *up)
{
	up->top = up->y;
	up->bottom = up->y + up->h;
	up->left = up->x;
	up->right = up->x + up->w;
	up->center_x = up->x + up->w / 2;
	up->center_y = up->y + up->h / 2;
}

void
init_units (void)
{
	double units, unit_x, unit_y;

	unit_x = 200;
	unit_y = 240;

	for (units = 1; units <= 2; units++) {
		struct unit *up;
		up = xcalloc (1, sizeof *up);

		if (first_unit == NULL) {
			first_unit = up;
		} else {
			last_unit->next = up;
		}

		last_unit = up;

		up->x = unit_x;
		up->y = unit_y;
		up->h = 20;
		up->w = 40;
		up->color = 0x00ff00ff;
		unit_def (up);
		unit_x += 200;
	}
}

void
selecting (void)
{
	struct unit *up;

	if (mousebutton[1]) {
		for (up = first_unit; up; up = up->next) {
			if (mouse_x >= up->left && mouse_x <= up->right
			    && mouse_y >= up->top && mouse_y <= up->bottom) {
				selection = up;
			}
		}
	}
}

void
destination (void)
{
	double dx, dy, now;

	if (mousebutton[3]) {
		if (selection) {
			now = get_secs();
			
			selection->moveto_x = mouse_x;
			selection->moveto_y = mouse_y;
			selection->speed = 100;
			dx = selection->moveto_x - selection->center_x;
			dy = selection->moveto_y - selection->center_y;
			selection->theta = atan2 (dy, dx);
			selection->timelimit = now
				+ hypot (dy, dx) / selection->speed;
			selection->lasttime = now;
		}
	}
}

int
collision_x (struct unit *up1)
{
	struct unit *up2;

	for (up2 = first_unit; up2; up2 = up2->next) {
		if (up2 == up1) {
			continue;
		}
		if (up1->right >= up2->left && up1->right <= up2->right) {
			if ((up1->top >= up2->top && up1->top <= up2->bottom)
			    || (up1->bottom >= up2->top && up1->bottom <= up2->bottom)) {
				up1->side_hit_h = RIGHT;
				return (up2->left);
			}
		}
		if (up1->left <= up2->right && up1->left >= up2->left) {
			if ((up1->top >= up2->top && up1->top <= up2->bottom)
			    || (up1->bottom >= up2->top && up1->bottom <= up2->bottom)) {
				up1->side_hit_h = LEFT;
				return (up2->right);
			}
		}
	}

	return (0);
}

int
collision_y (struct unit *up1)
{
	struct unit *up2;

	for (up2 = first_unit; up2; up2 = up2->next) {
		if (up2 == up1) {
			continue;
		}
		if (up1->top <= up2->bottom && up1->top >= up2->top) {
			if ((up1->right >= up2->left && up1->right <= up2->right)
			    || (up1->left >= up2->left && up1->left <= up2->right)) {
				up1->side_hit_v = TOP;
				return (up2->bottom);
			}
		}
		if (up1->bottom >= up2->top && up1->bottom <= up2->bottom) {
			if ((up1->right >= up2->left && up1->right <= up2->right)
			    || (up1->left >= up2->left && up1->left <= up2->right)) {
				up1->side_hit_v = BOTTOM;
				return (up2->top);
			}
		}
	}

	return (0);
}

void
moving (void)
{
	double now, ct, cx, cy, dx, dy, unit_hit_x, unit_hit_y;
	struct unit *up;

	for (up = first_unit; up; up = up->next) {
		now = get_secs ();

		if (now > up->timelimit) {
			up->speed = 0;
		}
		if (up->speed) {
			dx = up->moveto_x - up->center_x;
			dy = up->moveto_y - up->center_y;
			up->theta = atan2 (dy, dx);
			
			ct = now - up->lasttime;
			
			cx = up->speed * ct * cos (up->theta);
			cy = up->speed * ct * sin (up->theta);

			unit_hit_x = collision_x (up);

			switch (up->side_hit_h) {
			case RIGHT:
				up->x = unit_hit_x - up->w - 1;
				cx = 0;
				break;
			case LEFT:
				up->x = unit_hit_x;
				cx = 0;
				break;
			default:
				break;
			}
			
			unit_hit_y = collision_y (up);
/*
			switch (up->side_hit_v) {
			case TOP:
				up->y = unit_hit_y + 1;
				cy = 0;
				break;
			case BOTTOM:
				up->y = unit_hit_y - up->h - 1;
				cy = 0;
				break;
			default:
				break;
			}
*/
			up->x += cx;
			up->y += cy;
			
			up->lasttime = now;
			unit_def (up);
		}

		unit_def (up);
	}
}

void
draw (void)
{
	struct unit *up;
	for (up = first_unit; up; up = up->next) {
		boxColor (screen, up->left, up->top, up->right, up->bottom,
			  up->color);
		if (selection == up) {
			rectangleColor (screen, up->x - 3, up->y - 3,
					up->x + up->w + 3, up->y + up->h +3,
					up->color);
		}
	}
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
			destination ();
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

	init_units ();

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, 0x000000);
		selecting ();
		moving ();
		draw ();
		SDL_Flip (screen);
	}

	return (0);
}
