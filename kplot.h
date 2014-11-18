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
	size_t		  clr; /* colour index (SIZE_MAX for auto) */
	double		  pntsz; /* point arc radius */
	double		  pntlnsz; /* pen diameter for points */
	double		  lnsz; /* pen diameter for lines */
#define	EXTREMA_XMIN	  0x01
#define	EXTREMA_XMAX	  0x02
#define	EXTREMA_YMIN	  0x04
#define	EXTREMA_YMAX	  0x08
	unsigned int	  extrema; /* assumed bounds */
	double		  extrema_xmin;
	double		  extrema_xmax;
	double		  extrema_ymin;
	double		  extrema_ymax;
	cairo_line_join_t join;
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
	unsigned int	  border;
	size_t		  borderclr;
	size_t		  xtics;
	size_t		  ytics;
	double		  xticlabelrot;
	void		(*xticlabelfmt)(double, char *, size_t);
	void		(*yticlabelfmt)(double, char *, size_t);
	double		  ylabelpad;
	double		  xlabelpad;
#define	TICLABEL_LEFT	  0x01
#define	TICLABEL_RIGHT	  0x02
#define	TICLABEL_TOP	  0x04
#define	TICLABEL_BOTTOM	  0x08
	unsigned int	  ticlabel; /* labels to draw */
	size_t		  ticlabelclr; /* label colour index */
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
#define	GRID_X 		  0x01
#define GRID_Y 		  0x02
#define GRID_ALL 	  0x03
	unsigned int 	  grid;
	size_t	 	  gridclr;
	double	 	  gridsz;
};

__BEGIN_DECLS

struct kdata	*kdata_array_alloc(const struct kpair *, size_t);
void		 kdata_array_fill(struct kdata *, void *,
			void (*)(size_t, struct kpair *, void *));
int		 kdata_array_realloc(struct kdata *, 
			const struct kpair *, size_t);
struct kdata	*kdata_bucket_alloc(size_t, size_t);
int		 kdata_bucket_increment(struct kdata *, size_t);
int		 kdata_copy(const struct kdata *, struct kdata *);
void		 kdata_destroy(struct kdata *);
struct kdata	*kdata_hist_alloc(double, double, size_t);
int		 kdata_hist_increment(struct kdata *, double);
struct kdata	*kdata_vector_alloc(size_t);
int		 kdata_vector_add(struct kdata *, double, double);

void		 kdatacfg_defaults(struct kdatacfg *);
void		 kplotcfg_defaults(struct kplotcfg *);

struct kplot	*kplot_alloc(void);
int		 kplot_data(struct kplot *, struct kdata *, 
			enum kplottype, const struct kdatacfg *);
void		 kplot_draw(const struct kplot *, 
			double, double, cairo_t *, 
			const struct kplotcfg *);
void		 kplot_free(struct kplot *);

__END_DECLS

#endif
