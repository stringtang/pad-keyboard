/*
 * panel-utilitys.cpp : trival function collect for panel usage.
 *
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

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "panel-utility.h"

static Atom net_wm_strut              = None;
static Atom net_wm_strut_partial      = None;

#define	STRUT_LEFT          0
#define	STRUT_RIGHT         1
#define	STRUT_TOP           2
#define	STRUT_BOTTOM        3
#define	STRUT_LEFT_START    4
#define	STRUT_LEFT_END      5
#define	STRUT_RIGHT_START   6
#define	STRUT_RIGHT_END     7
#define	STRUT_TOP_START     8
#define	STRUT_TOP_END       9
#define	STRUT_BOTTOM_START  10
#define	STRUT_BOTTOM_END    11

void
_scim_panel_vkb_gtk_set_strut (GtkWidget *widget, guint32 size,
                               guint32 strut_start, guint32 strut_end)
{
    Display *display;
	Window window;
    gulong struts [12] = { 0, };

    if (!GTK_WIDGET_REALIZED (widget))
        return;

	display = GDK_WINDOW_XDISPLAY (widget->window);
	window = GDK_WINDOW_XWINDOW (widget->window);

    if (net_wm_strut == None)
        net_wm_strut = XInternAtom (display, "_NET_WM_STRUT", False);
    if (net_wm_strut_partial == None)
        net_wm_strut_partial = XInternAtom (display, "_NET_WM_STRUT_PARTIAL", False);

    struts [STRUT_BOTTOM] = size;
    struts [STRUT_BOTTOM_START] = strut_start;
    struts [STRUT_BOTTOM_END] = strut_end;

    XChangeProperty (display, window, net_wm_strut,
            XA_CARDINAL, 32, PropModeReplace,
            (guchar *) &struts, 4);
    XChangeProperty (display, window, net_wm_strut_partial,
            XA_CARDINAL, 32, PropModeReplace,
            (guchar *) &struts, 12);
}
