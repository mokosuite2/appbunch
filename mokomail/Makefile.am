INCLUDES = \
	$(GLIB_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(ELM_CFLAGS) \
	$(EVAS_CFLAGS) \
	$(EDJE_CFLAGS) \
	$(FSO_CFLAGS) \
	$(PHONEUTILS_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-Wall

bin_PROGRAMS = mokomail

mokomail_SOURCES = \
	main.c

mokomail_LDADD = \
	$(ELM_LIBS) \
	$(EVAS_LIBS) \
	$(EDJE_LIBS) \
	$(FSO_LIBS) \
	$(PHONEUTILS_LIBS) \
	-L../libmokosuite \
	-lmokosuite

mokomail_LDFLAGS = \
	`libetpan-config --libs`

MAINTAINERCLEANFILES = \
	Makefile.in
