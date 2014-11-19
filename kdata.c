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

int
kdata_copy(const struct kdata *src, struct kdata *dst)
{
	void	*p;

	dst->d = src->d;
	dst->type = src->type;

	if (src->pairsz > dst->pairbufsz) {
		dst->pairbufsz = src->pairsz;
		p = reallocarray(dst->pairs, dst->pairbufsz, sizeof(struct kpair));
		if (NULL == p)
			return(0);
		dst->pairs = p;
	}

	dst->pairsz = src->pairsz;
	memcpy(dst->pairs, src->pairs, dst->pairsz * sizeof(struct kpair));
	return(1);
}

void
kdata_destroy(struct kdata *d)
{

	if (NULL == d)
		return;
	assert(d->refs > 0);
	if (--d->refs > 0)
		return;

	free(d->pairs);
	free(d);
}

void
kdatacfg_defaults(struct kdatacfg *cfg)
{

	memset(cfg, 0, sizeof(struct kdatacfg));
	cfg->point.radius = 3.0;
	cfg->point.sz = 2.0;
	cfg->point.clr = SIZE_MAX;
	cfg->line.sz = 2.0;
	cfg->line.join = CAIRO_LINE_JOIN_ROUND;
	cfg->line.clr = SIZE_MAX;
}
