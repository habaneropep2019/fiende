H_FILES = $(patsubst %.gen_h,headers/%.h,$(GEN_H_FILES)) 

headers/%.h: headers $(top_srcdir)/src/%.gen_h
	$(GTKMM_PROC) --local --m4 $(notdir $*) $(top_srcdir)/src/ ./headers

headers/gnome--.h: headers
	cp $(top_srcdir)/src/gnome--.h ./headers
	cp $(top_srcdir)/src/gnome--/*.h ./headers

all-local: headers $(H_FILES) headers/gnome--.h html

headers:
	@mkdir $@

html: $(H_FILES) headers/gnome--.h
	@PERL@ $(top_srcdir)/docgen/pce2 --plgdir $(top_srcdir)/docgen/ headers/*.h
	touch html

DISTCLEANFILES = $(H_FILES)
CLEANFILES = $(H_FILES) *.html html

#Publish to the website:
SFDEST=$$USER@gtkmm.sourceforge.net:/home/groups/g/gt/gtkmm/htdocs/docs

tarball:
	(cd ..; tar cvzf gnomemm-ref.tar.gz `find . -name "*.html"`)

sf_install: tarball
	tar cf - *.html *.tar.gz | ssh $$USER@gtkmm.sourceforge.net "cd /home/groups/g/gt/gtkmm/htdocs/docs/gnome; tar xvf - ; chmod a+r *"
