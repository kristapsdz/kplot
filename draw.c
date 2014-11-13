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

static void
kpair_max(const struct kpair *p, size_t sz, struct kpair *maxv)
{
	size_t	 	i;

	for (i = 0; i < sz; i++) {
		if (0.0 != p[i].x && ! isnormal(p[i].x))
			continue;
		if (0.0 != p[i].y && ! isnormal(p[i].y))
			continue;
		if (p[i].x > maxv->x)
			maxv->x = p[i].x;
		if (p[i].y > maxv->y)
			maxv->y = p[i].y;
	}
}

static void
kdata_max(const struct kdata *d, struct kpair *maxv)
{

	kpair_max(d->pairs, d->pairsz, maxv);
}

static void
kpair_min(const struct kpair *p, size_t sz, struct kpair *minv)
{
	size_t	 	i;

	for (i = 0; i < sz; i++) {
		if (0.0 != p[i].x && ! isnormal(p[i].x))
			continue;
		if (0.0 != p[i].y && ! isnormal(p[i].y))
			continue;
		if (p[i].x < minv->x)
			minv->x = p[i].x;
		if (p[i].y < minv->y)
			minv->y = p[i].y;
	}
}

static void
kdata_min(const struct kdata *d, struct kpair *minv)
{

	kpair_min(d->pairs, d->pairsz, minv);
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

static inline void
kplotctx_point_to_real(const struct kpair *data, 
	struct kpair *real, const struct kplotctx *ctx)
{

	kpoint_to_real(data, real, &ctx->minv, &ctx->maxv, ctx->w, ctx->h);
}

static void
kplotctx_draw_init(struct kplotctx *ctx, const struct kplotdat *d)
{
	struct kplotclr	 clr;

	cairo_set_line_width(ctx->cr, d->cfg.lnsz);
	kplotctx_colour(ctx, d->cfg.clr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
}

static void
kplotctx_draw_lines(struct kplotctx *ctx, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 pair1, pair2;

	kplotctx_draw_init(ctx, d);

	for (i = 1; i < d->data->pairsz; i++) {
		kplotctx_point_to_real
			(&d->data->pairs[i - 1], &pair1, ctx);
		kplotctx_point_to_real
			(&d->data->pairs[i], &pair2, ctx);
		cairo_move_to(ctx->cr, pair1.x, pair1.y);
		cairo_line_to(ctx->cr, pair2.x, pair2.y);
		cairo_stroke(ctx->cr);
	}
}

static void
kplotctx_draw_points(struct kplotctx *ctx, const struct kplotdat *d)
{
	size_t		 i;
	struct kpair	 pair;

	kplotctx_draw_init(ctx, d);

	for (i = 0; i < d->data->pairsz; i++) {
		kplotctx_point_to_real(&d->data->pairs[i], &pair, ctx);
		cairo_arc(ctx->cr, pair.x, pair.y, d->cfg.pntsz, 0, 2 * M_PI);
		cairo_stroke(ctx->cr);
	}
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

	if (NULL == cfg) {
		ctx.cfg.border = BORDER_ALL;
		ctx.cfg.bordersz = 1.0;
		ctx.cfg.label = LABEL_LEFT | LABEL_BOTTOM;
		ctx.cfg.xtics = ctx.cfg.ytics = 5;
		ctx.cfg.margin = MARGIN_ALL;
		ctx.cfg.marginsz = 10.0;
		ctx.cfg.xlabelpad = ctx.cfg.ylabelpad = 10.0;
		ctx.cfg.clrsz = 5;
		ctx.cfg.clrs[0].r = 1.0;
		ctx.cfg.clrs[0].a = 1.0;
		ctx.cfg.clrs[1].g = 1.0;
		ctx.cfg.clrs[1].a = 1.0;
		ctx.cfg.clrs[2].b = 1.0;
		ctx.cfg.clrs[2].a = 1.0;
		ctx.cfg.clrs[3].a = 1.0;
		ctx.cfg.clrs[4].r = 0.5;
		ctx.cfg.clrs[4].g = 0.5;
		ctx.cfg.clrs[4].b = 0.5;
		ctx.cfg.clrs[4].a = 1.0;
		ctx.cfg.labelclr = 3;
		ctx.cfg.borderclr = 4;
		ctx.cfg.tic = TIC_LEFT_OUT | TIC_BOTTOM_OUT;
		ctx.cfg.ticclr = 3;
		ctx.cfg.ticlen = 5.0;
		ctx.cfg.ticsz = 1.0;
	} else {
		ctx.cfg = *cfg;
		if (ctx.cfg.marginsz < 0.0)
			ctx.cfg.marginsz = 0.0;
		if (ctx.cfg.bordersz < 0.0)
			ctx.cfg.bordersz = 0.0;
		if (ctx.cfg.ylabelpad < 0.0)
			ctx.cfg.ylabelpad = 0.0;
		if (ctx.cfg.xlabelpad < 0.0)
			ctx.cfg.xlabelpad = 0.0;
		if (ctx.cfg.xrot < 0.0)
			ctx.cfg.xrot = 0.0;
		if (ctx.cfg.xrot > M_PI_2)
			ctx.cfg.xrot = M_PI_2;
		if (ctx.cfg.ticlen < 0.0)
			ctx.cfg.ticlen = 0.0;
		if (ctx.cfg.ticsz < 0.0)
			ctx.cfg.ticsz = 0.0;
	}

	for (i = 0; i < p->datasz; i++) {
		kdata_min(p->datas[i].data, &ctx.minv);
		kdata_max(p->datas[i].data, &ctx.maxv);
	}

	kplotctx_margin_init(&ctx);
	kplotctx_label_init(&ctx);
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

