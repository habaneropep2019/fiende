NULL=

SUBDIRS = \
	macros intl 			\
	idl 				\
	libefs 				\
	bonobo storage-modules 		\
	monikers			\
	components samples	 	\
	gshell				\
	tests doc po

confexecdir = $(libdir)
confexec_DATA = bonoboConf.sh bonoboxConf.sh bonobox_printConf.sh

bonoboincludedir=$(includedir)/gnome-1.0
bonoboinclude_HEADERS = \
	bonobo.h

EXTRA_DIST =					\
	bonoboConf.sh.in			\
	bonoboxConf.sh.in			\
	bonobox_printConf.sh.in			\
	bonobo.spec.in				\
	xml-i18n-extract.in			\
	xml-i18n-merge.in			\
	xml-i18n-update.in			\
	$(NULL)

bonoboConf.sh: $(top_srcdir)/bonoboConf.sh.in Makefile
	sed -e 's?\@BONOBO_LIBDIR\@?$(BONOBO_LIBDIR)?g' \
	    -e 's?\@BONOBO_INCLUDEDIR\@?$(BONOBO_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@BONOBO_LIBS\@?$(BONOBO_LIBS)?g' \
	      < $(top_srcdir)/bonoboConf.sh.in > bonoboConf.tmp \
	  && mv bonoboConf.tmp bonoboConf.sh

bonoboxConf.sh: $(top_srcdir)/bonoboxConf.sh.in Makefile
	sed -e 's?\@BONOBO_LIBDIR\@?$(BONOBO_LIBDIR)?g' \
	    -e 's?\@BONOBO_INCLUDEDIR\@?$(BONOBO_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@BONOBOX_LIBS\@?$(BONOBOX_LIBS)?g' \
	      < $(top_srcdir)/bonoboxConf.sh.in > bonoboxConf.tmp \
	  && mv bonoboxConf.tmp bonoboxConf.sh

bonobox_printConf.sh: $(top_srcdir)/bonobox_printConf.sh.in Makefile
	sed -e 's?\@BONOBO_LIBDIR\@?$(BONOBO_LIBDIR)?g' \
	    -e 's?\@BONOBO_INCLUDEDIR\@?$(BONOBO_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@BONOBOX_PRINT_LIBS\@?$(BONOBOX_PRINT_LIBS)?g' \
	      < $(top_srcdir)/bonobox_printConf.sh.in > bonobox_printConf.tmp \
	  && mv bonobox_printConf.tmp bonobox_printConf.sh

dist-hook: bonobo.spec
	cp bonobo.spec $(distdir)

CLEANFILES=bonoboConf.sh bonoboxConf.sh bonobox_printConf.sh
