NULL=

@XML_I18N_MERGE_DESKTOP_RULE@

SUBDIRS =			\
	data \
	intl \
	eel \
	test \
	po \
	$(NULL)

EXTRA_DIST=					\
	COPYING.LIB				\
	HACKING					\
	MAINTAINERS				\
	eel.spec.in			\
	eelConf.sh.in			\
	eel-config.in			\
	xml-i18n-extract.in			\
	xml-i18n-merge.in			\
	xml-i18n-update.in			\
	$(NULL)

## [The following is shamelessly copied from gnome-vfs.]

## We create `eelConf.sh' here and not from configure because we
## want to get the paths expanded correctly.  Macros like srcdir are
## given the value NONE in configure if the user doesn't specify them
## (this is an autoconf feature, not a bug).

bin_SCRIPTS = $(top_builddir)/eel-config

confexecdir=$(libdir)
confexec_DATA =					\
	eelConf.sh				\
	$(NULL)

eelConf.sh: eelConf.sh.in Makefile
	sed -e 's?\@EEL_LIBDIR\@?$(EEL_LIBDIR)?g' \
	    -e 's?\@EEL_INCLUDEDIR\@?$(EEL_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@EEL_LIBS\@?$(EEL_LIBS)?g' \
	      < $(srcdir)/eelConf.sh.in > eelConf.tmp \
	  && mv eelConf.tmp eelConf.sh

eel-config: eel-config.in Makefile
	sed -e 's?\@LIBDIR\@?$(libdir)?g' \
	    -e 's?\@EEL_LIBDIR\@?$(EEL_LIBDIR)?g' \
	    -e 's?\@EEL_INCLUDEDIR\@?$(EEL_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@EEL_LIBS\@?$(EEL_LIBS)?g' \
	      < $(srcdir)/eel-config.in > eel-config.tmp \
	  && mv eel-config.tmp eel-config && \
		chmod 755 eel-config

##################################################################
#
# dist-hook
#
##################################################################

dist-hook: eel.spec
	cp eel.spec $(distdir)

# This is ridiculous, but automake apparently won't allow conditionals
# in EXTRA_DIST.

	mkdir -p $(distdir)/eazel-logos
	list='$(EAZEL_LOGOS_ICONS)'; \
	for i in $$list; \
	do \
		cp $$i $(distdir)/eazel-logos; \
	done

	mkdir -p $(distdir)/eazel-logos/throbber
	list='$(EAZEL_LOGOS_THROBBER)'; \
	for i in $$list; \
	do \
		cp $$i $(distdir)/eazel-logos/throbber; \
	done
