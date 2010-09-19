INCLUDES = $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(ELM_CFLAGS) $(FAKEKEY_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall

bin_PROGRAMS = mokohome

mokohome_SOURCES = \
	main.c \
	desktop.c \
	desktop.h \
	launchers.c \
	launchers.h \
	widgets.c \
	widgets.h

mokohome_LDADD = \
	-L../libmokosuite -lmokosuite $(FAKEKEY_LIBS)

MAINTAINERCLEANFILES = \
	Makefile.in
