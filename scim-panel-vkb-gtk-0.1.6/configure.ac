AC_PREREQ(2.61)

# package version number (not shared library version)
# odd micro numbers indicate in-progress development
# even micro numbers indicate released versions
m4_define(scim_panel_vkb_gtk_version_major, 0)
m4_define(scim_panel_vkb_gtk_version_minor, 1)
m4_define(scim_panel_vkb_gtk_version_micro, 6)

m4_define([scim_panel_vkb_gtk_version],
          [scim_panel_vkb_gtk_version_major.scim_panel_vkb_gtk_version_minor.scim_panel_vkb_gtk_version_micro])
m4_define([scim_panel_vkb_gtk_api_version],
          [scim_panel_vkb_gtk_version_major.scim_panel_vkb_gtk_version_minor])

AC_INIT(scim-panel-vkb-gtk, scim_panel_vkb_gtk_version)
AM_INIT_AUTOMAKE(scim-panel-vkb-gtk, scim_panel_vkb_gtk_version)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

# Check for programs
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_INSTALL

# enable pkg-config
PKG_PROG_PKG_CONFIG

# all libraries listed are associated with "SCIM_PANEL_VKB_GTK" tag
PKG_CHECK_MODULES(SCIM_PANEL_VKB_GTK, glib-2.0
			gtk+-2.0 >= 2.14
			gthread-2.0
			scim >= 1.4.9
			scim-gtkutils >= 1.4.9
			dbus-glib-1)

AC_SUBST(SCIM_PANEL_VKB_GTK_CFLAGS)
AC_SUBST(SCIM_PANEL_VKB_GTK_LIBS)

AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool, no)
AC_SUBST(DBUS_BINDING_TOOL)

SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
AC_SUBST(SCIM_ICONDIR)
SCIM_VERSION=`$PKG_CONFIG --modversion scim`
AC_SUBST(SCIM_VERSION)
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
AC_SUBST(SCIM_MODULEDIR)
SCIM_BINARY_VERSION=`$PKG_CONFIG --variable=scim_binary_version scim`
AC_SUBST(SCIM_BINARY_VERSION)



###########################################################
##             Definiation of compile args.              ##
###########################################################

AC_ARG_ENABLE(debug,
              [  --enable-debug            Turn on debugging], ,
              enable_debug=no)

if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in libscim])
  CFLAGS="$CFLAGS -g"
  CXXFLAGS="$CXXFLAGS -g"
fi

###########################################################


# i18n
GETTEXT_PACKAGE="scim-panel-vkb-gtk"
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])
AC_SUBST(GETTEXT_PACKAGE)
AS_ALL_LINGUAS
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.40.4])

# Output files
AC_OUTPUT([Makefile
po/Makefile.in
src/Makefile
modules/Makefile
modules/SetupUI/Makefile
data/Makefile
data/icons/Makefile
])

AC_MSG_RESULT([
Build options:
  Version                  scim_panel_vkb_gtk_version
  Install prefix           $prefix
  Enable debug             $enable_debug
])
