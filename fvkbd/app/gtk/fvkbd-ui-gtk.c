/*
 * fvkbd-ui-gtk.c main GTK UI entry for fvkbd  
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

#include <gtk/gtk.h>
#include <unique/unique.h>
#include "fvkbd.h"

#include "fvkbd-keyboard.h"
#include "fvkbd-panel.h"
#include "layout-utility.h"
#include "misc-utility.h"

#include "gtk-ui-base.h"
#include "gtk-misc-utility.h"
#include "fvkbd-keyboard-ui-gtk.h"
#include "fvkbd-panel-ui-gtk.h"

enum {
	COMMAND_LAYOUT = 1
};

static gchar *current_layout = NULL;
FvkbdGtkUI *the_keyboard_ui = NULL;

static gboolean kbd_initialized = FALSE;

static gboolean _cmd_menu_activated = FALSE;
static GtkWidget *_cmd_menu = NULL;
static GtkWidget * layout_history_menu_items[MAX_LAYOUT_HISTORY];

static gboolean _panel_window_draging = FALSE;
static gint _panel_window_drag_x;
static gint _panel_window_drag_y;

struct _fvkbd_opts {
	gchar *layout_file;
	gboolean embedded;
	gboolean dock;
};

static struct _fvkbd_opts opts;

typedef struct _KbdWindow KbdWindow;
struct _KbdWindow {
	GtkWidget *window;
};

struct _kbd_data {
	GSList *kbd_windows;
	GtkWidget *plug_window;
	gboolean show_plug_window;
	
};

static struct _kbd_data kbd_data;

static GOptionEntry opt_entries[] = {
	{"dockhint", 'd', 0, G_OPTION_ARG_NONE, &opts.dock, "Set dock hint for VKB windows(The behavior depends on specific WM)"},
	{"layout-file", 'l', 0, G_OPTION_ARG_STRING, &opts.layout_file, "Keyboard Layout file " "PATH/TO/FILE"},
	{"xid", 'x', 0, G_OPTION_ARG_NONE, &opts.embedded, "Print window ID to stdout(for embedding)"},
	{NULL}
};

static gboolean build_panel_windows (FvkbdGtkUI *ui);
static GtkWidget *panel_plug_embedded(GtkWidget *plug);
static gboolean _gtk_ui_change_layout(const gchar* name);


static void
gtk_ui_quit(void)
{
	gtk_main_quit();
}


static gboolean
panel_window_motion_cb (GtkWidget *window, GdkEventMotion *event, gpointer user_data)
{
	gint x, y;
	gint new_x, new_y;

	if (!_panel_window_draging)
		return FALSE;

	if ((event->state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)) != 0) {
		gtk_window_get_position(GTK_WINDOW (window), &x, &y);
		new_x = x + ((gint) event->x_root - _panel_window_drag_x);
		new_y = y + ((gint) event->y_root - _panel_window_drag_y);

		gtk_window_move (GTK_WINDOW (window), new_x, new_y);

		_panel_window_drag_x = (gint) event->x_root;
		_panel_window_drag_y = (gint) event->y_root;

		return TRUE;
	}

	return FALSE;
}


static gboolean
panel_window_click (GtkWidget *window, GdkEventButton *event, gboolean button_press)
{
	static gulong motion_handler;

	if (button_press) {
		if (_panel_window_draging)
			return FALSE;

		motion_handler = g_signal_connect(G_OBJECT(window), "motion-notify-event",
	                                           G_CALLBACK(panel_window_motion_cb), NULL);

		_panel_window_draging = TRUE;
		_panel_window_drag_x = (gint)event->x_root;
		_panel_window_drag_y = (gint)event->y_root;

		gdk_pointer_grab(window->window, TRUE,
				(GdkEventMask)(GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
				NULL, NULL, event->time);
	        return TRUE;
	} else {
		if(!_panel_window_draging)
			return FALSE;

		g_signal_handler_disconnect(G_OBJECT(window), motion_handler);
		gdk_pointer_ungrab(event->time);
		_panel_window_draging = FALSE;
		return TRUE;
	}

	return FALSE;
}


static gboolean
panel_window_move_start (GtkWidget *window, GdkEventButton *event, gpointer user_data)
{
	return panel_window_click(window, event, TRUE);
}


static gboolean
panel_window_move_end (GtkWidget *window, GdkEventButton *event, gpointer user_data)
{
	return panel_window_click(window, event, FALSE);
}


static void
cmd_menu_exit_activate_cb (GtkWidget *item, gpointer user_data)
{
	gtk_ui_quit();
}


static void
cmd_menu_layout_item_activate_cb (GtkWidget *item, gpointer user_data)
{
	const gchar *name;
	GtkWidget *label;

	label = gtk_bin_get_child(GTK_BIN(item));
	name = gtk_label_get_text(GTK_LABEL(label));

	_gtk_ui_change_layout(name);
}

static void
cmd_menu_deactivate_cb (GtkWidget *item, gpointer user_data)
{
	_cmd_menu_activated = FALSE;
}


static GtkWidget *
gtk_ui_get_cmd_menu(void)
{
	GtkWidget *menu_item;

	if (_cmd_menu != NULL)
		return _cmd_menu;

	_cmd_menu = gtk_menu_new();
	gtk_menu_shell_set_take_focus(GTK_MENU_SHELL(_cmd_menu), FALSE);

	/* create layout history list */
	{
		int i;
		for (i = 0; i < MAX_LAYOUT_HISTORY; i++) {
			menu_item = gtk_menu_item_new_with_label("");
			gtk_menu_shell_append(GTK_MENU_SHELL(_cmd_menu), menu_item);
			g_signal_connect(G_OBJECT(menu_item), "activate",
				G_CALLBACK(cmd_menu_layout_item_activate_cb), NULL);
			layout_history_menu_items[i] = menu_item;
		}

		/* create separator */
		menu_item = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(_cmd_menu), menu_item);
		gtk_widget_show_all(menu_item);
	}

	/* create layout files' list */
	{
		GSList *layout_files = NULL;
		LayoutFileInfo *info;
		GtkWidget *submenu;
		int i;

		layout_files = get_layout_file_lists();
		menu_item = gtk_menu_item_new_with_label(_("Change Layout"));
		gtk_widget_show_all(menu_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(_cmd_menu), menu_item);

		submenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
		gtk_widget_show(submenu);

		for (i = 0; i < g_slist_length(layout_files); i++) {
			info = g_slist_nth_data(layout_files, i);
			menu_item = gtk_menu_item_new_with_label(info->name);
			gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menu_item);
			g_signal_connect(G_OBJECT(menu_item), "activate",
				G_CALLBACK(cmd_menu_layout_item_activate_cb), NULL);
			gtk_widget_show_all(menu_item);
		}
	}

	/* create separator */
	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(_cmd_menu), menu_item);
	gtk_widget_show_all(menu_item);

	/* create menu item Exit */
	menu_item = gtk_menu_item_new_with_label(_("Exit"));
	gtk_menu_shell_append(GTK_MENU_SHELL(_cmd_menu), menu_item);
	g_signal_connect(G_OBJECT(menu_item), "activate",
		G_CALLBACK(cmd_menu_exit_activate_cb), NULL);
	gtk_widget_show_all(menu_item);

	g_signal_connect(G_OBJECT(_cmd_menu), "deactivate",
			G_CALLBACK(cmd_menu_deactivate_cb), NULL);

	return _cmd_menu;
}


static void
gtk_ui_show_menu(void)
{
	GSList *layout_history;
	gchar *name;
	GtkWidget *label;
	gint len = 0;
	gint i;

	if (_cmd_menu_activated)
		return;

	_cmd_menu_activated = TRUE;

	gtk_menu_popup(GTK_MENU(gtk_ui_get_cmd_menu()), NULL, NULL, NULL, NULL,
			0, gtk_get_current_event_time());
	
	layout_history = get_layout_history();

	len = g_slist_length(layout_history);
	if (len > MAX_LAYOUT_HISTORY)
		len = MAX_LAYOUT_HISTORY;

	for (i = 0; i < len; i++) {
		name = g_slist_nth_data(layout_history, i);
		label = gtk_bin_get_child(GTK_BIN(layout_history_menu_items[i]));
		gtk_label_set_text(GTK_LABEL(label), name);
		gtk_widget_show_all(layout_history_menu_items[i]);
	}

	for (; i < MAX_LAYOUT_HISTORY; i++)
		gtk_widget_hide(layout_history_menu_items[i]);
}

static gboolean
gtk_ui_kbd_func_cb(FvkbdKeyboard *keyboard, KbdFuncInfo *func, FvkbdKeyboardGtkUI *ui)
{
	gboolean ret = FALSE;

	switch (func->type) {
	case KBD_FUNC_EXIT:
		gtk_ui_quit();
		break;

	case KBD_FUNC_MENU:
		gtk_ui_show_menu();
		ret = TRUE;
		break;

	default:
		break;
	}

	return ret;
}


static GtkWidget *
build_panel_window (FvkbdGtkUI *ui)
{
	FvkbdUnit *unit;
	FvkbdPanel *panel;
	GtkWidget *panel_window;
	GdkColor bgcolor;

	gint x = 0, y = 0, width = 1, height = 1;
	int wx = 0, wy = 0, wwidth = 1, wheight = 1;

	g_return_val_if_fail(FVKBD_IS_PANEL_GTK_UI(ui), NULL);
	unit = ui->unit;
	panel = FVKBD_PANEL(unit);

	if(opts.embedded) {
		panel_window = kbd_data.plug_window;
	} else {
		panel_window = gtk_window_new(GTK_WINDOW_POPUP);

		if (opts.dock)
			gtk_window_set_type_hint(GTK_WINDOW(panel_window), GDK_WINDOW_TYPE_HINT_DOCK);

		gtk_window_set_decorated(GTK_WINDOW(panel_window), FALSE);
		gtk_window_set_accept_focus(GTK_WINDOW(panel_window), FALSE);
		gtk_window_set_focus(GTK_WINDOW(panel_window), NULL);
		gtk_window_set_keep_above(GTK_WINDOW(panel_window), TRUE);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(panel_window), TRUE);
		gtk_window_set_skip_pager_hint(GTK_WINDOW(panel_window), TRUE);
		//gtk_window_set_resizable (GTK_WINDOW(panel_window), TRUE);
	}

	if (get_gdkcolor(unit, KBD_COLOR_TYPE_PANEL_BG, &bgcolor)) {
		gtk_widget_modify_bg(panel_window, GTK_STATE_NORMAL, &bgcolor);
	}


	fvkbd_unit_get_size(unit, &width, &height);
	fvkbd_unit_get_position(unit, &x, &y);
	if (get_workarea(&wx, &wy, &wwidth, &wheight) != TRUE) {
		wwidth = width; //FIXME
		wheight = 800; // FIXME
		wx = 0;
		wy = 0;
	}

	DBG("--- workarea x=%d, y=%d, w=%d, h=%d ---", wx, wy, wwidth, wheight);

	switch (fvkbd_panel_get_docktype(panel)) {
	case LAYOUT_DOCK_NONE:
		gtk_widget_add_events(panel_window,GDK_BUTTON_PRESS_MASK);
		gtk_widget_add_events(panel_window,GDK_BUTTON_RELEASE_MASK);
		gtk_widget_add_events(panel_window,GDK_POINTER_MOTION_MASK);

		g_signal_connect(G_OBJECT(panel_window), "button-press-event",
				G_CALLBACK(panel_window_move_start), NULL);
		g_signal_connect(G_OBJECT(panel_window), "button-release-event",
				G_CALLBACK(panel_window_move_end), NULL);
		break;

	case LAYOUT_DOCK_TOP:
		height = (height * wwidth) / width;
		if (height > wheight)
			height = wheight / 2;
		width = wwidth;
		x = wx;
		y = wy;
		break;

	case LAYOUT_DOCK_BOTTOM:
		height = (height * wwidth) / width;
		if (height > wheight)
			height = wheight / 2;
		width = wwidth;
		x = wx;
		y = wheight + wy - height;
		break;

	case LAYOUT_DOCK_LEFT:
		width = (width * wheight) / height;
		if (width > wwidth)
			width = wwidth / 2;
		height = wheight;
		x = wx;
		y = wy;
		break;

	case LAYOUT_DOCK_RIGHT:
		width = (width * wheight) / height;
		if (width > wwidth)
			width = wwidth / 2;
		height = wheight;
		x = wwidth - width;
		y = wy;
		break;
	}

	gtk_container_add(GTK_CONTAINER(panel_window), fvkbd_gtk_ui_get_widget(ui));
	gtk_window_resize(GTK_WINDOW(panel_window), width, height);

	if(opts.embedded) {
		if (kbd_data.show_plug_window);
			gtk_widget_show_all(kbd_data.plug_window);
	} else {
		GdkGeometry hints;

		hints.base_width = width;
		hints.base_height = height;
		hints.min_width = width;
		hints.min_height = height;
		hints.max_width = width;
		hints.max_height = height;

		gtk_window_set_geometry_hints(GTK_WINDOW (panel_window),
						GTK_WIDGET (panel_window),
						&hints,
						GDK_HINT_MIN_SIZE |
						GDK_HINT_MAX_SIZE |
						GDK_HINT_BASE_SIZE);

		gtk_window_move(GTK_WINDOW(panel_window), x, y);
		gtk_widget_show_all(panel_window);
	}

	return panel_window;
}

static GtkWidget *
panel_plug_embedded(GtkWidget * plug)
{
	gtk_widget_show_all(plug);
	kbd_data.show_plug_window = TRUE;
	return NULL;
}

static gboolean
destroy_panel_windows (void)
{
	GSList *kbd_windows;
	KbdWindow *kbd_window;
	gboolean ret = TRUE;
	gint i;

	// destroy panel windows
	kbd_windows = kbd_data.kbd_windows;
	if (kbd_windows == NULL)
		goto done;

	for (i = 0; i < g_slist_length(kbd_windows); i++) {
		kbd_window = g_slist_nth_data(kbd_windows, i);

		/* in embedded mode, the plug window should not be freed */
		if (!opts.embedded)
			gtk_widget_destroy(kbd_window->window);

		g_free(kbd_window);
	}

	g_slist_free(kbd_windows);
	kbd_data.kbd_windows = NULL;

done:
	return ret;
}


static gboolean
build_panel_windows (FvkbdGtkUI *ui)
{
	FvkbdKeyboardGtkUI *keyboard_ui = FVKBD_KEYBOARD_GTK_UI(ui);

	FvkbdGtkUI *child_ui;
	GSList *children;
	GtkWidget *window;
	int i;
	KbdWindow *kbd_window;

	destroy_panel_windows();

	children = keyboard_ui->children;

	if(opts.embedded && (g_slist_length(children) != 1)) {
		g_print("when in embedded mode, only one panel is supported\n");
		return FALSE;
	}

	for (i = 0; i < g_slist_length(children); i++) {
		child_ui = g_slist_nth_data(children, i);
		if ((window = build_panel_window(child_ui)) == NULL)
			return FALSE;

		kbd_window = g_new0(KbdWindow, 1);
		kbd_window->window = window;
		kbd_data.kbd_windows = g_slist_append(kbd_data.kbd_windows, kbd_window);
	}

	return TRUE;
}


static UniqueResponse
unique_app_cb (UniqueApp *app, gint command, UniqueMessageData *data,
		guint timestamp, gpointer user_data)
{
	if (!kbd_initialized)
		return UNIQUE_RESPONSE_CANCEL;

	switch (command) {
	case UNIQUE_ACTIVATE:
		break;
	case COMMAND_LAYOUT:
		break;
	default:
		break;
	}

	return UNIQUE_RESPONSE_OK;
}


static gboolean
_gtk_ui_change_layout(const gchar* name)
{
	FvkbdGtkUI *keyboard_ui = NULL;
	FvkbdUnit *new_keyboard = NULL;
	FvkbdUnit *old_keyboard = NULL;
	const gchar *layout_file = NULL;

	DBG("current_layout = %s, new = %s", current_layout, name);
	// not change layout if it's the same one
	if (!g_strcmp0(current_layout, name))
		return TRUE;

	layout_file = get_layout_file_fullname(name);

	if (layout_file)
		DBG("layout file = %s", layout_file);
	else {
		g_fprintf(stderr, "Layout file not found or not readable\n");
		goto fail;
	}

	if ((new_keyboard = kbd_load_keyboard(name)) == NULL) {
		g_fprintf(stderr, "Failed to load : %s\n", layout_file);
		goto fail;
	}

	old_keyboard = the_keyboard_ui->unit;

	fvkbd_gtk_ui_destroy(the_keyboard_ui);
	the_keyboard_ui = NULL;

	keyboard_ui = fvkbd_keyboard_gtk_ui_new(new_keyboard);
	if (fvkbd_gtk_ui_build(keyboard_ui, NULL) == FALSE) {
		fprintf(stderr, "Failed to build ui for : %s\n", name);
		goto rollback;
	}

	if (build_panel_windows(keyboard_ui) != TRUE) {
		fprintf(stderr, "Failed to init keyboard UI\n");
		goto rollback;
	}

	g_signal_connect(G_OBJECT(new_keyboard), "kbd-func",
			G_CALLBACK(gtk_ui_kbd_func_cb), keyboard_ui);

	g_object_unref(old_keyboard);
	g_free(current_layout);
	current_layout = g_strdup(name);
	the_keyboard_ui = keyboard_ui;
	return TRUE;

rollback:

	fvkbd_gtk_ui_destroy(keyboard_ui);
	keyboard_ui = fvkbd_keyboard_gtk_ui_new(old_keyboard);
	fvkbd_gtk_ui_build(keyboard_ui, NULL);
	build_panel_windows(keyboard_ui);
	the_keyboard_ui = keyboard_ui;

fail:
	if (new_keyboard)
		g_object_unref(new_keyboard);

	return FALSE;
}


int
main(int argc, char* argv[])
{
	UniqueApp *app = NULL;
	GOptionContext *context;
	GError *error = NULL;
	gchar *layout_name;
	GTimer *timer;
	FvkbdUnit *keyboard;
	FvkbdGtkUI *keyboard_ui;

	gtk_init(&argc, &argv);
	opts.embedded = FALSE;
	opts.dock = FALSE;

	timer = TIMER_NEW();

	context = g_option_context_new(" fvkbd : Free Virtual Keyboard");
	g_option_context_add_main_entries(context, opt_entries, NULL);

	if (!g_option_context_parse(context, &argc, &argv, &error)){
		fprintf(stderr, "option parsing failed: %s\n", error->message);
		return -1;
	}

	g_option_context_free(context);

	TIMER_ELAPSED(timer);

	if (!opts.embedded) {
		app = unique_app_new_with_commands("com.intel.fvkbd.unique", NULL,
							"layout", COMMAND_LAYOUT,
							NULL);
		if (unique_app_is_running(app)) {
			UniqueResponse response;

			response = unique_app_send_message(app, UNIQUE_ACTIVATE, NULL);
			if (response == UNIQUE_RESPONSE_OK) {
				fprintf(stderr, "\nAnother instance is already running\n");
			//	goto done;
			} else if (response == UNIQUE_RESPONSE_CANCEL) {
				fprintf(stderr, "\nAnother instance is running, but seems not useable now, try again later\n");
				goto done;
			}
		}

		g_signal_connect(app, "message-received", G_CALLBACK(unique_app_cb), NULL);
	}

	kbd_data.kbd_windows = NULL;
	kbd_data.show_plug_window = FALSE;
	if(opts.embedded) {
		kbd_data.plug_window = gtk_plug_new(0);
	} else {
		kbd_data.plug_window = NULL;
	}

	layout_name = kbd_init(opts.layout_file);

	TIMER_ELAPSED(timer);

	if (!layout_name) {
		fprintf(stderr, "Could not find layout file\n");
		return -1;
	}

	if ((keyboard = kbd_load_keyboard(layout_name)) == NULL) {
		fprintf(stderr, "Failed to load : %s\n", layout_name);
		return -1;
	}

	current_layout = g_strdup(layout_name);

	TIMER_ELAPSED(timer);

	keyboard_ui = fvkbd_keyboard_gtk_ui_new(keyboard);
	if (fvkbd_gtk_ui_build(keyboard_ui, NULL) == FALSE) {
		fprintf(stderr, "Failed to build ui for : %s\n", layout_name);
		return -1;
	}

	TIMER_ELAPSED(timer);

	if (build_panel_windows(keyboard_ui) != TRUE) {
		fprintf(stderr, "Failed to init keyboard UI\n");
		return -1;
	}

	g_signal_connect(G_OBJECT(keyboard), "kbd-func",
			G_CALLBACK(gtk_ui_kbd_func_cb), keyboard_ui);

	if(opts.embedded) {
		g_signal_connect(G_OBJECT(kbd_data.plug_window), "embedded",
				G_CALLBACK(panel_plug_embedded), NULL);
		fprintf(stdout, "%d\n", gtk_plug_get_id(GTK_PLUG(kbd_data.plug_window)));
		fflush(stdout);
	}

	TIMER_ELAPSED(timer);

	the_keyboard_ui = keyboard_ui;

	kbd_initialized = TRUE;
	gtk_main();
	kbd_initialized = FALSE;

	kbd_cleanup();

done:
	TIMER_STOP(timer);
	TIMER_DESTROY(timer);

	if (app)
		g_object_unref(app);
	return 0;
}

