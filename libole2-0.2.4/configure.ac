dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.13)
AC_INIT(libole2/ms-ole.c)

dnl Set release number
dnl This is derived from "Versioning" chapter of info libtool documentation.
PACKAGE=libole2
dnl     4a) Increment when removing or changing interfaces.
LIBOLE2_MAJOR_VERSION=0
dnl     4a) 5) Increment when adding interfaces.
dnl     6) Set to zero when removing or changing interfaces.
LIBOLE2_MINOR_VERSION=2
dnl     3) Increment when interfaces not changed at all,
dnl               only bug fixes or internal changes made.
dnl     4b) Set to zero when adding, removing or changing interfaces.
LIBOLE2_MICRO_VERSION=4
dnl
dnl     Set this too (I don't know how to do it automatically =) ):
MAJOR_VERSION_PLUS_MINOR_VERSION=2
dnl
VERSION=$LIBOLE2_MAJOR_VERSION.$LIBOLE2_MINOR_VERSION.$LIBOLE2_MICRO_VERSION
dnl Version info for libraries = CURRENT:REVISION:AGE
VERSION_INFO=$MAJOR_VERSION_PLUS_MINOR_VERSION:$LIBOLE2_MICRO_VERSION:$LIBOLE2_MINOR_VERSION
AC_SUBST(VERSION_INFO)
AC_SUBST(LIBOLE2_MAJOR_VERSION)
AC_SUBST(LIBOLE2_MINOR_VERSION)
AC_SUBST(LIBOLE2_MICRO_VERSION)
dnl AC_DEFINE_UNQUOTED(VERSION_INFO,"${VERSION_INFO}",Define rev version)

dnl Start automake
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

dnl Checks for libraries.
AM_PATH_GLIB(1.2.0,,,)

dnl Checks for header files.
dnl AC_CHECK_HEADERS(fcntl.h malloc.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_C_CONST
dnl AC_TYPE_OFF_T
dnl AC_TYPE_SIZE_T
dnl AC_TYPE_MODE_T

dnl Checks for library functions.
dnl AC_HEADER_STDC
AC_FUNC_MMAP


# gtk-doc stuff
# borrowed from gdk-pixbuf

AC_CHECK_PROG(GTKDOC, gtkdoc-mkdb, true, false)
AM_CONDITIONAL(HAVE_GTK_DOC, $GTKDOC)
AC_SUBST(HAVE_GTK_DOC)

dnl Let people disable the gtk-doc stuff.
AC_ARG_ENABLE(gtk-doc, [  --enable-gtk-doc        Use gtk-doc to build documentation [default=auto]], enable_gtk_doc="$enableval", enable_gtk_doc=auto)

if test x$enable_gtk_doc = xauto ; then
  if test x$GTKDOC = xtrue ; then
    enable_gtk_doc=yes
  else
    enable_gtk_doc=no
  fi
fi

dnl NOTE: We need to use a separate automake conditional for this
dnl       to make this work with the tarballs.
AM_CONDITIONAL(ENABLE_GTK_DOC, test x$enable_gtk_doc = xyes)

LIBOLE2_DIR=`(cd $srcdir; pwd)`
AC_SUBST(LIBOLE2_DIR)

AM_CONDITIONAL(LIBOLE2_PUBLIC_LIBRARY, true)
AC_OUTPUT(Makefile libole2/Makefile libole2-config libole2/libole2.h \
	doc/Makefile test/Makefile libole2Conf.sh libole2/version.c  \
	libole2.spec)

AC_MSG_RESULT([Configured $PACKAGE $VERSION])
