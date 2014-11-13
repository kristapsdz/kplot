#ifndef EXTERN_H
#define EXTERN_H

struct	kdata {
	struct kpair	*pairs; /* data pairs */
	size_t		 pairsz; /* number of pairs */
	size_t		 pairmax; /* maximum (for buffering) */
	size_t		 refs; /* >0 references to data */
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

void	 kplotctx_label_init(struct kplotctx *);
void	 kplotctx_border_init(struct kplotctx *);
void	 kplotctx_margin_init(struct kplotctx *);
void	 kplotctx_tic_init(struct kplotctx *);

__END_DECLS

#endif
