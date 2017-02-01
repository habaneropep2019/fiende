/* gtk--config.h.  Generated automatically by configure.  */
#ifndef _GTKMM_CONFIG_H
#define _GTKMM_CONFIG_H 1

/* version numbers */
#define GTKMM_MAJOR_VERSION 1
#define GTKMM_MINOR_VERSION 2
#define GTKMM_MICRO_VERSION 8

// detect common platforms
#if defined(_WIN32)
// Win32 compilers have a lot of varation
#if defined(_MSC_VER)
#define GTKMM_MSC
#define GTKMM_WIN32
#elif defined(__CYGWIN__)
#define GTKMM_UNIX
#else
#warning "Unknown architecture (send me gcc --dumpspecs or equiv)"
#endif
#else
#define GTKMM_UNIX
#endif /* _WIN32 */

#ifdef GTKMM_UNIX
/* compiler feature tests that are used during compile time and run-time
   by gtk-- only. tests used by gdk-- and gtk-- should go into 
   gdk--config.h.in */
/* #undef GTKMM_CXX_HAVE_MUTABLE */
/* #undef GTKMM_CXX_HAVE_NAMESPACES */
/* #undef GTKMM_CXX_GAUB */
/* #undef GTKMM_CXX_AMBIGUOUS_TEMPLATES */
#endif

#ifdef GTKMM_MSC
#define GTKMM_CXX_HAVE_MUTABLE
#define GTKMM_CXX_HAVE_NAMESPACES
#pragma warning (disable: 4786 4355 4800 4181)
#endif

/* Using decls for names in std.*/
/*
  Make sure there is at least one space at the end of each of these #defines 
    If there is not, the autoheader sed script will not insert the
    values these defines are suppoed to have.
 */
#define GTKMM_USING_STD_BIDIRECTIONAL_ITERATOR_TAG  using std::bidirectional_iterator_tag;
#define GTKMM_USING_STD_FORWARD_ITERATOR_TAG  using std::forward_iterator_tag;
#define GTKMM_USING_STD_INPUT_ITERATOR_TAG  using std::input_iterator_tag;
#define GTKMM_USING_STD_MAP using std::map;

#endif /* _GTKMM_CONFIG_H */
