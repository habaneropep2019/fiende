
# this is from gtk's build_sources/Makefile.am, a little bit cleaned up

EXTRA_DIST = convert.m4 make_translate.pl gnomeconvert.m4 property.m4

gensourcedir=$(top_builddir)/src/gnome--

gnomemmhwithdirs = $(GEN_H_FILES:.gen_h=.h)
gnomemmh = $(gnomemmhwithdirs)
gnomemmc = $(gnomemmh:.h=.cc) gnome--.cc

gnomemmgenh = $(patsubst %.gen_h,$(srcdir)/../%.gen_h,$(gnomemmsources))

gnomemmsources = $(GNOME_GEN_H_FILES)
gnomemmh = $(patsubst %.gen_h,$(gensourcedir)/%.h,$(gnomemmsources))
gnomemmcc = $(patsubst %.gen_h,$(gensourcedir)/%.cc,$(gnomemmsources))

gensources: $(gnomemmh) $(gnomemmcc)
	echo > gensources

#$(gensourcedir)/%.cc $(gensourcedir)/%.h: $(srcdir)/../%.gen_h $(top_builddir)/src/gensig/gensig.cc
#	$(top_builddir)/src/gensig/gensig -g $(notdir $*) $(srcdir)/../ $(gensourcedir)/

$(genheaderdir)/%.h: $(srcdir)/%.h.m4
	rm -f $(genheaderdir)/$(notdir $*).h
	(cd $(srcdir); $(M4) $(notdir $*).h.m4) >$(genheaderdir)/$(notdir $*).h

$(gensourcedir)/%.cc $(gensourcedir)/%.h: $(srcdir)/../%.gen_h 
	$(GTKMM_PROC) --m4 $(srcdir) --gnome $(notdir $*) $(srcdir)/../ $(gensourcedir)/

$(gensourcedir)/wrap.cc: $(top_srcdir)/src/build_sources/make_translate.pl $(gtkmmgenh)
	perl $(top_srcdir)/src/build_sources/make_translate.pl --module=gnomemm --path=gnome-- $(srcdir)/../*.gen_h > $(gensourcedir)/wrap.cc

all-local: $(gensourcedir)/wrap.cc
#all-local: gensources translater



gnomemmtmp_SOURCES = gnomemmtmp.c
gnomemmtmp_DEPENDENCIES = gensources
noinst_PROGRAMS = gnomemmtmp

DISTCLEANFILES = gensources
CLEANFILES = gensources
