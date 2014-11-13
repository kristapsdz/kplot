#include <assert.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kplotctx_border_init(struct kplotctx *ctx)
{
	struct kplotclr	 clr;

	kplotctx_colour(ctx, ctx->cfg.borderclr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);
	cairo_set_line_width(ctx->cr, ctx->cfg.bordersz);

	if (BORDER_LEFT & ctx->cfg.border) {
		cairo_move_to(ctx->cr, ctx->offs.x, ctx->offs.y);
		cairo_rel_line_to(ctx->cr, 0.0, ctx->dims.y);
	}

	if (BORDER_RIGHT & ctx->cfg.border) {
		cairo_move_to(ctx->cr, 
			ctx->offs.x + ctx->dims.x, ctx->offs.y);
		cairo_rel_line_to(ctx->cr, 0.0, ctx->dims.y);
	}

	if (BORDER_TOP & ctx->cfg.border) {
		cairo_move_to(ctx->cr, ctx->offs.x, ctx->offs.y);
		cairo_rel_line_to(ctx->cr, ctx->dims.x, 0.0);
	}

	if (BORDER_BOTTOM & ctx->cfg.border) {
		cairo_move_to(ctx->cr, 
			ctx->offs.x, ctx->offs.y + ctx->dims.y);
		cairo_rel_line_to(ctx->cr, ctx->dims.x, 0.0);
	}

	cairo_stroke(ctx->cr);
}
