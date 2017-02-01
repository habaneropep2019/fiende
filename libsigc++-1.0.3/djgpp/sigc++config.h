/* Configure for libsigc++ with djgpp */
#ifndef _SIGC_CONFIG_H_
#define _SIGC_CONFIG_H_

#if 0
#define PACKAGE "libsigc++"

#endif

/* define only for Microsoft VC++ */
/* #define LIBSIGC_WIN32 */

#define SIGC_MAJOR_VERSION 0
#define SIGC_MINOR_VERSION 8
#define SIGC_MICRO_VERSION 2

/* #define SIGC_CXX_NAMESPACES 1   */
#define SIGC_CXX_PARTIAL_SPEC 1 /**/
#define SIGC_CXX_MEMBER_FUNC_TEMPLATES 1 /**/
#define SIGC_CXX_MEMBER_CLASS_TEMPLATES 1 /**/
#define SIGC_CXX_MUTABLE 1 /**/
/* #undef SIGC_CXX_FRIEND_TEMPLATES */
#define SIGC_CXX_TEMPLATE_CCTOR 1
/* #define SIGC_CXX_INT_CTOR 1 */

/* #define SIGC_PTHREADS 1 */
/* #undef SIGC_PTHREAD_DCE */

#ifdef LIBSIGC_WIN32
#ifdef LIBSIGC_EXPORTS
#define LIBSIGC_API __declspec(dllexport)
#define LIBSIGC_TMPL
#else
#define LIBSIGC_API __declspec(dllimport)
#define LIBSIGC_TMPL extern
#endif
#else
#define LIBSIGC_API
#endif
    
#endif /* _SIGC_CONFIG_H_ */
