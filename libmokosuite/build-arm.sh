#!/bin/bash
# use this script to compile a shr/OE application.
# THIS SCRIPT WILL DELETE "build" directory, whatever is inside it
# what must be customized:
# 1) those env variables below (they are self-explainatory)
# 2) header include paths in function compile_file
# 3) any code-generation tool like dbus-binding-tool (if any)
# 4) call compile_file <source.c> build/<source.o> for every source file
# 5) libraries to be linked, can be found in the final gcc line
#
# if the script is called with "nostrip", it doesn't execute strip on the final executable


set -e

BASE=/root/buildhost/shr-unstable/tmp
STAGE=$BASE/sysroots/armv4t-oe-linux-gnueabi
STAGE_NATIVE=$BASE/sysroots/i686-linux
EXENAME=libmokosuite.so.0.0.0
#EXTRA_CFLAGS="-DQVGA"

rm -fR build
mkdir -p build

function compile_file() {

#gcc -DPACKAGE_NAME=\"mokosuite\" -DPACKAGE_TARNAME=\"mokosuite\" -DPACKAGE_VERSION=\"0.1\" -DPACKAGE_STRING=\"mokosuite\ 0.1\" -DPACKAGE_BUGREPORT=\"daniele.athome@gmail.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"mokosuite\" -DVERSION=\"0.1\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DGETTEXT_PACKAGE=\"mokosuite\" -DHAVE_LOCALE_H=1 -DHAVE_LC_MESSAGES=1 -DHAVE_BIND_TEXTDOMAIN_CODESET=1 -DHAVE_GETTEXT=1 -DHAVE_DCGETTEXT=1 -DENABLE_NLS=1 -I. -I. -I/opt/e17/include -I/opt/e17/include/elementary -I/opt/e17/include/efreet -I/opt/e17/include/eina-0 -I/opt/e17/include/eina-0/eina -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/lua5.1 -I/usr/include/fso-glib -I/usr/include/gee-1.0   -I../libmokosuite    -g -O2 -MT simauthwin.o -MD -MP -MF ".deps/simauthwin.Tpo" -c -o simauthwin.o simauthwin.c;

$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-gcc -march=armv4t -mtune=arm920t -mthumb-interwork -mthumb \
-DPACKAGE_NAME=\"mokosuite\" -DPACKAGE_TARNAME=\"mokosuite\" -DPACKAGE_VERSION=\"0.1\" -DPACKAGE_STRING=\"mokosuite\ 0.1\" \
-DPACKAGE_BUGREPORT=\"daniele.athome@gmail.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"mokosuite\" -DCONTACTSDB_SQLITE $3 \
-DGETTEXT_PACKAGE=\"mokosuite\" -DHAVE_LC_MESSAGES=1 -DHAVE_BIND_TEXTDOMAIN_CODESET=1 -DHAVE_GETTEXT=1 -DHAVE_DCGETTEXT=1 \
-DENABLE_NLS=1 -DVERSION=\"0.1\" -DLOCALEDIR=\"/usr/share/locale\" -DSYSCONFDIR=\"/etc\" -DDATADIR=\"/usr/share\" -I. -I.. -Ibuild -Wall $EXTRA_CFLAGS \
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
-I$STAGE/usr/include/phone-utils \
-fexpensive-optimizations -fomit-frame-pointer -frename-registers -g -Os -c -o $2 $1

}

dbus-binding-tool --mode=glib-client --prefix=moko_settings --output=build/settings-glue.h settings-dbus.xml
dbus-binding-tool --mode=glib-server --prefix=moko_settings_service --output=build/settings-service-glue.h settings-dbus.xml
dbus-binding-tool --mode=glib-client --prefix=moko_notifications --output=build/notifications-glue.h notifications.xml

cd build
eggdbus-binding-tool -x ../bluetooth-dbus.xml -n bluez -d org.bluez
cd ..

compile_file application.c build/application.o
compile_file cfg.c build/cfg.o
compile_file dbus.c build/dbus.o
compile_file gui.c build/gui.o
compile_file misc.c build/misc.o
compile_file mokoinwin.c build/mokoinwin.o
compile_file mokowin.c build/mokowin.o
compile_file settings.c build/settings.o
compile_file fso.c build/fso.o
compile_file notifications.c build/notifications.o
compile_file settings-service.c build/settings-service.o -DSETTINGSDB_CFG
compile_file bluetooth-client.c build/bluetooth-client.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file bluetooth-agent.c build/bluetooth-agent.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file build/bluezadapter.c build/bluezadapter.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file build/bluezbindings.c build/bluezbindings.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file build/bluezbindingsmarshal.c build/bluezbindingsmarshal.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file build/bluezmanager.c build/bluezmanager.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file build/bluezdevice.c build/bluezdevice.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file build/bluezagent.c build/bluezagent.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file build/bluezheadset.c build/bluezheadset.o -DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
compile_file contactsdb.c build/contactsdb.o

#-fexpensive-optimizations -fomit-frame-pointer -frename-registers -Os 
$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-gcc -g -march=armv4t -mtune=arm920t -mthumb-interwork -mthumb \
-fexpensive-optimizations -fomit-frame-pointer -frename-registers \
-o build/$EXENAME build/*.o \
-Wl,-soname -Wl,libmokosuite.so.0 -shared -Wl,-rpath-link,$STAGE/usr/lib -Wl,-O1 -Wl,--hash-style=gnu \
-L$STAGE/usr/lib \
-L$STAGE/lib \
-ldbus-glib-1 -ldbus-1 -lelementary -lgobject-2.0 -lrt -lglib-2.0 -lframeworkd-glib -leggdbus-1 -ldb -lsqlite3 -lphone-utils

cd build
ln -s libmokosuite.so.0.0.0 libmokosuite.so
cd ..

if [ "$1" != "nostrip" ]; then
$STAGE_NATIVE/usr/armv4t/bin/arm-oe-linux-gnueabi-strip build/$EXENAME
fi

echo "Compiled."
