## Process this file with automake to produce Makefile.in
## Created by Anjuta
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = src data po

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	autogen.sh

check-syntax:
	$(COMPILE) -o /dev/null -S ${CHK_SOURCES}

DISTCHECK_CONFIGURE_FLAGS = --enable-introspection
-include $(top_srcdir)/git.mk
