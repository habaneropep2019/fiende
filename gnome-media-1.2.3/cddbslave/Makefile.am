INCLUDES = -I$(includedir) $(GNOME_INCLUDEDIR) \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = cddbslave cddbsubmit

cddbslave_SOURCES = 	socket.c main.c socket.h socks4.c cddb.h socks4.h

cddbslave_LDADD =    	$(GNOME_LIBDIR) \
                	$(GNOME_LIBS) \
			$(GHTTP_LIB) \
                	$(INTLLIBS)

cddbsubmit_SOURCES =	cddbsubmit.c
cddbsubmit_LDADD =	$(GNOME_LIBDIR) \
			$(GNOME_LIBS) \
			$(GHTTP_LIB) \
			$(INTLLIBS)

gnomedatadir =		$(datadir)/gnome

gnomedata_DATA =	cddb-submit-methods

EXTRA_DIST =		cddb-submit-methods

CLEANFILES=*~