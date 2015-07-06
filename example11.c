/*	$Id$ */
/*
 * Copyright (c) 2015 Kristaps Dzonsons <kristaps@bsd.lv>
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

#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __linux__
#include <bsd/stdlib.h>
#endif

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
	int		 rc;

	rc = EXIT_FAILURE;

	d1 = NULL;
	p = NULL;

	for (i = 0; i < 10; i++) {
		points1[i].x = time(NULL) + i * 24 * 60 * 60;
		points1[i].y = arc4random_uniform(100);
	}

	kplotcfg_defaults(&cfg);
	cfg.xticlabelpad = cfg.yticlabelpad = 10.0;
	cfg.xticlabelrot = M_PI_4;
	cfg.xticlabelfmt = format;
	cfg.xaxislabel = "xxxx";
	cfg.x2axislabel = "zzzz";
	cfg.yaxislabel = "yyyy";
	cfg.y2axislabel = "uuuu";
	cfg.yaxislabelrot = M_PI_2 * 3.0;

	if (NULL == (d1 = kdata_array_alloc(points1, 10))) {
		perror(NULL);
		goto out;
	} else if (NULL == (p = kplot_alloc(&cfg))) {
		perror(NULL);
		goto out;
	} else if ( ! kplot_attach_data(p, d1, KPLOT_POINTS, NULL)) {
		perror(NULL);
		goto out;
	}

	kdata_destroy(d1);
	d1 = NULL;

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

	cairo_set_font_size(cr, 14.0);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); 
	cairo_rectangle(cr, 0.0, 0.0, 600.0, 400.0);
	cairo_fill(cr);
	kplot_draw(p, 600.0, 400.0, cr);

	st = cairo_surface_write_to_png
		(cairo_get_target(cr), "example11.png");

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
#ifdef	__APPLE__
	cairo_debug_reset_static_data();
#endif
	return(rc);
}
