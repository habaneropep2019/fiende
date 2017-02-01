## Process this file with automake to produce Makefile.in

SUBDIRS = . tests doc doc-i18n-tool

NULL = 

aclocal_macros = intltool.m4 xml-i18n-tools.m4

CLEANFILES =					\
	intltoolize				\
	xml-i18n-toolize			\
	intltool-extract.in			\
	intltool-merge.in			\
	intltool-update.in			\
	intltool-extract			\
	intltool-merge				\
	intltool-prepare			\
	intltool-update				\
	intltool-unicodify			\
	*.gz					\
	$(NULL)

# These are required by intltoolize and xml-i18n-toolize.
pkgdata_DATA =					\
	intltool-extract.in			\
	intltool-merge.in			\
	intltool-update.in			\
	intltool-po-Makefile.in.in-patch-1	\
	intltool-po-Makefile.in.in-patch-2	\
	intltool-po-Makefile.in.in-patch-3	\
	xml-i18n-po-Makefile.in.in-patch-1	\
	xml-i18n-po-Makefile.in.in-patch-2	\
	$(NULL)


# This macro file should be visible to Automake's aclocal.
aclocal_DATA = $(aclocal_macros)

# The intltool distributor and installed versions of scripts

bin_SCRIPTS =					\
	intltoolize				\
	xml-i18n-toolize			\
	intltool-update				\
	intltool-merge				\
	intltool-extract			\
	intltool-prepare			\
	intltool-unicodify			\
	$(NULL)

EXTRA_DIST =					\
	$(aclocal_macros)			\
	intltoolize.in				\
	xml-i18n-toolize.in			\
	intltool-extract.in.in			\
	intltool-merge.in.in			\
	intltool-update.in.in			\
	intltool-prepare.in			\
	intltool-unicodify.in			\
	intltool.spec.in			\
	$(pkgdata_DATA)				\
	$(NULL)

dist-hook:
	cp intltool.spec $(distdir)

%: %.in Makefile
	sed -e "s:@INTLTOOL_PERL@:$(PERL):;" < $< > $@
