/*
 * fvkbd-key.c key unit for fvkbd  
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
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "misc-utility.h"
#include "fvkbd-keyboard.h"
#include "fvkbd-key.h"


#define FVKBD_KEY_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), FVKBD_TYPE_KEY, FvkbdKeyPrivate))

struct _FvkbdKeyPrivate {
	gint x;
	gint y;
	gint width;
	gint height;
	gboolean disable_pop;

	GSList *actions;
};

G_DEFINE_TYPE (FvkbdKey, fvkbd_key, FVKBD_TYPE_UNIT);

static KeyFuncName key_func_names[] = {
	{KBD_FUNC_NONE, "No Function Defined"},
	{KBD_FUNC_ModShift, "shift"},
	{KBD_FUNC_ModCaps, "caps"},
	{KBD_FUNC_ModControl, "control"},
	{KBD_FUNC_ModAlt, "alt"},

	{KBD_FUNC_MODE_FIRST, "mode:first"},
	{KBD_FUNC_MODE_LAST, "mode:last"},
	{KBD_FUNC_MODE_NEXT, "mode:next"},
	{KBD_FUNC_MODE_PREV, "mode:prev"},
	{KBD_FUNC_MODE_SELECT, "mode:"},

	{KBD_FUNC_EXIT, "exit"},
	{KBD_FUNC_MENU, "menu"},
	{KBD_FUNC_NONE, NULL}
};


/* vars for key press state */
gboolean fvkbd_has_key_held = FALSE;

static gboolean fvkbd_key_parse_xml (FvkbdUnit *unit, FvkbdParser *parser);
static gint fvkbd_key_get_size (FvkbdUnit *unit, gint *w, gint *h);
static gint fvkbd_key_set_size (FvkbdUnit *unit, gint w, gint h);
static gint fvkbd_key_get_position (FvkbdUnit *unit, gint *x, gint *y);
static gint fvkbd_key_set_position (FvkbdUnit *unit, gint x, gint y);


static void
action_free_func(KbdFuncInfo *func)
{
	switch (func->type) {
	case KBD_FUNC_MODE_SELECT:
		g_free(func->data);
	default:
		break;
	}
}


static void
action_free_font_info(FvkbdKeyAction *action)
{
	int i;

	for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++) {
		if (action->font[i]) {
			g_free(action->font[i]->family);
			g_free(action->font[i]->weight);
		}

		g_free(action->font[i]);
		action->font[i] = NULL;
	}
}


static void
fvkbd_key_action_free(FvkbdKeyAction *action)
{
	switch (action->type) {
	case KEY_ACTION_STRING:
	case KEY_ACTION_SCRIPT:

		g_free(action->u.string);
		break;

	case KEY_ACTION_STRING_GROUP:
		g_strfreev(action->u.string_group);
		break;

	case KEY_ACTION_FUNC:
		action_free_func(&(action->u.func));
		break;

	case KEY_ACTION_SYM:
		break;

	default:
		break;
	}

	g_free(action->disp);
	g_free(action->sup_label);
	g_free(action->img);
	g_free(action->img_dn);

	if (G_UNLIKELY(action->unique_font == TRUE))
		action_free_font_info(action);

	g_free(action);

}


static void
fvkbd_key_finalize (GObject *object)
{
	FvkbdKey *key = FVKBD_KEY(object);
	FvkbdKeyPrivate *priv = FVKBD_KEY_GET_PRIVATE(key);
	GSList *tmp;

	if (FVKBD_UNIT_HAS_KEY_SHAPE_INFO(object)) {
		FVKBD_UNIT_UNSET_FLAG(FVKBD_UNIT(object), FLAG_KEY_SHAPE_INFO);
		fvkbd_unit_set_qdata(FVKBD_UNIT(object), quark_key_shape_info, NULL, NULL);
	}

	if (FVKBD_UNIT_HAS_KEY_BG_FILE(object)) {
		FVKBD_UNIT_UNSET_FLAG(FVKBD_UNIT(object), FLAG_KEY_BG_FILE);
		fvkbd_unit_set_qdata(FVKBD_UNIT(object), quark_key_bg_file, NULL, NULL);
	}

	for (tmp = priv->actions; tmp; tmp = tmp->next)
		fvkbd_key_action_free(tmp->data);

	g_slist_free(priv->actions);
	priv->actions = NULL;

	G_OBJECT_CLASS(fvkbd_key_parent_class)->finalize(object);
}


static void
fvkbd_key_class_init (FvkbdKeyClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	FvkbdUnitClass *unit_class = FVKBD_UNIT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FvkbdKeyPrivate));

	gobject_class->finalize = fvkbd_key_finalize;

	unit_class->parse_xml = fvkbd_key_parse_xml;
	unit_class->get_size = fvkbd_key_get_size;
	unit_class->set_size = fvkbd_key_set_size;
	unit_class->get_position = fvkbd_key_get_position;
	unit_class->set_position = fvkbd_key_set_position;
}


static void
fvkbd_key_init (FvkbdKey *self)
{
	FvkbdKeyPrivate *priv;
	FvkbdUnit *unit = FVKBD_UNIT(self);
	priv = FVKBD_KEY_GET_PRIVATE(self);

	fvkbd_set_unit_type(unit, UNIT_TYPE_KEY);
}


FvkbdUnit *
fvkbd_key_new (void)
{
	return g_object_new(FVKBD_TYPE_KEY, NULL);
}


FvkbdKeyAction *
key_action_find_by_id (GSList *lists, gint id)
{
	gint i;
	FvkbdKeyAction *current = NULL;

	g_return_val_if_fail(lists != NULL, NULL);
	
	for (i = 0; i < g_slist_length(lists); i++) {
		current = (FvkbdKeyAction *)g_slist_nth_data(lists, i);
		if (current->mode_id == id)
			return current;
	}

	// if could not find the action with the id number, return the default action.
	return g_slist_nth_data(lists, 0);
}


static gboolean
key_actions_insert_sort_unique (GSList **lists_p, FvkbdKeyAction *action)
{
	gint i;
	gboolean ret = TRUE;
	FvkbdKeyAction *current = NULL;

	g_return_val_if_fail(lists_p != NULL, FALSE);
	
	for (i = 0; i < g_slist_length(*lists_p); i++) {
		current = (FvkbdKeyAction *)g_slist_nth_data(*lists_p, i);
		if (current->mode_id	> action->mode_id)
			break;

		if (current->mode_id	== action->mode_id) {
			ret = FALSE;
			goto done;
		}
	}

	*lists_p = g_slist_insert(*lists_p, action, i);
done:
	return ret;
}


static KbdFuncType
action_str_to_func (FvkbdKeyAction *action, gchar *name)
{
	KeyFuncName *p;
	int id;
	KbdFuncType ret = KBD_FUNC_NONE;

	for (p = key_func_names; p->name != NULL; p++) {
		if (!strncmp(name, p->name, strlen(p->name))) {
			ret = p->type;
			break;
		}
	}

	// continue to handle some special cases
	switch (ret) {
	case KBD_FUNC_MODE_SELECT:
		id = atoi(&name[5]);
		if (id < 0) {
			ret = KBD_FUNC_NONE;
			break;
		}

		action->u.func.data = g_new0(int, 1);
		*((int *)(action->u.func.data)) = id;
		break;
	default:
		break;
	}

	action->u.func.type = ret;
	return ret;
}


static gint
key_parse_action (FvkbdUnit *unit, FvkbdParser *parser, void *data)
{
	FvkbdKey *key = FVKBD_KEY(unit);
	FvkbdKeyPrivate *priv = FVKBD_KEY_GET_PRIVATE(key);
	FvkbdKeyAction *action = NULL;
	KbdFontInfo *font;
	gchar *tmp_str = NULL;
	gint id;
	gint ret = FALSE;

	action = g_new0(FvkbdKeyAction, 1);

	/* parsing mode_id attr */
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "mode_id")) == NULL) {
		id = 0;
	} else {
		id = atoi(tmp_str);
		if (id < 0) {
			fvkbd_parser_set_error(parser, "Invalid value for mode_id");
			goto done;
		}
	}

	action->mode_id = id;
	g_free(tmp_str);

	/* parsing type attr */
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "type")) == NULL) {
		action->type = KEY_ACTION_STRING;
	} else if (!g_strcmp0(tmp_str, "sym")) {
		action->type = KEY_ACTION_SYM;
	} else if (!g_strcmp0(tmp_str, "string")) {
		action->type = KEY_ACTION_STRING;
	} else if (!g_strcmp0(tmp_str, "strings")) {
		action->type = KEY_ACTION_STRING_GROUP;
	} else if (!g_strcmp0(tmp_str, "func")) {
		action->type = KEY_ACTION_FUNC;
	} else if (!g_strcmp0(tmp_str, "script")) {
		action->type = KEY_ACTION_SCRIPT;
	} else {
		fvkbd_parser_set_error(parser, "Invalid value for type");
		goto done;
	}

	g_free(tmp_str);

	/* parsing disp attr */
	tmp_str = fvkbd_parser_get_attribute(parser, "disp");
	action->disp = g_strdup(tmp_str);
	g_free(tmp_str);

	/* parsing superscript label attr */
	tmp_str = fvkbd_parser_get_attribute(parser, "sup_label");
	action->sup_label = g_strdup(tmp_str);
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "shift");
	action->shift = g_strdup(tmp_str);
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "symbol123");
	action->symbol123 = g_strdup(tmp_str);
	g_free(tmp_str);

	/* parsing img attr */
	tmp_str = fvkbd_parser_get_attribute(parser, "img");
	if (tmp_str != NULL) {
		action->img = locate_img_file(tmp_str);
	} else {
		action->img = g_strdup(tmp_str);
	}
	g_free(tmp_str);

	/* parsing img_dn attr */
	tmp_str = fvkbd_parser_get_attribute(parser, "img_dn");
	if (tmp_str != NULL) {
		action->img_dn = locate_img_file(tmp_str);
	} else {
		action->img_dn = g_strdup(tmp_str);
	}
	g_free(tmp_str);

	// parsing font attr
	tmp_str = fvkbd_parser_get_attribute(parser, "font");
	if (G_UNLIKELY(tmp_str)) {
		font = parse_font_property(tmp_str, kbd_get_default_font_family(KBD_FONT_TYPE_NORMAL),
				kbd_get_default_font_weight(KBD_FONT_TYPE_NORMAL),
				kbd_get_default_font_size(KBD_FONT_TYPE_NORMAL));

		action->font[KBD_FONT_TYPE_NORMAL] = font;
		action->unique_font = TRUE;
	}
	g_free(tmp_str);

	tmp_str = fvkbd_parser_get_attribute(parser, "efont");
	if (G_UNLIKELY(tmp_str)) {
		font = parse_font_property(tmp_str, kbd_get_default_font_family(KBD_FONT_TYPE_EXTRA),
				kbd_get_default_font_weight(KBD_FONT_TYPE_EXTRA),
				kbd_get_default_font_size(KBD_FONT_TYPE_EXTRA));

		action->font[KBD_FONT_TYPE_EXTRA] = font;
		action->unique_font = TRUE;
	}
	g_free(tmp_str);

	tmp_str = fvkbd_parser_get_attribute(parser, "ofont");
	if (G_UNLIKELY(tmp_str)) {
		font = parse_font_property(tmp_str, kbd_get_default_font_family(KBD_FONT_TYPE_POP),
				kbd_get_default_font_weight(KBD_FONT_TYPE_POP),
				kbd_get_default_font_size(KBD_FONT_TYPE_POP));

		action->font[KBD_FONT_TYPE_POP] = font;
		action->unique_font = TRUE;
	}
	g_free(tmp_str);


	/* read action's content */
	if ((tmp_str = fvkbd_parser_element_get_content(parser)) == NULL) {
		fvkbd_parser_set_error(parser, "Missing content");
		goto done;
	}

	switch (action->type) {
	case KEY_ACTION_STRING:
	case KEY_ACTION_SCRIPT:
		if (!g_utf8_validate(tmp_str, -1, NULL)) {
			fvkbd_parser_set_error(parser, "Not a valid utf8 string");
			goto done;
		}

		action->u.string = g_strdup(tmp_str);
		break;

	case KEY_ACTION_STRING_GROUP:
		if (!g_utf8_validate(tmp_str, -1, NULL)) {
			fvkbd_parser_set_error(parser, "Not a valid utf8 string");
			goto done;
		}

		action->u.string_group = g_strsplit(tmp_str, " ", 0);

		break;

	case KEY_ACTION_SYM:
		{
			KeySym sym = XStringToKeysym(tmp_str);
			if (sym == NoSymbol) {
				gchar *errmsg = g_strconcat("Unrecognized key sym : ", tmp_str, NULL);
				fvkbd_parser_set_error(parser, errmsg);
				g_free(errmsg);
				goto done;
			}

			action->u.sym = sym;
			break;
		}

	case KEY_ACTION_FUNC:
		if ((action_str_to_func(action, tmp_str)) == KBD_FUNC_NONE) {
			gchar *errmsg = g_strconcat("Unrecognized function : ", tmp_str, NULL);
			fvkbd_parser_set_error(parser, errmsg);
			g_free(errmsg);
			goto done;
		}

		break;

	default:
		break;
	}


	if (key_actions_insert_sort_unique(&priv->actions, action) == FALSE) {
		fvkbd_parser_set_error(parser, "Invalid action");
		goto done;
	}

	ret = TRUE;
done:
	g_free(tmp_str);
	return ret;
}


static UnitParserFunc key_element_parsers[] = {
	{"action", key_parse_action},
	{NULL}
};


static gboolean
fvkbd_key_parse_xml (FvkbdUnit *unit, FvkbdParser *parser)
{
	FvkbdKey *key;
	FvkbdKeyPrivate *priv;
	gchar *tmp_str = NULL;
	KbdFontInfo *font;
	gint value;
	UnitParserFunc *pfp;
	gint ret = FALSE;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), FALSE);
	key = FVKBD_KEY(unit);
	priv = FVKBD_KEY_GET_PRIVATE(key);

	// parsing x/y attr
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
		priv->width = KEY_DEFAULT_WIDTH;
	} else {
		value = atoi(tmp_str);
		if (value < 0){
			fvkbd_parser_set_error(parser, "Invalid value for width");
			goto done;
		}

		priv->width = value;
	}

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "height")) == NULL) {
		priv->height = KEY_DEFAULT_HEIGHT;
	} else {
		value = atoi(tmp_str);
		if (value < 0) {
			fvkbd_parser_set_error(parser, "Invalid value for height");
			goto done;
		}

		priv->height = value;
	}

	/* parsing bg_img attr */
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "bg_img");
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
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_BG, kbd_str_to_color(tmp_str));

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "fgcolor")) != NULL)
		fvkbd_unit_set_color(unit, KBD_COLOR_TYPE_KEY_FG, kbd_str_to_color(tmp_str));

	g_free(tmp_str);
	if ((tmp_str = fvkbd_parser_get_attribute(parser, "extra_fgcolor")) != NULL)
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

	/* parsing show_pop attr  */
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "pop");
	if (tmp_str != NULL) {
		if (!g_strcmp0(tmp_str, "none")) {
			priv->disable_pop = TRUE;
		}
	}

	/* parsing shape attr */
	g_free(tmp_str);
	tmp_str = fvkbd_parser_get_attribute(parser, "shape");
	if (tmp_str != NULL) {
		KbdShapeInfo *shape_info;

		shape_info = parse_shape_info_property(tmp_str);
		if (!shape_info) {
			fvkbd_parser_set_error(parser, "Invalid shape type");
			goto done;
		}

		/* parsing detail shape data */
		g_free(tmp_str);
		tmp_str = fvkbd_parser_get_attribute(parser, "shape_data");
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
	if (!fvkbd_parser_go_child_element(parser)) {
		fvkbd_parser_set_error(parser, "Missing children");
		goto done;
	}

	do {
		for (pfp = key_element_parsers; pfp->node_name != NULL; pfp++) {
			if (!fvkbd_parser_element_match(parser, pfp->node_name))
				continue;

			if (pfp->parser(unit, parser, NULL) != TRUE)
				goto done;

			break;
		}

		if (pfp->node_name == NULL) {
			fvkbd_parser_set_error(parser, "Unknown Element");
			goto done;
		}

	} while (fvkbd_parser_element_next(parser));

	if(priv->actions == NULL) {
		fvkbd_parser_set_error(parser, "no action found");
		goto done;
	}

	if (((FvkbdKeyAction *)g_slist_nth_data(priv->actions, 0))->mode_id != 0) {
		fvkbd_parser_set_error(parser, "Need to have a default action for mode_id \"0\"\n");
		goto done;
	}

	ret = TRUE;
done:
	g_free(tmp_str);
	fvkbd_parser_go_parent_element(parser);
	return ret;
}


static gint
fvkbd_key_get_size (FvkbdUnit *unit, gint *w, gint *h)
{
	FvkbdKey *key;
	FvkbdKeyPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), -1);
	key = FVKBD_KEY(unit);
	priv = FVKBD_KEY_GET_PRIVATE(key);

	*w = priv->width;
	*h = priv->height;

	return 0;
}


static gint
fvkbd_key_set_size (FvkbdUnit *unit, gint w, gint h)
{
	FvkbdKey *key;
	FvkbdKeyPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), -1);
	key = FVKBD_KEY(unit);
	priv = FVKBD_KEY_GET_PRIVATE(key);

	priv->width = w;
	priv->height = h;

	return 0;
}


static gint
fvkbd_key_get_position (FvkbdUnit *unit, gint *x, gint *y)
{
	FvkbdKey *key;
	FvkbdKeyPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), -1);
	key = FVKBD_KEY(unit);
	priv = FVKBD_KEY_GET_PRIVATE(key);

	*x = priv->x;
	*y = priv->y;

	return 0;
}


static gint
fvkbd_key_set_position (FvkbdUnit *unit, gint x, gint y)
{
	FvkbdKey *key;
	FvkbdKeyPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), -1);
	key = FVKBD_KEY(unit);
	priv = FVKBD_KEY_GET_PRIVATE(key);

	priv->x = x;
	priv->y = y;

	return 0;
}


gchar *
fvkbd_key_get_disp (FvkbdKey *self, gint id)
{
	FvkbdKeyPrivate *priv;
	FvkbdKeyAction *action;

	g_return_val_if_fail(FVKBD_IS_KEY(self), NULL);

	priv = FVKBD_KEY_GET_PRIVATE(self);

	if ((action = key_action_find_by_id(priv->actions, id)) == NULL)
		return NULL;

	return action->disp;
}


gchar *
fvkbd_key_get_image (FvkbdKey *self, gint id)
{
	FvkbdKeyPrivate *priv;
	FvkbdKeyAction *action;

	g_return_val_if_fail(FVKBD_IS_KEY(self), NULL);

	priv = FVKBD_KEY_GET_PRIVATE(self);

	if ((action = key_action_find_by_id(priv->actions, id)) == NULL)
		return NULL;

	return action->img;
}


void
fvkbd_key_send_utf8_string(gchar *string)
{
	gint i,ret = 1;
	gchar *p = string;

	for (i = g_utf8_strlen(string, -1); i > 0; i--) {
		ret = fakekey_press(get_fakekey_instance(),
				(unsigned char *)p, -1, 0);
		fakekey_release(get_fakekey_instance());

		if (ret == 0)
			break;

		p = g_utf8_find_next_char(p, NULL);
	}
	DBG("action: send string: %s, result:%d\n", string, ret);
}


void
fvkbd_key_send_xkeysym(KeySym sym)
{
	gint ret;

	ret = fakekey_press_keysym(get_fakekey_instance(), sym, 0);
	fakekey_release(get_fakekey_instance());
	DBG("action: send sym: %x, result:%d\n", (unsigned int)sym, ret);
}


FvkbdKeyAction *
fvkbd_key_get_action(FvkbdKey *self, gint id)
{
	FvkbdKeyPrivate *priv;

	g_return_val_if_fail(FVKBD_IS_KEY(self), NULL);

	priv = FVKBD_KEY_GET_PRIVATE(self);

	return key_action_find_by_id(priv->actions, id);
}

inline gboolean
fvkbd_key_pop_notify_disabled(FvkbdKey *self)
{
	FvkbdKeyPrivate *priv;
	priv = FVKBD_KEY_GET_PRIVATE(self);

	return priv->disable_pop;
}
