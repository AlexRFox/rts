#include "alexsdl.h"

enum {
	WIDTH = 640,
	HEIGHT = 480,
};

enum {
	SPEED = 100,
};

int mousebutton[10];
double mouse_x, mouse_y;

struct unit {
	struct unit *next;
	double x, y, h, w, center_x, center_y;
	double top, bottom, left, right;
	double vel_x, vel_y, moveto_x, moveto_y, moving;
	double lasttime;
	Uint32 color;
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

	for (units = 1; units <= 3; units++) {
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
		up->moveto_x = up->center_x;
		up->moveto_y = up->center_y;
		up->lasttime = get_secs();
		up->moving = 0;
		unit_x += 100;
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
	double now;
	
	if (mousebutton[3]) {
		if (selection) {
			now = get_secs();
			
			selection->moveto_x = mouse_x;
			selection->moveto_y = mouse_y;
			selection->lasttime = now;
			selection->moving = 1;
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
		if (up1->right + up1->vel_x < up2->left) {
			return (0);
		}
		if (up1->left + up1->vel_x > up2->right) {
			return (0);
		}
		if ((up1->top >= up2->top
		     && up1->top <= up2->bottom)
		    || (up1->bottom >= up2->top
			&& up1->bottom <= up2->bottom)) {
			return (1);
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
		if (up1->top + up1->vel_y > up2->bottom) {
			return (0);
		}
		if (up1->bottom + up1->vel_y < up2->top) {
			return (0);
		}
		if ((up1->left >= up2->left
		     && up1->left <= up2->right)
		    || (up1->right >= up2->left
			&& up1->right <= up2->right)) {
			return (1);
		}
	}

	return (0);
}

void
moving (void)
{
	double now, dt, dx, dy, theta;
	struct unit *up;

	for (up = first_unit; up; up = up->next) {
		now = get_secs ();

		if (up->moving) {
			dx = up->moveto_x - up->center_x;
			dy = up->moveto_y - up->center_y;
			theta = atan2 (dy, dx);
			
			dt = now - up->lasttime;
			
			up->vel_x = SPEED * dt * cos (theta);
			up->vel_y = SPEED * dt * sin (theta);
			
			if (fabs (up->vel_x) > fabs (dx)
			    && fabs (up->vel_y) > fabs (dy)) {
				up->x = up->moveto_x - up->w / 2;
				up->y = up->moveto_y - up->h / 2;
				up->moving = 0;
			} else {
				if (collision_x (up)) {
					up->vel_x = 0;
				}
				up->x += up->vel_x;
				if (collision_y (up)) {
					up->vel_y = 0;
				}
				up->y += up->vel_y;
			}
			
			up->lasttime = now;
			unit_def (up);
		}
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
