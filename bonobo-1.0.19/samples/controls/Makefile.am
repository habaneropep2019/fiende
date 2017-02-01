bin_PROGRAMS = bonobo-sample-controls sample-control-container

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

bonobo_sample_controls_SOURCES =			\
	bonobo-clock-control.c				\
	bonobo-clock-control.h				\
	bonobo-calculator-control.c			\
	bonobo-calculator-control.h			\
	sample-control-factory.c

bonobo_sample_controls_LDADD =				\
	$(top_builddir)/bonobo/libbonobo.la		\
	$(top_builddir)/bonobo/libbonobox.la		\
	$(BONOBOX_TEST_LIBS)				\
	$(INTLLIBS)

sample_control_container_SOURCES =			\
	sample-control-container.c

sample_control_container_LDADD =			\
	$(top_builddir)/bonobo/libbonobo.la		\
	$(top_builddir)/bonobo/libbonobox.la		\
	$(BONOBOX_TEST_LIBS)				\
	$(INTLLIBS)

oafdir = $(datadir)/oaf
oaf_in_files = Bonobo_Sample_Controls.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

EXTRA_DIST = $(oaf_in_files) $(oaf_DATA)



