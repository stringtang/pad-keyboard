/*
 * fvkbd-panel-ui-gtk.c panel unit gtk ui for fvkbd  
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
#include "gtk-misc-utility.h"

#include "fvkbd-panel-ui-gtk.h"
#include "fvkbd-key-ui-gtk.h"


#define GTK_VKB_PANEL_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_VKB_PANEL, GtkVkbPanelPrivate))

typedef struct _GtkVkbPanelPrivate GtkVkbPanelPrivate;
struct _GtkVkbPanelPrivate {
};

G_DEFINE_TYPE (GtkVkbPanel, gtk_vkb_panel, GTK_TYPE_FIXED);


#define FVKBD_PANEL_GTK_UI_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), FVKBD_TYPE_PANEL_GTK_UI, FvkbdPanelGtkUIPrivate))

typedef struct _FvkbdPanelGtkUIPrivate FvkbdPanelGtkUIPrivate;
struct _FvkbdPanelGtkUIPrivate {
	gfloat res_x_ratio;
	gfloat res_y_ratio;
};


G_DEFINE_TYPE (FvkbdPanelGtkUI, fvkbd_panel_gtk_ui, FVKBD_TYPE_GTK_UI);

static gboolean fvkbd_panel_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget);
static gboolean fvkbd_panel_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y,
						gfloat x_ratio, gfloat y_ratio);
static gboolean fvkbd_panel_gtk_ui_destroy (FvkbdGtkUI *ui);
static gboolean fvkbd_panel_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id);

static void
fvkbd_panel_gtk_ui_class_init (FvkbdPanelGtkUIClass *klass)
{
	FvkbdGtkUIClass *ui_class = FVKBD_GTK_UI_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FvkbdPanelGtkUIPrivate));

	ui_class->build = fvkbd_panel_gtk_ui_build;
	ui_class->allocate = fvkbd_panel_gtk_ui_allocate;
	ui_class->destroy = fvkbd_panel_gtk_ui_destroy;
	ui_class->set_mode = fvkbd_panel_gtk_ui_set_mode;
}


static void
fvkbd_panel_gtk_ui_init (FvkbdPanelGtkUI *self)
{
	FvkbdPanelGtkUIPrivate *priv;

	priv = FVKBD_PANEL_GTK_UI_GET_PRIVATE(self);

	priv->res_x_ratio = -1;
	priv->res_y_ratio = -1;

	self->children = NULL;
}


FvkbdGtkUI *
fvkbd_panel_gtk_ui_new (FvkbdUnit *unit)
{
	FvkbdGtkUI *ui;

	g_return_val_if_fail(FVKBD_IS_PANEL(unit), NULL);

	ui = g_object_new(FVKBD_TYPE_PANEL_GTK_UI, NULL);
	fvkbd_gtk_ui_set_unit(ui, unit);
	fvkbd_unit_set_ui_data(ui->unit, ui);

	return ui;
}



static void gtk_vkb_panel_size_allocate (GtkWidget *widget, GtkAllocation *allocation);


static void
_panel_allocate_default_key_shape_bitmap (FvkbdPanelGtkUI *panel_ui, KbdShapeInfo *info)
{
	FvkbdGtkUI *ui = FVKBD_GTK_UI(panel_ui);
	GdkBitmap *bitmap = NULL;
	gfloat x_ratio, y_ratio;

	g_return_if_fail(info->shape_type == KBD_SHAPE_BITMAP_MASK);

	fvkbd_gtk_ui_get_ratio(ui, &x_ratio, &y_ratio);
	bitmap = load_and_scale_bitmap(info->u.mask, x_ratio, y_ratio);

	if (bitmap) {
		fvkbd_gtk_ui_set_qdata(ui, quark_key_shape_bitmap, bitmap,
					(GDestroyNotify)g_object_unref);
		FVKBD_UI_SET_FLAG(ui, FLAG_KEY_SHAPE_BITMAP);
	}
}


static void
gtk_vkb_panel_class_init (GtkVkbPanelClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	widget_class->size_allocate = gtk_vkb_panel_size_allocate;
}


static void
gtk_vkb_panel_init (GtkVkbPanel *self)
{
	self->panel_unit = NULL;
}


GtkWidget *
gtk_vkb_panel_new (FvkbdPanel *panel)
{
	GtkWidget *panel_widget;

	g_return_val_if_fail(FVKBD_IS_PANEL(panel), NULL);

	panel_widget = g_object_new(GTK_TYPE_VKB_PANEL, NULL);
	GTK_VKB_PANEL(panel_widget)->panel_unit = panel;
	return panel_widget;
}


static gboolean
_fvkbd_panel_allocate_child (FvkbdPanelGtkUI *panel_ui, int x_offset, int y_offset,
					gfloat x_ratio, gfloat y_ratio)
{
	GSList *children;
	FvkbdGtkUI *child_ui;
	int base_x, base_y, new_x, new_y;
	int i;

	children = panel_ui->children;

	for (i = 0; i < g_slist_length(children); i++) {
		child_ui = g_slist_nth_data(children, i);
		if ((fvkbd_unit_get_position(child_ui->unit, &base_x, &base_y) == 0)) {
			new_x = base_x * x_ratio + x_offset;
			new_y = base_y * y_ratio + y_offset;

			fvkbd_gtk_ui_allocate(child_ui, new_x, new_y, x_ratio, y_ratio);
		}
	}

	return TRUE;
}


static void
_panel_gtk_ui_allocate_resources (FvkbdGtkUI *ui)
{
	FvkbdPanelGtkUI *self = FVKBD_PANEL_GTK_UI(ui);
	FvkbdPanelGtkUIPrivate *priv;
	FvkbdUnit *unit;
	gfloat new_x_ratio, new_y_ratio;

	fvkbd_gtk_ui_get_ratio(ui, &new_x_ratio, &new_y_ratio);

	priv = FVKBD_PANEL_GTK_UI_GET_PRIVATE(self);

	if ((priv->res_x_ratio == new_x_ratio) && (priv->res_y_ratio == new_y_ratio))
		return;

	unit = ui->unit;

	if (FVKBD_UNIT_HAS_KEY_BG_FILE(unit)) {
		gchar *bg_file;
		GdkPixmap *pixmap = NULL;

		bg_file = fvkbd_unit_get_qdata(unit, quark_key_bg_file);
		load_and_scale_pixmap_and_mask(bg_file, new_x_ratio, new_y_ratio, &pixmap, NULL);
		if (pixmap) {
			fvkbd_gtk_ui_set_qdata(ui, quark_key_bg_pixmap, pixmap,
						(GDestroyNotify)g_object_unref);
			FVKBD_UI_SET_FLAG(ui, FLAG_KEY_BG_PIXMAP);
		}
	}

	if (FVKBD_UNIT_HAS_KEY_SHAPE_INFO(unit)) {
		KbdShapeInfo *key_shape_info;

		key_shape_info = fvkbd_unit_get_qdata(unit, quark_key_shape_info);
		switch (key_shape_info->shape_type) {
		case KBD_SHAPE_BITMAP_MASK:
			_panel_allocate_default_key_shape_bitmap(self, key_shape_info);
			break;
		default:
			break;
		}
	}

	{
		PangoFontDescription **descs;
		KbdFontInfo *font;
		int i;

		descs = g_new(PangoFontDescription *, KBD_FONT_TYPE_NUMBER);
		for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++) {
			// we get font info from panel and/or keyboard level
			font = fvkbd_unit_get_font_recursive(unit, i, NULL);
			if (font)
				*(descs + i) = get_scaled_pango_font_description(font, new_x_ratio, new_y_ratio);
			else
				*(descs + i) = NULL;
		}

		fvkbd_gtk_ui_set_qdata(ui, quark_ui_font_descs, descs,
				(GDestroyNotify)free_ui_font_descs);
		FVKBD_UI_SET_FLAG(ui, FLAG_UI_FONT_DESCS);
	}

	priv->res_x_ratio = new_x_ratio;
	priv->res_y_ratio = new_y_ratio;
}

static void
gtk_vkb_panel_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	FvkbdUnit *unit;
	FvkbdPanel *panel;

	DBG("aw=%d,ah=%d\n", widget->allocation.width, widget->allocation.height);
	unit = FVKBD_UNIT(GTK_VKB_PANEL(widget)->panel_unit);
	panel = FVKBD_PANEL(GTK_VKB_PANEL(widget)->panel_unit);

	widget->allocation = *allocation;

	if (!GTK_WIDGET_NO_WINDOW(widget)) {
		if (GTK_WIDGET_REALIZED(widget))
			gdk_window_move_resize(widget->window,
					allocation->x,
					allocation->y,
					allocation->width,
					allocation->height);
	}

	return;
}


static void
panel_container_on_realize (GtkWidget *widget, gpointer data)
{
	FvkbdGtkUI *ui = FVKBD_GTK_UI(data);
	FvkbdUnit *unit = ui->unit;
	FvkbdPanel *panel = FVKBD_PANEL(unit);

	GdkPixmap *pixmap = NULL;

	GtkWidget *panel_widget;
	gchar *img = NULL;
	KbdColor *kc;
	guint color = 0xFFFFFFFF;

	if ((img = fvkbd_panel_get_img(panel)) == NULL)
		return;

	if ((kc = fvkbd_unit_get_color(unit, KBD_COLOR_TYPE_PANEL_BG)) != NULL)
		color = (kc->r << 24) + (kc->g << 16) + (kc->b << 8) + (kc->a);

	panel_widget = fvkbd_gtk_ui_get_widget(ui);

	load_and_scale_pixmap_and_mask(img, 1, 1, &pixmap, NULL);
	if (pixmap) {
		gtk_widget_set_app_paintable(widget, TRUE);
		gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);
		g_object_unref(pixmap);
	}

	//set_gtk_widget_bg_image(panel_widget, img, color);

	return;
}


static void
panel_container_on_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
	FvkbdGtkUI *ui = FVKBD_GTK_UI(data);
	FvkbdUnit *unit = ui->unit;
	FvkbdPanel *panel;
	int tmp_w = 1, tmp_h = 1;
	float x_ratio, y_ratio;

	unit = FVKBD_UNIT(GTK_VKB_PANEL(widget)->panel_unit);
	panel = FVKBD_PANEL(GTK_VKB_PANEL(widget)->panel_unit);


	fvkbd_unit_get_size(unit, &tmp_w, &tmp_h);
	x_ratio = (float)allocation->width / (float)tmp_w;
	y_ratio = (float)allocation->height / (float)tmp_h;

	fvkbd_gtk_ui_allocate(ui, allocation->x, allocation->y, x_ratio, y_ratio);
	return;
}

static gboolean
fvkbd_panel_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget)
{
	FvkbdPanelGtkUI *self = FVKBD_PANEL_GTK_UI(ui);
	FvkbdUnit *unit = ui->unit;
	FvkbdPanel *panel = FVKBD_PANEL(unit);;
	GtkWidget *panel_widget = NULL;

	FvkbdUnit *child_unit;
	FvkbdGtkUI *child_ui;

	GSList *children;
	GtkWidget *child_widget;

	GdkColor bgcolor;
	gint x = 0, y = 0;
	gint i;
	gboolean ret = TRUE;

	panel_widget = gtk_vkb_panel_new(panel);
	gtk_fixed_set_has_window(GTK_FIXED(panel_widget), TRUE);

	if (get_gdkcolor(unit, KBD_COLOR_TYPE_PANEL_BG, &bgcolor)) {
		gtk_widget_modify_bg(panel_widget, GTK_STATE_NORMAL, &bgcolor);
	}

	g_signal_connect(panel_widget, "realize",
			G_CALLBACK(panel_container_on_realize), ui);
	g_signal_connect(panel_widget, "size-allocate",
			G_CALLBACK(panel_container_on_size_allocate), ui);


	// packing child widget
	children = fvkbd_panel_get_children(panel);
	if (children == NULL)
		goto done;

	for (i = 0; i < g_slist_length(children); i++) {
		child_unit = g_slist_nth_data(children, i);
		if ((fvkbd_get_unit_type(child_unit) != UNIT_TYPE_KEY)) {
			continue;
		}

		child_ui = fvkbd_key_gtk_ui_new(child_unit);
		fvkbd_gtk_ui_set_parent(child_ui, ui);

		if ((ret  = fvkbd_gtk_ui_build(child_ui, &child_widget)) != TRUE)
			goto done;

		if (fvkbd_unit_get_position(child_unit, &x, &y) == 0) {
			gtk_fixed_put(GTK_FIXED(panel_widget), child_widget, x, y);
		}

		self->children = g_slist_append(self->children, child_ui);
	}

done:
	if (widget != NULL)
		*widget = panel_widget;

	fvkbd_gtk_ui_set_widget(ui, panel_widget);
	//gtk_widget_show_all(panel_widget);

	return ret;
}


#if 0
gboolean
fvkbd_panel_build_ui (FvkbdUnit *unit, gpointer *widget)
{
	FvkbdPanel *panel;
	GSList *children;
	FvkbdUnit *child;
	gboolean ret = TRUE;
	PanelUI *ui_data;
	GtkWidget *child_widget = NULL;
	GtkWidget *panel_widget = NULL;
	GdkColor bgcolor;
	gint x = 0, y = 0;
	gint i;

	g_return_val_if_fail(FVKBD_IS_PANEL(unit), FALSE);
	panel = FVKBD_PANEL(unit);

	ui_data = g_new0(PanelUI, 1);
	ui_data->alloc_w = 0;
	ui_data->alloc_h = 0;

	panel_widget = gtk_vkb_panel_new(panel);
	gtk_fixed_set_has_window(GTK_FIXED(panel_widget), TRUE);

	if (get_gdkcolor(unit, KBD_COLOR_TYPE_PANEL_BG, &bgcolor)) {
		gtk_widget_modify_bg(panel_widget, GTK_STATE_NORMAL, &bgcolor);
	}

	g_signal_connect(panel_widget, "realize",
			G_CALLBACK(panel_container_on_realize), unit);


	// packing child widget
	children = fvkbd_panel_get_children(panel);
	if (children == NULL)
		goto done;

	for (i = 0; i < g_slist_length(children); i++) {
		child = g_slist_nth_data(children, i);
		if ((fvkbd_get_unit_type(child) != UNIT_TYPE_KEY)) {
			continue;
		}

		if ((ret  = fvkbd_unit_build_ui(child, (gpointer *)&child_widget)) != TRUE)
			goto done;

		if (fvkbd_unit_get_position(child, &x, &y) == 0) {
			gtk_fixed_put(GTK_FIXED(panel_widget), child_widget, x, y);
		}
	}

	if (widget != NULL)
		*widget = panel_widget;
	fvkbd_unit_set_ui_data(unit, ui_data);
	fvkbd_unit_set_widget(unit, panel_widget);
	gtk_widget_show_all(panel_widget);
done:
	return ret;
}
#endif


static gboolean
fvkbd_panel_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y, gfloat x_ratio, gfloat y_ratio)
{
	FvkbdPanelGtkUI *self = FVKBD_PANEL_GTK_UI(ui);
	FvkbdUnit *unit;
	FvkbdPanel *panel;
	GtkWidget *panel_widget;

	unit = ui->unit;
	panel = FVKBD_PANEL(unit);
	panel_widget = fvkbd_gtk_ui_get_widget(ui);

	fvkbd_gtk_ui_set_ratio(ui, x_ratio, y_ratio);

	_panel_gtk_ui_allocate_resources(ui);

	if (GTK_WIDGET_NO_WINDOW(panel_widget)) {
		_fvkbd_panel_allocate_child(self, x, y,
					x_ratio, y_ratio);
	} else {
		_fvkbd_panel_allocate_child(self, 0, 0, x_ratio, y_ratio);
	}

	return TRUE;
}


static gboolean
fvkbd_panel_gtk_ui_destroy (FvkbdGtkUI *ui)
{
	FvkbdPanelGtkUI *self = FVKBD_PANEL_GTK_UI(ui);
	FvkbdPanel *panel;
	FvkbdGtkUI *child_ui;
	GtkWidget *panel_widget;
	GSList *children;
	gint i;
	gboolean ret = TRUE;

	panel = FVKBD_PANEL(ui->unit);

	panel_widget = ui->widget;

	gtk_widget_hide_all(panel_widget);

	children = self->children;

	// destroy child ui
	children = self->children;
	if (children != NULL) {
		for (i = 0; i < g_slist_length(children); i++) {
			child_ui = g_slist_nth_data(children, i);
			if ((fvkbd_gtk_ui_destroy(child_ui)) != TRUE)
				ret = FALSE;
		}

		g_slist_free(self->children);
		self->children = NULL;
	}

	gtk_widget_destroy(panel_widget);

	if (FVKBD_UI_HAS_KEY_SHAPE_BITMAP(ui)) {
		FVKBD_UI_UNSET_FLAG(ui, FLAG_KEY_SHAPE_BITMAP);
		fvkbd_gtk_ui_set_qdata(ui, quark_key_shape_bitmap, NULL, NULL);
	}

	if (FVKBD_UI_HAS_KEY_BG_PIXMAP(ui)) {
		FVKBD_UI_UNSET_FLAG(ui, FLAG_KEY_BG_PIXMAP);
		fvkbd_gtk_ui_set_qdata(ui, quark_key_bg_pixmap, NULL, NULL);
	}

	if (FVKBD_UI_HAS_FONT_DESCS(ui)) {
		FVKBD_UI_UNSET_FLAG(ui, FLAG_UI_FONT_DESCS);
		fvkbd_gtk_ui_set_qdata(ui, quark_ui_font_descs, NULL, NULL);
	}

	fvkbd_unit_set_ui_data(ui->unit, NULL);
	fvkbd_gtk_ui_set_widget(ui, NULL);

	g_object_unref(self);
	return ret;
}


static gboolean
fvkbd_panel_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id)
{
	GSList *children;
	FvkbdPanelGtkUI *self = FVKBD_PANEL_GTK_UI(ui);
	FvkbdGtkUI *child_ui;
	gint i;
	gboolean ret = TRUE;

	children = self->children;
	if (children == NULL)
		goto done;

	for (i = 0; i < g_slist_length(children); i++) {
		child_ui = g_slist_nth_data(children, i);
		if ( (fvkbd_gtk_ui_set_mode(child_ui, id)) != TRUE) {
			ret = FALSE;
			break;
		}
	}
done:
	return ret;
}

