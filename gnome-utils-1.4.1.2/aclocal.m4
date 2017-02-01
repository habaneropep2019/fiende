dnl aclocal.m4 generated automatically by aclocal 1.4-p5

dnl Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.


AC_DEFUN(XML_I18N_TOOLS_NEWER_THAN_0_9,[ true ])

dnl AC_PROG_XML_I18N_TOOLS([MINIMUM-VERSION [,"G2" if always using --utf8] ])
# serial 1 AC_PROG_XML_I18N_TOOLS
AC_DEFUN(AC_PROG_XML_I18N_TOOLS,
[
  AC_DEFUN(X18T_PFORG1,  dnl and -u for G2
        ifelse([$2],[G2],[ -u ], [ -p ]))
  AC_DEFUN(X18T_XML_KIND, 
        ifelse([$2],[G2],[ -u ],[ $(XML_I18N_XML_KIND) ]))
  AC_DEFUN(X18T_KEYS_KIND, 
        ifelse([$2],[G2],[ -u ],[ $(XML_I18N_KEYS_KIND) ]))

  XML_I18N_MERGE_DESKTOP_RULE='%.desktop:   %.desktop.in   $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -d X18T_PFORG1'
XML_I18N_MERGE_DIRECTORY_RULE='%.directory: %.directory.in $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -d X18T_PFORG1'
     XML_I18N_MERGE_KEYS_RULE='%.keys:      %.keys.in      $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -k X18T_KEYS_KIND'
      XML_I18N_MERGE_OAF_RULE='%.oaf:       %.oaf.in       $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -o -p'
     XML_I18N_MERGE_PONG_RULE='%.pong:      %.pong.in      $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -x X18T_PFORG1'
   XML_I18N_MERGE_SERVER_RULE='%.server:    %.server.in    $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -o -u'
    XML_I18N_MERGE_SHEET_RULE='%.sheet:     %.sheet.in     $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -x -u'
XML_I18N_MERGE_SOUNDLIST_RULE='%.soundlist: %.soundlist.in $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -d X18T_PFORG1'
      XML_I18N_MERGE_XML_RULE='%.xml:       %.xml.in       $(top_builddir)/xml-i18n-merge $(wildcard $(top_srcdir)/po/*.po) ; $(top_builddir)/xml-i18n-merge $(top_srcdir)/po $< [$]@ -x X18T_XML_KIND'

AC_SUBST(XML_I18N_MERGE_DESKTOP_RULE)
AC_SUBST(XML_I18N_MERGE_DIRECTORY_RULE)
AC_SUBST(XML_I18N_MERGE_KEYS_RULE)
AC_SUBST(XML_I18N_MERGE_OAF_RULE)
AC_SUBST(XML_I18N_MERGE_PONG_RULE)
AC_SUBST(XML_I18N_MERGE_SERVER_RULE)
AC_SUBST(XML_I18N_MERGE_SHEET_RULE)
AC_SUBST(XML_I18N_MERGE_SOUNDLIST_RULE)
AC_SUBST(XML_I18N_MERGE_XML_RULE)

# Use the tools built into the package, not the ones that are installed.

XML_I18N_EXTRACT='$(top_builddir)/xml-i18n-extract'
AC_SUBST(XML_I18N_EXTRACT)dnl

XML_I18N_MERGE='$(top_builddir)/xml-i18n-merge'
AC_SUBST(XML_I18N_MERGE)dnl

XML_I18N_UPDATE='$(top_builddir)/xml-i18n-update'
AC_SUBST(XML_I18N_UPDATE)dnl

AC_PATH_PROG(INTLTOOL_PERL, perl)
if test -z "$INTLTOOL_PERL"; then
   AC_MSG_ERROR([perl not found; required for xml-i18n-tools])
fi
if test -z "`$INTLTOOL_PERL -v | fgrep '5.' 2> /dev/null`"; then
   AC_MSG_ERROR([perl 5.x required for xml-i18n-tools])
fi

dnl  manually sed perl in so people don't have to put the xml-i18n-tools scripts in their 
dnl  AC_OUTPUT
AC_OUTPUT_COMMANDS([
sed -e "s:@INTLTOOL_PERL@:${INTLTOOL_PERL}:;" < ${srcdir}/xml-i18n-extract.in > xml-i18n-extract.out
if cmp -s xml-i18n-extract xml-i18n-extract.out 2>/dev/null; then
  rm -f xml-i18n-extract.out
else
  mv -f xml-i18n-extract.out xml-i18n-extract
fi
chmod ugo+x xml-i18n-extract
chmod u+w xml-i18n-extract

sed -e "s:@INTLTOOL_PERL@:${INTLTOOL_PERL}:;" < ${srcdir}/xml-i18n-merge.in > xml-i18n-merge.out
if cmp -s xml-i18n-merge xml-i18n-merge.out 2>/dev/null; then
  rm -f xml-i18n-merge.out
else
  mv -f xml-i18n-merge.out xml-i18n-merge
fi
chmod ugo+x xml-i18n-merge
chmod u+w xml-i18n-merge

sed -e "s:@INTLTOOL_PERL@:${INTLTOOL_PERL}:;" < ${srcdir}/xml-i18n-update.in > xml-i18n-update.out
if cmp -s xml-i18n-update xml-i18n-update.out 2>/dev/null; then
  rm -f xml-i18n-update.out
else
  mv -f xml-i18n-update.out xml-i18n-update
fi
chmod ugo+x xml-i18n-update
chmod u+w xml-i18n-update
], INTLTOOL_PERL=${INTLTOOL_PERL})

# Redirect the config.log output again, so that the ltconfig log is not
# clobbered by the next message.
exec 5>>./config.log
])

dnl old names
AC_DEFUN(AM_PROG_XML_I18N_TOOLS, [indir([AC_PROG_XML_I18N_TOOLS])])dnl

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN([AM_CONFIG_HEADER],
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN([AM_INIT_AUTOMAKE],
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN([AM_SANITY_CHECK],
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN([AM_MISSING_PROG],
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Add --enable-maintainer-mode option to configure.
# From Jim Meyering

# serial 1

AC_DEFUN([AM_MAINTAINER_MODE],
[AC_MSG_CHECKING([whether to enable maintainer-specific portions of Makefiles])
  dnl maintainer-mode is disabled by default
  AC_ARG_ENABLE(maintainer-mode,
[  --enable-maintainer-mode enable make rules and dependencies not useful
                          (and sometimes confusing) to the casual installer],
      USE_MAINTAINER_MODE=$enableval,
      USE_MAINTAINER_MODE=no)
  AC_MSG_RESULT($USE_MAINTAINER_MODE)
  AM_CONDITIONAL(MAINTAINER_MODE, test $USE_MAINTAINER_MODE = yes)
  MAINT=$MAINTAINER_MODE_TRUE
  AC_SUBST(MAINT)dnl
]
)

# Define a conditional.

AC_DEFUN([AM_CONDITIONAL],
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

# aclocal-include.m4
# 
# This macro adds the name macrodir to the set of directories
# that `aclocal' searches for macros.  

# serial 1

dnl AM_ACLOCAL_INCLUDE(macrodir)
AC_DEFUN([AM_ACLOCAL_INCLUDE],
[
	AM_CONDITIONAL(INSIDE_GNOME_COMMON, test x = y)

	test -n "$ACLOCAL_FLAGS" && ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

	for k in $1 ; do ACLOCAL="$ACLOCAL -I $k" ; done
])

dnl
dnl GNOME_INIT_HOOK (script-if-gnome-enabled, [failflag], [additional-inits])
dnl
dnl if failflag is "fail" then GNOME_INIT_HOOK will abort if gnomeConf.sh
dnl is not found. 
dnl

AC_DEFUN([GNOME_INIT_HOOK],[
	AC_SUBST(GNOME_LIBS)
	AC_SUBST(GNOMEUI_LIBS)
	AC_SUBST(GNOMEGNORBA_LIBS)
	AC_SUBST(GTKXMHTML_LIBS)
	AC_SUBST(ZVT_LIBS)
	AC_SUBST(GNOME_LIBDIR)
	AC_SUBST(GNOME_INCLUDEDIR)

	AC_ARG_WITH(gnome-includes,
	[  --with-gnome-includes   Specify location of GNOME headers],[
	CFLAGS="$CFLAGS -I$withval"
	])
	
	AC_ARG_WITH(gnome-libs,
	[  --with-gnome-libs       Specify location of GNOME libs],[
	LDFLAGS="$LDFLAGS -L$withval"
	gnome_prefix=$withval
	])

	AC_ARG_WITH(gnome,
	[  --with-gnome            Specify prefix for GNOME files],
		if test x$withval = xyes; then
	    		want_gnome=yes
	    		dnl Note that an empty true branch is not
			dnl valid sh syntax.
	    		ifelse([$1], [], :, [$1])
        	else
	    		if test "x$withval" = xno; then
	        		want_gnome=no
	    		else
	        		want_gnome=yes
	    			LDFLAGS="$LDFLAGS -L$withval/lib"
	    			CFLAGS="$CFLAGS -I$withval/include"
	    			gnome_prefix=$withval/lib
	    		fi
  		fi,
		want_gnome=yes)

	if test "x$want_gnome" = xyes; then

	    AC_PATH_PROG(GNOME_CONFIG,gnome-config,no)
	    if test "$GNOME_CONFIG" = "no"; then
	      no_gnome_config="yes"
	    else
	      AC_MSG_CHECKING(if $GNOME_CONFIG works)
	      if $GNOME_CONFIG --libs-only-l gnome >/dev/null 2>&1; then
	        AC_MSG_RESULT(yes)
	        GNOME_GNORBA_HOOK([],$2)
	        GNOME_LIBS="`$GNOME_CONFIG --libs-only-l gnome`"
	        GNOMEUI_LIBS="`$GNOME_CONFIG --libs-only-l gnomeui`"
	        GNOMEGNORBA_LIBS="`$GNOME_CONFIG --libs-only-l gnorba gnomeui`"
	        GTKXMHTML_LIBS="`$GNOME_CONFIG --libs-only-l gtkxmhtml`"
		ZVT_LIBS="`$GNOME_CONFIG --libs-only-l zvt`"
	        GNOME_LIBDIR="`$GNOME_CONFIG --libs-only-L gnorba gnomeui`"
	        GNOME_INCLUDEDIR="`$GNOME_CONFIG --cflags gnorba gnomeui`"
                $1
	      else
	        AC_MSG_RESULT(no)
	        no_gnome_config="yes"
              fi
            fi

	    if test x$exec_prefix = xNONE; then
	        if test x$prefix = xNONE; then
		    gnome_prefix=$ac_default_prefix/lib
	        else
 		    gnome_prefix=$prefix/lib
	        fi
	    else
	        gnome_prefix=`eval echo \`echo $libdir\``
	    fi
	
	    if test "$no_gnome_config" = "yes"; then
              AC_MSG_CHECKING(for gnomeConf.sh file in $gnome_prefix)
	      if test -f $gnome_prefix/gnomeConf.sh; then
	        AC_MSG_RESULT(found)
	        echo "loading gnome configuration from" \
		     "$gnome_prefix/gnomeConf.sh"
	        . $gnome_prefix/gnomeConf.sh
	        $1
	      else
	        AC_MSG_RESULT(not found)
 	        if test x$2 = xfail; then
	          AC_MSG_ERROR(Could not find the gnomeConf.sh file that is generated by gnome-libs install)
 	        fi
	      fi
            fi
	fi

	if test -n "$3"; then
	  n="$3"
	  for i in $n; do
	    AC_MSG_CHECKING(extra library \"$i\")
	    case $i in 
	      applets)
		AC_SUBST(GNOME_APPLETS_LIBS)
		GNOME_APPLETS_LIBS=`$GNOME_CONFIG --libs-only-l applets`
		AC_MSG_RESULT($GNOME_APPLETS_LIBS);;
	      docklets)
		AC_SUBST(GNOME_DOCKLETS_LIBS)
		GNOME_DOCKLETS_LIBS=`$GNOME_CONFIG --libs-only-l docklets`
		AC_MSG_RESULT($GNOME_DOCKLETS_LIBS);;
	      capplet)
		AC_SUBST(GNOME_CAPPLET_LIBS)
		GNOME_CAPPLET_LIBS=`$GNOME_CONFIG --libs-only-l capplet`
		AC_MSG_RESULT($GNOME_CAPPLET_LIBS);;
	      *)
		AC_MSG_RESULT(unknown library)
	    esac
            EXTRA_INCLUDEDIR=`$GNOME_CONFIG --cflags $i`
            GNOME_INCLUDEDIR="$GNOME_INCLUDEDIR $EXTRA_INCLUDEDIR"
	  done
	fi
])

dnl
dnl GNOME_INIT ([additional-inits])
dnl

AC_DEFUN([GNOME_INIT],[
	GNOME_INIT_HOOK([],fail,$1)
])

dnl
dnl GNOME_GNORBA_HOOK (script-if-gnorba-found, failflag)
dnl
dnl if failflag is "failure" it aborts if gnorba is not found.
dnl

AC_DEFUN([GNOME_GNORBA_HOOK],[
	GNOME_ORBIT_HOOK([],$2)
	AC_CACHE_CHECK([for gnorba libraries],gnome_cv_gnorba_found,[
		gnome_cv_gnorba_found=no
		if test x$gnome_cv_orbit_found = xyes; then
			GNORBA_CFLAGS="`gnome-config --cflags gnorba gnomeui`"
			GNORBA_LIBS="`gnome-config --libs gnorba gnomeui`"
			if test -n "$GNORBA_LIBS"; then
				gnome_cv_gnorba_found=yes
			fi
		fi
	])
	AM_CONDITIONAL(HAVE_GNORBA, test x$gnome_cv_gnorba_found = xyes)
	if test x$gnome_cv_orbit_found = xyes; then
		$1
		GNORBA_CFLAGS="`gnome-config --cflags gnorba gnomeui`"
		GNORBA_LIBS="`gnome-config --libs gnorba gnomeui`"
		AC_SUBST(GNORBA_CFLAGS)
		AC_SUBST(GNORBA_LIBS)
	else
	    	if test x$2 = xfailure; then
			AC_MSG_ERROR(gnorba library not installed or installation problem)
	    	fi
	fi
])

AC_DEFUN([GNOME_GNORBA_CHECK], [
	GNOME_GNORBA_HOOK([],failure)
])

dnl
dnl GNOME_ORBIT_HOOK (script-if-orbit-found, failflag)
dnl
dnl if failflag is "failure" it aborts if orbit is not found.
dnl

AC_DEFUN([GNOME_ORBIT_HOOK],[
	AC_PATH_PROG(ORBIT_CONFIG,orbit-config,no)
	AC_PATH_PROG(ORBIT_IDL,orbit-idl,no)
	AC_CACHE_CHECK([for working ORBit environment],gnome_cv_orbit_found,[
		if test x$ORBIT_CONFIG = xno -o x$ORBIT_IDL = xno; then
			gnome_cv_orbit_found=no
		else
			gnome_cv_orbit_found=yes
		fi
	])
	AM_CONDITIONAL(HAVE_ORBIT, test x$gnome_cv_orbit_found = xyes)
	if test x$gnome_cv_orbit_found = xyes; then
		$1
		ORBIT_CFLAGS=`orbit-config --cflags client server`
		ORBIT_LIBS=`orbit-config --use-service=name --libs client server`
		AC_SUBST(ORBIT_CFLAGS)
		AC_SUBST(ORBIT_LIBS)
	else
    		if test x$2 = xfailure; then
			AC_MSG_ERROR(ORBit not installed or installation problem)
    		fi
	fi
])

AC_DEFUN([GNOME_ORBIT_CHECK], [
	GNOME_ORBIT_HOOK([],failure)
])

#serial 1
# This test replaces the one in autoconf.
# Currently this macro should have the same name as the autoconf macro
# because gettext's gettext.m4 (distributed in the automake package)
# still uses it.  Otherwise, the use in gettext.m4 makes autoheader
# give these diagnostics:
#   configure.in:556: AC_TRY_COMPILE was called before AC_ISC_POSIX
#   configure.in:556: AC_TRY_RUN was called before AC_ISC_POSIX

undefine([AC_ISC_POSIX])

AC_DEFUN([AC_ISC_POSIX],
  [
    dnl This test replaces the obsolescent AC_ISC_POSIX kludge.
    AC_CHECK_LIB(cposix, strerror, [LIBS="$LIBS -lcposix"])
  ]
)


dnl AM_PROG_LEX
dnl Look for flex, lex or missing, then run AC_PROG_LEX and AC_DECL_YYTEXT
AC_DEFUN([AM_PROG_LEX],
[missing_dir=ifelse([$1],,`cd $ac_aux_dir && pwd`,$1)
AC_CHECK_PROGS(LEX, flex lex, "$missing_dir/missing flex")
AC_PROG_LEX
AC_DECL_YYTEXT])

dnl GNOME_COMPILE_WARNINGS
dnl Turn on many useful compiler warnings
dnl For now, only works on GCC
AC_DEFUN([GNOME_COMPILE_WARNINGS],[
  AC_ARG_ENABLE(compile-warnings, 
    [  --enable-compile-warnings=[no/minimum/yes]	Turn on compiler warnings.],,enable_compile_warnings=minimum)

  AC_MSG_CHECKING(what warning flags to pass to the C compiler)
  warnCFLAGS=
  if test "x$GCC" != xyes; then
    enable_compile_warnings=no
  fi

  if test "x$enable_compile_warnings" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCFLAGS="-Wall -Wunused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_compile_warnings" = "xyes"; then
	warnCFLAGS="$warnCFLAGS -Wmissing-prototypes -Wmissing-declarations"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCFLAGS)

  AC_ARG_ENABLE(iso-c,
    [  --enable-iso-c          Try to warn if code is not ISO C ],,
    enable_iso_c=no)

  AC_MSG_CHECKING(what language compliance flags to pass to the C compiler)
  complCFLAGS=
  if test "x$enable_iso_c" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -ansi" ;;
      esac

      case " $CFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -pedantic" ;;
      esac
    fi
  fi
  AC_MSG_RESULT($complCFLAGS)
  if test "x$cflags_set" != "xyes"; then
    CFLAGS="$CFLAGS $warnCFLAGS $complCFLAGS"
    cflags_set=yes
    AC_SUBST(cflags_set)
  fi
])

dnl For C++, do basically the same thing.

AC_DEFUN([GNOME_CXX_WARNINGS],[
  AC_ARG_ENABLE(cxx-warnings, 
    [  --enable-cxx-warnings=[no/minimum/yes]	Turn on compiler warnings.],,enable_cxx_warnings=minimum)

  AC_MSG_CHECKING(what warning flags to pass to the C++ compiler)
  warnCXXFLAGS=
  if test "x$GCC" != xyes; then
    enable_compile_warnings=no
  fi
  if test "x$enable_cxx_warnings" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCXXFLAGS="-Wall -Wno-unused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_cxx_warnings" = "xyes"; then
	warnCXXFLAGS="$warnCXXFLAGS -Wmissing-prototypes -Wmissing-declarations -Wshadow -Woverloaded-virtual"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCXXFLAGS)

   AC_ARG_ENABLE(iso-cxx,
     [  --enable-iso-cxx          Try to warn if code is not ISO C++ ],,
     enable_iso_cxx=no)

   AC_MSG_CHECKING(what language compliance flags to pass to the C++ compiler)
   complCXXFLAGS=
   if test "x$enable_iso_cxx" != "xno"; then
     if test "x$GCC" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -ansi" ;;
      esac

      case " $CXXFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -pedantic" ;;
      esac
     fi
   fi
  AC_MSG_RESULT($complCXXFLAGS)
  if test "x$cxxflags_set" != "xyes"; then
    CXXFLAGS="$CXXFLAGS $warnCXXFLAGS $complCXXFLAGS"
    cxxflags_set=yes
    AC_SUBST(cxxflags_set)
  fi
])

dnl GNOME_X_CHECKS
dnl
dnl Basic X11 related checks for X11.  At the end, the following will be
dnl defined/changed:
dnl   GTK_{CFLAGS,LIBS}      From AM_PATH_GTK
dnl   CPPFLAGS		     Will include $X_CFLAGS
dnl   GNOME_HAVE_SM	     `true' or `false' depending on whether session
dnl                          management is available.  It is available if
dnl                          both -lSM and X11/SM/SMlib.h exist.  (Some
dnl                          Solaris boxes have the library but not the header)
dnl   XPM_LIBS               -lXpm if Xpm library is present, otherwise ""
dnl
dnl The following configure cache variables are defined (but not used):
dnl   gnome_cv_passdown_{x_libs,X_LIBS,X_CFLAGS}
dnl
AC_DEFUN([GNOME_X_CHECKS],
[
	AM_PATH_GTK(1.2.0,,AC_MSG_ERROR(GTK not installed, or gtk-config not in path))
	dnl Hope that GTK_CFLAGS have only -I and -D.  Otherwise, we could
	dnl   test -z "$x_includes" || CPPFLAGS="$CPPFLAGS -I$x_includes"
	dnl
	dnl Use CPPFLAGS instead of CFLAGS because AC_CHECK_HEADERS uses
	dnl CPPFLAGS, not CFLAGS
        CPPFLAGS="$CPPFLAGS $GTK_CFLAGS"

        saved_ldflags="$LDFLAGS"
        LDFLAGS="$LDFLAGS $GTK_LIBS"

	gnome_cv_passdown_x_libs="$GTK_LIBS"
	gnome_cv_passdown_X_LIBS="$GTK_LIBS"
	gnome_cv_passdown_X_CFLAGS="$GTK_CFLAGS"
	gnome_cv_passdown_GTK_LIBS="$GTK_LIBS"

        LDFLAGS="$saved_ldflags $GTK_LIBS"

dnl We are requiring GTK >= 1.1.1, which means this will be fine anyhow.
	USE_DEVGTK=true

dnl	AC_MSG_CHECKING([whether to use features from (unstable) GTK+ 1.1.x])
dnl	AC_EGREP_CPP(answer_affirmatively,
dnl	[#include <gtk/gtkfeatures.h>
dnl	#ifdef GTK_HAVE_FEATURES_1_1_0
dnl	   answer_affirmatively
dnl	#endif
dnl	], dev_gtk=yes, dev_gtk=no)
dnl	if test "$dev_gtk" = "yes"; then
dnl	   USE_DEVGTK=true
dnl	fi
dnl	AC_MSG_RESULT("$dev_gtk")

	GNOME_HAVE_SM=true
	case "$GTK_LIBS" in
	 *-lSM*)
	    dnl Already found it.
	    ;;
	 *)
	    dnl Assume that if we have -lSM then we also have -lICE.
	    AC_CHECK_LIB(SM, SmcSaveYourselfDone,
	        [GTK_LIBS="-lSM -lICE $GTK_LIBS"],GNOME_HAVE_SM=false,
		$x_libs -lICE)
	    ;;
	esac

	if test "$GNOME_HAVE_SM" = true; then
	   AC_CHECK_HEADERS(X11/SM/SMlib.h,,GNOME_HAVE_SM=false)
	fi

	if test "$GNOME_HAVE_SM" = true; then
	   AC_DEFINE(HAVE_LIBSM)
	fi

	XPM_LIBS=""
	AC_CHECK_LIB(Xpm, XpmFreeXpmImage, [XPM_LIBS="-lXpm"], , $x_libs)
	AC_SUBST(XPM_LIBS)

	AC_REQUIRE([GNOME_PTHREAD_CHECK])
        LDFLAGS="$saved_ldflags"

	AC_PROVIDE([GNOME_X_CHECKS])
])

# Configure paths for GTK+
# Owen Taylor     97-11-3

dnl AM_PATH_GTK([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GTK, and define GTK_CFLAGS and GTK_LIBS
dnl
AC_DEFUN(AM_PATH_GTK,
[dnl 
dnl Get the cflags and libraries from the gtk-config script
dnl
AC_ARG_WITH(gtk-prefix,[  --with-gtk-prefix=PFX   Prefix where GTK is installed (optional)],
            gtk_config_prefix="$withval", gtk_config_prefix="")
AC_ARG_WITH(gtk-exec-prefix,[  --with-gtk-exec-prefix=PFX Exec prefix where GTK is installed (optional)],
            gtk_config_exec_prefix="$withval", gtk_config_exec_prefix="")
AC_ARG_ENABLE(gtktest, [  --disable-gtktest       Do not try to compile and run a test GTK program],
		    , enable_gtktest=yes)

  for module in . $4
  do
      case "$module" in
         gthread) 
             gtk_config_args="$gtk_config_args gthread"
         ;;
      esac
  done

  if test x$gtk_config_exec_prefix != x ; then
     gtk_config_args="$gtk_config_args --exec-prefix=$gtk_config_exec_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_exec_prefix/bin/gtk-config
     fi
  fi
  if test x$gtk_config_prefix != x ; then
     gtk_config_args="$gtk_config_args --prefix=$gtk_config_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_prefix/bin/gtk-config
     fi
  fi

  AC_PATH_PROG(GTK_CONFIG, gtk-config, no)
  min_gtk_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for GTK - version >= $min_gtk_version)
  no_gtk=""
  if test "$GTK_CONFIG" = "no" ; then
    no_gtk=yes
  else
    GTK_CFLAGS=`$GTK_CONFIG $gtk_config_args --cflags`
    GTK_LIBS=`$GTK_CONFIG $gtk_config_args --libs`
    gtk_config_major_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtk_config_minor_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtk_config_micro_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtktest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GTK_CFLAGS"
      LIBS="$GTK_LIBS $LIBS"
dnl
dnl Now check if the installed GTK is sufficiently new. (Also sanity
dnl checks the results of gtk-config to some extent
dnl
      rm -f conf.gtktest
      AC_TRY_RUN([
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtktest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtk_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtk_version");
     exit(1);
   }

  if ((gtk_major_version != $gtk_config_major_version) ||
      (gtk_minor_version != $gtk_config_minor_version) ||
      (gtk_micro_version != $gtk_config_micro_version))
    {
      printf("\n*** 'gtk-config --version' returned %d.%d.%d, but GTK+ (%d.%d.%d)\n", 
             $gtk_config_major_version, $gtk_config_minor_version, $gtk_config_micro_version,
             gtk_major_version, gtk_minor_version, gtk_micro_version);
      printf ("*** was found! If gtk-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK+. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtk-config was wrong, set the environment variable GTK_CONFIG\n");
      printf("*** to point to the correct copy of gtk-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (GTK_MAJOR_VERSION) && defined (GTK_MINOR_VERSION) && defined (GTK_MICRO_VERSION)
  else if ((gtk_major_version != GTK_MAJOR_VERSION) ||
	   (gtk_minor_version != GTK_MINOR_VERSION) ||
           (gtk_micro_version != GTK_MICRO_VERSION))
    {
      printf("*** GTK+ header files (version %d.%d.%d) do not match\n",
	     GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtk_major_version, gtk_minor_version, gtk_micro_version);
    }
#endif /* defined (GTK_MAJOR_VERSION) ... */
  else
    {
      if ((gtk_major_version > major) ||
        ((gtk_major_version == major) && (gtk_minor_version > minor)) ||
        ((gtk_major_version == major) && (gtk_minor_version == minor) && (gtk_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK+ (%d.%d.%d) was found.\n",
               gtk_major_version, gtk_minor_version, gtk_micro_version);
        printf("*** You need a version of GTK+ newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK+ is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtk-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK+, but you can also set the GTK_CONFIG environment to point to the\n");
        printf("*** correct copy of gtk-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtk=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtk" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GTK_CONFIG" = "no" ; then
       echo "*** The gtk-config script installed by GTK could not be found"
       echo "*** If GTK was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTK_CONFIG environment variable to the"
       echo "*** full path to gtk-config."
     else
       if test -f conf.gtktest ; then
        :
       else
          echo "*** Could not run GTK test program, checking why..."
          CFLAGS="$CFLAGS $GTK_CFLAGS"
          LIBS="$LIBS $GTK_LIBS"
          AC_TRY_LINK([
#include <gtk/gtk.h>
#include <stdio.h>
],      [ return ((gtk_major_version) || (gtk_minor_version) || (gtk_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK or finding the wrong"
          echo "*** version of GTK. If it is not finding GTK, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the GTK package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps gtk gtk-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK was incorrectly installed"
          echo "*** or that you have moved GTK since it was installed. In the latter case, you"
          echo "*** may want to edit the gtk-config script: $GTK_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTK_CFLAGS=""
     GTK_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
  rm -f conf.gtktest
])

dnl
dnl And better, use gthreads instead...
dnl

AC_DEFUN([GNOME_PTHREAD_CHECK],[
	PTHREAD_LIB=""
	AC_CHECK_LIB(pthread, pthread_create, PTHREAD_LIB="-lpthread",
		[AC_CHECK_LIB(pthreads, pthread_create, PTHREAD_LIB="-lpthreads",
		    [AC_CHECK_LIB(c_r, pthread_create, PTHREAD_LIB="-lc_r",
			[AC_CHECK_LIB(pthread, __pthread_attr_init_system, PTHREAD_LIB="-lpthread",
				[AC_CHECK_FUNC(pthread_create)]
			)]
		    )]
		)]
	)
	AC_SUBST(PTHREAD_LIB)
	AC_PROVIDE([GNOME_PTHREAD_CHECK])
])

AC_DEFUN([GNOME_CHECK_OBJC],
[
dnl Look for an ObjC compiler.
dnl FIXME: extend list of possible names of ObjC compilers.
  AC_CHECK_PROGS(OBJC, $OBJC egcs, "")
  if test "x$OBJC" = "x" ; then
    AC_CHECK_PROGS(OBJC, $OBJC egcc, "")
    if test "x$OBJC" = "x" ; then
      AC_CHECK_PROGS(OBJC, $OBJC gcc, "")
    fi
  fi

  AC_REQUIRE([GNOME_PTHREAD_CHECK])

  OBJC_LIBS="-lobjc $PTHREAD_LIB"
  AC_CHECK_FUNC(sched_yield,,[
    AC_CHECK_LIB(rt,sched_yield,
      OBJC_LIBS="$OBJC_LIBS -lrt",[
      AC_CHECK_LIB(posix4,sched_yield,
        OBJC_LIBS="$OBJC_LIBS -lposix4",, 
        $OBJC_LIBS)],
      $OBJC_LIBS)])
  AC_SUBST(OBJC_LIBS)

  AC_CACHE_CHECK([if Objective C compiler ($OBJC) works],
		 ac_cv_prog_objc_works, [
    if test -n "$OBJC"; then
      cat > conftest.m <<EOF
#include <objc/Object.h>
@interface myRandomObj : Object
{
}
@end
@implementation myRandomObj
@end
int main () {
  /* No, you are not seeing double.  Remember that square brackets
     are the autoconf m4 quotes.  */
  id myid = [[myRandomObj alloc]];
  [[myid free]];
  return 0;
}
EOF

      $OBJC $CFLAGS -o conftest $LDFLAGS conftest.m $OBJC_LIBS 1>&AC_FD_CC 2>&1
      result=$?
      rm -f conftest*

      if test $result -eq 0; then
        ac_cv_prog_objc_works=yes
      fi
    else
      ac_cv_prog_objc_works=no
    fi
  ])

  AM_CONDITIONAL(OBJECTIVE_C, test x$ac_cv_prog_objc_works = xyes)
  dnl Also set the shell variable OBJECTIVE_C to "yes" or "no".
  OBJECTIVE_C=$ac_cv_prog_objc_works
])

AC_DEFUN([GNOME_INIT_OBJC],
[
	AC_MSG_CHECKING(for an obGnomeConf.sh)
	my_gnome_libdir=`$GNOME_CONFIG --libdir`
	if test -f $my_gnome_libdir/obGnomeConf.sh; then
	    . $my_gnome_libdir/obGnomeConf.sh
	    AC_MSG_RESULT(found $my_gnome_libdir)
	    ac_cv_have_gnome_objc=yes
	else
	    AC_MSG_RESULT(not found)
	    AC_MSG_WARN(Could not find the obGnomeConf.sh file that is generated by gnome-objc install)
	    ac_cv_have_gnome_objc=no
	fi
	
	dnl Add a conditional on whether or not we have gnome-objc
	AM_CONDITIONAL(HAVE_GNOME_OBJC, test x$ac_cv_have_gnome_objc = xyes)
	HAVE_GNOME_OBJC=$ac_cv_have_gnome_objc

	AC_SUBST(OBGNOME_INCLUDEDIR)
	AC_SUBST(OBGNOME_LIBS)
	AC_SUBST(OBGTK_LIBS)
])

# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# Modified to never use included libintl. 
# Owen Taylor <otaylor@redhat.com>, 12/15/1998
#
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 5

AC_DEFUN([AM_GNOME_WITH_NLS],
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    BUILD_INCLUDED_LIBINTL=no
    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
#      AC_DEFINE(ENABLE_NLS)
#      AC_MSG_CHECKING([whether included gettext is requested])
#      AC_ARG_WITH(included-gettext,
#        [  --with-included-gettext use the GNU gettext library included here],
#        nls_cv_force_use_gnu_gettext=$withval,
#        nls_cv_force_use_gnu_gettext=no)
#      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)
      nls_cv_force_use_gnu_gettext="no"

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CACHE_CHECK([for gettext in libintl],
		 gt_cv_func_gettext_libintl,
		 [AC_CHECK_LIB(intl, gettext,
		  gt_cv_func_gettext_libintl=yes,
		  gt_cv_func_gettext_libintl=no)],
		 gt_cv_func_gettext_libintl=no)])
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$gt_cv_func_gettext_libintl" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT)
	      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		AC_CHECK_FUNCS(dcgettext)
		AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
	      fi
	    fi

	    # Added by Martin Baulig 12/15/98 for libc5 systems
	    if test "$gt_cv_func_gettext_libc" != "yes" \
	       && test "$gt_cv_func_gettext_libintl" = "yes"; then
	       INTLLIBS=-lintl
	       LIBS=`echo $LIBS | sed -e 's/-lintl//'`
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS)
	       INTLOBJS="\$(CATOBJS)"
	       AC_PATH_PROG(GENCAT, gencat, no)dnl
#	       if test "$GENCAT" != "no"; then
#		 AC_PATH_PROG(GMSGFMT, gmsgfmt, no)
#		 if test "$GMSGFMT" = "no"; then
#		   AM_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
#		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)
#		 fi
#		 AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
#		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
#		 USE_INCLUDED_LIBINTL=yes
#		 CATOBJEXT=.cat
#		 INSTOBJEXT=.cat
#		 DATADIRNAME=lib
#		 INTLDEPS='$(top_builddir)/intl/libintl.a'
#		 INTLLIBS=$INTLDEPS
#		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
#		 nls_cv_header_intl=intl/libintl.h
#		 nls_cv_header_libgt=intl/libgettext.h
#              fi
            ])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" != "yes"; then
        AC_DEFINE(ENABLE_NLS)
      else
         # Unset this variable since we use the non-zero value as a flag.
         CATOBJEXT=
#        dnl Mark actions used to generate GNU NLS library.
#        INTLOBJS="\$(GETTOBJS)"
#        AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
#	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
#        AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
#        AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
#	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
#        AC_SUBST(MSGFMT)
#	USE_INCLUDED_LIBINTL=yes
#        CATOBJEXT=.gmo
#        INSTOBJEXT=.mo
#        DATADIRNAME=share
#	INTLDEPS='$(top_builddir)/intl/libintl.a'
#	INTLLIBS=$INTLDEPS
#	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
#        nls_cv_header_intl=intl/libintl.h
#        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi

      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


#    # If this is used in GNU gettext we have to set USE_NLS to `yes'
#    # because some of the sources are only built for this goal.
#    if test "$PACKAGE" = gettext; then
#      USE_NLS=yes
#      USE_INCLUDED_LIBINTL=yes
#    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(BUILD_INCLUDED_LIBINTL)
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN([AM_GNOME_GETTEXT],
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY)
   fi

   AM_LC_MESSAGES
   AM_GNOME_WITH_NLS

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       if test "x$LINGUAS" = "x"; then
           LINGUAS=$ALL_LINGUAS
       fi
       for lang in $LINGUAS; do
         case "$ALL_LINGUAS" in
          *\ $lang\ *|$lang\ *|*\ $lang) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but ($top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   if test "x$srcdir" != "x."; then
     if test "x`echo $srcdir | sed 's@/.*@@'`" = "x"; then
       posrcprefix="$srcdir/"
     else
       posrcprefix="../$srcdir/"
     fi
   else
     posrcprefix="../"
   fi
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])


# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU General Public
# License or the GNU Library General Public License but which still want
# to provide support for the GNU gettext functionality.
# Please note that the actual code of the GNU gettext library is covered
# by the GNU Library General Public License, and the rest of the GNU
# gettext package package is covered by the GNU General Public License.
# They are *not* in the public domain.

# serial 2

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN([AM_PATH_PROG_WITH_TEST],
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test ifelse([$4], , [-n "[$]$1"], ["[$]$1" != "$4"]); then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])

# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU General Public
# License or the GNU Library General Public License but which still want
# to provide support for the GNU gettext functionality.
# Please note that the actual code of the GNU gettext library is covered
# by the GNU Library General Public License, and the rest of the GNU
# gettext package package is covered by the GNU General Public License.
# They are *not* in the public domain.

# serial 2

AC_DEFUN([AM_LC_MESSAGES],
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES, 1,
        [Define if your <locale.h> file defines LC_MESSAGES.])
    fi
  fi])

dnl
dnl GNOME_CHECK_GUILE (failflag)
dnl
dnl if failflag is "fail" then GNOME_CHECK_GUILE will abort if guile is not found.
dnl

AC_DEFUN([GNOME_CHECK_GUILE],
[
dnl	AC_MSG_WARN([Withval is: $withval])
	guile_msg = 'Huh?'
if test x$withval = xno ; then
	guile_msg = 'disabled'
	GUILE_LIBS=
	GUILE_INCS=
	AC_SUBST(GUILE_LIBS)
	AC_SUBST(GUILE_INCS)
	AM_CONDITIONAL(GUILE, /bin/false)
else
	guile_msg="no"

	saved_ldflags="$LDFLAGS"
	saved_cppflags="$CPPFLAGS"
	LDFLAGS="$LDFLAGS $GNOME_LIBDIR"

	AC_CHECK_LIB(qthreads,qt_null,[
		QTTHREADS_LIB="-lqthreads"
	],[
		AC_CHECK_LIB(qt, qt_null, QTTHREADS_LIB="-lqt")
	],$LIBS)
	AC_SUBST(QTTHREADS_LIB)

	AC_CHECK_LIB(termcap,main,TERMCAP_LIB="-ltermcap")
	AC_CHECK_LIB(readline,main,READLINE_LIB="-lreadline",,$TERMCAP_LIB)

	AC_SUBST(TERMCAP_LIB)
	AC_SUBST(READLINE_LIB)

	if test "x$cross_compiling" = "xyes" ; then
	  name_build_guile="$target_alias-guile-config"
	else
	  name_build_guile="guile-config"
	fi

	AC_CHECK_PROG(BUILD_GUILE, $name_build_guile, yes, no)

	if test "x$BUILD_GUILE" = "xyes"; then
	    AC_MSG_CHECKING(whether $name_build_guile works)
	    if test x`$name_build_guile --version >/dev/null 2>&1 || \
		echo no` = xno; then
		BUILD_GUILE=no
	    fi
	    AC_MSG_RESULT($BUILD_GUILE)
	else

	    if test "x$cross_compiling" = "xyes" ; then
		name_build_guile="$target_alias-build-guile"
	    else	
		name_build_guile="build-guile"
	    fi

	    AC_CHECK_PROG(BUILD_GUILE, $name_build_guile, yes, no)

	    if test "x$BUILD_GUILE" = "xyes"; then
		AC_MSG_CHECKING(whether $name_build_guile works)
		if test x`$name_build_guile --version >/dev/null 2>&1 || \
	 	    echo no` = xno; then
		    BUILD_GUILE=no
		fi
		AC_MSG_RESULT($BUILD_GUILE)
	    fi
	fi

	AC_CHECK_LIB(m, sin)

	if test "x$BUILD_GUILE" = "xyes"; then
		AC_MSG_CHECKING(for guile libraries)
		GUILE_LIBS="`$name_build_guile link`"
		AC_MSG_RESULT($GUILE_LIBS)
		AC_MSG_CHECKING(for guile headers)
		GUILE_INCS="`$name_build_guile compile`"
		AC_MSG_RESULT($GUILE_INCS)
	else
		GUILE_LIBS="$GNOME_LIBDIR"
		GUILE_INCS="$GNOME_INCLUDEDIR"
		AC_CHECK_LIB(rx, main, GUILE_LIBS="-lrx $GUILE_LIBS")
		AC_CHECK_LIB(qt, qt_null, GUILE_LIBS="-lqt $GUILE_LIBS")
		AC_CHECK_LIB(dl, dlopen, GUILE_LIBS="-ldl $GUILE_LIBS")
		AC_CHECK_LIB(nsl, t_accept, GUILE_LIBS="$GUILE_LIBS -lnsl")
		AC_CHECK_LIB(socket, socket, GUILE_LIBS="$GUILE_LIBS -lsocket")
		GUILE_LIBS="-lguile $GUILE_LIBS $QTTHREADS_LIB $READLINE_LIB $TERMCAP_LIB"
	fi

	AC_SUBST(GUILE_LIBS)
	AC_SUBST(GUILE_INCS)

	saved_LIBS="$LIBS"
	LIBS="$LIBS $GUILE_LIBS"
	CPPFLAGS="$saved_cppflags $GUILE_INCS"

	AC_MSG_CHECKING(whether guile works)
	AC_TRY_LINK([
		#include <libguile.h>
		#include <guile/gh.h>
	],[
		gh_eval_str("(newline)");
		scm_boot_guile(0,NULL,NULL,NULL);
	],[
		ac_cv_guile_found=yes
		AC_DEFINE(HAVE_GUILE)
	],[
		ac_cv_guile_found=no
	])
	AC_MSG_RESULT($ac_cv_guile_found)

	guile_msg=$ac_cv_guile_found

	if test x$ac_cv_guile_found = xno ; then
		if test x$1 = xfail ; then
		  AC_MSG_ERROR(Can not find Guile on this system)
		else
		  AC_MSG_WARN(Can not find Guile on this system)
		fi
		ac_cv_guile_found=no
		GUILE_LIBS= GUILE_INCS=
	fi

	LIBS="$saved_LIBS"
	LDFLAGS="$saved_ldflags"
	CPPFLAGS="$saved_cppflags"

	AC_SUBST(GUILE_LIBS)
	AM_CONDITIONAL(GUILE, test x$ac_cv_guile_found = xyes)
fi
])

# Configure paths for GNOME-PRINT
# Chris Lahey	99-2-5
# stolen from Manish Singh again
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_GNOME_PRINT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for GNOME-PRINT, and define GNOME_PRINT_CFLAGS and GNOME_PRINT_LIBS
dnl
AC_DEFUN([AM_PATH_GNOME_PRINT],
[
  min_version=ifelse([$1],,0.21,$1)

  gnome_print_ok=""

  AC_PATH_PROG(GNOME_CONFIG, gnome-config, no)
  if test "$GNOME_CONFIG" = "no" ; then
    AC_MSG_RESULT(gnome-config is missing, check your gnome installation)
  else
    AC_MSG_CHECKING(for GNOME-PRINT - version >= $min_version)
    if `$GNOME_CONFIG --libs print > /dev/null 2>&1`; then
      rqmajor=`echo "$min_version" | sed -e 's/cvs-//' | sed 's/\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
      rqminor=`echo "$min_version" | sed -e 's/cvs-//' | sed 's/\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
      major=`$GNOME_CONFIG --modversion print | sed -e 's/gnome-print-//' | sed -e 's/cvs-//' | sed 's/\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
      minor=`$GNOME_CONFIG --modversion print | sed -e 's/gnome-print-//' | sed -e 's/cvs-//' | sed 's/\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
      if test "$major" -ge "$rqmajor"; then
        if test "$major" -gt "$rqmajor"; then
          AC_MSG_RESULT("found $major.$minor")
          gnome_print_ok="yes"
        else
          if test "$minor" -ge "$rqminor"; then
            AC_MSG_RESULT("found $major.$minor")
            gnome_print_ok="yes"
          else
            AC_MSG_RESULT("you have $major.$minor")
          fi
        fi
      else
        AC_MSG_RESULT("you have $major.$minor")
      fi
    else
      AC_MSG_RESULT("did not find any version")
    fi
  fi

  if test "x$gnome_print_ok" != "x" ; then
    GNOME_PRINT_CFLAGS=`$GNOME_CONFIG --cflags print`
    GNOME_PRINT_LIBS=`$GNOME_CONFIG --libs print`
    ifelse([$2], , :, [$2])
  else
     GNOME_PRINT_CFLAGS=""
     GNOME_PRINT_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_SUBST(GNOME_PRINT_CFLAGS)
  AC_SUBST(GNOME_PRINT_LIBS)
])

AC_DEFUN([GNOME_PRINT_CHECK], [
	AM_PATH_GNOME_PRINT($1,,[AC_MSG_ERROR(GNOME-PRINT not found or wrong version)])
])

dnl
dnl GNOME_XML_HOOK (script-if-xml-found, failflag)
dnl
dnl If failflag is "failure", script aborts due to lack of XML
dnl 
dnl Check for availability of the libxml library
dnl the XML parser uses libz if available too
dnl

AC_DEFUN([GNOME_XML_HOOK],[
	AC_PATH_PROG(GNOME_CONFIG,gnome-config,no)
	if test "$GNOME_CONFIG" = no; then
		if test x$2 = xfailure; then
			AC_MSG_ERROR(Could not find gnome-config)
		fi
	fi
	GNOME_XML_CFLAGS=`$GNOME_CONFIG --cflags xml`
	AC_SUBST(GNOME_XML_CFLAGS)
	AC_CHECK_LIB(xml, xmlNewDoc, [
		$1
		GNOME_XML_LIB=`$GNOME_CONFIG --libs xml`
	], [
		if test x$2 = xfailure; then 
			AC_MSG_ERROR(Could not link sample xml program)
		fi
	], `$GNOME_CONFIG --libs xml`)
	AC_SUBST(GNOME_XML_LIB)
])

AC_DEFUN([GNOME_XML_CHECK], [
	GNOME_XML_HOOK([],failure)
])

dnl
dnl LIBGTOP_CHECK_TYPE
dnl
dnl Improved version of AC_CHECK_TYPE which takes into account
dnl that we need to #include some other header files on some
dnl systems to get some types.

dnl AC_LIBGTOP_CHECK_TYPE(TYPE, DEFAULT)
AC_DEFUN([AC_LIBGTOP_CHECK_TYPE],
[AC_REQUIRE([AC_HEADER_STDC])dnl
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(ac_cv_type_$1,
[AC_EGREP_CPP(dnl
changequote(<<,>>)dnl
<<(^|[^a-zA-Z_0-9])$1[^a-zA-Z_0-9]>>dnl
changequote([,]), [#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif

/* For Tru64 */
#ifdef HAVE_SYS_BITYPES_H
#include <sys/bitypes.h>
#endif
], ac_cv_type_$1=yes, ac_cv_type_$1=no)])dnl
AC_MSG_RESULT($ac_cv_type_$1)
if test $ac_cv_type_$1 = no; then
  AC_DEFINE($1, $2)
fi
])

dnl
dnl GNOME_LIBGTOP_TYPES
dnl
dnl some typechecks for libgtop.
dnl

AC_DEFUN([GNOME_LIBGTOP_TYPES],
[
	AC_CHECK_HEADERS(sys/bitypes.h)
	AC_LIBGTOP_CHECK_TYPE(u_int64_t, unsigned long long int)
	AC_LIBGTOP_CHECK_TYPE(int64_t, signed long long int)
])

dnl
dnl GNOME_LIBGTOP_HOOK (minversion, script-if-libgtop-enabled, failflag)
dnl
dnl if failflag is "fail" then GNOME_LIBGTOP_HOOK will abort if LibGTop
dnl is not found. 
dnl

AC_DEFUN([GNOME_LIBGTOP_HOOK],
[	
	AC_REQUIRE([GNOME_LIBGTOP_TYPES])

	AC_SUBST(LIBGTOP_LIBDIR)
	AC_SUBST(LIBGTOP_INCLUDEDIR)
	AC_SUBST(LIBGTOP_EXTRA_LIBS)
	AC_SUBST(LIBGTOP_LIBS)
	AC_SUBST(LIBGTOP_INCS)
	AC_SUBST(LIBGTOP_NAMES_LIBS)
	AC_SUBST(LIBGTOP_NAMES_INCS)
	AC_SUBST(LIBGTOP_MAJOR_VERSION)
	AC_SUBST(LIBGTOP_MINOR_VERSION)
	AC_SUBST(LIBGTOP_MICRO_VERSION)
	AC_SUBST(LIBGTOP_VERSION)
	AC_SUBST(LIBGTOP_VERSION_CODE)
	AC_SUBST(LIBGTOP_SERVER_VERSION)
	AC_SUBST(LIBGTOP_INTERFACE_AGE)
	AC_SUBST(LIBGTOP_BINARY_AGE)
	AC_SUBST(LIBGTOP_BINDIR)
	AC_SUBST(LIBGTOP_SERVER)

	dnl Get the cflags and libraries from the libgtop-config script
	dnl
	AC_ARG_WITH(libgtop,
	[  --with-libgtop=PFX      Prefix where LIBGTOP is installed (optional)],
	libgtop_config_prefix="$withval", libgtop_config_prefix="")
	AC_ARG_WITH(libgtop-exec,
	[  --with-libgtop-exec=PFX Exec prefix where LIBGTOP is installed (optional)],
	libgtop_config_exec_prefix="$withval", libgtop_config_exec_prefix="")

	if test x$libgtop_config_exec_prefix != x ; then
	  libgtop_config_args="$libgtop_config_args --exec-prefix=$libgtop_config_exec_prefix"
	  if test x${LIBGTOP_CONFIG+set} != xset ; then
	    LIBGTOP_CONFIG=$libgtop_config_exec_prefix/bin/libgtop-config
	  fi
	fi
	if test x$libgtop_config_prefix != x ; then
	  libgtop_config_args="$libgtop_config_args --prefix=$libgtop_config_prefix"
	  if test x${LIBGTOP_CONFIG+set} != xset ; then
	    LIBGTOP_CONFIG=$libgtop_config_prefix/bin/libgtop-config
	  fi
	fi

	AC_PATH_PROG(LIBGTOP_CONFIG, libgtop-config, no)
	dnl IMPORTANT NOTICE:
	dnl   If you increase this number here, this means that *ALL*
	dnl   modules will require the new version, even if they explicitly
	dnl   give a lower number in their `configure.in' !!!
	real_min_libgtop_version=1.0.0
	min_libgtop_version=ifelse([$1], ,$real_min_libgtop_version,$1)
	dnl I know, the following code looks really ugly, but if you want
	dnl to make changes, please test it with a brain-dead /bin/sh and
	dnl with a brain-dead /bin/test (not all shells/tests support the
	dnl `<' operator to compare strings, that's why I convert everything
	dnl into numbers and test them).
	min_libgtop_major=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	min_libgtop_minor=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	min_libgtop_micro=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	test x$min_libgtop_micro = x && min_libgtop_micro=0
	real_min_libgtop_major=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	real_min_libgtop_minor=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	real_min_libgtop_micro=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	test x$real_min_libgtop_micro = x && real_min_libgtop_micro=0
	dnl You cannot require a version less then $real_min_libgtop_version,
	dnl so you don't need to update each `configure.in' when it's increased.
	if test $real_min_libgtop_major -gt $min_libgtop_major ; then
	  min_libgtop_major=$real_min_libgtop_major
	  min_libgtop_minor=$real_min_libgtop_minor
	  min_libgtop_micro=$real_min_libgtop_micro
	elif test $real_min_libgtop_major = $min_libgtop_major ; then
	  if test $real_min_libgtop_minor -gt $min_libgtop_minor ; then
	    min_libgtop_minor=$real_min_libgtop_minor
	    min_libgtop_micro=$real_min_libgtop_micro
	  elif test $real_min_libgtop_minor = $min_libgtop_minor ; then
	    if test $real_min_libgtop_micro -gt $min_libgtop_micro ; then
	      min_libgtop_micro=$real_min_libgtop_micro
	    fi
	  fi
	fi
	min_libgtop_version="$min_libgtop_major.$min_libgtop_minor.$min_libgtop_micro"
	AC_MSG_CHECKING(for libgtop - version >= $min_libgtop_version)
	no_libgtop=""
	if test "$LIBGTOP_CONFIG" = "no" ; then
	  no_libgtop=yes
	else
	  configfile=`$LIBGTOP_CONFIG --config`
	  libgtop_major_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	  libgtop_minor_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	  libgtop_micro_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	  if test $libgtop_major_version != $min_libgtop_major ; then 
	    no_libgtop=mismatch
	  else 
 	    test $libgtop_minor_version -lt $min_libgtop_minor && no_libgtop=yes
	    if test $libgtop_minor_version = $min_libgtop_minor ; then
	      test $libgtop_micro_version -lt $min_libgtop_micro && no_libgtop=yes
	    fi
	  fi
	  . $configfile
	fi
	if test x$no_libgtop = x ; then
	  AC_DEFINE(HAVE_LIBGTOP)
	  AC_DEFINE_UNQUOTED(LIBGTOP_VERSION, "$LIBGTOP_VERSION")
	  AC_DEFINE_UNQUOTED(LIBGTOP_VERSION_CODE, $LIBGTOP_VERSION_CODE)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MAJOR_VERSION, $LIBGTOP_MAJOR_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MINOR_VERSION, $LIBGTOP_MINOR_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MICRO_VERSION, $LIBGTOP_MICRO_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_SERVER_VERSION, $LIBGTOP_SERVER_VERSION)
	  AC_MSG_RESULT(yes)
	  dnl Note that an empty true branch is not valid sh syntax.
	  ifelse([$2], [], :, [$2])
	else
	  AC_MSG_RESULT(no)
	  if test "$no_libgtop"x = mismatchx; then
	    AC_MSG_ERROR(LibGTop major version mismatch $libgtop_major_version != $min_libgtop_major)
	  fi
	  if test "x$3" = "xfail"; then
	    AC_MSG_ERROR(LibGTop >= $min_libgtop_version not found)
	  else
	    AC_MSG_WARN(LibGTop >= $min_libgtop_version not found)
	  fi
	fi

	AM_CONDITIONAL(HAVE_LIBGTOP, test x$no_libgtop != xyes)
])

AC_DEFUN([GNOME_INIT_LIBGTOP],[
	GNOME_LIBGTOP_HOOK($1,[ifelse([$3], [], :, [$3])],$2)
])

dnl
dnl GNOME_LIBGTOP_DOCU
dnl
dnl checks whether the documentation of LibGTop is installed
dnl

AC_DEFUN([GNOME_LIBGTOP_DOCU],
[
	AC_REQUIRE([GNOME_LIBGTOP_HOOK])

	helpdir="$LIBGTOP_DATADIR/gnome/help/libgtop"

	AC_MSG_CHECKING(whether you have the LibGTop Documentation)

	if test -f "$helpdir/C/topic.dat" ; then
	  have_libgtop_docu=yes
	  AC_DEFINE(HAVE_LIBGTOP_DOCU)
	else
	  have_libgtop_docu=no
	fi

	AC_MSG_RESULT($have_libgtop_docu)

	AM_CONDITIONAL(HAVE_LIBGTOP_DOCU, test x$have_libgtop_docu = xyes)
])


dnl Curses detection: Munged from Midnight Commander's configure.in
dnl
dnl What it does:
dnl =============
dnl
dnl - Determine which version of curses is installed on your system
dnl   and set the -I/-L/-l compiler entries and add a few preprocessor
dnl   symbols 
dnl - Do an AC_SUBST on the CURSES_INCLUDEDIR and CURSES_LIBS so that
dnl   @CURSES_INCLUDEDIR@ and @CURSES_LIBS@ will be available in
dnl   Makefile.in's
dnl - Modify the following configure variables (these are the only
dnl   curses.m4 variables you can access from within configure.in)
dnl   CURSES_INCLUDEDIR - contains -I's and possibly -DRENAMED_CURSES if
dnl                       an ncurses.h that's been renamed to curses.h
dnl                       is found.
dnl   CURSES_LIBS       - sets -L and -l's appropriately
dnl   CFLAGS            - if --with-sco, add -D_SVID3 
dnl   has_curses        - exports result of tests to rest of configure
dnl
dnl Usage:
dnl ======
dnl 1) Add lines indicated below to acconfig.h
dnl 2) call AC_CHECK_CURSES after AC_PROG_CC in your configure.in
dnl 3) Instead of #include <curses.h> you should use the following to
dnl    properly locate ncurses or curses header file
dnl
dnl    #if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
dnl    #include <ncurses.h>
dnl    #else
dnl    #include <curses.h>
dnl    #endif
dnl
dnl 4) Make sure to add @CURSES_INCLUDEDIR@ to your preprocessor flags
dnl 5) Make sure to add @CURSES_LIBS@ to your linker flags or LIBS
dnl
dnl Notes with automake:
dnl - call AM_CONDITIONAL(HAS_CURSES, test "$has_curses" = true) from
dnl   configure.in
dnl - your Makefile.am can look something like this
dnl   -----------------------------------------------
dnl   INCLUDES= blah blah blah $(CURSES_INCLUDEDIR) 
dnl   if HAS_CURSES
dnl   CURSES_TARGETS=name_of_curses_prog
dnl   endif
dnl   bin_PROGRAMS = other_programs $(CURSES_TARGETS)
dnl   other_programs_SOURCES = blah blah blah
dnl   name_of_curses_prog_SOURCES = blah blah blah
dnl   other_programs_LDADD = blah
dnl   name_of_curses_prog_LDADD = blah $(CURSES_LIBS)
dnl   -----------------------------------------------
dnl
dnl
dnl The following lines should be added to acconfig.h:
dnl ==================================================
dnl
dnl /*=== Curses version detection defines ===*/
dnl /* Found some version of curses that we're going to use */
dnl #undef HAS_CURSES
dnl    
dnl /* Use SunOS SysV curses? */
dnl #undef USE_SUNOS_CURSES
dnl 
dnl /* Use old BSD curses - not used right now */
dnl #undef USE_BSD_CURSES
dnl 
dnl /* Use SystemV curses? */
dnl #undef USE_SYSV_CURSES
dnl 
dnl /* Use Ncurses? */
dnl #undef USE_NCURSES
dnl 
dnl /* If you Curses does not have color define this one */
dnl #undef NO_COLOR_CURSES
dnl 
dnl /* Define if you want to turn on SCO-specific code */
dnl #undef SCO_FLAVOR
dnl 
dnl /* Set to reflect version of ncurses *
dnl  *   0 = version 1.*
dnl  *   1 = version 1.9.9g
dnl  *   2 = version 4.0/4.1 */
dnl #undef NCURSES_970530
dnl
dnl /*=== End new stuff for acconfig.h ===*/
dnl 


AC_DEFUN([AC_CHECK_CURSES],[
	search_ncurses=true
	screen_manager=""
	has_curses=false

	CFLAGS=${CFLAGS--O}

	AC_SUBST(CURSES_LIBS)
	AC_SUBST(CURSES_INCLUDEDIR)

	AC_ARG_WITH(sco,
	  [  --with-sco              Use this to turn on SCO-specific code],[
	  if test x$withval = xyes; then
		AC_DEFINE(SCO_FLAVOR)
		CFLAGS="$CFLAGS -D_SVID3"
	  fi
	])

	AC_ARG_WITH(sunos-curses,
	  [  --with-sunos-curses     Used to force SunOS 4.x curses],[
	  if test x$withval = xyes; then
		AC_USE_SUNOS_CURSES
	  fi
	])

	AC_ARG_WITH(osf1-curses,
	  [  --with-osf1-curses      Used to force OSF/1 curses],[
	  if test x$withval = xyes; then
		AC_USE_OSF1_CURSES
	  fi
	])

	AC_ARG_WITH(vcurses,
	  [  --with-vcurses[=incdir] Used to force SysV curses],
	  if test x$withval != xyes; then
		CURSES_INCLUDEDIR="-I$withval"
	  fi
	  AC_USE_SYSV_CURSES
	)

	AC_ARG_WITH(ncurses,
	  [  --with-ncurses[=dir]  Compile with ncurses/locate base dir],
	  if test x$withval = xno ; then
		search_ncurses=false
	  elif test x$withval != xyes ; then
		CURSES_LIBS="$LIBS -L$withval/lib -lncurses"
		CURSES_INCLUDEDIR="-I$withval/include"
		search_ncurses=false
		screen_manager="ncurses"
		AC_DEFINE(USE_NCURSES)
		AC_DEFINE(HAS_CURSES)
		has_curses=true
	  fi
	)

	if $search_ncurses
	then
		AC_SEARCH_NCURSES()
	fi


])


AC_DEFUN([AC_USE_SUNOS_CURSES], [
	search_ncurses=false
	screen_manager="SunOS 4.x /usr/5include curses"
	AC_MSG_RESULT(Using SunOS 4.x /usr/5include curses)
	AC_DEFINE(USE_SUNOS_CURSES)
	AC_DEFINE(HAS_CURSES)
	has_curses=true
	AC_DEFINE(NO_COLOR_CURSES)
	AC_DEFINE(USE_SYSV_CURSES)
	CURSES_INCLUDEDIR="-I/usr/5include"
	CURSES_LIBS="/usr/5lib/libcurses.a /usr/5lib/libtermcap.a"
	AC_MSG_RESULT(Please note that some screen refreshs may fail)
])

AC_DEFUN([AC_USE_OSF1_CURSES], [
       AC_MSG_RESULT(Using OSF1 curses)
       search_ncurses=false
       screen_manager="OSF1 curses"
       AC_DEFINE(HAS_CURSES)
       has_curses=true
       AC_DEFINE(NO_COLOR_CURSES)
       AC_DEFINE(USE_SYSV_CURSES)
       CURSES_LIBS="-lcurses"
])

AC_DEFUN([AC_USE_SYSV_CURSES], [
	AC_MSG_RESULT(Using SysV curses)
	AC_DEFINE(HAS_CURSES)
	has_curses=true
	AC_DEFINE(USE_SYSV_CURSES)
	search_ncurses=false
	screen_manager="SysV/curses"
	CURSES_LIBS="-lcurses"
])

dnl AC_ARG_WITH(bsd-curses,
dnl [--with-bsd-curses         Used to compile with bsd curses, not very fancy],
dnl 	search_ncurses=false
dnl	screen_manager="Ultrix/cursesX"
dnl	if test $system = ULTRIX
dnl	then
dnl	    THIS_CURSES=cursesX
dnl        else
dnl	    THIS_CURSES=curses
dnl	fi
dnl
dnl	CURSES_LIBS="-l$THIS_CURSES -ltermcap"
dnl	AC_DEFINE(HAS_CURSES)
dnl	has_curses=true
dnl	AC_DEFINE(USE_BSD_CURSES)
dnl	AC_MSG_RESULT(Please note that some screen refreshs may fail)
dnl	AC_MSG_WARN(Use of the bsdcurses extension has some)
dnl	AC_MSG_WARN(display/input problems.)
dnl	AC_MSG_WARN(Reconsider using xcurses)
dnl)

	
dnl
dnl Parameters: directory filename cureses_LIBS curses_INCLUDEDIR nicename
dnl
AC_DEFUN([AC_NCURSES], [
    if $search_ncurses
    then
        if test -f $1/$2
	then
	    AC_MSG_RESULT(Found ncurses on $1/$2)
 	    CURSES_LIBS="$3"
	    CURSES_INCLUDEDIR="$4"
	    search_ncurses=false
	    screen_manager=$5
            AC_DEFINE(HAS_CURSES)
            has_curses=true
	    AC_DEFINE(USE_NCURSES)
	fi
    fi
])

AC_DEFUN([AC_SEARCH_NCURSES], [
    AC_CHECKING("location of ncurses.h file")

    AC_NCURSES(/usr/include, ncurses.h, -lncurses,, "ncurses on /usr/include")
    AC_NCURSES(/usr/include/ncurses, ncurses.h, -lncurses, -I/usr/include/ncurses, "ncurses on /usr/include/ncurses")
    AC_NCURSES(/usr/local/include, ncurses.h, -L/usr/local/lib -lncurses, -I/usr/local/include, "ncurses on /usr/local")
    AC_NCURSES(/usr/local/include/ncurses, ncurses.h, -L/usr/local/lib -L/usr/local/lib/ncurses -lncurses, -I/usr/local/include/ncurses, "ncurses on /usr/local/include/ncurses")

    AC_NCURSES(/usr/local/include/ncurses, curses.h, -L/usr/local/lib -lncurses, -I/usr/local/include/ncurses -DRENAMED_NCURSES, "renamed ncurses on /usr/local/.../ncurses")

    AC_NCURSES(/usr/include/ncurses, curses.h, -lncurses, -I/usr/include/ncurses -DRENAMED_NCURSES, "renamed ncurses on /usr/include/ncurses")

    dnl
    dnl We couldn't find ncurses, try SysV curses
    dnl
    if $search_ncurses 
    then
        AC_EGREP_HEADER(init_color, /usr/include/curses.h,
	    AC_USE_SYSV_CURSES)
	AC_EGREP_CPP(USE_NCURSES,[
#include <curses.h>
#ifdef __NCURSES_H
#undef USE_NCURSES
USE_NCURSES
#endif
],[
	CURSES_INCLUDEDIR="$CURSES_INCLUDEDIR -DRENAMED_NCURSES"
        AC_DEFINE(HAS_CURSES)
	has_curses=true
        AC_DEFINE(USE_NCURSES)
        search_ncurses=false
        screen_manager="ncurses installed as curses"
])
    fi

    dnl
    dnl Try SunOS 4.x /usr/5{lib,include} ncurses
    dnl The flags USE_SUNOS_CURSES, USE_BSD_CURSES and BUGGY_CURSES
    dnl should be replaced by a more fine grained selection routine
    dnl
    if $search_ncurses
    then
	if test -f /usr/5include/curses.h
	then
	    AC_USE_SUNOS_CURSES
        fi
    else
        # check for ncurses version, to properly ifdef mouse-fix
	AC_MSG_CHECKING(for ncurses version)
	ncurses_version=unknown
cat > conftest.$ac_ext <<EOF
[#]line __oline__ "configure"
#include "confdefs.h"
#ifdef RENAMED_NCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif
#undef VERSION
VERSION:NCURSES_VERSION
EOF
        if (eval "$ac_cpp conftest.$ac_ext") 2>&AC_FD_CC |
  egrep "VERSION:" >conftest.out 2>&1; then
changequote(,)dnl
            ncurses_version=`cat conftest.out|sed -e 's/^[^"]*"//' -e 's/".*//'`
changequote([,])dnl
	fi
	rm -rf conftest*
        AC_MSG_RESULT($ncurses_version)
	case "$ncurses_version" in
changequote(,)dnl
	4.[01])
changequote([,])dnl
            AC_DEFINE(NCURSES_970530,2)
            ;;
	1.9.9g)
            AC_DEFINE(NCURSES_970530,1)
            ;;
	1*)
            AC_DEFINE(NCURSES_970530,0)
            ;;
	esac
    fi
])






dnl AM_PATH_OAF([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for OAF, and define OAF_CFLAGS and OAF_LIBS
dnl
AC_DEFUN(AM_PATH_OAF,
[dnl 
dnl Get the cflags and libraries from the oaf-config script
dnl
AC_ARG_WITH(oaf-prefix,[  --with-oaf-prefix=PFX   Prefix where OAF is installed (optional)],
            oaf_config_prefix="$withval", oaf_config_prefix="")
AC_ARG_WITH(oaf-exec-prefix,[  --with-oaf-exec-prefix=PFX Exec prefix where OAF is installed (optional)],
            oaf_config_exec_prefix="$withval", oaf_config_exec_prefix="")
AC_ARG_ENABLE(oaftest, [  --disable-oaftest       Do not try to compile and run a test OAF program],
		    , enable_oaftest=yes)

  oaf_config_args="$oaf_config_args $4"

  if test x$oaf_config_exec_prefix != x ; then
     oaf_config_args="$oaf_config_args --exec-prefix=$oaf_config_exec_prefix"
     if test x${OAF_CONFIG+set} != xset ; then
        OAF_CONFIG=$oaf_config_exec_prefix/bin/oaf-config
     fi
  fi
  if test x$oaf_config_prefix != x ; then
     oaf_config_args="$oaf_config_args --prefix=$oaf_config_prefix"
     if test x${OAF_CONFIG+set} != xset ; then
        OAF_CONFIG=$oaf_config_prefix/bin/oaf-config
     fi
  fi

  AC_PATH_PROG(OAF_CONFIG, oaf-config, no)
  min_oaf_version=ifelse([$1], , 0.1, $1)
  AC_MSG_CHECKING(for OAF - version >= $min_oaf_version)
  no_oaf=""
  if test "$OAF_CONFIG" = "no" ; then
    no_oaf=yes
  else
    OAF_CFLAGS="`$OAF_CONFIG $oaf_config_args --cflags`"
    OAF_LIBS="`$OAF_CONFIG $oaf_config_args --libs`"
    oaf_config_major_version=`$OAF_CONFIG $oaf_config_args --version | \
	   sed -e 's,^[[^0-9.]]*,,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    oaf_config_minor_version=`$OAF_CONFIG $oaf_config_args --version | \
	   sed -e 's,^[[^0-9.]]*,,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    oaf_config_micro_version=`$OAF_CONFIG $oaf_config_args --version | \
	   sed -e 's,^[[^0-9\.]]*,,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_oaftest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $OAF_CFLAGS"
      LIBS="$OAF_LIBS $LIBS"
dnl
dnl Now check if the installed OAF is sufficiently new. (Also sanity
dnl checks the results of oaf-config to some extent
dnl
      rm -f conf.oaftest
      AC_TRY_RUN([
#include <liboaf/liboaf.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.oaftest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_oaf_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_oaf_version");
     exit(1);
   }

  if ((liboaf_major_version != $oaf_config_major_version) ||
      (liboaf_minor_version != $oaf_config_minor_version) ||
      (liboaf_micro_version != $oaf_config_micro_version))
    {
      printf("\n*** 'oaf-config --version' returned %d.%d.%d, but OAF (%d.%d.%d)\n", 
             $oaf_config_major_version, $oaf_config_minor_version, $oaf_config_micro_version,
             liboaf_major_version, liboaf_minor_version, liboaf_micro_version);
      printf ("*** was found! If oaf-config was correct, then it is best\n");
      printf ("*** to remove the old version of OAF. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If oaf-config was wrong, set the environment variable OAF_CONFIG\n");
      printf("*** to point to the correct copy of oaf-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (OAF_MAJOR_VERSION) && defined (OAF_MINOR_VERSION) && defined (OAF_MICRO_VERSION)
  else if ((liboaf_major_version != OAF_MAJOR_VERSION) ||
	   (liboaf_minor_version != OAF_MINOR_VERSION) ||
           (liboaf_micro_version != OAF_MICRO_VERSION))
    {
      printf("*** OAF header files (version %d.%d.%d) do not match\n",
	     OAF_MAJOR_VERSION, OAF_MINOR_VERSION, OAF_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     liboaf_major_version, liboaf_minor_version, liboaf_micro_version);
    }
#endif /* defined (OAF_MAJOR_VERSION) ... */
  else
    {
      if ((liboaf_major_version > major) ||
        ((liboaf_major_version == major) && (liboaf_minor_version > minor)) ||
        ((liboaf_major_version == major) && (liboaf_minor_version == minor) && (liboaf_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of OAF (%d.%d.%d) was found.\n",
               liboaf_major_version, liboaf_minor_version, liboaf_micro_version);
        printf("*** You need a version of OAF newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** OAF is always available from ftp://ftp.gnome.org/pub/GNOME/sources/oaf/.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the oaf-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of OAF, but you can also set the OAF_CONFIG environment to point to the\n");
        printf("*** correct copy of oaf-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_oaf=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_oaf" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$OAF_CONFIG" = "no" ; then
       echo "*** The oaf-config script installed by OAF could not be found"
       echo "*** If OAF was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the OAF_CONFIG environment variable to the"
       echo "*** full path to oaf-config."
     else
       if test -f conf.oaftest ; then
        :
       else
          echo "*** Could not run OAF test program, checking why..."
          CFLAGS="$CFLAGS $OAF_CFLAGS"
          LIBS="$LIBS $OAF_LIBS"
          AC_TRY_LINK([
#include <liboaf/liboaf.h>
#include <stdio.h>
],      [ return ((liboaf_major_version) || (liboaf_minor_version) || (liboaf_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding OAF or finding the wrong"
          echo "*** version of OAF. If it is not finding OAF, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the OAF package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps oaf oaf-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means OAF was incorrectly installed"
          echo "*** or that you have moved OAF since it was installed. In the latter case, you"
          echo "*** may want to edit the oaf-config script: $OAF_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     OAF_CFLAGS=""
     OAF_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(OAF_CFLAGS)
  AC_SUBST(OAF_LIBS)
  rm -f conf.oaftest
])

dnl AM_PATH_GCONF([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GCONF, and define GCONF_CFLAGS and GCONF_LIBS
dnl
AC_DEFUN(AM_PATH_GCONF,
[dnl 
dnl Get the cflags and libraries from the gconf-config script
dnl
AC_ARG_WITH(gconf-prefix,[  --with-gconf-prefix=PFX   Prefix where GCONF is installed (optional)],
            gconf_config_prefix="$withval", gconf_config_prefix="")
AC_ARG_WITH(gconf-exec-prefix,[  --with-gconf-exec-prefix=PFX Exec prefix where GCONF is installed (optional)],
            gconf_config_exec_prefix="$withval", gconf_config_exec_prefix="")
AC_ARG_ENABLE(gconftest, [  --disable-gconftest       Do not try to compile and run a test GCONF program],
		    , enable_gconftest=yes)

  gconf_config_args="$gconf_config_args"

  if test x$gconf_config_exec_prefix != x ; then
     gconf_config_args="$gconf_config_args --exec-prefix=$gconf_config_exec_prefix"
     if test x${GCONF_CONFIG+set} != xset ; then
        GCONF_CONFIG=$gconf_config_exec_prefix/bin/gconf-config
     fi
  fi
  if test x$gconf_config_prefix != x ; then
     gconf_config_args="$gconf_config_args --prefix=$gconf_config_prefix"
     if test x${GCONF_CONFIG+set} != xset ; then
        GCONF_CONFIG=$gconf_config_prefix/bin/gconf-config
     fi
  fi

  AC_PATH_PROG(GCONF_CONFIG, gconf-config, no)
  min_gconf_version=ifelse([$1], , 0.5, $1)
  AC_MSG_CHECKING(for GCONF - version >= $min_gconf_version)
  no_gconf=""
  if test "$GCONF_CONFIG" = "no" ; then
    no_gconf=yes
  else
    GCONF_CFLAGS="`$GCONF_CONFIG  $gconf_config_args --cflags $4`"
    GCONF_LIBS="`$GCONF_CONFIG  $gconf_config_args --libs $4`"
    gconf_config_major_version=`$GCONF_CONFIG $gconf_config_args --version | \
	   sed -e 's,^[[^0-9.]]*,,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gconf_config_minor_version=`$GCONF_CONFIG $gconf_config_args --version | \
	   sed -e 's,^[[^0-9.]]*,,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gconf_config_micro_version=`$GCONF_CONFIG $gconf_config_args --version | \
	   sed -e 's,^[[^0-9\.]]*,,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  fi
  if test "x$no_gconf" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GCONF_CONFIG" = "no" ; then
       echo "*** The gconf-config script installed by GCONF could not be found"
       echo "*** If GCONF was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GCONF_CONFIG environment variable to the"
       echo "*** full path to gconf-config."
     else
	:
     fi
     GCONF_CFLAGS=""
     GCONF_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GCONF_CFLAGS)
  AC_SUBST(GCONF_LIBS)
  rm -f conf.gconftest
])

dnl AM_GCONF_SOURCE
dnl Define GCONF_SCHEMA_CONFIG_SOURCE
dnl
AC_DEFUN(AM_GCONF_SOURCE,
[
  if test "x$GCONF_SCHEMA_INSTALL_SOURCE" = "x"; then
    GCONF_SCHEMA_CONFIG_SOURCE=`gconftool --get-default-source`
  else
    GCONF_SCHEMA_CONFIG_SOURCE=$GCONF_SCHEMA_INSTALL_SOURCE
  fi

  AC_ARG_WITH(gconf-source, 
  [  --with-gconf-source=sourceaddress      Config database for installing schema files.],GCONF_SCHEMA_CONFIG_SOURCE="$withval",)

  AC_SUBST(GCONF_SCHEMA_CONFIG_SOURCE)
  AC_MSG_RESULT("Using config source $GCONF_SCHEMA_CONFIG_SOURCE for schema installation")

  if test "x$GCONF_SCHEMA_FILE_DIR" = "x"; then
    GCONF_SCHEMA_FILE_DIR=$sysconfdir/gconf/schemas/
  else
    GCONF_SCHEMA_FILE_DIR=$GCONF_SCHEMA_FILE_DIR
  fi

  AC_ARG_WITH(gconf-schema-file-dir, 
  [  --with-gconf-schema-file-dir=dir        Directory for installing schema files.],GCONF_SCHEMA_FILE_DIR="$withval",)

  AC_SUBST(GCONF_SCHEMA_FILE_DIR)
  AC_MSG_RESULT("Using $GCONF_SCHEMA_FILE_DIR as install directory for schema files")
])

