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
#include <stdlib.h>
#include <string.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

static void
kplotdat_free(struct kplotdat *p)
{
	size_t	 i;

	if (NULL == p)
		return;

	for (i = 0; i < p->datasz; i++)
		kdata_destroy(p->datas[i]);

	free(p->datas);
	free(p->cfgs);
	free(p->types);
}

struct kplot *
kplot_alloc(void)
{

	return(calloc(1, sizeof(struct kplot)));
}

void
kplot_free(struct kplot *p)
{

	if (NULL == p)
		return;

	kplot_data_remove_all(p);
	free(p->datas);
	free(p);
}

void
ksmthcfg_defaults(struct ksmthcfg *p)
{

	p->movsamples = 3;
}

void
kplot_data_remove_all(struct kplot *p)
{
	size_t	 i;

	for (i = 0; i < p->datasz; i++)
		kplotdat_free(&p->datas[i]);

	free(p->datas);
	p->datas = NULL;
	p->datasz = 0;
}

static int
kplotdat_attach(struct kplot *p, size_t sz, struct kdata **d, 
	const struct kdatacfg *const *cfg,
	const enum kplottype *types, enum kplotstype stype, 
	enum ksmthtype smthtype, const struct ksmthcfg *smth)
{
	void	*pp;
	size_t	 i;

	pp = reallocarray(p->datas, 
		p->datasz + 1, sizeof(struct kplotdat));
	if (NULL == pp)
		return(0);
	p->datas = pp;

	p->datas[p->datasz].datas = 
		calloc(sz, sizeof(struct kdata *));
	if (NULL == p->datas[p->datasz].datas)
		return(0);
	p->datas[p->datasz].cfgs = 
		calloc(sz, sizeof(struct kdatacfg));
	if (NULL == p->datas[p->datasz].cfgs)
		return(0);
	p->datas[p->datasz].types = 
		calloc(sz, sizeof(enum kplottype));
	if (NULL == p->datas[p->datasz].types)
		return(0);

	for (i = 0; i < sz; i++) {
		p->datas[p->datasz].datas[i] = d[i];
		p->datas[p->datasz].types[i] = types[i];
		if (NULL == cfg || NULL == cfg[i])
			kdatacfg_defaults(&p->datas[p->datasz].cfgs[i]);
		else
			p->datas[p->datasz].cfgs[i] = *cfg[i];
		if (SIZE_MAX == p->datas[p->datasz].cfgs[i].point.clr)
			p->datas[p->datasz].cfgs[i].point.clr = p->datasz;
		if (SIZE_MAX == p->datas[p->datasz].cfgs[i].line.clr)
			p->datas[p->datasz].cfgs[i].line.clr = p->datasz;
		d[i]->refs++;
	}

	p->datas[p->datasz].smthtype = smthtype;
	if (NULL != smth)  {
		p->datas[p->datasz].smth = *smth;
		/* Make sure we're odd around the sample. */
		if (0 == (2 % p->datas[p->datasz].smth.movsamples))
			p->datas[p->datasz].smth.movsamples++;
	} else
		ksmthcfg_defaults(&p->datas[p->datasz].smth);
	p->datas[p->datasz].datasz = sz;
	p->datas[p->datasz].stype = stype;
	p->datasz++;
	return(1);
}

int
kplot_smthdata_attach(struct kplot *p, struct kdata *d, 
	enum kplottype t, const struct kdatacfg *cfg,
	enum ksmthtype smthtype, const struct ksmthcfg *smth)
{

	return(kplotdat_attach(p, 1, &d, &cfg, 
		&t, KPLOTS_SINGLE, smthtype, smth));
}

int
kplot_data_attach(struct kplot *p, struct kdata *d, 
	enum kplottype t, const struct kdatacfg *cfg)
{

	return(kplotdat_attach(p, 1, &d, &cfg, 
		&t, KPLOTS_SINGLE, KSMOOTH_NONE, NULL));
}

int
kplot_datas_attach(struct kplot *p, size_t sz, 
	struct kdata **d, const enum kplottype *t, 
	const struct kdatacfg *const *cfg, enum kplotstype st)
{

	if (sz < 2)
		return(0);
	return(kplotdat_attach(p, sz, d, 
		cfg, t, st, KSMOOTH_NONE, NULL));
}
