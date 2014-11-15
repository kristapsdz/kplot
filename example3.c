#include <assert.h>
#include <cairo.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kplot.h"

int
main(int argc, char *argv[])
{
	struct kdata	*d;
	struct kplot	*p;
	cairo_surface_t	*surf;
	size_t		 i;
	cairo_t		*cr;
	cairo_status_t	 st;
	int		 rc, c;

	rc = EXIT_FAILURE;

	d = NULL;
	p = NULL;

	if (NULL == (d = kdata_hist_alloc(0.0, 1.0, 50))) {
		perror(NULL);
		goto out;
	}

	for (i = 0; i < 1000; i++) {
		c = kdata_hist_increment
			(d, arc4random() / (double)UINT32_MAX);
		assert(c);
	}

	if (NULL == (p = kplot_alloc())) {
		perror(NULL);
		goto out;
	} else if ( ! kplot_data(p, d, KPLOT_LINES, NULL)) {
		perror(NULL);
		goto out;
	}

	kdata_destroy(d);
	d = NULL;

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

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); 
	cairo_rectangle(cr, 0.0, 0.0, 400.0, 400.0);
	cairo_fill(cr);
	kplot_draw(p, 400.0, 400.0, cr, NULL);

	st = cairo_surface_write_to_png
		(cairo_get_target(cr), "example3.png");

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
	kdata_destroy(d);
	return(rc);
}
