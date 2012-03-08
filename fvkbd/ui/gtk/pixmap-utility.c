/*
 * pixmap-utility.c pixmap realted functions for fvkbd
 *
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */


#include "pixmap-utility.h"

static GdkGC *
_get_bitmap_gc(void)
{
	static GdkGC *gc = NULL;

	if (!gc) {
		GdkDrawable *bitmap;
		bitmap = (GdkDrawable *)gdk_pixmap_new(NULL, 1, 1, 1);
		gc = gdk_gc_new(bitmap);
		g_object_unref(bitmap);
	}

	return gc;
}


static GdkBitmap *
_get_chamfered_rectangle_bitmap(gint width, gint height, gint corner)
{
	GdkDrawable *bitmap;
	GdkGC *gc;
	static GdkColor white = { 0, 65535, 65535, 65535 };
	static GdkColor black = { 0, 0, 0, 0 };

	bitmap = (GdkDrawable *)gdk_pixmap_new(NULL, width, height, 1);
	gc = _get_bitmap_gc();

	if (white.pixel == 0)
		gdk_colormap_alloc_color(gtk_widget_get_default_colormap(), &white, FALSE, TRUE);
	if (black.pixel == 0)
		gdk_colormap_alloc_color(gtk_widget_get_default_colormap(), &black, FALSE, TRUE);

	gdk_gc_set_background(gc, &white);
	gdk_gc_set_foreground(gc, &black);
	gdk_draw_rectangle(bitmap, gc, TRUE, 0, 0, width, height);

	gdk_gc_set_background(gc, &black);
	gdk_gc_set_foreground(gc, &white);


	/* polygon with fill is acting strangely */
	{
		GdkPoint points[8];
		points[0].x = corner;
		points[0].y = 0;
		points[1].x = width - corner;
		points[1].y = 0;
		points[2].x = width;
		points[2].y = corner;
		points[3].x = width;
		points[3].y = height - corner - 1;
		points[4].x = width - corner - 1;
		points[4].y = height;
		points[5].x = corner + 1;
		points[5].y = height;
		points[6].x = 0;
		points[6].y = height - corner - 1;
		points[7].x = 0;
		points[7].y = corner;

		gdk_draw_polygon(bitmap, gc, TRUE, points, 8);
	}

	return bitmap;
}


GdkBitmap *
get_chamfered_rectangle_bitmap (gint width, gint height, gint corner)
{
	g_return_val_if_fail(((width > 0) && (height > 0)), NULL);

	if (corner < 0)
		corner = 0;

	if (corner > (width / 2))
		corner = width / 2;

	if (corner > (height / 2))
		corner = height / 2;

	return _get_chamfered_rectangle_bitmap(width, height, corner);
}

