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

#ifndef _FVKBD_PANEL_H
#define _FVKBD_PANEL_H


#include "fvkbd-base.h"

G_BEGIN_DECLS

typedef struct _FvkbdPanel FvkbdPanel;
typedef struct _FvkbdPanelPrivate FvkbdPanelPrivate;
typedef struct _FvkbdPanelClass FvkbdPanelClass;

#define FVKBD_TYPE_PANEL			(fvkbd_panel_get_type())
#define FVKBD_PANEL(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_PANEL, FvkbdPanel))
#define FVKBD_PANEL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_PANEL, FvkbdPanelClass))
#define FVKBD_IS_PANEL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_PANEL))
#define FVKBD_IS_PANEL_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_PANEL))
#define FVKBD_PANEL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_PANEL, FvkbdPanelClass))

struct _FvkbdPanel {
	FvkbdUnit parent;
	FvkbdPanelPrivate *priv;
};

struct _FvkbdPanelClass {
	FvkbdUnitClass parent;
};

GType fvkbd_panel_get_type (void);

typedef enum _PanelLayoutType PanelLayoutType;
enum _PanelLayoutType {
	LAYOUT_TYPE_ROW = 1,
	LAYOUT_TYPE_XY
};

typedef enum _PanelDockType PanelDockType;
enum _PanelDockType {
	LAYOUT_DOCK_NONE = 0,
	LAYOUT_DOCK_TOP,
	LAYOUT_DOCK_BOTTOM,
	LAYOUT_DOCK_LEFT,
	LAYOUT_DOCK_RIGHT
};

FvkbdUnit *fvkbd_panel_new (void);

gint fvkbd_panel_get_docktype (FvkbdPanel *self);
gchar *fvkbd_panel_get_img (FvkbdPanel *self);
GSList *fvkbd_panel_get_children (FvkbdPanel *self);

G_END_DECLS

#endif //_FVKBD_PANEL_H