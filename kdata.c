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
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kdata_destroy(struct kdata *d)
{
	size_t	 i;

	if (NULL == d)
		return;

	assert(d->refs > 0);
	if (--d->refs > 0)
		return;

	switch (d->type) {
	case (KDATA_MEAN):
		free(d->d.mean.ns);
		break;
	case (KDATA_STDDEV):
		free(d->d.stddev.ns);
		free(d->d.stddev.m1s);
		free(d->d.stddev.m2s);
		break;
	default:
		break;
	}

	/* Destroy dependeants along with ourselves. */
	for (i = 0; i < d->depsz; i++)
		kdata_destroy(d->deps[i].dep);

	free(d->deps);
	free(d->pairs);
	free(d);
}

ssize_t
kdata_max(const struct kdata *d, struct kpair *kp)
{
	size_t	 	i, max;
	struct kpair	pair;

	if (0 == d->pairsz)
		return(-1);

	max = 0;
	pair = d->pairs[max];
	for (i = 1; i < d->pairsz; i++) {
		if (pair.y > d->pairs[i].y)
			continue;
		pair = d->pairs[i];
		max = i;
	}
	if (NULL != kp)
		*kp = pair;
	return(max);
}

ssize_t
kdata_min(const struct kdata *d, struct kpair *kp)
{
	size_t	 	i, min;
	struct kpair	pair;

	if (0 == d->pairsz)
		return(-1);

	min = 0;
	pair = d->pairs[min];
	for (i = 1; i < d->pairsz; i++) {
		if (pair.y < d->pairs[i].y)
			continue;
		pair = d->pairs[i];
		min = i;
	}
	if (NULL != kp)
		*kp = pair;
	return(min);
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

/*
 * We've modified a value at (pair) position "pos".
 * Pass this through to the underlying functional sources, if any.
 */
int
kdata_dep_run(struct kdata *data, size_t pos)
{
	size_t	 i;
	int	 rc;
	double	 x, y;

	x = data->pairs[pos].x;
	y = data->pairs[pos].y;

	for (rc = 1, i = 0; 0 != rc && i < data->depsz; i++)
		rc = data->deps[i].func
			(data->deps[i].dep, pos, x, y);

	return(rc);
}

/*
 * Add a functional kdata source "data" (e.g., stddev) to another kdata
 * source "dep" as a dependent.
 * All a source's dependents are updated with each modification of the
 * source's internal pair values.
 */
int
kdata_dep_add(struct kdata *data, struct kdata *dep, ksetfunc fp)
{
	void	*p;

	p = reallocarray(dep->deps, 
		dep->depsz + 1, sizeof(struct kdep));
	if (NULL == p)
		return(0);
	dep->deps = p;
	dep->deps[dep->depsz].dep = data;
	dep->deps[dep->depsz].func = fp;
	dep->depsz++;

	/* While the parent exists, we must exist. */
	data->refs++;
	return(1);
}
