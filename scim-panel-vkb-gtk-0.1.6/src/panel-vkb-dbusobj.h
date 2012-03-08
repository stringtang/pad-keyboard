/*
 * panel-vkb-dbusobj.h : dbus object defination for panel-vkb
 *
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Vincent Huang <chenglan.huang@intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 3 as published by the Free Software Foundation.
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

#ifndef PANEL_VKB_DBUSOBJ_H
#define PANEL_VKB_DBUSOBJ_H

#include <glib-object.h>

/* Well-known name for this service. */
#define PANEL_VKB_DBUSOBJ_SERVICE_NAME        "org.moblin.scim.vkb"
/* Object path to the provided object. */
#define PANEL_VKB_DBUSOBJ_SERVICE_OBJECT_PATH "/org/moblin/scim/vkb"
#define PANEL_VKB_DBUSOBJ_SERVICE_INTERFACE   "org.moblin.scim.vkb"

#define PANEL_VKB_TYPE_DBUSOBJ (panel_vkb_dbusobj_get_type())
#define PANELVKBDBUSOBJ(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PANEL_VKB_TYPE_DBUSOBJ, PanelVkbDbusObj))

typedef struct _PanelVkbDbusObj PanelVkbDbusObj;
typedef struct _PanelVkbDbusObjClass PanelVkbDbusObjClass;

struct _PanelVkbDbusObj {
	GObject parent;
	gchar * name;
};

struct _PanelVkbDbusObjClass {
	GObjectClass parent_class;
	gchar * class_name;
};

GType panel_vkb_dbusobj_get_type(void);
void panel_vkb_dbusobj_init(PanelVkbDbusObj *obj);
void panel_vkb_dbusobj_class_init(PanelVkbDbusObjClass *objclass);

PanelVkbDbusObj * panel_vkb_dbus_init();
gboolean panel_vkb_turn_on_panel(PanelVkbDbusObj * dbusobj, GError **error);
gboolean panel_vkb_move_panel(PanelVkbDbusObj * dbusobj, GError **error);
gboolean panel_vkb_change_inputmethod(PanelVkbDbusObj * dbusobj, GError **error);
gboolean panel_vkb_turn_off_panel(PanelVkbDbusObj * dbusobj, GError **error);
gboolean panel_vkb_toggle_vkb_alone(PanelVkbDbusObj * dbusobj, GError **error);
//gboolean panel_vkb_recover_input_panel(PanelVkbDbusObj * dbusobj, GError **error);
#endif
