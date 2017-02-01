## Process this file with automake to produce Makefile.in.

# Documentation
SUBDIRS = C de da es it no

INCLUDES = 						\
	-I$(includedir)					\
	-I$(top_srcdir)/intl				\
	-I$(top_builddir)/intl				\
	$(GNOME_TERMINAL_CFLAGS)			\
	-DGNOME_TERMINAL_GLADEDIR=\""$(gladedir)"\"	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale\""

LDADD = \
	$(GNOME_TERMINAL_LIBS)	\
	$(INTLLIBS)

bin_PROGRAMS = gnome-terminal

CORBA_GENERATED =		\
	Terminal.h		\
	Terminal-stubs.c	\
	Terminal-skels.c	\
	Terminal-common.c

CLEANFILES += $(CORBA_GENERATED)
BUILT_SOURCES = $(CORBA_GENERATED)

$(CORBA_GENERATED) : $(top_srcdir)/idl/Terminal.idl
	$(ORBIT_IDL) `gnome-config --cflags idl` $(top_srcdir)/idl/Terminal.idl

corba.o: Terminal.h

gnome_terminal_SOURCES = 	\
	gnome-terminal.c	\
	corba.c			\
	gnome-terminal.h	\
	$(CORBA_GENERATED)

gnome_terminal_LDADD = $(LDADD)

gnorbadir = $(sysconfdir)/CORBA/servers
gnorba_DATA = gnome-terminal.gnorba

utildir = $(datadir)/gnome/apps/System
util_in_files = gnome-terminal.desktop.in
util_DATA = $(util_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gladedir   = $(datadir)/gnome-terminal/glade
glade_DATA = gnome-terminal.glade
glade_msgs = gnome-terminal.glade.h

man_MANS = gnome-terminal.1

EXTRA_DIST =				\
	$(gnorba_DATA) 			\
	$(util_DATA)			\
	$(glade_DATA)			\
	$(glade_msgs)			\
	$(man_MANS)

