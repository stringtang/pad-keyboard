#!/bin/sh
[ -e config.cache ] && rm -f config.cache

glib-gettextize -c

intltoolize --copy --force --automake

aclocal -I m4
libtoolize --install --copy
autoconf
autoheader
automake -a
./configure $@
exit

