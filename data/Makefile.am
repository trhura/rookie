uidir = $(datadir)/rookie
ui_DATA = \
    AppUI.ui    \
    AddDownloadDialog.ui    \
    CategoryDialog.ui\
    categories.ini

gsettings_SCHEMAS = net.launchpad.rookie.gschema.xml
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

Utilitiesdir = $(datadir)/applications
Utilities_in_files = rookie.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

#man1_MANS = gcalctool.1

EXTRA_DIST = $(ui_DATA) net.lauchpad.rookie.gschema.xml rookie.desktop.in #$(man1_MANS)

DISTCLEANFILES = \
	rookie.desktop
	
MAINTAINERCLEANFILES = \
	Makefile.in
