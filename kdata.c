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
	cfg->pntsz = 5.0;
	cfg->lnsz = 1.0;
	cfg->clr = SIZE_MAX;
}
