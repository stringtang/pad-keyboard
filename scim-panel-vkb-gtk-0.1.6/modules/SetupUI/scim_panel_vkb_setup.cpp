/**
 * scim_panel_vkb_setup.cpp:implementation of
 * Setup Module of scim-panel-vkb-gtk.
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Vincent Huang  <chenglan.huang@intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_PANEL

#include <gtk/gtk.h>
#include <sys/stat.h>
#include "scim.h"

#include "config.h"
#include "panel-settings.h"

#if defined(ENABLE_NLS)
#include <glib/gi18n-lib.h>
#else
#define _(String) (String)
#define N_(String) (String)
#define bindtextdomain(Package,Directory)
#define bind_textdomain_codeset(domain,codeset)
#endif

using namespace scim;

#define scim_module_init panel_vkb_setup_LTX_scim_module_init
#define scim_module_exit panel_vkb_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       panel_vkb_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    panel_vkb_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        panel_vkb_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description panel_vkb_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     panel_vkb_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     panel_vkb_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   panel_vkb_setup_LTX_scim_setup_module_query_changed

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("Panel");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Virtual Keyboard"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("A panel daemon with VKB."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"

// Internal data declaration.
static bool   __config_toolbar_auto_snap         = bool (SCIM_DEFAULT_VKB_PANEL_TOOLBAR_AUTO_SNAP);
static int    __config_toolbar_hide_timeout      = gint (SCIM_DEFAULT_VKB_PANEL_TOOLBAR_HIDE_TIMEOUT);
static bool   __config_toolbar_show_factory_icon = bool (SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_FACTORY_ICON);
static bool   __config_toolbar_show_factory_name = bool (SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_FACTORY_NAME);
static bool   __config_toolbar_show_menu_icon    = bool (SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_MENU_ICON);
static bool   __config_toolbar_show_help_icon    = bool (SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_HELP_ICON);
static bool   __config_toolbar_show_property_label = bool (SCIM_DEFAULT_VKB_PANEL_TOOLBAR_SHOW_PROPERTY_LABEL);
static bool   __config_vkb_enable                = bool (SCIM_DEFAULT_VKB_PANEL_VKB_ENABLED);
static bool   __config_vkb_dockpanel             = bool (SCIM_DEFAULT_VKB_DOCKPANEL);
static bool   __config_vkb_panel_extend          = bool (SCIM_DEFAULT_VKB_PANEL_EXTEND);
static gint   __config_vkb_window_width          = gint (SCIM_DEFAULT_VKB_WINDOW_WIDTH);
static gint   __config_vkb_window_height         = gint (SCIM_DEFAULT_VKB_WINDOW_HEIGHT);

static String __config_font                      = String (SCIM_DEFAULT_VKB_PANEL_FONT);
static String __config_vkb_program               = String (SCIM_DEFAULT_VKB_PROGRAM);
static String __config_vkb_parameters            = String (SCIM_DEFAULT_VKB_PARAMETERS);
static String __config_vkb_panel_normal_text_color  = String (SCIM_DEFAULT_VKB_PANEL_NORMAL_TEXT_COLOR);
static String __config_vkb_panel_normal_bg_color    = String (SCIM_DEFAULT_VKB_PANEL_NORMAL_BG_COLOR);
static String __config_vkb_panel_active_text_color  = String (SCIM_DEFAULT_VKB_PANEL_ACTIVE_TEXT_COLOR);
static String __config_vkb_panel_active_bg_color    = String (SCIM_DEFAULT_VKB_PANEL_ACTIVE_BG_COLOR);

static bool   __have_changed                     = false;
static gint   __default_screen_width             = gint (SCIM_DEFAULT_VKB_WINDOW_WIDTH);
static gint   __default_screen_height            = gint (SCIM_DEFAULT_VKB_WINDOW_HEIGHT);
static gint   __default_drawarea_width           = 30;
static gint   __default_drawarea_height          = 20;

static GtkWidget * window                              = 0;
static GtkWidget * __widget_toolbar_hide_timeout      = 0;
static GtkWidget * __widget_toolbar_show_factory_icon  = 0;
static GtkWidget * __widget_toolbar_show_factory_name  = 0;
static GtkWidget * __widget_toolbar_show_menu_icon   = 0;
static GtkWidget * __widget_toolbar_show_help_icon    = 0;
static GtkWidget * __widget_toolbar_show_property_label = 0;
static GtkWidget * __widget_font                      = 0;
static GtkWidget * __widget_vkb_enable                = 0;
static GtkWidget * __widget_vkb_pager                 = 0;
static GtkWidget * __widget_vkb_program               = 0;
static GtkWidget * __widget_vkb_program_selection     = 0;
static GtkWidget * __widget_vkb_parameters            = 0;
static GtkWidget * __widget_vkb_dockpanel             = 0;
static GtkWidget * __widget_vkb_panel_extend          = 0;
static GtkWidget * __widget_vkb_window_height         = 0;
static GtkWidget * __widget_vkb_window_width          = 0;
static GtkWidget * __widget_vkb_set_default           = 0;
static GtkWidget * __widget_vkb_normal_text_color     = 0;
static GtkWidget * __widget_vkb_normal_bg_color       = 0;
static GtkWidget * __widget_vkb_active_text_color     = 0;
static GtkWidget * __widget_vkb_active_bg_color       = 0;


static GtkTooltips * __widget_tooltips                = 0;

enum ToolbarShowFlavourType {
    SCIM_TOOLBAR_SHOW_ALWAYS,
    SCIM_TOOLBAR_SHOW_ON_DEMAND,
    SCIM_TOOLBAR_SHOW_NEVER
};

// Declaration of internal functions.
static void
on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                      gpointer         user_data);

static void
on_enable_vkb_button_toggled         (GtkToggleButton *togglebutton,
		                              gpointer   user_data);

static void
on_default_spin_button_changed       (GtkSpinButton   *spinbutton,
                                      gpointer         user_data);

static void
on_font_selection_clicked            (GtkButton       *button,
                                      gpointer         user_data);

static void
on_vkb_program_changed               (GtkEntry         *entry,
                                      gpointer         user_data);

static void
on_vkb_program_selection_clicked     (GtkButton *button,
                                      gpointer   user_data);

static void
on_vkb_parameters_changed            (GtkEntry         *entry,
                                      gpointer         user_data);

static void
on_vkb_set_default                   (GtkButton *button,
                                      gpointer   user_data);

static void
on_color_selection_clicked           (GtkButton *button,
		                              gpointer user_data);

static void
setup_widget_value ();

// Function implementations.
GtkWidget *
create_setup_window ()
{
    window = 0;

    GdkScreen* screen = NULL;

    screen = gdk_screen_get_default();
    __default_screen_width = gdk_screen_get_width(screen);
    __default_screen_height = gdk_screen_get_height(screen);

    if (!window) {
        GtkWidget *notebook;
        GtkWidget *table;
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *label;
        GtkWidget *hbox;

        __widget_tooltips = gtk_tooltips_new ();
        // Create the Notebook.
        notebook = gtk_notebook_new ();
        gtk_widget_show (notebook);

        // Create the Input Method panel setup page.
        vbox = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vbox);
        gtk_container_add (GTK_CONTAINER (notebook), vbox);

        // Create the label for this note page.
        label = gtk_label_new (_("IM Panel"));
        gtk_widget_show (label);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), label);

        // Create the ToolBar setup block.
        frame = gtk_frame_new (_("ToolBar"));
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);

        table = gtk_table_new (4, 2, FALSE);
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (frame), table);
        gtk_table_set_row_spacings (GTK_TABLE (table), 4);
        gtk_table_set_col_spacings (GTK_TABLE (table), 8);

        __widget_toolbar_show_factory_icon = gtk_check_button_new_with_mnemonic (
                                             _("Show _input method icon"));
        gtk_widget_show (__widget_toolbar_show_factory_icon);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_factory_icon, 0, 1, 0, 1,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        __widget_toolbar_show_factory_name = gtk_check_button_new_with_mnemonic (
                                             _("Show inp_ut method name"));
        gtk_widget_show (__widget_toolbar_show_factory_name);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_factory_name, 0, 1, 1, 2,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 0, 1,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("Hi_de timeout:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

        __widget_toolbar_hide_timeout = gtk_spin_button_new_with_range (0, 60, 1);
        gtk_widget_show (__widget_toolbar_hide_timeout);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_toolbar_hide_timeout, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_toolbar_hide_timeout);


        __widget_toolbar_show_menu_icon = gtk_check_button_new_with_mnemonic (_("Show m_enu icon"));
        gtk_widget_show (__widget_toolbar_show_menu_icon);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_menu_icon, 1, 2, 1, 2,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        __widget_toolbar_show_help_icon = gtk_check_button_new_with_mnemonic (_("Show _help icon"));
        gtk_widget_show (__widget_toolbar_show_help_icon);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_help_icon, 1, 2, 2, 3,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        __widget_toolbar_show_property_label = gtk_check_button_new_with_mnemonic (_("Show _property label"));
        gtk_widget_show (__widget_toolbar_show_property_label);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_property_label, 0, 1, 2, 3,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        hbox = gtk_hbox_new (FALSE, 8);
        gtk_widget_show (hbox);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

        frame = gtk_frame_new (_("Mode"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

        table = gtk_table_new (2,3,false);
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (frame), table);

        __widget_vkb_enable = gtk_check_button_new_with_mnemonic (_("Enable virtual _keyboard"));
        gtk_widget_show (__widget_vkb_enable);
		gtk_table_attach(GTK_TABLE(table), __widget_vkb_enable, 0,1,0,1,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) (GTK_EXPAND), 4, 0);

        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
		gtk_table_attach(GTK_TABLE(table), hbox, 1,2,0,1,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("_Font:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

        __widget_font = gtk_button_new_with_label (_("default"));
        gtk_widget_show (__widget_font);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_font), 4);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_font, FALSE, FALSE, 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_font);

		//panel text color selector
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
		gtk_table_attach(GTK_TABLE(table), hbox, 0,1,1,2,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("Normal_Text:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

		__widget_vkb_normal_text_color = gtk_button_new();
		gtk_widget_set_size_request(GTK_WIDGET(__widget_vkb_normal_text_color), __default_drawarea_width, __default_drawarea_height);

        gtk_widget_show(__widget_vkb_normal_text_color);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_vkb_normal_text_color, FALSE, FALSE, 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_vkb_normal_text_color);

		//panel background color selector
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
		gtk_table_attach(GTK_TABLE(table), hbox, 1,2,1,2,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("_NormalBG:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

        __widget_vkb_normal_bg_color =  gtk_button_new();
		gtk_widget_set_size_request(GTK_WIDGET(__widget_vkb_normal_bg_color), __default_drawarea_width, __default_drawarea_height);
        gtk_widget_show (__widget_vkb_normal_bg_color);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_vkb_normal_bg_color, FALSE, FALSE, 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_vkb_normal_bg_color);

		//panel active text color selector
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
		gtk_table_attach(GTK_TABLE(table), hbox, 0,1,2,3,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("_ActiveText:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

        __widget_vkb_active_text_color =  gtk_button_new();
		gtk_widget_set_size_request(GTK_WIDGET(__widget_vkb_active_text_color), __default_drawarea_width, __default_drawarea_height);
        gtk_widget_show (__widget_vkb_active_text_color);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_vkb_active_text_color, FALSE, FALSE, 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_vkb_active_text_color);

		//panel active bg color selector
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
		gtk_table_attach(GTK_TABLE(table), hbox, 1,2,2,3,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("Active_BG:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

        __widget_vkb_active_bg_color =  gtk_button_new();
		gtk_widget_set_size_request(GTK_WIDGET(__widget_vkb_active_bg_color), __default_drawarea_width, __default_drawarea_height);
        gtk_widget_show (__widget_vkb_active_bg_color);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_vkb_active_bg_color, FALSE, FALSE, 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_vkb_active_bg_color);

        // Create the Virtual Keyboard setup page.
        __widget_vkb_pager = gtk_vbox_new(FALSE, 0);
        gtk_widget_show (__widget_vkb_pager);
        gtk_container_add (GTK_CONTAINER (notebook), __widget_vkb_pager);

        // Create the label for this note page.
        label = gtk_label_new (_("VKB"));
        gtk_widget_show (label);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook),
                gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 1), label);

        frame = gtk_frame_new (_("Options"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (__widget_vkb_pager), frame, FALSE, FALSE, 0);

        table = gtk_table_new (2, 2, FALSE);
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (frame), table);
        gtk_table_set_row_spacings (GTK_TABLE (table), 4);
        gtk_table_set_col_spacings (GTK_TABLE (table), 1);

        label = gtk_label_new_with_mnemonic (_("VKB _Program:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                (GtkAttachOptions) (0),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox,1, 2, 0, 1,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        __widget_vkb_program = gtk_entry_new ();
        gtk_widget_show(__widget_vkb_program);
        gtk_box_pack_start(GTK_BOX(hbox), __widget_vkb_program, TRUE, TRUE,0);

        __widget_vkb_program_selection = gtk_button_new_with_label ("...");
        gtk_widget_show(__widget_vkb_program_selection);
        gtk_box_pack_start(GTK_BOX(hbox), __widget_vkb_program_selection ,
                           FALSE, FALSE,2);

        label = gtk_label_new_with_mnemonic (_("Para_meters:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                          (GtkAttachOptions) (0),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        __widget_vkb_parameters = gtk_entry_new();
        gtk_widget_show(__widget_vkb_parameters);
        gtk_table_attach (GTK_TABLE (table), __widget_vkb_parameters,
                          1, 2, 1, 2,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        frame = gtk_frame_new (_("Layout"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (__widget_vkb_pager), frame, TRUE, TRUE, 0);

        table = gtk_table_new(3, 2, FALSE);
        gtk_widget_show(table);
        gtk_container_add (GTK_CONTAINER (frame), table);
        gtk_table_set_row_spacings (GTK_TABLE (table), 4);
        gtk_table_set_col_spacings (GTK_TABLE (table), 8);

        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, 0, 1,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("_Width:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

        __widget_vkb_window_width = gtk_spin_button_new_with_range(0, __default_screen_width, 1);
        gtk_widget_show(__widget_vkb_window_width);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_vkb_window_width, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_vkb_window_width), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_vkb_window_width), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_vkb_window_width), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_vkb_window_width);


        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 0, 1,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        label = gtk_label_new_with_mnemonic (_("_Height:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);

        __widget_vkb_window_height = gtk_spin_button_new_with_range(0,
                                     __default_screen_height, 1);
        gtk_widget_show(__widget_vkb_window_height);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_vkb_window_height, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_vkb_window_height), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_vkb_window_height), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_vkb_window_height), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_vkb_window_height);

        __widget_vkb_dockpanel = gtk_check_button_new_with_mnemonic (_("_Dock panel"));
        gtk_widget_show (__widget_vkb_dockpanel);
        gtk_table_attach (GTK_TABLE (table), __widget_vkb_dockpanel, 0, 1, 1, 2,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        __widget_vkb_panel_extend = gtk_check_button_new_with_mnemonic (
                _("_Extended VKB Panel"));
        gtk_widget_show (__widget_vkb_panel_extend);
        gtk_table_attach (GTK_TABLE (table), __widget_vkb_panel_extend, 1, 2, 1, 2,
                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);

        __widget_vkb_set_default = gtk_button_new_with_mnemonic (_("Reset Default _Value"));
        gtk_widget_show(__widget_vkb_set_default);
        gtk_table_attach (GTK_TABLE (table), __widget_vkb_set_default, 0, 1, 2, 3,
                (GtkAttachOptions) (GTK_EXPAND),
                (GtkAttachOptions) (GTK_EXPAND), 4, 0);


        // Connect all signals.
        g_signal_connect ((gpointer) __widget_toolbar_hide_timeout, "value_changed",
                G_CALLBACK (on_default_spin_button_changed),
                &__config_toolbar_hide_timeout);

        g_signal_connect ((gpointer) __widget_toolbar_show_factory_icon, "toggled",
                G_CALLBACK (on_default_toggle_button_toggled),
                &__config_toolbar_show_factory_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_factory_name, "toggled",
                G_CALLBACK (on_default_toggle_button_toggled),
                &__config_toolbar_show_factory_name);


        g_signal_connect ((gpointer) __widget_toolbar_show_menu_icon, "toggled",
                G_CALLBACK (on_default_toggle_button_toggled),
                &__config_toolbar_show_menu_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_help_icon, "toggled",
                G_CALLBACK (on_default_toggle_button_toggled),
                &__config_toolbar_show_help_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_property_label, "toggled",
                G_CALLBACK (on_default_toggle_button_toggled),
                &__config_toolbar_show_property_label);


        g_signal_connect ((gpointer) __widget_font, "clicked",
                G_CALLBACK (on_font_selection_clicked),
                NULL);
    
		g_signal_connect((gpointer) __widget_vkb_enable, "toggled",
				G_CALLBACK (on_enable_vkb_button_toggled),
				&__config_vkb_enable);

        //for VKB
        g_signal_connect ((gpointer) __widget_vkb_program, "changed",
                G_CALLBACK (on_vkb_program_changed),
                NULL);

        g_signal_connect ((gpointer) __widget_vkb_program_selection, "clicked",
                G_CALLBACK (on_vkb_program_selection_clicked),
                NULL);

        g_signal_connect ((gpointer) __widget_vkb_parameters, "changed",
                G_CALLBACK (on_vkb_parameters_changed),
                NULL);

        g_signal_connect ((gpointer) __widget_vkb_window_width, "value_changed",
                G_CALLBACK (on_default_spin_button_changed),
                &__config_vkb_window_width);

        g_signal_connect ((gpointer) __widget_vkb_window_height, "value_changed",
                G_CALLBACK (on_default_spin_button_changed),
                &__config_vkb_window_height);

        g_signal_connect ((gpointer) __widget_vkb_dockpanel, "toggled",
                G_CALLBACK (on_default_toggle_button_toggled),
                &__config_vkb_dockpanel);

        g_signal_connect ((gpointer) __widget_vkb_panel_extend, "toggled",
                G_CALLBACK (on_default_toggle_button_toggled),
                &__config_vkb_panel_extend);

        g_signal_connect ((gpointer) __widget_vkb_set_default, "clicked",
                G_CALLBACK (on_vkb_set_default),
                NULL);


		//For panel color setting
		g_signal_connect ((gpointer) __widget_vkb_normal_text_color, "clicked",
				G_CALLBACK (on_color_selection_clicked),
				&__config_vkb_panel_normal_text_color);

		g_signal_connect ((gpointer) __widget_vkb_normal_bg_color, "clicked",
				G_CALLBACK (on_color_selection_clicked),
				&__config_vkb_panel_normal_bg_color);

		g_signal_connect ((gpointer) __widget_vkb_active_text_color, "clicked",
				G_CALLBACK (on_color_selection_clicked),
				&__config_vkb_panel_active_text_color);

		g_signal_connect ((gpointer) __widget_vkb_active_bg_color, "clicked",
				G_CALLBACK (on_color_selection_clicked),
				&__config_vkb_panel_active_bg_color);

        // Set all tooltips.
        gtk_tooltips_set_tip (__widget_tooltips, __widget_toolbar_hide_timeout,
               _("The toolbar will be hidden out after "
                 "this timeout is elapsed. "
                 "This option is only valid when "
                 "\"Always show\" is selected. "
                 "Set to zero to disable this behavior."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_toolbar_show_factory_icon,
               _("If this option is checked, "
                 "the input method icon will be showed on the toolbar."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_toolbar_show_factory_name,
               _("If this option is checked, "
                 "the input method name will be showed on the toolbar."), NULL);


        gtk_tooltips_set_tip (__widget_tooltips, __widget_toolbar_show_menu_icon,
               _("If this option is checked, "
                 "the menu icon will be showed on the toolbar."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_toolbar_show_help_icon,
               _("If this option is checked, "
                 "the help icon will be showed on the toolbar."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_toolbar_show_property_label,
               _("If this option is checked, "
                 "the text label of input method properties will be showed on the toolbar."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_font,
               _("The font setting will be used in "
                 "the input and lookup table windows."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_program,
               _("Set the virtual keyboard program" ), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_parameters,
               _("Set runtime parameters for the virtual keyboard program"), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_window_width,
               _("Set vkb panel's width"), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_window_height,
               _("Set vkb panel's height"), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_dockpanel,
               _("If this option is checked, "
                 "the vkb panel will always be showed on the bottom of screen."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_panel_extend,
               _("If this option is checked, "
                 "the vkb panel will extend to the screen width no matter what window width you set."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_set_default,
               _("Reset all the options to default value "), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_normal_text_color,
               _("To choose the text color of input panel under normal stat."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_normal_bg_color,
               _("To choose the background color of input panel under normal stat."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_active_text_color,
               _("To choose the text color of input panel under normal stat."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_vkb_active_bg_color,
               _("To choose the background color of input panel under normal stat."), NULL);

        window = notebook;

        setup_widget_value ();
    }
    return window;
}

    void
setup_widget_value ()
{
	GdkColor  color;

    if (__widget_toolbar_hide_timeout) {
        gtk_spin_button_set_value (
                GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout),
                __config_toolbar_hide_timeout);
    }

    if (__widget_toolbar_show_factory_icon) {
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_toolbar_show_factory_icon),
                __config_toolbar_show_factory_icon);
    }

    if (__widget_toolbar_show_factory_name) {
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_toolbar_show_factory_name),
                __config_toolbar_show_factory_name);
    }


    if (__widget_toolbar_show_menu_icon) {
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_toolbar_show_menu_icon),
                __config_toolbar_show_menu_icon);
    }

    if (__widget_toolbar_show_help_icon) {
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_toolbar_show_help_icon),
                __config_toolbar_show_help_icon);
    }

    if (__widget_toolbar_show_property_label) {
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_toolbar_show_property_label),
                __config_toolbar_show_property_label);
    }


	if(__widget_vkb_enable)
	{
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(__widget_vkb_enable),
				__config_vkb_enable);
		if(__widget_vkb_pager)
		{
			gtk_widget_set_sensitive(
					__widget_vkb_pager,
					__config_vkb_enable
					);
		}
	}

    if (__widget_font) {
        gtk_button_set_label (
                GTK_BUTTON (__widget_font),
                __config_font.c_str ());
    }

    if (__widget_vkb_program) {
        gtk_entry_set_text  (
                GTK_ENTRY (__widget_vkb_program),
                __config_vkb_program.c_str());
    }

    if (__widget_vkb_parameters) {
        gtk_entry_set_text  (
                GTK_ENTRY (__widget_vkb_parameters),
                __config_vkb_parameters.c_str());
    }

    if (__widget_vkb_window_width) {
        gtk_spin_button_set_value (
                GTK_SPIN_BUTTON (__widget_vkb_window_width),
                __config_vkb_window_width);
    }

    if (__widget_vkb_window_height) {
        gtk_spin_button_set_value (
                GTK_SPIN_BUTTON (__widget_vkb_window_height),
                __config_vkb_window_height);
    }

    if (__widget_vkb_dockpanel) {
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_vkb_dockpanel),
                __config_vkb_dockpanel);
    }

    if (__widget_vkb_panel_extend) {
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_vkb_panel_extend),
                __config_vkb_panel_extend);
    }

	if(__widget_vkb_normal_text_color){
		gdk_color_parse(__config_vkb_panel_normal_text_color.c_str(), &color);
		gtk_widget_modify_bg (__widget_vkb_normal_text_color, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg (__widget_vkb_normal_text_color, GTK_STATE_ACTIVE, &color);
		gtk_widget_modify_bg (__widget_vkb_normal_text_color, GTK_STATE_PRELIGHT, &color);
	}

	if(__widget_vkb_normal_bg_color){
		gdk_color_parse(__config_vkb_panel_normal_bg_color.c_str(), &color);
		gtk_widget_modify_bg (__widget_vkb_normal_bg_color, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg (__widget_vkb_normal_bg_color, GTK_STATE_ACTIVE, &color);
		gtk_widget_modify_bg (__widget_vkb_normal_bg_color, GTK_STATE_PRELIGHT, &color);
	}

	if(__widget_vkb_active_text_color){
		gdk_color_parse(__config_vkb_panel_active_text_color.c_str(), &color);
		gtk_widget_modify_bg (__widget_vkb_active_text_color, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg (__widget_vkb_active_text_color, GTK_STATE_ACTIVE, &color);
		gtk_widget_modify_bg (__widget_vkb_active_text_color, GTK_STATE_PRELIGHT, &color);
	}

	if(__widget_vkb_active_bg_color){
		gdk_color_parse(__config_vkb_panel_active_bg_color.c_str(), &color);
		gtk_widget_modify_bg (__widget_vkb_active_bg_color, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg (__widget_vkb_active_bg_color, GTK_STATE_ACTIVE, &color);
		gtk_widget_modify_bg (__widget_vkb_active_bg_color, GTK_STATE_PRELIGHT, &color);
	}

}

    void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        __config_toolbar_auto_snap =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_AUTO_SNAP),
                    __config_toolbar_auto_snap);
        __config_toolbar_hide_timeout =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_HIDE_TIMEOUT),
                    __config_toolbar_hide_timeout);
        __config_toolbar_show_factory_icon =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_FACTORY_ICON),
                    __config_toolbar_show_factory_icon);
        __config_toolbar_show_factory_name =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_FACTORY_NAME),
                    __config_toolbar_show_factory_name);
        __config_toolbar_show_menu_icon =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_MENU_ICON),
                    __config_toolbar_show_menu_icon);
        __config_toolbar_show_help_icon =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_HELP_ICON),
                    __config_toolbar_show_help_icon);
        __config_toolbar_show_property_label =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_PROPERTY_LABEL),
                    __config_toolbar_show_property_label);
        __config_font =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_FONT),
                    __config_font);
        __config_vkb_program = 
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_PROGRAM),
                    __config_vkb_program);
        __config_vkb_parameters = 
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_PARAMETERS),
                    __config_vkb_parameters);
        __config_vkb_window_width = 
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_WIDTH),
                    __config_vkb_window_width);
        __config_vkb_window_height = 
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_HEIGHT),
                    __config_vkb_window_height);
        __config_vkb_dockpanel = 
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_DOCK_PANEL),
                    __config_vkb_dockpanel);
        __config_vkb_panel_extend = 
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_EXTEND),
                    __config_vkb_panel_extend);
        __config_vkb_enable =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_ENABLE_VKB),
                    __config_vkb_enable);
        __config_vkb_panel_normal_text_color =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_NORMAL_TEXT_COLOR),
                    __config_vkb_panel_normal_text_color);
        __config_vkb_panel_normal_bg_color =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_NORMAL_BG_COLOR),
                    __config_vkb_panel_normal_bg_color);
        __config_vkb_panel_active_text_color =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_ACTIVE_TEXT_COLOR),
                    __config_vkb_panel_active_text_color);
        __config_vkb_panel_active_bg_color =
            config->read (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_ACTIVE_BG_COLOR),
                    __config_vkb_panel_active_bg_color);
        setup_widget_value ();

        __have_changed = false;
    }
}

    void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_AUTO_SNAP),
                __config_toolbar_auto_snap);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_HIDE_TIMEOUT),
                __config_toolbar_hide_timeout);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_FACTORY_ICON),
                __config_toolbar_show_factory_icon);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_FACTORY_NAME),
                __config_toolbar_show_factory_name);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_MENU_ICON),
                __config_toolbar_show_menu_icon);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_HELP_ICON),
                __config_toolbar_show_help_icon);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_TOOLBAR_SHOW_PROPERTY_LABEL),
                __config_toolbar_show_property_label);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_FONT),
                __config_font);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_PROGRAM),
                __config_vkb_program);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_PARAMETERS),
                __config_vkb_parameters);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_WIDTH),
                __config_vkb_window_width);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_VKB_HEIGHT),
                __config_vkb_window_height);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_DOCK_PANEL),
                __config_vkb_dockpanel);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_EXTEND),
                __config_vkb_panel_extend);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_ENABLE_VKB),
                __config_vkb_enable);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_NORMAL_TEXT_COLOR),
                __config_vkb_panel_normal_text_color);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_NORMAL_BG_COLOR),
                __config_vkb_panel_normal_bg_color);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_ACTIVE_TEXT_COLOR),
                __config_vkb_panel_active_text_color);
        config->write (String (SCIM_CONFIG_PANEL_VKB_GTK_PANEL_ACTIVE_BG_COLOR),
                __config_vkb_panel_active_bg_color);

        __have_changed = false;
    }
}

    bool
query_changed ()
{
    return __have_changed;
}

    static void
on_default_spin_button_changed (GtkSpinButton *spinbutton,
        gpointer       user_data)
{
    int *value = static_cast <int *> (user_data);

    if (value) {
        *value = gtk_spin_button_get_value_as_int (spinbutton);
        __have_changed = true;
    }
}

    static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
        gpointer         user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
}

static void
on_enable_vkb_button_toggled (GtkToggleButton *togglebutton,
		gpointer 		user_data)
{
	bool *toggle = static_cast<bool*> (user_data);

	if(toggle)
	{
		*toggle = gtk_toggle_button_get_active (togglebutton);
		__have_changed = true;
	}
	if(__widget_vkb_pager)
	{
		gtk_widget_set_sensitive(
				__widget_vkb_pager,
				__config_vkb_enable
				);
	}
}

static void
on_font_selection_clicked (GtkButton *button,
        gpointer   user_data)
{
    GtkWidget *font_selection = gtk_font_selection_dialog_new (_("Select Interface Font"));
    gint result;

    if (__config_font != _("default")) {
        gtk_font_selection_dialog_set_font_name (
                GTK_FONT_SELECTION_DIALOG (font_selection),
                __config_font.c_str ());
    }

    result = gtk_dialog_run (GTK_DIALOG (font_selection));

    if (result == GTK_RESPONSE_OK) {
        __config_font = String (
                gtk_font_selection_dialog_get_font_name (
                    GTK_FONT_SELECTION_DIALOG (font_selection)));

        gtk_button_set_label (
                GTK_BUTTON (__widget_font),
                __config_font.c_str ());

        __have_changed = true;
    }

    gtk_widget_destroy (font_selection);
}

static void
on_vkb_program_changed (GtkEntry *entry,
                            gpointer     user_data)
{
    __config_vkb_program = gtk_entry_get_text(entry);
    __have_changed = true;
}

static void
on_vkb_parameters_changed (GtkEntry *entry,
                            gpointer     user_data)
{
    __config_vkb_parameters = gtk_entry_get_text(entry);
    __have_changed = true;
}

static void
on_vkb_program_selection_clicked (GtkButton *button,
        gpointer   user_data)
{
    GtkWidget *vkb_program_selection = gtk_file_selection_new(_("Select Virtual Keyboard Program"));
    gint result;
    String tmp;
    result = gtk_dialog_run (GTK_DIALOG (vkb_program_selection));

    if (result == GTK_RESPONSE_OK) {
        struct stat filestat;
        tmp = String (
            gtk_file_selection_get_filename(GTK_FILE_SELECTION(vkb_program_selection)));
        //check if file exists and is excutable
        stat (tmp.c_str (), &filestat);
        if(S_ISREG (filestat.st_mode) &&  (S_IXUSR&filestat.st_mode))
        {
            __config_vkb_program = tmp; 
        }
        gtk_entry_set_text  (
                GTK_ENTRY (__widget_vkb_program),
                __config_vkb_program.c_str());
        __have_changed = true;
    }

    gtk_widget_destroy (vkb_program_selection);
}

static void
on_color_selection_clicked(GtkButton *button,
		gpointer user_data)
{
	GtkColorSelection *colorsel;
	gint result;
	GdkColor color;
    String * colorstr = static_cast<String *> (user_data);
	if (! colorstr)
	{
		return;
	}
	GtkWidget *dialog = gtk_color_selection_dialog_new(_("Select Color"));
	gdk_color_parse((*colorstr).c_str(), &color);

	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

	colorsel = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);

	gtk_color_selection_set_previous_color (colorsel, &color);
	gtk_color_selection_set_current_color (colorsel, &color);
	gtk_color_selection_set_has_palette (colorsel, TRUE);

	result = gtk_dialog_run (GTK_DIALOG (dialog));
    if (result == GTK_RESPONSE_OK) {
		gtk_color_selection_get_current_color (colorsel, &color);
		*colorstr = String(gdk_color_to_string(&color));
		gdk_color_parse((*colorstr).c_str(), &color);
		gtk_widget_modify_bg (GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg (GTK_WIDGET(button), GTK_STATE_ACTIVE, &color);
		gtk_widget_modify_bg (GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
		__have_changed = true;
	}
	gtk_widget_destroy (dialog);
}


static void
on_vkb_set_default(GtkButton *button,
		gpointer   user_data)
{
	__config_vkb_dockpanel             = bool (SCIM_DEFAULT_VKB_DOCKPANEL);
	__config_vkb_panel_extend          = bool (SCIM_DEFAULT_VKB_PANEL_EXTEND);
	__config_vkb_window_width          = gint (SCIM_DEFAULT_VKB_WINDOW_WIDTH);
	__config_vkb_window_height         = gint (SCIM_DEFAULT_VKB_WINDOW_HEIGHT);
	__config_vkb_program               = String (SCIM_DEFAULT_VKB_PROGRAM);
	__config_vkb_parameters            = String (SCIM_DEFAULT_VKB_PARAMETERS);
	__config_vkb_panel_normal_text_color  = String (SCIM_DEFAULT_VKB_PANEL_NORMAL_TEXT_COLOR);
	__config_vkb_panel_normal_bg_color    = String (SCIM_DEFAULT_VKB_PANEL_NORMAL_BG_COLOR);
	__config_vkb_panel_active_text_color  = String (SCIM_DEFAULT_VKB_PANEL_ACTIVE_TEXT_COLOR);
	__config_vkb_panel_active_bg_color    = String (SCIM_DEFAULT_VKB_PANEL_ACTIVE_BG_COLOR);
	setup_widget_value ();

	__have_changed                     = true;
}
