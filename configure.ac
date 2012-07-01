dnl Process this file with autoconf to produce a configure script.

m4_define(rookie_major_version,  0)
m4_define(rookie_minor_version,  2)
m4_define(rookie_micro_version,  6)
m4_define(rookie_version, [rookie_major_version.rookie_minor_version.rookie_micro_version])

AC_INIT(rookie, [rookie_version],[http://launchpad.net/rookie],[rookie])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE(rookie, rookie_version)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

GLIB_GSETTINGS
dnl ================================================================
dnl Basic checks
dnl ================================================================

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

AC_CHECK_FUNCS(fsync)

dnl ###########################################################################
dnl Dependencies
dnl ###########################################################################

PKG_CHECK_MODULES(ROOKIE, [
    gtk+-3.0
    libsoup-2.4
    libnotify
])

dnl ###########################################################################
dnl Internationalization
dnl ###########################################################################
GETTEXT_PACKAGE=rookie
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])

AM_PROG_LIBTOOL

dnl ###########################################################################
dnl Files to generate
dnl ###########################################################################

AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
data/Makefile
])
