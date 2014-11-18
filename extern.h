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
#ifndef EXTERN_H
#define EXTERN_H

struct	kdatahist {
	double		 rmin; /* minimum bucket value */
	double		 rmax; /* maximum bucket value */
};

struct	kdataarray {
	/* Empty. */
};

struct	kdatabucket {
	size_t		 rmin;
	size_t		 rmax;
};

struct	kdatavector {
	size_t		 stepsz; /* reallocation step */
};

enum	kdatatype {
	KDATA_HIST,
	KDATA_ARRAY,
	KDATA_VECTOR,
	KDATA_BUCKET
};

struct	kdata {
	struct kpair	*pairs; /* data pairs */
	size_t		 pairsz; /* number of pairs */
	size_t		 pairbufsz; /* allocated buffer size */
	size_t		 refs; /* >0 references to data */
	enum kdatatype	 type;
	union {
		struct kdatahist	hist;
		struct kdataarray	array;
		struct kdatavector	vector;
		struct kdatabucket	bucket;
	} d;
};

struct	kplotdat {
	struct kdata	*data; /* referenced data */
	struct kdatacfg	 cfg; /* plot configuration */
	enum kplottype	 type; /* plot type */
};

struct	kplot {
	struct kplotdat	*datas; /* data sets per plot */
	size_t		 datasz; /* number of data sets */
};

struct	kplotctx {
	cairo_t		*cr; /* cairo context */
	double		 h; /* height of context */
	double		 w; /* width of context */
	struct kpair	 minv; /* minimum data point values */
	struct kpair	 maxv; /* maximum data point values */
	struct kplotcfg	 cfg; /* configuration */

	/*
	 * When computing the plot context, we need to account for a
	 * margin, labels, and boundary.
	 * To do this, we use these "soft" offset and dimensions.
	 * Once we've accounted for the above, we'll use this to
	 * translate and resize the Cairo context for graphing.
	 */
	struct kpair	 offs;
	struct kpair	 dims;
};

__BEGIN_DECLS

void	 kplotctx_colour(struct kplotctx *, size_t, struct kplotclr *);

void	 kplotctx_border_init(struct kplotctx *);
void	 kplotctx_grid_init(struct kplotctx *);
void	 kplotctx_margin_init(struct kplotctx *);
void	 kplotctx_tic_init(struct kplotctx *);
void	 kplotctx_label_init(struct kplotctx *);

double	 kplotctx_line_fix(const struct kplotctx *, double);

__END_DECLS

#endif
