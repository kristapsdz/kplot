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
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kplotctx_colour_init(struct kplotctx *ctx, 
	size_t idx, struct kplotclr *res)
{

	if (0 == ctx->cfg.clrsz) {
		res->r = 0.0;
		res->g = 0.0;
		res->b = 0.0;
		res->a = 1.0;
	} else
		*res = ctx->cfg.clrs[idx % ctx->cfg.clrsz];
}


void
kplotctx_font_init(struct kplotctx *ctx, const struct kplotfont *font)
{
	struct kplotclr	 clr;

	kplotctx_colour_init(ctx, font->clr, &clr);
	cairo_select_font_face
		(ctx->cr, font->family,
		 font->slant,
		 font->weight);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_font_size(ctx->cr, font->sz);
}

void
kplotctx_point_init(struct kplotctx *ctx, const struct kplotpoint *pnt)
{
	struct kplotclr	 clr;

	kplotctx_colour_init(ctx, pnt->clr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_line_width(ctx->cr, pnt->sz);
	cairo_set_dash(ctx->cr, pnt->dashes, 
		pnt->dashesz, pnt->dashoff);
}

void
kplotctx_ticln_init(struct kplotctx *ctx, const struct kplotticln *line)
{
	struct kplotclr	 clr;

	kplotctx_colour_init(ctx, line->clr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_line_width(ctx->cr, line->sz);
	cairo_set_dash(ctx->cr, line->dashes, 
		line->dashesz, line->dashoff);
}

void
kplotctx_line_init(struct kplotctx *ctx, const struct kplotline *line)
{
	struct kplotclr	 clr;

	kplotctx_colour_init(ctx, line->clr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_line_width(ctx->cr, line->sz);
	cairo_set_dash(ctx->cr, line->dashes, 
		line->dashesz, line->dashoff);
	cairo_set_line_join(ctx->cr, line->join);
}

/*
 * Given a plotting context and a position for drawing a line, determine
 * whether we want to "fix" the line so that it's fine.
 * This is a foible of Cairo and drawing with doubles.
 */
double
kplotctx_line_fix(const struct kplotctx *ctx, double sz, double pos)
{
	double	 v;

	if (0 == (int)sz % 2)
		return(pos);
	v = pos - floor(pos);
	return(v < DBL_EPSILON ? pos + 0.5 : pos - v + 0.5);
}
