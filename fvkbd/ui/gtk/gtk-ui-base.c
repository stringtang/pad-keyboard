/*
 * gtk-ui-base.c base object utility for fvkbd gtk ui frontend
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

#include "gtk-ui-base.h"


G_DEFINE_ABSTRACT_TYPE (FvkbdGtkUI, fvkbd_gtk_ui, G_TYPE_OBJECT)

static gboolean fvkbd_gtk_ui_real_build (FvkbdGtkUI *ui, GtkWidget **widget);
static gboolean fvkbd_gtk_ui_real_allocate (FvkbdGtkUI *ui, gint x, gint y,
						gfloat x_ratio, gfloat y_ratio);
static gboolean fvkbd_gtk_ui_real_destroy (FvkbdGtkUI *ui);

GQuark quark_key_shape_bitmap = 0;
GQuark quark_key_bg_pixmap = 0;
GQuark quark_ui_font_descs = 0;


static void
fvkbd_gtk_ui_finalize(GObject *object)
{
	G_OBJECT_CLASS(fvkbd_gtk_ui_parent_class)->finalize(object);
}


static void
fvkbd_gtk_ui_class_init (FvkbdGtkUIClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = fvkbd_gtk_ui_finalize;

	klass->build = fvkbd_gtk_ui_real_build;
	klass->allocate = fvkbd_gtk_ui_real_allocate;
	klass->destroy = fvkbd_gtk_ui_real_destroy;

	quark_key_shape_bitmap = g_quark_from_static_string("gtk-ui-key-shape-bitmap");
	quark_key_bg_pixmap = g_quark_from_static_string("gtk-ui-key-bg-pixmap");
	quark_ui_font_descs = g_quark_from_static_string("gtk-ui-font-descs");
}


static void
fvkbd_gtk_ui_init (FvkbdGtkUI *ui)
{
	ui->x_ratio = 1;
	ui->y_ratio = 1;
	ui->parent_ui = NULL;
	ui->unit = NULL;
	ui->widget = NULL;
}



static gboolean
fvkbd_gtk_ui_real_build (FvkbdGtkUI *ui, GtkWidget **widget)
{
	if (widget)
		*widget = NULL;
	return TRUE;
}


static gboolean
fvkbd_gtk_ui_real_allocate (FvkbdGtkUI *ui, gint x, gint y, gfloat x_ratio, gfloat y_ratio)
{
	ui->x_ratio = x_ratio;
	ui->y_ratio = y_ratio;
	return FALSE;
}


static gboolean
fvkbd_gtk_ui_real_destroy (FvkbdGtkUI *ui)
{
	return FALSE;
}


gboolean
fvkbd_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget)
{
	FvkbdGtkUIClass *klass;

	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), FALSE);
	klass = FVKBD_GTK_UI_GET_CLASS(ui);

	return klass->build(ui, widget);
}


gboolean
fvkbd_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y, gfloat x_ratio, gfloat y_ratio)
{
	FvkbdGtkUIClass *klass;

	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), FALSE);
	klass = FVKBD_GTK_UI_GET_CLASS(ui);

	return klass->allocate(ui, x, y, x_ratio, y_ratio);
}


gboolean
fvkbd_gtk_ui_destroy (FvkbdGtkUI *ui)
{
	FvkbdGtkUIClass *klass;

	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), FALSE);
	klass = FVKBD_GTK_UI_GET_CLASS(ui);

	return klass->destroy(ui);
}


void
fvkbd_gtk_ui_set_parent (FvkbdGtkUI *ui, FvkbdGtkUI *parent)
{
	g_return_if_fail(FVKBD_IS_GTK_UI(ui));
	ui->parent_ui = parent;
}


FvkbdGtkUI *
fvkbd_gtk_ui_get_parent (FvkbdGtkUI *ui)
{
	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), NULL);
	return ui->parent_ui;
}


void
fvkbd_gtk_ui_set_unit (FvkbdGtkUI *ui, FvkbdUnit *unit)
{
	g_return_if_fail(FVKBD_IS_GTK_UI(ui));
	ui->unit = unit;
}


FvkbdUnit *
fvkbd_gtk_ui_get_unit (FvkbdGtkUI *ui)
{
	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), NULL);
	return ui->unit;
}


gboolean
fvkbd_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id)
{
	FvkbdGtkUIClass *klass;

	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), FALSE);
	klass = FVKBD_GTK_UI_GET_CLASS(ui);

	if (!klass->set_mode)
		return TRUE;

	return klass->set_mode(ui, id);
}


void
fvkbd_gtk_ui_set_widget (FvkbdGtkUI *ui, GtkWidget *widget)
{
	g_return_if_fail(FVKBD_IS_GTK_UI(ui));
	ui->widget = widget;
}


GtkWidget *
fvkbd_gtk_ui_get_widget (FvkbdGtkUI *ui)
{
	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), NULL);
	return ui->widget;
}


gint
fvkbd_gtk_ui_set_ratio (FvkbdGtkUI *ui, gfloat x_ratio, gfloat y_ratio)
{
	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), -1);
	ui->x_ratio = x_ratio;
	ui->y_ratio = y_ratio;
	return 0;
}


gint
fvkbd_gtk_ui_get_ratio (FvkbdGtkUI *ui, gfloat *x_ratio, gfloat *y_ratio)
{
	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), -1);
	if (x_ratio)
		*x_ratio = ui->x_ratio;
	if (y_ratio)
		*y_ratio = ui->y_ratio;
	return 0;
}


gpointer
_fvkbd_gtk_ui_get_qdata_full (FvkbdGtkUI *ui, GQuark quark,
				gboolean recursive, gboolean *from_parent)
{
	gpointer data;

	data = g_object_get_qdata(G_OBJECT(ui), quark);
	if (data)
		return data;

	if (!recursive || !ui->parent_ui)
		return NULL;

	if (from_parent)
		*from_parent = TRUE;

	return _fvkbd_gtk_ui_get_qdata_full(ui->parent_ui, quark, recursive, NULL);
}


gpointer
fvkbd_gtk_ui_get_qdata_recursive (FvkbdGtkUI *ui, GQuark quark,
					gboolean *from_parent)
{
	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), NULL);

	if (from_parent)
		*from_parent = FALSE;

	return _fvkbd_gtk_ui_get_qdata_full(ui, quark, TRUE, from_parent);
}


gpointer
fvkbd_gtk_ui_get_qdata (FvkbdGtkUI *ui, GQuark quark)
{
	g_return_val_if_fail(FVKBD_IS_GTK_UI(ui), NULL);

	return _fvkbd_gtk_ui_get_qdata_full(ui, quark, FALSE, NULL);
}


void
fvkbd_gtk_ui_set_qdata(FvkbdGtkUI *ui, GQuark quark, gpointer data,
				GDestroyNotify destroy)
{
	g_return_if_fail(FVKBD_IS_GTK_UI(ui));
	g_object_set_qdata_full(G_OBJECT(ui), quark, data, destroy);
}

