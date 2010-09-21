INCLUDES = $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(ELM_CFLAGS) $(FSO_CFLAGS) $(ALSA_CFLAGS) $(PHONEUTILS_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall

bin_PROGRAMS = mokophone
dbus_sources = mokophone-service-glue.h

BUILT_SOURCES = $(dbus_sources)

mokophone_SOURCES = $(dbus_sources) \
	main.c \
	mokophone.c \
	mokophone.h \
	gsm.c \
	gsm.h \
	phonewin.c \
	phonewin.h \
	callsdb.c \
	callsdb.h \
	callwin.c \
	callwin.h \
	callblock.c \
	callblock.h \
	simauthwin.c \
	simauthwin.h \
	logview.c \
	logview.h \
	contactsview.c \
	contactsview.h \
	logentry.c \
	logentry.h \
	sound.c \
	sound.h

mokophone_LDADD = \
	$(FSO_LIBS) $(ALSA_LIBS) $(PHONEUTILS_LIBS) -L../libmokosuite -lmokosuite

MAINTAINERCLEANFILES = \
	Makefile.in

mokophone-service-glue.h: mokophone-dbus.xml
	dbus-binding-tool --mode=glib-server --prefix=moko_phone --output=$@ $<
