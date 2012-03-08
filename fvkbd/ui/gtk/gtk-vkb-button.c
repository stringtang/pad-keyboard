/*
 * gtk-vkb-button.c custom gtk button widget for fvkbd
 *
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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


#include "gtk-vkb-button.h"

#define GTK_VKB_BUTTON_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_VKB_BUTTON, GtkVkbButtonPrivate))

struct _GtkVkbButtonPrivate {
	GdkBitmap	*mask;
};

G_DEFINE_TYPE (GtkVkbButton, gtk_vkb_button, GTK_TYPE_BIN);

enum
{
	PRESSED,
	RELEASED,

	LAST_SIGNAL
};

static guint vkb_button_signals[LAST_SIGNAL] = { 0, };

static gboolean gtk_vkb_button_press (GtkWidget *widget, GdkEventButton *event);
static gboolean gtk_vkb_button_release (GtkWidget *widget, GdkEventButton *event);
static void gtk_real_vkb_button_pressed (GtkVkbButton *button);
static void gtk_real_vkb_button_released (GtkVkbButton *button);
static void gtk_vkb_button_realize (GtkWidget *widget);
static void gtk_vkb_button_unrealize (GtkWidget *widget);
static void gtk_vkb_button_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_vkb_button_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static gboolean gtk_vkb_button_expose (GtkWidget *widget, GdkEventExpose *event);


void
vkb_button_set_bg(GtkWidget *widget, GtkStateType state, const GdkColor *color)
{
	GtkVkbButton *button;
	GdkColormap *colormap;

	g_return_if_fail(GTK_IS_VKB_BUTTON(widget));
	g_return_if_fail(state >= GTK_STATE_NORMAL && state <= GTK_STATE_INSENSITIVE);

	button = GTK_VKB_BUTTON(widget);

	colormap = gtk_widget_get_colormap(GTK_WIDGET(button));

	if (button->bgcolor[state].pixel != 0) {
		if (gdk_color_equal(&button->bgcolor[state], color))
			goto done;
		gdk_colormap_free_colors(colormap, &button->bgcolor[state], 1);
	}

	button->bgcolor[state] = *color;
	button->bgcolor[state].pixel = 0;
	gdk_colormap_alloc_color(colormap, &button->bgcolor[state], FALSE, TRUE);
done:
	return;
}


void
vkb_button_set_bg_pixmap(GtkWidget *widget, GdkPixmap *pixmap)
{
	GtkVkbButton *button;

	g_return_if_fail(GTK_IS_VKB_BUTTON(widget));

	button = GTK_VKB_BUTTON(widget);

	if (button->bg_pixmap != NULL) {
		g_object_unref(button->bg_pixmap);
	}

	if (GDK_IS_PIXMAP(pixmap))
		button->bg_pixmap = g_object_ref(pixmap);
	else
		button->bg_pixmap = NULL;

	if (widget->window)
		gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);
}


static void free_vkb_button_resources (GtkVkbButton *button)
{
	int i;
	GdkColormap *colormap;

	colormap = gtk_widget_get_colormap(GTK_WIDGET(button));

	for (i = 0; i < 4; i++) {
		if (button->bgcolor[i].pixel != 0) {
		gdk_colormap_free_colors(colormap, &button->bgcolor[i], 1);
		button->bgcolor[i].pixel = 0;
		}
	}

	if (button->bg_pixmap != NULL) {
		g_object_unref(button->bg_pixmap);
		button->bg_pixmap = NULL;
	}
}


static void
gtk_vkb_button_finalize (GObject *object)
{
	GtkVkbButton *button = GTK_VKB_BUTTON(object);

	free_vkb_button_resources(button);
	G_OBJECT_CLASS(gtk_vkb_button_parent_class)->finalize(object);
}


static void
gtk_vkb_button_class_init (GtkVkbButtonClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GtkVkbButtonPrivate));

	gobject_class->finalize = gtk_vkb_button_finalize;

	widget_class->realize = gtk_vkb_button_realize;
	widget_class->unrealize = gtk_vkb_button_unrealize;
	widget_class->expose_event = gtk_vkb_button_expose;
	widget_class->size_request = gtk_vkb_button_size_request;
	widget_class->size_allocate = gtk_vkb_button_size_allocate;
	widget_class->button_press_event = gtk_vkb_button_press;
	widget_class->button_release_event = gtk_vkb_button_release;

	klass->pressed = gtk_real_vkb_button_pressed;
	klass->released = gtk_real_vkb_button_released;

	vkb_button_signals[PRESSED] =
		g_signal_new("pressed",
			G_OBJECT_CLASS_TYPE(gobject_class),
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(GtkVkbButtonClass, pressed),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);

	vkb_button_signals[RELEASED] =
		g_signal_new ("released",
			G_OBJECT_CLASS_TYPE(gobject_class),
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET(GtkVkbButtonClass, released),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);
}


static void
gtk_vkb_button_init (GtkVkbButton *self)
{
	int i;

	GTK_WIDGET_UNSET_FLAGS(self, GTK_NO_WINDOW);

	for (i = 0; i < 4; i++) {
		self->bgcolor[i].red = 0;
		self->bgcolor[i].green = 0;
		self->bgcolor[i].blue = 0;
		self->bgcolor[i].pixel = 0;
	}

	self->bg_pixmap = NULL;
}


GtkWidget *
gtk_vkb_button_new (void)
{
	return g_object_new(GTK_TYPE_VKB_BUTTON, NULL);
}


static gboolean
gtk_vkb_button_press (GtkWidget *widget, GdkEventButton *event)
{
	GtkVkbButton *button = GTK_VKB_BUTTON(widget);

	if (event->type == GDK_BUTTON_PRESS){
		if (event->button == 1)
			g_signal_emit(button, vkb_button_signals[PRESSED], 0);
	}

	return TRUE;
}


static gboolean
gtk_vkb_button_release (GtkWidget *widget, GdkEventButton *event)
{
	GtkVkbButton *button = GTK_VKB_BUTTON(widget);;

	if (event->button == 1)
		g_signal_emit(button, vkb_button_signals[RELEASED], 0);

	return TRUE;
}


static void
gtk_real_vkb_button_pressed (GtkVkbButton *button)
{
	gtk_widget_set_state(GTK_WIDGET(button), GTK_STATE_ACTIVE);
}


static void
gtk_real_vkb_button_released (GtkVkbButton *button)
{
	gtk_widget_set_state(GTK_WIDGET(button), GTK_STATE_NORMAL);
}


static void
gtk_vkb_button_realize (GtkWidget *widget)
{
	GdkWindowAttr attributes;
	gint attributes_mask;
	GtkVkbButton *button = GTK_VKB_BUTTON(widget);

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events(widget)
			| GDK_BUTTON_MOTION_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_EXPOSURE_MASK
			| GDK_ENTER_NOTIFY_MASK
			| GDK_LEAVE_NOTIFY_MASK;

	attributes.visual = gtk_widget_get_visual(widget);
	attributes.colormap = gtk_widget_get_colormap(widget);
	attributes.wclass = GDK_INPUT_OUTPUT;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
					&attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, widget);
	widget->style = gtk_style_attach(widget->style, widget->window);

	if (button->bg_pixmap)
		gdk_window_set_back_pixmap(widget->window, button->bg_pixmap, FALSE);
}


static void
gtk_vkb_button_unrealize (GtkWidget *widget)
{
	//GtkVkbButton *button = GTK_VKB_BUTTON(widget);

	GTK_WIDGET_CLASS(gtk_vkb_button_parent_class)->unrealize(widget);
}


static void
gtk_vkb_button_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkBin *bin = GTK_BIN(widget);

	requisition->width = 0;
	requisition->height = 0;

	if (bin->child && GTK_WIDGET_VISIBLE(bin->child)) {
		GtkRequisition child_requisition;
		gtk_widget_size_request(bin->child, &child_requisition);
		requisition->width += child_requisition.width;
		requisition->height += child_requisition.height;
	}
}

static void
gtk_vkb_button_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkBin *bin;
	GtkAllocation child_allocation;

	widget->allocation = *allocation;
	bin = GTK_BIN (widget);

	child_allocation.x = 0;
	child_allocation.y = 0;

	child_allocation.width = MAX(allocation->width, 0);
	child_allocation.height = MAX(allocation->height, 0);

	if (GTK_WIDGET_REALIZED(widget)){
		gdk_window_move_resize(widget->window, allocation->x, allocation->y,
				child_allocation.width, child_allocation.height);
	}

	if (bin->child)
		gtk_widget_size_allocate (bin->child, &child_allocation);
}


static gboolean
gtk_vkb_button_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GtkVkbButton *button = GTK_VKB_BUTTON(widget);
	GtkWidget *child;
	GList *children;
	GList *tmplist;

	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_VKB_BUTTON(widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	if(GTK_WIDGET_DRAWABLE(widget)){
		GtkStateType state = GTK_WIDGET_STATE(widget);
		if (!button->bg_pixmap) {
			gdk_window_set_background(widget->window, &button->bgcolor[state]);
			gdk_window_clear(widget->window);
		}

		children = gtk_container_get_children(GTK_CONTAINER(widget));
		tmplist = children;
		while (tmplist) {
			child = GTK_WIDGET(tmplist->data);
			tmplist = tmplist->next;
			gtk_container_propagate_expose(GTK_CONTAINER(widget),
							child, event);
		}

		g_list_free(children);
	}

	return FALSE;
}

