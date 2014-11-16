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
	d->d.vector.stepsz = step;
	return(d);
}

int
kdata_vector_add(struct kdata *d, double x, double y)
{
	void	*p;

	assert(KDATA_VECTOR == d->type);

	if (d->pairsz + 1 >= d->pairbufsz) {
		d->pairbufsz += d->d.vector.stepsz;
		assert(d->pairbufsz > d->pairsz + 1);
		p = reallocarray(d->pairs, 
			d->pairbufsz, sizeof(struct kpair));
		if (NULL == p)
			return(0);
		d->pairs = p;
	}

	d->pairs[d->pairsz].x = x;
	d->pairs[d->pairsz].y = y;
	d->pairsz++;
	return(1);
}
