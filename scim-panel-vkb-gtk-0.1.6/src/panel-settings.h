/*
 * panel-settings.h: default vkb panel setting values
 *
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
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

#ifndef __PANEL_SETTINGS_H
#define __PANEL_SETTINGS_H

// keys

#define SCIM_CONFIG_PANEL_VKB_GTK_FONT                      "/Panel/VKB/Gtk/Font"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_AUTO_SNAP         "/Panel/VKB/Gtk/ToolBar/AutoSnap"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_HIDE_TIMEOUT      "/Panel/VKB/Gtk/ToolBar/HideTimeout"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_POS_X             "/Panel/VKB/Gtk/ToolBar/POS_X"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_POS_Y             "/Panel/VKB/Gtk/ToolBar/POS_Y"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_FACTORY_ICON "/Panel/VKB/Gtk/ToolBar/ShowFactoryIcon"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_FACTORY_NAME "/Panel/VKB/Gtk/ToolBar/ShowFactoryName"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_STICK_ICON   "/Panel/VKB/Gtk/ToolBar/ShowStickIcon"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_HELP_ICON    "/Panel/VKB/Gtk/ToolBar/ShowHelpIcon"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_MENU_ICON    "/Panel/VKB/Gtk/ToolBar/ShowMenuIcon"
#define SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_PROPERTY_LABEL "/Panel/VKB/Gtk/ToolBar/ShowPropertyLabel"
#define SCIM_CONFIG_PANEL_VKB_GTK_DEFAULT_STICKED           "/Panel/VKB/Gtk/DefaultSticked"
#define SCIM_CONFIG_PANEL_VKB_GTK_SHOW_TRAY_ICON            "/Panel/VKB/Gtk/ShowTrayIcon"
#define SCIM_CONFIG_PANEL_VKB_GTK_DOCK_PANEL                "/Panel/VKB/Gtk/DockPanel"
#define SCIM_CONFIG_PANEL_VKB_GTK_ENABLE_VKB                "/Panel/VKB/Gtk/EnableVKB"
#define SCIM_CONFIG_PANEL_VKB_GTK_PANEL_EXTEND              "/Panel/VKB/Gtk/PanelExtend"
#define SCIM_CONFIG_PANEL_VKB_GTK_VKB_PROGRAM               "/Panel/VKB/Gtk/VKBProgram"
#define SCIM_CONFIG_PANEL_VKB_GTK_VKB_PARAMETERS            "/Panel/VKB/Gtk/VKBParameters"
#define SCIM_CONFIG_PANEL_VKB_GTK_VKB_WIDTH                 "/Panel/VKB/Gtk/VKB_Width"
#define SCIM_CONFIG_PANEL_VKB_GTK_VKB_HEIGHT                "/Panel/VKB/Gtk/VKB_Height"
#define SCIM_CONFIG_PANEL_VKB_GTK_PANEL_NORMAL_TEXT_COLOR   "/Panel/VKB/Gtk/Panel/Normal/TextColor"
#define SCIM_CONFIG_PANEL_VKB_GTK_PANEL_NORMAL_FG_COLOR     "/Panel/VKB/Gtk/Panel/Normal/FGColor"
#define SCIM_CONFIG_PANEL_VKB_GTK_PANEL_NORMAL_BG_COLOR     "/Panel/VKB/Gtk/Panel/Normal/BGColor"
#define SCIM_CONFIG_PANEL_VKB_GTK_PANEL_ACTIVE_TEXT_COLOR   "/Panel/VKB/Gtk/Panel/Active/TextColor"
#define SCIM_CONFIG_PANEL_VKB_GTK_PANEL_ACTIVE_BG_COLOR     "/Panel/VKB/Gtk/Panel/Active/BGColor"


// default values

#define SCIM_DEFAULT_VKB_PROGRAM                            "fvkbd-gtk"
#define SCIM_DEFAULT_VKB_PARAMETERS                         "--xid"
#define SCIM_DEFAULT_VKB_WINDOW_WIDTH                       800
#define SCIM_DEFAULT_VKB_WINDOW_HEIGHT                      300
//#define SCIM_DEFAULT_VKB_WINDOW_HEIGHT                      200
#define SCIM_DEFAULT_VKB_DOCKPANEL                          true
#define SCIM_DEFAULT_VKB_PANEL_EXTEND                       true
#define SCIM_DEFAULT_VKB_PANEL_NORMAL_TEXT_COLOR            "#FFFFFF"
#define SCIM_DEFAULT_VKB_PANEL_NORMAL_FG_COLOR              "#2e2e2e"
#define SCIM_DEFAULT_VKB_PANEL_NORMAL_BG_COLOR              "#2e2e2e"
//#define SCIM_DEFAULT_VKB_PANEL_NORMAL_BG_COLOR              "#ffffff"
#define SCIM_DEFAULT_VKB_PANEL_ACTIVE_TEXT_COLOR            "light blue"
#define SCIM_DEFAULT_VKB_PANEL_ACTIVE_BG_COLOR              "black"
#define SCIM_DEFAULT_VKB_PANEL_FONT                         "droid sans bold 16"
#define SCIM_DEFAULT_VKB_PANEL_TOOLBAR_AUTO_SNAP            false
#define SCIM_DEFAULT_VKB_PANEL_TOOLBAR_HIDE_TIMEOUT         2
#define SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_FACTORY_ICON    true
#define SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_FACTORY_NAME    true
#define SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_MENU_ICON       true
#define SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_HELP_ICON       false
#define SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_PROPERTY_LABEL  true
#define SCIM_DEFAULT_VKB_PANEL_VKB_ENABLED                  true

#endif // __PANEL_SETTINGS_H

