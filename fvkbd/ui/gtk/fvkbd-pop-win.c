/*
 * fvkbd-pop-win.c pop window utility for fvkbd
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
#include "fvkbd-key.h"
#include "fvkbd-keyboard.h"
#include "fvkbd-keyboard-ui-gtk.h"
#include "misc-utility.h"
#include "pixmap-utility.h"

#include "fvkbd-pop-win.h"


typedef struct _CandidateItem CandidateItem;
struct _CandidateItem {
	GtkWidget *widget;
	GtkWidget *dummy_width_widget;
	GtkWidget *label;
	GtkWidget *separator;
	KeyActionType type;
	gchar *disp;
	union
	{
		gchar *string;
		KeySym sym;
		KbdFuncInfo func;
	} u;
};


typedef struct _PopWinData PopWinData;
struct _PopWinData {
	GtkWidget *window;
	GtkWidget *dummy_height_widget;
};

#define VS_IMAGE "vs_gray10.png"

static PopWinData *POP_WIN = NULL;
static CandidateItem candidate_items[MAX_POP_WIN_ITEMS];

static PopWinData *get_pop_win(void);
static void set_candidate_item_string(gint n, gchar *disp, gchar *str, PangoFontDescription *desc);
static void candidate_item_clicked_cb(GtkButton *button, CandidateItem *item);
static void init_candidate_items(void);


static void
set_candidate_item_dummy(gint n)
{
	GtkWidget *button;

	if (n >= MAX_POP_WIN_ITEMS)
		return;

	candidate_items[n].type = KEY_ACTION_NONE;
	button = candidate_items[n].label;
	gtk_button_set_label(GTK_BUTTON(button), "");
}


static void
set_candidate_item_string(gint n, gchar *disp, gchar *str, PangoFontDescription *desc)
{
	GtkWidget *button;
	GtkWidget *key_label;
	if (str == NULL || n >= MAX_POP_WIN_ITEMS)
		return;

	candidate_items[n].type = KEY_ACTION_STRING;
	candidate_items[n].u.string = str;
	button = candidate_items[n].label;
	gtk_button_set_label(GTK_BUTTON(button), disp);
	key_label = gtk_bin_get_child(GTK_BIN(button));
	gtk_widget_modify_font(key_label, desc);
}


static void
set_candidate_item_color(gint n, KbdColorType type, GdkColor *color)
{
	GtkWidget *button;
	GtkWidget *label;

	if (n >= MAX_POP_WIN_ITEMS)
		return;

	if ((type != KBD_COLOR_TYPE_KEY_POP_BG) && (type != KBD_COLOR_TYPE_KEY_POP_FG))
		return;

	button = candidate_items[n].label;
	if (type == KBD_COLOR_TYPE_KEY_POP_BG) {
		gtk_widget_modify_bg(button, GTK_STATE_NORMAL, color);
		gtk_widget_modify_bg(button, GTK_STATE_PRELIGHT, color);
		gtk_widget_modify_bg(button, GTK_STATE_ACTIVE, color);
	} else if (type == KBD_COLOR_TYPE_KEY_POP_FG){
		label = gtk_bin_get_child(GTK_BIN(button));
		if (label) {
			gtk_widget_modify_fg(label, GTK_STATE_NORMAL, color);
			gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, color);
			gtk_widget_modify_fg(label, GTK_STATE_ACTIVE, color);
		}
	}
}


static void
candidate_item_clicked_cb(GtkButton *button, CandidateItem *item)
{
	FvkbdKeyboard *keyboard = fvkbd_keyboard_gtk_ui_get_keyboard();
	FvkbdKeyboardGtkUI *keyboard_ui = fvkbd_keyboard_gtk_ui_get_ui();

	switch (item->type) {

	case KEY_ACTION_STRING:
		fvkbd_key_send_utf8_string(item->u.string);
		break;

	case KEY_ACTION_SYM:
		fvkbd_key_send_xkeysym(item->u.sym);
		break;

	default:
		break;
	}

	{
		GtkWidget *w = get_pop_window();
		if (GTK_WIDGET_VISIBLE(w))
			gtk_widget_hide(w);
	}

	if ((fvkbd_keyboard_get_current_mode(keyboard) ==  KEYBOARD_MODE_STATUS_TEMP))
		keyboard_ui_resume_default_mode(keyboard_ui);

}


static void
init_candidate_items(void)
{
	int i;
	GtkWidget *vbox, *hbox;
	GdkPixbuf *pixbuf = NULL;
	gchar *vs_file = NULL;
	GdkColor separator_color = { 0, 0xA000, 0xA000, 0xA000 };

	vs_file = locate_img_file(VS_IMAGE);
	if (vs_file)
		pixbuf = gdk_pixbuf_new_from_file(vs_file, NULL);
	g_free(vs_file);

	for (i = 0; i < MAX_POP_WIN_ITEMS; i++) {
		if (pixbuf) {
			candidate_items[i].separator = gtk_image_new_from_pixbuf(pixbuf);
		} else {
			candidate_items[i].separator = gtk_vseparator_new();
			gtk_widget_modify_bg(candidate_items[i].separator, GTK_STATE_NORMAL, &separator_color);
		}

		vbox = gtk_vbox_new(FALSE, 0);
		candidate_items[i].widget = vbox;

		hbox = gtk_hbox_new(FALSE, 0);
		candidate_items[i].dummy_width_widget = hbox;
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		candidate_items[i].label = gtk_button_new();
		gtk_box_pack_start(GTK_BOX(vbox), candidate_items[i].label, TRUE, TRUE, 0);

		gtk_button_set_relief(GTK_BUTTON(candidate_items[i].label), GTK_RELIEF_NONE);
		g_signal_connect(G_OBJECT(candidate_items[i].label), "clicked",
			G_CALLBACK(candidate_item_clicked_cb), &candidate_items[i]);
	}

	if (pixbuf)
		g_object_unref(pixbuf);
}


void
pop_win_set_height_request(int height)
{
	static int current_height = -1;
	PopWinData *pop_win;

	if (current_height == height)
		return;

	pop_win = get_pop_win();
	gtk_widget_set_size_request(pop_win->dummy_height_widget,
						-1, height);
	current_height = height;
}


void
pop_win_items_set_width_request(int width)
{
	int i;
	static int current_width = -1;

	if (current_width == width)
		return;

	for (i = 0; i < MAX_POP_WIN_ITEMS; i++)
		gtk_widget_set_size_request(candidate_items[i].dummy_width_widget,
						width, -1);
	current_width = width;
}


void
update_pop_win_item_none(void)
{
	int i;

	for (i = 0; i < MAX_POP_WIN_ITEMS; i++) {
		gtk_widget_hide(candidate_items[i].widget);
		gtk_widget_hide(candidate_items[i].separator);
	}
}


void
update_pop_win_item_string(gchar *disp, gchar *str, PangoFontDescription *desc,
				gboolean reverse)
{
	int i;

	if (!reverse) {
		set_candidate_item_dummy(0);
		set_candidate_item_string(1, disp, str, desc);
	} else {
		set_candidate_item_string(0, disp, str, desc);
		set_candidate_item_dummy(1);
	}

	gtk_widget_hide(candidate_items[0].separator);
	gtk_widget_show(candidate_items[0].widget);
	gtk_widget_show(candidate_items[1].separator);
	gtk_widget_show(candidate_items[1].widget);

	for (i = 2; i < MAX_POP_WIN_ITEMS; i++) {
		gtk_widget_hide(candidate_items[i].separator);
		gtk_widget_hide(candidate_items[i].widget);
	}
}


void
update_pop_win_item_string_group(gchar **strs, PangoFontDescription *desc,
					gboolean reverse)
{
	int i;
	int n = 0;
	gchar **tmp;

	if (strs == NULL)
		return;

	for (i = 0, tmp = strs; (i < (MAX_POP_WIN_ITEMS - 1)) && *tmp; i++, tmp++)
		n++;

	if (!reverse) {
		set_candidate_item_dummy(0);
		gtk_widget_show(candidate_items[0].widget);

		for (i = 1; i <= n; i++, strs++) {
			set_candidate_item_string(i, *strs, *strs, desc);
			gtk_widget_show(candidate_items[i].separator);
			gtk_widget_show(candidate_items[i].widget);
		}
	} else {
		for (i = 0; i < n; i++, strs++) {
			int index = n - 1 - i;
			set_candidate_item_string(index, *strs, *strs, desc);
			if (index != 0)
				gtk_widget_show(candidate_items[index].separator);
			else
				gtk_widget_hide(candidate_items[index].separator);
			gtk_widget_show(candidate_items[index].widget);
		}
		set_candidate_item_dummy(n);
		gtk_widget_show(candidate_items[n].separator);
		gtk_widget_show(candidate_items[n].widget);

	}

	for (i = n + 1; i < MAX_POP_WIN_ITEMS; i++) {
		gtk_widget_hide(candidate_items[i].separator);
		gtk_widget_hide(candidate_items[i].widget);
	}
}


void
update_pop_win_item_sym(gchar *disp, KeySym sym, PangoFontDescription *desc,
				gboolean reverse)
{
	int i;
	GtkWidget *button;
	GtkWidget *key_label;

	if (!reverse) {
		set_candidate_item_dummy(0);
		i = 1;
	} else {
		set_candidate_item_dummy(1);
		i = 0;
	}

	gtk_widget_hide(candidate_items[0].separator);
	gtk_widget_show(candidate_items[0].widget);
	gtk_widget_show(candidate_items[1].separator);
	gtk_widget_show(candidate_items[1].widget);

	candidate_items[i].type = KEY_ACTION_SYM;
	candidate_items[i].u.sym = sym;
	button = candidate_items[i].label;
	gtk_button_set_label(GTK_BUTTON(button), disp);
	key_label = gtk_bin_get_child(GTK_BIN(button));
	gtk_widget_modify_font(key_label, desc);
	gtk_widget_show(button);

	for (i = 2; i < MAX_POP_WIN_ITEMS; i++) {
		gtk_widget_hide(candidate_items[i].separator);
		gtk_widget_hide(candidate_items[i].widget);
	}

}


static void
pop_win_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{

	GdkBitmap *bitmap = NULL;
	bitmap = get_chamfered_rectangle_bitmap(allocation->width,
		allocation->height, 2);

	if (bitmap) {
		gtk_widget_shape_combine_mask(widget, bitmap, 0, 0);
		g_object_unref(bitmap);
	}
}

static void
init_pop_win(void)
{
	if (POP_WIN == NULL) {
		GtkWidget *vbox, *hbox;
		int i;

		POP_WIN = g_new0(PopWinData, 1);
		POP_WIN->window = gtk_window_new(GTK_WINDOW_POPUP);
		g_signal_connect(G_OBJECT(POP_WIN->window), "size-allocate",
				G_CALLBACK(pop_win_size_allocate), NULL);

		gtk_window_set_accept_focus(GTK_WINDOW(POP_WIN->window), FALSE);
		gtk_window_set_resizable(GTK_WINDOW(POP_WIN->window), FALSE);

		vbox = gtk_vbox_new(FALSE, 5);
		hbox = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

		POP_WIN->dummy_height_widget = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), POP_WIN->dummy_height_widget,
						FALSE, FALSE, 0);
		init_candidate_items();
		for (i = 0; i < MAX_POP_WIN_ITEMS; i++) {
			gtk_box_pack_start(GTK_BOX(hbox), candidate_items[i].separator,
						FALSE, FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), candidate_items[i].widget,
						FALSE, FALSE, 0);
		}

		gtk_widget_show_all(vbox);

		// we never show first separator;
		gtk_widget_hide(candidate_items[0].separator);

		gtk_container_add (GTK_CONTAINER(POP_WIN->window), vbox);
	}
}


static PopWinData *
get_pop_win(void)
{
	if (G_UNLIKELY(POP_WIN == NULL))
		init_pop_win();

	return POP_WIN;
}


GtkWidget *
get_pop_window(void)
{
	return get_pop_win()->window;
}


void
settle_pop_window_color(gint number, GdkColor *bgcolor, GdkColor *fgcolor)
{
	GtkWidget *w = get_pop_window();
	int i;

	gtk_widget_modify_bg(w, GTK_STATE_NORMAL, bgcolor);

	for (i = 0; i < number; i++) {
		set_candidate_item_color(i, KBD_COLOR_TYPE_KEY_POP_BG, bgcolor);
		set_candidate_item_color(i, KBD_COLOR_TYPE_KEY_POP_FG, fgcolor);
	}
}

