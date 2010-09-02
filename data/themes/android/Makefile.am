MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/mokosuite/themes
files_DATA = android.edj

EXTRA_DIST = $(files_DATA) phone.edc panel.edc home.edc idle.edc

clean-local:
	rm -f $(files_DATA)

android.edj: theme.edc phone.edc panel.edc home.edc idle.edc
	@edje_cc@ $< $@
