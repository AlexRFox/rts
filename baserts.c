#include "alexsdl.h"

enum {
	WIDTH = 640,
	HEIGHT = 480,
};

enum {
	SPEED = 300,
};

int mousebutton[10];
double mouse_x, mouse_y;

struct rect {
	double x1, y1, x2, y2, h, w, center_x, center_y;
	double top, bottom, left, right;
	int drawing, direction;
	Uint32 color;
};

struct rect selectbox;

struct unit {
	struct unit *next;
	struct rect pos;
	double mov_x, mov_y, moveto_x, moveto_y, moving, selected;
	double lasttime;
	Uint32 color;
};

struct unit *first_unit, *last_unit;

struct dest {
	double x, y;
	SDL_Surface *frames[8];
	double lasttime, drawing;
};

struct dest destimg;

struct pathblock {
	struct pathblock *next;
	struct rect pos;
	double x, y, h, w;
	double top, bottom, left, right;
	Uint32 color;
};

struct pathblock *first_pathblock, *last_pathblock;

void
unit_def (struct unit *up, struct pathblock *pp)
{
	if (up) {
		up->pos.top = up->pos.y1;
		up->pos.bottom = up->pos.y1 + up->pos.h;
		up->pos.left = up->pos.x1;
		up->pos.right = up->pos.x1 + up->pos.w;
		up->pos.center_x = up->pos.x1 + up->pos.w / 2;
		up->pos.center_y = up->pos.y1 + up->pos.h / 2;
	}
	if (pp) {
		pp->top = pp->y;
		pp->bottom = pp->y + pp->h;
		pp->left = pp->x;
		pp->right = pp->x + pp->w;
		pp->pos.top = pp->y;
		pp->pos.bottom = pp->y + pp->h;
		pp->pos.left = pp->x;
		pp->pos.right = pp->x + pp->w;
	}
}

void
init_pathblock (void)
{
	double pathblocks, pathblock_x, pathblock_y;

	pathblock_x = 300;
	pathblock_y = 240;

	for (pathblocks = 0; pathblocks <= 0; pathblocks++) {
		struct pathblock *pp;
		pp = xcalloc (1, sizeof *pp);

		if (first_pathblock == NULL) {
			first_pathblock = pp;
		} else {
			last_pathblock->next = pp;
		}

		last_pathblock = pp;

		pp->x = pathblock_x;
		pp->y = pathblock_y;
		pp->h = 50;
		pp->w = 50;
		pp->color = 0x777777ff;
		unit_def (NULL, pp);
	}
}

void
init_units (void)
{
	double units, unit_x, unit_y;

	unit_x = 200;
	unit_y = 240;

	for (units = 0; units < 1; units++) {
		struct unit *up;
		up = xcalloc (1, sizeof *up);

		if (first_unit == NULL) {
			first_unit = up;
		} else {
			last_unit->next = up;
		}

		last_unit = up;

		up->pos.x1 = unit_x;
		up->pos.y1 = unit_y;
		up->pos.h = 20;
		up->pos.w = 40;
		up->color = 0x00ff00ff;
		unit_def (up, NULL);
		up->moveto_x = up->pos.center_x;
		up->moveto_y = up->pos.center_y;
		up->lasttime = get_secs();
		up->moving = 0;
		unit_x += 200;
	}
}

void
init_selectbox (void)
{
	selectbox.drawing = 0;
	selectbox.color = 0x0000ffff;
}

void
init_destimg (void)
{
	double now;

	now = get_secs ();

	load_blit (&destimg.frames[0], "destination/frame00.png");
	load_blit (&destimg.frames[1], "destination/frame01.png");
	load_blit (&destimg.frames[2], "destination/frame02.png");
	load_blit (&destimg.frames[3], "destination/frame03.png");
	load_blit (&destimg.frames[4], "destination/frame04.png");
	load_blit (&destimg.frames[5], "destination/frame05.png");
	load_blit (&destimg.frames[6], "destination/frame06.png");
	load_blit (&destimg.frames[7], "destination/frame07.png");
	destimg.lasttime = now;
}

void
run_inits (void)
{
	init_units ();
	init_selectbox ();
	init_destimg ();
	init_pathblock ();
}

void
check_direction (struct rect *sp)
{
	if (sp->x1 > sp->x2) {
		if (sp->y1 > sp->y2) {
			sp->direction = 1;
		} else {
			sp->direction = 3;
		}
	} else {
		if (sp->y1 > sp->y2) {
			sp->direction = 2;
		} else {
			sp->direction = 4;
		}
	}
}

void
select_check (double left, double right, double top, double bottom)
{
	struct unit *up;

	for (up = first_unit; up; up = up->next) {
		if (right < up->pos.left
		    || left > up->pos.right
		    || top > up->pos.bottom
		    || bottom < up->pos.top) {
			up->selected = 0;
		} else {
			up->selected = 1;
		}
	}
}

void
selecting (void)
{
	struct unit *up;

	if (mousebutton[1]) {
		if (selectbox.drawing == 0) {
			selectbox.drawing = 1;
			selectbox.x1 = mouse_x;
			selectbox.y1 = mouse_y;
			selectbox.x2 = mouse_x;
			selectbox.y2 = mouse_y;
			check_direction (&selectbox);
		} else {
			selectbox.x2 = mouse_x;
			selectbox.y2 = mouse_y;
			check_direction (&selectbox);
		}
		for (up = first_unit; up; up = up->next) {
			switch (selectbox.direction) {
			case 1:
				select_check (selectbox.x2, selectbox.x1,
					      selectbox.y2, selectbox.y1);
				break;
			case 2:
				select_check (selectbox.x1, selectbox.x2,
					      selectbox.y2, selectbox.y1);
				break;
			case 3:
				select_check (selectbox.x2, selectbox.x1,
					      selectbox.y1, selectbox.y2);
				break;
			case 4:
				select_check (selectbox.x1, selectbox.x2,
					      selectbox.y1, selectbox.y2);
				break;
			}
		}
	}
	if (mousebutton[1] == 0) {
		selectbox.drawing = 0;
	}
}

void
destination (void)
{
	double now;
	struct unit *up;
	
	now = get_secs ();

	if (mousebutton[3]) {
		for (up = first_unit; up; up = up->next) {
			if (up->selected) {
				now = get_secs ();
				
				up->moveto_x = mouse_x;
				up->moveto_y = mouse_y;
				up->lasttime = now;
				up->moving = 1;
			}
		}
		destimg.drawing = 1;
		destimg.x = mouse_x - 20;
		destimg.y = mouse_y - 20;
		destimg.lasttime = now;
	}
}

int
detect_intersect (struct rect *r1, struct rect *r2)
{
	int collide;

	collide = 0;

	if (r1->right < r2->left
	    || r1->left > r2->right
	    || r1->bottom < r2->top
	    || r1->top > r2->bottom) {
		collide = 0;
	} else {
		collide = 1;
	}
		
	return (collide);
}

int
collision_x (struct unit *up1)
{
	struct unit *up2;
	struct pathblock *pp;
	struct rect newpos;
	int collide;

	newpos.left = up1->pos.left + up1->mov_x;
	newpos.right = up1->pos.right + up1->mov_x;
	newpos.top = up1->pos.top;
	newpos.bottom = up1->pos.bottom;

	collide = 0;

	if (newpos.left <= 0 || newpos.right >= WIDTH) {
		collide = 1;
	}

	for (up2 = first_unit; up2; up2 = up2->next) {
		if (collide) {
			break;
		}

		if (up2 == up1) {
			continue;
		}

		collide = detect_intersect (&newpos, &up2->pos);
	}
	for (pp = first_pathblock; pp; pp = pp->next) {
		if (collide) {
			break;
		}

		collide = detect_intersect (&newpos, &pp->pos);
	}
	return (collide);
}

int
collision_y (struct unit *up1)
{
	struct unit *up2;
	struct pathblock *pp;
	struct rect newpos;
	double collide;

	newpos.left = up1->pos.left;
	newpos.right = up1->pos.right;
	newpos.top = up1->pos.top + up1->mov_y;
	newpos.bottom = up1->pos.bottom + up1->mov_y;

	collide = 0;

	if (newpos.top <= 0 || newpos.bottom >= HEIGHT) {
		collide = 1;
	}

	for (up2 = first_unit; up2; up2 = up2->next) {
		if (collide) {
			break;
		}

		if (up2 == up1) {
			continue;
		}

		collide = detect_intersect (&newpos, &up2->pos);
	}

	for (pp = first_pathblock; pp; pp = pp->next) {
		if (collide) {
			break;
		}

		collide = detect_intersect (&newpos, &pp->pos);
	}
	return (collide);
}

void
moving (void)
{
	double now, dt, dx, dy, theta;
	struct unit *up;

	for (up = first_unit; up; up = up->next) {
		now = get_secs ();

		if (up->moving) {
			dx = up->moveto_x - up->pos.center_x;
			dy = up->moveto_y - up->pos.center_y;
			theta = atan2 (dy, dx);
			
			dt = now - up->lasttime;
			
			up->mov_x = SPEED * dt * cos (theta);
			up->mov_y = SPEED * dt * sin (theta);
			
			if (fabs (up->mov_x) > fabs (dx)
			    && fabs (up->mov_y) > fabs (dy)) {
				up->pos.x1 = up->moveto_x - up->pos.w / 2;
				up->pos.y1 = up->moveto_y - up->pos.h / 2;
				up->moving = 0;
			} else {
				if (collision_x (up)) {
					up->mov_x = 0;
				}
				up->pos.x1 += up->mov_x;
				if (collision_y (up)) {
					up->mov_y = 0;
				}
				up->pos.y1 += up->mov_y;
			}
			
			up->lasttime = now;
			unit_def (up, NULL);
		}
	}
}

void
draw (void)
{
	int i, j;
	double now, dt;
	struct unit *up;
	struct pathblock *pp;

	for (i = 0; i <= WIDTH / 25; i++) {
		for (j = 0; j <= HEIGHT / 25; j++) {
			rectangleColor (screen, i * 25, j * 25,
					(i + 1) * 25, (j + 1) * 25,
					0x66666666);
		}
	}

	now = get_secs ();

	if (destimg.drawing) {
		dt = floor (10 * (now - destimg.lasttime));
		if (dt < 8) {
			draw_blit (destimg.frames[(int) dt], destimg.x, destimg.y);
		} else {
			destimg.drawing = 0;
		}
	}

	for (pp = first_pathblock; pp; pp = pp->next) {
		boxColor (screen, pp->left, pp->top, pp->right, pp->bottom,
			  pp->color);
	}

	for (up = first_unit; up; up = up->next) {
		boxColor (screen, up->pos.left, up->pos.top, up->pos.right, up->pos.bottom,
			  up->color);
		if (up->selected == 1) {
			circleColor (screen, up->pos.center_x, up->pos.center_y,
				     hypot (up->pos.h / 2, up->pos.w / 2) + 3,
				     0x00ff00ff);
			aacircleColor (screen, up->pos.center_x, up->pos.center_y,
				       hypot (up->pos.h / 2, up->pos.w / 2) + 3,
				       0x00ff00ff);
		}
	}

	if (selectbox.drawing) {
		rectangleColor (screen, selectbox.x1, selectbox.y1,
				selectbox.x2, selectbox.y2, selectbox.color);
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

	run_inits ();

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
