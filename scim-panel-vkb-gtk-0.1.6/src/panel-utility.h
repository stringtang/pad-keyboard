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

#ifndef __PANEL_UTILITY_H
#define __PANEL_UTILITY_H

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

void _scim_panel_vkb_gtk_set_strut (GtkWidget *widget, guint32 size,
                                    guint32 strut_start, guint32 strut_end);

#endif //__PANEL_UTILITY_H
