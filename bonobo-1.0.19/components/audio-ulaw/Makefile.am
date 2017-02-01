SUBDIRS = doc

bin_PROGRAMS = bonobo-audio-ulaw

bonobo_audio_ulaw_SOURCES =	\
	bonobo-audio-ulaw.c	\
	bonobo-audio-ulaw.h	\
	item-audio.c		\
	item-audio.h		\
	color.c			\
	color.h

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

bonobo_audio_ulaw_LDADD = 			\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)			\
	$(INTLLIBS)

oafdir = $(datadir)/oaf
oaf_in_files = Bonobo_Sample_Audio.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

EXTRA_DIST = $(oaf_in_files) $(oaf_DATA)

