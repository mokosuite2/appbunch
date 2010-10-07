INCLUDES = $(GLIB_CFLAGS) $(ELM_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall

noinst_PROGRAMS = theme_buttons

theme_buttons_SOURCES = \
	theme_buttons.c

theme_buttons_LDADD = \
	-L../libmokosuite -lmokosuite

MAINTAINERCLEANFILES = \
	Makefile.in
