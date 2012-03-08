if FVKBD_BUILD_GTK_UI

desktop_in_files = fvkbd-gtk.desktop.in
applicationdir = $(datadir)/applications
dist_application_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/pixmaps
dist_icon_DATA = fvkbd-gtk.png

DATADIR = $(datadir)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/ui/gtk

bin_PROGRAMS = fvkbd-gtk

fvkbd_gtk_SOURCES = 			\
		fvkbd-ui-gtk.c

fvkbd_gtk_LDADD = @GTK_LIBS@ \
		$(top_builddir)/src/libfvkbd-@FVKBD_API_VERSION@.la \
		$(top_builddir)/ui/gtk/libfvkbd-gtk-@FVKBD_API_VERSION@.la

AM_CFLAGS =  @FVKBD_CFLAGS@ @GTK_CFLAGS@  -Wall -g3 -O0 -ggdb

EXTRA_DIST = fvkbd-gtk.desktop.in

CLEANFILES = *~ fvkbd-gtk.desktop

endif
