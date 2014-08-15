/*
 * Copyright © 2014 Pekka Paalanen <pq@iki.fi>
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wesgr.h"

struct svg_context {
	FILE *fp;
};

int
graph_data_init(struct graph_data *gdata)
{
	memset(gdata, 0, sizeof *gdata);

	return 0;
}

static void
line_block_destroy(struct line_block *bl)
{
	free(bl->desc);
	free(bl);
}

static void
line_graph_release(struct line_graph *linegr)
{
	struct line_block *lb, *tmp;

	for (lb = linegr->block; lb; lb = tmp) {
		tmp = lb->next;
		line_block_destroy(lb);
	}
}

static void
output_graph_destroy(struct output_graph *og)
{
	line_graph_release(&og->repaint_line);
	free(og);
}

void
graph_data_release(struct graph_data *gdata)
{
	struct output_graph *og, *tmp;

	for (og = gdata->output; og; og = tmp) {
		tmp = og->next;
		output_graph_destroy(og);
	}
}

static int
line_block_to_svg(struct line_block *lb, struct svg_context *ctx)
{
	fprintf(ctx->fp, "%s %lld.%09ld -> %lld.%09ld\n",
		lb->style, (long long)lb->begin.tv_sec, lb->begin.tv_nsec,
		(long long)lb->end.tv_sec, lb->end.tv_nsec);

	return 0;
}

static int
line_graph_to_svg(struct line_graph *linegr, struct svg_context *ctx)
{
	struct line_block *lb;

	for (lb = linegr->block; lb; lb = lb->next)
		if (line_block_to_svg(lb, ctx) < 0)
			return -1;

	return 0;
}

static int
output_graph_to_svg(struct output_graph *og, struct svg_context *ctx)
{
	return line_graph_to_svg(&og->repaint_line, ctx);
}

int
graph_data_to_svg(struct graph_data *gdata, const char *filename)
{
	struct output_graph *og;
	struct svg_context ctx;

	ctx.fp = stdout;

	for (og = gdata->output; og; og = og->next)
		if (output_graph_to_svg(og, &ctx) < 0)
			return -1;

	return 0;
}

