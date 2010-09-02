VALAC_ARGS = \
	--vapidir=$(top_srcdir)/vapi \
	--pkg dbus-glib-1 \
	--pkg panel \
	--save-temps \
	--basedir $(top_srcdir)/mokopanel

INCLUDES = $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(ELM_CFLAGS) $(FSO_CFLAGS) $(EGGDBUS_CFLAGS) $(X11_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -Wall \
	-DEGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE

bin_PROGRAMS = mokopanel

BUILT_SOURCES = .vala.stamp

.vala.stamp: $(mokopanel_VALASOURCES)
	$(VALAC) -C $(VALAC_ARGS) $^ -H notifications-service.h
	touch $@

mokopanel_VALASOURCES = \
        notifications-service.vala

mokopanel_SOURCES = $(dbus_sources) \
	$(mokopanel_VALASOURCES:.vala=.c) \
	notifications-service.h \
	main.c \
	panel.c \
	panel.h \
	notifications-win.c \
	notifications-win.h \
	clock.c \
	clock.h \
	battery.c \
	battery.h \
	gsm.c \
	gsm.h \
	idle.c \
	idle.h \
	shutdown.c \
	shutdown.h

mokopanel_LDADD = \
	$(FSO_LIBS) $(EGGDBUS_LIBS) $(X11_LIBS) -L../libmokosuite -lmokosuite

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	$(mokopanel_VALASOURCES:.vala=.c) \
	notifications-service.h

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(mokopanel_VALASOURCES:.vala=.c) \
	notifications-service.h

MAINTAINERCLEANFILES = \
	Makefile.in
