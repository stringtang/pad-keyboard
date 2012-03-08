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

#ifndef _FVKBD_H
#define _FVKBD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>

#include <fakekey/fakekey.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "fvkbd-base.h"

G_BEGIN_DECLS

#if (ENABLE_DEBUG)
#define DBG(x, a...) g_fprintf (stderr,  __FILE__ ",%d,%s: " x "\n", __LINE__, __func__, ##a)
#else
#define DBG(x, a...) do {} while (0)
#endif

#define STEP() DBG("")

#define _(string) string

#if (ENABLE_TIMER)
#define TIMER_NEW() g_timer_new()
#define TIMER_STOP(t) g_timer_stop(t)
#define TIMER_DESTROY(t) g_timer_destroy(t)
#define TIMER_ELAPSED(t) \
do {\
g_timer_stop(t);\
g_printf(__FILE__ ",%d,%s: %f (s)\n", __LINE__, __func__, g_timer_elapsed(t, NULL));\
g_timer_continue(t);\
} while (0)
#else
#define TIMER_NEW() NULL
#define TIMER_STOP(t)
#define TIMER_DESTROY(t)
#define TIMER_ELAPSED(t)
#endif

typedef enum _KbdUIFuncType KbdUIFuncType;
enum _KbdUIFuncType {
	KBD_UI_FUNC_NONE = 0,

	KBD_UI_FUNC_QUIT = 1,
	KBD_UI_FUNC_SHOW_MENU,

	KBD_UI_FUNC_LAST
};


KbdColor *kbd_get_default_kbd_bg_color (void);
KbdColor *kbd_get_default_key_bg_color (void);
KbdColor *kbd_get_default_key_fg_color (void);
KbdColor *kbd_get_default_key_extra_fg_color (void);
KbdColor *kbd_get_default_key_pop_bg_color(void);
KbdColor *kbd_get_default_key_pop_fg_color(void);

gchar *kbd_get_default_font_family (KbdFontType type);
gint kbd_get_default_font_size (KbdFontType type);
gchar *kbd_get_default_font_weight (KbdFontType type);

gchar *kbd_init (gchar *layout_file);
void kbd_cleanup (void);
FvkbdUnit *kbd_load_keyboard(const gchar* name);

G_END_DECLS

#endif //_FVKBD_H
