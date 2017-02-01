## top directory

SUBDIRS		= panel--
DIST_SUBDIRS	= panel-- examples scripts

confexecdir	= $(libdir)
confexec_DATA	= panelmmConf.sh

EXTRA_DIST	= configure panelmmConf.sh.in

panelmmConf.sh: panelmmConf.sh.in Makefile
## Use sed and then mv to avo`id problems if the user interrupts.
	sed -e 's?\@PANELMM_LIBDIR\@?$(PANELMM_LIBDIR)?g' \
	     -e 's?\@PANELMM_INCLUDEDIR\@?$(PANELMM_INCLUDEDIR)?g' \
	     -e 's?\@VERSION\@?$(VERSION)?g' \
	     -e 's?\@PANELMM_LIBS\@?$(PANELMM_LIBS)?g' \
	        < $(srcdir)/panelmmConf.sh.in > panelmmConf.tmp \
	&& mv panelmmConf.tmp panelmmConf.sh
