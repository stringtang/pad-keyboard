/*
 * fvkbd-key-ui-gtk.c key unit gtk ui for fvkbd  
 *
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
#include "gtk-misc-utility.h"
#include "gtk-vkb-button.h"
#include "pixmap-utility.h"
#include "fvkbd-pop-win.h"
#include "fvkbd-keyboard.h"
#include "fvkbd-keyboard-ui-gtk.h"
#include "fvkbd-key-ui-gtk.h"


#define DEFAULT_CHAMFER_SIZE 2

static gboolean longpress_detected = FALSE;
static guint longpress_timeout_id = 0;
static int toggle ;
static int up ;
static int flag ;// when in shift or symbole state only show ying input method
static guint pop_window_hide_timeout_id = 0;

static guint32 latest_mode_key_id = 0;

#define FVKBD_KEY_GTK_UI_GET_PRIVATE(object)\
	(G_TYPE_INSTANCE_GET_PRIVATE((object), FVKBD_TYPE_KEY_GTK_UI, FvkbdKeyGtkUIPrivate))

typedef struct _FvkbdKeyGtkUIPrivate FvkbdKeyGtkUIPrivate;
struct _FvkbdKeyGtkUIPrivate {
	GtkWidget *main_label;
	GtkWidget *sup_label;
	GtkWidget *img;
	GtkWidget *img_dn;

	PangoFontDescription *action_font_descs[KBD_FONT_TYPE_NUMBER];
};


G_DEFINE_TYPE (FvkbdKeyGtkUI, fvkbd_key_gtk_ui, FVKBD_TYPE_GTK_UI);

static void _toggle_key_img_simple (FvkbdKeyGtkUI *key_ui, gboolean down);
static PangoFontDescription * get_key_pango_font_description (FvkbdKeyGtkUI *unit, KbdFontType type);
static FvkbdKeyAction * _key_gtk_ui_get_current_action(FvkbdKeyGtkUI *key_ui);
static void fvkbd_key_shape_mask_none (FvkbdKeyGtkUI *key, GtkWidget *key_widget);
static void fvkbd_key_shape_mask_chamfer (FvkbdKeyGtkUI *key, GtkWidget *key_widget);
static void fvkbd_key_shape_mask_bitmap (FvkbdKeyGtkUI *key, GtkWidget *key_widget, KbdShapeInfo *info);
static void fvkbd_key_settle_bg_image (FvkbdKeyGtkUI *key, GtkWidget *key_widget);


static gboolean fvkbd_key_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget);
static gboolean fvkbd_key_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y,
						gfloat x_ratio, gfloat y_ratio);
static gboolean fvkbd_key_gtk_ui_destroy (FvkbdGtkUI *ui);
static gboolean fvkbd_key_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id);

static void
fvkbd_key_gtk_ui_class_init (FvkbdKeyGtkUIClass *klass)
{
	FvkbdGtkUIClass *ui_class = FVKBD_GTK_UI_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FvkbdKeyGtkUIPrivate));

	ui_class->build = fvkbd_key_gtk_ui_build;
	ui_class->allocate = fvkbd_key_gtk_ui_allocate;
	ui_class->destroy = fvkbd_key_gtk_ui_destroy;
	ui_class->set_mode = fvkbd_key_gtk_ui_set_mode;
}


static void
fvkbd_key_gtk_ui_init (FvkbdKeyGtkUI *self)
{
	FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(self);
	int i;

	priv->main_label = NULL;
	priv->sup_label = NULL;
	priv->img = NULL;
	priv->img_dn = NULL;

	for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++) {
		priv->action_font_descs[i] = NULL;
	}
}


FvkbdGtkUI *
fvkbd_key_gtk_ui_new (FvkbdUnit *unit)
{
	FvkbdGtkUI *ui;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), NULL);

	ui = g_object_new(FVKBD_TYPE_KEY_GTK_UI, NULL);
	fvkbd_gtk_ui_set_unit(ui, unit);
	fvkbd_unit_set_ui_data(ui->unit, ui);

	return ui;
}

static FvkbdKeyAction *
_key_gtk_ui_get_current_action(FvkbdKeyGtkUI *key_ui)
{
	FvkbdKey *key = FVKBD_KEY(FVKBD_GTK_UI(key_ui)->unit);
	FvkbdKeyboard *keyboard = fvkbd_keyboard_gtk_ui_get_keyboard();
	gint mode;

	mode = fvkbd_keyboard_get_current_mode(keyboard);
	return fvkbd_key_get_action(key, mode);
}


static void
_key_gtk_ui_func_handlers(FvkbdKeyGtkUI *key_ui, FvkbdKeyAction *action, void *data)
{
	gint err;
	FvkbdKeyboardGtkUI *keyboard_ui = fvkbd_keyboard_gtk_ui_get_ui();
	FvkbdKeyboard *keyboard = fvkbd_keyboard_gtk_ui_get_keyboard();

	switch (action->u.func.type) {
	case KBD_FUNC_MODE_SELECT:
		{
			int id;
			KeyboardModeStatus status = KEYBOARD_MODE_STATUS_NORMAL;

			id = *((int *)action->u.func.data);
			if (data)
				status = *((KeyboardModeStatus *)data);

			err = keyboard_ui_change_mode(keyboard_ui, id, status);

			break;
		}

	case KBD_FUNC_EXIT:
	case KBD_FUNC_MENU:
		fvkbd_keyboard_do_func(keyboard, &(action->u.func));
		break;

	default:
		break;
	}
}

static void
fvkbd_key_gtk_pressed_cb(GtkVkbButton *button, FvkbdKeyGtkUI *key_ui)
{
	_toggle_key_img_simple(key_ui, TRUE);

}

static void
fvkbd_key_gtk_released_cb(GtkVkbButton *button, FvkbdKeyGtkUI *key_ui)
{
	FvkbdKeyboardGtkUI *keyboard_ui = fvkbd_keyboard_gtk_ui_get_ui();
	FvkbdKeyboard *keyboard = fvkbd_keyboard_gtk_ui_get_keyboard();
	guint32 key_id = FVKBD_UNIT(FVKBD_GTK_UI(key_ui)->unit)->uid;

	FvkbdKeyAction *action;
	gboolean need_reset_default_mode = TRUE;

	_toggle_key_img_simple(key_ui, FALSE);

	/* if the candiate window is shown, then we do nothing and return */
	if (pop_window_hide_timeout_id != 0) {
		return;
	}

	if (longpress_timeout_id != 0) {
		g_source_remove(longpress_timeout_id);
		longpress_timeout_id = 0;
	}

	//hide_pop_window();

	if ((action = _key_gtk_ui_get_current_action(key_ui)) == NULL)
		return;

	switch (action->type) {
	case KEY_ACTION_STRING:
		fvkbd_key_send_utf8_string(action->u.string);
		break;

	case KEY_ACTION_STRING_GROUP:
		fvkbd_key_send_utf8_string(action->u.string_group[0]);
		break;

	case KEY_ACTION_SYM:
		fvkbd_key_send_xkeysym(action->u.sym);
		break;

	case KEY_ACTION_FUNC:
		switch (action->u.func.type) {
		case KBD_FUNC_MODE_SELECT:
			latest_mode_key_id = key_id;
			//if currently in tmp mode, another mode switch always lead to default mode
			//if (fvkbd_keyboard_get_mode_status(keyboard) ==  KEYBOARD_MODE_STATUS_TEMP)
				//break;

			if (longpress_detected) {
				KeyboardModeStatus status = KEYBOARD_MODE_STATUS_LOCK;

				_key_gtk_ui_func_handlers(key_ui, action, &status);
			} else {
				KeyboardModeStatus status = KEYBOARD_MODE_STATUS_TEMP;

				_key_gtk_ui_func_handlers(key_ui, action, &status);
				need_reset_default_mode = FALSE;
				//change method to engish when press button_shift and button_symbol123
				fvkbd_do_script("change-inputmethod.py");
			}
			break;
		default:
			_key_gtk_ui_func_handlers(key_ui, action, NULL);
		}

		break;

	case KEY_ACTION_SCRIPT:
		fvkbd_do_script(action->u.string);
		break;

	default:
		break;
	}
	if ((fvkbd_keyboard_get_mode_status(keyboard) ==  KEYBOARD_MODE_STATUS_TEMP)
		&& need_reset_default_mode)
		{
	if((action->disp != NULL)&&(action->shift != NULL)&&(action->symbol123 == NULL))
	{
		 keyboard_ui_change_mode(keyboard_ui, 0,0); 
	  	 flag = 0;
	 }
	if((action->disp != NULL)&&(action->shift == NULL)&&(action->symbol123 != NULL))
	{
		 keyboard_ui_change_mode(keyboard_ui, 0,0); 
	  	 flag = 0;
	 }
	toggle = 0;	    
}

	if((action->disp != NULL)&&(action->shift != NULL)&&(action->symbol123 == NULL))
{
	toggle = 0;	    
}
	if((!need_reset_default_mode)&&(fvkbd_keyboard_get_mode_status(keyboard) !=  KEYBOARD_MODE_STATUS_NORMAL)&&(action->disp != NULL)&&((action->shift != NULL)||(action->symbol123 != NULL)))
{
    flag = 1;
}
}

static void
fvkbd_key_gtk_size_allocate_cb (GtkVkbButton *button,
					GtkAllocation *allocation, FvkbdKeyGtkUI *key_ui)
{
	FvkbdKey *key = FVKBD_KEY(FVKBD_GTK_UI(key_ui)->unit);
	KbdShapeInfo *info;

	info = fvkbd_unit_get_qdata_recursive(FVKBD_UNIT(key),	quark_key_shape_info, NULL);
	if (!info)
		return;

	switch (info->shape_type) {
	case KBD_SHAPE_NONE:
		fvkbd_key_shape_mask_none(key_ui, GTK_WIDGET(button));
		break;

	case KBD_SHAPE_CHAMFER:
		fvkbd_key_shape_mask_chamfer(key_ui, GTK_WIDGET(button));
		break;

	case KBD_SHAPE_BITMAP_MASK:
		fvkbd_key_shape_mask_bitmap(key_ui, GTK_WIDGET(button), info);
		break;

	default:
		fvkbd_key_shape_mask_none(key_ui, GTK_WIDGET(button));
	}
}

static void
fvkbd_key_settle_color (FvkbdKeyGtkUI *key_ui, GtkWidget *key_widget)
{
	FvkbdKeyboard *keyboard = fvkbd_keyboard_gtk_ui_get_keyboard();
	KeyboardModeStatus status = fvkbd_keyboard_get_mode_status(keyboard);
	FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(key_ui);
	FvkbdUnit *unit = FVKBD_GTK_UI(key_ui)->unit;
	GdkColor color;
	GtkWidget *label;
	gboolean reverse = ((latest_mode_key_id == unit->uid) &&
				(status == KEYBOARD_MODE_STATUS_TEMP));

	// set key background color
	if (!reverse) {
		if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_BG, &color)) {
			vkb_button_set_bg(key_widget, GTK_STATE_NORMAL, &color);
			vkb_button_set_bg(key_widget, GTK_STATE_PRELIGHT, &color);
		}

		if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_FG, &color)) {
			vkb_button_set_bg(key_widget, GTK_STATE_ACTIVE, &color);
		}
	} else {
		if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_FG, &color)) {
			vkb_button_set_bg(key_widget, GTK_STATE_NORMAL, &color);
			vkb_button_set_bg(key_widget, GTK_STATE_PRELIGHT, &color);
		}

		if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_BG, &color)) {
			vkb_button_set_bg(key_widget, GTK_STATE_ACTIVE, &color);
		}
	}

	// set main label color
	label = priv->main_label;
	if (label) {
		if (!reverse) {
			if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_FG, &color)) {
				gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &color);
				gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &color);
			}

			if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_BG, &color)) {
				gtk_widget_modify_fg(label, GTK_STATE_ACTIVE, &color);
			}
		} else {
			if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_BG, &color)) {
				gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &color);
				gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &color);
			}

			if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_FG, &color)) {
				gtk_widget_modify_fg(label, GTK_STATE_ACTIVE, &color);
			}
		}
	}

	// set sub label color
	label = priv->sup_label;
	if (label) {
		if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_EXTRA_FG, &color)) {
			gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &color);
			gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &color);
		}

		if (get_gdkcolor(unit, KBD_COLOR_TYPE_KEY_BG, &color)) {
			gtk_widget_modify_fg(label, GTK_STATE_ACTIVE, &color);
		}
	}
}


static PangoFontDescription *
get_key_pango_font_description (FvkbdKeyGtkUI *key_ui, KbdFontType type)
{
	FvkbdGtkUI *ui = FVKBD_GTK_UI(key_ui);
	gfloat x_ratio, y_ratio;
	FvkbdKeyAction *action = _key_gtk_ui_get_current_action(key_ui);
	KbdFontInfo *font = NULL;

	PangoFontDescription *desc = NULL;

	if ((type < 0) || (type >= KBD_FONT_TYPE_NUMBER))
		return NULL;

	fvkbd_gtk_ui_get_ratio(FVKBD_GTK_UI(key_ui), &x_ratio, &y_ratio);

	if(G_UNLIKELY(action->unique_font == TRUE)) {
		if ((font = action->font[type]) != NULL) {
			FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(key_ui);

			desc = get_scaled_pango_font_description(font, x_ratio, y_ratio);

			if (priv->action_font_descs[type])
				pango_font_description_free(priv->action_font_descs[type]);

			priv->action_font_descs[type] = desc;
		}
	}

	if (!desc) {
		PangoFontDescription **descs;

		descs = fvkbd_gtk_ui_get_qdata_recursive(ui, quark_ui_font_descs, NULL);
		if (descs)
			desc = *(descs + type);
	}

	return desc;
}


static void
fvkbd_key_settle_font (FvkbdKeyGtkUI *key_ui, GtkWidget *key_widget)
{
	FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(key_ui);
	PangoFontDescription *desc;
	GtkWidget *label;

	// set main label font
	label = priv->main_label;
	if (label) {
		desc = get_key_pango_font_description(key_ui, KBD_FONT_TYPE_NORMAL);
		gtk_widget_modify_font(label, desc);
	}

	// set sub label font
	label = priv->sup_label;
	if (label) {
		desc = get_key_pango_font_description(key_ui, KBD_FONT_TYPE_EXTRA);
		gtk_widget_modify_font(label, desc);
	}
}


static void
fvkbd_key_shape_mask_none (FvkbdKeyGtkUI *key_ui, GtkWidget *key_widget)
{
	gtk_widget_shape_combine_mask(key_widget, NULL, 0, 0);
}


static void
fvkbd_key_shape_mask_chamfer (FvkbdKeyGtkUI *key_ui, GtkWidget *key_widget)
{
	GdkBitmap *bitmap = NULL;
	bitmap = get_chamfered_rectangle_bitmap(key_widget->allocation.width,
		key_widget->allocation.height, DEFAULT_CHAMFER_SIZE);
	if (bitmap) {
		gtk_widget_shape_combine_mask(key_widget, bitmap, 0, 0);
		g_object_unref(bitmap);
	}
}


static void
fvkbd_key_shape_mask_bitmap (FvkbdKeyGtkUI *key_ui, GtkWidget *key_widget,
					KbdShapeInfo *info)
{
	FvkbdUnit *unit = FVKBD_GTK_UI(key_ui)->unit;
	gfloat x_ratio, y_ratio;
	GdkBitmap *bitmap = NULL;
	gboolean local = FALSE;

	g_return_if_fail(info->shape_type == KBD_SHAPE_BITMAP_MASK);

	if (!FVKBD_UNIT_HAS_KEY_SHAPE_INFO(unit))
		bitmap = fvkbd_gtk_ui_get_qdata_recursive(FVKBD_GTK_UI(key_ui),
							quark_key_shape_bitmap, NULL);

	/* if this is a local shape info, or parent do not set bitmap data for some reason, then we create it locally */
	if (!bitmap) {
		fvkbd_gtk_ui_get_ratio(FVKBD_GTK_UI(key_ui), &x_ratio, &y_ratio);
		bitmap = load_and_scale_bitmap(info->u.mask, x_ratio, y_ratio);
		local = TRUE;
	}

	if (bitmap) {
		gtk_widget_shape_combine_mask(key_widget, bitmap, 0, 0);
		if (local)
			g_object_unref(bitmap);
	}
}


static void
fvkbd_key_settle_bg_image (FvkbdKeyGtkUI *key_ui, GtkWidget *key_widget)
{
	FvkbdGtkUI *ui = FVKBD_GTK_UI(key_ui);
	gfloat x_ratio, y_ratio;
	FvkbdUnit *unit = ui->unit;
	GdkPixmap *pixmap = NULL;

	if (FVKBD_UNIT_HAS_KEY_BG_FILE(unit)) {
		gchar *bg_file = fvkbd_unit_get_qdata(unit, quark_key_bg_file);

		fvkbd_gtk_ui_get_ratio(ui, &x_ratio, &y_ratio);
		load_and_scale_pixmap_and_mask(bg_file, x_ratio, y_ratio, &pixmap, NULL);
		if (pixmap) {
			fvkbd_gtk_ui_set_qdata(ui, quark_key_bg_pixmap, pixmap,
					(GDestroyNotify)g_object_unref);
			FVKBD_UI_SET_FLAG(ui, FLAG_KEY_BG_PIXMAP);
		}	
	}

	/* if no local bg img or fail to load it, then try parent */
	if (!pixmap) {
		pixmap = fvkbd_gtk_ui_get_qdata_recursive(ui, quark_key_bg_pixmap, NULL);
	}

	vkb_button_set_bg_pixmap(key_widget, pixmap);
}

static GtkWidget *
_create_key_label (FvkbdKeyGtkUI *key_ui, gint id, KeyboardModeStatus status)
{
	FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(key_ui);
	FvkbdUnit *unit = FVKBD_GTK_UI(key_ui)->unit;
	FvkbdKey *key;
	FvkbdKeyAction *action;

	gint w,h;
	gfloat x_ratio, y_ratio, ratio;
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	GtkWidget *label = NULL, *sup_label = NULL;
	GtkWidget *img = NULL, *img_dn = NULL;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), FALSE);
	key = FVKBD_KEY(unit);


	fvkbd_unit_get_size(unit, &w, &h);

	if ((action = _key_gtk_ui_get_current_action(key_ui)) == NULL)
		goto done;

	fvkbd_gtk_ui_get_ratio(FVKBD_GTK_UI(key_ui), &x_ratio, &y_ratio);
	ratio = (x_ratio < y_ratio) ? x_ratio : y_ratio;

	if (action->img != NULL) {
		img = load_and_scale_img(action->img, ratio, ratio);
		if (action->img_dn != NULL) {
			img_dn = load_and_scale_img(action->img_dn, ratio, ratio);
		}

		priv->img = img;
		priv->img_dn = img_dn;
		
		if (priv->img) {
			g_object_ref_sink(priv->img);
			gtk_container_add(GTK_CONTAINER(vbox), img);
		}
		
		if (priv->img_dn) {
			g_object_ref_sink(priv->img_dn);
		}

		goto done;
	}

	if (action->sup_label != NULL)
		sup_label = gtk_label_new(action->sup_label);

	switch (action->type) {
	case KEY_ACTION_STRING_GROUP:
		if (sup_label == NULL) {
			gchar *label_str = NULL;

			#if 0
			// show all the optional characters
			label_str = g_strjoinv(" ", &(action->u.string_group[1]));
			#else
			// only show the first optional character
			label_str = g_strdup(action->u.string_group[1]);
			#endif

			sup_label = gtk_label_new(label_str);
			g_free(label_str);
		}

		label = gtk_label_new(fvkbd_key_get_disp(key, id));
		break;

	case KEY_ACTION_STRING:
	case KEY_ACTION_SYM:
	case KEY_ACTION_SCRIPT:
		label = gtk_label_new(fvkbd_key_get_disp(key, id));
		break;

	case KEY_ACTION_FUNC:
		if ((action->u.func.type == KBD_FUNC_MODE_SELECT)) {
			if (unit->uid == latest_mode_key_id) {
				if (status == KEYBOARD_MODE_STATUS_TEMP) {
					#if 0
					gchar *disp;
					gint p_mode;

					p_mode = fvkbd_keyboard_get_previous_mode(fvkbd_keyboard_gtk_ui_get_keyboard());
					disp = g_strconcat("<u>", fvkbd_key_get_disp(key, p_mode), "</u>", NULL);
					label = gtk_label_new(disp);
					gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
					g_free(disp);
					#endif
				}
			}
		}

		if (!label)
			label = gtk_label_new(fvkbd_key_get_disp(key, id));
		break;

	default:
		break;
	}

	if (sup_label == NULL)
		sup_label = gtk_label_new(NULL);

	priv->sup_label = sup_label;
	if (sup_label)
		gtk_box_pack_start(GTK_BOX(vbox), sup_label, FALSE, FALSE, 0);

	priv->main_label = label;
	if (label)
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

done:

	gtk_widget_show_all(vbox);
	return vbox;
}

static void
_toggle_key_img_simple (FvkbdKeyGtkUI *key_ui, gboolean down)
{
	FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(key_ui);

	GtkWidget *key_widget;
	GtkWidget *key_label;
	FvkbdKeyAction *action;
	int tmp = 0;

	if ((action = _key_gtk_ui_get_current_action(key_ui)) == NULL)
	{
		return;
	}

	if ((action->img == NULL) || (action->img_dn == NULL))
	{
		return;
	}
	key_widget = fvkbd_gtk_ui_get_widget(FVKBD_GTK_UI(key_ui));
	
	key_label = gtk_bin_get_child(GTK_BIN(key_widget));

	if(action->disp == NULL) //Characters part
	{
	if (down) {
		gtk_container_remove(GTK_CONTAINER(key_label), priv->img);
		gtk_container_add(GTK_CONTAINER(key_label), priv->img_dn);
		gtk_widget_show(priv->img_dn);
	} else {
		gtk_container_remove(GTK_CONTAINER(key_label), priv->img_dn);
		gtk_container_add(GTK_CONTAINER(key_label), priv->img);
		gtk_widget_show(priv->img);
	}
	}
	else if((action->disp != NULL) &&(down == TRUE) && (action->sup_label != NULL)&&(action->shift == NULL)&&(action->symbol123 == NULL)&&(flag == 0)) //Chinese to English exchange part
	{
	tmp = toggle;
	if (tmp == 0) {
		gtk_container_remove(GTK_CONTAINER(key_label), priv->img);
		gtk_container_add(GTK_CONTAINER(key_label), priv->img_dn);
		gtk_widget_show(priv->img_dn);
		toggle = 1 ;
	} else {
		gtk_container_remove(GTK_CONTAINER(key_label), priv->img_dn);
		gtk_container_add(GTK_CONTAINER(key_label), priv->img);
		gtk_widget_show(priv->img);
		toggle = 0;
	}
	}
	else if((action->disp != NULL) &&(down == TRUE) && (action->sup_label == NULL)&&(action->shift == NULL)&&(action->symbol123 == NULL)) //Up and down part
	{
	tmp = up;
	if (tmp == 0) {
		gtk_container_remove(GTK_CONTAINER(key_label), priv->img);
		gtk_container_add(GTK_CONTAINER(key_label), priv->img_dn);
		gtk_widget_show(priv->img_dn);
		up = 1 ;
	} else {
		gtk_container_remove(GTK_CONTAINER(key_label), priv->img_dn);
		gtk_container_add(GTK_CONTAINER(key_label), priv->img);
		gtk_widget_show(priv->img);
		up = 0;
	}
	}
}

static void
_update_key_label (FvkbdKeyGtkUI *key_ui, gint id, KeyboardModeStatus status)
{
	FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(key_ui);
	GtkWidget *key_widget;
	GtkWidget *key_label;

	key_widget = fvkbd_gtk_ui_get_widget(FVKBD_GTK_UI(key_ui));
	
	key_label = gtk_bin_get_child(GTK_BIN(key_widget));

	if (key_label) {
		if (priv->img) {
			g_object_unref(priv->img);
			priv->img = NULL;
		}

		if (priv->img_dn) {
			g_object_unref(priv->img_dn);
			priv->img_dn = NULL;
		}

		gtk_widget_destroy(key_label);
		priv->main_label = NULL;
		priv->sup_label = NULL;

		key_label = NULL;
	}

	key_label = _create_key_label(key_ui, id, status);
	gtk_container_add(GTK_CONTAINER(key_widget), key_label);
}


static gboolean
fvkbd_key_gtk_ui_build (FvkbdGtkUI *ui, GtkWidget **widget)
{
	FvkbdKeyGtkUI *key_ui = FVKBD_KEY_GTK_UI(ui);
	FvkbdUnit *unit = ui->unit;
	gint w,h;
	GtkWidget *key_widget;
	gboolean ret = TRUE;

	g_return_val_if_fail(FVKBD_IS_KEY(ui->unit), FALSE);

	key_widget = gtk_vkb_button_new();

	fvkbd_unit_get_size(unit, &w, &h);

	g_signal_connect(G_OBJECT(key_widget), "pressed",
			G_CALLBACK(fvkbd_key_gtk_pressed_cb), key_ui);
	g_signal_connect(G_OBJECT(key_widget), "released",
			G_CALLBACK(fvkbd_key_gtk_released_cb), key_ui);
	g_signal_connect(G_OBJECT(key_widget), "size-allocate",
			G_CALLBACK(fvkbd_key_gtk_size_allocate_cb), key_ui);

	if (widget != NULL)
		*widget = key_widget;

	fvkbd_gtk_ui_set_widget(ui, key_widget);
	return ret;
}


static gboolean
fvkbd_key_gtk_ui_allocate (FvkbdGtkUI *ui, gint x, gint y, gfloat x_ratio, gfloat y_ratio)
{
	FvkbdKeyboard *keyboard = fvkbd_keyboard_gtk_ui_get_keyboard();
	FvkbdKeyGtkUI *key_ui = FVKBD_KEY_GTK_UI(ui);
	FvkbdUnit *unit = ui->unit;
	GtkWidget *key_widget;
	FvkbdKeyAction *action;
	int tmp_w, tmp_h;
	int new_w, new_h;
	GtkAllocation allocation;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), FALSE);

	key_widget = fvkbd_gtk_ui_get_widget(ui);

	if (fvkbd_unit_get_size(unit, &tmp_w, &tmp_h) == 0) {
		new_w = tmp_w * x_ratio;
		new_h = tmp_h * y_ratio;
	} else
		return FALSE;

	fvkbd_gtk_ui_set_ratio(ui, x_ratio, y_ratio);
	allocation.x = x;
	allocation.y = y;
	allocation.width = new_w;
	allocation.height = new_h;

	// if width and height do not change, then just pass down the allocation information
	if ((key_widget->allocation.width == new_w) && (key_widget->allocation.height == new_h)) {
		gtk_widget_size_allocate(key_widget, &allocation);
		return TRUE;
	}

	if ((action = _key_gtk_ui_get_current_action(key_ui)) == NULL)
		return FALSE;

	if (FVKBD_UNIT_HAS_FONT_INFO(unit)) {
		PangoFontDescription **descs;
		KbdFontInfo *font;
		int i;

		descs = g_new(PangoFontDescription *, KBD_FONT_TYPE_NUMBER);
		for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++) {
			font = fvkbd_unit_get_font(unit, i);
			if (font)
				*(descs + i) = get_scaled_pango_font_description(font, x_ratio, y_ratio);
			else
				*(descs + i) = NULL;
		}

		fvkbd_gtk_ui_set_qdata(ui, quark_ui_font_descs, descs,
				(GDestroyNotify)free_ui_font_descs);
		FVKBD_UI_SET_FLAG(ui, FLAG_UI_FONT_DESCS);
	}

	if ((action->img != NULL) || (gtk_bin_get_child(GTK_BIN(key_widget)) == NULL)) {
		_update_key_label(key_ui,
				fvkbd_keyboard_get_current_mode(keyboard),
				fvkbd_keyboard_get_mode_status(keyboard));
	}

	// set color
	fvkbd_key_settle_color(key_ui, key_widget);

	// set font
	fvkbd_key_settle_font(key_ui, key_widget);

	fvkbd_key_settle_bg_image(key_ui, key_widget);
	gtk_widget_size_allocate(key_widget, &allocation);

	return TRUE;
}


static gboolean
fvkbd_key_gtk_ui_destroy (FvkbdGtkUI *ui)
{
	FvkbdKeyGtkUI *key_ui = FVKBD_KEY_GTK_UI(ui);
	FvkbdUnit *unit = ui->unit;
	FvkbdKeyGtkUIPrivate *priv = FVKBD_KEY_GTK_UI_GET_PRIVATE(key_ui);
	FvkbdKey *key;
	gboolean ret = TRUE;
	GtkWidget *key_widget;
	int i;

	g_return_val_if_fail(FVKBD_IS_KEY(unit), FALSE);

	key = FVKBD_KEY(unit);
	key_widget = fvkbd_gtk_ui_get_widget(ui);

	gtk_widget_hide_all(key_widget);

	for (i = 0; i < KBD_FONT_TYPE_NUMBER; i++) {
		if (priv->action_font_descs[i])
			pango_font_description_free(priv->action_font_descs[i]);
	}

	if (priv->img)
		g_object_unref(priv->img);
	if (priv->img_dn)
		g_object_unref(priv->img_dn);

	gtk_widget_destroy(key_widget);

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

	fvkbd_unit_set_ui_data(unit, NULL);
	fvkbd_gtk_ui_set_widget(ui, NULL);

	g_object_unref(ui);
	return ret;
}


static gboolean
fvkbd_key_gtk_ui_set_mode (FvkbdGtkUI *ui, gint id)
{
	FvkbdKeyGtkUI *key_ui;
	FvkbdKey *key;
	FvkbdKeyboard *keyboard = fvkbd_keyboard_gtk_ui_get_keyboard();

	KeyboardModeStatus status;
	GtkWidget *key_widget;

	g_return_val_if_fail(FVKBD_IS_KEY_GTK_UI(ui), FALSE);

	key_ui = FVKBD_KEY_GTK_UI(ui);
	key = FVKBD_KEY(ui->unit);
	key_widget = fvkbd_gtk_ui_get_widget(ui);
	status = fvkbd_keyboard_get_mode_status(keyboard);

	_update_key_label(key_ui, id, status);

	// set color
	fvkbd_key_settle_color(key_ui, key_widget);

	// settle font
	fvkbd_key_settle_font(key_ui, key_widget);

	return TRUE;
}

