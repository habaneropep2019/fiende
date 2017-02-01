# Configure paths for libole2
# Arturo Tena (tenix)	  1999-09-24
#
# This file is a modified version of the
# file glib.m4 that came with glib 1.2.3:
# Configure paths for GLIB
# Owen Taylor     97-11-3

# This m4 macro don't depend on GNOME, just glib.

dnl AM_PATH_LIBOLE2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for libole2, and define LIBOLE2_CFLAGS and LIBOLE2_LIBS
dnl
dnl Example of use (write the following in your 'configure.in'):
dnl AM_PATH_LIBOLE2(0.0.1, [LIBS="$LIBS $LIBOLE2_LIBS" CFLAGS="$CFLAGS $LIBOLE2_CFLAGS"], AC_MSG_ERROR([Can't find libole2.]))
dnl
dnl
AC_DEFUN(AM_PATH_LIBOLE2,
[dnl 
dnl Get the cflags and libraries from the libole2-config script
dnl
AC_ARG_WITH(libole2-prefix,[  --with-libole2-prefix=PFX   Prefix where libole2 is installed (optional)],
           libole2_config_prefix="$withval", libole2_config_prefix="")
AC_ARG_WITH(libole2-exec-prefix,[  --with-libole2-exec-prefix=PFX Exec prefix where libole2 is installed (optional)],
           libole2_config_exec_prefix="$withval", libole2_config_exec_prefix="")
AC_ARG_ENABLE(libole2test, [  --disable-libole2test       Do not try to compile and run a test libole2 program],
		    , enable_libole2test=yes)

  if test x$libole2_config_exec_prefix != x ; then
     libole2_config_args="$libole2_config_args --exec-prefix=$libole2_config_exec_prefix"
     if test x${LIBOLE2_CONFIG+set} != xset ; then
        LIBOLE2_CONFIG=$libole2_config_exec_prefix/bin/libole2-config
     fi
  fi
  if test x$libole2_config_prefix != x ; then
     libole2_config_args="$libole2_config_args --prefix=$libole2_config_prefix"
     if test x${LIBOLE2_CONFIG+set} != xset ; then
        LIBOLE2_CONFIG=$libole2_config_prefix/bin/libole2-config
     fi
  fi

dnl  for module in . $4
dnl  do
dnl      case "$module" in
dnl         gmodule) 
dnl             glib_config_args="$glib_config_args gmodule"
dnl         ;;
dnl         gthread) 
dnl             glib_config_args="$glib_config_args gthread"
dnl         ;;
dnl      esac
dnl  done

  AC_PATH_PROG(LIBOLE2_CONFIG, libole2-config, no)
  min_libole2_version=ifelse([$1], ,0.0.1,$1)
  AC_MSG_CHECKING(for libole2 - version >= $min_libole2_version)
  no_libole2=""
  if test "$LIBOLE2_CONFIG" = "no" ; then
    no_libole2=yes
  else
    LIBOLE2_CFLAGS="`$LIBOLE2_CONFIG $libole2_config_args --cflags` `glib-config --cflags`"
    LIBOLE2_LIBS=`$LIBOLE2_CONFIG $libole2_config_args --libs`
    libole2_config_major_version=`$LIBOLE2_CONFIG $libole2_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libole2_config_minor_version=`$LIBOLE2_CONFIG $libole2_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    libole2_config_micro_version=`$LIBOLE2_CONFIG $libole2_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    if test "x$enable_libole2test" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $LIBOLE2_CFLAGS"
      LIBS="$LIBOLE2_LIBS $LIBS"
dnl
dnl Now check if the installed libole2 is sufficiently new. (Also sanity
dnl checks the results of libole2-config to some extent)
dnl
      rm -f conf.libole2test
      AC_TRY_RUN([
#include <libole2/libole2.h>
#include <stdio.h>
#include <stdlib.h>

int
mystrlen (char * s)
{
	char *p;
	for (p = s; *p; p++) ;
	return p - s;
}

char *
mystrdup (char * s)
{
	char *ret, *p1, *p2;

	ret = malloc (mystrlen (s));
	if (ret == NULL) return NULL;

	p1 = s; p2 = ret;
	while (*p1) {
		*p2 = *p1;
		p1++;
		p2++;
	}

	return ret;
}

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.libole2test");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = mystrdup("$min_libole2_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_libole2_version");
     exit(1);
   }

  if ((libole2_major_version != $libole2_config_major_version) ||
      (libole2_minor_version != $libole2_config_minor_version) ||
      (libole2_micro_version != $libole2_config_micro_version))
    {
      printf("\n*** 'libole2-config --version' returned %d.%d.%d, but libole2 (%d.%d.%d)\n", 
             $libole2_config_major_version, $libole2_config_minor_version, $libole2_config_micro_version,
             libole2_major_version, libole2_minor_version, libole2_micro_version);
      printf ("*** was found! If libole2-config was correct, then it is best\n");
      printf ("*** to remove the old version of libole2. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If libole2-config was wrong, set the environment variable LIBOLE2_CONFIG\n");
      printf("*** to point to the correct copy of libole2-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
    } 
  else if ((libole2_major_version != LIBOLE2_MAJOR_VERSION) ||
	   (libole2_minor_version != LIBOLE2_MINOR_VERSION) ||
           (libole2_micro_version != LIBOLE2_MICRO_VERSION))
    {
      printf("*** LIBOLE2 header files (version %d.%d.%d) do not match\n",
	   LIBOLE2_MAJOR_VERSION, LIBOLE2_MINOR_VERSION, LIBOLE2_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	   libole2_major_version, libole2_minor_version, libole2_micro_version);
    }
  else
    {
      if ((libole2_major_version > major) ||
        ((libole2_major_version == major) && (libole2_minor_version > minor)) ||
        ((libole2_major_version == major) && (libole2_minor_version == minor) && (libole2_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of libole2 (%d.%d.%d) was found.\n",
               libole2_major_version, libole2_minor_version, libole2_micro_version);
        printf("*** You need a version of libole2 newer than %d.%d.%d. The latest version of\n", major, minor, micro);
        printf("*** libole2 is always available from ftp://ftp.gnome.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the libole2-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of libole2, but you can also set the LIBOLE2_CONFIG environment to point to\n");
        printf("*** the correct copy of libole2-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_libole2=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_libole2" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$LIBOLE2_CONFIG" = "no" ; then
       echo "*** The libole2-config script installed by libole2 could not be found"
       echo "*** If libole2 was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LIBOLE2_CONFIG environment variable to the"
       echo "*** full path to libole2-config."
     else
       if test -f conf.libole2test ; then
        :
       else
          echo "*** Could not run libole2 test program, checking why..."
          CFLAGS="$CFLAGS $LIBOLE2_CFLAGS"
          LIBS="$LIBS $LIBOLE2_LIBS"
          AC_TRY_LINK([
#include <libole2/libole2.h>
#include <stdio.h>
],      [ return ((libole2_major_version) || (libole2_minor_version) || (libole2_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding libole2 or finding the wrong"
          echo "*** version of libole2. If it is not finding libole2, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location. Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means libole2 was incorrectly"
          echo "*** installed or that you have moved libole2 since it was installed. In the"
          echo "*** latter case, you may want to edit the libole2-config script:"
          echo "*** $LIBOLE2_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     LIBOLE2_CFLAGS=""
     LIBOLE2_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LIBOLE2_CFLAGS)
  AC_SUBST(LIBOLE2_LIBS)
  rm -f conf.libole2test
])
