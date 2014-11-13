#ifndef KPLOT_H
#define KPLOT_H

struct 	kpair {
	double	 x;
	double	 y;
};

enum	kplottype {
	KPLOT_POINTS, /* set of points */
	KPLOT_LINES /* data points joined by lines */
};

struct	kdatacfg {
	size_t		 clr; /* colour index */
	/*
	 * All of these dimensions are in user-space coordinate system
	 * defined by cairo.
	 */
	double		 pntsz; /* point arc radius */
	double		 lnsz; /* pen diameter */
};

struct	kplotclr {
	double		 r; /* [0,1] */
	double		 g; /* [0,1] */
	double		 b; /* [0,1] */
	double		 a; /* [0,1] */
};

struct	kplotcfg {
	double		  marginsz; /* margin dimensions */
#define	MARGIN_LEFT	  0x01
#define	MARGIN_RIGHT	  0x02
#define	MARGIN_TOP	  0x04
#define	MARGIN_BOTTOM	  0x08
#define	MARGIN_ALL	  0xf
	unsigned int	  margin; /* margin bitfield */
	double		  bordersz; /* border diameter */
#define	BORDER_LEFT	  0x01
#define	BORDER_RIGHT	  0x02
#define	BORDER_TOP	  0x04
#define	BORDER_BOTTOM	  0x08
#define	BORDER_ALL	  0xf
	unsigned int	  border; /* borders to draw */
	size_t		  borderclr; /* border colour index */
	size_t		  xtics; /* number of labels on x-axis */
	size_t		  ytics; /* number of labels on y-axis */
	double		  xrot; /* x-axis rotation */
	void		(*xformat)(double, char *, size_t);
	void		(*yformat)(double, char *, size_t);
	double		  ylabelpad; /* label padding */
	double		  xlabelpad; /* label padding */
#define	LABEL_LEFT	  0x01
#define	LABEL_RIGHT	  0x02
#define	LABEL_TOP	  0x04
#define	LABEL_BOTTOM	  0x08
	unsigned int	  label; /* labels to draw */
	size_t		  labelclr; /* label colour index */
#define	COLOURS_MAX	  10
	struct kplotclr	  clrs[COLOURS_MAX]; 
	size_t		  clrsz; 
#define	TIC_LEFT_IN	  0x01
#define	TIC_LEFT_OUT	  0x02
#define	TIC_RIGHT_IN	  0x04
#define	TIC_RIGHT_OUT	  0x08
#define	TIC_TOP_IN	  0x10
#define	TIC_TOP_OUT	  0x20
#define	TIC_BOTTOM_IN	  0x40
#define	TIC_BOTTOM_OUT	  0x80
	unsigned int	  tic;
	size_t		  ticclr;
	double		  ticlen;
	double		  ticsz;
};

__BEGIN_DECLS

struct kdata	*kdata_alloc(const struct kpair *, size_t);
int		 kdata_realloc(struct kdata *, 
			const struct kpair *, size_t);
void		 kdata_destroy(struct kdata *);
void		 kdata_fill(struct kdata *, void *,
			void (*)(size_t, struct kpair *, void *));

void		 kdatacfg_defaults(struct kdatacfg *);

struct kplot	*kplot_alloc(void);
int		 kplot_data(struct kplot *, struct kdata *, 
			enum kplottype, const struct kdatacfg *);
void		 kplot_free(struct kplot *);

void		 kplotcfg_defaults(struct kplotcfg *);

void		 kplot_draw(const struct kplot *, 
			double, double, cairo_t *, 
			const struct kplotcfg *);

__END_DECLS

#endif
