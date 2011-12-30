//#include "alexsdl.h"
#include "alex.h"

#define WIDTH 640
#define HEIGHT 480
#define SPEED 300
#define PATH_RESO 25

enum bool {
	FALSE,
	TRUE,
};

enum state {
	MOVEMENT,
	PATHING,
	PICK_GOAL,
};

enum dir {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};

enum state mode;
int mousebutton[10];
double mouse_x, mouse_y, path_x, path_y;

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
	struct node *loc;
	struct path *path;
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
	Uint32 color;
};

struct pathblock *first_pathblock, *last_pathblock;

struct node {
	struct rect r;
	int occupied, cost, extended;
	int block_x, block_y;
};

struct node *map[(int)(WIDTH/PATH_RESO)+1][(int)(HEIGHT/PATH_RESO)+1], *goal;

struct path {
	struct path *path_next, *path_prev;
	struct nodelink *links_head, *links_tail;
};

struct nodelink {
	struct nodelink *next;
	struct node *np;
	int cost;
	double heur;
};

void rect_def (struct rect *r1);
void unit_def (struct unit *up, struct pathblock *pp);
void init_pathblock (void);
void init_selectbox (void);
void init_destimg (void);
void init_nodes (void);
void run_inits (void);
void check_direction (struct rect *sp);
void select_check (double left, double right, double top, double bottom);
void pathing (void);
void selecting (void);
void destination (void);
int detect_intersect (struct rect *r1, struct rect *r2);
int illegal_move (struct rect *r1, struct rect *r2);
void moving (void);
void draw (void);
void process_input (void);

void
rect_def (struct rect *r1)
{
	r1->top = r1->y1;
	r1->bottom = r1->y1 + r1->h;
	r1->left = r1->x1;
	r1->right = r1->x1 + r1->w;
	r1->x2 = r1->right;
	r1->y2 = r1->bottom;
	r1->center_x = r1->x1 + r1->w / 2;
	r1->center_y = r1->y1 + r1->h / 2;
}

void
unit_def (struct unit *up, struct pathblock *pp)
{
	if (up) {
		rect_def (&up->pos);
	}
	if (pp) {
		rect_def (&pp->pos);
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

		pp->pos.x1 = pathblock_x;
		pp->pos.y1 = pathblock_y;
		pp->pos.h = 50;
		pp->pos.w = 50;
		pp->color = 0x777777ff;
		unit_def (NULL, pp);
	}
}

void
init_units (void)
{
	int i, j;
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
		up->pos.h = 15;
		up->pos.w = 15;
		up->color = 0x00ff00ff;
		unit_def (up, NULL);
		up->moveto_x = up->pos.center_x;
		up->moveto_y = up->pos.center_y;
		up->lasttime = get_secs();
		up->moving = 0;
		unit_x += 200;

		for (i = 0; i <= (int) (WIDTH/PATH_RESO); i++) {
			for (j = 0; j <= (int) (HEIGHT/PATH_RESO); j++) {
				if (i*PATH_RESO <= up->pos.center_x
				    && (i+1)*PATH_RESO > up->pos.center_x
				    && j*PATH_RESO <= up->pos.center_y
				    && (j+1)*PATH_RESO > up->pos.center_y)
					up->loc = map[i][j];
			}
		}
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
init_nodes (void)
{
	struct pathblock *pp;
	int i, j;

	for (i = 0; i <= WIDTH / 25; i++) {
		for (j = 0; j <= HEIGHT / 25; j++) {
			struct node *np;

			np = xcalloc (1, sizeof *np);

			map[i][j] = np;

			np->occupied = 0;
			np->r.x1 = i * 25;
			np->r.y1 = j * 25;
			np->r.h = 25;
			np->r.w = 25;
			np->block_x = i;
			np->block_y = j;
			
			rect_def (&np->r);

			for (pp = first_pathblock; pp; pp = pp->next) {
				if (detect_intersect (&np->r, &pp->pos)) {
					np->occupied = 1;
				}
			}
		}
	}
}

void
run_inits (void)
{
	init_pathblock ();
	init_nodes ();
	init_units ();
	init_selectbox ();
	init_destimg ();
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

double
get_heur (struct node *np1, struct node *np2)
{
	struct vect v;

	v.x = np1->r.center_x - np2->r.center_x;
	v.y = np1->r.center_y - np2->r.center_y;

	return (hypot (v.x, v.y));
}

struct nodelink *
ext_node (struct nodelink *lp1, enum dir d)
{
	struct node *np;
	struct nodelink *lp2;

	np = NULL;

	switch (d) {
	case UP:
		if (lp1->np->r.top >= PATH_RESO)
			np = map[lp1->np->block_x][lp1->np->block_y-1];
		break;
	case DOWN:
		if (lp1->np->r.bottom + PATH_RESO <= HEIGHT)
			np = map[lp1->np->block_x][lp1->np->block_y+1];
		break;
	case LEFT:
		if (lp1->np->r.left >= PATH_RESO)
			np = map[lp1->np->block_x-1][lp1->np->block_y];
		break;
	case RIGHT:
		if (lp1->np->r.right + PATH_RESO <= WIDTH)
			np = map[lp1->np->block_x+1][lp1->np->block_y];
		break;
	}

	if (np != NULL && np->occupied != 1 && np->extended == 0) {
		np->extended = 1;

		lp2 = xcalloc (1, sizeof *lp2);
		lp2->np = np;
		lp2->cost = lp1->cost + 1;
		lp2->heur = get_heur (lp2->np, goal);

		return (lp2);
	}

	return (NULL);
}

struct path *
clone_path (struct path *pp1)
{
	struct nodelink *lp1, *lp2;
	struct path *pp2;

	pp2 = xcalloc (1, sizeof *pp2);

	for (lp1 = pp1->links_head; lp1; lp1 = lp1->next) {
		lp2 = xcalloc (1, sizeof *lp2);
		lp2->np = lp1->np;
		lp2->cost = lp1->cost;
		lp2->heur = lp2->heur;
		
		if (pp2->links_head == NULL) {
			pp2->links_head = lp2;
			pp2->links_tail = lp2;
		} else {
			pp2->links_tail->next = lp2;
			pp2->links_tail = lp2;
		}
	}

	return (pp2);
}

void
pathing (void)
{
	int i, j;
	double best_val;
	struct unit *up;
	struct path *path, queue, *best;
	struct nodelink *lp;

	for (i = 0; i <= (int) (WIDTH/PATH_RESO); i++) {
		for (j = 0; j <= (int) (HEIGHT/PATH_RESO); j++) {
			map[i][j]->extended = 0;
		}
	}

	queue.path_next = &queue;
	queue.path_prev = &queue;

	up = first_unit;

	lp = xcalloc (1, sizeof *lp);
	lp->np = up->loc;
	lp->cost = 0;
	lp->heur = get_heur (lp->np, goal);

	path = xcalloc (1, sizeof *path);
	path->links_head = lp;
	path->links_tail = lp;

	path->path_next = &queue;
	path->path_prev = queue.path_prev;
	queue.path_prev->path_next = path;
	queue.path_prev = path;

	best_val = INFINITY;
	while (queue.path_next != &queue) {
		best = queue.path_next;
		for (path = queue.path_next; path != &queue; path = path->path_next) {
			if (path->links_tail->cost + path->links_tail->heur
			    <= best_val) {
				best_val = path->links_tail->cost
					+ path->links_tail->heur;
				best = path;
			}
		}

		best->path_prev->path_next = best->path_next;
		best->path_next->path_prev = best->path_prev;
		best->path_next = NULL;
		best->path_prev = NULL;

		if (best->links_tail->np == goal) {
			up->path = best;
			mode = MOVEMENT;
			return;
		}

		if ((lp = ext_node (best->links_tail, UP)) != NULL) {
			path = clone_path (best);
			path->links_tail->next = lp;
			path->links_tail = lp;

			path->path_prev = &queue;
			path->path_next = queue.path_next;
			queue.path_next->path_prev = path;
			queue.path_next = path;
		}

		if ((lp = ext_node (best->links_tail, DOWN)) != NULL) {
			path = clone_path (best);
			path->links_tail->next = lp;
			path->links_tail = lp;

			path->path_prev = &queue;
			path->path_next = queue.path_next;
			queue.path_next->path_prev = path;
			queue.path_next = path;
		}

		if ((lp = ext_node (best->links_tail, LEFT)) != NULL) {
			path = clone_path (best);
			path->links_tail->next = lp;
			path->links_tail = lp;

			path->path_prev = &queue;
			path->path_next = queue.path_next;
			queue.path_next->path_prev = path;
			queue.path_next = path;
		}

		if ((lp = ext_node (best->links_tail, RIGHT)) != NULL) {
			path = clone_path (best);
			path->links_tail->next = lp;
			path->links_tail = lp;

			path->path_prev = &queue;
			path->path_next = queue.path_next;
			queue.path_next->path_prev = path;
			queue.path_next = path;
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

	if (r1->right <= r2->left
	    || r1->left >= r2->right
	    || r1->bottom <= r2->top
	    || r1->top >= r2->bottom) {
		collide = 0;
	} else {
		collide = 1;
	}
		
	return (collide);
}

int
illegal_move (struct rect *r1, struct rect *r2)
{
	int collide;

	collide = 0;

	if (r1->left <= 0 || r1->right >= WIDTH) {
		collide = 1;
	}

	collide = detect_intersect (r1, r2);

	return (collide);
}

void
moving (void)
{
	double now, dt, dx, dy, theta;
	struct unit *up1, *up2;
	struct rect newpos;
	struct pathblock *pp;

	for (up1 = first_unit; up1; up1 = up1->next) {
		now = get_secs ();

		if (up1->moving) {
			dx = up1->moveto_x - up1->pos.center_x;
			dy = up1->moveto_y - up1->pos.center_y;
			theta = atan2 (dy, dx);
			
			dt = now - up1->lasttime;
			
			up1->mov_x = SPEED * dt * cos (theta);
			up1->mov_y = SPEED * dt * sin (theta);
			
			if (fabs (up1->mov_x) > fabs (dx)
			    && fabs (up1->mov_y) > fabs (dy)) {
				up1->pos.x1 = up1->moveto_x - up1->pos.w / 2;
				up1->pos.y1 = up1->moveto_y - up1->pos.h / 2;
				up1->moving = 0;
			} else {
				newpos.left = up1->pos.left + up1->mov_x;
				newpos.right = up1->pos.right + up1->mov_x;
				newpos.top = up1->pos.top;
				newpos.bottom = up1->pos.bottom;

				for (up2 = first_unit; up2; up2 = up2->next) {
					if (up2 == up1) {
						continue;
					}

					if (illegal_move (&newpos, &up2->pos)) {
						up1->mov_x = 0;
					}
				}

				for (pp = first_pathblock; pp; pp = pp->next) {
					if (illegal_move (&newpos, &pp->pos)) {
						up1->mov_x = 0;
					}
				}

				up1->pos.x1 += up1->mov_x;

				newpos.left = up1->pos.left;
				newpos.right = up1->pos.right;
				newpos.top = up1->pos.top + up1->mov_y;
				newpos.bottom = up1->pos.bottom + up1->mov_y;

				for (up2 = first_unit; up2; up2 = up2->next) {
					if (up2 == up1) {
						continue;
					}

					if (illegal_move (&newpos, &up2->pos)) {
						up1->mov_y = 0;
					}
				}

				for (pp = first_pathblock; pp; pp = pp->next) {
					if (illegal_move (&newpos, &pp->pos)) {
						up1->mov_y = 0;
					}
				}

				up1->pos.y1 += up1->mov_y;
			}
			
			up1->lasttime = now;
			unit_def (up1, NULL);
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

	circleColor (screen, path_x, path_y, 3, 0xff0000ff);
	aacircleColor (screen, path_x, path_y, 3, 0xff0000ff);

	for (i = 0; i <= (int) (WIDTH/PATH_RESO); i++) {
		for (j = 0; j <= (int) (HEIGHT/PATH_RESO); j++) {
			rectangleColor (screen,
					map[i][j]->r.left, map[i][j]->r.top,
					map[i][j]->r.right, map[i][j]->r.bottom,
					0x66666666);
			if (map[i][j]->occupied) {
				boxColor (screen,
					  map[i][j]->r.left, map[i][j]->r.top,
					  map[i][j]->r.right, map[i][j]->r.bottom,
					  0x66666666);
			}				

			up = xcalloc (1, sizeof *up);
			up = first_unit;
			if (i*PATH_RESO <= up->pos.center_x
			    && (i+1)*PATH_RESO > up->pos.center_x
			    && j*PATH_RESO <= up->pos.center_y
			    && (j+1)*PATH_RESO > up->pos.center_y) {
				up->loc = map[i][j];
				boxColor (screen, up->loc->r.left, up->loc->r.top,
					  up->loc->r.right, up->loc->r.bottom, 0xffff00ff);
			}
		}
	}

	if (goal != NULL)
		boxColor (screen, goal->r.left, goal->r.top, goal->r.right,
			  goal->r.bottom, 0xff00ffff);

	struct nodelink *lp;
	struct rect *rp;
	if (first_unit->path != NULL) {
		for (lp = first_unit->path->links_head; lp; lp = lp->next) {
			rp = &lp->np->r;
			boxColor (screen, rp->left, rp->top, rp->right, rp->bottom,
				  0x0000ff88);
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
		boxColor (screen, pp->pos.left, pp->pos.top, pp->pos.right, pp->pos.bottom,
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
	int i, j;
	SDL_Event event;
	int key;

	while (SDL_PollEvent (&event)) {
		key = event.key.keysym.sym;
		switch (event.type) {
		case SDL_QUIT:
			exit (0);
		case SDL_KEYDOWN:
			if (key == 'p') {
				mode = PICK_GOAL;
			}
			if (key == 'm') {
				mode = MOVEMENT;
			}
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

			if (mode == PICK_GOAL) {
				mode = PATHING;
				path_x = mouse_x;
				path_y = mouse_y;

				for (i = 0; i <= (int) (WIDTH/PATH_RESO); i++) {
					for (j = 0; j <= (int) (HEIGHT/PATH_RESO);
					     j++) {
						if (i*PATH_RESO <= path_x
						    && (i+1)*PATH_RESO > path_x
						    && j*PATH_RESO <= path_y
						    && (j+1)*PATH_RESO > path_y) {
							goal = map[i][j];
						}
					}
				}
			}

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

	mode = MOVEMENT;

	run_inits ();

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, 0x000000);
		switch (mode) {
		case MOVEMENT:
			selecting ();
			break;
		case PATHING:
			pathing ();
			break;
		case PICK_GOAL:
			break;
		}
		moving ();
		draw ();
		SDL_Flip (screen);
	}

	return (0);
}
