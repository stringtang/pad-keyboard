DATADIR = $(datadir)
PKGDATADIR = $(datadir)/fvkbd

lib_LTLIBRARIES = libfvkbd-@FVKBD_API_VERSION@.la

fvkbd_source_c = 			\
		fvkbd.c 		\
		fvkbd-base.c		\
		fvkbd-key.c		\
		fvkbd-keyboard.c	\
		fvkbd-panel.c		\
		layout-utility.c 	\
		misc-utility.c		\
		parser-utility-libxml.c

fvkbd_source_h = 			\
		fvkbd.h			\
		fvkbd-base.h		\
		fvkbd-key.h		\
		fvkbd-keyboard.h	\
		fvkbd-panel.h		\
		layout-utility.h	\
		misc-utility.h		\
		parser-utility-libxml.h

libfvkbd_@FVKBD_API_VERSION@_la_SOURCES = \
		$(fvkbd_source_c)	\
		$(fvkbd_source_h)

libfvkbd_@FVKBD_API_VERSION@_la_LIBADD = @FVKBD_LIBS@ @FAKEKEY_LIBS@

AM_CFLAGS = @FVKBD_CFLAGS@ @FAKEKEY_CFLAGS@ -Wall -g3 -O0 -ggdb -DPKGDATADIR="\"$(PKGDATADIR)\""

CLEANFILES = *~

