#ifndef __GNOME_PRINT_ADMIN_UTILS_H__
#define __GNOME_PRINT_ADMIN_UTILS_H__

BEGIN_GNOME_DECLS

#include <xml-utils.h>
#include <gpa-private.h>
#include <gpa-known.h>
#include <time.h>   /* For time() */

extern gboolean   debug_turned_on;

/* Hash handling */
GHashTable* gpa_hash_copy (GHashTable *values);
gboolean    gpa_hash_free (GHashTable *values);
xmlNodePtr  gpa_hash_write (XmlParseContext *context,
					   GHashTable *values,
					   const gchar *name);
gboolean      gpa_hash_item_set (GHashTable *hash,
						   const gchar *key,
						   const gchar *content);
const gchar * gpa_hash_item_get (GHashTable *hash,
						   const gchar *key);


gboolean gpa_hash_verify (GHashTable *hash,
					 const GpaKnownNodes *nodes,
					 gboolean xtra_flag,
					 const gchar *section);

gchar * gpa_utils_convert_date_to_string (time_t clock);

/*
gboolean
gp_hash_verify (GHashTable *hash, const GpaKnownNodes nodes, gboolean xtra_flag);
*/


END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_UTILS_H__ */
