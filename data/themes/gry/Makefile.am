MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/mokosuite/themes
files_DATA = gry.edj

EXTRA_DIST = $(files_DATA) phone.edc panel.edc idle.edc messages.edc

clean-local:
	rm -f $(files_DATA)

gry.edj: theme.edc phone.edc panel.edc idle.edc messages.edc
	@edje_cc@ $< $@
