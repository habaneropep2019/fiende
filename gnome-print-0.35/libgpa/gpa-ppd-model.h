#ifndef __GNOME_PRINT_ADMIN_PPD_MODEL_H__
#define __GNOME_PRINT_ADMIN_PPD_MODEL_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

#include "gpa-private.h"

#define gpa_ppd_error g_warning

gboolean gpa_ppd_to_model (const gchar *source,
					  GpaModel **model_);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_MODEL_H__ */



