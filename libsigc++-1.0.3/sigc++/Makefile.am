SUBDIRS = config macros

INCLUDES = -I$(top_srcdir) -I$(top_builddir)
M4_DIR = $(top_srcdir)/sigc++/macros
M4_INCLUDES = -I$(M4_DIR)

BUILT_SOURCES = slot.h object_slot.h func_slot.h class_slot.h \
                basic_signal.h \
                convert.h bind.h rettype.h retbind.h

libsigc_la_SOURCES= \
   object.cc   scope.cc   marshal.cc   slot.cc\
   basic_signal.cc adaptor.cc thread.cc

libsigc_la_LDFLAGS = -version-info $(LIBSIGC_SO_VERSION) \
                     -release $(SIGC_RELEASE) -no-undefined
libsigcdir=$(prefix)/include/sigc++

lib_LTLIBRARIES = libsigc.la

libsigcincludedir=$(includedir)/sigc++
libsigcinclude_HEADERS = \
    marshal.h handle_system.h signal_system.h \
    generator.h $(BUILT_SOURCES) type.h\
    handle.h object.h scope.h adaptor.h thread.h


%.h: $(M4_DIR)/%.h.m4 $(M4_DIR)/template.macros.m4        
	$(M4) $(M4_INCLUDES) $(DEFINES) $<  > $@

all-local: $(LTLIBRARIES) 

EXTRA_DIST = README
