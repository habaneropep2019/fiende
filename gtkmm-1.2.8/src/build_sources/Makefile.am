
# This is huge hackery to make automake build the sources before
# we start compiling them (anyone know better way?) - problem
# is with dependencies, normally it expects only stuffs mentioned in
# the dependencies are required to build it...


# is this proper place to generated source files?
gensourcedir=$(top_builddir)/src/gtk--

# this is only for signal system header stuffs.
genheaderdir=$(top_builddir)/src/gtk--

gtkmmsources = $(GEN_H_FILES)
gtkmmgenh = $(patsubst %.gen_h,$(srcdir)/../%.gen_h,$(gtkmmsources))
gtkmmh = $(patsubst %.gen_h,$(gensourcedir)/%.h,$(gtkmmsources))
gtkmmcc = $(patsubst %.gen_h,$(gensourcedir)/%.cc,$(gtkmmsources))



$(gensourcedir)/%.cc $(gensourcedir)/%.h: $(srcdir)/../%.gen_h ../gtkmmproc/stage1.m4
	../gtkmmproc/gtkmmproc --m4 $(srcdir)/../gtkmmproc $(notdir $*) $(srcdir)/../ $(gensourcedir)/

$(gensourcedir)/wrap.cc: make_translate.pl $(gtkmmgenh)
	perl $(srcdir)/make_translate.pl $(srcdir)/../*.gen_h > $(gensourcedir)/wrap.cc.tmp
	if test -f $(gensourcedir)/wrap.cc ; then \
	diff -q $(gensourcedir)/wrap.cc $(gensourcedir)/wrap.cc.tmp; \
	if test "$$?" -ne 0 ; then \
	cp $(gensourcedir)/wrap.cc.tmp $(gensourcedir)/wrap.cc; \
	fi ; \
	else  \
	cp $(gensourcedir)/wrap.cc.tmp $(gensourcedir)/wrap.cc; \
	fi
	rm $(gensourcedir)/wrap.cc.tmp 

$(gensourcedir)/proxy.h: proxy.h.m4
	(cd $(srcdir); $(M4) proxy.h.m4 > $(genheaderdir)/proxy.h)


all-local: $(gtkmmh) $(gtkmmcc) $(gensourcedir)/proxy.h $(gensourcedir)/wrap.cc

EXTRA_DIST = proxy.h.m4 template.macros.m4 make_translate.pl

DISTCLEANFILES = gensources
CLEANFILES = gensources
