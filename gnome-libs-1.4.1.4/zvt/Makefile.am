sbin_PROGRAMS    = gnome-pty-helper
lib_LTLIBRARIES  = libzvt.la
noinst_PROGRAMS  = zterm test-utmp

libzvtincludedir = $(includedir)/gnome-1.0/zvt

INCLUDES = -I.. -I$(srcdir)/.. $(SUPPORTINCS)		\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(includedir) $(GTK_CFLAGS)  			\
	$(GDK_IMLIB_CFLAGS)				\
	-DGNOMESBINDIR=\""$(sbindir)"\"

libzvt_la_SOURCES =		\
	gnome-login-support.h	\
	gnome-login-support.c	\
	lists.c			\
	subshell.h		\
	subshell.c		\
	subshell-includes.h	\
	update.c		\
	vt.c			\
	zvtterm.c

libzvt_la_LDFLAGS = -version-info 4:10:2 -rpath $(libdir)

libzvtinclude_HEADERS =		\
	lists.h			\
	vt.h			\
	vtx.h			\
	zvtterm.h

man_MANS = gnome-pty-helper.1

#gnome_pty_helper_SOURCES = 	\
#	gnome-pty.h		\
#	gnome-pty-helper.c	\
#	gnome-login-support.c	\
#	gnome-utmp.c		\
#	gnome-login-support.h

#gnome_pty_helper_LDADD =	\
#	$(UTIL_LIBS)

# We really dont want $(LIBS) linked in with this
gnome-pty-helper: gnome-pty-helper.o gnome-login-support.o gnome-utmp.o
	$(CC) -o $@ $^ $(UTIL_LIBS)

# Since we do gnome-pty-helper manually, include the relevant sources here
EXTRA_DIST = TODO BUGS README	\
	gnome-pty.h gnome-pty-helper.c gnome-utmp.c $(man_MANS)

zterm_SOURCES	= zterm.c

zterm_LDADD	= 			\
	libzvt.la			\
	$(UTIL_LIBS)			\
	$(GTK_LIBS) 			\
	$(GDK_IMLIB_LIBS)		\
	@LIBSUPPORT@ $(INTLLIBS)

test_utmp_SOURCES =			\
	gnome-utmp.c			\
	test-utmp.c

test_utmp_LDADD =			\
	$(UTIL_LIBS)

install-exec-local:
	-file=$(DESTDIR)$(sbindir)/gnome-pty-helper; \
	chown root $$file && chgrp root $$file && chmod u+s $$file
