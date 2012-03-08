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

#ifndef _MISC_UTILITY_H
#define _MISC_UTILITY_H

G_BEGIN_DECLS

#define DEFAULT_DPI	96

Display *get_x_display(void);
FakeKey *get_fakekey_instance(void);
gboolean get_workarea(int *x, int *y, int *width, int *height);
void get_resolution(int *xres, int *yres);

KbdColor *kbd_str_to_color(gchar *str);
KbdFontInfo *parse_font_property(gchar *str, gchar *default_font_family, gchar *default_font_weight, gint default_font_size);
KbdShapeInfo *parse_shape_info_property (gchar *str);
void parse_shape_data_property (KbdShapeInfo *info, gchar *str);
void kbd_shape_info_destroy (KbdShapeInfo *info);

void fvkbd_do_script(gchar *string);
gchar *locate_img_file(gchar *filename);

gboolean _fvkbd_boolean_handled_accumulator (GSignalInvocationHint *ihint,
						GValue *accumulate,
						const GValue *h_return,
						gpointer dummy);


void _fvkbd_marshal_BOOLEAN__POINTER (GClosure *closure,
					GValue *return_value G_GNUC_UNUSED,
					guint n_param_values,
					const GValue *param_values,
					gpointer invocation_hint G_GNUC_UNUSED,
					gpointer marshal_data);


G_END_DECLS

#endif //_MISC_UTILITY_H

