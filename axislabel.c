#include <assert.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kplotctx_axislabel_init(struct kplotctx *ctx)
{
#if 0
	char		 buf[128];
	size_t		 i;
	cairo_text_extents_t e;
	double		 maxh, maxw, offs, lastx, lasty, firsty;
	struct kplotclr	 clr;
	const char	*xlabel = "foo", *ylabel = "bar";

	kplotctx_colour(ctx, ctx->cfg.labelclr, &clr);
	cairo_set_source_rgba(ctx->cr, clr.r, clr.g, clr.b, clr.a);

	cairo_text_extents(ctx->cr, xlabel, &e);

	ctx->offs.x += e.height * 2.0;

	if (ctx->cfg.xlabelrot > 0.0) {
		cairo_move_to(ctx->cr, 
			ctx->offs.x + 
			offs * ctx->dims.x,
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
			if (ctx->cfg.xrot > 0.0)
				cairo_restore(ctx->cr);
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

		if (NULL == ctx->cfg.yformat)
			snprintf(buf, sizeof(buf), "%g", 
				ctx->minv.y + offs *
				(ctx->maxv.y - ctx->minv.y));
		else
			(*ctx->cfg.yformat)
				(ctx->minv.y + offs *
				 (ctx->maxv.y - ctx->minv.y),
				 buf, sizeof(buf));

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
#endif
}
