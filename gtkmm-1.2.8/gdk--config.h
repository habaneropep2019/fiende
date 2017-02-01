/* gdk--config.h.  Generated automatically by configure.  */
#ifndef _GDKMM_CONFIG_H
#define _GDKMM_CONFIG_H 1

// detect common platforms
#if defined(_WIN32)
// Win32 compilers have a lot of varation
#if defined(_MSC_VER)
#define GDKMM_MSC
#define GDKMM_WIN32
#elif defined(__CYGWIN__)
#define GDKMM_UNIX
#else
#warning "Unknown architecture (send me gcc --dumpspecs)"
#endif
#else
#define GDKMM_UNIX
#endif // _WIN32

/* compiler feature tests that are used during compile time and run-time
   by gdk-- and gtk-- */
#ifdef GDKMM_UNIX
/* #undef GTKMM_CXX_HAVE_PARTIAL_SPECIALIZATION */
/* #undef GTKMM_CXX_HAVE_NAMESPACES */
#endif // GDKMM_UNIX

#ifdef GDKMM_MSC
#pragma warning (disable: 4786 4355 4800)
#define GTKMM_CXX_HAVE_NAMESPACES
#endif


/* Using decls for names in std.*/
/*
  Make sure there is at least one space at the end of each of these #defines 
    If there is not, the autoheader sed script will not insert the
    values these defines are suppoed to have.
 */
#define GTKMM_USING_STD_STRING using std::string;

#endif // _GDKMM_CONFIG_H
