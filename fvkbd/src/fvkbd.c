/*
 * fvkbd.c main entry for fvkbd  
 * The starting point of the application
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
#include "fvkbd-keyboard.h"
#include "layout-utility.h"
#include "parser-utility-libxml.h"

#define ROOT_NODE_NAME "keyboard"

FvkbdParser *kbd_parser;

KbdColor DEFAULT_KBD_BG_COLOR = {0x80, 0x80, 0x80, 0xFF};
KbdColor DEFAULT_KEY_BG_COLOR = {0x30, 0x30, 0x30, 0xFF};
KbdColor DEFAULT_KEY_FG_COLOR = {0xFF, 0xFF, 0x00, 0xFF};
KbdColor DEFAULT_KEY_EXTRA_FG_COLOR = {0x80, 0x80, 0x80, 0xFF};
KbdColor DEFAULT_KEY_POP_BG_COLOR = {0xFF, 0xFF, 0x00, 0xFF};
KbdColor DEFAULT_KEY_POP_FG_COLOR = {0x30, 0x30, 0x30, 0xFF};


#define DEFAULT_FONT_FAMILY	"sans"
#define DEFAULT_FONT_SIZE	12
#define DEFAULT_FONT_WEIGHT	"bold"

#define DEFAULT_EXTRA_FONT_FAMILY	"sans"
#define DEFAULT_EXTRA_FONT_SIZE		10
#define DEFAULT_EXTRA_FONT_WEIGHT	"normal"

#define DEFAULT_POP_FONT_FAMILY	"sans"
#define DEFAULT_POP_FONT_SIZE	12
#define DEFAULT_POP_FONT_WEIGHT	"bold"


static gchar *kbd_default_font_family = DEFAULT_FONT_FAMILY;
static gint kbd_default_font_size = DEFAULT_FONT_SIZE;
static gchar *kbd_default_font_weight = DEFAULT_FONT_WEIGHT;

static gchar *kbd_default_efont_family = DEFAULT_EXTRA_FONT_FAMILY;
static gint kbd_default_efont_size = DEFAULT_EXTRA_FONT_SIZE;
static gchar *kbd_default_efont_weight = DEFAULT_EXTRA_FONT_WEIGHT;

static gchar *kbd_default_ofont_family = DEFAULT_POP_FONT_FAMILY;
static gint kbd_default_ofont_size = DEFAULT_POP_FONT_SIZE;
static gchar *kbd_default_ofont_weight = DEFAULT_POP_FONT_WEIGHT;


KbdColor *
kbd_get_default_kbd_bg_color(void)
{
	KbdColor *color;

	color = g_new0(KbdColor, 1);
	*color = DEFAULT_KBD_BG_COLOR;
	return color;
}


KbdColor *
kbd_get_default_key_bg_color(void)
{
	KbdColor *color;

	color = g_new0(KbdColor, 1);
	*color = DEFAULT_KEY_BG_COLOR;
	return color;
}


KbdColor *
kbd_get_default_key_fg_color(void)
{
	KbdColor *color;

	color = g_new0(KbdColor, 1);
	*color = DEFAULT_KEY_FG_COLOR;
	return color;
}


KbdColor *
kbd_get_default_key_extra_fg_color(void)
{
	KbdColor *color;

	color = g_new0(KbdColor, 1);
	*color = DEFAULT_KEY_EXTRA_FG_COLOR;
	return color;
}


KbdColor *
kbd_get_default_key_pop_bg_color(void)
{
	KbdColor *color;

	color = g_new0(KbdColor, 1);
	*color = DEFAULT_KEY_POP_BG_COLOR;
	return color;
}


KbdColor *
kbd_get_default_key_pop_fg_color(void)
{
	KbdColor *color;

	color = g_new0(KbdColor, 1);
	*color = DEFAULT_KEY_POP_FG_COLOR;
	return color;
}


gchar *
kbd_get_default_font_family(KbdFontType type)
{
	switch (type) {
		case KBD_FONT_TYPE_NORMAL:
			return kbd_default_font_family;
		case KBD_FONT_TYPE_EXTRA:
			return kbd_default_efont_family;
		case KBD_FONT_TYPE_POP:
			return kbd_default_ofont_family;
		default:
			return NULL;
	}
}


gint
kbd_get_default_font_size(KbdFontType type)
{
	switch (type) {
		case KBD_FONT_TYPE_NORMAL:
			return kbd_default_font_size;
		case KBD_FONT_TYPE_EXTRA:
			return kbd_default_efont_size;
		case KBD_FONT_TYPE_POP:
			return kbd_default_ofont_size;
		default:
			return 0;
	}
}


gchar *
kbd_get_default_font_weight(KbdFontType type)
{
	switch (type) {
		case KBD_FONT_TYPE_NORMAL:
			return kbd_default_font_weight;
		case KBD_FONT_TYPE_EXTRA:
			return kbd_default_efont_weight;
		case KBD_FONT_TYPE_POP:
			return kbd_default_ofont_weight;
		default:
			return NULL;
	}
}

gchar *
kbd_init(gchar *layout_file)
{
	gchar *name;

	if((name = find_layout_file(layout_file)) == NULL)
		return NULL;

	// find all the other layout files in layout dirs
	find_layout_files();

	return name;
}


void
kbd_cleanup(void)
{
	STEP();
	fvkbd_parser_cleanup(kbd_parser);
}


FvkbdUnit *
kbd_load_keyboard(const gchar* name)
{
	const gchar *layout_file = NULL;
	FvkbdUnit *keyboard = NULL;

	DBG("layout name = %s", name);

	layout_file = get_layout_file_fullname(name);

	if (layout_file)
		DBG("layout file = %s", layout_file);
	else {
		g_fprintf(stderr, "Layout file not found or not readable\n");
		goto done;
	}


	if (!kbd_parser)
		kbd_parser = fvkbd_parser_new();
	if (!kbd_parser)
		goto done;

	if (fvkbd_parser_load_file(kbd_parser, layout_file) != 0) {
		goto done;
	}

	if (!fvkbd_parser_element_match(kbd_parser, ROOT_NODE_NAME)) {
		g_fprintf(stderr, "Document of wrong type, root node != %s\n", ROOT_NODE_NAME);
		goto done;
	}

	keyboard = fvkbd_keyboard_new();

	if (fvkbd_unit_parse_xml(keyboard, kbd_parser) != TRUE) {
		g_fprintf(stderr, "%s\n", kbd_parser->err_msg);
		g_object_unref(keyboard);
		keyboard = NULL;
		goto done;
	}

	add_recent_layout_file(name);
done:
	if (kbd_parser)
		fvkbd_parser_free_file(kbd_parser);

	return keyboard;
}

