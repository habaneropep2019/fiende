gtop_helpdir = $(datadir)/gnome/help/gtop/C

gtop_help_DATA = 	\
	index.html		\
	topic.dat

EXTRA_DIST = gtop.sgml topic.dat

all: index.html

index.html: gtop/index.html
	-cp gtop/index.html .

# the wierd srcdir trick is because the db2html from the Cygnus RPMs
# cannot handle relative filenames
gtop/index.html: $(srcdir)/gtop.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/gtop.sgml
	-cp -pr $(srcdir)/figs gtop

dist-hook: index.html
	mkdir $(distdir)/gtop
	-cp gtop/*.html $(distdir)/gtop
	-cp -pr $(srcdir)/figs $(distdir)/gtop

install-data-local: index.html
	$(mkinstalldirs) $(DESTDIR)$(gtop_helpdir)/figs
	-for file in $(srcdir)/gtop/*.html; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(gtop_helpdir)/$$basefile; \
	done
	-for file in $(srcdir)/gtop/figs/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(gtop_helpdir)/figs/$$basefile; \
	done

gtop.ps: gtop.sgml
	-db2ps $<

gtop.rtf: gtop.sgml
	-db2rtf $<
