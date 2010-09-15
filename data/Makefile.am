MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = themes

filesdir = $(datadir)/mokosuite
files_DATA = \
	wallpaper_mountain.jpg call-end.png call-hold.png call-start.png \
	log_call-in.png log_call-missed.png log_call-out.png message-dock.png \
	battery-unknown.png battery-0.png battery-10.png battery-20.png battery-40.png \
	battery-60.png battery-80.png battery-100.png \
	charging-10.png charging-20.png charging-40.png charging-60.png \
	charging-80.png charging-100.png gsm-null.png gsm-offline.png \
	gsm-0.png gsm-1.png gsm-2.png gsm-3.png gsm-4.png

dbusdir = $(sysconfdir)/dbus-1/system.d
dbus_DATA = mokosuite2.conf

configdir = $(sysconfdir)
config_DATA = mokosuite.conf

desktopdir = $(datadir)/applications
desktop_DATA = mokophone-phone.desktop mokosettings.desktop

bin_SCRIPTS = mokophone-activate.sh

EXTRA_DIST = $(files_DATA) $(dbus_DATA) \
	$(config_DATA) $(desktop_DATA)
