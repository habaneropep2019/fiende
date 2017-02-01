#ifndef __XML_UTILS_H__
#define __XML_UTILS_H__

#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/parserInternals.h>
#include <gnome-xml/xmlmemory.h>

typedef struct _XmlParseContext XmlParseContext;

struct _XmlParseContext {
	xmlDocPtr doc;
	xmlNsPtr  ns;
};


gboolean     gpa_xml_get_value_int          (xmlNodePtr node, const char *name, int *val);
gboolean     gpa_xml_get_value_int_required (xmlNodePtr node, const char *name, int *val);

gchar *      gpa_xml_get_value_string          (xmlNodePtr node, const char *name);
gchar *      gpa_xml_get_value_string_required (xmlNodePtr node,
					    const char *name,
					    const char *xtra_info);

xmlNodePtr   gpa_xml_search_child          (xmlNodePtr node, const char *name);
xmlNodePtr   gpa_xml_search_child_required (xmlNodePtr tree, const gchar* name);

gboolean     gpa_xml_node_verify (xmlNodePtr node, const gchar *name);

void         gpa_xml_set_value (xmlNodePtr node, const char *name, const char *val);

/* Parse Context */
XmlParseContext * gpa_xml_parse_context_new     (xmlDocPtr doc, xmlNsPtr name_space);
void              gpa_xml_parse_context_destroy (XmlParseContext *context);

gboolean          gpa_xml_parse_context_free (XmlParseContext *context);
XmlParseContext * gpa_xml_parse_context_new_from_path (const gchar *full_path,
										 const gchar *nspace,
										 const gchar *root_name);


/* Hash */
GHashTable * gpa_xml_utils_new_hash_from_node (xmlNodePtr tree, const gchar *hash_type);

xmlNodePtr gpa_xml_utils_hash_write (XmlParseContext *context,
						   GHashTable *hash,
						   const gchar *name);

#endif /* __XML_UTILS_H__ */
