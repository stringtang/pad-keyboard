CLEANFILES		= *.bak

INCLUDES		= -I$(top_builddir) \
				-I$(top_srcdir)/src \
				-I$(top_srcdir)/modules/SetupUI \
				-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
				-DSCIM_VERSION=\"@SCIM_VERSION@\" \
				-DSCIM_MODULEDIR=\"@SCIM_MODULEDIR@\" \
				-DSCIM_BINARY_VERSION=\"@SCIM_BINARY_VERSION@\"

CONFIG_PANEL_VKB_SETUP_MODULE= panel-vkb-setup.la
CONFIG_PANEL_SETUP_MODULE= aaa-panel-setup.la

# Rules for setup module
setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(CONFIG_PANEL_SETUP_MODULE) \
						   $(CONFIG_PANEL_VKB_SETUP_MODULE)

AM_CXXFLAGS = @SCIM_PANEL_VKB_GTK_CFLAGS@ -Wall -Werror -DLOCALEDIR=\"$(localedir)\"

aaa_panel_setup_la_SOURCES = scim_panel_setup.cpp
aaa_panel_setup_la_CFLAGS  = @SCIM_PANEL_VKB_GTK_CFLAGS@
aaa_panel_setup_la_LDFLAGS = -avoid-version \
							 -rpath $(setupmoduledir) \
							 -module \
							 @SCIM_PANEL_VKB_GTK_LIBS@

panel_vkb_setup_la_SOURCES     = scim_panel_vkb_setup.cpp
panel_vkb_setup_la_CFLAGS      = @SCIM_PANEL_VKB_GTK_CFLAGS@
panel_vkb_setup_la_LDFLAGS     = -avoid-version \
								 -rpath $(setupmoduledir) \
								 -module \
							 @SCIM_PANEL_VKB_GTK_LIBS@
