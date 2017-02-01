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
