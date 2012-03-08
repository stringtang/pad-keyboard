/*
 * fvkbd-base.c base object utility for fvkbd  
 *
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
 * 
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


#include "fvkbd.h"

#include "fvkbd-base.h"


G_DEFINE_ABSTRACT_TYPE (FvkbdUnit, fvkbd_unit, G_TYPE_OBJECT)


static gboolean fvkbd_unit_real_parse_xml (FvkbdUnit *unit, FvkbdParser *parser);
static gint fvkbd_unit_real_get_size (FvkbdUnit *unit, gint *w, gint *h);
static gint fvkbd_unit_real_set_size (FvkbdUnit *unit, gint w, gint h);
static gint fvkbd_unit_real_get_position (FvkbdUnit *unit, gint *x, gint *y);
static gint fvkbd_unit_real_set_position (FvkbdUnit *unit, gint x, gint y);
static KbdColor *fvkbd_unit_real_get_color (FvkbdUnit *unit, KbdColorType type);
void fvkbd_unit_real_set_color (FvkbdUnit *unit, KbdColorType type, KbdColor *color);

GQuark quark_key_shape_info = 0;
GQuark quark_key_bg_file = 0;

static void
fvkbd_unit_finalize(GObject *object)
{
	int i;
	FvkbdUnit *unit = FVKBD_UNIT(object);

	g_free(unit->name);

	for (i = 0; i < KBD_COLOR_TYPE_NUMBER; i++)
		g_free(unit->color[i]);

	for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++) {
		if (unit->font[i]) {
			g_free(unit->font[i]->family);
			g_free(unit->font[i]->weight);
		}

		g_free(unit->font[i]);
	}

	G_OBJECT_CLASS(fvkbd_unit_parent_class)->finalize(object);
}


static void
fvkbd_unit_class_init (FvkbdUnitClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = fvkbd_unit_finalize;

	klass->parse_xml = fvkbd_unit_real_parse_xml;
	klass->get_size = fvkbd_unit_real_get_size;
	klass->set_size = fvkbd_unit_real_set_size;
	klass->get_position = fvkbd_unit_real_get_position;
	klass->set_position = fvkbd_unit_real_set_position;
	klass->get_color = fvkbd_unit_real_get_color;
	klass->set_color = fvkbd_unit_real_set_color;

	quark_key_shape_info = g_quark_from_static_string("fvkbd-key-shape-info");
	quark_key_bg_file = g_quark_from_static_string("fvkbd-key-bg-file");
}


static void
fvkbd_unit_init (FvkbdUnit *fvkbd_unit)
{
	static guint32 uid = 0;
	int i;

	fvkbd_unit->parent_unit = NULL;
	fvkbd_unit->flags = 0;
	fvkbd_unit->uid = uid++;

	for (i = 0; i < KBD_COLOR_TYPE_NUMBER; i++)
		fvkbd_unit->color[i] = NULL;

	for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++)
		fvkbd_unit->font[i] = NULL;
}


static gboolean
fvkbd_unit_real_parse_xml (FvkbdUnit *unit, FvkbdParser *parser)
{
	return TRUE;
}


static gint
fvkbd_unit_real_get_size (FvkbdUnit *unit, gint *w, gint *h)
{
	return -1;
}


static gint
fvkbd_unit_real_set_size (FvkbdUnit *unit, gint w, gint h)
{
	return 0;
}


static gint
fvkbd_unit_real_get_position (FvkbdUnit *unit, gint *x, gint *y)
{
	return -1;
}


static gint
fvkbd_unit_real_set_position (FvkbdUnit *unit, gint x, gint y)
{
	return 0;
}


static KbdColor *
fvkbd_unit_real_get_color (FvkbdUnit *unit, KbdColorType type)
{
	KbdColor *color = NULL;
	FvkbdUnit *parent;

	if (type >= KBD_COLOR_TYPE_NUMBER)
		goto done;

	if ((color = unit->color[type]) != NULL)
		goto done;

	parent = fvkbd_unit_get_parent(unit);
	return fvkbd_unit_get_color(parent, type);
done:
	return color;
}


void
fvkbd_unit_real_set_color (FvkbdUnit *unit, KbdColorType type, KbdColor *color)
{
	if (type >= KBD_COLOR_TYPE_NUMBER)
		return;

	if (unit->color[type] != NULL)
		g_free(unit->color[type]);

	unit->color[type] = color;
}


gboolean
fvkbd_unit_parse_xml (FvkbdUnit *unit, FvkbdParser *parser)
{
	FvkbdUnitClass *klass;

	g_return_val_if_fail(FVKBD_IS_UNIT(unit), FALSE);
	klass = FVKBD_UNIT_GET_CLASS(unit);

	return klass->parse_xml(unit, parser);
}


void
fvkbd_unit_set_parent (FvkbdUnit *unit, FvkbdUnit *parent)
{
	g_return_if_fail(FVKBD_IS_UNIT(unit));
	unit->parent_unit = parent;
}


FvkbdUnit *
fvkbd_unit_get_parent (FvkbdUnit *unit)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), NULL);
	return unit->parent_unit;
}


gint
fvkbd_unit_get_size (FvkbdUnit *unit, gint *w, gint *h)
{
	FvkbdUnitClass *klass;

	g_return_val_if_fail(FVKBD_IS_UNIT(unit), -1);
	klass = FVKBD_UNIT_GET_CLASS(unit);

	return klass->get_size(unit, w, h);
}


gint
fvkbd_unit_set_size (FvkbdUnit *unit, gint w, gint h)
{
	FvkbdUnitClass *klass;

	g_return_val_if_fail(FVKBD_IS_UNIT(unit), -1);
	klass = FVKBD_UNIT_GET_CLASS(unit);

	return klass->set_size(unit, w, h);
}


gint
fvkbd_unit_get_position (FvkbdUnit *unit, gint *x, gint *y)
{
	FvkbdUnitClass *klass;

	g_return_val_if_fail(FVKBD_IS_UNIT(unit), -1);
	klass = FVKBD_UNIT_GET_CLASS(unit);

	return klass->get_position(unit, x, y);
}


gint
fvkbd_unit_set_position (FvkbdUnit *unit, gint x, gint y)
{
	FvkbdUnitClass *klass;

	g_return_val_if_fail(FVKBD_IS_UNIT(unit), -1);
	klass = FVKBD_UNIT_GET_CLASS(unit);

	return klass->set_position(unit, x, y);
}


KbdColor *
fvkbd_unit_get_color (FvkbdUnit *unit, KbdColorType type)
{
	FvkbdUnitClass *klass;

	g_return_val_if_fail(FVKBD_IS_UNIT(unit), NULL);
	klass = FVKBD_UNIT_GET_CLASS(unit);

	return klass->get_color(unit, type);
}


void
fvkbd_unit_set_color (FvkbdUnit *unit, KbdColorType type, KbdColor *color)
{
	FvkbdUnitClass *klass;

	g_return_if_fail(FVKBD_IS_UNIT(unit));
	klass = FVKBD_UNIT_GET_CLASS(unit);

	klass->set_color(unit, type, color);
}


void
fvkbd_unit_set_ui_data (FvkbdUnit *unit, void *data)
{
	g_return_if_fail(FVKBD_IS_UNIT(unit));
	unit->ui_data = data;
}


void *
fvkbd_unit_get_ui_data (FvkbdUnit *unit)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), NULL);
	return unit->ui_data;
}


gint
fvkbd_get_unit_type(FvkbdUnit *unit)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), -1);
	return ((unit->type & UNIT_TYPE_MASK) >> UNIT_TYPE_SHIFT);
}


void
fvkbd_set_unit_type(FvkbdUnit *unit, gint value)
{
	g_return_if_fail(FVKBD_IS_UNIT(unit));
	unit->type = (unit->type & ~UNIT_TYPE_MASK)
			| (UNIT_TYPE_MASK & (value << UNIT_TYPE_SHIFT));
}


gint
fvkbd_get_unit_subtype(FvkbdUnit *unit)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), -1);
	return ((unit->type & UNIT_SUBTYPE_MASK) >> UNIT_SUBTYPE_SHIFT);
}


void
fvkbd_set_unit_subtype(FvkbdUnit *unit, gint value)
{
	g_return_if_fail(FVKBD_IS_UNIT(unit));

	unit->type = (unit->type & ~UNIT_SUBTYPE_MASK)
			| (UNIT_SUBTYPE_MASK & (value << UNIT_SUBTYPE_SHIFT));
}


gint
fvkbd_get_unit_flag(FvkbdUnit *unit)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), -1);
	return ((unit->type & UNIT_FLAG_MASK) >> UNIT_FLAG_SHIFT);
}


void
fvkbd_set_unit_flag(FvkbdUnit *unit, gint value)
{
	g_return_if_fail(FVKBD_IS_UNIT(unit));

	unit->type = (unit->type & ~UNIT_FLAG_MASK)
			| (UNIT_FLAG_MASK & (value << UNIT_FLAG_SHIFT));
}


static KbdFontInfo *
_fvkbd_unit_get_font_full (FvkbdUnit *unit, KbdFontType type,
				gboolean recursive, gboolean *from_parent)
{
	KbdFontInfo *font = unit->font[type];

	if (font)
		return font;

	if (!recursive || !unit->parent_unit)
		return NULL;

	if (from_parent)
		*from_parent = TRUE;

	return _fvkbd_unit_get_font_full(unit->parent_unit, type, recursive, NULL);
}


KbdFontInfo *
fvkbd_unit_get_font_recursive (FvkbdUnit *unit, KbdFontType type,
				gboolean *from_parent)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), NULL);
	if ((type < 0) || (type >= KBD_FONT_TYPE_NUMBER))
		return NULL;

	if (from_parent)
		*from_parent = FALSE;

	return _fvkbd_unit_get_font_full(unit, type, TRUE, from_parent);
}


KbdFontInfo *
fvkbd_unit_get_font (FvkbdUnit *unit, KbdFontType type)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), NULL);
	if ((type < 0) || (type >= KBD_FONT_TYPE_NUMBER))
		return NULL;

	return _fvkbd_unit_get_font_full(unit, type, FALSE, NULL);
}


void
fvkbd_unit_set_font (FvkbdUnit *unit, KbdFontType type, KbdFontInfo *font)
{
	if ((type < 0) || (type >= KBD_FONT_TYPE_NUMBER))
		return;

	if (unit->font[type] != NULL) {
		g_free(unit->font[type]->family);
		g_free(unit->font[type]->weight);
		g_free(unit->font[type]);
	}

	unit->font[type] = font;
	if (font)
		FVKBD_UNIT_SET_FLAG(unit, FLAG_FONT_INFO);
}


gpointer
_fvkbd_unit_get_qdata_full(FvkbdUnit *unit, GQuark quark,
				gboolean recursive, gboolean *from_parent)
{
	gpointer data;

	data = g_object_get_qdata(G_OBJECT(unit), quark);
	if (data)
		return data;

	if (!recursive || !unit->parent_unit)
		return NULL;

	if (from_parent)
		*from_parent = TRUE;

	return _fvkbd_unit_get_qdata_full(unit->parent_unit, quark, recursive, NULL);
}

gpointer
fvkbd_unit_get_qdata_recursive(FvkbdUnit *unit, GQuark quark,
					gboolean *from_parent)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), NULL);

	if (from_parent)
		*from_parent = FALSE;

	return _fvkbd_unit_get_qdata_full(unit, quark, TRUE, from_parent);
}

gpointer
fvkbd_unit_get_qdata(FvkbdUnit *unit, GQuark quark)
{
	g_return_val_if_fail(FVKBD_IS_UNIT(unit), NULL);

	return _fvkbd_unit_get_qdata_full(unit, quark, FALSE, NULL);
}

void
fvkbd_unit_set_qdata(FvkbdUnit *unit, GQuark quark, gpointer data,
				GDestroyNotify destroy)
{
	g_return_if_fail(FVKBD_IS_UNIT(unit));
	g_object_set_qdata_full(G_OBJECT(unit), quark, data, destroy);
}
