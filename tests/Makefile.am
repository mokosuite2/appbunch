INCLUDES = \
	$(GLIB_CFLAGS) \
	$(ELM_CFLAGS) \
	$(EVAS_CFLAGS) \
	$(ECORE_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-Wall

noinst_PROGRAMS = theme_buttons

theme_buttons_SOURCES = \
	theme_buttons.c

theme_buttons_LDADD = \
	$(GLIB_LIBS) \
	$(ELM_LIBS) \
	$(EVAS_LIBS) \
	$(ECORE_LIBS) \
	-L../libmokosuite -lmokosuite

MAINTAINERCLEANFILES = \
	Makefile.in
