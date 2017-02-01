if BUILD_GTT
gtt = gtt
endif

if GUILE
if FALSE
guile_extra_DIRS = cromagnon
endif
guile_DIRS = $(gtt) $(guile_extra_DIRS)
endif

if HAVE_LIBGTOP
gdiskfree = gdiskfree
if HAVE_LIBXML
stripchart = stripchart
endif
endif

if BUILD_GFLOPPY
gfloppy = gfloppy
endif

SUBDIRS = po intl macros idl $(gfloppy) logview gcharmap \
	gcalc gsearchtool $(gdiskfree) $(stripchart) gdictsrc \
	$(guile_DIRS) mini-utils splash gdialog \
	meat-grinder exe-handler omf-install

DIST_SUBDIRS = po intl macros idl gfloppy logview gcharmap \
	gcalc gsearchtool gdiskfree stripchart gdictsrc \
	gtt cromagnon mini-utils splash gdialog \
	meat-grinder exe-handler omf-install

EXTRA_DIST = autogen.sh gnome-utils.spec.in

dist-hook: gnome-utils.spec
	cp gnome-utils.spec $(distdir)

## to automatically rebuild aclocal.m4 if any of the macros in
## `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep
