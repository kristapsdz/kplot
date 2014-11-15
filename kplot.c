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

static void
kdata_ref(struct kdata *d)
{

	assert(d->refs > 0);
	d->refs++;
}

int
kdata_copy(const struct kdata *src, struct kdata *dst)
{
	void	*p;

	dst->d = src->d;
	dst->type = src->type;

	if (dst->pairsz != src->pairsz) {
		dst->pairsz = src->pairsz;
		p = reallocarray(dst->pairs, dst->pairsz, sizeof(struct kpair));
		if (NULL == p)
			return(0);
		memcpy(dst->pairs, src->pairs, dst->pairsz * sizeof(struct kpair));
	}

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
kdatacfg_defaults(struct kdatacfg *cfg)
{

	memset(cfg, 0, sizeof(struct kdatacfg));
	cfg->pntsz = 5.0;
	cfg->lnsz = 1.0;
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
		kdatacfg_defaults(&p->datas[p->datasz].cfg);
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
