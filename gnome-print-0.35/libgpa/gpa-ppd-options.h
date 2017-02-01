#ifndef __GNOME_PRINT_ADMIN_PPD_OPTIONS_H__
#define __GNOME_PRINT_ADMIN_PPD_OPTIONS_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

#define gpa_ppd_error g_warning

gboolean gpa_ppd_add_options (GpaPpdInfo *ppd_info);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_OPTIONS_H__ */

