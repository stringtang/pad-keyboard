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

#ifndef _FVKBD_KEYBOARD_UI_GTK_H
#define _FVKBD_KEYBOARD_UI_GTK_H

#include "gtk-ui-base.h"

G_BEGIN_DECLS

typedef struct _FvkbdKeyboardGtkUI FvkbdKeyboardGtkUI;
typedef struct _FvkbdKeyboardGtkUIClass FvkbdKeyboardGtkUIClass;

#define FVKBD_TYPE_KEYBOARD_GTK_UI		(fvkbd_keyboard_gtk_ui_get_type())
#define FVKBD_KEYBOARD_GTK_UI(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_KEYBOARD_GTK_UI, FvkbdKeyboardGtkUI))
#define FVKBD_KEYBOARD_GTK_UI_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_KEYBOARD_GTK_UI, FvkbdKeyboardGtkUIClass))
#define FVKBD_IS_KEYBOARD_GTK_UI(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_KEYBOARD_GTK_UI))
#define FVKBD_IS_KEYBOARD_GTK_UI_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_KEYBOARD_GTK_UI))
#define FVKBD_KEYBOARD_GTK_UI_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_KEYBOARD_GTK_UI, FvkbdKeyboardGtkUIClass))


struct _FvkbdKeyboardGtkUI
{
	FvkbdGtkUI parent;
	GSList *children;
};

struct _FvkbdKeyboardGtkUIClass
{
	FvkbdGtkUIClass parent_class;
};

GType fvkbd_keyboard_gtk_ui_get_type (void);

FvkbdGtkUI *fvkbd_keyboard_gtk_ui_new (FvkbdUnit *unit);

FvkbdKeyboardGtkUI *fvkbd_keyboard_gtk_ui_get_ui (void);
FvkbdKeyboard *fvkbd_keyboard_gtk_ui_get_keyboard (void);

gboolean keyboard_ui_change_mode (FvkbdKeyboardGtkUI *keyboard_ui,
					gint id, KeyboardModeStatus status);
gboolean keyboard_ui_resume_default_mode (FvkbdKeyboardGtkUI *keyboard_ui);
gboolean keyboard_ui_resume_previous_mode (FvkbdKeyboardGtkUI *keyboard_ui);

G_END_DECLS

#endif //_FVKBD_KEYBOARD_UI_GTK_H