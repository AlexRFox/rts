#include "alex.h"

void *
xcalloc (int a, int b)
{
        void *p;

	if ((p = calloc (a, b)) == NULL) {
                fprintf (stderr, "memory error\n");
                exit (1);
        }
        return (p);
}

double
get_secs (void)
{
        struct timeval tv;
	gettimeofday (&tv, NULL);
        return (tv.tv_sec + tv.tv_usec/1e6);
}

double
get_usecs (void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_usec);
}

double
d_to_r (double degrees)
{
        return (degrees / 360.0 * 2 * M_PI);
}

int
alexsdl_init (int width, int height, Uint32 flags)
{
        if (SDL_Init (SDL_INIT_VIDEO) != 0) {
		printf ("Unable to initialize SDL: %s\n", SDL_GetError());
		return (1);
	}

	screen = SDL_SetVideoMode (width, height, 32, flags);
	if (screen == NULL) {
		printf ("Unable to set video mode: %s\n", SDL_GetError());
		return (1);
	}
	return 0;
}

int
alexttf_init (char *setfont, double fontsize)
{
	TTF_Init ();
	font = TTF_OpenFont (setfont, fontsize);

	if (font == NULL) {
		printf ("Unable to locate or set font");
		return (1);
	}
	return 0;
}

void
draw_text (char *string, TTF_Font *font, double x, double y, Uint32 color)
{
	SDL_Color sdlcolor;
	SDL_Rect destination;
	SDL_Surface *text;

	SDL_GetRGB (color, screen->format, &(sdlcolor.r), &(sdlcolor.g),
		    &(sdlcolor.b));
	sdlcolor.unused = 0;

	destination.x = x;
	destination.w = 0;
	destination.y = y;
	destination.h = 0;

	text = TTF_RenderText_Solid (font, string, sdlcolor);
	SDL_BlitSurface (text, NULL, screen, &destination);
	SDL_FreeSurface (text);
}

void
load_blit (SDL_Surface **image, char *string)
{
	*image = SDL_DisplayFormat (IMG_Load (string));
	if (image == NULL) {
		printf ("WAAAA! Image didn't load!\n");
		exit (1);
	}
	SDL_SetColorKey (*image, SDL_SRCCOLORKEY, 0xff00ff);
}

void
draw_blit (SDL_Surface *image, int x, int y)
{
	SDL_Rect dest;

	dest.x = x;
	dest.y = y;

	SDL_BlitSurface (image, NULL, screen, &dest);
}

void
init_gl (int *argc, char **argv)
{
	qobj[SOLID] = gluNewQuadric ();
	gluQuadricDrawStyle (qobj[SOLID], GLU_FILL);
	qobj[WIRE] = gluNewQuadric ();
	gluQuadricDrawStyle (qobj[WIRE], GLU_LINE);
	
	glShadeModel (GL_SMOOTH);

	glutInit (argc, argv);

	libview.dist = 10;
	libview.phi = M_PI / 4;
}

void
init_sdl_gl_flags (int width, int height, Uint32 flags)
{
	if (SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) != 0) {
		printf ("Error: %s\n", SDL_GetError ());
		exit (1);
	}

	atexit (SDL_Quit);

	srandom (time (NULL));

	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 1);

	SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 6);
	SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 5);

	if (SDL_SetVideoMode (width, height, 16, SDL_OPENGL | flags) == NULL) {
		printf ("Error: %s\n", SDL_GetError ());
		exit(1);
	}
}

void grid (int lights, float color, int colors)
{
	float normal_color[3];

	if (lights)
		glDisable (GL_LIGHTING);
	
	normal_color[0] = color;
	normal_color[1] = color;
	normal_color[2] = color;

	glBegin (GL_LINES);
	if (colors)
		glColor3f (0, 0, 1);
	glVertex3f (-5, 5, 0);
	glVertex3f (5, 5, 0);
	glColor3fv (normal_color);
	glVertex3f(-5, 4, 0);
	glVertex3f(5, 4, 0);
	glVertex3f(-5, 3, 0);
	glVertex3f(5, 3, 0);
	glVertex3f(-5, 2, 0);
	glVertex3f(5, 2, 0);
	glVertex3f(-5, 1, 0);
	glVertex3f(5, 1, 0);
	if (colors)
		glColor3f(1, 0, 0);
	glVertex3f(-5, 0, 0);
	glVertex3f(5, 0, 0);
	glColor3fv(normal_color);
	glVertex3f(-5, -1, 0);
	glVertex3f(5, -1, 0);
	glVertex3f(-5, -2, 0);
	glVertex3f(5, -2, 0);
	glVertex3f(-5, -3, 0);
	glVertex3f(5, -3, 0);
	glVertex3f(-5, -4, 0);
	glVertex3f(5, -4, 0);
	if (colors)
		glColor3f(1, 1, 0);
	glVertex3f(-5, -5, 0);
	glVertex3f(5, -5, 0);
	glEnd();
	glBegin(GL_LINES);
	glColor3fv(normal_color);
	glVertex3f(-5, 5, 0);
	glVertex3f(-5, -5, 0);
	glVertex3f(-4, 5, 0);
	glVertex3f(-4, -5, 0);
	glVertex3f(-3, 5, 0);
	glVertex3f(-3, -5, 0);
	glVertex3f(-2, 5, 0);
	glVertex3f(-2, -5, 0);
	glVertex3f(-1, 5, 0);
	glVertex3f(-1, -5, 0);
	if (colors)
		glColor3f(0, 1, 0);
	glVertex3f(-0, 5, 0);
	glVertex3f(-0, -5, 0);
	glColor3fv(normal_color);
	glVertex3f(1, 5, 0);
	glVertex3f(1, -5, 0);
	glVertex3f(2, 5, 0);
	glVertex3f(2, -5, 0);
	glVertex3f(3, 5, 0);
	glVertex3f(3, -5, 0);
	glVertex3f(4, 5, 0);
	glVertex3f(4, -5, 0);
	glVertex3f(5, 5, 0);
	glVertex3f(5, -5, 0);
	glEnd();
	if (lights)
		glEnable (GL_LIGHTING);

}

void
distanceLookAt (double x, double y, double z, double dist, double theta,
		double phi)
{
	double pos[3];
	int zaxis = 1;
	int intphi;
	double floatphi;

	theta = RTOD (theta);
	phi = RTOD (phi);

	intphi = (int) phi;
	floatphi = phi - intphi;

	intphi = intphi % 360;

	phi = intphi + floatphi;

	if (abs(phi) > 90) {
		zaxis = -1;
		if (abs(phi) > 270) {
			zaxis = 1;
		}
	}
			
	theta = DTOR(theta);
	phi = DTOR(phi);

	if (dist < 0)
		dist = 0;

	pos[0] = x + dist * cos (theta) * cos (phi);
	pos[1] = y + dist * sin (theta) * cos (phi);
	pos[2] = z + dist * sin (phi);

	gluLookAt (pos[0], pos[1], pos[2],
		   x, y, z,
		   0, 0, zaxis);
}

void
color_coords (int lights)
{
	if (lights)
		glDisable (GL_LIGHTING);

	glBegin (GL_LINES);
	glColor3f (1, 0, 0);
	glVertex3f (0, 0, 0);
	glVertex3f (30, 0, 0);
	glEnd ();

	glBegin (GL_LINES);
	glColor3f (0, 1, 0);
	glVertex3f (0, 0, 0);
	glVertex3f (0, 30, 0);
	glEnd ();

	glBegin (GL_LINES);
	glColor3f (0, 0, 1);
	glVertex3f (0, 0, 0);
	glVertex3f (0, 0, 30);
	glEnd ();

	if (lights)
		glEnable (GL_LIGHTING);
}

void
makeTexture (GLuint texName, int ImageSize, GLubyte ***Image)
{
	glBindTexture (GL_TEXTURE_2D, texName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, ImageSize, ImageSize, 0,
		      GL_RGBA, GL_UNSIGNED_BYTE, Image);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

double
vdot (struct vect *v1, struct vect *v2)
{
	return ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z));
}

void
vcross (struct vect *v1, struct vect *v2, struct vect *v3)
{
	struct vect result;

	result.x = (v2->y * v3->z) - (v2->z * v3->y);
	result.y = (v2->z * v3->x) - (v2->x * v3->z);
	result.z = (v2->x * v3->y) - (v2->y * v3->x);

	*v1 = result;
}

void
vadd (struct vect *v1, struct vect *v2, struct vect *v3)
{
	v1->x = v2->x + v3->x;
	v1->y = v2->y + v3->y;
	v1->z = v2->z + v3->z;
}

void
vsub (struct vect *v1, struct vect *v2, struct vect *v3)
{
	v1->x = v2->x - v3->x;
	v1->y = v2->y - v3->y;
	v1->z = v2->z - v3->z;
}

void
vscal (struct vect *v1, struct vect *v2, double s)
{
	v1->x = v2->x * s;
	v1->y = v2->y * s;
	v1->z = v2->z * s;
}

double
square (double a)
{
	return (a * a);
}

double
hypot3 (double a, double b, double c)
{
	return (sqrt (square (a) + square (b) + square (c)));
}

double
hypot3v (struct vect *v)
{
	return (sqrt (square (v->x) + square (v->y) + square (v->z)));
}

double
hypotv (struct vect *v)
{
	return (hypot (v->x, v->y));
}

void
psub (struct vect *v1, struct pt *p1, struct pt *p2)
{
	v1->x = p1->x - p2->x;
	v1->y = p1->y - p2->y;
	v1->z = p1->z - p2->z;
}

void
vnorm (struct vect *v1, struct vect *v2)
{
	struct vect result;
	
	if (hypot3v (v2) == 0)
		printf ("file %s: line %d: floating point exception\n",
			__FILE__, __LINE__);

	result.x = v2->x / hypot3v (v2);
	result.y = v2->y / hypot3v (v2);
	result.z = v2->z / hypot3v (v2);

	*v1 = result;
}

void
arrayvcross (double *a, double *b, double *c)
{
	// a = b cross c
	a[0] = b[1] * c[2] - b[2] * c[1];
	a[1] = b[2] * c[0] - b[0] * c[2];
	a[2] = b[0] * c[1] - b[1] * c[0];
}

void
arrayvsub (double *a, double *b, double *c)
{
	// a = b - c
	a[0] = b[0] - c[0];
	a[1] = b[1] - c[1];
	a[2] = b[2] - c[2];
}

void
addvtopt (struct pt *p1, struct pt *p2, struct vect *v)
{
	p1->x = p2->x + v->x;
	p1->y = p2->y + v->y;
	p1->z = p2->z + v->z;
}

void
vset (struct vect *v, double x, double y, double z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

void
write_logvars (void)
{
	struct logvar *vp;
	char filename[1000];
	FILE *f;
	int idx;

	for (vp = logvars; vp; vp = vp->next) {
		sprintf (filename, "%s.dat", vp->name);
		if ((f = fopen (filename, "w")) == NULL) {
			fprintf (stderr, "can't create %s\n", filename);
			exit (1);
		}
		for (idx = 0; idx < logvar_idx; idx++) {
			fprintf (f, "%.14g %.14g\n",
				 logvar_times[idx] - logvar_times[0],
				 vp->samples[idx]);
		}
		fclose (f);
	}
}

void
add_logvar (char *name, double *valp)
{
	struct logvar *vp;
	static int beenhere;

	if (beenhere == 0) {
		beenhere = 1;
		atexit (write_logvars);
	}

	vp = xcalloc (1, sizeof *vp);
	vp->name = strdup (name);
	vp->valp = valp;

	vp->next = logvars;
	logvars = vp;
}

void
capture_logvars (void)
{
	struct logvar *vp;

	if (logvar_idx < LOGVAR_NSAMPLES) {
		logvar_times[logvar_idx] = get_secs ();
		for (vp = logvars; vp; vp = vp->next) {
			vp->samples[logvar_idx] = *vp->valp;
		}
	}
	logvar_idx++;
}

void
vprint (struct vect *v)
{
	printf ("%8.3f, %8.3f, %8.3f\n", v->x, v->y, v->z);
}

void
pset (struct pt *p, double x, double y, double z)
{
	p->x = x;
	p->y = y;
	p->z = z;
}

double
dist_pt_to_plane (struct pt *p, struct plane *pl)
{
	return (-(((pl->a * p->x) + (pl->b * p->y) + (pl->c * p->z) + pl->d)
		  / (square (pl->a) + square (pl->b) + square (pl->c))));

	/* return ((pl->a * p->x + pl->b * p->y + pl->c * p->z) */
	/* 	/ (sqrt (square (pl->a) + square (pl->b) + square (pl->c)))); */
}

double
z_at_pt_on_plane (struct pt *p, struct plane *pl)
{
	return ((-pl->a * p->x - pl->b * p->y - pl->d) / pl->c);
}

void
pt_on_z_plane (struct pt *p1, struct pt *p2, struct plane *plane)
{
	struct pt p3;
	double b2;

	b2 = p2->y - (plane->b / plane->a) * p2->x;

	p3.x = -(((plane->c * p2->z) / plane->b) + (plane->d / plane->a) + b2)
		/ ((plane->b / plane->a) + (plane->a / plane->b));
	p3.y = ((plane->b / plane->a) * p3.x) + b2;
	p3.z = p2->z;

	p1->x = p3.x;
	p1->y = p3.y;
	p1->z = p3.z;
}

void
gauss_e3x3 (struct pt *pt, struct plane *pl1,
	    struct plane *pl2, struct plane *pl3)
{
	int i;
	double A[3][4], k, stash;

	A[0][0] = pl1->a;
	A[0][1] = pl1->b;
	A[0][2] = pl1->c;
	A[0][3] = -pl1->d;

	A[1][0] = pl2->a;
	A[1][1] = pl2->b;
	A[1][2] = pl2->c;
	A[1][3] = -pl2->d;

	A[2][0] = pl3->a;
	A[2][1] = pl3->b;
	A[2][2] = pl3->c;
	A[2][3] = -pl3->d;

	if (fabs (A[1][0]) >= fabs (A[0][0])
	    && fabs (A[1][0]) >= fabs (A[2][0])) {
		for (i = 0; i < 4; i++) {
			stash = A[1][i];
			A[1][i] = A[0][i];
			A[0][i] = stash;
		}
	} else if (fabs (A[2][0]) >= fabs (A[0][0])
		   && fabs (A[2][0]) >= fabs (A[1][0])) {
		for (i = 0; i < 4; i++) {
			stash = A[2][i];
			A[2][i] = A[0][i];
			A[0][i] = stash;
		}
	}

	if (fabs (0 - A[0][0]) < 1e-6) {
		printf ("no single intersection point, all a's are 0");
		exit (1);
	}

	k = A[1][0] / A[0][0];
	A[1][0] = 0;
	A[1][1] -= A[0][1] * k;
	A[1][2] -= A[0][2] * k;
	A[1][3] -= A[0][3] * k;

	k = A[2][0] / A[0][0];
	A[2][0] = 0;
	A[2][1] -= A[0][1] * k;
	A[2][2] -= A[0][2] * k;
	A[2][3] -= A[0][3] * k;


	if (fabs (A[2][1]) >= fabs (A[1][1])) {
		for (i = 0; i < 4; i++) {
			stash = A[2][i];
			A[2][i] = A[1][i];
			A[1][i] = stash;
		}
	}

	if (fabs (0 - A[1][1]) < 1e-6) {
		printf ("no single intersection point, all b's are 0\n");
		exit (1);
	}

	k = A[2][1] / A[1][1];
	A[2][1] = 0;
	A[2][2] -= A[1][2] * k;
	A[2][3] -= A[1][3] * k;

	if (fabs (0 - A[2][2]) < 1e-6) {
		printf ("no single intersection point, last c is 0\n");
	}

	pt->z = A[2][3] / A[2][2];
	pt->y = (A[1][3] - (A[1][2] * pt->z)) / A[1][1];
	pt->x = (A[0][3] - (A[0][1] * pt->y) - (A[0][2] * pt->z)) / A[0][0];
}
