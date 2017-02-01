
INCLUDES = -I$(top_srcdir)

LDADD = $(top_builddir)/sigc++/libsigc.la @THREAD_LIB@ 

TESTS = ctor    handle  list_test

noinst_PROGRAMS =  ctor    handle  list_test   

ctor_SOURCES = ctor.cc
handle_SOURCES = handle.cc
list_test_SOURCES = list_test.cc

all_local:  \
 ctor        handle      list_test  

EXTRA_DIST = README
