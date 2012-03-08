if FVKBD_BUILD_GTK_UI

lib_LTLIBRARIES = libfvkbd-gtk-@FVKBD_API_VERSION@.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src

libfvkbd_gtk_@FVKBD_API_VERSION@_la_SOURCES = \
		fvkbd-key-ui-gtk.c	\
		fvkbd-key-ui-gtk.h	\
		fvkbd-keyboard-ui-gtk.c	\
		fvkbd-keyboard-ui-gtk.h	\
		fvkbd-panel-ui-gtk.c	\
		fvkbd-panel-ui-gtk.h	\
		fvkbd-pop-win.c		\
		fvkbd-pop-win.h		\
		gtk-misc-utility.c	\
		gtk-misc-utility.h	\
		gtk-ui-base.c		\
		gtk-ui-base.h		\
		gtk-vkb-button.c	\
		gtk-vkb-button.h	\
		pixmap-utility.c	\
		pixmap-utility.h

libfvkbd_gtk_@FVKBD_API_VERSION@_la_LIBADD = \
			@GTK_LIBS@ \
			$(top_builddir)/src/libfvkbd-@FVKBD_API_VERSION@.la

AM_CFLAGS =  @FVKBD_CFLAGS@ @GTK_CFLAGS@  -Wall -g3 -O0 -ggdb

CLEANFILES = *~

endif
