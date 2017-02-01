#ifndef _ACCONFIG_H
#define _ACCONFIG_H 1

/* package & version name */
#undef VERSION
#undef PACKAGE

/* feature tests that are used during library creation only */
#undef HAVE_WORKING_BASENAME

/* autoheader reqiures all defines inside acconfig.h, so all
   defines from g[dt]k--config.h are added here inside of #if 0 */
#if 0
#undef GNOMEMM_MAJOR_VERSION
#undef GNOMEMM_MICRO_VERSION
#undef GNOMEMM_MINOR_VERSION
#endif

/* autoheader adds more tests below this */
@TOP@
@BOTTOM@

#endif /* _ACCONFIG_H */
