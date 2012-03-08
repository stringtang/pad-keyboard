/*
 * fvkbd-keyboard.c keyboard unit for fvkbd  
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

#include "fvkbd-keyboard.h"
#include "fvkbd-panel.h"
#include "misc-utility.h"


#define FVKBD_KEYBOARD_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), FVKBD_TYPE_KEYBOARD, FvkbdKeyboardPrivate))

struct _FvkbdKeyboardPrivate {
	gint modes_num;
	gint current_mode;
	gint previous_mode;
	KeyboardModeStatus mode_status;
	GSList *modes;
	GSList *panels;
};

G_DEFINE_TYPE (FvkbdKeyboard, fvkbd_keyboard, FVKBD_TYPE_UNIT)

typedef struct _FvkbdKeyboardMode FvkbdKeyboardMode;
struct _FvkbdKeyboardMode {
	gint id;
	gchar *name;
};

enum {
	KBD_FUNCTION,
	LAST_SIGNAL
};

static guint fvkbd_keyboard_signals[LAST_SIGNAL] = { 0 };

static gboolean fvkbd_keyboard_parse_xml (FvkbdUnit *unit, FvkbdParser *parser);


gint
fvkbd_keyboard_get_modes_number (FvkbdKeyboard *keyboard)
{
	FvkbdKeyboardPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEYBOARD(keyboard), -1);

	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);
	return priv->modes_num;
}


GSList *
fvkbd_keyboard_get_panels (FvkbdKeyboard *keyboard)
{
	FvkbdKeyboardPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEYBOARD(keyboard), NULL);

	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);
	return priv->panels;
}


gint
fvkbd_keyboard_get_current_mode (FvkbdKeyboard *keyboard)
{
	FvkbdKeyboardPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEYBOARD(keyboard), -1);
	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);

	return priv->current_mode;
}


gint
fvkbd_keyboard_get_previous_mode (FvkbdKeyboard *keyboard)
{
	FvkbdKeyboardPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEYBOARD(keyboard), -1);
	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);

	return priv->previous_mode;
}


gint
fvkbd_keyboard_set_mode (FvkbdKeyboard *keyboard, gint mode)
{
	FvkbdKeyboardPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEYBOARD(keyboard), -1);

	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);

	if (mode >= priv->modes_num)
		return -1;

	if(priv->current_mode == mode)
		return priv->current_mode;

	priv->previous_mode = priv->current_mode;
	priv->current_mode = mode;

	return priv->previous_mode;
}


KeyboardModeStatus
fvkbd_keyboard_get_mode_status(FvkbdKeyboard *keyboard)
{
	FvkbdKeyboardPrivate *priv;
	g_return_val_if_fail(FVKBD_IS_KEYBOARD(keyboard), KEYBOARD_MODE_STATUS_ERROR);

	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);
	return priv->mode_status;
}


gboolean
fvkbd_keyboard_set_mode_status(FvkbdKeyboard *keyboard, KeyboardModeStatus status)
{
	FvkbdKeyboardPrivate *priv;
	g_return_val_if_fail(FVKBD_IS_KEYBOARD(keyboard), FALSE);
	g_return_val_if_fail((status < KEYBOARD_MODE_STATUS_NUMBER) , FALSE);

	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);
	priv->mode_status = status;
	return TRUE;
}


void
fvkbd_keyboard_do_func (FvkbdKeyboard *keyboard, KbdFuncInfo *func)
{
	FvkbdKeyboardPrivate *priv;
	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);
	gboolean return_value = FALSE;

	g_signal_emit(keyboard, fvkbd_keyboard_signals[KBD_FUNCTION], 0, func, &return_value);
}


#define MIN_KEYBOARD_MODES 1
#define MAX_KEYBOARD_MODES 16


static gint
keyboard_parse_modes (FvkbdUnit *unit, FvkbdParser *parser, void *data)
{
	FvkbdKeyboard *keyboard = FVKBD_KEYBOARD(unit);
	FvkbdKeyboardPrivate *priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);

	FvkbdKeyboardMode *mode = NULL;
	gchar *tmp_str = NULL;
	gint id;
	gint ret = FALSE;

	if ((tmp_str = fvkbd_parser_get_attribute(parser, "id")) == NULL) {
		fvkbd_parser_set_error(parser, "Missing attribute : id");
		goto done;
	}

	id = atoi(tmp_str);
	if (id < 0) {
		fvkbd_parser_set_error(parser, "Invalid id value");
		goto done;
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "name")) == NULL) {
		fvkbd_parser_set_error(parser, "Missing attribure : name");
		goto done;
	}

	mode = g_new0(FvkbdKeyboardMode, 1);
	mode->id = id;
	mode->name = g_strdup(tmp_str);
	priv->modes = g_slist_append(priv->modes, mode);

	ret = TRUE;
done:
	g_free(tmp_str);
	return ret;
}


static gint
keyboard_parse_panel (FvkbdUnit *unit, FvkbdParser *parser, void *data)
{
	FvkbdKeyboard *keyboard = FVKBD_KEYBOARD(unit);
	FvkbdKeyboardPrivate *priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);
	FvkbdUnit *panel;
	gint ret = FALSE;

	panel = fvkbd_panel_new();
	fvkbd_unit_set_parent(panel, unit);
	if (fvkbd_unit_parse_xml(panel, parser) != TRUE) {
		g_object_unref(panel);
		goto done;
	}

	priv->panels = g_slist_append(priv->panels, panel);
	ret = TRUE;

done:
	return ret;
}


static UnitParserFunc keyboard_element_parsers_1[] = {
	{"mode", keyboard_parse_modes},
	{NULL}
};


static UnitParserFunc keyboard_element_parsers_2[] = {
	{"panel", keyboard_parse_panel},
	{NULL}
};


static void
fvkbd_keyboard_finalize (GObject *object)
{
	FvkbdKeyboard *kbd = FVKBD_KEYBOARD(object);
	FvkbdKeyboardPrivate *priv = FVKBD_KEYBOARD_GET_PRIVATE(kbd);
	GSList *tmp;
	FvkbdKeyboardMode *mode = NULL;

	for (tmp = priv->modes; tmp; tmp = tmp->next) {
		mode = tmp->data;
		g_free(mode->name);
		g_free(mode);
	}

	g_slist_free(priv->modes);
	priv->modes = NULL;

	for (tmp = priv->panels; tmp; tmp = tmp->next)
		g_object_unref(tmp->data);

	g_slist_free(priv->panels);
	priv->panels = NULL;

	G_OBJECT_CLASS(fvkbd_keyboard_parent_class)->finalize(object);
}


static void
fvkbd_keyboard_class_init (FvkbdKeyboardClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	FvkbdUnitClass *unit_class = FVKBD_UNIT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FvkbdKeyboardPrivate));

	gobject_class->finalize = fvkbd_keyboard_finalize;

	unit_class->parse_xml = fvkbd_keyboard_parse_xml;

	fvkbd_keyboard_signals[KBD_FUNCTION] =
		g_signal_new("kbd_func",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST,
		0,
		_fvkbd_boolean_handled_accumulator, NULL,
		_fvkbd_marshal_BOOLEAN__POINTER,
		G_TYPE_BOOLEAN, 1,
		G_TYPE_POINTER);

}


static void
fvkbd_keyboard_init (FvkbdKeyboard *self)
{
	FvkbdKeyboardPrivate *priv = FVKBD_KEYBOARD_GET_PRIVATE(self);
	FvkbdUnit *unit = FVKBD_UNIT(self);

	priv->modes_num = 0;
	priv->current_mode = 0;
	priv->previous_mode = 0;
	priv->mode_status = KEYBOARD_MODE_STATUS_NORMAL;

	fvkbd_set_unit_type(unit, UNIT_TYPE_KEYBOARD);
}


FvkbdUnit *
fvkbd_keyboard_new (void)
{
	return g_object_new(FVKBD_TYPE_KEYBOARD, NULL);
}


static gboolean fvkbd_keyboard_parse_xml (FvkbdUnit *unit, FvkbdParser *parser)
{
	FvkbdKeyboard *keyboard;
	FvkbdKeyboardPrivate *priv;
	UnitParserFunc *pfp;
	gchar *tmp_str = NULL;
	KbdFontInfo *font;
	gint modes_num;
	gint i;
	gint ret = FALSE;

	g_return_val_if_fail(FVKBD_IS_KEYBOARD(unit), -1);
	keyboard = FVKBD_KEYBOARD(unit);
	priv = FVKBD_KEYBOARD_GET_PRIVATE(keyboard);

	// parsing attr num_of modes
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "num_of_modes")) == NULL) {
		fvkbd_parser_set_error(parser, "Missing attribute : num_of_modes");
		goto done;
	}

	modes_num = atoi(tmp_str);
	if ((modes_num < MIN_KEYBOARD_MODES) || (modes_num > MAX_KEYBOARD_MODES)) {
		fvkbd_parser_set_error(parser, "num_of_modes out of valid range");
		goto done;
	}

	priv->modes_num = modes_num;

	// paring attr font, efont, ofont. parse_font_property could deal with NULL
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

	// parsing color attr
	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "panel_bgcolor")) == NULL) {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_PANEL_BG, kbd_get_default_kbd_bg_color());
	} else {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_PANEL_BG, kbd_str_to_color(tmp_str));
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "key_bgcolor")) == NULL) {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_BG, kbd_get_default_key_bg_color());
	} else {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_BG, kbd_str_to_color(tmp_str));
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "key_fgcolor")) == NULL) {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_FG, kbd_get_default_key_fg_color());
	} else {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_FG, kbd_str_to_color(tmp_str));
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "pop_bgcolor")) == NULL) {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_POP_BG, kbd_get_default_key_pop_bg_color());
	} else {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_POP_BG, kbd_str_to_color(tmp_str));
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "pop_fgcolor")) == NULL) {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_POP_FG, kbd_get_default_key_pop_fg_color());
	} else {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_POP_FG, kbd_str_to_color(tmp_str));
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "key_extra_fgcolor")) == NULL) {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_EXTRA_FG,
					kbd_get_default_key_extra_fg_color());
	} else {
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_EXTRA_FG, kbd_str_to_color(tmp_str));
	}

	// parsing children elements
	if (!fvkbd_parser_go_child_element(parser)) {
		fvkbd_parser_set_error(parser, "Missing children");
		goto done;
	}

	// parsing element group 1
	do {
		for (pfp = keyboard_element_parsers_1; pfp->node_name != NULL; pfp++) {
			if (!fvkbd_parser_element_match(parser, pfp->node_name))
				continue;

			if (pfp->parser(unit, parser, NULL) != TRUE)
				goto done;

			break;
		}

		if (pfp->node_name == NULL)
			break;
	} while (fvkbd_parser_element_next(parser));

	if (g_slist_length(priv->modes) != priv->modes_num) {
		fvkbd_parser_set_error(parser, "Mode elements' number not matching");
		goto done;
	}

	for (i = 0; i < g_slist_length(priv->modes); i++) {
		if (((FvkbdKeyboardMode *)g_slist_nth_data(priv->modes, i))->id == 0)
			break;
	}

	if (i == g_slist_length(priv->modes)) {
		fvkbd_parser_set_error(parser, "Could not find default mode with id \"0\"");
		goto done;
	}

	// parsing element group 2
	do {
		for (pfp = keyboard_element_parsers_2; pfp->node_name != NULL; pfp++) {
			if (!fvkbd_parser_element_match(parser, pfp->node_name))
				continue;

			if (pfp->parser(unit, parser, NULL) != TRUE)
				goto done;

			break;
		}
	} while(fvkbd_parser_element_next(parser));

	ret = TRUE;
done:
	g_free(tmp_str);
	fvkbd_parser_go_parent_element(parser);
	return ret;
}

