AC_INIT([mokosuite], [1.0], [daniele.athome@gmail.com], [mokosuite])
AC_CONFIG_SRCDIR(Makefile.am)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")
AC_PROG_CC

AC_PATH_PROG(VALAC, valac, valac)
AC_SUBST(VALAC)

IT_PROG_INTLTOOL([0.35.0])

if test "x$localedir" == x
then
    localedir='${datadir}/locale'
    AC_SUBST(localedir)
    AM_GLIB_DEFINE_LOCALEDIR([localedir])
fi

AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE="mokosuite"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], [Name of the gettext message domain])

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

ELM_REQUIRED=0.0.0
EDJE_REQUIRED=0.0.0
ECORE_REQUIRED=0.0.0
EDBUS_REQUIRED=0.5.0
EFREET_REQUIRED=0.5.0
GLIB_REQUIRED=2.18.0
DBUS_REQUIRED=1.1.1
DBUS_GLIB_REQUIRED=0.74
SQLITE3_REQUIRED=3.0
EGGDBUS_REQUIRED=0.5
PHONEUTILS_REQUIRED=0.1
FSO_REQUIRED=0.2

PKG_CHECK_MODULES(GLIB,
  glib-2.0 >= $GLIB_REQUIRED
  gobject-2.0 >= $GLIB_REQUIRED
)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(DBUS,
  dbus-1 >= $DBUS_REQUIRED
  dbus-glib-1 >= $DBUS_GLIB_REQUIRED
)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

PKG_CHECK_MODULES(ELM,
  elementary >= $ELM_REQUIRED
)
AC_SUBST(ELM_CFLAGS)
AC_SUBST(ELM_LIBS)

PKG_CHECK_MODULES(EDJE,
  edje >= $EDJE_REQUIRED
)
AC_SUBST(EDJE_CFLAGS)
AC_SUBST(EDJE_LIBS)

PKG_CHECK_MODULES(ECORE,
  ecore >= $ECORE_REQUIRED
  ecore-x >= $ECORE_REQUIRED
  ecore-evas >= $ECORE_REQUIRED
  ecore-input >= $ECORE_REQUIRED
  eet >= $ECORE_REQUIRED
)
AC_SUBST(ECORE_CFLAGS)
AC_SUBST(ECORE_LIBS)

PKG_CHECK_MODULES(EFREET, efreet >= $EFREET_REQUIRED)
AC_SUBST(EFREET_CFLAGS)
AC_SUBST(EFREET_LIBS)

PKG_CHECK_MODULES(FSO, libfreesmartphone-glib >= $FSO_REQUIRED)
AC_SUBST(FSO_CFLAGS)
AC_SUBST(FSO_LIBS)

PKG_CHECK_MODULES(ALSA, alsa)
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)

PKG_CHECK_MODULES(SQLITE, sqlite3 >= $SQLITE3_REQUIRED)
AC_SUBST(SQLITE_CFLAGS)
AC_SUBST(SQLITE_LIBS)

PKG_CHECK_MODULES(EGGDBUS, eggdbus-1 >= $EGGDBUS_REQUIRED)
AC_SUBST(EGGDBUS_CFLAGS)
AC_SUBST(EGGDBUS_LIBS)

PKG_CHECK_MODULES(X11, x11)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

PKG_CHECK_MODULES(FAKEKEY, libfakekey)
AC_SUBST(FAKEKEY_CFLAGS)
AC_SUBST(FAKEKEY_LIBS)

PKG_CHECK_MODULES(PHONEUTILS, libphone-utils)
AC_SUBST(PHONEUTILS_CFLAGS)
AC_SUBST(PHONEUTILS_LIBS)

AC_ARG_WITH(edje-cc,
[  --with-edje-cc=PATH	specify a specific path to edje_cc],
[
  v=$withval;
  edje_cc=$v
  echo "  Enlightenment edje_cc explicitly set to "$edje_cc;
],[
  edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc
])
AC_SUBST(edje_cc)

AC_ARG_ENABLE(qvga,
    AC_HELP_STRING([--enable-qvga], [Enable compiling for QVGA (default: disabled)]),
    enable_qvga=$enableval,
    enable_qvga=no)

if test "x$enable_qvga" != "xno"; then
    SCALE_FACTOR=1.0
    AC_DEFINE(QVGA, 1, [Define to enable QVGA build])
else
    SCALE_FACTOR=2.0
fi

AM_CONDITIONAL(ENABLE_QVGA, test "x$enable_qvga" = "xyes")
AC_SUBST(SCALE_FACTOR)

AC_ARG_ENABLE(settings_config,
    AC_HELP_STRING([--enable-settings-config], [Enable mokosuite.conf usage as settings storage (default: BerkeleyDB storage)]),
    enable_settings_config=$enableval,
    enable_settings_config=no)

if test "x$enable_settings_config" != "xno"; then
    AC_DEFINE(SETTINGSDB_CFG, 1, [Define to enable mokosuite.conf usage as settings storage])
fi

AM_CONDITIONAL(ENABLE_SETTINGS_CONFIG, test "x$enable_settings_config" = "xyes")

AC_ARG_ENABLE(callsdb_sqlite,
    AC_HELP_STRING([--enable-callsdb-sqlite], [Enable SQLite usage for calls database (default: disabled)]),
    enable_callsdb_sqlite=$enableval,
    enable_callsdb_sqlite=no)

if test "x$enable_callsdb_sqlite" != "xno"; then
    AC_DEFINE(CALLSDB_SQLITE, 1, [Define to enable SQLite usage for calls database])
fi

AM_CONDITIONAL(ENABLE_CALLSDB_SQLITE, test "x$enable_callsdb_sqlite" = "xyes")


AC_ARG_ENABLE(contactsdb_sqlite,
    AC_HELP_STRING([--enable-contactsdb-sqlite], [Enable SQLite usage for contacts database (default: disabled)]),
    enable_contactsdb_sqlite=$enableval,
    enable_contactsdb_sqlite=no)

if test "x$enable_contactsdb_sqlite" != "xno"; then
    AC_DEFINE(CONTACTSDB_SQLITE, 1, [Define to enable SQLite usage for contacts database])
fi

AM_CONDITIONAL(ENABLE_CONTACTSDB_SQLITE, test "x$enable_contactsdb_sqlite" = "xyes")


AC_ARG_ENABLE(threads,
    AC_HELP_STRING([--enable-threads], [Enable threads usage for various tasks (default: disabled)]),
    enable_threads=$enableval,
    enable_threads=no)

if test "x$enable_threads" != "xno"; then
    AC_DEFINE(USE_THREADS, 1, [Define to use threads for various])
fi

AM_CONDITIONAL(ENABLE_THREADS, test "x$enable_threads" = "xyes")


AC_OUTPUT([
  Makefile
  libmokosuite/libmokosuite.pc
  libmokosuite/Makefile
  data/mokosession
  data/Makefile
  data/themes/Makefile
  data/themes/gry/Makefile
  data/themes/gry/theme.edc
  data/themes/android/Makefile
  po/Makefile.in
  mokophone/Makefile
  mokomessages/Makefile
  mokomail/Makefile
  mokopanel/Makefile
  mokohome/Makefile
  mokosettings/Makefile
  mokowm/Makefile
  tests/Makefile])
