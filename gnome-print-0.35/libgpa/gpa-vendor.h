#ifndef __GNOME_PRINT_ADMIN_VENDOR_H__
#define __GNOME_PRINT_ADMIN_VENDOR_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

#include "gpa-structs.h"

#define GPA_IS_VENDOR(obj) (obj) /* For now check for NULL only */

GList * gpa_vendor_list_get (void);

/* Access to the structure */
const gchar * gpa_vendor_get_name (const GpaVendor *vendor);
      gchar * gpa_vendor_dup_name (const GpaVendor *vendor);
      GList * gpa_vendor_get_models_list (const GpaVendor *vendor);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_VENDOR_H__ */

