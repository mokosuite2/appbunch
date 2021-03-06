DESCRIPTION = "Desktop environment and phone stack GUI for SHR"
HOMEPAGE = "https://gitorious.org/mokosuite2"
AUTHOR = "Daniele Ricci"
LICENSE = "GPLv3"
DEPENDS = "mokowm elementary libfreesmartphone-glib libphone-utils eggdbus glib-2.0 dbus-glib alsa-lib db sqlite3 vala-native edje-native"
SECTION = "misc/utils"

PV = "1.0.99+gitr${SRCPV}"
#SRCREV = "719a200354d9b7aa825de75d19169067fd4871b0"

SRC_URI = "git://gitorious.org/mokosuite2/appbunch.git;protocol=git"
S = "${WORKDIR}/git"

PARALLEL_MAKE = ""

CFLAGS += "-DOPENMOKO"
EXTRA_OECONF = " --enable-contactsdb-sqlite --enable-settings-config --with-edje-cc=${STAGING_BINDIR_NATIVE}/edje_cc"
FILES_${PN} += "${datadir}/mokosuite ${sysconfdir}/dbus-1 ${sysconfdir}/X11 ${sysconfdir}/mokosuite.conf {$datadir}/applications"
CONFFILES_${PN} = "${sysconfdir}/mokosuite.conf"

inherit pkgconfig autotools
