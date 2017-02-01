SUBDIRS = tests

# fileconvert.test \

TESTS = \
	metadata.test \
	parse-path.test \
	skel.test \
	split.test \
	trigs.test

noinst_SCRIPTS = $(TESTS)

@MAINT@%.test: 
@MAINT@	echo "the_test=$*" > $(srcdir)/$@; \
@MAINT@	echo '. $$srcdir/runtest.sh' >> $(srcdir)/$@; \
@MAINT@	chmod +x $(srcdir)/$@

# Always run via sh so we don't have to worry about permissions
TESTS_ENVIRONMENT = $(SHELL) 

EXTRA_DIST =					\
	runtest.sh				\
	expected/fileconvert.out		\
	expected/skel.out			\
	expected/split.out			\
	expected/trigs.out			\
	expected/metadata.out			\
	expected/parse-path.out data/sample	\
	$(TESTS)
