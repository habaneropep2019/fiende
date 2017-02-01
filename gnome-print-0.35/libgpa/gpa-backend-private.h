#ifndef __GNOME_PRINT_ADMIN_BACKEND_PRIVATE_H__
#define __GNOME_PRINT_ADMIN_BACKEND_PRIVATE_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"
#include "gpa-backend.h"
#include "xml-utils.h"

struct _GpaBackend
{
	gchar *id;
#if 0 /* Deprecate */
	gchar *driver;
#endif
	GHashTable *values;
	gboolean def;
};


GList *   gpa_backend_list_new_from_node (xmlNodePtr tree_);
xmlNodePtr gpa_backend_list_write (XmlParseContext *context, GList *list_);

GpaBackend * gpa_backend_new (void);


END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_BACKEND_PRIVATE_H__ */



