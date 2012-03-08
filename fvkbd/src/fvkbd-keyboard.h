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

#ifndef _FVKBD_KEYBOARD_H
#define _FVKBD_KEYBOARD_H


#include "fvkbd-base.h"

G_BEGIN_DECLS

typedef struct _FvkbdKeyboard FvkbdKeyboard;
typedef struct _FvkbdKeyboardPrivate FvkbdKeyboardPrivate;
typedef struct _FvkbdKeyboardClass FvkbdKeyboardClass;

#define FVKBD_TYPE_KEYBOARD			(fvkbd_keyboard_get_type())
#define FVKBD_KEYBOARD(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_KEYBOARD, FvkbdKeyboard))
#define FVKBD_KEYBOARD_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_KEYBOARD, FvkbdKeyboardClass))
#define FVKBD_IS_KEYBOARD(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_KEYBOARD))
#define FVKBD_IS_KEYBOARD_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_KEYBOARD))
#define FVKBD_KEYBOARD_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_KEYBOARD, FvkbdKeyboardClass))

struct _FvkbdKeyboard {
	FvkbdUnit parent;
	FvkbdKeyboardPrivate *priv;
};

struct _FvkbdKeyboardClass {
	FvkbdUnitClass parent;
};

GType fvkbd_keyboard_get_type (void);

typedef enum
{
	KEYBOARD_MODE_STATUS_ERROR = -1,
	KEYBOARD_MODE_STATUS_NORMAL = 0,
	KEYBOARD_MODE_STATUS_TEMP,
	KEYBOARD_MODE_STATUS_LOCK,

	KEYBOARD_MODE_STATUS_NUMBER
} KeyboardModeStatus;

FvkbdUnit *fvkbd_keyboard_new (void);

GSList *fvkbd_keyboard_get_panels (FvkbdKeyboard *keyboard);
gint fvkbd_keyboard_get_current_mode (FvkbdKeyboard *keyboard);
gint fvkbd_keyboard_get_previous_mode (FvkbdKeyboard *keyboard);
gint fvkbd_keyboard_set_mode (FvkbdKeyboard *keyboard, gint mode);
KeyboardModeStatus fvkbd_keyboard_get_mode_status(FvkbdKeyboard *keyboard);
gboolean fvkbd_keyboard_set_mode_status(FvkbdKeyboard *keyboard, KeyboardModeStatus status);
void fvkbd_keyboard_do_func (FvkbdKeyboard *keyboard, KbdFuncInfo *func);

G_END_DECLS

#endif //_FVKBD_KEYBOARD_H
