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

#ifndef _FVKBD_KEY_H
#define _FVKBD_KEY_H


#include "fvkbd-base.h"

G_BEGIN_DECLS

typedef struct _FvkbdKey FvkbdKey;
typedef struct _FvkbdKeyPrivate FvkbdKeyPrivate;
typedef struct _FvkbdKeyClass FvkbdKeyClass;

#define FVKBD_TYPE_KEY			(fvkbd_key_get_type())
#define FVKBD_KEY(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_KEY, FvkbdKey))
#define FVKBD_KEY_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_KEY, FvkbdKeyClass))
#define FVKBD_IS_KEY(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_KEY))
#define FVKBD_IS_KEY_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_KEY))
#define FVKBD_KEY_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_KEY, FvkbdKeyClass))

struct _FvkbdKey {
	FvkbdUnit parent;

	KbdShapeInfo shape_info;

	FvkbdKeyPrivate *priv;
};

struct _FvkbdKeyClass {
	FvkbdUnitClass parent;
};

GType fvkbd_key_get_type (void);

#define KEY_DEFAULT_WIDTH 50
#define KEY_DEFAULT_HEIGHT 40

typedef enum _KeyActionType KeyActionType;
enum _KeyActionType {
	KEY_ACTION_NONE = 0,
	KEY_ACTION_SYM,
	KEY_ACTION_STRING,
	KEY_ACTION_STRING_GROUP,
	KEY_ACTION_FUNC,
	KEY_ACTION_SCRIPT
};

typedef struct _FvkbdKeyAction FvkbdKeyAction;
struct _FvkbdKeyAction {
	gint mode_id;
	KeyActionType type;
	gchar *disp;
	gchar *sup_label;
	gchar *img;			//image for show on key
	gchar *img_dn;			//if img_dn is not set, show *img upon key press down
	gchar *shift;
	gchar *symbol123;

	gboolean unique_font;
	KbdFontInfo *font[KBD_FONT_TYPE_NUMBER];	//there is rare case that a key need to use different font setting for different action mode.

	union 
	{
		gchar *string;
		gchar **string_group;
		KeySym sym;
		KbdFuncInfo func;
	} u;
};

typedef struct {
	KbdFuncType type;
	gchar *name;
} KeyFuncName;

FvkbdUnit *fvkbd_key_new (void);

void fvkbd_key_send_utf8_string (gchar *string);
void fvkbd_key_send_xkeysym (KeySym sym);

gchar *fvkbd_key_get_disp (FvkbdKey *self, gint id);
gchar *fvkbd_key_get_image (FvkbdKey *self, gint id);

FvkbdKeyAction *fvkbd_key_get_action(FvkbdKey *self, gint id);
gboolean fvkbd_key_pop_notify_disabled(FvkbdKey *self);


G_END_DECLS

#endif //_FVKBD_KEY_H
