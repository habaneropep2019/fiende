lib_LTLIBRARIES = libgpa.la

libgpaincludedir = $(includedir)/gnome-1.0/libgpa

INCLUDES = 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 		\
	-DGPA_DATA_DIR=\""$(datadir)/libgpa/data/"\"		\
	-I$(includedir)  		\
	-I$(top_srcdir)		\
	-I$(srcdir)			\
	$(EXTRA_GNOME_CFLAGS)	\
	@LIBGPA_INCLUDEDIR@

libgpa_la_SOURCES = \
		gpa-backend.c		\
		gpa-code.c		\
		gpa-constraints.c	\
		gpa-include.c		\
		gpa-generic-ps.c	\
		gpa-model.c		\
		gpa-model-info.c	\
		gpa-option.c		\
		gpa-options.c		\
		gpa-printer.c		\
		gpa-settings.c		\
		gpa-utils.c		\
		gpa-values.c		\
		gpa-vendor.c		\
		text-utils.c		\
		xml-utils.c		\
						  \
		gpa-ppd.c			  \
		gpa-ppd-backend.c	  \
		gpa-ppd-code.c		  \
		gpa-ppd-constraints.c \
		gpa-ppd-div.c		  \
		gpa-ppd-model.c	  \
		gpa-ppd-option.c	  \
		gpa-ppd-options.c	  \
		gpa-ppd-paper.c	  \
		gpa-ppd-utils.c	  \
		gpa-ppd-settings.c	  \
						  \
		gpa-backend.h			\
		gpa-backend-private.h	\
		gpa-code.h			\
		gpa-constraints.h		\
		gpa-model.h			\
		gpa-model-private.h 	\
		gpa-model-info.h		\
		gpa-model-info-private.h	\
		gpa-option.h			\
		gpa-option-private.h	\
		gpa-options.h			\
		gpa-options-private.h	\
		gpa-printer.h			\
		gpa-printer-private.h	\
		gpa-settings.h			\
		gpa-settings-private.h	\
		gpa-vendor.h			\
		gpa-vendor-private.h	\
							\
		gpa-defs.h			\
		gpa-enums.h			\
		gpa-i18n.h			\
		gpa-include.h			\
		gpa-generic-ps.h         \
		gpa-known.h			\
		gpa-option.h			\
		gpa-options.h			\
							\
		gpa-ppd-backend.h		\
		gpa-ppd-code.h			\
		gpa-ppd-constraints.h	\
		gpa-ppd-div.h			\
		gpa-ppd-model.h		\
		gpa-ppd-option.h		\
		gpa-ppd-options.h		\
		gpa-ppd-paper.h		\
		gpa-ppd-settings.h		\
		gpa-ppd-utils.h		\
		gpa-ppd.h				\
							\
		gpa-private.h			\
		gpa-utils.h			\
		gpa-values.h			\
		text-utils.h			\
		xml-utils.h			


libgpainclude_HEADERS =				\
			libgpa.h				\
								\
			gpa-backend.h			\
			gpa-code.h			\
			gpa-constraints.h		\
			gpa-model.h			\
			gpa-model-info.h		\
			gpa-option.h			\
			gpa-options.h			\
			gpa-generic-ps.h         \
			gpa-printer.h			\
			gpa-ppd.h				\
			gpa-settings.h			\
			gpa-structs.h			\
			gpa-tags.h			\
			gpa-vendor.h	

libgpa_la_LDFLAGS = -version-info $(GPA_CURRENT):$(GPA_REVISION):$(GPA_AGE)


EXTRA_DIST = libgpaConf.sh.in

confexecdir=$(libdir)
confexec_DATA = libgpaConf.sh

libgpaConf.sh: libgpaConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's?\@LIBGPA_LIBDIR\@?$(LIBGPA_LIBDIR)?g' \
	    -e 's?\@LIBGPA_INCLUDEDIR\@?$(LIBGPA_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?0.0.$(GPA_CURRENT)?g' \
	    -e 's?\@LIBGPA_LIBS\@?$(LIBGPA_LIBS)?g' \
	      < $(srcdir)/libgpaConf.sh.in > libgpaConf.tmp \
	  && mv libgpaConf.tmp libgpaConf.sh
