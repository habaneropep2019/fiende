AC_DEFUN(AM_GNOME_CHECK_TYPE,
  [AC_CACHE_CHECK([$1 in <sys/types.h>], ac_cv_type_$1,
     [AC_TRY_COMPILE([
#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
],[$1 foo;],
     ac_cv_type_$1=yes, ac_cv_type_$1=no)])
   if test $ac_cv_type_$1 = no; then
      AC_DEFINE($1, $2, $1)
   fi
])

AC_DEFUN(AM_GNOME_SIZE_T,
  [AM_GNOME_CHECK_TYPE(size_t, unsigned)
   AC_PROVIDE([AC_TYPE_SIZE_T])
])

AC_DEFUN(AM_GNOME_OFF_T,
  [AM_GNOME_CHECK_TYPE(off_t, long)
   AC_PROVIDE([AC_TYPE_OFF_T])
])

