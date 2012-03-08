/*
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

#ifndef _GTK_MISC_UTILITY_H
#define _GTK_MISC_UTILITY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

gboolean get_gdkcolor (FvkbdUnit *unit, KbdColorType type, GdkColor *color);
gboolean get_gdkcolor_reverse (FvkbdUnit *unit, KbdColorType type, GdkColor *color);
PangoFontDescription *get_scaled_pango_font_description (KbdFontInfo *font, gfloat x_ratio, gfloat y_ratio);
void free_ui_font_descs (PangoFontDescription **descs);
gboolean set_gtk_widget_bg_image (GtkWidget *widget, const gchar *filename, guint bgcolor);
GtkWidget *load_and_scale_img (const gchar *file, gfloat x_ratio, gfloat y_ratio);
GdkBitmap *load_and_scale_bitmap (const gchar *file, gfloat x_ratio, gfloat y_ratio);
void load_and_scale_pixmap_and_mask (const gchar *file, gfloat x_ratio, gfloat y_ratio,
					GdkPixmap **pixmap_return, GdkBitmap **mask_return);
G_END_DECLS

#endif //_GTK_MISC_UTILITY_H