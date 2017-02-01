H_FILES = $(patsubst %.gen_h,headers/%.h,$(GEN_H_FILES)) 

headers/%.h: $(top_srcdir)/src/%.gen_h
	$(top_builddir)/src/gtkmmproc/gtkmmproc --doc --m4 $(top_srcdir)/src/gtkmmproc $(notdir $*) $(top_srcdir)/src/ ./headers

headers/base.h:
	cp $(top_srcdir)/src/gtk--/base.h ./headers


all-local: headers $(H_FILES) headers/base.h html

headers:
	@mkdir $@

html:
	@PERL@ $(top_srcdir)/docgen/pce2 --plgdir $(top_srcdir)/docgen/ headers/*.h
	touch html

DISTCLEANFILES = $(H_FILES)
CLEANFILES = $(H_FILES) *.html html

SFDEST=$$USER@gtkmm.sourceforge.net:/home/groups/gtkmm/htdocs

tarball:
	(cd ..; tar cvzf gtk/gtkmm-ref.tar.gz `find gtk -name "*.html"`)

sf_install: tarball
	tar cf - *.html *.tar.gz | ssh $$USER@gtkmm.sourceforge.net "cd /home/groups/g/gt/gtkmm/htdocs/docs/gtk; tar xvf - ; chmod a+r *"
