
INCLUDES = -I$(top_srcdir)
LDADD = $(top_builddir)/sigc++/libsigc.la @THREAD_LIB@

TESTS = thread_test marshal_test

noinst_PROGRAMS =  \
   thread_test  marshal_test

# noinst_HEADERS =

thread_test_SOURCES = thread_test.cc
marshal_test_SOURCES = marshal_test.cc

