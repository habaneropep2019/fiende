noinst_PROGRAMS = 	testprint  \
				testtext   \
				testprint2 \
				testprint3 \
				testprint4 \
				testprint5 \
				testprint6 \
				testfax	 \
				testpdf    \
				testmetrics\
				testembed  \
				testzip

if LIBGPA
LIBGPA_TEST_CFLAGS = -I$(top_srcdir)/libgpa
LIBGPA_TEST_LIBS = $(top_builddir)/libgpa/libgpa.la
else
LIBGPA_TEST_CFLAGS = 
LIBGPA_TEST_LIBS =
endif

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/intl \
	-I$(top_builddir)/intl \
	-I$(top_srcdir)/libgnomeprint \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DPROFILEDIR=\""$(profilesdir)"\" 

CPPFLAGS = @CPPFLAGS@ \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\"

print_libs = \
	$(top_builddir)/libgnomeprint/libgnomeprint.la \
	$(LIBGNOMEPRINT_LIBS) \
	$(INTLLIBS)

testprint_SOURCES = testprint.c
testprint_LDFLAGS =
testprint_DEPENDENCIES = $(DEPS)
testprint_LDADD = \
	$(print_libs)

testprint2_SOURCES = testprint2.c
testprint2_LDFLAGS =
testprint2_DEPENDENCIES = $(DEPS)
testprint2_LDADD = \
	$(print_libs)

testprint3_SOURCES = testprint3.c
testprint3_LDFLAGS =
testprint3_DEPENDENCIES = $(DEPS)
testprint3_LDADD = \
	$(print_libs)

testprint4_SOURCES = testprint4.c
testprint4_LDFLAGS =
testprint4_DEPENDENCIES = $(DEPS)
testprint4_LDADD = \
	$(print_libs)

testprint5_SOURCES = testprint5.c
testprint5_LDFLAGS =
testprint5_DEPENDENCIES = $(DEPS)
testprint5_LDADD = \
	$(print_libs)

testprint6_SOURCES = testprint6.c
testprint6_LDFLAGS =
testprint6_DEPENDENCIES = $(DEPS)
testprint6_LDADD = \
	$(print_libs)

testfax_SOURCES = testfax.c
testfax_LDFLAGS =
testfax_DEPENDENCIES = $(DEPS)
testfax_LDADD = \
	$(print_libs)

testpdf_SOURCES = testpdf.c
testpdf_LDFLAGS =
testpdf_DEPENDENCIES = $(DEPS)
testpdf_LDADD = \
	$(print_libs)

testmetrics_SOURCES = testmetrics.c
testmetrics_LDFLAGS =
testmetrics_DEPENDENCIES = $(DEPS)
testmetrics_LDADD = \
	$(print_libs)

testembed_SOURCES = testembed.c
testembed_LDFLAGS =
testembed_DEPENDENCIES = $(DEPS)
testembed_LDADD = \
	$(print_libs)

testzip_SOURCES = testzip.c
testzip_LDFLAGS =
testzip_DEPENDENCIES = $(DEPS)
testzip_LDADD = \
	$(print_libs)

testtext_SOURCES = testtext.c
testtext_LDFLAGS =
testtext_DEPENDENCIES = $(DEPS)
testtext_LDADD = 	\
	$(print_libs)
