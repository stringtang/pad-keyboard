/*
 * gtk-misc-utility.c misc functions
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

#include "fvkbd.h"
#include <gtk/gtk.h>

#include "misc-utility.h"
#include "gtk-misc-utility.h"


PangoFontDescription *kbd_pango_font_description = NULL;

inline void
kbdcolor_to_gdkcolor (KbdColor *kc, GdkColor *color)
{
	if (color != NULL) {
		color->red = kc->r * (65535 / 255);
		color->green = kc->g * (65535 / 255);
		color->blue = kc->b * (65535 / 255);
	}
}

gboolean
get_gdkcolor (FvkbdUnit *unit, KbdColorType type, GdkColor *color)
{
	KbdColor *kc;

	if (!color)
		return FALSE;

	if ((kc = fvkbd_unit_get_color(unit, type)) == NULL)
		return FALSE;

	kbdcolor_to_gdkcolor(kc, color);
	return TRUE;
}

gboolean
get_gdkcolor_reverse (FvkbdUnit *unit, KbdColorType type, GdkColor *color)
{
	KbdColor *kc;
	KbdColor rev_color;

	if (!color)
		return FALSE;

	if ((kc = fvkbd_unit_get_color(unit, type)) == NULL)
		return FALSE;

	rev_color.r = 255 - kc->r;
	rev_color.g = 255 - kc->g;
	rev_color.b = 255 - kc->b;

	if (!color)
		return FALSE;

	kbdcolor_to_gdkcolor(&rev_color, color);
	return TRUE;
}


static PangoFontDescription *
get_pango_font_description_from_info (gchar *font_family, gchar *font_weight, gint font_size)
{
	PangoFontDescription *description = NULL;
	PangoWeight pw = PANGO_WEIGHT_NORMAL;

	g_assert(font_family);
	g_assert(font_weight);
	g_assert(font_size > 0);

	description = pango_font_description_new();
	pango_font_description_set_family(description, font_family);

	if (!g_strcmp0(font_weight, "bold")) {
		pw = PANGO_WEIGHT_BOLD;
	} else {
		pw = PANGO_WEIGHT_NORMAL;
	}

	pango_font_description_set_weight(description, pw);

	pango_font_description_set_size(description,
					font_size * PANGO_SCALE);

	return description;
}


PangoFontDescription *
get_scaled_pango_font_description (KbdFontInfo *font, gfloat x_ratio, gfloat y_ratio)
{
	int dpi_xres, dpi_yres;
	gfloat ratio;

	get_resolution(&dpi_xres, &dpi_yres);

	x_ratio = x_ratio * DEFAULT_DPI / dpi_xres;
	y_ratio = y_ratio * DEFAULT_DPI / dpi_yres;

	ratio = (x_ratio < y_ratio) ? x_ratio : y_ratio;

	return get_pango_font_description_from_info(
				font->family,
				font->weight,
				font->size * ratio);
}


void
free_ui_font_descs (PangoFontDescription **descs)
{
	int i;
	PangoFontDescription *desc;

	for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++) {
		desc = *(descs + i);
		if (desc)
			pango_font_description_free(desc);
	}

	g_free(descs);
}


gboolean
set_gtk_widget_bg_image (GtkWidget *widget, const gchar *filename, guint bgcolor)
{
	GdkPixbuf *pbuf, *bg;
	GdkPixmap *pixmap;
	gint width, height;

	if (GTK_WIDGET_NO_WINDOW(widget) || !GTK_WIDGET_REALIZED(widget) || !filename)
		goto fail;
   
	pbuf = gdk_pixbuf_new_from_file(filename, NULL);
	if (!pbuf)
		goto fail;

	width = gdk_pixbuf_get_width(pbuf);
	height = gdk_pixbuf_get_height(pbuf);

	bg = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE,
				gdk_pixbuf_get_bits_per_sample(pbuf),
				width, height);
	gdk_pixbuf_fill(bg, bgcolor);
	gdk_pixbuf_composite(pbuf, bg, 0, 0, width, height,
				0, 0, 1, 1, GDK_INTERP_BILINEAR,0xFF);

	pixmap = gdk_pixmap_new(widget->window, width, height, -1);
	gdk_draw_pixbuf(pixmap, NULL, bg, 0, 0, 0, 0,
			-1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);

  	g_object_unref(pbuf);
  	g_object_unref(bg);
   
	gtk_widget_set_app_paintable(widget,TRUE);
	gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);
	g_object_unref(pixmap);
	//gtk_widget_queue_draw(widget);

	return TRUE;
fail:
	return FALSE;
}


static GdkPixbuf *
_load_and_scale_pixbuf (const gchar *file, gfloat x_ratio, gfloat y_ratio)
{
	GdkPixbuf *pixbuf = NULL;

	pixbuf = gdk_pixbuf_new_from_file(file, NULL);

	if (!pixbuf)
		return NULL;

	if ((x_ratio > 1.05 || x_ratio < 0.95) ||
		(y_ratio > 1.05 || y_ratio < 0.95)) {
		int w,h;
		GdkPixbuf *tmp;

	        w = gdk_pixbuf_get_width(pixbuf) * x_ratio;
		h = gdk_pixbuf_get_height(pixbuf) * y_ratio;
		tmp = gdk_pixbuf_scale_simple(pixbuf, w, h, GDK_INTERP_HYPER);
		g_object_unref(pixbuf);
		pixbuf = tmp;
	}

	return pixbuf;
}


GtkWidget *
load_and_scale_img (const gchar *file, gfloat x_ratio, gfloat y_ratio)
{
	GdkPixbuf *pixbuf = NULL;
	GtkWidget *img;

	pixbuf = _load_and_scale_pixbuf(file, x_ratio, y_ratio);

	if (!pixbuf)
		return NULL;

	img = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	return img;
}


GdkBitmap *
load_and_scale_bitmap (const gchar *file, gfloat x_ratio, gfloat y_ratio)
{
	GdkPixbuf *pixbuf = NULL;
	GdkBitmap *bitmap;

	pixbuf = _load_and_scale_pixbuf(file, x_ratio, y_ratio);

	if (!pixbuf)
		return NULL;

	bitmap = gdk_pixmap_new(NULL, gdk_pixbuf_get_width(pixbuf),
				gdk_pixbuf_get_height(pixbuf), 1);

	gdk_pixbuf_render_threshold_alpha(pixbuf, bitmap,
						0, 0, 0, 0,
						-1, -1,	128);

	g_object_unref(pixbuf);
	return bitmap;
}

void
load_and_scale_pixmap_and_mask (const gchar *file, gfloat x_ratio, gfloat y_ratio,
					GdkPixmap **pixmap_return, GdkBitmap **mask_return)
{
	GdkPixbuf *pixbuf = NULL;
	if (pixmap_return)
		*pixmap_return = NULL;

	if (mask_return)
		*mask_return = NULL;

	pixbuf = _load_and_scale_pixbuf(file, x_ratio, y_ratio);

	if (!pixbuf)
		return;

	gdk_pixbuf_render_pixmap_and_mask(pixbuf, pixmap_return,
					mask_return, 128);

	g_object_unref(pixbuf);
	return;
}