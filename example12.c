/*	$Id$ */
/*
 * Copyright (c) 2014 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "compat.h"

#include <assert.h>
#include <cairo.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <bsd/stdlib.h>
#endif

#include "kplot.h"

static void
fill(size_t pos, struct kpair *val, void *dat)
{

	val->x = pos;
	val->y = arc4random() / (double)UINT32_MAX;
}

int
main(int argc, char *argv[])
{
	struct kdata	*d[4];
	struct kplot	*p;
	cairo_surface_t	*surf;
	size_t		 i;
	cairo_t		*cr;
	cairo_status_t	 st;
	int		 rc, c;

	rc = EXIT_FAILURE;

	d[0] = kdata_array_alloc(NULL, 100);
	d[1] = kdata_array_alloc(NULL, 100);
	d[2] = kdata_array_alloc(NULL, 100);
	d[3] = kdata_array_alloc(NULL, 100);

	for (i = 0; i < 100; i++) {
		c = kdata_array_fill(d[0], NULL, fill);
		assert(c);
		c = kdata_array_fill(d[1], NULL, fill);
		assert(c);
		c = kdata_array_fill(d[2], NULL, fill);
		assert(c);
		c = kdata_array_fill(d[3], NULL, fill);
		assert(c);
	}

	if (NULL == (p = kplot_alloc(NULL))) {
		perror(NULL);
		goto out;
	} 

	rc = kplot_attach_data(p, d[0], KPLOT_POINTS, NULL);
	assert(rc);
	rc = kplot_attach_data(p, d[1], KPLOT_POINTS, NULL);
	assert(rc);
	rc = kplot_attach_data(p, d[2], KPLOT_POINTS, NULL);
	assert(rc);
	rc = kplot_attach_data(p, d[3], KPLOT_POINTS, NULL);
	assert(rc);

	rc = kplot_detach(p, d[2]);
	assert(rc);

	kdata_destroy(d[0]);
	kdata_destroy(d[1]);
	kdata_destroy(d[2]);
	kdata_destroy(d[3]);

	d[0] = d[1] = d[2] = d[3] = NULL;

	surf = cairo_image_surface_create
		(CAIRO_FORMAT_ARGB32, 600, 400);

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
	cairo_rectangle(cr, 0.0, 0.0, 600.0, 400.0);
	cairo_fill(cr);
	kplot_draw(p, 600.0, 400.0, cr);

	st = cairo_surface_write_to_png
		(cairo_get_target(cr), "example12.png");

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
	kdata_destroy(d[0]);
	kdata_destroy(d[1]);
	kdata_destroy(d[2]);
	kdata_destroy(d[3]);
#ifdef	__APPLE__
	cairo_debug_reset_static_data();
#endif
	return(rc);
}
