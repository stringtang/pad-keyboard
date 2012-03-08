/**
 * scim-panel-setup.cpp : scim panel setup module,
 * to select and setup an input panel
 *
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Vincent Huang  <chenglan.huang@intel.com>
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

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_EVENT
#define Uses_SCIM_PANEL

#include <iostream>
#include <gtk/gtk.h>
#include "scim.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#if defined(ENABLE_NLS)
#include <glib/gi18n-lib.h>
#else
#define _(String) (String)
#define N_(String) (String)
#define bindtextdomain(Package,Directory)
#define bind_textdomain_codeset(domain,codeset)
#endif

using namespace scim;

#define scim_module_init aaa_panel_setup_LTX_scim_module_init
#define scim_module_exit aaa_panel_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       aaa_panel_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    aaa_panel_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        aaa_panel_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description aaa_panel_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     aaa_panel_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     aaa_panel_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   aaa_panel_setup_LTX_scim_setup_module_query_changed

#define SCIM_PANEL_PROGRAM_PREFIX "scim-panel-"
#define SCIM_PANEL_UNKNOWN "scim-panel-unknown"

static GtkWidget *    create_setup_window ();
static void           load_config (const ConfigPointer &config);
static void           save_config (const ConfigPointer &config);
static bool           query_changed ();
static void           scim_get_panel_program_paths (std::vector <String> &paths);
static int            scim_get_panel_program_list (std::vector <String>& panel_list);
static void           on_panel_program_changed (GtkComboBox *combobox, gpointer  user_data);
static void           setup_widget_value ();
static GtkWidget *    __widget_panel_program = NULL;
static GtkTooltips *  __widget_tooltips      = 0;
static std::vector <String> panel_list;


// Internal data declaration.
static String __config_panel_program     = String (SCIM_PANEL_UNKNOWN);
static bool   __have_changed             = false;



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
        return String (_("Global Setup"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("Setup the global options used by all the  panel setup modules."));
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

static GtkWidget * create_setup_window ()
{
    static GtkWidget *window = 0;
    if (!window) {
        GtkWidget *frame;
        GtkWidget *hbox;
        GtkWidget *vbox;
        GtkWidget *label;
        int i;

        __widget_tooltips = gtk_tooltips_new();

        //Create the toplevel box
        window = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(window);

        frame = gtk_frame_new (_("Options"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (window), frame, FALSE, FALSE, 0);
        
        vbox = gtk_vbox_new (FALSE, 4);
        gtk_widget_show (vbox);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
        gtk_container_add (GTK_CONTAINER (frame), vbox);
        
        hbox = gtk_hbox_new (FALSE, 4);
        gtk_widget_show (hbox);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

        label = gtk_label_new_with_mnemonic(_("Setup Panel:"));
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        __widget_panel_program = gtk_combo_box_new_text ();
        gtk_widget_show(__widget_panel_program);

        gtk_label_set_mnemonic_widget(GTK_LABEL(label), __widget_panel_program);

        int panel_program_num = scim_get_panel_program_list(panel_list);

        for (i = 0; i < panel_program_num; i++) {
            gtk_combo_box_insert_text(GTK_COMBO_BOX(__widget_panel_program),
                    i, panel_list[i].c_str());
            SCIM_DEBUG_MODULE(0) << "add panel :" << panel_list[i].c_str() << "\n";
        }
        g_signal_connect (G_OBJECT (__widget_panel_program), "changed",
                          G_CALLBACK (on_panel_program_changed),
                          NULL);

        gtk_box_pack_start (GTK_BOX (hbox), __widget_panel_program, TRUE, TRUE, 0);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_panel_program,
                              _("You should choose your currently used panel program here "), NULL);

        setup_widget_value ();
    }
    return window;
}

static void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        __config_panel_program = scim_global_config_read(SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_PROGRAM,
                                                         (String (SCIM_PANEL_UNKNOWN)));
        SCIM_DEBUG_MODULE(0) << "__config_panel_program is :"
                             << __config_panel_program << "\n";
        setup_widget_value ();
        __have_changed = false;
    }
}

static void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        unsigned int act = gtk_combo_box_get_active (GTK_COMBO_BOX (__widget_panel_program));

        if (act >= 0 && act < panel_list.size())
            __config_panel_program = panel_list[act].c_str();
        else
            __config_panel_program = SCIM_PANEL_UNKNOWN;

        scim_global_config_write(String (SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_PROGRAM),
                             __config_panel_program);
        __have_changed = false;
    }
}

static void
setup_widget_value ()
{
        for (unsigned int i = 0; i < panel_list.size(); i++) {
        if(__config_panel_program.compare(panel_list[i].c_str()) == 0)
        {
            gtk_combo_box_set_active (GTK_COMBO_BOX (__widget_panel_program), i);
            break;
        }
    }
}

static void
scim_get_panel_program_paths (std::vector <String> &paths)
{
    paths.clear ();

    String scim_module_dir = (String (SCIM_MODULEDIR)); //for example : "/usr/lib/scim-1.0/1.4.0"
    String scim_binary_version = (String (SCIM_BINARY_VERSION)); //for example "1.4.0"

    //what we need is "/usr/lib/scim-1.0/"
    String default_panel_path = scim_module_dir.substr(0,
                                scim_module_dir.find(scim_binary_version));

    //for now , no user-defined panel path is considered
    paths.push_back (default_panel_path);
}

static int
scim_get_panel_program_list (std::vector <String>& panel_list)
{
    std::vector<String> paths;
    scim_get_panel_program_paths(paths);

    panel_list.clear ();

    for (std::vector<String>::iterator i = paths.begin (); i!= paths.end (); ++i)
    {
        DIR *dir = opendir (i->c_str ());
        if (dir)
        {
            struct dirent *file = readdir (dir);
            while (file)
            {
                struct stat filestat;
                String absfn = *i + String (SCIM_PATH_DELIM_STRING)
                               + file->d_name;
                stat (absfn.c_str (), &filestat);
                //a valid panel name must begin with "scim_panel_"
                if (S_ISREG (filestat.st_mode)
                    && (String (file->d_name)).find(SCIM_PANEL_PROGRAM_PREFIX)
                        != String::npos)
                {
                    //cut off suffix of panel priogram name
                    std::vector<String> vec;
                    scim_split_string_list (vec, String (file->d_name), '.');
                    panel_list.push_back (vec [0]);
                    SCIM_DEBUG_MODULE(0) << "add panel [" << vec[0] << "] \n";
                }
                file = readdir (dir);
            }
            closedir (dir);
        }
    }
    panel_list.push_back(String (SCIM_PANEL_UNKNOWN));
    std::sort (panel_list.begin (), panel_list.end ());
    panel_list.erase (std::unique (panel_list.begin(),
                      panel_list.end()), panel_list.end());
    return panel_list.size ();
}

static void
on_panel_program_changed (GtkComboBox *combobox,
                            gpointer     user_data)
{
    __have_changed = true;
}

static bool
query_changed ()
{
    return __have_changed;
}
