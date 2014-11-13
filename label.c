#include <assert.h>
#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kplotctx_label_init(struct kplotctx *ctx)
{
	char		buf[64];
	size_t		i;
	cairo_text_extents_t e;
	double		maxh, maxw, offs;
	struct kplotclr	clr;

	kplotctx_colour(ctx, ctx->cfg.labelclr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_line_width(ctx->cr, ctx->cfg.bordersz);

	/*
	 * First, acquire the maximum space that will be required for
	 * the vertical (left or right) or horizontal (top or bottom)
	 * labels.
	 * If we're rotating, make sure that we accomodate for the
	 * maximum rotated box.
	 */
	for (maxh = 0.0, i = 0; i < ctx->cfg.xtics; i++) {
		offs = 1 == ctx->cfg.xtics ? 0.5 : 
			i / (double)(ctx->cfg.xtics - 1);

		snprintf(buf, sizeof(buf), "%g", 
			ctx->minv.x + offs *
			(ctx->maxv.x - ctx->minv.x));

		cairo_text_extents(ctx->cr, buf, &e);

		if (ctx->cfg.xrot > 0.0)
			e.height = e.width * sin(ctx->cfg.xrot) +
				e.height * cos(M_PI * 2.0 - 
					(M_PI_2 - ctx->cfg.xrot));

		if (e.height > maxh)
			maxh = e.height;
	}

	for (maxw = 0.0, i = 0; i < ctx->cfg.ytics; i++) {
		offs = 1 == ctx->cfg.ytics ? 0.5 : 
			i / (double)(ctx->cfg.ytics - 1);

		snprintf(buf, sizeof(buf), "%g", 
			ctx->minv.y + offs *
			(ctx->maxv.y - ctx->minv.y));

		cairo_text_extents(ctx->cr, buf, &e);
		if (e.width > maxw)
			maxw = e.width;
	}
	
	if (LABEL_LEFT & ctx->cfg.label) {
		ctx->offs.x += maxw + ctx->cfg.ylabelpad;
		ctx->dims.x -= maxw + ctx->cfg.ylabelpad; 
	}
	if (LABEL_RIGHT & ctx->cfg.label)
		ctx->dims.x -= maxw + ctx->cfg.ylabelpad;

	if (LABEL_TOP & ctx->cfg.label) {
		ctx->offs.y += maxh + ctx->cfg.xlabelpad;
		ctx->dims.y -= maxh + ctx->cfg.xlabelpad;
	}
	if (LABEL_BOTTOM & ctx->cfg.label)
		ctx->dims.y -= maxh + ctx->cfg.xlabelpad;

	for (i = 0; i < ctx->cfg.xtics; i++) {
		offs = 1 == ctx->cfg.xtics ? 0.5 : 
			i / (double)(ctx->cfg.xtics - 1);

		snprintf(buf, sizeof(buf), "%g", 
			ctx->minv.x + offs *
			(ctx->maxv.x - ctx->minv.x));
		cairo_text_extents(ctx->cr, buf, &e);

		if (LABEL_BOTTOM & ctx->cfg.label) {
			if (ctx->cfg.xrot > 0.0) {
				cairo_move_to(ctx->cr, 
					ctx->offs.x + 
					offs * ctx->dims.x -
					0.5 * e.width * 
					cos(ctx->cfg.xrot) +
					e.height * sin
					 (M_PI * 2.0 - 
					  (M_PI_2 - ctx->cfg.xrot)),
					ctx->offs.y + ctx->dims.y + 
					e.height * cos
					 (M_PI * 2.0 - 
					  (M_PI_2 - ctx->cfg.xrot)) +
					ctx->cfg.xlabelpad);
				cairo_save(ctx->cr);
				cairo_rotate(ctx->cr, ctx->cfg.xrot);
			} else 
				cairo_move_to(ctx->cr, 
					ctx->offs.x + offs * ctx->dims.x -
					(e.width / 2.0), 
					ctx->offs.y + ctx->dims.y + 
					maxh + ctx->cfg.xlabelpad);

			cairo_show_text(ctx->cr, buf);
			if (ctx->cfg.xrot > 0.0) {
				cairo_restore(ctx->cr);
			}
		}

		if (LABEL_TOP & ctx->cfg.label) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + offs * ctx->dims.x -
				(e.width / 2.0), 
				ctx->offs.y - maxh);
			cairo_show_text(ctx->cr, buf);
		}
	}

	for (i = 0; i < ctx->cfg.ytics; i++) {
		offs = 1 == ctx->cfg.ytics ? 0.5 : 
			i / (double)(ctx->cfg.ytics - 1);

		snprintf(buf, sizeof(buf), "%g", 
			ctx->minv.y + offs *
			(ctx->maxv.y - ctx->minv.y));
		cairo_text_extents(ctx->cr, buf, &e);

		if (LABEL_LEFT & ctx->cfg.label) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x - e.width - 
				ctx->cfg.ylabelpad,
				(ctx->offs.y + ctx->dims.y) - 
				(offs * ctx->dims.y) + 
				(e.height / 2.0));
			cairo_show_text(ctx->cr, buf);
		}
		if (LABEL_RIGHT & ctx->cfg.label) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + ctx->dims.x + 
				ctx->cfg.ylabelpad,
				(ctx->offs.y + ctx->dims.y) - 
				(offs * ctx->dims.y) + 
				(e.height / 2.0));
			cairo_show_text(ctx->cr, buf);
		}
	}
}
