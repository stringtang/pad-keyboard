/*
 * layout-utility.c layout file operation utilities  
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

#include "layout-utility.h"

#define DEFAULT_LAYOUT_FILENAME "keyboard.xml"
#define USER_PKGDATA_DIR ".fvkbd"

#define LAYOUT_EXT ".XML"

static GSList *layout_files = NULL;
static GSList *layout_name_history = NULL;

static inline int file_readable(gchar *filename)
{
	return (g_access(filename, R_OK) == 0);
}


static void
find_layout_files_in_path(gchar *path)
{
	LayoutFileInfo *info;
	GDir *dir;
	const char *file;
	gchar *extend;
	gint len;

	if (!path)
		return;

	dir = g_dir_open(path, 0, NULL);
	if (dir) {
		while ((file = g_dir_read_name(dir))) {
			len = strlen(file);
			if (len < strlen(LAYOUT_EXT))
				break;

			extend = g_ascii_strup((file + len - strlen(LAYOUT_EXT)), -1);
			if (!g_strcmp0(extend, LAYOUT_EXT)) {
				gchar *name = g_strndup(file, (len - strlen(LAYOUT_EXT)));

				// whether there are already layout with the same name
				if (get_layout_file_fullname(name) != NULL) {
					g_free(name);
				} else {
					info = g_new0(LayoutFileInfo, 1);
					info->name = name;
					info->filename = g_build_filename(path, file, NULL);
					layout_files = g_slist_append(layout_files, info);
				}
			}
			g_free(extend);
		}
		g_dir_close (dir);
	}
}


void
find_layout_files(void)
{
	gchar *home, *path;

	home = g_strdup(getenv("HOME"));
	path = g_build_filename(home, USER_PKGDATA_DIR, NULL);
	find_layout_files_in_path(path);

	find_layout_files_in_path(PKGDATADIR);

	g_free(home);
	g_free(path);
}


gchar *
find_layout_file(const gchar *filename)
{
	gchar *fullname = NULL, *name = NULL;
	gchar *path = NULL, *tmp = NULL;

	if (filename == NULL)
		tmp = g_strdup(DEFAULT_LAYOUT_FILENAME);
	else
		tmp = g_strdup(filename);

	if (g_path_is_absolute(tmp)) {
		if (file_readable(tmp))
			fullname = g_strdup(tmp);

		goto done;
	}

	path = g_strdup(getenv("HOME"));

	fullname = g_build_filename(path, USER_PKGDATA_DIR, tmp, NULL);
	if (file_readable(fullname))
		goto done;

	g_free(fullname);

	fullname = g_build_filename(PKGDATADIR, tmp, NULL);
	if (file_readable(fullname))
		goto done;

	fullname = NULL;

done:
	if (fullname != NULL) {
		gchar *file;
		LayoutFileInfo *info;
		gint len;

		file = g_path_get_basename(fullname);
		len = strlen(file);
		name = g_strndup(file, (len - strlen(LAYOUT_EXT)));

		if (get_layout_file_fullname(name) == NULL) {
			info = g_new0(LayoutFileInfo, 1);
			info->name = name;
			info->filename = g_strdup(fullname);
			layout_files = g_slist_append(layout_files, info);
		}

		g_free(file);
	}

	DBG("name = %s, fullname = %s", name, fullname);

	g_free(fullname);
	g_free(tmp);
	g_free(path);

	return name;
}


const gchar *
get_layout_file_fullname(const gchar *name)
{
	int i;
	LayoutFileInfo *info;

	if (layout_files == NULL)
		return NULL;

	for (i = 0; i < g_slist_length(layout_files); i++) {
		info = g_slist_nth_data(layout_files, i);
		if (g_strcmp0(info->name, name))
			continue;
		return info->filename;
	}

	return NULL;
}


GSList *
get_layout_file_lists(void)
{
	return layout_files;
}


void
add_recent_layout_file(const gchar *new_name)
{
	int i;
	gchar *name = NULL;
	gint len;

	if (new_name == NULL)
		return;

	if (layout_name_history == NULL)
		goto add;

	len = g_slist_length(layout_name_history);

	for (i = 0; i < len; i++) {
		name = g_slist_nth_data(layout_name_history, i);
		if (!g_strcmp0(name, new_name))
			break;
	}

	if ((i == len) && (i >= MAX_LAYOUT_HISTORY))
		i = len -1;

	if (i < len) {
		layout_name_history = g_slist_remove(layout_name_history, name);
		g_free(name);
	}
add:
	layout_name_history = g_slist_prepend(layout_name_history, g_strdup(new_name));
}


GSList *
get_layout_history(void)
{
	return layout_name_history;
}

