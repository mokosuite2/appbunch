INCLUDES = \
	$(GLIB_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(ELM_CFLAGS) \
	$(EVAS_CFLAGS) \
	$(FSO_CFLAGS) \
	$(EGGDBUS_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-Wall

bin_PROGRAMS = mokosettings

mokosettings_SOURCES = \
	main.c \
	menu-common.c \
	menu-common.h \
	menu-main.c \
	menu-main.h \
	menu-network.c \
	menu-network.h \
	menu-bluetooth.c \
	menu-bluetooth.h \
	menu-display.c \
	menu-display.h \
	menu-sounds.c \
	menu-sounds.h

mokosettings_LDADD = \
	$(ELM_LIBS) \
	$(EVAS_LIBS) \
	$(FSO_LIBS) \
	$(EGGDBUS_LIBS) \
	-L../libmokosuite -lmokosuite

MAINTAINERCLEANFILES = \
	Makefile.in
