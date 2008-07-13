#include "alexsdl.h"

int mousebutton[10];
double mouse_x, mouse_y;

struct rect *selection, rect1, rect2;

void
rect_init (void)
{
	rect_def (&rect1, 200, 240, 30, 50, 0x00ff00);
	rect_def (&rect2, 400, 240, 50, 40, 0xff0000);
}

void
selecting()
{
	if (mousebutton[1]) {
		if (mouse_x >= rect1.right && mouse_x <= rect1.right
		    && mouse_y >= rect1.top && mouse_y <= rect1.bottom) {
			selection = &rect1;
		} else if (mouse_x >= rect2.right && mouse_x <= rect2.right
			   && mouse_y >= rect2.top && mouse_y <= rect2.bottom) {
			selection = &rect2;
		}
	}
}

void
destination()
{
	double dx, dy, now;

	if (mousebutton[3]) {
		if (selection) {
			now = get_secs();
			
			selection->moveto_x = mouse_x;
			selection->moveto_y = mouse_y;
			dx = selection->moveto_x - selection->middle_x;
			dy = selection->moveto_y - selection->middle_y;
			selection->theta = atan2 (dy, dx);
			selection->speed = 100;
			selection->timelimit = now
				+ hypot (dy, dx) / selection->speed;

			selection->lasttime = now;
		}
	}
}

/*int
collision (struct rect *rp1, struct rect *rp2)
{

}*/

void
moving(struct rect *rp)
{
	double now, dt, dx, dy, old_x, old_y;

	if (rp) {
		now = get_secs();
		old_x = rp->x;
		old_y = rp->y;
		if (now > rp->timelimit) {
			rp->speed = 0;
		}
		if (rp->speed) {
			
			dt = now - rp->lasttime;
			
			dx = rp->speed * dt * cos (rp->theta);
			dy = rp->speed * dt * sin (rp->theta);
			
			rp->x += dx;
			rp->y += dy;
			rp->middle_x += dx;
			rp->middle_y += dy;

			rp->lasttime = now;
		}
/*		if (touching (&c1, &c2)) {
			rp->x = old_x;
			rp->y = old_y;
			}*/
	}
}

void
draw()
{
	boxColor (screen, rect1.left, rect1.top, rect1.right, rect1.bottom,
		  rect1.color);
/*	boxColor (screen, rect2.x, rect2.y, rect2.x + rect2.w, rect2.y + rect2.h,
	rect2.color);*/
/*	if (selection == &rect1) {
		circleColor (screen, rect1.x, rect1.y, (rect1.w + rect1.h) / 2,
			     rect1.color);
	}
	if (selection == &rect2) {
		circleColor (screen, rect2.x, rect2.y, (rect2.w + rect2.h) / 2,
			     rect2.color);
			     }*/
}

void
process_input()
{
	SDL_Event event;
	int key;

	while (SDL_PollEvent(&event)) {
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
			destination();
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
main(int argc, char **argv)
{
	alexsdl_init (640, 480, SDL_HWSURFACE | SDL_DOUBLEBUF);

	rect_init ();

	while (1) {
		process_input();
		SDL_FillRect(screen, NULL, 0x000000);
/*		selecting();
		moving(&rect1);
		moving(&rect2);*/
		draw();
		SDL_Flip(screen);
	}

	return 0;
}
