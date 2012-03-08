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

#ifndef _GTK_UI_BASE_H
#define _GTK_UI_BASE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FvkbdGtkUI FvkbdGtkUI;
typedef struct _FvkbdGtkUIClass FvkbdGtkUIClass;

#define FVKBD_TYPE_GTK_UI		(fvkbd_gtk_ui_get_type())
#define FVKBD_GTK_UI(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_GTK_UI, FvkbdGtkUI))
#define FVKBD_GTK_UI_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_GTK_UI, FvkbdGtkUIClass))
#define FVKBD_IS_GTK_UI(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_GTK_UI))
#define FVKBD_IS_GTK_UI_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_GTK_UI))
#define FVKBD_GTK_UI_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_GTK_UI, FvkbdGtkUIClass))

typedef enum
{
	FLAG_KEY_SHAPE_BITMAP = 1 << 0,
	FLAG_KEY_BG_PIXMAP = 1 << 1,
	FLAG_UI_FONT_DESCS = 1 << 2	
} FvkbdUIFlags;

#define FVKBD_UI_FLAGS(u)			(FVKBD_GTK_UI(u)->flags)
#define FVKBD_UI_HAS_KEY_SHAPE_BITMAP(obj)	((FVKBD_UI_FLAGS(obj) & FLAG_KEY_SHAPE_BITMAP) != 0)
#define FVKBD_UI_HAS_KEY_BG_PIXMAP(obj)		((FVKBD_UI_FLAGS(obj) & FLAG_KEY_BG_PIXMAP) != 0)
#define FVKBD_UI_HAS_FONT_DESCS(obj)		((FVKBD_UI_FLAGS(obj) & FLAG_UI_FONT_DESCS) != 0)

#define FVKBD_UI_SET_FLAG(u, flag)		(FVKBD_UI_FLAGS(u) |= (flag))
#define FVKBD_UI_UNSET_FLAG(u, flag)		(FVKBD_UI_FLAGS(u) &= ~(flag))

struct _FvkbdGtkUI {
	GObject parent;
	FvkbdGtkUI *parent_ui;
	FvkbdUnit *unit;

	guint16 flags;

	gfloat x_ratio;
	gfloat y_ratio;

	GtkWidget *widget;
};

struct _FvkbdGtkUIClass {
	GObjectClass parent;

	/* Virtual functions */
	gboolean (*build)		(FvkbdGtkUI *ui, GtkWidget **widget);
	gboolean (*allocate)		(FvkbdGtkUI *ui, gint x, gint y, gfloat x_ratio, gfloat y_ratio);
	gboolean (*destroy)		(FvkbdGtkUI *ui);
	gboolean (*set_mode)		(FvkbdGtkUI *ui, gint id);
};

GType fvkbd_gtk_ui_get_type (void);

/* variable */
extern GQuark quark_key_shape_bitmap;
extern GQuark quark_key_bg_pixmap;
extern GQuark quark_ui_font_descs;

/* Public Functions */
gboolean fvkbd_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget);
gboolean fvkbd_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y,
					gfloat x_ratio, gfloat y_ratio);
gboolean fvkbd_gtk_ui_destroy (FvkbdGtkUI *ui);
gboolean fvkbd_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id);

void fvkbd_gtk_ui_set_parent (FvkbdGtkUI *ui, FvkbdGtkUI *parent);
FvkbdGtkUI *fvkbd_gtk_ui_get_parent (FvkbdGtkUI *ui);

void fvkbd_gtk_ui_set_unit (FvkbdGtkUI *ui, FvkbdUnit *unit);
FvkbdUnit *fvkbd_gtk_ui_get_unit (FvkbdGtkUI *ui);

void fvkbd_gtk_ui_set_widget (FvkbdGtkUI *ui, GtkWidget *data);
GtkWidget *fvkbd_gtk_ui_get_widget (FvkbdGtkUI *ui);

gint fvkbd_gtk_ui_set_ratio (FvkbdGtkUI *ui, gfloat x_ratio, gfloat y_ratio);
gint fvkbd_gtk_ui_get_ratio (FvkbdGtkUI *ui, gfloat *x_ratio, gfloat *y_ratio);

gpointer fvkbd_gtk_ui_get_qdata_recursive(FvkbdGtkUI *ui, GQuark quark,
					gboolean *from_parent);
gpointer fvkbd_gtk_ui_get_qdata(FvkbdGtkUI *ui, GQuark quark);
void fvkbd_gtk_ui_set_qdata(FvkbdGtkUI *ui, GQuark quark, gpointer data,
				GDestroyNotify destroy);

G_END_DECLS

#endif //_GTK_UI_BASE_H