AC_PREREQ(2.61)

# package version number (not shared library version)
# odd micro numbers indicate in-progress development
# even micro numbers indicate released versions
m4_define(fvkbd_version_major, 0)
m4_define(fvkbd_version_minor, 2)
m4_define(fvkbd_version_micro, 3)

m4_define([fvkbd_version],
          [fvkbd_version_major.fvkbd_version_minor.fvkbd_version_micro])
m4_define([fvkbd_api_version],
          [fvkbd_version_major.fvkbd_version_minor])

AC_INIT(fvkbd, fvkbd_version)
AM_INIT_AUTOMAKE(fvkbd, fvkbd_version)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

FVKBD_API_VERSION=fvkbd_api_version
AC_SUBST(FVKBD_API_VERSION)

# Check for programs
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_INSTALL

# enable pkg-config
PKG_PROG_PKG_CONFIG

# Check for function availability
# AC_CHECK_FUNCS([memset])

# all libraries listed are associated with "FVKBD" tag
PKG_CHECK_MODULES(FVKBD, glib-2.0
			gobject-2.0
			libxml-2.0 >= 2.6)

AC_SUBST(FVKBD_CFLAGS)
AC_SUBST(FVKBD_LIBS)

# Check for LIB FAKEKEY
PKG_CHECK_MODULES(FAKEKEY, libfakekey)
AC_SUBST(FAKEKEY_CFLAGS)
AC_SUBST(FAKEKEY_LIBS)

# Check whether we have GTK and related library to build gtk ui
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.20.1
			unique-1.0,
			[FVKBD_HAS_GTK=yes],
			[FVKBD_HAS_GTK=no])

AC_ARG_ENABLE(gtk-ui,
		[AS_HELP_STRING([--enable-gtk-ui], [Build gtk ui, default=yes])],
		[enable_gtk_ui=$enableval],
		[enable_gtk_ui=yes])

if test x"$enable_gtk_ui" = x"yes"; then
  if test x"$FVKBD_HAS_GTK" != x"yes"; then
    AC_MSG_ERROR([can not find gtk+-2.0 >= 2.20.1])
  else
    FVKBD_BUILD_GTK_UI=1;
  fi
  FVKBD_BUILD_GTK_UI=0;
fi

AM_CONDITIONAL(FVKBD_BUILD_GTK_UI,
		[test x"$enable_gtk_ui" = x"yes"])

# Add --enable-debug arg
AC_ARG_ENABLE(debug,
              [AS_HELP_STRING([--enable-debug], [Turn on debugging])], ,
              enable_debug=no)
if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in fvkbd])
fi

# Add --enable-timer arg
AC_ARG_ENABLE(timer,
              [AS_HELP_STRING([--enable-timer], [Turn on timer for time debugging])], ,
              enable_timer=no)
if test "$enable_timer" = "yes"; then
  AC_DEFINE(ENABLE_TIMER,1,[Define this to enable the timer debug facility in fvkbd])
fi


# i18n
GETTEXT_PACKAGE="fvkbd"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])
AS_ALL_LINGUAS
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.40.4])

# Output files
AC_OUTPUT([Makefile
src/Makefile
po/Makefile.in
ui/Makefile
ui/gtk/Makefile
app/Makefile
app/gtk/Makefile
layout/Makefile
scripts/Makefile
data/Makefile
data/images/Makefile
examples/Makefile
examples/images/Makefile
examples/layout/Makefile
])

echo "
	fvkbd $VERSION
	=====================

	prefix:		${prefix}
	datarootdir:	${datarootdir}

	build gtk ui:	$enable_gtk_ui

	enable_debug:	$enable_debug
	enable_timer:	$enable_timer

	To build the project, run \"make\"
"

