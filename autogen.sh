#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

autoreconf -v --install || exit 1
intltoolize -c --automake --force || exit 1
cd $ORIGDIR || exit $?

$srcdir/configure --enable-contactsdb-sqlite --enable-settings-config "$@"
