#if GTKMM_GENDOCS
#MANPAGES = yes
#else
#MANPAGES = no
#endif

SUBDIRS = gtk gdk 

EXTRA_DIST= \
FAQ/Makefile            \
FAQ/FAQ.sgml            \
pix/details.gif		\
pix/examples.gif	\
pix/members.gif		\
pix/properties.gif	\
pix/top.gif		\
css/default.css         \
tutorial/Makefile 	\
tutorial/adjustments.sgml.m4	\
tutorial/advanced_events.sgml.m4	\
tutorial/attributes.sgml.m4	\
tutorial/button_widgets.sgml.m4	\
tutorial/clist_widget.sgml.m4	\
tutorial/code_examples.sgml.m4	\
tutorial/container.sgml.m4	\
tutorial/contributing.sgml.m4	\
tutorial/credits.sgml.m4	\
tutorial/gdk_events.sgml.m4	\
tutorial/getting_started.sgml.m4	\
tutorial/glib.sgml.m4		\
tutorial/gtkmm_signals.sgml.m4	\
tutorial/gtkmm_specifics.sgml.m4	\
tutorial/introduction.sgml.m4	\
tutorial/list_widget.sgml.m4	\
tutorial/memory.sgml.m4 	\
tutorial/menu_widget.sgml.m4	\
tutorial/misc_widgets.sgml.m4	\
tutorial/moving_on.sgml.m4	\
tutorial/multiitem.sgml.m4	\
tutorial/notice.sgml.m4		\
tutorial/packing_widgets.sgml.m4	\
tutorial/range_widgets.sgml.m4	\
tutorial/rc_files.sgml.m4	\
tutorial/scribble.sgml.m4	\
tutorial/selections.sgml.m4	\
tutorial/sgml.macros.m4		\
tutorial/singleitem.sgml.m4	\
tutorial/text_widget.sgml.m4	\
tutorial/timeouts.sgml.m4	\
tutorial/tips.sgml.m4		\
tutorial/tree_widget.sgml.m4	\
tutorial/tutorial.sgml.m4	\
tutorial/undocumented.sgml.m4	\
tutorial/widget_overview.sgml.m4 \
tutorial/writing_widgets.sgml.m4 \
tutorial/gtkmm-tut-html/gtk_tut_packbox1.png \
tutorial/gtkmm-tut-html/gtk_tut_packbox2.png \
tutorial/gtkmm-tut-html/gtk_tut_table.png 

#install-data-local:
#	if [ x$(MANPAGES) != xno ] ; then \
#	    $(mkinstalldirs) $(DESTDIR)$(mandir)/man3; \
#	    for i in *.3; do \
#	        $(INSTALL) $$i $(DESTDIR)$(mandir)/man3/$$i; \
#	    done \
#	fi

all-local:
