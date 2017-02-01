glade_helpdir = $(datadir)/gnome/help/glade/it

glade_help_DATA =         \
        topic.dat


# Files that aren't in a binary/data/library target have to be listed here
# to be included in the tarball when you 'make dist'.
# We include the timestamp files so it won't even try to rebuild the docs
# unless you edit the SGML files.
EXTRA_DIST = \
	faq.sgml		\
	faq.stamp		\
        topic.dat


## The '-' before the command means to ignore it if it fails.  That way
## people can still build the software without the docbook tools.

all: faq.stamp 

faq.stamp: faq.sgml
	-db2html faq.sgml
	@echo timestamp > faq.stamp 2> /dev/null


faq.ps: faq.sgml
	-db2ps $<

faq.rtf: faq.sgml
	-db2rtf $<


## When we make dist, we include the generated HTML so people don't
## have to have the docbook tools.
dist-hook:
	mkdir $(distdir)/faq
	mkdir $(distdir)/faq/stylesheet-images
	-cp faq/*.html faq/*.css $(distdir)/faq
	-cp faq/stylesheet-images/*.gif $(distdir)/faq/stylesheet-images
	-cp faq/stylesheet-images/*.png $(distdir)/faq/stylesheet-images

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(glade_helpdir)/faq/stylesheet-images
	-for file in $(srcdir)/faq/*.html $(srcdir)/faq/*.css; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/faq/$$basefile; \
	done
	-for file in $(srcdir)/faq/stylesheet-images/*.png $(srcdir)/faq/stylesheet-images/*.gif; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/faq/stylesheet-images/$$basefile; \
	done

clean:
	-rm -rf faq faq.stamp faq.junk

