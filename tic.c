#include <assert.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kplotctx_tic_init(struct kplotctx *ctx)
{
	struct kplotclr	 clr;
	double		 offs;
	size_t		 i;

	kplotctx_colour(ctx, ctx->cfg.ticclr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_line_width(ctx->cr, ctx->cfg.ticsz);

	for (i = 0; i < ctx->cfg.xtics; i++) {
		offs = 1 == ctx->cfg.xtics ? 0.5 : 
			i / (double)(ctx->cfg.xtics - 1);
		if (TIC_BOTTOM_IN & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + offs * ctx->dims.x, 
				ctx->offs.y + ctx->dims.y);
			cairo_rel_line_to(ctx->cr, 0.0, -ctx->cfg.ticlen);
		}
		if (TIC_BOTTOM_OUT & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + offs * ctx->dims.x, 
				ctx->offs.y + ctx->dims.y);
			cairo_rel_line_to(ctx->cr, 0.0, ctx->cfg.ticlen);
		}
		if (TIC_TOP_IN & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + offs * ctx->dims.x, 
				ctx->offs.y);
			cairo_rel_line_to(ctx->cr, 0.0, ctx->cfg.ticlen);
		}
		if (TIC_TOP_OUT & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + offs * ctx->dims.x, 
				ctx->offs.y);
			cairo_rel_line_to(ctx->cr, 0.0, -ctx->cfg.ticlen);
		}
	}

	for (i = 0; i < ctx->cfg.ytics; i++) {
		offs = 1 == ctx->cfg.ytics ? 0.5 : 
			i / (double)(ctx->cfg.ytics - 1);
		if (TIC_LEFT_IN & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x,
				ctx->offs.y + offs * ctx->dims.y);
			cairo_rel_line_to(ctx->cr, ctx->cfg.ticlen, 0.0);
		}
		if (TIC_LEFT_OUT & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x,
				ctx->offs.y + offs * ctx->dims.y);
			cairo_rel_line_to(ctx->cr, -ctx->cfg.ticlen, 0.0);
		}
		if (TIC_RIGHT_IN & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + ctx->dims.x, 
				ctx->offs.y + offs * ctx->dims.y);
			cairo_rel_line_to(ctx->cr, -ctx->cfg.ticlen, 0.0);
		}
		if (TIC_RIGHT_OUT & ctx->cfg.tic) {
			cairo_move_to(ctx->cr, 
				ctx->offs.x + ctx->dims.x, 
				ctx->offs.y + offs * ctx->dims.y);
			cairo_rel_line_to(ctx->cr, ctx->cfg.ticlen, 0.0);
		}
	}

	cairo_stroke(ctx->cr);
}
