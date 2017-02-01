#if GTKMM_GENDOCS
#DOCSDEF = docs
#else
#DOCSDEF =
#endif

EXTRA_DIST = \
pce2 		\
pce2.txt		\
README		\
class.plg		\
output.plg		\
cross.plg		\
desc.plg		\
init.plg		\
input.plg               \
namespace.plg 		


#all-local: copystuffs docsauto
#
#docsauto: copystuffs
#	if test x${DOCSDEF} = xdocs ; then { \
#	echo "*** Generating Documentation ***" ; \
#	rm -f ../docs/*.html ../docs/*.3 ; \
#	perl $(srcdir)/perceps -b -d ../docs/ -t $(srcdir) headers/*.h ; \
#	echo "*** Documentation generated ***" ; };\
#	fi ; \
#	touch docsauto
#
#copystuffs:
#	if test "x$(srcdir)" != "x." ; then { \
#	cp $(srcdir)/html/filters/*.flt ./html/filters/ ; \
#	cp $(srcdir)/man/filters/*.flt ./man/filters/ ; }; \
#	fi ; \
#	touch copystuffs
#
#cleanupdocgen:
#	rm -f copystuffs docsauto
#
#docs: cleanupdocgen all
#	echo "*** Generating Documentation ***" ; \
#	rm -f ../docs/*.html ../docs/*.3 ; \
#	perl $(srcdir)/perceps -b -d ../docs/ -t $(srcdir) headers/*.h ; \
#	echo "*** Documentation generated ***"
