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

#ifndef _FVKBD_PANEL_UI_GTK_H
#define _FVKBD_PANEL_UI_GTK_H

#include "gtk-ui-base.h"
#include "fvkbd-panel.h"

G_BEGIN_DECLS

typedef struct _GtkVkbPanel GtkVkbPanel;
typedef struct _GtkVkbPanelClass GtkVkbPanelClass;

#define GTK_TYPE_VKB_PANEL		(gtk_vkb_panel_get_type())
#define GTK_VKB_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_VKB_PANEL, GtkVkbPanel))
#define GTK_VKB_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_VKB_PANEL, GtkVkbPanelClass))
#define GTK_IS_VKB_PANEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_VKB_PANEL))
#define GTK_IS_VKB_PANEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_VKB_PANEL))
#define GTK_VKB_PANEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_VKB_PANEL, GtkVkbPanelClass))


struct _GtkVkbPanel
{
	GtkFixed parent;
	FvkbdPanel *panel_unit;
};

struct _GtkVkbPanelClass
{
	GtkFixedClass parent_class;
};

GType gtk_vkb_panel_get_type (void);

GtkWidget *gtk_vkb_panel_new (FvkbdPanel *unit);


typedef struct _FvkbdPanelGtkUI FvkbdPanelGtkUI;
typedef struct _FvkbdPanelGtkUIClass FvkbdPanelGtkUIClass;

#define FVKBD_TYPE_PANEL_GTK_UI			(fvkbd_panel_gtk_ui_get_type())
#define FVKBD_PANEL_GTK_UI(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_PANEL_GTK_UI, FvkbdPanelGtkUI))
#define FVKBD_PANEL_GTK_UI_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_PANEL_GTK_UI, FvkbdPanelGtkUIClass))
#define FVKBD_IS_PANEL_GTK_UI(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_PANEL_GTK_UI))
#define FVKBD_IS_PANEL_GTK_UI_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_PANEL_GTK_UI))
#define FVKBD_PANEL_GTK_UI_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_PANEL_GTK_UI, FvkbdPanelGtkUIClass))


struct _FvkbdPanelGtkUI
{
	FvkbdGtkUI parent;
	GSList *children;
};

struct _FvkbdPanelGtkUIClass
{
	FvkbdGtkUIClass parent_class;
};

GType fvkbd_panel_gtk_ui_get_type (void);

FvkbdGtkUI *fvkbd_panel_gtk_ui_new (FvkbdUnit *unit);

G_END_DECLS

#endif //_FVKBD_PANEL_UI_GTK_H