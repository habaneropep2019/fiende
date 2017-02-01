
INCLUDES = -I$(top_srcdir)  -I$(top_builddir)
LDADD = $(top_builddir)/sigc++/libsigc.la @THREAD_LIB@

noinst_PROGRAMS = \
   func_slot_test  object_slot_test   array_test  \
   basic           vbasic \
   bind_slot_test  rettype_slot_test  convert_slot_test \
   disconnect      retbind_slot_test

TESTS = basic bind_slot_test convert_slot_test disconnect

# noinst_HEADERS =

# size_test_SOURCES = size_test.cc
# bind_SOURCES = bind.cc

#
# Slot tests
func_slot_test_SOURCES = func_slot_test.cc
object_slot_test_SOURCES = object_slot_test.cc
array_test_SOURCES = array_test.cc

#
# Signal tests
basic_SOURCES = basic.cc
vbasic_SOURCES = vbasic.cc
disconnect_SOURCES = disconnect.cc

#
# Adaptor tests
bind_slot_test_SOURCES = bind_slot_test.cc
rettype_slot_test_SOURCES = rettype_slot_test.cc
retbind_slot_test_SOURCES = retbind_slot_test.cc
convert_slot_test_SOURCES = convert_slot_test.cc

