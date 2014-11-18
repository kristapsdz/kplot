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
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

struct kplot *
kplot_alloc(void)
{

	return(calloc(1, sizeof(struct kplot)));
}

void
kplot_free(struct kplot *p)
{
	size_t	 i;

	if (NULL == p)
		return;

	for (i = 0; i < p->datasz; i++)
		kdata_destroy(p->datas[i].data);

	free(p->datas);
	free(p);
}

int
kplot_data(struct kplot *p, struct kdata *d, 
	enum kplottype t, const struct kdatacfg *cfg)
{
	void	*pp;

	pp = reallocarray(p->datas, 
		p->datasz + 1, sizeof(struct kplotdat));

	if (NULL == pp)
		return(0);

	p->datas = pp;
	memset(&p->datas[p->datasz].cfg, 0, sizeof(struct kdatacfg));
	p->datas[p->datasz].data = d;
	p->datas[p->datasz].type = t;

	if (NULL == cfg)
		kdatacfg_defaults(&p->datas[p->datasz].cfg);
	else
		p->datas[p->datasz].cfg = *cfg;

	if (SIZE_MAX == p->datas[p->datasz].cfg.clr)
		p->datas[p->datasz].cfg.clr = p->datasz;

	p->datasz++;
	d->refs++;
	return(1);
}

void
kplotctx_colour(struct kplotctx *ctx, size_t idx, struct kplotclr *res)
{

	if (0 == ctx->cfg.clrsz) {
		res->r = 0.0;
		res->g = 0.0;
		res->b = 0.0;
		res->a = 1.0;
	} else
		*res = ctx->cfg.clrs[idx % ctx->cfg.clrsz];
}

/*
 * Quick-ish test to see if we're an integer.
 */
static inline int 
border_integer(double n)
{

	return(n - floor(n) < DBL_EPSILON);
}

/*
 * Given a plotting context and a position for drawing a line, determine
 * whether we want to "fix" the line so that it's fine.
 * This is a foible of Cairo and drawing with doubles.
 */
double
kplotctx_line_fix(const struct kplotctx *ctx, double pos)
{
	double	 v;

	if (0 == (int)ctx->cfg.bordersz % 2)
		return(pos);
	v = pos - floor(pos);
	return(v < DBL_EPSILON ? pos + 0.5 : pos - v + 0.5);
}
