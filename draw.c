/*	$Id$ */
/*
 * Copyright (c) 2014, 2015 Kristaps Dzonsons <kristaps@bsd.lv>
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
kdata_extrema_yerr(const struct kplotdat *d, 
	struct kpair *minv, struct kpair *maxv)
{
	size_t	 	 i, sz;
	struct kpair	*p, *err;

	assert(d->datasz > 1);
	p = d->datas[0]->pairs;
	err = d->datas[1]->pairs;

	/*
	 * Use the smaller of the two pair lengths.
	 */
	sz = d->datas[0]->pairsz < d->datas[1]->pairsz ?
		d->datas[0]->pairsz : d->datas[1]->pairsz;

	for (i = 0; i < sz; i++) {
		if (0.0 != p[i].x && ! isnormal(p[i].x))
			continue;
		if (0.0 != p[i].y && ! isnormal(p[i].y))
			continue;
		if (0.0 != err[i].y && ! isnormal(err[i].y))
			continue;
		if (p[i].x < minv->x)
			minv->x = p[i].x;
		if (p[i].x > maxv->x)
			maxv->x = p[i].x;
		/* 
		 * Since the error can be negative, check in both
		 * directions from the basis point.
		 */
		if (p[i].y - err[i].y < minv->y)
			minv->y = p[i].y - err[i].y;
		if (p[i].y + err[i].y < minv->y)
			minv->y = p[i].y + err[i].y;
		if (p[i].y - err[i].y > maxv->y)
			maxv->y = p[i].y - err[i].y;
		if (p[i].y + err[i].y > maxv->y)
			maxv->y = p[i].y + err[i].y;
	}

	if (EXTREMA_XMIN & d->cfgs[0].extrema)
		minv->x = d->cfgs[0].extrema_xmin;
	if (EXTREMA_YMIN & d->cfgs[0].extrema)
		minv->y = d->cfgs[0].extrema_ymin;
	if (EXTREMA_XMAX & d->cfgs[0].extrema)
		maxv->x = d->cfgs[0].extrema_xmax;
	if (EXTREMA_YMAX & d->cfgs[0].extrema)
		maxv->y = d->cfgs[0].extrema_ymax;
}

static void
kdata_extrema_single(const struct kplotdat *d, 
	struct kpair *minv, struct kpair *maxv)
{
	size_t	 	 i;
	struct kpair	*p;

	p = d->datas[0]->pairs;

	for (i = 0; i < d->datas[0]->pairsz; i++) {
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

	if (EXTREMA_XMIN & d->cfgs[0].extrema)
		minv->x = d->cfgs[0].extrema_xmin;
	if (EXTREMA_YMIN & d->cfgs[0].extrema)
		minv->y = d->cfgs[0].extrema_ymin;
	if (EXTREMA_XMAX & d->cfgs[0].extrema)
		maxv->x = d->cfgs[0].extrema_xmax;
	if (EXTREMA_YMAX & d->cfgs[0].extrema)
		maxv->y = d->cfgs[0].extrema_ymax;
}

static inline int
kpair_vrfy(const struct kpair *data)
{

	if (0.0 != data->x && ! isnormal(data->x))
		return(0);
	if (0.0 != data->y && ! isnormal(data->y))
		return(0);
	return(1);
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

	if ( ! kpair_vrfy(data))
		return(0);
	kpoint_to_real(data, real, 
		&ctx->minv, &ctx->maxv, ctx->w, ctx->h);
	return(1);
}

static size_t
kplotctx_draw_yerrline_start(struct kplotctx *ctx, 
	const struct kplotdat *d, size_t *end)
{
	size_t	 start;

	/* Overlap between both point sets. */
	*end = d->datas[0]->pairsz < d->datas[1]->pairsz ?
		d->datas[0]->pairsz : d->datas[1]->pairsz;

	/* Skip past bad points to get to initial. */
	for (start = 0; start < *end; start++)
		if (kpair_vrfy(&d->datas[0]->pairs[start]) &&
			kpair_vrfy(&d->datas[1]->pairs[start]))
			return(start);

	return(*end);
}

static void
kplotctx_draw_yerrline_basepoints(struct kplotctx *ctx, 
	size_t start, size_t end, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 pair;
	int		 rc;

	/* Write baseline first. */
	kplotctx_point_init(ctx, &d->cfgs[0].point);
	for (i = start; i < end; i++) {
		if ( ! (kpair_vrfy(&d->datas[0]->pairs[i]) &&
			kpair_vrfy(&d->datas[1]->pairs[i])))
			continue;
		rc = kplotctx_point_to_real
			(&d->datas[0]->pairs[i], &pair, ctx);
		assert(0 != rc);
		cairo_arc(ctx->cr, pair.x, pair.y, 
			d->cfgs[0].point.radius, 0, 2 * M_PI);
		cairo_stroke(ctx->cr);
	}
}

static void
kplotctx_draw_yerrline_pairpoints(struct kplotctx *ctx, 
	size_t start, size_t end, const struct kplotdat *d)
{
	size_t	 	i;
	struct kpair	orig, pair;
	int		rc;

	/* Now line above. */
	kplotctx_point_init(ctx, &d->cfgs[1].point);
	for (i = start; i < end; i++) {
		if ( ! (kpair_vrfy(&d->datas[0]->pairs[i]) &&
			kpair_vrfy(&d->datas[1]->pairs[i])))
			continue;
		orig.x = d->datas[0]->pairs[i].x;
		orig.y = d->datas[0]->pairs[i].y +
			 d->datas[1]->pairs[i].y;
		rc = kplotctx_point_to_real(&orig, &pair, ctx);
		assert(0 != rc);
		cairo_arc(ctx->cr, pair.x, pair.y,
			d->cfgs[1].point.radius, 0, 2 * M_PI);
		cairo_stroke(ctx->cr);
	}

	/* Now line above. */
	kplotctx_point_init(ctx, &d->cfgs[1].point);
	for (i = start; i < end; i++) {
		if ( ! (kpair_vrfy(&d->datas[0]->pairs[i]) &&
			kpair_vrfy(&d->datas[1]->pairs[i])))
			continue;
		orig.x = d->datas[0]->pairs[i].x;
		orig.y = d->datas[0]->pairs[i].y -
			 d->datas[1]->pairs[i].y;
		rc = kplotctx_point_to_real(&orig, &pair, ctx);
		assert(0 != rc);
		cairo_arc(ctx->cr, pair.x, pair.y,
			d->cfgs[1].point.radius, 0, 2 * M_PI);
		cairo_stroke(ctx->cr);
	}
}

static void
kplotctx_draw_yerrline_baselines(struct kplotctx *ctx, 
	size_t start, size_t end, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 pair;
	int		 rc;

	assert(d->datasz > 1);

	kplotctx_line_init(ctx, &d->cfgs[0].line);
	kplotctx_point_to_real
		(&d->datas[0]->pairs[start], &pair, ctx);
	cairo_move_to(ctx->cr, pair.x, pair.y);
	for (i = start; i < end; i++) {
		if ( ! (kpair_vrfy(&d->datas[0]->pairs[i]) &&
			kpair_vrfy(&d->datas[1]->pairs[i])))
			continue;
		rc = kplotctx_point_to_real
			(&d->datas[0]->pairs[i], &pair, ctx);
		assert(0 != rc);
		cairo_line_to(ctx->cr, pair.x, pair.y);
	}
	cairo_stroke(ctx->cr);
}

static void
kplotctx_draw_yerrline_pairlines(struct kplotctx *ctx, 
	size_t start, size_t end, const struct kplotdat *d)
{
	struct kpair	 orig, pair;
	size_t		 i;
	int		 rc;

	/* Now line above. */
	kplotctx_line_init(ctx, &d->cfgs[1].line);
	orig.x = d->datas[0]->pairs[start].x;
	orig.y = d->datas[0]->pairs[start].y +
		 d->datas[1]->pairs[start].y;
	kplotctx_point_to_real(&orig, &pair, ctx);
	cairo_move_to(ctx->cr, pair.x, pair.y);
	for (i = start; i < end; i++) {
		if ( ! (kpair_vrfy(&d->datas[0]->pairs[i]) &&
			kpair_vrfy(&d->datas[1]->pairs[i])))
			continue;
		orig.x = d->datas[0]->pairs[i].x;
		orig.y = d->datas[0]->pairs[i].y +
			 d->datas[1]->pairs[i].y;
		rc = kplotctx_point_to_real(&orig, &pair, ctx);
		assert(0 != rc);
		cairo_line_to(ctx->cr, pair.x, pair.y);
	}
	cairo_stroke(ctx->cr);

	/* Now line below. */
	kplotctx_line_init(ctx, &d->cfgs[1].line);
	orig.x = d->datas[0]->pairs[start].x;
	orig.y = d->datas[0]->pairs[start].y -
		 d->datas[1]->pairs[start].y;
	kplotctx_point_to_real(&orig, &pair, ctx);
	cairo_move_to(ctx->cr, pair.x, pair.y);
	for (i = start; i < end; i++) {
		if ( ! (kpair_vrfy(&d->datas[0]->pairs[i]) &&
			kpair_vrfy(&d->datas[1]->pairs[i])))
			continue;
		orig.x = d->datas[0]->pairs[i].x;
		orig.y = d->datas[0]->pairs[i].y -
			 d->datas[1]->pairs[i].y;
		rc = kplotctx_point_to_real(&orig, &pair, ctx);
		assert(0 != rc);
		cairo_line_to(ctx->cr, pair.x, pair.y);
	}
	cairo_stroke(ctx->cr);
}

static void
kplotctx_draw_lines(struct kplotctx *ctx, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 pair;
	int		 rc;

	/* Skip past bad points to get to initial. */
	for (i = 0; i < d->datas[0]->pairsz; i++) {
		rc = kplotctx_point_to_real
			(&d->datas[0]->pairs[i], &pair, ctx);
		if (rc > 0)
			break;
	}

	if (i == d->datas[0]->pairsz)
		return;

	/* Draw remaining points. */
	kplotctx_line_init(ctx, &d->cfgs[0].line);
	cairo_move_to(ctx->cr, pair.x, pair.y);
	for ( ; i < d->datas[0]->pairsz; i++) {
		rc = kplotctx_point_to_real
			(&d->datas[0]->pairs[i], &pair, ctx);
		if (0 == rc)
			continue;
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

	kplotctx_point_init(ctx, &d->cfgs[0].point);

	/* Draw only good points. */
	for (i = 0; i < d->datas[0]->pairsz; i++) {
		rc = kplotctx_point_to_real
			(&d->datas[0]->pairs[i], &pair, ctx);
		if (0 == rc)
			continue;
		cairo_arc(ctx->cr, pair.x, pair.y, 
			d->cfgs[0].point.radius, 0, 2 * M_PI);
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
	size_t	 	i, start, end;
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
		switch (p->datas[i].stype) {
		case (KPLOTS_YERRORLINE):
			kdata_extrema_yerr(&p->datas[i], 
				&ctx.minv, &ctx.maxv);
			break;
		default:
			kdata_extrema_single(&p->datas[i], 
				&ctx.minv, &ctx.maxv);
			break;
		}

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
		switch (p->datas[i].stype) {
		case (KPLOTS_SINGLE):
			switch (p->datas[i].types[0]) {
			case (KPLOT_POINTS):
				kplotctx_draw_points
					(&ctx, &p->datas[i]);
				break;
			case (KPLOT_LINES):
				kplotctx_draw_lines
					(&ctx, &p->datas[i]);
				break;
			}
			break;
		case (KPLOTS_YERRORLINE):
			start = kplotctx_draw_yerrline_start
				(&ctx, &p->datas[i], &end);
			if (start == end)
				break;
			assert(p->datas[i].datasz > 1);
			switch (p->datas[i].types[0]) {
			case (KPLOT_POINTS):
				kplotctx_draw_yerrline_basepoints
					(&ctx, start, end, &p->datas[i]);
				break;
			case (KPLOT_LINES):
				kplotctx_draw_yerrline_baselines
					(&ctx, start, end, &p->datas[i]);
				break;
			}
			switch (p->datas[i].types[1]) {
			case (KPLOT_POINTS):
				kplotctx_draw_yerrline_pairpoints
					(&ctx, start, end, &p->datas[i]);
				break;
			case (KPLOT_LINES):
				kplotctx_draw_yerrline_pairlines
					(&ctx, start, end, &p->datas[i]);
				break;
			}
			break;
		default:
			break;
		}
}
