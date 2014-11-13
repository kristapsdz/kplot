#include <assert.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

#include "kplot.h"
#include "compat.h"
#include "extern.h"

void
kplotctx_margin_init(struct kplotctx *ctx)
{

	ctx->dims.x = ctx->w;
	ctx->dims.y = ctx->h;

	if (MARGIN_LEFT & ctx->cfg.margin) {
		ctx->dims.x -= ctx->cfg.marginsz;
		ctx->offs.x = ctx->cfg.marginsz;
	}
	if (MARGIN_RIGHT & ctx->cfg.margin)
		ctx->dims.x -= ctx->cfg.marginsz;

	if (MARGIN_TOP & ctx->cfg.margin) {
		ctx->dims.y -= ctx->cfg.marginsz;
		ctx->offs.y = ctx->cfg.marginsz;
	}
	if (MARGIN_BOTTOM & ctx->cfg.margin)
		ctx->dims.y -= ctx->cfg.marginsz;
}

