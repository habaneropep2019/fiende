#ifndef __GNOME_PRINT_ADMIN_MODEL_INFO_H__
#define __GNOME_PRINT_ADMIN_MODEL_INFO_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"

#define GPA_IS_MODEL_INFO(obj) (obj) /* For now check for NULL only */

extern gboolean debug_turned_on;

GpaModelInfo * gpa_model_info_new (const gchar *name,
								    const gchar *id,
								    const GpaVendor *vendor);

/* Access to the struct */
    const gchar * gpa_model_info_get_name (GpaModelInfo *mi);
          gchar * gpa_model_info_dup_name (GpaModelInfo *mi);
    const gchar * gpa_model_info_get_id (GpaModelInfo *mi);
const GpaVendor * gpa_model_info_get_vendor (GpaModelInfo *mi);


END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_MODEL_INFO_H__ */

