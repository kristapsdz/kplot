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

struct	kplotclr {
	double		 r; /* [0,1] */
	double		 g; /* [0,1] */
	double		 b; /* [0,1] */
	double		 a; /* [0,1] */
};

struct 	kplotfont {
	cairo_font_slant_t   slant;
	cairo_font_weight_t  weight;
	const char	    *family;
	double		     sz;
	size_t		     clr;
};

struct	kplotticln {
	double		  sz;
	double		  len;
#define	KPLOT_DASH_MAX	  8
	double	  	  dashes[KPLOT_DASH_MAX];
	size_t		  dashesz;
	double	 	  dashoff;
	size_t		  clr;
};

struct	kplotpoint {
	double		  sz;
	double		  radius;
	double	  	  dashes[KPLOT_DASH_MAX];
	size_t		  dashesz;
	double	 	  dashoff;
	size_t		  clr;
};

struct	kplotline {
	double		  sz;
	double	  	  dashes[KPLOT_DASH_MAX];
	size_t		  dashesz;
	double	 	  dashoff;
	cairo_line_join_t join;
	size_t		  clr;
};

struct	kdatacfg {
	struct kplotline  line;
	struct kplotpoint point;
#define	EXTREMA_XMIN	  0x01
#define	EXTREMA_XMAX	  0x02
#define	EXTREMA_YMIN	  0x04
#define	EXTREMA_YMAX	  0x08
	unsigned int	  extrema; /* assumed bounds */
	double		  extrema_xmin;
	double		  extrema_xmax;
	double		  extrema_ymin;
	double		  extrema_ymax;
};

struct	kplotcfg {
#define	COLOURS_MAX	  10
	struct kplotclr	  clrs[COLOURS_MAX]; 
	size_t		  clrsz; 
	double		  marginsz; 
#define	MARGIN_LEFT	  0x01
#define	MARGIN_RIGHT	  0x02
#define	MARGIN_TOP	  0x04
#define	MARGIN_BOTTOM	  0x08
#define	MARGIN_ALL	  0xf
	unsigned int	  margin;
	struct kplotline  borderline;
	double		  bordersz;
#define	BORDER_LEFT	  0x01
#define	BORDER_RIGHT	  0x02
#define	BORDER_TOP	  0x04
#define	BORDER_BOTTOM	  0x08
#define	BORDER_ALL	  0xf
	unsigned int	  border;
	size_t		  borderclr;
	size_t		  xtics;
	size_t		  ytics;
	struct kplotticln ticline;
#define	TIC_LEFT_IN	  0x01
#define	TIC_LEFT_OUT	  0x02
#define	TIC_RIGHT_IN	  0x04
#define	TIC_RIGHT_OUT	  0x08
#define	TIC_TOP_IN	  0x10
#define	TIC_TOP_OUT	  0x20
#define	TIC_BOTTOM_IN	  0x40
#define	TIC_BOTTOM_OUT	  0x80
	unsigned int	  tic;
	double		  xticlabelrot;
	void		(*xticlabelfmt)(double, char *, size_t);
	void		(*yticlabelfmt)(double, char *, size_t);
	double		  yticlabelpad;
	double		  xticlabelpad;
	struct kplotfont  ticlabelfont;
#define	TICLABEL_LEFT	  0x01
#define	TICLABEL_RIGHT	  0x02
#define	TICLABEL_TOP	  0x04
#define	TICLABEL_BOTTOM	  0x08
	unsigned int	  ticlabel;
#define	GRID_X 		  0x01
#define GRID_Y 		  0x02
#define GRID_ALL 	  0x03
	unsigned int 	  grid;
	struct kplotline  gridline;
	double		  xaxislabelpad;
	double		  yaxislabelpad;
	const char	 *xaxislabel;
	const char	 *yaxislabel;
	struct kplotfont  axislabelfont;
	double		  xaxislabelrot;
	double		  yaxislabelrot;
};

struct 	kdata;

__BEGIN_DECLS

int		 kdata_array_add(struct kdata *, size_t, double);
struct kdata	*kdata_array_alloc(const struct kpair *, size_t);
int		 kdata_array_fill(struct kdata *, void *,
			void (*)(size_t, struct kpair *, void *));
int		 kdata_array_set(struct kdata *, size_t, double);
int		 kdata_bucket_add(struct kdata *, size_t, double);
struct kdata	*kdata_bucket_alloc(size_t, size_t);
int		 kdata_bucket_set(struct kdata *, size_t, double);
struct kdata	*kdata_buffer_alloc(void);
int		 kdata_buffer_copy(struct kdata *, const struct kdata *);
void		 kdata_destroy(struct kdata *);
int		 kdata_hist_add(struct kdata *, double, double);
struct kdata	*kdata_hist_alloc(double, double, size_t);
int		 kdata_hist_set(struct kdata *, double, double);
struct kdata	*kdata_mean_alloc(struct kdata *);
int		 kdata_mean_attach(struct kdata *, struct kdata *);
struct kdata	*kdata_vector_alloc(size_t);
int		 kdata_vector_append(struct kdata *, double, double);

void		 kdatacfg_defaults(struct kdatacfg *);
void		 kplotcfg_defaults(struct kplotcfg *);

struct kplot	*kplot_alloc(void);
int		 kplot_data_add(struct kplot *, struct kdata *, 
			enum kplottype, const struct kdatacfg *);
void		 kplot_data_remove_all(struct kplot *);
void		 kplot_draw(const struct kplot *, 
			double, double, cairo_t *, 
			const struct kplotcfg *);
void		 kplot_free(struct kplot *);

__END_DECLS

#endif
