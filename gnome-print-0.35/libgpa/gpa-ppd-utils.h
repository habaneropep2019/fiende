#ifndef __GNOME_PRINT_ADMIN_PPD_UTILS_H__
#define __GNOME_PRINT_ADMIN_PPD_UTILS_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

#define gpa_ppd_error g_warning
#define gpa_ppd_warning g_warning

gchar*   gpa_ppd_utils_get_string (const gchar *tag, GpaPpdInfo *ppd_info);

gchar*   gpa_ppd_utils_create_id (const gchar *name);

gboolean gpa_ppd_utils_get_ui (const gchar *name,
						 GpaPpdInfo *info,
						 gchar **ui_,
						 gchar **real_name,
						 gboolean jcl);

void gpa_ppd_append_syntax_error (GpaPpdInfo *ppd, gchar *messg, gint offset, gboolean fatal);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_UTILS_H__ */

