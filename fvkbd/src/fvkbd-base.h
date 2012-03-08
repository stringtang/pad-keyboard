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

#ifndef _FVKBD_BASE_H
#define _FVKBD_BASE_H

#include "parser-utility-libxml.h"

G_BEGIN_DECLS

typedef struct _FvkbdUnit FvkbdUnit;
typedef struct _FvkbdUnitClass FvkbdUnitClass;

#define FVKBD_TYPE_UNIT			(fvkbd_unit_get_type())
#define FVKBD_UNIT(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), FVKBD_TYPE_UNIT, FvkbdUnit))
#define FVKBD_UNIT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), FVKBD_TYPE_UNIT, FvkbdUnitClass))
#define FVKBD_IS_UNIT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), FVKBD_TYPE_UNIT))
#define FVKBD_IS_UNIT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), FVKBD_TYPE_UNIT))
#define FVKBD_UNIT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), FVKBD_TYPE_UNIT, FvkbdUnitClass))

typedef enum
{
	FLAG_KEY_SHAPE_INFO = 1 << 0,
	FLAG_KEY_BG_FILE = 1 << 1,
	FLAG_FONT_INFO = 1 << 2
} FvkbdUnitFlags;

#define FVKBD_UNIT_FLAGS(u)			(FVKBD_UNIT(u)->flags)
#define FVKBD_UNIT_HAS_KEY_SHAPE_INFO(obj)	((FVKBD_UNIT_FLAGS(obj) & FLAG_KEY_SHAPE_INFO) != 0)
#define FVKBD_UNIT_HAS_KEY_BG_FILE(obj)		((FVKBD_UNIT_FLAGS(obj) & FLAG_KEY_BG_FILE) != 0)
#define FVKBD_UNIT_HAS_FONT_INFO(obj)		((FVKBD_UNIT_FLAGS(obj) & FLAG_FONT_INFO) != 0)

#define FVKBD_UNIT_SET_FLAG(u, flag)		(FVKBD_UNIT_FLAGS(u) |= (flag))
#define FVKBD_UNIT_UNSET_FLAG(u, flag)		(FVKBD_UNIT_FLAGS(u) &= ~(flag))

typedef enum _KbdColorType KbdColorType;
enum _KbdColorType {
	KBD_COLOR_TYPE_PANEL_BG = 0,
	KBD_COLOR_TYPE_KEY_BG,
	KBD_COLOR_TYPE_KEY_FG,
	KBD_COLOR_TYPE_KEY_EXTRA_FG,
	KBD_COLOR_TYPE_KEY_POP_BG,
	KBD_COLOR_TYPE_KEY_POP_FG,

	KBD_COLOR_TYPE_NUMBER
};

typedef struct _KbdColor KbdColor;
struct _KbdColor {
	gchar r;
	gchar g;
	gchar b;
	gchar a;
};

typedef enum _KbdFontType KbdFontType;
enum _KbdFontType {
	KBD_FONT_TYPE_NORMAL = 0,
	KBD_FONT_TYPE_EXTRA,
	KBD_FONT_TYPE_POP,

	KBD_FONT_TYPE_NUMBER
};

typedef struct _KbdFontInfo KbdFontInfo;
struct _KbdFontInfo {
	gchar *family;
	gchar *weight;
	gint size;
};

typedef enum _KbdShapeType KbdShapeType;
enum _KbdShapeType {
	KBD_SHAPE_NULL = 0,

	KBD_SHAPE_NONE,
	KBD_SHAPE_CHAMFER,
	KBD_SHAPE_BITMAP_MASK,

	KBD_SHAPE_NUMBER
};

typedef struct _KbdShapeInfo KbdShapeInfo;
struct _KbdShapeInfo {
	KbdShapeType shape_type;
	union
	{
		gchar *mask;
	} u;
};

typedef enum _KbdFuncType KbdFuncType;
enum _KbdFuncType {
	KBD_FUNC_NONE = 0,

	KBD_FUNC_ModShift = 1,
	KBD_FUNC_ModCaps,
	KBD_FUNC_ModControl,
	KBD_FUNC_ModAlt,

	KBD_FUNC_MODE_FIRST = 11,
	KBD_FUNC_MODE_LAST,
	KBD_FUNC_MODE_NEXT,
	KBD_FUNC_MODE_PREV,
	KBD_FUNC_MODE_SELECT,

	KBD_FUNC_EXIT = 21,
	KBD_FUNC_MENU,

	KBD_FUNC_LAST
};

typedef struct _KbdFuncInfo KbdFuncInfo;
struct _KbdFuncInfo {
	KbdFuncType type;
	void *data;
};


struct _FvkbdUnit {
	GObject parent;

	FvkbdUnit *parent_unit;
	gint type;
	gchar *name;
	guint32 uid;

	KbdColor *color[KBD_COLOR_TYPE_NUMBER];
	KbdFontInfo *font[KBD_FONT_TYPE_NUMBER];

	guint16 flags;

	void *ui_data;
};

struct _FvkbdUnitClass {
	GObjectClass parent;

	/* Virtual functions */
	gboolean (*parse_xml)		(FvkbdUnit *unit, FvkbdParser *parser);

	gint (*get_size)		(FvkbdUnit *unit, gint *w, gint *h);
	gint (*set_size)		(FvkbdUnit *unit, gint w, gint h);
	gint (*get_position)		(FvkbdUnit *unit, gint *x, gint *y);
	gint (*set_position)		(FvkbdUnit *unit, gint x, gint y);

	KbdColor * (*get_color)	(FvkbdUnit *unit, KbdColorType type);
	void (*set_color)		(FvkbdUnit *unit, KbdColorType type, KbdColor *color);
};

GType fvkbd_unit_get_type (void);

/* definination for fvkbd unit type */
#define UNIT_TYPE_MASK		0xFF000000
#define UNIT_SUBTYPE_MASK	0x00FF0000
#define UNIT_FLAG_MASK		0x0000FFFF

#define UNIT_TYPE_SHIFT		24
#define UNIT_SUBTYPE_SHIFT	16
#define UNIT_FLAG_SHIFT		0

/* type defination for UNIT_TYPE_MASK */
#define UNIT_TYPE_KEYBOARD	0x01
#define UNIT_TYPE_PANEL		0x02
#define UNIT_TYPE_KEY		0x03

/* sub type defination for UNIT_TYPE_PANEL */
#define UNIT_SUBTYPE_PANEL_NORMAL	0x01


typedef struct _UnitParserFunc UnitParserFunc;
struct _UnitParserFunc {
	gchar *node_name;
	gboolean (*parser)			(FvkbdUnit *unit, FvkbdParser *parser, void *data);
};

/* variable */
extern GQuark quark_key_shape_info;
extern GQuark quark_key_bg_file;

/* Public Functions */
gboolean fvkbd_unit_parse_xml (FvkbdUnit *unit, FvkbdParser *parser);

void fvkbd_unit_set_parent (FvkbdUnit *unit, FvkbdUnit *parent);
FvkbdUnit *fvkbd_unit_get_parent (FvkbdUnit *unit);

gint fvkbd_unit_get_size (FvkbdUnit *unit, gint *w, gint *h);
gint fvkbd_unit_set_size (FvkbdUnit *unit, gint w, gint h);
gint fvkbd_unit_get_position (FvkbdUnit *unit, gint *x, gint *y);
gint fvkbd_unit_set_position (FvkbdUnit *unit, gint x, gint y);

KbdColor *fvkbd_unit_get_color (FvkbdUnit *unit, KbdColorType type);
void fvkbd_unit_set_color (FvkbdUnit *unit, KbdColorType type, KbdColor *color);

KbdFontInfo *fvkbd_unit_get_font_recursive (FvkbdUnit *unit, KbdFontType type,
						gboolean *from_parent);
KbdFontInfo *fvkbd_unit_get_font (FvkbdUnit *unit, KbdFontType type);
void fvkbd_unit_set_font (FvkbdUnit *unit, KbdFontType type, KbdFontInfo *font);

gint fvkbd_unit_set_mode (FvkbdUnit *unit, gint id);

void fvkbd_unit_set_ui_data (FvkbdUnit *unit, void *data);
void *fvkbd_unit_get_ui_data (FvkbdUnit *unit);

gint fvkbd_get_unit_type(FvkbdUnit *unit);
void fvkbd_set_unit_type(FvkbdUnit *unit, gint value);
gint fvkbd_get_unit_subtype(FvkbdUnit *unit);
void fvkbd_set_unit_subtype(FvkbdUnit *unit, gint value);
gint fvkbd_get_unit_flag(FvkbdUnit *unit);
void fvkbd_set_unit_flag(FvkbdUnit *unit, gint value);

gpointer fvkbd_unit_get_qdata_recursive(FvkbdUnit *unit, GQuark quark,
					gboolean *from_parent);
gpointer fvkbd_unit_get_qdata(FvkbdUnit *unit, GQuark quark);
void fvkbd_unit_set_qdata(FvkbdUnit *unit, GQuark quark, gpointer data,
				GDestroyNotify destroy);

G_END_DECLS

#endif //_FVKBD_BASE_H