/*
 * fvkbd-panel.c panel unit for fvkbd  
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

#include "misc-utility.h"
#include "fvkbd-panel.h"
#include "fvkbd-key.h"

#define FVKBD_PANEL_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), FVKBD_TYPE_PANEL, FvkbdPanelPrivate))

struct _FvkbdPanelPrivate {
	PanelLayoutType layout;
	gint x;
	gint y;
	gint width;
	gint height;
	gchar *img;
	PanelDockType dock;

	GSList *children;
};

G_DEFINE_TYPE (FvkbdPanel, fvkbd_panel, FVKBD_TYPE_UNIT);

#define PANEL_DEFAULT_H_SPACING		5
#define PANEL_DEFAULT_V_SPACING		5
#define PANEL_DEFAULT_ROW_HEIGHT	40


typedef struct _PanelParserData PanelParserData;
struct _PanelParserData {
	gint current_x;
	gint current_y;
	gint max_x;
	gint max_y;
	gint h_spacing;
	gint v_spacing;
	gint row_height;
};

static gboolean fvkbd_panel_parse_xml (FvkbdUnit *unit, FvkbdParser *parser);
static gint fvkbd_panel_get_position (FvkbdUnit *unit, gint *x, gint *y);
static gint fvkbd_panel_get_size (FvkbdUnit *unit, gint *w, gint *h);
static gint fvkbd_panel_set_size (FvkbdUnit *unit, gint w, gint h);


gint
fvkbd_panel_get_docktype (FvkbdPanel *self)
{
	FvkbdPanelPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_PANEL(self), -1);

	priv = FVKBD_PANEL_GET_PRIVATE(self);

	return priv->dock;
}


gchar *
fvkbd_panel_get_img (FvkbdPanel *self)
{
	FvkbdPanelPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_PANEL(self), NULL);

	priv = FVKBD_PANEL_GET_PRIVATE(self);

	return priv->img;
}


GSList *
fvkbd_panel_get_children (FvkbdPanel *self)
{
	FvkbdPanelPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_PANEL(self), NULL);

	priv = FVKBD_PANEL_GET_PRIVATE(self);

	return priv->children;
}


static gboolean
panel_parse_key (FvkbdUnit *unit, FvkbdParser *parser, void *data)
{
	FvkbdPanel *panel = FVKBD_PANEL(unit);
	FvkbdPanelPrivate *priv = FVKBD_PANEL_GET_PRIVATE(panel);
	PanelParserData *pd = (PanelParserData *)data;
	FvkbdUnit *key;
	gint w,h;
	gint ret = FALSE;

	key = fvkbd_key_new();
	fvkbd_unit_set_parent(key, unit);
	if (fvkbd_unit_parse_xml(key, parser) != TRUE) {
		g_object_unref(key);
		goto done;
	}

	if (priv->layout == LAYOUT_TYPE_ROW) {
		fvkbd_unit_set_position(key,pd->current_x,pd->current_y);
		DBG("key_pos: x=%d,y=%d", pd->current_x, pd->current_y);

		if (fvkbd_unit_get_size(key, &w, &h) != 0) {
			fvkbd_parser_set_error(parser, "Could not get key size");
			g_object_unref(key);
			goto done;
		}

		pd->current_x += (w + pd->h_spacing);
	}

	priv->children = g_slist_append(priv->children, key);

	if (pd->current_x > pd->max_x)
		pd->max_x = pd->current_x;

	ret = TRUE;
done:
	return ret;
}


static gboolean
panel_parse_row (FvkbdUnit *unit, FvkbdParser *parser, void *data)
{
	PanelParserData *pd = (PanelParserData *)data;

	pd->current_x = pd->h_spacing;
	pd->current_y += (pd->row_height + pd->v_spacing);
	pd->max_y = pd->current_y;

	return TRUE;
}


static gboolean
panel_parse_space (FvkbdUnit *unit, FvkbdParser *parser, void *data)
{
	PanelParserData *pd = (PanelParserData *)data;
	gchar *tmp_str = NULL;
	gint value;

	gint ret = FALSE;

	if ((tmp_str = fvkbd_parser_get_attribute(parser, "width")) == NULL) {
		fvkbd_parser_set_error(parser, "Missing attribute : width");
		goto done;
	}

	value = atoi(tmp_str);
	if (value < 0) {
		fvkbd_parser_set_error(parser, "invalid value of width");
		goto done;
	}

	pd->current_x += value;
	if (pd->current_x > pd->max_x)
		pd->max_x = pd->current_x;

	ret = TRUE;
done:
	return ret;
}


static UnitParserFunc panel_xy_parsers[] = {
	{"key", panel_parse_key},
	{NULL}
};

static UnitParserFunc panel_row_parsers[] = {
	{"key", panel_parse_key},
	{"new_row", panel_parse_row},
	{"space", panel_parse_space},
	{NULL}
};


static void
fvkbd_panel_finalize (GObject *object)
{
	FvkbdPanel *panel = FVKBD_PANEL(object);
	FvkbdPanelPrivate *priv = FVKBD_PANEL_GET_PRIVATE(panel);
	GSList *tmp;

	if (FVKBD_UNIT_HAS_KEY_SHAPE_INFO(object)) {
		FVKBD_UNIT_UNSET_FLAG(FVKBD_UNIT(object), FLAG_KEY_SHAPE_INFO);
		fvkbd_unit_set_qdata(FVKBD_UNIT(object), quark_key_shape_info, NULL, NULL);
	}

	if (FVKBD_UNIT_HAS_KEY_BG_FILE(object)) {
		FVKBD_UNIT_UNSET_FLAG(FVKBD_UNIT(object), FLAG_KEY_BG_FILE);
		fvkbd_unit_set_qdata(FVKBD_UNIT(object), quark_key_bg_file, NULL, NULL);
	}

	for (tmp = priv->children; tmp; tmp = tmp->next)
		g_object_unref(tmp->data);

	g_slist_free(priv->children);
	priv->children = NULL;

	g_free(priv->img);

	G_OBJECT_CLASS(fvkbd_panel_parent_class)->finalize(object);
}


static void
fvkbd_panel_class_init (FvkbdPanelClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	FvkbdUnitClass *unit_class = FVKBD_UNIT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FvkbdPanelPrivate));

	gobject_class->finalize = fvkbd_panel_finalize;

	unit_class->parse_xml = fvkbd_panel_parse_xml;
	unit_class->get_position = fvkbd_panel_get_position;
	unit_class->get_size = fvkbd_panel_get_size;
	unit_class->set_size = fvkbd_panel_set_size;
}


static void
fvkbd_panel_init (FvkbdPanel *self)
{
	FvkbdPanelPrivate *priv = FVKBD_PANEL_GET_PRIVATE(self);
	FvkbdUnit *unit = FVKBD_UNIT(self);

	fvkbd_set_unit_type(unit, UNIT_TYPE_PANEL);
	fvkbd_set_unit_subtype(unit, UNIT_SUBTYPE_PANEL_NORMAL);

	priv->img = NULL;
}


FvkbdUnit *
fvkbd_panel_new (void)
{
	return g_object_new(FVKBD_TYPE_PANEL, NULL);
}


static gboolean
fvkbd_panel_parse_xml (FvkbdUnit *unit, FvkbdParser *parser)
{

	FvkbdPanel *panel;
	FvkbdPanelPrivate *priv;
	gchar *tmp_str = NULL;
	KbdFontInfo *font;
	UnitParserFunc *pfp = NULL;
	gint value;
	PanelParserData *parser_data = g_new0(PanelParserData, 1);
	gint ret = FALSE;

	g_return_val_if_fail(FVKBD_IS_PANEL(unit), -1);
	panel = FVKBD_PANEL(unit);
	priv = FVKBD_PANEL_GET_PRIVATE(panel);

	/* parsing layout attr  */
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "layout")) == NULL) {
		fvkbd_parser_set_error(parser, "Missing attribute : layout");
		goto done;
	}

	if (!g_strcmp0(tmp_str, "row")) {
		priv->layout = LAYOUT_TYPE_ROW;
	} else if (!g_strcmp0(tmp_str, "xy")) {
		priv->layout = LAYOUT_TYPE_XY;
	} else {
		fvkbd_parser_set_error(parser, "Invalid value for \"layout\"");
		goto done;
	}

	/* parsing dock attr */
	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "dock")) == NULL) {
		priv->dock = LAYOUT_DOCK_NONE;
	} else if (!g_strcmp0(tmp_str, "none")) {
		priv->dock = LAYOUT_DOCK_NONE;
	} else if (!g_strcmp0(tmp_str, "top")) {
		priv->dock = LAYOUT_DOCK_TOP;
	} else if (!g_strcmp0(tmp_str, "bottom")) {
		priv->dock = LAYOUT_DOCK_BOTTOM;
	} else if (!g_strcmp0(tmp_str, "left")) {
		priv->dock = LAYOUT_DOCK_LEFT;
	} else if (!g_strcmp0(tmp_str, "right")) {
		priv->dock = LAYOUT_DOCK_RIGHT;
	} else {
		fvkbd_parser_set_error(parser, "Invalid value for \"dock\"");
		goto done;
	}

	// parsing x/y attr
	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "x")) == NULL) {
		priv->x = 0;
	} else {
		value = atoi(tmp_str);
		if (value < 0)
			value = 0;
		priv->x = value;
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "y")) == NULL) {
		priv->y = 0;
	} else {
		value = atoi(tmp_str);
		if (value < 0)
			value = 0;
		priv->y = value;
	}

	// parsing width/height attr
	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "width")) == NULL) {
		if (priv->layout == LAYOUT_TYPE_XY) {
			fvkbd_parser_set_error(parser, "width attribute is needed for xy layout");
			goto done;
		}

		priv->width = 0;
	} else {
		value = atoi(tmp_str);
		if (value < 0) {
			fvkbd_parser_set_error(parser, "Invalid value for width");
			goto done;
		}

		priv->width = value;
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "height")) == NULL) {
		if (priv->layout == LAYOUT_TYPE_XY) {
			fvkbd_parser_set_error(parser, "height attribute is needed for xy layout");
			goto done;
		}

		priv->height = 0;
	} else {
		value = atoi(tmp_str);
		if (value < 0) {
			fvkbd_parser_set_error(parser, "Invalid value for height");
			goto done;
		}

		priv->height = value;
	}

	/* parsing img attr */
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "img");
	if (tmp_str != NULL) {
		priv->img = locate_img_file(tmp_str);
	}

	/* parsing key_bg_img attr */
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "key_bg_img");
	if (tmp_str != NULL) {
		gchar *bg_file;
		if ((bg_file = locate_img_file(tmp_str)) != NULL) {
			fvkbd_unit_set_qdata(unit, quark_key_bg_file, bg_file,
					(GDestroyNotify)g_free);
			FVKBD_UNIT_SET_FLAG(unit, FLAG_KEY_BG_FILE);
		}
	}

	// parsing color attr
	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "bgcolor")) != NULL)
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_PANEL_BG, kbd_str_to_color(tmp_str));

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "key_bgcolor")) != NULL)
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_BG, kbd_str_to_color(tmp_str));

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "key_fgcolor")) != NULL)
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_FG, kbd_str_to_color(tmp_str));

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "key_extra_fgcolor")) != NULL)
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_EXTRA_FG, kbd_str_to_color(tmp_str));

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "pop_bgcolor")) != NULL)
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_POP_BG, kbd_str_to_color(tmp_str));

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "pop_fgcolor")) != NULL)
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_POP_FG, kbd_str_to_color(tmp_str));

	// parsing font attr
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "font");
	if (tmp_str) {
		font = parse_font_property(tmp_str, kbd_get_default_font_family(KBD_FONT_TYPE_NORMAL),
				kbd_get_default_font_weight(KBD_FONT_TYPE_NORMAL),
				kbd_get_default_font_size(KBD_FONT_TYPE_NORMAL));

		fvkbd_unit_set_font(unit, KBD_FONT_TYPE_NORMAL, font);
	}

	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "efont");
	if (tmp_str) {
		font = parse_font_property(tmp_str, kbd_get_default_font_family(KBD_FONT_TYPE_EXTRA),
				kbd_get_default_font_weight(KBD_FONT_TYPE_EXTRA),
				kbd_get_default_font_size(KBD_FONT_TYPE_EXTRA));

		fvkbd_unit_set_font(unit, KBD_FONT_TYPE_EXTRA, font);
	}

	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "ofont");
	if (tmp_str) {
		font = parse_font_property(tmp_str, kbd_get_default_font_family(KBD_FONT_TYPE_POP),
				kbd_get_default_font_weight(KBD_FONT_TYPE_POP),
				kbd_get_default_font_size(KBD_FONT_TYPE_POP));

		fvkbd_unit_set_font(unit, KBD_FONT_TYPE_POP, font);
	}

	/* parsing default key shape attr */
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "key_shape");
	if (tmp_str != NULL) {
		KbdShapeInfo *shape_info;

		shape_info = parse_shape_info_property(tmp_str);
		if (!shape_info) {
			fvkbd_parser_set_error(parser, "Invalid shape type");
			goto done;
		}

		/* parsing detail shape data */
		g_free(tmp_str);
		tmp_str = fvkbd_parser_get_attribute(parser, "key_shape_data");
		parse_shape_data_property(shape_info, tmp_str);

		if (shape_info->shape_type != KBD_SHAPE_NULL) {
			fvkbd_unit_set_qdata(unit, quark_key_shape_info, shape_info,
						(GDestroyNotify)kbd_shape_info_destroy);
			FVKBD_UNIT_SET_FLAG(unit, FLAG_KEY_SHAPE_INFO);
		} else {
			g_slice_free(KbdShapeInfo, shape_info);
		}
	}

	// parsing children elements
	parser_data->current_x = PANEL_DEFAULT_H_SPACING;
	parser_data->current_y = PANEL_DEFAULT_V_SPACING;
	parser_data->max_x = parser_data->current_x;
	parser_data->max_y = parser_data->current_y;
	parser_data->h_spacing = PANEL_DEFAULT_H_SPACING;
	parser_data->v_spacing = PANEL_DEFAULT_V_SPACING;
	parser_data->row_height = PANEL_DEFAULT_ROW_HEIGHT;

	if (!fvkbd_parser_go_child_element(parser)) {
		fvkbd_parser_set_error(parser, "children missing");
		goto done;
	}

	do {
		if (priv->layout == LAYOUT_TYPE_ROW)
			pfp = panel_row_parsers;
		else if (priv->layout == LAYOUT_TYPE_XY)
			pfp = panel_xy_parsers;
	
		for (; pfp->node_name != NULL; pfp++) {
			if (!fvkbd_parser_element_match(parser, pfp->node_name))
				continue;

			if (pfp->parser(unit, parser, parser_data) != TRUE)
				goto done;
			break;
		}

		if (pfp->node_name == NULL) {
			fvkbd_parser_set_error(parser, "Unknown Element");
			goto done;
		}

	} while (fvkbd_parser_element_next(parser));

	// for row type layout, calculate the base size of the panel
	if (priv->layout == LAYOUT_TYPE_ROW) {
		parser_data->max_y += (parser_data->row_height + parser_data->v_spacing);
		fvkbd_unit_set_size(unit, parser_data->max_x, parser_data->max_y);
	}

	ret = TRUE;
done:
	g_free(tmp_str);
	g_free(parser_data);

	fvkbd_parser_go_parent_element(parser);
	return ret;
}


static gint
fvkbd_panel_get_position (FvkbdUnit *unit, gint *x, gint *y)
{
	FvkbdPanel *panel;
	FvkbdPanelPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_PANEL(unit), -1);
	panel = FVKBD_PANEL(unit);
	priv = FVKBD_PANEL_GET_PRIVATE(panel);

	*x = priv->x;
	*y = priv->y;

	return 0;
}


static gint
fvkbd_panel_get_size (FvkbdUnit *unit, gint *w, gint *h)
{
	FvkbdPanel *panel;
	FvkbdPanelPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_PANEL(unit), -1);
	panel = FVKBD_PANEL(unit);
	priv = FVKBD_PANEL_GET_PRIVATE(panel);

	*w = priv->width;
	*h = priv->height;

	return 0;
}


static gint
fvkbd_panel_set_size (FvkbdUnit *unit, gint w, gint h)
{
	FvkbdPanel *panel;
	FvkbdPanelPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_PANEL(unit), -1);
	panel = FVKBD_PANEL(unit);
	priv = FVKBD_PANEL_GET_PRIVATE(panel);

	priv->width = w;
	priv->height = h;

	return 0;
}


