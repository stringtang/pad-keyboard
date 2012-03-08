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

#ifndef _FVKBD_POP_WIN_H
#define _FVKBD_POP_WIN_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MAX_POP_WIN_ITEMS 10

GtkWidget *get_pop_window(void);

void update_pop_win_item_none(void);
void update_pop_win_item_string(gchar *disp, gchar *str, PangoFontDescription *desc, gboolean reverse);
void update_pop_win_item_string_group(gchar **strs, PangoFontDescription *desc, gboolean reverse);
void update_pop_win_item_sym(gchar *disp, KeySym sym, PangoFontDescription *desc, gboolean reverse);

void settle_pop_window_color(gint number, GdkColor *bgcolor, GdkColor *fgcolor);
void pop_win_set_height_request(int height);
void pop_win_items_set_width_request(int width);

G_END_DECLS

#endif //_FVKBD_POP_WIN_H