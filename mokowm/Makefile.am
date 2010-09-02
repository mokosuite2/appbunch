INCLUDES = $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(ECORE_CFLAGS) $(EDJE_CFLAGS) $(FAKEKEY_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall

bin_PROGRAMS = mokowm

mokowm_SOURCES = \
	main.c \
	input.c \
	input.h \
	wm.c \
	wm.h \
	client.c \
	client.h \
	dock_client.c \
	dock_client.h \
	desktop_client.c \
	desktop_client.h \
	splash_client.c \
	splash_client.h \
	dialog_client.c \
	dialog_client.h \
	input_client.c \
	input_client.h

mokowm_LDADD = \
	$(FAKEKEY_LIBS) $(ECORE_LIBS) $(EDJE_LIBS) $(GLIB_LIBS)

MAINTAINERCLEANFILES = \
	Makefile.in
