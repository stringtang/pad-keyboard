INCLUDES =	-I$(top_srcdir) \
		-I$(top_srcdir)/src \
		-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
		-DSCIM_VERSION=\"@SCIM_VERSION@\" \
		-DSCIM_MODULEDIR=\"@SCIM_MODULEDIR@\" \
		-DSCIM_BINARY_VERSION=\"@SCIM_BINARY_VERSION@\" 

bin_PROGRAMS = scim-panel-vkb-gtk
dist_bin_SCRIPTS = hack-scim-global-config

bindir = @prefix@/lib/scim-1.0
scim_panel_vkb_gtk_SOURCES =	scim-panel-vkb-gtk.cpp \
				panel-keyboard.cpp \
				panel-keyboard.h \
				panel-utility.cpp \
				panel-utility.h \
				panel_private.h \
				panel-settings.h \
				panel-vkb-dbusobj.h \
				panel-vkb-dbusobj.cpp

DBUS_GLUE_FILES = panel-vkb-dbus-glue.h
EXTRA_DIST = $(DBUS_GLUE_FILES) panel-vkb-dbus.xml
BUILT_SOURCES = $(DBUS_GLUE_FILES)

$(DBUS_GLUE_FILES): panel-vkb-dbus.xml
	$(DBUS_BINDING_TOOL) $< --mode=glib-server --prefix=panel_vkb --output=$@



AM_CXXFLAGS = @SCIM_PANEL_VKB_GTK_CFLAGS@ -Wall -ggdb -DPKGDATADIR="$(pkgdatadir)" -DLOCALEDIR=\"$(localedir)\"
AM_LDFLAGS = @SCIM_PANEL_VKB_GTK_LIBS@ 

CLEANFILES = *~
DISTCLEANFILES = $(DBUS_GLUE_FILES)

