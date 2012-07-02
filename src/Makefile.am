## Process this file with automake to produce Makefile.in
NULL=

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(ROOKIE_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g \
	 $(NULL)

bin_PROGRAMS = rookie
lib_LTLIBRARIES = librookie.la

INST_H_FILES = \
	rookie-app.h		\
	g-downloadable.h	\
	g-downloadable-activatable.h	\
	g-download-list.h	\
	g-download-list-controller.h	\
	$(NULL)

headerdir = $(includedir)/rookie/$(Rookie_API_VERSION)
header_DATA = $(INST_H_FILES)

# Rookie main library (used for main player; separate to allow gtk-doc to be used)
librookie_la_SOURCES = \
	$(INST_H_FILES)		\
	rookie-app.c		\
	g-downloadable.c 	\
	g-download-list.c	\
	g-download-list-controller.c	\
	g-downloadable-activatable.c 	\
	rookie-i18n.h		\
	rookie-debug.c		\
	rookie-debug.h		\
	rookie-settings.c	\
	rookie-settings.h	\
	rookie-services.c	\
	rookie-services.h	\
	rookie-misc.c		\
	rookie-misc.h		\
	categories.c		\
	categories.h		\
	gio-download.c		\
	gio-download.h		\
	g-downloadable-backends.c	\
	g-downloadable-backends.h	\
	statusbar.c		\
	statusbar.h		\
	main-window.c		\
	main-window.h		\
	sidepane.c		\
	sidepane.h		\
	bottom-pane.c		\
	bottom-pane.h		\
	download-dialog.c	\
	download-dialog.h	\
	category-dialog.c	\
	category-dialog.h	\
	preferences-dialog.c	\
	preferences-dialog.h	\
	soup-download.c		\
	soup-download.h		\
	$(NULL)

librookie_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

librookie_la_CFLAGS = \
	$(AM_CFLAGS)

librookie_la_LDFLAGS = \
	-export-dynamic	\
	-no-undefined \
	$(AM_LDFLAGS)

librookie_la_LIBADD = \
	$(ROOKIE_LIBS)

rookie_SOURCES =		\
	main.c	\
	$(librookie_la_SOURCES) \
	$(NULL)


rookie_LDFLAGS = \
	-Wl,--export-dynamic

rookie_LDADD = $(ROOKIE_LIBS)

check-syntax:
	$(COMPILE) -o /dev/null -S ${CHK_SOURCES}

# Introspection
-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION

introspection_sources = $(INST_H_FILES) $(filter %.c,$(librookie_la_SOURCES))

# NOTE: This should change with $(Rookie_API_VERSION)
Rookie-1.0.gir: librookie.la
Rookie_1_0_gir_INCLUDES = GLib-2.0 GObject-2.0 Gtk-3.0
Rookie_1_0_gir_SCANNERFLAGS = --c-include=rookie.h --pkg-export=rookie --identifier-prefix="G" --symbol-prefix="g"
#Rookie_1_0_gir_CFLAGS =
Rookie_1_0_gir_NAMESPACE = Rookie
Rookie_1_0_gir_LIBS = librookie.la
Rookie_1_0_gir_FILES = $(introspection_sources)
Rookie_1_0_gir_EXPORT_PACKAGES = Rookie
INTROSPECTION_GIRS += Rookie-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)
endif

-include $(top_srcdir)/git.mk
