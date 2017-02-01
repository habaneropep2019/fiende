lang = C


glade_help_DATA =         \
        topic.dat

sgml_files = faq.sgml  turbo-start.sgml  user-guide.sgml

omffiles = faq-C.omf turbo-start-C.omf user-guide-C.omf

# Files that aren't in a binary/data/library target have to be listed here
# to be included in the tarball when you 'make dist'.
# We include the timestamp files so it won't even try to rebuild the docs
# unless you edit the SGML files.
EXTRA_DIST = \
	$(sgml_files)		\
	$(omffiles)		\
	user-guide.stamp	\
	turbo-start.stamp	\
	faq.stamp		\
        topic.dat

omfstampfiles = user-guide-omf.stamp faq-omf.stamp turbo-start-omf.stamp

CLEANFILES = $(omfstampfiles)

glade_helpdir = $(datadir)/gnome/help/glade/$(lang)

omf_dir=$(top_srcdir)/omf-install

## The '-' before the command means to ignore it if it fails.  That way
## people can still build the software without the docbook tools.

all: html omf

html: user-guide.stamp faq.stamp turbo-start.stamp 

user-guide.stamp: user-guide.sgml
	-db2html user-guide.sgml
	@echo timestamp > user-guide.stamp 2> /dev/null

turbo-start.stamp: turbo-start.sgml
	-db2html turbo-start.sgml
	@echo timestamp > turbo-start.stamp 2> /dev/null

faq.stamp: faq.sgml
	-db2html faq.sgml
	@echo timestamp > faq.stamp 2> /dev/null

omf: $(omfstampfiles)

user-guide-omf.stamp: user-guide-C.omf
	-for file in `ls user-guide-C.omf`; do \
	  scrollkeeper-preinstall $(glade_helpdir)/user-guide/user-guide.sgml $$file $(omf_dir)/$$file; \
	done
	touch user-guide-omf.stamp

faq-omf.stamp: faq-C.omf
	-for file in `ls faq-C.omf`; do \
	  scrollkeeper-preinstall $(glade_helpdir)/faq/faq.sgml $$file $(omf_dir)/$$file; \
	done
	touch faq-omf.stamp

turbo-start-omf.stamp: turbo-start-C.omf
	-for file in `ls turbo-start-C.omf`; do \
	  scrollkeeper-preinstall $(glade_helpdir)/turbo-start/turbo-start.sgml $$file $(omf_dir)/$$file; \
	done
	touch turbo-start-omf.stamp


# I haven't tested these yet.
user-guide.ps: user-guide.sgml
	-db2ps $<

turbo-start.ps: turbo-start.sgml
	-db2ps $<

faq.ps: faq.sgml
	-db2ps $<

user-guide.rtf: user-guide.sgml
	-db2rtf $<

turbo-start.rtf: turbo-start.sgml
	-db2rtf $<

faq.rtf: faq.sgml
	-db2rtf $<


## When we make dist, we include the generated HTML so people don't
## have to have the docbook tools.
dist-hook:
	mkdir $(distdir)/user-guide
	mkdir $(distdir)/user-guide-images
	mkdir $(distdir)/user-guide/stylesheet-images
	mkdir $(distdir)/faq
	mkdir $(distdir)/faq/stylesheet-images
	mkdir $(distdir)/turbo-start
	mkdir $(distdir)/turbo-start/stylesheet-images
	-cp user-guide/*.html user-guide/*.css $(distdir)/user-guide
	-cp user-guide-images/*.png $(distdir)/user-guide-images
	-cp user-guide/stylesheet-images/*.gif $(distdir)/user-guide/stylesheet-images
	-cp user-guide/stylesheet-images/*.png $(distdir)/user-guide/stylesheet-images
	-cp faq/*.html faq/*.css $(distdir)/faq
	-cp faq/stylesheet-images/*.gif $(distdir)/faq/stylesheet-images
	-cp faq/stylesheet-images/*.png $(distdir)/faq/stylesheet-images
	-cp turbo-start/*.html turbo-start/*.css $(distdir)/turbo-start
	-cp turbo-start/stylesheet-images/*.gif $(distdir)/turbo-start/stylesheet-images
	-cp turbo-start/stylesheet-images/*.png $(distdir)/turbo-start/stylesheet-images

install-data-local: omf
	$(mkinstalldirs) $(DESTDIR)$(glade_helpdir)/user-guide/user-guide-images
	$(mkinstalldirs) $(DESTDIR)$(glade_helpdir)/user-guide/stylesheet-images
	-for file in $(srcdir)/user-guide/*.html $(srcdir)/user-guide/*.css $(srcdir)/user-guide.sgml; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/user-guide/$$basefile; \
	done
	-for file in $(srcdir)/user-guide-images/*.png; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/user-guide/user-guide-images/$$basefile; \
	done
	-for file in $(srcdir)/user-guide/stylesheet-images/*.png $(srcdir)/user-guide/stylesheet-images/*.gif; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/user-guide/stylesheet-images/$$basefile; \
	done
	$(mkinstalldirs) $(DESTDIR)$(glade_helpdir)/faq/stylesheet-images
	-for file in $(srcdir)/faq/*.html $(srcdir)/faq/*.css $(srcdir)/faq.sgml; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/faq/$$basefile; \
	done
	-for file in $(srcdir)/faq/stylesheet-images/*.png $(srcdir)/faq/stylesheet-images/*.gif; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/faq/stylesheet-images/$$basefile; \
	done
	$(mkinstalldirs) $(DESTDIR)$(glade_helpdir)/turbo-start/stylesheet-images
	-for file in $(srcdir)/turbo-start/*.html $(srcdir)/turbo-start/*.css $(srcdir)/turbo-start.sgml; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/turbo-start/$$basefile; \
	done
	-for file in $(srcdir)/turbo-start/stylesheet-images/*.png $(srcdir)/turbo-start/stylesheet-images/*.gif; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glade_helpdir)/turbo-start/stylesheet-images/$$basefile; \
	done

clean:
	-rm -rf user-guide faq turbo-start
	-rm -f user-guide.stamp faq.stamp turbo-start.stamp
	-rm -rf user-guide.junk faq.junk turbo-start.junk
