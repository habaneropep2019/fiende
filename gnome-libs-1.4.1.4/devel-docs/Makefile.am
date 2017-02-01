SUBDIRS = gnome gnome-dev-info gnomeui zvt \
	templates ui-guide gnorba gnome-libs-tutorial

TEXT_FILES =					\
	README.gtkcauldron			\
	README.gtkcauldron_for_python		\
	adding-file-manager-new-items.txt	\
	adding-sounds.txt			\
	api-comment-style.txt			\
	gtk_dialog_cauldron.3			\
	suggestions.txt				\
	session-management.txt			\
	mime-type-handling.txt

# automake does not yet dig .sgml files, so for now the DocBook
# files must be extra_dist.  Also, distribute the `.html' file
# so that people without DocBook can still use the docs.
EXTRA_DIST = \
	TODO.sgml			\
	gnome-doc-framework.txt		\
	$(TEXT_FILES)

docdir   = $(prefix)/doc
doc_DATA = $(TEXT_FILES)

# devel-docs/Makefile.am ends here
