/* sigc++/config/sigc++config.h.  Generated automatically by configure.  */
/* sigc++/config/sigc++config.h.in.  Generated automatically from configure.in by autoheader.  */
#ifndef _SIGC_CONFIG_H_
#define _SIGC_CONFIG_H_

#if 0

/* Name of package */
#define PACKAGE "libsigc++"

/* Version number of package */
#define VERSION "1.0.3"

#define PACKAGE "libsigc++"
#endif /* 0 */

#define SIGC_MAJOR_VERSION 1
#define SIGC_MINOR_VERSION 0
#define SIGC_MICRO_VERSION 3

// detect common platforms
#if defined(_WIN32)
// Win32 compilers have a lot of varation
#if defined(__BORLANDC__)
#define LIBSIGC_BC
#define LIBSIGC_WIN32
#elif defined(_MSC_VER)
#define LIBSIGC_MSC
#define LIBSIGC_WIN32
#elif defined(__CYGWIN__)
#define LIBSIGC_UNIX
#else
#error "Unknown architecture (send me gcc --dumpspecs)"
#endif
#else
#define LIBSIGC_UNIX
#endif /* _WIN32 */


#ifdef LIBSIGC_UNIX
#define SIGC_CXX_NAMESPACES 1
#define SIGC_CXX_PARTIAL_SPEC 1
#define SIGC_CXX_MEMBER_FUNC_TEMPLATES 1
#define SIGC_CXX_MEMBER_CLASS_TEMPLATES 1
#define SIGC_CXX_MUTABLE 1
#define SIGC_CXX_FRIEND_TEMPLATES 1
/* #undef SIGC_CXX_TEMPLATE_CCTOR */
#define SIGC_CXX_INT_CTOR 1
#define SIGC_CXX_VOID_RETURN 1
#define SIGC_CXX_SPECIALIZE_REFERENCES 1
#define SIGC_PTHREADS 1
/* #undef SIGC_PTHREAD_DCE */
#ifdef SIGC_PTHREADS
#define SIGC_PTHREAD_COND_ATTR 4
#define SIGC_PTHREAD_COND_IMPL 12
#define SIGC_PTHREAD_MUTEX_ATTR 4
#define SIGC_PTHREAD_MUTEX_IMPL 24
#define SIGC_PTHREAD_THREAD_ATTR 4
#define SIGC_PTHREAD_THREAD_IMPL 4
#define SIGC_PTHREAD_KEY_IMPL 4
#endif /* SIGC_PTHREADS */
#endif /* LIBSIGC_UNIX */

#ifdef LIBSIGC_BC
#define SIGC_CXX_NAMESPACES 1
#define SIGC_CXX_PARTIAL_SPEC 1
#define SIGC_CXX_MEMBER_FUNC_TEMPLATES 1
#define SIGC_CXX_MEMBER_CLASS_TEMPLATES 1
#define SIGC_CXX_MUTABLE 1
#define SIGC_CXX_FRIEND_TEMPLATES 1
#define SIGC_CXX_INT_CTOR 1
#define SIGC_CXX_VOID_RETURN 1
#define SIGC_CXX_SPECIALIZE_REFERENCES 1
#endif /* LIBSIGC_BC */

#ifdef LIBSIGC_MSC
#define SIGC_CXX_NAMESPACES 1
#define SIGC_CXX_MEMBER_FUNC_TEMPLATES 1
#define SIGC_CXX_MEMBER_CLASS_TEMPLATES 1
#define SIGC_CXX_MUTABLE 1
#define SIGC_CXX_TEMPLATE_CCTOR 1
#define SIGC_CXX_INT_CTOR 1
#endif /* LIBSIGC_MSC */

#ifdef LIBSIGC_WIN32
#ifdef LIBSIGC_EXPORTS
#define LIBSIGC_API __declspec(dllexport)
#define LIBSIGC_TMPL 
#else
#define LIBSIGC_API __declspec(dllimport)
#define LIBSIGC_TMPL extern 
#endif /* LIBSIGC_EXPORTS */
#else
#define LIBSIGC_API
#endif /* LIBSIGC_WIN32 */


#endif /* _SIGC_CONFIG_H_ */
