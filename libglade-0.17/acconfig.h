#ifndef CONFIG_H
#define CONFIG_H

@TOP@
/* The package name */
#undef PACKAGE

/* The package version */
#undef VERSION

/* extra debugging output */
#undef DEBUG

/* If true, include GNOME support */
#undef ENABLE_GNOME

/* Whether we are using Bonobo */
#undef ENABLE_BONOBO

/* for gettext ... */
#undef HAVE_LC_MESSAGES
#undef HAVE_STPCPY
#undef ENABLE_NLS
#undef HAVE_CATGETS
#undef HAVE_GETTEXT

#undef HAVE_UNISTD_H

@BOTTOM@

#ifdef DEBUG
#  define debug(stmnt) stmnt
#else
#  define debug(stmnt) /* nothing */
#endif

#endif
