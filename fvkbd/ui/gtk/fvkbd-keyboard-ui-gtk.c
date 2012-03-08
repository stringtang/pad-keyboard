/*
 * fvkbd-keyboard-ui-gtk.c keyboard unit gtk ui for fvkbd  
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

#include <gtk/gtk.h>
#include "fvkbd.h"

#include "fvkbd-keyboard.h"
#include "fvkbd-panel-ui-gtk.h"
#include "gtk-misc-utility.h"

#include "fvkbd-keyboard-ui-gtk.h"

#define FVKBD_KEYBOARD_GTK_UI_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), FVKBD_TYPE_KEYBOARD_GTK_UI, FvkbdKeyboardGtkUIPrivate))

typedef struct _FvkbdKeyboardGtkUIPrivate FvkbdKeyboardGtkUIPrivate;
struct _FvkbdKeyboardGtkUIPrivate {
};


G_DEFINE_TYPE (FvkbdKeyboardGtkUI, fvkbd_keyboard_gtk_ui, FVKBD_TYPE_GTK_UI);

FvkbdKeyboardGtkUI *the_keyboard_ui = NULL;

static gboolean fvkbd_keyboard_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget);
static gboolean fvkbd_keyboard_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y,
					gfloat x_ratio, gfloat y_ratio);
static gboolean fvkbd_keyboard_gtk_ui_destroy (FvkbdGtkUI *ui);
static gboolean fvkbd_keyboard_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id);


static void
fvkbd_keyboard_gtk_ui_finalize (GObject *object)
{
	the_keyboard_ui = NULL;
	G_OBJECT_CLASS(fvkbd_keyboard_gtk_ui_parent_class)->finalize(object);
}

static void
fvkbd_keyboard_gtk_ui_class_init (FvkbdKeyboardGtkUIClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	FvkbdGtkUIClass *ui_class = FVKBD_GTK_UI_CLASS(klass);

	gobject_class->finalize = fvkbd_keyboard_gtk_ui_finalize;

	ui_class->build = fvkbd_keyboard_gtk_ui_build;
	ui_class->destroy = fvkbd_keyboard_gtk_ui_destroy;
	ui_class->allocate = fvkbd_keyboard_gtk_ui_allocate;
	ui_class->set_mode = fvkbd_keyboard_gtk_ui_set_mode;
}


static void
fvkbd_keyboard_gtk_ui_init (FvkbdKeyboardGtkUI *self)
{
	self->children = NULL;
}


FvkbdGtkUI *
fvkbd_keyboard_gtk_ui_new (FvkbdUnit *unit)
{
	FvkbdGtkUI *ui;

	if (the_keyboard_ui != NULL)
		return FVKBD_GTK_UI(the_keyboard_ui);

	g_return_val_if_fail(FVKBD_IS_KEYBOARD(unit), NULL);

	the_keyboard_ui = g_object_new(FVKBD_TYPE_KEYBOARD_GTK_UI, NULL);
	ui = FVKBD_GTK_UI(the_keyboard_ui);
	fvkbd_gtk_ui_set_unit(ui, unit);
	fvkbd_unit_set_ui_data(ui->unit, ui);

	return ui;
}


static gboolean
fvkbd_keyboard_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget)
{
	FvkbdKeyboardGtkUI *self = FVKBD_KEYBOARD_GTK_UI(ui);
	GSList *panels;
	FvkbdUnit *panel;
	FvkbdGtkUI *child_ui = NULL;
	gint i;
	gboolean ret = FALSE;

	panels = fvkbd_keyboard_get_panels(FVKBD_KEYBOARD(ui->unit));
	if (panels == NULL)
		goto done;

	for (i = 0; i < g_slist_length(panels); i++) {
		panel = g_slist_nth_data(panels, i);
		switch (fvkbd_get_unit_type(panel)) {
		case UNIT_TYPE_PANEL:
			if ((fvkbd_get_unit_subtype(panel) != UNIT_SUBTYPE_PANEL_NORMAL))
				continue;

			child_ui = fvkbd_panel_gtk_ui_new(panel);
			fvkbd_gtk_ui_set_parent(child_ui, ui);
			if ((ret  = fvkbd_gtk_ui_build(child_ui, NULL)) != TRUE)
				goto done;

			self->children = g_slist_append(self->children, child_ui);

			break;
		default:
			break;
		}
	}

done:
	return ret;
}


static gboolean
fvkbd_keyboard_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y,
				gfloat x_ratio, gfloat y_ratio)
{
	fvkbd_gtk_ui_set_ratio(ui, x_ratio, y_ratio);
	return TRUE;
}


static gboolean
fvkbd_keyboard_gtk_ui_destroy (FvkbdGtkUI *ui)
{
	GSList *list;
	FvkbdKeyboardGtkUI *self = FVKBD_KEYBOARD_GTK_UI(ui);
	FvkbdGtkUI *child_ui;
	gint i;
	gboolean ret = TRUE;

	list = self->children;
	if (list == NULL)
		goto done;

	for (i = 0; i < g_slist_length(list); i++) {
		child_ui = g_slist_nth_data(list, i);
		if (fvkbd_gtk_ui_destroy(child_ui) != TRUE)
			ret  = FALSE;
	}

	g_slist_free(self->children);
	self->children = NULL;
	fvkbd_unit_set_ui_data(ui->unit, NULL);

done:
	g_object_unref(ui);
	return ret;
}


static gboolean
fvkbd_keyboard_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id)
{
	GSList *list;
	FvkbdKeyboardGtkUI *self = FVKBD_KEYBOARD_GTK_UI(ui);
	FvkbdGtkUI *child_ui;
	gint i;
	gboolean ret = TRUE;

	list = self->children;
	if (list == NULL)
		goto done;

	for (i = 0; i < g_slist_length(list); i++) {
		child_ui = g_slist_nth_data(list, i);
		if ( (fvkbd_gtk_ui_set_mode(child_ui, id)) != TRUE) {
			ret = FALSE;
			break;
		}
	}
done:
	return ret;
}


FvkbdKeyboardGtkUI *
fvkbd_keyboard_gtk_ui_get_ui (void)
{
	return the_keyboard_ui;
}


FvkbdKeyboard *
fvkbd_keyboard_gtk_ui_get_keyboard (void)
{
	if (the_keyboard_ui)
		return FVKBD_KEYBOARD(FVKBD_GTK_UI(the_keyboard_ui)->unit);

	return NULL;
}


gboolean
keyboard_ui_change_mode (FvkbdKeyboardGtkUI *keyboard_ui, gint id,
				KeyboardModeStatus status)
{
	FvkbdKeyboard *keyboard;
	gint ret = TRUE;
	gint p_mode;
	KeyboardModeStatus p_status;

	g_return_val_if_fail(FVKBD_IS_KEYBOARD_GTK_UI(keyboard_ui), FALSE);
	keyboard = FVKBD_KEYBOARD(FVKBD_GTK_UI(keyboard_ui)->unit);

	DBG("current mode = %d, new mode = %d, new mode_status = %d\n",
		fvkbd_keyboard_get_current_mode(keyboard), id, status);

	p_status = fvkbd_keyboard_get_mode_status(keyboard);
	p_mode = fvkbd_keyboard_set_mode(keyboard, id);

	if (p_mode < 0)
		return FALSE;

	if (p_mode == id && p_status == status)
		return TRUE;

	fvkbd_keyboard_set_mode_status(keyboard, status);

	if ((fvkbd_gtk_ui_set_mode(FVKBD_GTK_UI(keyboard_ui), id)) != TRUE) {
		ret = FALSE;
		g_warning("Failed to switch to mode %d\n", id);
		fvkbd_keyboard_set_mode(keyboard, p_mode);
		fvkbd_keyboard_set_mode_status(keyboard, KEYBOARD_MODE_STATUS_NORMAL);
		//FIXME: we should also try to reset ui to previous mode too.
	}

	return ret;
}


gboolean
keyboard_ui_resume_default_mode (FvkbdKeyboardGtkUI *keyboard_ui)
{
	return keyboard_ui_change_mode(keyboard_ui, 0, KEYBOARD_MODE_STATUS_NORMAL);
}


gboolean
keyboard_ui_resume_previous_mode (FvkbdKeyboardGtkUI *keyboard_ui)
{
	FvkbdKeyboard *keyboard;
	gint p_mode;
  
	g_return_val_if_fail(FVKBD_IS_KEYBOARD_GTK_UI(keyboard_ui), FALSE);
	keyboard = FVKBD_KEYBOARD(FVKBD_GTK_UI(keyboard_ui)->unit);

	p_mode = fvkbd_keyboard_get_previous_mode(keyboard);
   

	return keyboard_ui_change_mode(keyboard_ui, p_mode, KEYBOARD_MODE_STATUS_NORMAL);
}

