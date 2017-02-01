#ifndef __GNOME_PRINT_ADMIN_BACKEND_H__
#define __GNOME_PRINT_ADMIN_BACKEND_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"

extern gboolean   debug_turned_on;

#define GPA_IS_BACKEND(obj) (obj) /* For now check for NULL only */


gboolean   gpa_backend_list_free (GList *backends);
gboolean   gpa_backend_list_verify (GList *backends);
GList *   gpa_backend_list_copy (GList *backends);
GpaBackend * gpa_backend_get_default (GpaModel *model);
GpaBackend * gpa_backend_get_from_id (GpaModel *model,
							   const gchar *id);
GpaBackend * gpa_backend_get_selected (GpaSettings *settings);

/* Access to the struct */ 
const gchar * gpa_backend_get_id (GpaBackend *backend);



END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_BACKEND_H__ */



