htmldir = $(datadir)/gnome/help/gnome-dev-info/C
#html_DATA = gnome-dev-info.html

all-local: gnome-dev-info/book1.html

gnome-dev-info/book1.html: gnome-dev-info.sgml libgnomeui.sgml libgnome.sgml
	-(cd $(srcdir); db2html gnome-dev-info.sgml) > /dev/null

libgnomeui.sgml:
	if $(SHELL) -c 'gdoc -V > /dev/null 2>&1 < /dev/null'; then \
## Inputs and generated files all are in srcdir.
		cd $(srcdir); \
		: > libgnomeui.sgml; \
		for i in ../../libgnomeui/*.h; do \
## FIXME: should use explicit perl invocation here.
			../gdoc/gdoc.pl $$i >> libgnomeui.sgml; \
		done; \
		cat trailer.sgml >> libgnomeui.sgml; \
	fi

libgnome.sgml:
	if $(SHELL) -c 'gdoc -V > /dev/null 2>&1 < /dev/null'; then \
## Inputs and generated files all are in srcdir.
		cd $(srcdir); \
		: > libgnome.sgml; \
## FIXME: should use explicit perl invocation here.
		for i in ../../libgnome/*.h; do \
			../gdoc/gdoc.pl $$i >> libgnome.sgml; \
		done; \
		cat trailer.sgml >> libgnome.sgml; \
	fi

TEXT_FILES =

PIC_FILES= \
	gnome-hello-0-basic.jpg \
	gnome-hello-1-menus.jpg \
	gnome-hello-2-i18n.jpg

# automake does not yet dig .sgml files, so for now the DocBook
# files must be extra_dist.  Also, distribute the `.html' file
# so that people without DocBook can still use the docs.
EXTRA_DIST = \
	gnome-app-helper.sgml		\
	gnome-canvas.sgml		\
	gnome-dev-info.sgml		\
	gnome-message-utils.sgml	\
	x-concepts.sgml			\
	libgnorba.sgml			\
	gnome-mdi.sgml			\
	libgnomeui.sgml			\
	libgnome.sgml			\
	trailer.sgml			\
	$(TEXT_FILES)			\
	$(PIC_FILES)

dist-hook:
	tar cvf - gnome-dev-info | (cd $(distdir); tar xf -)
	
install-data-local: gnome-dev-info/book1.html
	$(mkinstalldirs) $(DESTDIR)$(htmldir)
	installfiles=`echo $(srcdir)/gnome-dev-info/*.html`;		    \
	if test "$$installfiles" != '$(srcdir)/gnome-dev-info/*.html'; then \
	  for file in $$installfiles; do				    \
            basefile=`echo $$file | sed -e 's,^.*/,,'`;			    \
            $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$basefile;	    \
          done;								    \
	else :; fi
	installfiles=`echo $(srcdir)/gnome-dev-info/*.css`;		   \
	if test "$$installfiles" != '$(srcdir)/gnome-dev-info/*.css'; then \
	  for file in $$installfiles; do				   \
            basefile=`echo $$file | sed -e 's,^.*/,,'`;			   \
            $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$basefile;	   \
          done;								   \
	else :; fi
	installfiles=`echo $(srcdir)/gnome-dev-info/*.jpg`;		   \
	if test "$$installfiles" != '$(srcdir)/gnome-dev-info/*.jpg'; then \
	  for file in $$installfiles; do				   \
            basefile=`echo $$file | sed -e 's,^.*/,,'`;			   \
            $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$basefile;	   \
          done;								   \
	else :; fi

# devel-docs/gnome-dev-info/Makefile.am ends here

