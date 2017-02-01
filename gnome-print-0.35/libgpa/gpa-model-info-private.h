#ifndef __GNOME_PRINT_ADMIN_MODEL_INFO_PRIVATE_H__
#define __GNOME_PRINT_ADMIN_MODEL_INFO_PRIVATE_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"
#include "xml-utils.h"

gboolean gpa_load_model_info_list_from_tree (XmlParseContext *context,
									xmlNodePtr tree,
									GList **model_info_list,
									GpaVendor *vendor);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_MODEL_INFO_PRIVATE_H__ */

