/*
 * misc-utility.c misc functions
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

#include <sys/wait.h>
#include "fvkbd.h"
#include <X11/Xatom.h>

#include "misc-utility.h"

#define FONT_DELIMITER "_"
#define FONT_SPLIT_NUM 3

static Display *the_xdpy = NULL;

Display *
get_x_display(void)
{
	if (the_xdpy == NULL)
		the_xdpy = XOpenDisplay(getenv("DISPLAY"));

	return the_xdpy;
}


FakeKey *
get_fakekey_instance(void)
{
	static FakeKey *fakekey_instance = NULL;

	if (G_UNLIKELY(fakekey_instance == NULL)) {
		fakekey_instance = fakekey_init(get_x_display());
	}

	return fakekey_instance;
}

gboolean
get_workarea(int *x, int *y, int *width, int *height)
{
	Atom workarea_atom, type;
	int ret, format;
	unsigned long item_nums, remain_bytes;
	unsigned char *workarea = NULL;
	int size;

	Display *xdpy;
	int xscreen;

	xdpy = get_x_display();
	xscreen = DefaultScreen(xdpy);
	workarea_atom = XInternAtom(get_x_display(), "_NET_WORKAREA", FALSE);

	ret = XGetWindowProperty(xdpy, RootWindow(xdpy, xscreen), workarea_atom,
				0, 16L, FALSE, XA_CARDINAL, &type, &format,
				&item_nums, &remain_bytes, (unsigned char **)&workarea);

	if (ret != Success || item_nums < 4 || workarea == NULL) {
		if (workarea)
			XFree(workarea);
		return FALSE;
	}

	size = format / 8;
	if ((format == 32) && (size != sizeof(long))){
		// It seems that on some x86_64 machine, the data of _NET_WORKAREA is actually 64 bit
		// though format said it is 32. It might be the bug of WM?
		size = 8; //hack it!
	}

	if (x != NULL)
		*x = *(int*)workarea;
	if (y != NULL)
		*y = *(int*)(workarea + size);
	if (width != NULL)
		*width = *(int*)(workarea + size * 2);
	if (height != NULL)
		*height = *(int*)(workarea + size * 3);
  
	XFree(workarea);

	return TRUE;
}


void
get_resolution(int *xres, int *yres)
{
	Display *xdpy;
	int xscreen;
	static int s_xres = -1, s_yres = -1;

	if (s_xres == -1) {
		xdpy = get_x_display();

		/* FIXME: We might need to get info from the right screen number instead of default */
		xscreen = DefaultScreen(xdpy);

		s_xres = (int)((((double)DisplayWidth(xdpy,xscreen)) * 25.4) /
			((double)DisplayWidthMM(xdpy,xscreen)) + 0.5);
		s_yres = (int)((((double)DisplayHeight(xdpy,xscreen)) * 25.4) /
			((double)DisplayHeightMM(xdpy,xscreen)) + 0.5);
	}
	
	if (xres)
		*xres = s_xres;
	if (yres)
		*yres = s_yres;
}


KbdColor *
kbd_str_to_color(gchar *str)
{
	//FIXME It's just dummy function now.
	unsigned long int tmp;
	KbdColor *color;

	color = g_new0(KbdColor, 1);

	tmp = strtoul(str, NULL, 16);
	color->r = (tmp & 0xFF0000) >> 16;
	color->g = (tmp & 0xFF00) >> 8;
	color->b = (tmp & 0xFF);
	color->a = 0xFF;

	return color;
}

KbdFontInfo *
parse_font_property(gchar *str, gchar *default_font_family, gchar *default_font_weight, gint default_font_size)
{
	gchar **str_vec = NULL;
	KbdFontInfo *font;

	g_assert (default_font_family);
	g_assert (default_font_weight);
	g_assert (default_font_size > 0);

	font = g_new0(KbdFontInfo, 1);

	if (!str)
		goto done;

	str_vec = g_strsplit(str, FONT_DELIMITER, FONT_SPLIT_NUM);

	if (str_vec == NULL)
		goto done;

	if (str_vec[0])
		font->family = g_strdup(str_vec[0]);
	else
		goto done;

	if (str_vec[1])
		font->weight = g_strdup(str_vec[1]);
	else
		goto done;

	if (str_vec[2] && (atoi(str_vec[2]) > 0))
		font->size = atoi(str_vec[2]);
	else
		goto done;

done:
	if (!font->family)
		font->family = g_strdup(default_font_family);
	if (!font->weight)
		font->weight = g_strdup(default_font_weight);
	if (font->size <= 0)
		font->size = default_font_size;

	g_strfreev(str_vec);
	return font;
}


KbdShapeInfo *
parse_shape_info_property (gchar *str)
{
	KbdShapeInfo *shape_info = NULL;

	if (!str)
		goto done;

	shape_info = g_slice_new(KbdShapeInfo);
	shape_info->shape_type = KBD_SHAPE_NULL;

	if (!g_strcmp0(str, "none")) {
		shape_info->shape_type = KBD_SHAPE_NONE;
	} else if (!g_strcmp0(str, "chamfer")) {
		shape_info->shape_type = KBD_SHAPE_CHAMFER;
	} else if (!g_strcmp0(str, "bitmap")) {
		shape_info->shape_type = KBD_SHAPE_BITMAP_MASK;
	} else {
		g_slice_free(KbdShapeInfo, shape_info);
		shape_info = NULL;
		goto done;
	}
done:
	return shape_info;
}


void
parse_shape_data_property (KbdShapeInfo *info, gchar *str)
{
	switch (info->shape_type) {
	case KBD_SHAPE_BITMAP_MASK:
		if (str != NULL) {
			info->u.mask = locate_img_file(str);
			if (info->u.mask == NULL)
				info->shape_type = KBD_SHAPE_NULL;
		} else {
			info->shape_type = KBD_SHAPE_NULL;
		}
		break;
	default:
		break;
	}
}


void
kbd_shape_info_destroy (KbdShapeInfo *info)
{
	switch (info->shape_type) {
	case KBD_SHAPE_BITMAP_MASK:
		g_free(info->u.mask);
		break;
	default:
		break;
	}

	g_slice_free(KbdShapeInfo, info);
}

static inline int file_exist(gchar *filename)
{
	return (g_access(filename, F_OK) == 0);
}


#define USER_PKGDATA_DIR ".fvkbd"
#define SCRIPT_DIR_NAME "scripts"
#define IMG_DIR_NAME "images"


gchar *
locate_script_file(gchar *filename)
{
	gchar *fullname = NULL;
	gchar *path = NULL;

	if (filename == NULL)
		return NULL;

	if (g_path_is_absolute(filename)) {
		if (file_exist(filename))
			fullname = g_strdup(filename);

		goto done;
	}

	path = g_strdup(getenv("HOME"));

	fullname = g_build_filename(path, USER_PKGDATA_DIR, SCRIPT_DIR_NAME, filename, NULL);
	if (file_exist(fullname))
		goto done;

	g_free(fullname);

	fullname = g_build_filename(PKGDATADIR, SCRIPT_DIR_NAME, filename, NULL);
	if (file_exist(fullname))
		goto done;

	fullname = NULL;

done:
	g_free(path);
	return fullname;
}


gchar *
locate_img_file(gchar *filename)
{
	gchar *fullname = NULL;
	gchar *path = NULL;

	if (filename == NULL)
		return NULL;

	if (g_path_is_absolute(filename)) {
		if (file_exist(filename))
			fullname = g_strdup(filename);

		goto done;
	}

	path = g_strdup(getenv("HOME"));

	fullname = g_build_filename(path, USER_PKGDATA_DIR, IMG_DIR_NAME, filename, NULL);
	if (file_exist(fullname))
		goto done;

	g_free(fullname);

	fullname = g_build_filename(PKGDATADIR, IMG_DIR_NAME, filename, NULL);
	if (file_exist(fullname))
		goto done;

	fullname = NULL;

done:
	g_free(path);
	return fullname;
}


void
fvkbd_do_script(gchar *string)
{
	gchar *cmd = NULL;
	gchar **args = NULL;
	pid_t pid;

	if (string == NULL)
		return;

	args = g_strsplit(string, " ", 0);

	cmd = locate_script_file(string);
	if (cmd == NULL)
		return;

	switch (pid = fork()) {
	case 0:
	// trick from APUE to fork twice to not wait for child exit.
		if ((pid = fork()) < 0)
			exit(0);
		else if (pid > 0)
			exit(0);

		execv(cmd, args);
		exit(0);
	case -1:
		fprintf(stdout, "### unable to run script ### \n\n");
		return;
	}

	waitpid(pid, NULL, 0);

	g_free(cmd);
	g_strfreev(args);

	return;
}


gboolean
_fvkbd_boolean_handled_accumulator (GSignalInvocationHint *ihint,
						GValue *accumulate,
						const GValue *h_return,
						gpointer dummy)
{
	gboolean ret;
	gboolean handled;

	handled = g_value_get_boolean(h_return);
	g_value_set_boolean(accumulate, handled);
	ret = !handled;

	return ret;
}


void
_fvkbd_marshal_BOOLEAN__POINTER (GClosure *closure,
					GValue *return_value G_GNUC_UNUSED,
					guint n_param_values,
					const GValue *param_values,
					gpointer invocation_hint G_GNUC_UNUSED,
					gpointer marshal_data)
{
	typedef gboolean (*GMarshalFunc_BOOLEAN__BOXED) (gpointer data1,
							gpointer arg_1,
							gpointer data2);
	register GMarshalFunc_BOOLEAN__BOXED callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;
	gboolean v_return;

	g_return_if_fail (return_value != NULL);
	g_return_if_fail (n_param_values == 2);

	if (G_CCLOSURE_SWAP_DATA (closure)) {
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	} else {
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}

	callback = (GMarshalFunc_BOOLEAN__BOXED) (marshal_data ? marshal_data : cc->callback);

	v_return = callback(data1,
				g_value_get_pointer (param_values + 1),
				data2);

	g_value_set_boolean (return_value, v_return);
}

