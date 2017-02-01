# Configure paths for Gnome--
# Agustin Ferrin - Mar 2001
# Stolen from Gtk--.m4


dnl Test for GNOMEMM, and define GNOMEMM_CFLAGS and GNOMEMM_LIBS
dnl   to be used as follows:
dnl AM_PATH_GNOMEMM([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_GNOMEMM,
[dnl
dnl Get the cflags and libraries from the gnome-config script
dnl
AC_ARG_WITH(gnome-prefix,[  --with-gnome-prefix=PREFIX
                          Prefix where gnome-config is installed (optional)],
            gnome_config_prefix="$withval", gnome_config_prefix="")
AC_ARG_WITH(gnome-exec-prefix,[  --with-gnome-exec-prefix=PREFIX
                          Exec prefix where gnome-config is installed (optional)],
            gnome_config_exec_prefix="$withval", gnome_config_exec_prefix="")
AC_ARG_ENABLE(gnomemmtest, [  --disable-gnomemmtest     Do not try to compile and run a test Gnome-- program],
		    , enable_gnomemmtest=yes)

  if test x$gnome_config_exec_prefix != x ; then
     gnome_config_args="$gnome_config_args --exec-prefix=$gnome_config_exec_prefix"
     if test x${GNOME_CONFIG+set} != xset ; then
        GNOME_CONFIG=$gnome_config_exec_prefix/bin/gnome-config
     fi
  fi
  if test x$gnomemm_config_prefix != x ; then
     gnome_config_args="$gnome_config_args --prefix=$gnome_config_prefix"
     if test x${GNOME_CONFIG+set} != xset ; then
        GNOME_CONFIG=$gnome_config_prefix/bin/gnome-config
     fi
  fi

  AC_PATH_PROG(GNOME_CONFIG, gnome-config, no)
  min_gnomemm_version=ifelse([$1], ,0.10.0,$1)

  AC_MSG_CHECKING(for Gnome-- - version >= $min_gnomemm_version)
  AC_LANG_SAVE
  no_gnomemm=""
  if test "$GNOME_CONFIG" = "no" ; then
    no_gnomemm=yes
  else
    AC_LANG_CPLUSPLUS

    GNOMEMM_CFLAGS=`$GNOME_CONFIG $gnomemm_config_args --cflags gnomemm`
    GNOMEMM_LIBS=`$GNOME_CONFIG $gnomemm_config_args --libs gnomemm`
    gnomemm_config_major_version=`$GNOME_CONFIG $gnome_config_args --modversion gnomemm | \
           sed 's/gnomemm-\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gnomemm_config_minor_version=`$GNOME_CONFIG $gnome_config_args --modversion gnomemm | \
           sed 's/gnomemm-\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gnomemm_config_micro_version=`$GNOME_CONFIG $gnome_config_args --modversion gnomemm | \
           sed 's/gnomemm-\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gnomemmtest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $GNOMEMM_CFLAGS"
      LIBS="$LIBS $GNOMEMM_LIBS"
dnl
dnl Now check if the installed GTK-- is sufficiently new. (Also sanity
dnl checks the results of gnome-config gnomemm to some extent
dnl
      rm -f conf.gnomemmtest
      AC_TRY_RUN([
#include <gnome--.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gnomemmtest");

  /* HP/UX 0 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gnomemm_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gnomemm_version");
     exit(1);
   }
   if (($gnomemm_config_major_version > major) ||
      (($gnomemm_config_major_version == major) && ($gnomemm_config_minor_version > minor)) ||
      (($gnomemm_config_major_version == major) && ($gnomemm_config_minor_version == minor) && ($gnomemm_config_micro_version >= micro)))
   {
     return 0;
   }
   else
   {
      printf("\n*** An old version of Gnome-- (%d.%d.%d) was found.\n",
              $gnomemm_config_major_version, $gnomemm_config_minor_version, $gnomemm_config_micro_version);
      printf("*** You need a version of Gnome-- newer than %d.%d.%d. The latest version of\n",
              major, minor, micro);
      printf("*** Gnome-- is always available from http://gtkmm.sourceforge.net.\n");
      printf("***\n");
      printf("*** If you have already installed a sufficiently new version, this error\n");
      printf("*** probably means that the wrong copy of the gnome-config shell script is\n");
      printf("*** being found. The easiest way to fix this is to remove the old version\n");
      printf("*** of Gnome--, but you can also set the GNOME_CONFIG environment to point to the\n");
      printf("*** correct copy of gnome-config. (In this case, you will have to\n");
      printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
      printf("*** so that the correct libraries are found at run-time))\n");
    }
  return 1;
}
],, no_gnomemm=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gnomemm" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GNOME_CONFIG" = "no" ; then
       echo "*** The 'gnome-config' script installed by Gnome could not be found"
       echo "*** If Gnome was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GNOME_CONFIG environment variable to the"
       echo "*** full path to gnome-config."
     else
       if test -f conf.gnomemmtest ; then
        :
       else
          echo "*** Could not run Gnome-- test program, checking why..."
          CXXFLAGS="$CXXFLAGS $GNOMEMM_CFLAGS"
          LIBS="$LIBS $GNOMEMM_LIBS"
          AC_TRY_LINK([
#include <gnome--.h>
#include <stdio.h>
],      [ return (($gnomemm_config_major_version) || ($gnomemm_config_minor_version) || ($gnomemm_config_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding Gnome-- or finding the wrong"
          echo "*** version of Gnome--. If it is not finding Gnome--, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
      	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means Gnome-- was incorrectly installed"
          echo "*** or that you have moved Gnome-- since it was installed. In the latter case, you"
          echo "*** may want to edit the gnome-config script: $GNOME_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GNOMEMM_CFLAGS=""
     GNOMEMM_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_LANG_RESTORE
  AC_SUBST(GNOMEMM_CFLAGS)
  AC_SUBST(GNOMEMM_LIBS)
  rm -f conf.gnomemmtest
])
