#ifndef OS_FEATURE_ALLOCA_H
#define OS_FEATURE_ALLOCA_H 1

#  if ORBIT_HAVE_ALLOCA_H
#  include <alloca.h>
#  endif

#  include <string.h>

#  if defined(__GNUC__)

#    if defined(__STRICT_ANSI__)
#      define alloca __builtin_alloca
#    endif

#  elif !(ORBIT_HAVE_ALLOCA_H)

#    if defined(_AIX)
 #pragma alloca
#    elif !defined(alloca) /* predefined by HP cc +Olibcalls */
char *alloca ();
#    endif

#  endif /* __GNUC__ etc. */

#endif /* OS_FEATURE_ALLOCA_H */
