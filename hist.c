#include <assert.h>
#include <cairo.h>
#include <stdlib.h>
#include <string.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

struct kdata *
kdata_hist_alloc(double rmin, double rmax, size_t bins)
{
	struct kdata	*d;
	size_t		 i;

	assert(rmax > rmin);

	if (NULL == (d = calloc(1, sizeof(struct kdata))))
		return(NULL);

	d->refs = 1;
	d->pairsz = d->pairbufsz = bins;
	d->pairs = calloc(d->pairsz, sizeof(struct kpair));
	if (NULL == d->pairs) {
		free(d);
		return(NULL);
	}

	for (i = 0; i < bins; i++) 
		d->pairs[i].x = rmin + 
			i / (double)bins * (rmax - rmin);

	d->type = KDATA_HIST;
	d->d.hist.rmin = rmin;
	d->d.hist.rmax = rmax;
	return(d);
}

int
kdata_hist_increment(struct kdata *d, double v)
{
	size_t	 bucket;
	double	 frac;

	assert(KDATA_HIST == d->type);

	if (v < d->d.hist.rmin)
		return(0);
	else if (v >= d->d.hist.rmax)
		return(0);

	frac = (v - d->d.hist.rmin) / 
		(d->d.hist.rmax - d->d.hist.rmin);

	assert(frac >= 0.0 && frac < 1.0);
	bucket = (size_t)(d->pairsz * frac);

	if (bucket == d->pairsz - 1) {
		assert(d->pairs[bucket].x <= frac);
	} else {
		assert(d->pairs[bucket].x <= frac);
		assert(d->pairs[bucket + 1].x > frac);
	}

	d->pairs[bucket].y++;
	return(1);
}
