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
kplotctx_ticlabel_init(struct kplotctx *ctx)
{
	char		buf[128];
	size_t		i;
	cairo_text_extents_t e;
	double		maxh, maxw, offs, lastx, lasty, firsty;
	struct kplotclr	clr;

	kplotctx_colour(ctx, ctx->cfg.ticlabelclr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_line_width(ctx->cr, ctx->cfg.bordersz);
	maxh = maxw = lastx = lasty = firsty = 0.0;

	/*
	 * First, acquire the maximum space that will be required for
	 * the vertical (left or right) or horizontal (top or bottom)
	 * labels.
	 * If we're rotating, make sure that we accomodate for the
	 * maximum rotated box.
	 */
	for (i = 0; i < ctx->cfg.xtics; i++) {
		offs = 1 == ctx->cfg.xtics ? 0.5 : 
			i / (double)(ctx->cfg.xtics - 1);

		/* Call out to xformat function? */
		if (NULL == ctx->cfg.xticlabelfmt)
			snprintf(buf, sizeof(buf), "%g", 
				ctx->minv.x + offs *
				(ctx->maxv.x - ctx->minv.x));
		else
			(*ctx->cfg.xticlabelfmt)
				(ctx->minv.x + offs *
				 (ctx->maxv.x - ctx->minv.x),
				 buf, sizeof(buf));

		cairo_text_extents(ctx->cr, buf, &e);

		/* 
		 * Important: if we're on the last x-axis value, then
		 * save the width, because we'll check that the
		 * right-hand buffer zone accomodates for it.
		 * FIXME: only do this is TICLABEL_TOP, etc...
		 */
		if (i == ctx->cfg.xtics - 1 && ctx->cfg.xticlabelrot > 0.0)
			lastx = e.width * cos
				(M_PI * 2.0 - 
				 (M_PI_2 - ctx->cfg.xticlabelrot)) +
				e.height * sin((ctx->cfg.xticlabelrot));
		else if (i == ctx->cfg.xtics - 1)
			lastx = e.width / 2.0;

		/* 
		 * If we're rotating, get our height by computing the
		 * sum of the vertical segments.
		 */
		if (ctx->cfg.xticlabelrot > 0.0)
			e.height = e.width * sin(ctx->cfg.xticlabelrot) +
				e.height * cos(M_PI * 2.0 - 
					(M_PI_2 - ctx->cfg.xticlabelrot));

		if (e.height > maxh)
			maxh = e.height;
	}

	for (i = 0; i < ctx->cfg.ytics; i++) {
		offs = 1 == ctx->cfg.ytics ? 0.5 : 
			i / (double)(ctx->cfg.ytics - 1);

		if (NULL == ctx->cfg.yticlabelfmt)
			snprintf(buf, sizeof(buf), "%g", 
				ctx->minv.y + offs *
				(ctx->maxv.y - ctx->minv.y));
		else
			(*ctx->cfg.yticlabelfmt)
				(ctx->minv.y + offs *
				 (ctx->maxv.y - ctx->minv.y),
				 buf, sizeof(buf));

		cairo_text_extents(ctx->cr, buf, &e);

		if (i == 0)
			firsty = e.height / 2.0;
		if (i == ctx->cfg.ytics - 1)
			lasty = e.height / 2.0;

		if (e.width > maxw)
			maxw = e.width;
	}
	
	if (TICLABEL_LEFT & ctx->cfg.ticlabel) {
		ctx->offs.x += maxw + ctx->cfg.ylabelpad;
		ctx->dims.x -= maxw + ctx->cfg.ylabelpad; 
	}

	/* 
	 * Accomodate for the right label.
	 * Also check if our overflow for the horizontal axes into the
	 * right buffer zone exists.
	 */
	if (TICLABEL_RIGHT & ctx->cfg.ticlabel) {
		if (maxw + ctx->cfg.ylabelpad > lastx)
			ctx->dims.x -= maxw + ctx->cfg.ylabelpad;
		else
			ctx->dims.x -= lastx;
	} else if (lastx > 0.0) 
		ctx->dims.x -= lastx;

	/*
	 * Like with TICLABEL_RIGHT, we accomodate for the topmost vertical
	 * axes bleeding into the horizontal axis area above.
	 */
	if (TICLABEL_TOP & ctx->cfg.ticlabel) {
		if (maxh + ctx->cfg.xlabelpad > lasty) {
			ctx->offs.y += maxh + ctx->cfg.xlabelpad;
			ctx->dims.y -= maxh + ctx->cfg.xlabelpad;
		} else {
			ctx->offs.y += lasty;
			ctx->dims.y -= lasty;
		}
	} else if (lasty > 0.0) {
		ctx->offs.y += lasty;
		ctx->dims.y -= lasty;
	}

	if (TICLABEL_BOTTOM & ctx->cfg.ticlabel) {
		if (maxh + ctx->cfg.xlabelpad > firsty)
			ctx->dims.y -= maxh + ctx->cfg.xlabelpad;
		else
			ctx->dims.y -= firsty;
	} else if (firsty > 0.0)
		ctx->dims.y -= firsty;

	for (i = 0; i < ctx->cfg.xtics; i++) {
		offs = 1 == ctx->cfg.xtics ? 0.5 : 
			i / (double)(ctx->cfg.xtics - 1);

		if (NULL == ctx->cfg.xticlabelfmt)
			snprintf(buf, sizeof(buf), "%g", 
				ctx->minv.x + offs *
				(ctx->maxv.x - ctx->minv.x));
		else
			(*ctx->cfg.xticlabelfmt)
				(ctx->minv.x + offs *
				 (ctx->maxv.x - ctx->minv.x),
				 buf, sizeof(buf));

		cairo_text_extents(ctx->cr, buf, &e);

		if (TICLABEL_BOTTOM & ctx->cfg.ticlabel) {
			if (ctx->cfg.xticlabelrot > 0.0) {
				cairo_move_to(ctx->cr, 
					ctx->offs.x + 
					offs * ctx->dims.x,
					ctx->offs.y + ctx->dims.y + 
					e.height * cos
					 (M_PI * 2.0 - 
					  (M_PI_2 - ctx->cfg.xticlabelrot)) +
					ctx->cfg.xlabelpad);
				cairo_save(ctx->cr);
				cairo_rotate(ctx->cr, ctx->cfg.xticlabelrot);
			} else 
				cairo_move_to(ctx->cr, 
					ctx->offs.x + offs * ctx->dims.x -
					(e.width / 2.0), 
					ctx->offs.y + ctx->dims.y + 
					maxh + ctx->cfg.xlabelpad);

			cairo_show_text(ctx->cr, buf);
			if (ctx->cfg.xticlabelrot > 0.0)
				cairo_restore(ctx->cr);
		}

		if (TICLABEL_TOP & ctx->cfg.ticlabel) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + offs * ctx->dims.x -
				(e.width / 2.0), 
				ctx->offs.y - maxh);
			cairo_show_text(ctx->cr, buf);
		}
	}

	for (i = 0; i < ctx->cfg.ytics; i++) {
		offs = 1 == ctx->cfg.ytics ? 0.5 : 
			i / (double)(ctx->cfg.ytics - 1);

		if (NULL == ctx->cfg.yticlabelfmt)
			snprintf(buf, sizeof(buf), "%g", 
				ctx->minv.y + offs *
				(ctx->maxv.y - ctx->minv.y));
		else
			(*ctx->cfg.yticlabelfmt)
				(ctx->minv.y + offs *
				 (ctx->maxv.y - ctx->minv.y),
				 buf, sizeof(buf));

		cairo_text_extents(ctx->cr, buf, &e);

		if (TICLABEL_LEFT & ctx->cfg.ticlabel) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x - e.width - 
				ctx->cfg.ylabelpad,
				(ctx->offs.y + ctx->dims.y) - 
				(offs * ctx->dims.y) + 
				(e.height / 2.0));
			cairo_show_text(ctx->cr, buf);
		}
		if (TICLABEL_RIGHT & ctx->cfg.ticlabel) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + ctx->dims.x + 
				ctx->cfg.ylabelpad,
				(ctx->offs.y + ctx->dims.y) - 
				(offs * ctx->dims.y) + 
				(e.height / 2.0));
			cairo_show_text(ctx->cr, buf);
		}
	}
}
