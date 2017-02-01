AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = guile-foo
lib_LTLIBRARIES = libfoo.la

INCLUDES = $(GUILEGTK_CFLAGS)

guile_foo_SOURCES = main.c
guile_foo_LDFLAGS = -export-dynamic
guile_foo_LDADD   = libfoo.la $(GUILEGTK_LIBS)

libfoo_la_SOURCES = foo.c foo.h foo-glue.c
libfoo_la_LDFLAGS = -version-info 0:0:0 -export-dynamic
libfoo_la_LIBADD  = $(GUILEGTK_LIBLIBS)

BUILT_SOURCES = foo-glue.c main.c

foo-glue.c: foo.defs
	build-guile-gtk glue $< >tmp && mv tmp $@

main.c: foo.defs
	build-guile-gtk main $< >tmp && mv tmp $@

scmgtkdir=$(datadir)/guile/gtk
scmgtk_DATA = foo.scm
