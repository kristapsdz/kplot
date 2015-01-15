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
#ifndef EXTERN_H
#define EXTERN_H

struct	kdatahist {
	double		 rmin;
	double		 rmax;
};

struct	kdatabucket {
	size_t		 rmin;
	size_t		 rmax;
};

struct	kdatamean {
	size_t		*ns;
};

struct	kdatavector {
	size_t		 stepsz;
};

enum	kdatatype {
	KDATA_ARRAY,
	KDATA_BUCKET,
	KDATA_BUFFER,
	KDATA_HIST,
	KDATA_MEAN,
	KDATA_VECTOR
};

typedef	int (*ksetfunc)(struct kdata *, size_t, double, double);

struct	kdep {
	struct kdata	 *dep;
	ksetfunc	  func;
};

struct	kdata {
	struct kpair	*pairs; /* data pairs */
	size_t		 pairsz; /* number of pairs */
	size_t		 pairbufsz; /* allocated buffer size */
	size_t		 refs; /* >0 references to data */
	struct kdep	*deps; /* dependants */
	size_t		 depsz; /* number of dependants */
	enum kdatatype	 type;
	union {
		struct kdatahist	hist;
		struct kdatavector	vector;
		struct kdatabucket	bucket;
		struct kdatamean	mean;
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

int	 kdata_dep_add(struct kdata *, struct kdata *, ksetfunc);
int	 kdata_dep_run(struct kdata *, size_t);

void	 kplotctx_border_init(struct kplotctx *);
void	 kplotctx_grid_init(struct kplotctx *);
void	 kplotctx_margin_init(struct kplotctx *);
void	 kplotctx_tic_init(struct kplotctx *);
void	 kplotctx_label_init(struct kplotctx *);

double	 kplotctx_line_fix(const struct kplotctx *, double, double);

void	 kplotctx_colour_init(struct kplotctx *, 
		size_t, struct kplotclr *);
void	 kplotctx_font_init(struct kplotctx *, 
		const struct kplotfont *);
void	 kplotctx_line_init(struct kplotctx *, 
		const struct kplotline *);
void	 kplotctx_point_init(struct kplotctx *, 
		const struct kplotpoint *);
void	 kplotctx_ticln_init(struct kplotctx *, 
		const struct kplotticln *);

__END_DECLS

#endif
