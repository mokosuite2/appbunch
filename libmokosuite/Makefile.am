
INCLUDES = $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(ELM_CFLAGS) $(FSO_CFLAGS) $(EGGDBUS_CFLAGS) $(PHONEUTILS_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall \
	-DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE

if ENABLE_CONTACTSDB_SQLITE
INCLUDES += $(SQLITE_CFLAGS)
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmokosuite.pc

mokosuiteincludedir = $(includedir)/libmokosuite
mokosuiteinclude_HEADERS = \
	mokosuite.h \
	application.h \
	dbus.h \
	misc.h \
	gui.h \
	settings-service.h \
	settings.h \
	cfg.h \
	fso.h \
	notifications.h \
	bluetooth-client.h \
	bluetooth-agent.h \
	contactsdb.h

lib_LTLIBRARIES = libmokosuite.la

dbus_sources = \
	settings-service-glue.h \
	settings-glue.h \
	notifications-glue.h \
	bluezbindings.h

bt_sources = \
	bluezadapter.c \
	bluezadapter.h \
	bluezbindings.c \
	bluezbindings.h \
	bluezbindingsmarshal.c \
	bluezbindingsmarshal.h \
	bluezbindingstypes.h \
	bluezmanager.c \
	bluezmanager.h \
	bluezdevice.c \
	bluezdevice.h \
	bluezagent.c \
	bluezagent.h \
	bluezheadset.c \
	bluezheadset.h

BUILT_SOURCES = $(dbus_sources) $(bt_sources)

libmokosuite_la_SOURCES = $(dbus_sources) $(bt_sources) \
	application.c \
	application.h \
	dbus.c \
	dbus.h \
	misc.c \
	misc.h \
	gui.c \
	gui.h \
	settings-service.c \
	settings-service.h \
	settings.c \
	settings.h \
	cfg.c \
	cfg.h \
	fso.c \
	fso.h \
	bluetooth-client.c \
	bluetooth-client.h \
	bluetooth-agent.c \
	bluetooth-agent.h \
	notifications.c \
	notifications.h \
	mokowin.c \
	mokoinwin.c \
	mokolist.c \
	contactsdb.c \
	contactsdb.h

libmokosuite_la_LIBADD = \
	$(GLIB_LIBS) $(DBUS_LIBS) $(ELM_LIBS) $(FSO_LIBS) $(EGGDBUS_LIBS) $(PHONEUTILS_LIBS) -ldb

if ENABLE_CONTACTSDB_SQLITE
libmokosuite_la_LIBADD += $(SQLITE_LIBS)
endif

EXTRA_DIST = \
	libmokosuite.pc.in

DISTCLEANFILES = 

MAINTAINERCLEANFILES = \
	Makefile.in

settings-service-glue.h: settings-dbus.xml
	dbus-binding-tool --mode=glib-server --prefix=moko_settings_service --output=$@ $<

settings-glue.h: settings-dbus.xml
	dbus-binding-tool --mode=glib-client --prefix=moko_settings --output=$@ $<

notifications-glue.h: notifications.xml
	dbus-binding-tool --mode=glib-client --prefix=moko_notifications --output=$@ $<

bluezbindings.h: bluetooth-dbus.xml
	eggdbus-binding-tool -x $< -n bluez -d org.bluez

maintainer-clean-local:
	rm -f $(bt_sources) docbook* *.list
