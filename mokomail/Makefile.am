INCLUDES = $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(ELM_CFLAGS) $(FSO_CFLAGS) $(PHONEUTILS_CFLAGS) `libetpan-config --cflags` \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall

bin_PROGRAMS = mokomail

mokomail_SOURCES = \
	main.c

mokomail_LDADD = \
	$(FSO_LIBS) $(PHONEUTILS_LIBS) -L../libmokosuite -lmokosuite

mokomail_LDFLAGS = \
	`libetpan-config --libs`

MAINTAINERCLEANFILES = \
	Makefile.in
