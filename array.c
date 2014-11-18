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

	if (NULL != new)
		memcpy(*p, new, newsz * sizeof(struct kpair));
	*oldsz = newsz;
	return(1);
}


struct kdata *
kdata_array_alloc(const struct kpair *np, size_t npsz)
{
	struct kdata	*d;

	if (NULL == (d = calloc(1, sizeof(struct kdata))))
		return(NULL);

	if ( ! paircpy(&d->pairs, np, npsz, 
		&d->pairsz, &d->pairbufsz)) {
		free(d);
		return(NULL);
	}

	d->refs = 1;
	d->type = KDATA_ARRAY;
	return(d);
}

void
kdata_array_fill(struct kdata *d, void *arg, 
	void (*fp)(size_t, struct kpair *, void *))
{
	size_t	 i;

	assert(KDATA_ARRAY == d->type);
	for (i = 0; i < d->pairsz; i++)
		(*fp)(i, &d->pairs[i], arg);
}

int
kdata_array_realloc(struct kdata *d, const struct kpair *np, size_t npsz)
{

	assert(KDATA_ARRAY == d->type);
	return(paircpy(&d->pairs, np, npsz, 
		&d->pairsz, &d->pairbufsz));
}
