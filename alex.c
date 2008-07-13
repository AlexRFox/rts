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
d_to_r (double degrees)
{
        return (degrees / 360.0 * 2 * M_PI);
}
