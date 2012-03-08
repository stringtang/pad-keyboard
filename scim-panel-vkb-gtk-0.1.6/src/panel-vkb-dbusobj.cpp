/*
 * panel-vkb-dbusobj.cpp : implementation of panel-vkb-dbusobj.h
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

#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "panel-vkb-dbusobj.h"
#include "panel-vkb-dbus-glue.h"

GType panel_vkb_dbusobj_get_type(void)
{
    static GType panel_vkb_dbusobj_type = 0;

    if(!panel_vkb_dbusobj_type)
    {
        static const GTypeInfo panel_vkb_dbusobj_info = {
            sizeof(PanelVkbDbusObjClass),
            NULL, NULL,
            (GClassInitFunc) panel_vkb_dbusobj_class_init,
            NULL, NULL,
            sizeof(PanelVkbDbusObj),
            0,
            (GInstanceInitFunc) panel_vkb_dbusobj_init
        };

        panel_vkb_dbusobj_type = g_type_register_static(G_TYPE_OBJECT, "PanelVKBDbusObj", &panel_vkb_dbusobj_info, GTypeFlags(0));
    }

    return panel_vkb_dbusobj_type;
}


void panel_vkb_dbusobj_init(PanelVkbDbusObj *obj)
{
    obj->name=0;
}

void panel_vkb_dbusobj_class_init(PanelVkbDbusObjClass *objclass)
{
    objclass->class_name=strdup("PanelVkbDbusObjClass");
    g_assert(objclass != NULL);
    dbus_g_object_type_install_info(PANEL_VKB_TYPE_DBUSOBJ,
            &dbus_glib_panel_vkb_object_info);
}

//dbusobj init functions
PanelVkbDbusObj * panel_vkb_dbus_init()
{
    DBusGConnection* conn = NULL;
    DBusGProxy* proxy = NULL;

    PanelVkbDbusObj* dbusobj = NULL;

    guint result;
    GError* error = NULL;

    g_type_init();
    conn = dbus_g_bus_get(DBUS_BUS_SESSION, &error);

    if (conn == NULL){
        std::cerr << "DBus init failed : Cannot conncet to session bus" << error->message << "\n";
        g_error_free(error);
        exit(-1);
    }

    proxy = dbus_g_proxy_new_for_name(conn,
            DBUS_SERVICE_DBUS,
            DBUS_PATH_DBUS,
            DBUS_INTERFACE_DBUS);
    if(proxy == NULL)
        std::cerr << "DBus init failed : Cannot create DBus proxy\n";

    /* Attempt to register the well-known name.*/
    if (!dbus_g_proxy_call(proxy,
                "RequestName",
                &error,
                G_TYPE_STRING,
                PANEL_VKB_DBUSOBJ_SERVICE_NAME,
                G_TYPE_UINT,
                0,
                G_TYPE_INVALID,
                G_TYPE_UINT,
                &result,
                G_TYPE_INVALID)) {
        std::cerr << "DBus init failed : D-Bus.RequestName RPC failed" << error->message <<"\n";
    }

    dbusobj = (PanelVkbDbusObj *) g_object_new(PANEL_VKB_TYPE_DBUSOBJ, NULL);
    if (dbusobj == NULL) {
        std::cerr << "DBus init failed : Failed to create dbus_obj\n";
    }

    dbus_g_connection_register_g_object(conn,
            PANEL_VKB_DBUSOBJ_SERVICE_OBJECT_PATH,
            G_OBJECT(dbusobj));

    return dbusobj;
}

