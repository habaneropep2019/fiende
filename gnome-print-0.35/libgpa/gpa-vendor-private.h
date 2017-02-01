#ifndef __GPA_VENDOR_PRIVATE_H__
#define __GPA_VENDOR_PRIVATE_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

#include "gpa-structs.h"

struct _GpaVendor {
	gchar *name;
	GList *models; /* List of GpaModels */
};

GpaVendor * gpa_vendor_get_from_name (const gchar *vendor_name);
GpaVendor * gpa_vendor_new (const gchar * name);

gboolean  gpa_vendor_list_load_all (void);

gboolean gpa_vendor_verify (GpaVendor *vendor);

END_GNOME_DECLS

#endif /* __GPA_VENDOR_PRIVATE_H__ */
