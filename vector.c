#include <assert.h>
#include <cairo.h>
#include <stdlib.h>
#include <string.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

struct kdata *
kdata_vector_alloc(size_t step)
{
	struct kdata	*d;

	if (NULL == (d = calloc(1, sizeof(struct kdata))))
		return(NULL);

	d->refs = 1;
	d->type = KDATA_VECTOR;
	return(d);
}

int
kdata_vector_add(struct kdata *d, double x, double y)
{
	void	*p;

	assert(KDATA_VECTOR == d->type);

	if (d->pairsz + 1 >= d->d.vector.pairmax) {
		d->d.vector.pairmax += d->d.vector.stepsz;
		assert(d->d.vector.pairmax > d->pairsz + 1);
		p = reallocarray(d->pairs, 
			d->d.vector.pairmax, sizeof(struct kpair));
		if (NULL == p)
			return(0);
		d->pairs = p;
	}

	d->pairs[d->pairsz].x = x;
	d->pairs[d->pairsz].y = y;
	d->pairsz++;
	return(1);
}
