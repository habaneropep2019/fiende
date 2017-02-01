#ifndef __GNOME_PRINT_ADMIN_VALUES_H__
#define __GNOME_PRINT_ADMIN_VALUES_H__

BEGIN_GNOME_DECLS

/* This header is not installed */

#include <gpa-structs.h>
#include <gpa-private.h>
#include <gpa-settings-private.h> /* Contains the GpaValue struct (should not be that way) */
#include <xml-utils.h>

extern gboolean   debug_turned_on;

gboolean     gpa_values_verify_printer  (GHashTable *table, gboolean in_profile);
gboolean     gpa_values_verify_option   (GpaOption *option);
gboolean     gpa_values_verify_settings (GpaSettings *settings);

GpaValue * gpa_value_new (const gchar *key, const gchar *val);

GList *    gpa_values_copy_list (GList *list_in);
gboolean   gpa_values_free_list (GList *list_in);

xmlNodePtr gpa_values_write_list (XmlParseContext *context,
						    GList *list_in,
						    const gchar *node_name);
GList *    gpa_values_new_from_node (xmlNodePtr tree_, const gchar *node_name);

GList * gpa_value_insert (GList *list, const gchar *key, const gchar *value);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_VALUES_H__ */



