#!/bin/bash

set -e

BASE=/root/buildhost/shr-unstable/tmp
STAGE=$BASE/sysroots/armv4t-oe-linux-gnueabi
STAGE_NATIVE=$BASE/sysroots/i686-linux
VAPI_DIR=$STAGE_NATIVE/usr/share/vala/vapi
EXENAME=mokopanel

rm -fR build
mkdir -p build

function compile_file() {

#gcc -DPACKAGE_NAME=\"mokosuite\" -DPACKAGE_TARNAME=\"mokosuite\" -DPACKAGE_VERSION=\"0.1\" -DPACKAGE_STRING=\"mokosuite\ 0.1\" -DPACKAGE_BUGREPORT=\"daniele.athome@gmail.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"mokosuite\" -DVERSION=\"0.1\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DGETTEXT_PACKAGE=\"mokosuite\" -DHAVE_LOCALE_H=1 -DHAVE_LC_MESSAGES=1 -DHAVE_BIND_TEXTDOMAIN_CODESET=1 -DHAVE_GETTEXT=1 -DHAVE_DCGETTEXT=1 -DENABLE_NLS=1 -I. -I. -I/opt/e17/include -I/opt/e17/include/elementary -I/opt/e17/include/efreet -I/opt/e17/include/eina-0 -I/opt/e17/include/eina-0/eina -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/lua5.1 -I/usr/include/fso-glib -I/usr/include/gee-1.0   -I../libmokosuite    -g -O2 -MT simauthwin.o -MD -MP -MF ".deps/simauthwin.Tpo" -c -o simauthwin.o simauthwin.c;

$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-gcc -march=armv4t -mtune=arm920t -mthumb-interwork -mthumb $EXTRA_CFLAGS \
-DPACKAGE_NAME=\"mokosuite\" -DPACKAGE_TARNAME=\"mokosuite\" -DPACKAGE_VERSION=\"0.1\" -DPACKAGE_STRING=\"mokosuite\ 0.1\" \
-DPACKAGE_BUGREPORT=\"daniele.athome@gmail.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"mokosuite\" -DOPENMOKO \
-DGETTEXT_PACKAGE=\"mokosuite\" -DHAVE_LC_MESSAGES=1 -DHAVE_BIND_TEXTDOMAIN_CODESET=1 -DHAVE_GETTEXT=1 -DHAVE_DCGETTEXT=1 \
-DENABLE_NLS=1 -DVERSION=\"0.1\" -DLOCALEDIR=\"/usr/share/locale\" -DSYSCONFDIR=\"/etc\" -DDATADIR=\"/usr/share\" -I. -I.. -Ibuild -Wall \
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
-fexpensive-optimizations -fomit-frame-pointer -frename-registers -g -Os -c -o $2 $1

}

$STAGE_NATIVE/usr/bin/valac -C  --vapidir="$VAPI_DIR" --vapidir=../vapi --pkg dbus-glib-1 --pkg panel --save-temps --basedir .. notifications-service.vala -H notifications-service.h

compile_file main.c build/main.o
compile_file panel.c build/panel.o
compile_file clock.c build/clock.o
compile_file battery.c build/battery.o
compile_file gsm.c build/gsm.o
compile_file idle.c build/idle.o
compile_file shutdown.c build/shutdown.o
compile_file notifications-win.c build/notifications-win.o
compile_file notifications-service.c build/notifications-service.o

# vala generated sources
rm -f notifications-service.c notifications-service.h

$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-gcc -g -march=armv4t -mtune=arm920t -mthumb-interwork -mthumb \
-fexpensive-optimizations -fomit-frame-pointer -frename-registers -Os -o build/$EXENAME build/*.o \
-Wl,-rpath-link,$STAGE/usr/lib -Wl,-O1 -Wl,--hash-style=gnu \
-L$STAGE/usr/lib \
-L$STAGE/lib \
-L../libmokosuite/build \
-lelementary -lgobject-2.0 -lrt -lglib-2.0 -lmokosuite -lX11

if [ "$1" != "nostrip" ]; then
$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-strip build/$EXENAME
fi

echo "Compiled."
