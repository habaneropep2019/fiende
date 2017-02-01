
SRC = \
adjustments.sgml.m4      glib.sgml.m4             scribble.sgml.m4        \
advanced_events.sgml.m4  gtkmm_signals.sgml.m4    selections.sgml.m4      \
attributes.sgml.m4       introduction.sgml.m4     text_widget.sgml.m4     \
button_widgets.sgml.m4   list_widget.sgml.m4      timeouts.sgml.m4        \
clist_widget.sgml.m4     menu_widget.sgml.m4      tips.sgml.m4            \
code_examples.sgml.m4    misc_widgets.sgml.m4     tree_widget.sgml.m4     \
container.sgml.m4        moving_on.sgml.m4        tutorial.sgml.m4        \
contributing.sgml.m4     notice.sgml.m4           undocumented.sgml.m4    \
credits.sgml.m4          packing_widgets.sgml.m4  widget_overview.sgml.m4 \
gdk_events.sgml.m4       range_widgets.sgml.m4    writing_widgets.sgml.m4 \
getting_started.sgml.m4  rc_files.sgml.m4         singleitem.sgml.m4      \
multiitem.sgml.m4		 memory.sgml.m4

SGML2HTML = $(shell awk '/^SGML2HTML/ {print $$3}' ../Makefile)
M4 = $(shell awk '/^M4/ {print $$3}' ../Makefile)

HTMLDEST=gtkmm.sourceforge.net:/home/groups/gtkmm/htdocs/tutorial

all: tutorial.html

tutorial.sgml: $(SRC) sgml.macros.m4
	@if test ! -n "$(M4)" ; then \
	  echo "m4 is required to build the tutorial.  Skipping." ; \
	else \
	  $(M4) tutorial.sgml.m4 > tutorial.sgml ; \
	fi

tutorial.html: tutorial.sgml
	@if test ! -n "$(SGML2HTML)" ; then \
	  echo "sgml2html is required to build the tutorial.  Skipping." ; \
	else \
	  $(SGML2HTML) tutorial.sgml ; \
	fi
#	- mkdir gtkmm-tut-html
	rm -f gtkmm-tut-html/*.html*
	mv *.html gtkmm-tut-html

gtkmm-tut-html.tgz:
	tar czf gtkmm-tut-html.tgz gtkmm-tut-html

latex: tutorial.dvi
ps: tutorial.ps
pdf: tutorial.pdf

tutorial.dvi: tutorial.sgml
	sgml2latex -P "\usepackage{times}\usepackage{courier}\usepackage{helvetic}" tutorial.sgml

tutorial.ps:  tutorial.dvi
	dvips tutorial.dvi -o tutorial.ps

tutorial.pdf: tutorial.ps
	distill tutorial.ps

post-pdf:
	tar cf - tutorial.dvi tutorial.ps tutorial.pdf | ssh $$SSH_OPT  $$USER@gtkmm.sourceforge.net "cd /home/groups/gtkmm/htdocs/tutorial; tar xvf - ; chmod a+r,g+w * ; chgrp gtkmm *"

post-html: gtkmm-tut-html.tgz
	(cd gtkmm-tut-html ; tar cf - *.html ../gtkmm-tut-html.tgz | \
	 ssh gtkmm.sourceforge.net "cd /home/groups/gtkmm/htdocs/tutorial; tar xvf - ; chmod a+r,g+w * ; chgrp gtkmm *")
#	rsync -v -e ssh --delete-excluded -z gtkmm-tut-html/* $(HTMLDEST)
#	rsync -v -e ssh --stats --progress gtkmm-tut-html.tgz gtkmm.sourceforge.net:/home/groups/gtkmm/htdocs


clean: 
	rm -f tutorial.sgml tutorial-*.html tutorial.html
	rm -f gtkmm-tut-html/*.html*
