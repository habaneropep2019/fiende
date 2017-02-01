#ifndef __GNOME_PRINT_ADMIN_PPD_OPTION_H__
#define __GNOME_PRINT_ADMIN_PPD_OPTION_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

gboolean
gpa_ppd_create_option_from_token (const gchar *token,
						    const gchar *token2,
						    GpaOptions *parent);


END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_OPTION_H__ */

