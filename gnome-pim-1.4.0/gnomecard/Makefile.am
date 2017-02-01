nSUBDIRS = help

xpmhelp_base = $(datadir)/gnome/help/cal

INCLUDES = 			\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)				\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	$(PISOCK_INCLUDEDIR)                            \
	-I../../gcalendar

if HAVE_GNOME_PILOT
extra_pilot_bins = 				\
		address-conduit-capplet	
endif

bin_PROGRAMS = gnomecard $(extra_pilot_bins)

gnomecard_SOURCES =	\
        $(CORBA_SRCLIST) \
	card.c		\
	card.h		\
	columnhdrs.c    \
	columnhdrs.h    \
	cardtypes.h	\
	canvas.c	\
	canvas.h	\
	del.c		\
	del.h		\
	deladdrlist.c   \
	deladdrlist.h   \
	dialog.c        \
	dialog.h        \
	dnd.h		\
	emaillist.c     \
	emaillist.h     \
	gnomecard.c	\
	gnomecard.h	\
	images.h        \
	init.c          \
	init.h          \
	list.c		\
	list.h		\
	misc.c          \
	misc.h          \
	my.c            \
	my.h            \
	phonelist.c     \
	phonelist.h     \
	pix.c           \
	pix.h           \
	popup-menu.c	\
	popup-menu.h	\
	sort.c          \
	gnome-pim-impl.c\
	sort.h          \
	pairs.h         \
	type_name.h
#         \
#	tree.c          \
#	tree.h

CORBA_SRCLIST = gnome-pim-common.c \
	gnome-pim-skels.c	\
	gnome-pim-stubs.c	\
	gnome-pim.h

CLEANFILES += $(CORBA_SRCLIST)

$(CORBA_SRCLIST): $(top_srcdir)/idl/gnome-pim.idl $(ORBIT_IDL)
	orbit-idl $(top_srcdir)/idl/gnome-pim.idl

LINK_FLAGS = \
	$(INTLLIBS)	\
	../libversit/libversit.la

gnomecard_LDADD = $(LINK_FLAGS)\
	$(GNOME_LIBDIR) \
	$(GNOMEGNORBA_LIBS)

if HAVE_GNOME_PILOT
address_conduitsdir=$(libdir)/gnome-pilot/conduits

address_conduits_LTLIBRARIES = libaddress_conduit.la
endif 

libaddress_conduit_la_SOURCES = \
	card.c \
	my.c \
	address-conduit.h \
	address-conduit.c

libaddress_conduit_la_LIBADD = \
	../libversit/libversit.la \
	$(PISOCK_LIBS) \
	$(GNOME_LIBDIR) \
	$(GNOME_LIBS)

address_conduit_capplet_SOURCES = \
	address-conduit-capplet.c \
	gnomecard.conduit.in

address_conduit_capplet_LDADD = \
	$(CAPPLET_LIBS) \
	$(GNOME_LIBDIR) \
        $(GNOME_CAPPLET_LIBS) \
        $(PISOCK_LIBS) \
        $(GNOMEUI_LIBS) \
	$(GNOME_XML_LIB) \
	$(GNOME_PILOT_LIBS) \
        $(INTLLIBS) 

gnomecard.conduit: gnomecard.conduit.in Makefile
	sed -e 's^\@prefix\@^$(prefix)^g' < $(srcdir)/gnomecard.conduit.in > gnomecard.conduit.tmp \
	&& mv gnomecard.conduit.tmp gnomecard.conduit

if HAVE_GNOME_PILOT
ccenterdir = $(datadir)/control-center
ccenterPalmPilotdir = $(ccenterdir)/Peripherals
ccenterConduitsdir = $(ccenterPalmPilotdir)/Conduits

ccenterConduits_DATA = \
	address-conduit-capplet.desktop 

panelConduitsdir = $(datadir)/gnome/apps/Settings/Peripherals/Conduits
panelConduits_DATA = $(ccenterConduits_DATA)

Conduitdir = $(datadir)/gnome-pilot/conduits/
Conduit_DATA = gnomecard.conduit
endif

EXTRA_DIST =		\
	addr.xpm	\
	addrtool.xpm	\
	cardedit.xpm	\
	cardnew.xpm	\
	cardfind.xpm	\
	email.xpm	\
	expl.xpm	\
	first.xpm	\
	geo.xpm		\
	ident.xpm	\
	last.xpm	\
	org.xpm		\
	sec.xpm		\
	tel.xpm		\
	teltool.xpm	\
	phone.xpm	\
	world.xpm	\
	gnome-card.xpm  \
	gnomecard.desktop \
	address-conduit-capplet.desktop \
	gnomecard.conduit \
	gnome-gnomecard.png \
	gnomecard.gnorba \
	ChangeLog \
	BUGS

Productivitydir = $(datadir)/gnome/apps/Applications

Productivity_DATA = gnomecard.desktop

Icondir = $(datadir)/pixmaps
Icon_DATA = gnome-gnomecard.png

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/CORBA/servers
	$(INSTALL_DATA)  $(srcdir)/gnomecard.gnorba \
		$(DESTDIR)$(sysconfdir)/CORBA/servers/gnomecard.gnorba
	$(mkinstalldirs) $(Conduitsdir)

