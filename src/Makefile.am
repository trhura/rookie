## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(ROOKIE_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = rookie

rookie_SOURCES =		\
	main.c			\
	rookie-i18n.h		\
	rookie-app.h		\
	rookie-app.c		\
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
	g-downloadable.c 	\
	g-downloadable.h	\
	gio-download.c		\
	gio-download.h		\
	g-download-list.c	\
	g-download-list.h	\
	g-download-list-controller.c	\
	g-download-list-controller.h	\
	g-downloadable-backends.c	\
	g-downloadable-backends.h	\
	main-window.c		\
	main-window.h		\
	sidepane.c		\
	sidepane.h		\
	bottom-pane.c		\
	bottom-pane.h		\
	dropzone.c		\
	dropzone.h		\
	download-dialog.c	\
	download-dialog.h	\
	category-dialog.c	\
	category-dialog.h	\
	preferences-dialog.c	\
	preferences-dialog.h	\
	soup-download.c		\
	soup-download.h

rookie_LDFLAGS = \
	-Wl,--export-dynamic

rookie_LDADD = $(ROOKIE_LIBS)

check-syntax:
	$(COMPILE) -o /dev/null -S ${CHK_SOURCES}
