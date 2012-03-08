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

#ifndef _GTK_VKB_BUTTON_H
#define _GTK_VKB_BUTTON_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GtkVkbButton GtkVkbButton;
typedef struct _GtkVkbButtonPrivate GtkVkbButtonPrivate;
typedef struct _GtkVkbButtonClass GtkVkbButtonClass;

#define GTK_TYPE_VKB_BUTTON		(gtk_vkb_button_get_type())
#define GTK_VKB_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_VKB_BUTTON, GtkVkbButton))
#define GTK_VKB_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_VKB_BUTTON, GtkVkbButtonClass))
#define GTK_IS_VKB_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_VKB_BUTTON))
#define GTK_IS_VKB_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_VKB_BUTTON))
#define GTK_VKB_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_VKB_BUTTON, GtkVkbButtonClass))


struct _GtkVkbButton
{
	GtkBin parent;

	GdkColormap *colormap;
	GdkColor bgcolor[5];
	GdkPixmap *bg_pixmap;
};

struct _GtkVkbButtonClass
{
	GtkBinClass parent_class;
	void (*pressed)		(GtkVkbButton *button);
	void (*released)	(GtkVkbButton *button);
};

GType gtk_vkb_button_get_type (void);

GtkWidget *gtk_vkb_button_new (void);
void vkb_button_set_bg(GtkWidget *widget, GtkStateType state, const GdkColor *color);
void vkb_button_set_bg_pixmap(GtkWidget *widget, GdkPixmap *pixmap);


G_END_DECLS

#endif //_GTK_VKB_BUTTON_H
