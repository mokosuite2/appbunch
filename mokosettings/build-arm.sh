#!/bin/bash

set -e

BASE=/root/buildhost/shr-unstable/tmp
STAGE=$BASE/sysroots/armv4t-oe-linux-gnueabi
STAGE_NATIVE=$BASE/sysroots/i686-linux
EXENAME=mokosettings

rm -fR build
mkdir -p build

function compile_file() {

#gcc -DPACKAGE_NAME=\"mokosuite\" -DPACKAGE_TARNAME=\"mokosuite\" -DPACKAGE_VERSION=\"0.1\" -DPACKAGE_STRING=\"mokosuite\ 0.1\" -DPACKAGE_BUGREPORT=\"daniele.athome@gmail.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"mokosuite\" -DVERSION=\"0.1\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DGETTEXT_PACKAGE=\"mokosuite\" -DHAVE_LOCALE_H=1 -DHAVE_LC_MESSAGES=1 -DHAVE_BIND_TEXTDOMAIN_CODESET=1 -DHAVE_GETTEXT=1 -DHAVE_DCGETTEXT=1 -DENABLE_NLS=1 -I. -I. -I/opt/e17/include -I/opt/e17/include/elementary -I/opt/e17/include/efreet -I/opt/e17/include/eina-0 -I/opt/e17/include/eina-0/eina -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/lua5.1 -I/usr/include/fso-glib -I/usr/include/gee-1.0   -I../libmokosuite    -g -O2 -MT simauthwin.o -MD -MP -MF ".deps/simauthwin.Tpo" -c -o simauthwin.o simauthwin.c;

$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-gcc -march=armv4t -mtune=arm920t -mthumb-interwork -mthumb \
-DPACKAGE_NAME=\"mokosuite\" -DPACKAGE_TARNAME=\"mokosuite\" -DPACKAGE_VERSION=\"0.1\" -DPACKAGE_STRING=\"mokosuite\ 0.1\" \
-DPACKAGE_BUGREPORT=\"daniele.athome@gmail.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"mokosuite\" -DOPENMOKO $3 $EXTRA_CFLAGS \
-DGETTEXT_PACKAGE=\"mokosuite\" -DHAVE_LC_MESSAGES=1 -DHAVE_BIND_TEXTDOMAIN_CODESET=1 -DHAVE_GETTEXT=1 -DHAVE_DCGETTEXT=1 \
-DENABLE_NLS=1 -DVERSION=\"0.1\" -DLOCALEDIR=\"/usr/share/locale\" -DSYSCONFDIR=\"/etc\" -DDATADIR=\"/usr/share\" \
-I. -I.. -Ibuild -I../libmokosuite/build -Wall \
-I$STAGE/usr/include \
-I$STAGE/usr/include/elementary-0 \
-I$STAGE/usr/include/e_dbus-1 \
-I$STAGE/usr/include/efreet-1 \
-I$STAGE/usr/include/eet-1 \
-I$STAGE/usr/include/edje-1 \
-I$STAGE/usr/include/evas-1 \
-I$STAGE/usr/include/ecore-1 \
-I$STAGE/usr/include/eina-1 \
-I$STAGE/usr/include/eina-1/eina \
-I$STAGE/usr/include/dbus-1.0 \
-I$STAGE/usr/lib/dbus-1.0/include \
-I$STAGE/usr/include/glib-2.0 \
-I$STAGE/usr/lib/glib-2.0/include \
-I$STAGE/usr/include/freetype2 \
-I$STAGE/usr/include/lua5.1 \
-I$STAGE/usr/include/frameworkd-glib \
-I$STAGE/usr/include/eggdbus-1 \
-fexpensive-optimizations -fomit-frame-pointer -frename-registers -g -Os -c -o $2 $1

}

compile_file main.c build/main.o
compile_file menu-main.c build/menu-main.o
compile_file menu-common.c build/menu-common.o
compile_file menu-network.c build/menu-network.o
compile_file menu-bluetooth.c build/menu-bluetooth.o
compile_file menu-display.c build/menu-display.o
compile_file menu-sounds.c build/menu-sounds.o

$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-gcc -g -march=armv4t -mtune=arm920t -mthumb-interwork -mthumb \
-fexpensive-optimizations -fomit-frame-pointer -frename-registers -Os -o build/$EXENAME build/*.o \
-Wl,-rpath-link,$STAGE/usr/lib -Wl,-O1 -Wl,--hash-style=gnu \
-L$STAGE/usr/lib \
-L$STAGE/lib \
-L../libmokosuite/build \
-lelementary -lgobject-2.0 -lrt -lglib-2.0 -lmokosuite

if [ "$1" != "nostrip" ]; then
$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-strip build/$EXENAME
fi

echo "Compiled."
