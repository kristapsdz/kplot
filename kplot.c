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

static int
paircpy(struct kpair **p, const struct kpair *new, 
	size_t newsz, size_t *oldsz, size_t *oldmax)
{
	void	*pp;

	if (newsz > *oldmax) {
		*oldmax = newsz;
		pp = reallocarray(*p, *oldmax, sizeof(struct kpair));
		if (NULL == pp)
			return(0);
		*p = pp;
	}

	memcpy(*p, new, newsz * sizeof(struct kpair));
	*oldsz = newsz;
	return(1);
}

static void
kdata_ref(struct kdata *d)
{

	assert(d->refs > 0);
	d->refs++;
}

struct kdata *
kdata_alloc(void)
{
	struct kdata	*d;

	if (NULL == (d = calloc(1, sizeof(struct kdata))))
		return(NULL);

	d->refs = 1;
	return(d);
}

int
kdata_data(struct kdata *d, const struct kpair *np, size_t npsz)
{

	return(paircpy(&d->pairs, np, npsz, &d->pairsz, &d->pairmax));
}

void
kdata_unref(struct kdata *d)
{

	if (NULL == d)
		return;
	assert(d->refs > 0);
	if (--d->refs > 0)
		return;

	free(d->pairs);
	free(d);
}

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
		kdata_unref(p->datas[i].data);

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

	if (NULL == cfg) {
		p->datas[p->datasz].cfg.pntsz = 10.0;
		p->datas[p->datasz].cfg.lnsz = 1.0;
		p->datas[p->datasz].cfg.clr = p->datasz;
	} else
		p->datas[p->datasz].cfg = *cfg;

	p->datasz++;
	kdata_ref(d);
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
