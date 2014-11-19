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

void
kplot_data_remove_all(struct kplot *p)
{
	size_t	 i;

	for (i = 0; i < p->datasz; i++)
		kdata_destroy(p->datas[i].data);
	p->datasz = 0;
}

int
kplot_data_add(struct kplot *p, struct kdata *d, 
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

	if (SIZE_MAX == p->datas[p->datasz].cfg.point.clr)
		p->datas[p->datasz].cfg.point.clr = p->datasz;
	if (SIZE_MAX == p->datas[p->datasz].cfg.line.clr)
		p->datas[p->datasz].cfg.line.clr = p->datasz;

	p->datasz++;
	d->refs++;
	return(1);
}

