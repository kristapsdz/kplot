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
#include <assert.h>
#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kplotctx_axislabel_init(struct kplotctx *ctx)
{
#if 0
	const char	*xaxislabel = "foo";
	const double	 xaxislabelrot = M_PI_2 - 0.01;
	const size_t	 xaxislabelclr = 4;
	double 		 h, w;
	cairo_text_extents_t e;
	struct kplotclr	 clr;

	kplotctx_colour(ctx, xaxislabelclr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);

	cairo_text_extents(ctx->cr, xaxislabel, &e);

	h = abs(e.width * sin(xaxislabelrot)) + abs(e.height * cos(xaxislabelrot));
	w = abs(e.width * cos(xaxislabelrot)) + abs(e.height * sin(xaxislabelrot));

	warnx("h = %g, w = %g (%g, %g)", h, w, e.height, e.width);

	cairo_move_to(ctx->cr, 
			ctx->offs.x + ctx->dims.x / 2.0 - w,
			ctx->offs.y + ctx->dims.y - h);
	cairo_save(ctx->cr);
	cairo_rotate(ctx->cr, xaxislabelrot);
	cairo_show_text(ctx->cr, xaxislabel);
	cairo_restore(ctx->cr);
	ctx->dims.y -= h;
#endif
}
