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

static inline int
kpair_vrfy(const struct kpair *data)
{

	if (0.0 != data->x && ! isnormal(data->x))
		return(0);
	if (0.0 != data->y && ! isnormal(data->y))
		return(0);
	return(1);
}

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
		if ( ! (kpair_vrfy(&p[i]) && kpair_vrfy(&err[i])))
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
kpair_set(const struct kplotdat *d, size_t pos, struct kpair *kp)
{
	size_t		 j, sz, samps;
	ssize_t		 start;

	switch (d->smthtype) {
	case (KSMOOTH_CDF):
		kp->x = d->datas[0]->pairs[pos].x;
		kp->y += d->datas[0]->pairs[pos].y;
		break;
	case (KSMOOTH_MOVAVG):
		*kp = d->datas[0]->pairs[pos];
		samps = d->smth.movsamples / 2;
		start = pos - samps;
		sz = pos + samps;
		if (start < 0 || sz >= d->datas[0]->pairsz)
			break;
		for (kp->y = 0.0, j = start; j <= sz; j++) {
			if ( ! kpair_vrfy(&d->datas[0]->pairs[j]))
				break;
			kp->y += d->datas[0]->pairs[j].y;
		}
		kp->y /= (double)d->smth.movsamples;
		if (j <= sz)
			*kp = d->datas[0]->pairs[pos];
		break;
	default:
		*kp = d->datas[0]->pairs[pos];
		break;
	}
}

static void
kdata_extrema_single(const struct kplotdat *d, 
	struct kpair *minv, struct kpair *maxv)
{
	size_t	 	 i;
	struct kpair	 kp;

	memset(&kp, 0, sizeof(struct kpair));
	for (i = 0; i < d->datas[0]->pairsz; i++) {
		if ( ! kpair_vrfy(&d->datas[0]->pairs[i]))
			continue;
		kpair_set(d, i, &kp);
		if (kp.x < minv->x)
			minv->x = kp.x;
		if (kp.y < minv->y)
			minv->y = kp.y;
		if (kp.x > maxv->x)
			maxv->x = kp.x;
		if (kp.y > maxv->y)
			maxv->y = kp.y;
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
kplotctx_draw_yerrline_pairbars(struct kplotctx *ctx, 
	size_t start, size_t end, const struct kplotdat *d)
{
	size_t	 	i;
	struct kpair	bot, top, pair;
	int		rc;

	/* Now line above. */
	kplotctx_line_init(ctx, &d->cfgs[1].line);
	for (i = start; i < end; i++) {
		if ( ! (kpair_vrfy(&d->datas[0]->pairs[i]) &&
			kpair_vrfy(&d->datas[1]->pairs[i])))
			continue;

		bot.x = top.x = d->datas[0]->pairs[i].x;
		bot.y = d->datas[0]->pairs[i].y -
			 d->datas[1]->pairs[i].y;
		top.y = d->datas[0]->pairs[i].y +
			 d->datas[1]->pairs[i].y;

		rc = kplotctx_point_to_real(&bot, &pair, ctx);
		assert(0 != rc);
		cairo_move_to(ctx->cr, pair.x, pair.y);

		rc = kplotctx_point_to_real(&top, &pair, ctx);
		assert(0 != rc);
		cairo_line_to(ctx->cr, pair.x, pair.y);
	}

	cairo_stroke(ctx->cr);
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
	struct kpair	 kp, pair;
	int		 rc;

	memset(&kp, 0, sizeof(struct kpair));
	for (i = 0; i < d->datas[0]->pairsz; i++) {
		kpair_set(d, i, &kp);
		rc = kplotctx_point_to_real(&kp, &pair, ctx);
		if (rc > 0)
			break;
	}

	if (i == d->datas[0]->pairsz)
		return;

	kplotctx_line_init(ctx, &d->cfgs[0].line);
	cairo_move_to(ctx->cr, pair.x, pair.y);
	memset(&kp, 0, sizeof(struct kpair));
	for ( ; i < d->datas[0]->pairsz; i++) {
		if ( ! kpair_vrfy(&d->datas[0]->pairs[i]))
			continue;
		kpair_set(d, i, &kp);
		rc = kplotctx_point_to_real(&kp, &pair, ctx);
		if ( ! rc)
			continue;
		cairo_line_to(ctx->cr, pair.x, pair.y);
	}
	cairo_stroke(ctx->cr);
}

static void
kplotctx_draw_points(struct kplotctx *ctx, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 kp, pair;
	int		 rc;

	memset(&kp, 0, sizeof(struct kpair));
	kplotctx_point_init(ctx, &d->cfgs[0].point);
	for (i = 0; i < d->datas[0]->pairsz; i++) {
		if ( ! kpair_vrfy(&d->datas[0]->pairs[i]))
			continue;
		kpair_set(d, i, &kp);
		rc = kplotctx_point_to_real(&kp, &pair, ctx);
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

	memset(font, 0, sizeof(struct kplotfont));

	font->family = "serif";
	font->sz = 12.0;
	font->slant = CAIRO_FONT_SLANT_NORMAL;
	font->weight = CAIRO_FONT_WEIGHT_NORMAL;
}

void
kplotcfg_defaults(struct kplotcfg *cfg)
{

	memset(cfg, 0, sizeof(struct kplotcfg));

	/* Five left and bottom grey tic labels. */
	kplotfont_defaults(&cfg->ticlabelfont);
	cfg->ticlabel = TICLABEL_LEFT | TICLABEL_BOTTOM;
	cfg->xticlabelpad = cfg->yticlabelpad = 15.0;
	cfg->xtics = cfg->ytics = 5;

	/* A bit of margin. */
	cfg->margin = MARGIN_ALL;
	cfg->marginsz = 15.0;
	
	/* Innie tics, grey. */
	cfg->tic = TIC_LEFT_IN | TIC_BOTTOM_IN;
	cfg->ticline.len = 5.0;
	cfg->ticline.sz = 1.0;

	/* Grid line: dotted, grey. */
	cfg->grid = GRID_ALL;
	cfg->gridline.sz = 1.0;
	cfg->gridline.dashes[0] = 1.0;
	cfg->gridline.dashes[1] = 4.0;
	cfg->gridline.dashesz = 2;

	/* Border line: solid, grey. */
	cfg->border = BORDER_LEFT | BORDER_BOTTOM;
	cfg->borderline.sz = 1.0;

	/* Black axis labels. */
	kplotfont_defaults(&cfg->axislabelfont);
	cfg->xaxislabelpad = cfg->yaxislabelpad = 15.0;
}

int
kplot_draw(const struct kplot *p, double w, 
	double h, cairo_t *cr, const struct kplotcfg *cfg)
{
	size_t	 	 i, start, end;
	double		 ux, uy;
	struct kplotctx	 ctx;
	cairo_surface_t	*surf;
	cairo_status_t	 st;
	cairo_t		*subcr;
	struct kplotdat	*d;
	int		 rc;

	memset(&ctx, 0, sizeof(struct kplotctx));

	subcr = NULL;
	surf = NULL;
	ctx.w = w;
	ctx.h = h;
	ctx.cr = cr;
	ctx.minv.x = ctx.minv.y = DBL_MAX;
	ctx.maxv.x = ctx.maxv.y = -DBL_MAX;

	if (NULL == cfg)
		kplotcfg_defaults(&ctx.cfg);
	else 
		ctx.cfg = *cfg;

	if (0 == ctx.cfg.clrsz) {
		ctx.cfg.clrsz = 7;
		ctx.cfg.clrs = calloc
			(ctx.cfg.clrsz, 
			 sizeof(cairo_pattern_t *));
		if (NULL == ctx.cfg.clrs)
			return(0);
		ctx.cfg.clrs[0] = cairo_pattern_create_rgb
			(0x94 / 255.0, 0x00 / 255.0, 0xd3 / 255.0);
		ctx.cfg.clrs[1] = cairo_pattern_create_rgb
			(0x00 / 255.0, 0x9e / 255.0, 0x73 / 255.0);
		ctx.cfg.clrs[2] = cairo_pattern_create_rgb
			(0x56 / 255.0, 0xb4 / 255.0, 0xe9 / 255.0);
		ctx.cfg.clrs[3] = cairo_pattern_create_rgb
			(0xe6 / 255.0, 0x9f / 255.0, 0x00 / 255.0);
		ctx.cfg.clrs[4] = cairo_pattern_create_rgb
			(0xf0 / 255.0, 0xe4 / 255.0, 0x42 / 255.0);
		ctx.cfg.clrs[5] = cairo_pattern_create_rgb
			(0x00 / 255.0, 0x72 / 255.0, 0xb2 / 255.0);
		ctx.cfg.clrs[6] = cairo_pattern_create_rgb
			(0xe5 / 255.0, 0x1e / 255.0, 0x10 / 255.0);
		for (i = 0; i < ctx.cfg.clrsz; i++) {
			st = cairo_pattern_status(ctx.cfg.clrs[i]);
			if (CAIRO_STATUS_SUCCESS != st)
				goto out;
		}
	} else 
		for (i = 0; i < ctx.cfg.clrsz; i++)
			cairo_pattern_reference(ctx.cfg.clrs[i]);

	rc = kplotccfg_init_rgb
		(&ctx.cfg.borderline.clr, 0.0, 0.0, 0.0);
	if ( ! rc)
		goto out;

	rc = kplotccfg_init_rgb
		(&ctx.cfg.ticline.clr, 0.0, 0.0, 0.0);
	if ( ! rc)
		goto out;

	rc = kplotccfg_init_rgb
		(&ctx.cfg.gridline.clr, 0.5, 0.5, 0.5);
	if ( ! rc)
		goto out;

	rc = kplotccfg_init_rgb
		(&ctx.cfg.ticlabelfont.clr, 0.5, 0.5, 0.5);
	if ( ! rc)
		goto out;

	rc = kplotccfg_init_rgb
		(&ctx.cfg.axislabelfont.clr, 0.0, 0.0, 0.0);
	if ( ! rc)
		goto out;

	for (i = 0; i < p->datasz; i++) {
		d = &p->datas[i];
		switch (d->stype) {
		case (KPLOTS_YERRORBAR):
		case (KPLOTS_YERRORLINE):
			kdata_extrema_yerr(d, &ctx.minv, &ctx.maxv);
			break;
		case (KPLOTS_SINGLE):
			kdata_extrema_single(d, &ctx.minv, &ctx.maxv);
			break;
		}
	}

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
	/*
	 * Here we create a subwindow.
	 * This is important because we can arbitrarily set window
	 * minima and maxima and then scribble outside of the window.
	 * This makes sure that we don't do that: we'll scribble into
	 * nowhere.
	 */
	/* GTK will have its surface be much bigger than the cr. */
	ux = uy = 0.0;
	cairo_user_to_device(cr, &ux, &uy);
	surf = cairo_surface_create_for_rectangle
		(cairo_get_target(cr),
		 ux, uy, ctx.dims.x, ctx.dims.y + 2.0);
	st = cairo_surface_status(surf);
	if (CAIRO_STATUS_SUCCESS != st)
		goto out;
	subcr = ctx.cr = cairo_create(surf);
	cairo_surface_destroy(surf);
	surf = NULL;
	st = cairo_status(ctx.cr);
	if (CAIRO_STATUS_SUCCESS != st)
		goto out;

	ctx.h = ctx.dims.y;
	ctx.w = ctx.dims.x;

	for (i = 0; i < p->datasz; i++) {
		d = &p->datas[i];
		switch (d->stype) {
		case (KPLOTS_SINGLE):
			switch (d->types[0]) {
			case (KPLOT_POINTS):
				kplotctx_draw_points(&ctx, d);
				break;
			case (KPLOT_LINES):
				kplotctx_draw_lines(&ctx, d);
				break;
			case (KPLOT_LINESPOINTS):
				kplotctx_draw_lines(&ctx, d);
				kplotctx_draw_points(&ctx, d);
				break;
			default:
				abort();
				break;
			}
			break;
		case (KPLOTS_YERRORBAR):
		case (KPLOTS_YERRORLINE):
			start = kplotctx_draw_yerrline_start
				(&ctx, d, &end);
			if (start == end)
				break;
			assert(d->datasz > 1);
			switch (d->types[0]) {
			case (KPLOT_POINTS):
				kplotctx_draw_yerrline_basepoints
					(&ctx, start, end, d);
				break;
			case (KPLOT_LINES):
				kplotctx_draw_yerrline_baselines
					(&ctx, start, end, d);
				break;
			case (KPLOT_LINESPOINTS):
				kplotctx_draw_yerrline_baselines
					(&ctx, start, end, d);
				kplotctx_draw_yerrline_basepoints
					(&ctx, start, end, d);
				break;
			default:
				abort();
				break;
			}
			switch (p->datas[i].types[1]) {
			case (KPLOT_POINTS):
				kplotctx_draw_yerrline_pairpoints
					(&ctx, start, end, d);
				break;
			case (KPLOT_LINES):
				kplotctx_draw_yerrline_pairlines
					(&ctx, start, end, d);
				break;
			case (KPLOT_LINESPOINTS):
				kplotctx_draw_yerrline_pairlines
					(&ctx, start, end, d);
				kplotctx_draw_yerrline_pairpoints
					(&ctx, start, end, d);
				break;
			default:
				abort();
				break;
			}
			if (KPLOTS_YERRORBAR == d->stype)
				kplotctx_draw_yerrline_pairbars
					(&ctx, start, end, d);
			break;
		default:
			break;
		}
	}

	rc = 1;
out:
	if (NULL != subcr)
		cairo_destroy(subcr);
	if (NULL != surf)
		cairo_surface_destroy(surf);
	kplotccfg_destroy(&ctx.cfg.borderline.clr);
	kplotccfg_destroy(&ctx.cfg.ticline.clr);
	kplotccfg_destroy(&ctx.cfg.gridline.clr);
	kplotccfg_destroy(&ctx.cfg.ticlabelfont.clr);
	kplotccfg_destroy(&ctx.cfg.axislabelfont.clr);
	for (i = 0; i < ctx.cfg.clrsz; i++)
		cairo_pattern_destroy(ctx.cfg.clrs[i]);
	return(rc);
}
