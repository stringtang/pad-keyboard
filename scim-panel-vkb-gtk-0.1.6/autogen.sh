#!/bin/sh
[ -e config.cache ] && rm -f config.cache

glib-gettextize -c
intltoolize --copy --force --automake

libtoolize --automake
aclocal -I m4
autoconf
autoheader
automake -a
./configure $@
exit

