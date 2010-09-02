INCLUDES = $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(ELM_CFLAGS) $(FSO_CFLAGS) $(PHONEUTILS_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall

bin_PROGRAMS = mokomessages

mokomessages_SOURCES = \
	main.c \
	threadwin.c \
	threadwin.h \
	msglist.c \
	msglist.h

mokomessages_LDADD = \
	$(FSO_LIBS) $(PHONEUTILS_LIBS) -L../libmokosuite -lmokosuite

MAINTAINERCLEANFILES = \
	Makefile.in
