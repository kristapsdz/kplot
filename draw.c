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
#include <string.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

enum	defclr {
	DEFCLR_RED,
	DEFCLR_GREEN,
	DEFCLR_BLUE,
	DEFCLR_GREY,
	DEFCLR_BLACK,
	DEFCLR__MAX
};

static void
kdata_extrema(const struct kplotdat *d, 
	struct kpair *minv, struct kpair *maxv)
{
	size_t	 	 i;
	struct kpair	*p;

	p = d->data->pairs;

	for (i = 0; i < d->data->pairsz; i++) {
		if (0.0 != p[i].x && ! isnormal(p[i].x))
			continue;
		if (0.0 != p[i].y && ! isnormal(p[i].y))
			continue;
		if (p[i].x < minv->x)
			minv->x = p[i].x;
		if (p[i].y < minv->y)
			minv->y = p[i].y;
		if (p[i].x > maxv->x)
			maxv->x = p[i].x;
		if (p[i].y > maxv->y)
			maxv->y = p[i].y;
	}

	if (EXTREMA_XMIN & d->cfg.extrema)
		minv->x = d->cfg.extrema_xmin;
	if (EXTREMA_YMIN & d->cfg.extrema)
		minv->y = d->cfg.extrema_ymin;
	if (EXTREMA_XMAX & d->cfg.extrema)
		maxv->x = d->cfg.extrema_xmax;
	if (EXTREMA_YMAX & d->cfg.extrema)
		maxv->y = d->cfg.extrema_ymax;
}

static inline void
kpoint_to_real(const struct kpair *data, struct kpair *real,
	const struct kpair *minv, const struct kpair *maxv,
	double w, double h)
{

	if (maxv->x == minv->x)
		real->x = 0.0;
	else
		real->x = w * (data->x - minv->x) / (maxv->x - minv->x);

	if (maxv->y == minv->y)
		real->y = h;
	else
		real->y = h - h * (data->y - minv->y) / (maxv->y - minv->y);
}

static int
kplotctx_point_to_real(const struct kpair *data, 
	struct kpair *real, const struct kplotctx *ctx)
{

	if (0.0 != data->x && ! isnormal(data->x))
		return(0);
	if (0.0 != data->y && ! isnormal(data->y))
		return(0);

	kpoint_to_real(data, real, &ctx->minv, &ctx->maxv, ctx->w, ctx->h);
	return(1);
}

static void
kplotctx_draw_lines(struct kplotctx *ctx, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 pair;
	int		 rc;

	/* Skip past bad points to get to initial. */
	for (rc = 0, i = 0; 0 == rc && i < d->data->pairsz; i++)
		rc = kplotctx_point_to_real
			(&d->data->pairs[i], &pair, ctx);

	kplotctx_line_init(ctx, &d->cfg.line);
	cairo_move_to(ctx->cr, pair.x, pair.y);

	/* Mark remaining. */
	for ( ; i < d->data->pairsz; i++) {
		rc = kplotctx_point_to_real
			(&d->data->pairs[i], &pair, ctx);
		if (rc > 0)
			cairo_line_to(ctx->cr, pair.x, pair.y);
	}
	cairo_stroke(ctx->cr);
}

static void
kplotctx_draw_points(struct kplotctx *ctx, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 pair;
	int		 rc;

	kplotctx_point_init(ctx, &d->cfg.point);

	for (i = 0; i < d->data->pairsz; i++) {
		/* Skip bad points. */
		rc = kplotctx_point_to_real
			(&d->data->pairs[i], &pair, ctx);
		if (0 == rc)
			continue;
		cairo_arc(ctx->cr, pair.x, pair.y, 
			d->cfg.point.radius, 0, 2 * M_PI);
		cairo_stroke(ctx->cr);
	}
}

void
kplotfont_defaults(struct kplotfont *font)
{

	font->family = "serif";
	font->sz = 12.0;
	font->slant = CAIRO_FONT_SLANT_NORMAL;
	font->weight = CAIRO_FONT_WEIGHT_NORMAL;
	font->clr = DEFCLR_BLACK;
}

void
kplotcfg_defaults(struct kplotcfg *cfg)
{

	memset(cfg, 0, sizeof(struct kplotcfg));

	/* Colours: red, green, black, grey. */
	cfg->clrsz = DEFCLR__MAX;
	cfg->clrs[DEFCLR_RED].r = 1.0;
	cfg->clrs[DEFCLR_RED].a = 1.0;
	cfg->clrs[DEFCLR_GREEN].g = 1.0;
	cfg->clrs[DEFCLR_GREEN].a = 1.0;
	cfg->clrs[DEFCLR_BLUE].b = 1.0;
	cfg->clrs[DEFCLR_BLUE].a = 1.0;
	cfg->clrs[DEFCLR_BLACK].a = 1.0;
	cfg->clrs[DEFCLR_GREY].r = 0.5;
	cfg->clrs[DEFCLR_GREY].g = 0.5;
	cfg->clrs[DEFCLR_GREY].b = 0.5;
	cfg->clrs[DEFCLR_GREY].a = 1.0;

	/* Five left and bottom grey tic labels. */
	kplotfont_defaults(&cfg->ticlabelfont);
	cfg->ticlabelfont.clr = DEFCLR_GREY;
	cfg->ticlabel = TICLABEL_LEFT | TICLABEL_BOTTOM;
	cfg->xticlabelpad = cfg->yticlabelpad = 15.0;
	cfg->xtics = cfg->ytics = 5;

	/* A bit of margin. */
	cfg->margin = MARGIN_ALL;
	cfg->marginsz = 15.0;
	
	/* Innie tics, grey. */
	cfg->tic = TIC_LEFT_IN | TIC_BOTTOM_IN;
	cfg->ticline.clr = DEFCLR_BLACK;
	cfg->ticline.len = 5.0;
	cfg->ticline.sz = 1.0;

	/* Grid line: dotted, grey. */
	cfg->grid = GRID_ALL;
	cfg->gridline.clr = DEFCLR_GREY;
	cfg->gridline.sz = 1.0;
	cfg->gridline.dashes[0] = 1.0;
	cfg->gridline.dashes[1] = 4.0;
	cfg->gridline.dashesz = 2;

	/* Border line: solid, grey. */
	cfg->border = BORDER_LEFT | BORDER_BOTTOM;
	cfg->borderline.clr = DEFCLR_BLACK;
	cfg->borderline.sz = 1.0;

	/* Black axis labels. */
	kplotfont_defaults(&cfg->axislabelfont);
	cfg->xaxislabelpad = cfg->yaxislabelpad = 15.0;
}

void
kplot_draw(const struct kplot *p, double w, 
	double h, cairo_t *cr, const struct kplotcfg *cfg)
{
	size_t	 	i;
	struct kplotctx	ctx;

	memset(&ctx, 0, sizeof(struct kplotctx));

	ctx.w = w;
	ctx.h = h;
	ctx.cr = cr;
	ctx.minv.x = ctx.minv.y = DBL_MAX;
	ctx.maxv.x = ctx.maxv.y = -DBL_MAX;

	if (NULL == cfg)
		kplotcfg_defaults(&ctx.cfg);
	else 
		ctx.cfg = *cfg;

	for (i = 0; i < p->datasz; i++) 
		kdata_extrema(&p->datas[i], &ctx.minv, &ctx.maxv);

	/*
	 * If we've read in no points, then initialise ourselves to be
	 * at zero.
	 */
	if (ctx.minv.x > ctx.maxv.x)
		ctx.minv.x = ctx.maxv.x = 0.0;
	if (ctx.minv.y > ctx.maxv.y)
		ctx.minv.y = ctx.maxv.y = 0.0;

	kplotctx_margin_init(&ctx);
	kplotctx_label_init(&ctx);
	kplotctx_grid_init(&ctx);
	kplotctx_border_init(&ctx);
	kplotctx_tic_init(&ctx);

	cairo_translate(ctx.cr, ctx.offs.x, ctx.offs.y);

	ctx.h = ctx.dims.y;
	ctx.w = ctx.dims.x;

	for (i = 0; i < p->datasz; i++)
		switch (p->datas[i].type) {
		case (KPLOT_POINTS):
			kplotctx_draw_points(&ctx, &p->datas[i]);
			break;
		case (KPLOT_LINES):
			kplotctx_draw_lines(&ctx, &p->datas[i]);
			break;
		}
}

