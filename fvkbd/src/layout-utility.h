/*
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

#ifndef _LAYOUT_UTILITY_H
#define _LAYOUT_UTILITY_H

G_BEGIN_DECLS

typedef struct _LayoutFileInfo LayoutFileInfo;
struct _LayoutFileInfo {
	gchar *name;
	gchar *filename;
};

#define MAX_LAYOUT_HISTORY 3

void find_layout_files(void);
gchar *find_layout_file(const gchar *filename);
const gchar *get_layout_file_fullname(const gchar *name);
GSList *get_layout_file_lists(void);
void add_recent_layout_file(const gchar *new_name);
GSList *get_layout_history(void);

G_END_DECLS

#endif //_LAYOUT_UTILITY_H