#ifndef __GNOME_PRINT_ADMIN_PPD_CODE_H__
#define __GNOME_PRINT_ADMIN_PPD_CODE_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

gboolean gpa_ppd_add_code_fragments (GpaPpdInfo *info);
gboolean gpa_ppd_add_options_code_fragment (GpaPpdInfo *info,
								    gchar *code,
								    GpaOptions *options);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_CODE_H__ */

