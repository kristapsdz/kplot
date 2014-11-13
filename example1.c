#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"

int
main(int argc, char *argv[])
{
	struct kpair	 points1[10], points2[10];
	struct kdata	*d1, *d2;
	struct kplot	*p;
	cairo_surface_t	*surf;
	size_t		 i;
	cairo_t		*cr;
	cairo_status_t	 st;
	int		 rc;

	rc = EXIT_FAILURE;

	d1 = d2 = NULL;
	p = NULL;

	for (i = 0; i < 10; i++) {
		points1[i].x = points1[i].y = (double)i * 1000;
		points2[i].x = points2[i].y = (double)i * -1000;
	}

	if (NULL == (d1 = kdata_alloc())) {
		perror(NULL);
		goto out;
	} else if (NULL == (d2 = kdata_alloc())) {
		perror(NULL);
		goto out;
	} else if ( ! kdata_data(d1, points1, 10)) {
		perror(NULL);
		goto out;
	} else if ( ! kdata_data(d2, points2, 10)) {
		perror(NULL);
		goto out;
	} else if (NULL == (p = kplot_alloc())) {
		perror(NULL);
		goto out;
	} else if ( ! kplot_data(p, d1, KPLOT_LINES, NULL)) {
		perror(NULL);
		goto out;
	} else if ( ! kplot_data(p, d2, KPLOT_POINTS, NULL)) {
		perror(NULL);
		goto out;
	}

	kdata_unref(d1);
	kdata_unref(d2);

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

	kplot_draw(p, 400.0, 400.0, cr, NULL);

	st = cairo_surface_write_to_png
		(cairo_get_target(cr), "example1.png");

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
	kdata_unref(d1);
	kdata_unref(d2);
	return(rc);
}
