#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "kplot.h"

static void
format(double v, char *buf, size_t bufsz)
{
	time_t	t = (time_t)v;

	strftime(buf, bufsz, "%F", localtime(&t));
}

int
main(int argc, char *argv[])
{
	struct kplotcfg	 cfg;
	struct kpair	 points1[10];
	struct kdata	*d1;
	struct kplot	*p;
	cairo_surface_t	*surf;
	size_t		 i;
	cairo_t		*cr;
	cairo_status_t	 st;
	time_t		 t;
	int		 rc;

	rc = EXIT_FAILURE;

	d1 = NULL;
	p = NULL;
	t = time(NULL);

	for (i = 0; i < 10; i++) {
		points1[i].x = time(NULL) + i * 24 * 60 * 60;
		points1[i].y = arc4random_uniform(100);
	}

	kplotcfg_defaults(&cfg);
	cfg.label &= ~LABEL_BOTTOM;
	cfg.marginsz = 0.0;
	cfg.xlabelpad = cfg.ylabelpad = 10.0;
	cfg.xrot = M_PI_4;
	cfg.xformat = format;

	if (NULL == (d1 = kdata_array_alloc(points1, 10))) {
		perror(NULL);
		goto out;
	} else if (NULL == (p = kplot_alloc())) {
		perror(NULL);
		goto out;
	} else if ( ! kplot_data(p, d1, KPLOT_POINTS, NULL)) {
		perror(NULL);
		goto out;
	}

	kdata_destroy(d1);
	d1 = NULL;

	surf = cairo_image_surface_create
		(CAIRO_FORMAT_ARGB32, 400, 400);

	st = cairo_surface_status(surf);
	if (CAIRO_STATUS_SUCCESS != st) {
		fprintf(stderr, "%s", cairo_status_to_string(st));
		cairo_surface_destroy(surf);
		kplot_free(p);
		return(EXIT_FAILURE);
	}

	cr = cairo_create(surf);
	cairo_surface_destroy(surf);

	st = cairo_status(cr);
	if (CAIRO_STATUS_SUCCESS != st) {
		fprintf(stderr, "%s", cairo_status_to_string(st));
		cairo_destroy(cr);
		kplot_free(p);
		return(EXIT_FAILURE);

	}

	cairo_set_font_size(cr, 14.0);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); 
	cairo_rectangle(cr, 0.0, 0.0, 400.0, 400.0);
	cairo_fill(cr);
	kplot_draw(p, 400.0, 400.0, cr, &cfg);

	st = cairo_surface_write_to_png
		(cairo_get_target(cr), "example2.png");

	if (CAIRO_STATUS_SUCCESS != st) {
		fprintf(stderr, "%s", cairo_status_to_string(st));
		cairo_destroy(cr);
		kplot_free(p);
		return(EXIT_FAILURE);
	}

	cairo_destroy(cr);
	rc = EXIT_SUCCESS;
out:
	kplot_free(p);
	kdata_destroy(d1);
	return(rc);
}
