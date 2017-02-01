/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This functions are based on code from gnumeric/xml-io.c  */
/* Authors:
 *   Daniel Veillard <Daniel.Veillard@w3.org>
 *   Miguel de Icaza <miguel@gnu.org> */

#include "gpa-defs.h"
#include <glib.h>

#include "xml-utils.h"
#include "gpa-include.h"


#define skip_text(node) if (strcmp ( ((xmlNodePtr)node)->name, "text") == 0) { \
			node = ((xmlNodePtr)node)->next; continue ; };


/*
 * Set a string value for a node either carried as an attibute or as
 * the content of a child.
 */
void
gpa_xml_set_value (xmlNodePtr node, const char *name, const char *val)
{
	char *ret;
	xmlNodePtr child;

	ret = xmlGetProp (node, name);
	if (ret != NULL){
		xmlFree (ret);
		xmlSetProp (node, name, val);
		return;
	}
	child = node->childs;
	while (child != NULL){
		if (!strcmp (child->name, name)){
			xmlNodeSetContent (child, val);
			return;
		}
		child = child->next;
	}
	xmlSetProp (node, name, val);
}

/*
 * Get a value for a node either carried as an attibute or as
 * the content of a child.
 *
 * Returns a g_malloc'ed string.  Caller must free.
 * (taken from gnumeric )
 *
 */
static char *
gpa_xml_get_value (xmlNodePtr node, const char *name)
{
	char *ret, *val;
	xmlNodePtr child;

	val = (char *) xmlGetProp (node, name);
	if (val != NULL) {
		ret = g_strdup (val);
		xmlFree (val);
		return ret;
	}
	child = node->childs;

	while (child != NULL) {
		if (!strcmp (child->name, name)) {
		        /*
			 * !!! Inefficient, but ...
			 */
			val = xmlNodeGetContent(child);
			if (val != NULL) {
				ret = g_strdup (val);
				xmlFree (val);
				return ret;
			}
		}
		child = child->next;
	}

	return NULL;
}



gboolean
gpa_xml_node_verify (xmlNodePtr node, const gchar *name)
{
	g_return_val_if_fail (node != NULL, FALSE);
	
	if (strcmp (node->name, name) != 0) {
		g_warning  ("Expected node was \"%s\", encountered \"%s\"",
			    name, node->name);
		return FALSE;
	}
	return TRUE;
}


/*
 * Get an integer value for a node either carried as an attibute or as
 * the content of a child.
 */
gboolean
gpa_xml_get_value_int (xmlNodePtr node, const char *name, int *val)
{
	char *ret;
	int i;
	int res;

	ret = gpa_xml_get_value (node, name);
	if (ret == NULL) return 0;
	res = sscanf (ret, "%d", &i);
	g_free (ret);

	if (res == 1) {
	        *val = i;
		return TRUE;
	}
	return FALSE;
}

/**
 * gpa_xml_get_value_int_required:
 * @node: 
 * @name: 
 * @val: 
 * 
 * A wrapper arround get_value_int that displays a warning if the
 * node did not contained the requested tag
 * 
 * Return Value: 
 **/
gboolean
gpa_xml_get_value_int_required (xmlNodePtr node, const char *name, int *val)
{
	gboolean ret;
	
	ret = gpa_xml_get_value_int (node, name, val);

	if (ret == FALSE)
		g_warning ("The file did not contained the required value \"%s\"\n"
			   "Under the \"%s\" tag.", name, node->name);
			
	return ret;
}


/*
 * Get a String value for a node either carried as an attibute or as
 * the content of a child.
 */
gchar *
gpa_xml_get_value_string (xmlNodePtr node, const char *name)
{
	return gpa_xml_get_value (node, name);
}

gchar *
gpa_xml_get_value_string_required (xmlNodePtr node, const char *name, const char *xtra)
{
	gchar *value = gpa_xml_get_value (node, name);

	if (value == NULL) {
		if (xtra == NULL)
			g_warning ("The file did not contained the required value \"%s\"\n"
				   "Under the \"%s\" tag.", name, node->name);
		else
			g_warning ("The file did not contained the required value \"%s\"\n"
				   "Under the \"%s\" tag (%s).", name, node->name, xtra);
	}
	return value;
}



/*
 * Search a child by name,
 */
xmlNodePtr
gpa_xml_search_child (xmlNodePtr node, const char *name)
{
	xmlNodePtr child;

	child = node->childs;
	while (child != NULL){
		if (!strcmp (child->name, name))
			return child;
		child = child->next;
	}

	return NULL;
}

/**
 * gpa_xml_search_child_required:
 * @tree: 
 * @name: 
 * 
 * just a small wrapper arround gpa_xml_searc_hchild that displays
 * an error if the child was not found
 *
 * Return Value: 
 **/
xmlNodePtr
gpa_xml_search_child_required (xmlNodePtr tree, const gchar* name)
{
	xmlNodePtr child;
	
	child = gpa_xml_search_child (tree, name);

	if (child == NULL)
		g_warning ("The file did not contained the required tag \"%s\"\n"
			   "Under the \"%s\" node.", name, tree->name);

	return child;
}
	




static void
gpa_xml_utils_new_hash_item_from_node (xmlNodePtr tree,
				   gchar **key_,
				   gchar **value_,
				   const gchar *hash_type)
{
	gchar *key;
	gchar *value;

	*key_   = NULL;
	*value_ = NULL;

	key = g_strdup (tree->name);
	value = xmlNodeGetContent(tree);

	*key_   = key;
	*value_ = value;

	if ((key == NULL) || (value == NULL))
		g_warning ("Could not load item for hash \"%s\"\n", hash_type);
}

GHashTable *
gpa_xml_utils_new_hash_from_node (xmlNodePtr tree_, const gchar *hash_type)
{
	GHashTable *hash;
	xmlNodePtr node;
	xmlNodePtr tree;
	gchar *key;
	gchar *value;

	g_return_val_if_fail (tree_ != NULL, NULL);
	g_return_val_if_fail (hash_type != NULL, NULL);

	tree = gpa_include_node (tree_);
	
	if (!gpa_xml_node_verify (tree, hash_type))
		return NULL;
		
	hash = g_hash_table_new (g_str_hash, g_str_equal);
	/* This is not an error, since the hash can be empty */
	if (tree->childs == NULL)
		return hash;

	node = tree->childs;
	while (node != NULL) {
		skip_text (node);
		gpa_xml_utils_new_hash_item_from_node (node, &key, &value, hash_type);
		if ((key == NULL) || (value == NULL))
			return NULL;
		g_hash_table_insert (hash, key, value);
		node = node->next;
	}

	return hash;
}


static void
gpa_xml_utils_hash_item_write (gpointer key_in, gpointer value_in, gpointer data)
{
	xmlNodePtr item;
	xmlNodePtr node;
	gchar *key;
	gchar *value;

	key   = (gchar *) key_in;
	value = (gchar *) value_in;
	node  = (xmlNodePtr) data;
	
	item = xmlNewChild (node, NULL, key, value);

	if (item == NULL)
		g_warning ("Could not add the key \"%s\" with value \"%s\" to the tree",
			   key, value);
}	

xmlNodePtr
gpa_xml_utils_hash_write (XmlParseContext *context, GHashTable *hash, const gchar *name)
{
	xmlNodePtr node;
	
	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (hash    != NULL, NULL);

	node = xmlNewDocNode (context->doc, context->ns, name, NULL);

	g_hash_table_foreach (hash,
			      gpa_xml_utils_hash_item_write,
			      node);

	return node;
}


/* --------------------------- Parse Context ----------------------------*/
XmlParseContext *
gpa_xml_parse_context_new (xmlDocPtr             doc,
		       xmlNsPtr              ns)
{
	XmlParseContext *context = g_new0 (XmlParseContext, 1);

	context->doc = doc;
	context->ns  = ns;
	
	return context;
}

void
gpa_xml_parse_context_destroy (XmlParseContext *context)
{
	g_return_if_fail (context != NULL);
	g_free (context);
}


XmlParseContext *
gpa_xml_parse_context_new_from_path (const gchar *full_path,
				 const gchar *nspace,
				 const gchar *root_name)
{
	XmlParseContext *context;
	xmlDocPtr doc;
	xmlNsPtr  name_space;

	g_return_val_if_fail (full_path != NULL, NULL);
	
	doc = xmlParseFile (full_path);
	
	if (doc == NULL) {
		g_warning ("File not found or file with errors [%s]", full_path);
		return NULL;
	}
	if (doc->root == NULL) {
		g_warning ("Invalid xml File, tree empty [%s]", full_path);
		xmlFreeDoc (doc);
		return NULL;
	}

	name_space = xmlSearchNsByHref (doc, doc->root, nspace);
	if (name_space == NULL) {
		g_warning ("The file did not contained the expected root name\n"
			   "Expected \"%s\" [%s]",
			   nspace, full_path);
		xmlFreeDoc (doc);
		return NULL;
	}
	
	if ((doc->root->name == NULL) || (strcmp (doc->root->name, root_name)!=0)) {
		g_warning ("The file did not contained the expected root name\n"
			   "Expected \"%s\", actual : \"%s\" [%s]",
			   root_name, doc->root->name, full_path);
		xmlFreeDoc (doc);
		return FALSE;
	}
	context = gpa_xml_parse_context_new (doc, name_space);

	return context;
}

gboolean
gpa_xml_parse_context_free (XmlParseContext *context)
{
	g_return_val_if_fail (context != NULL, FALSE);

	xmlFreeDoc (context->doc);
	gpa_xml_parse_context_destroy (context);

	return TRUE;
}


