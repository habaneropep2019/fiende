/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-node.c: Code to manipulate BonoboUINode objects
 *
 * Author:
 *	Havoc Pennington <hp@redhat.com>
 *
 * Copyright 2000 Red Hat, Inc.
 */

#include "config.h"
#include <bonobo/bonobo-ui-node.h>
#include <stdlib.h>
#include <string.h>

#include <gnome-xml/parser.h>
#include <gnome-xml/parserInternals.h>
#include <gnome-xml/xmlmemory.h>

/* Having this struct here makes debugging nicer. */
struct _BonoboUINode {
	xmlNode real_node;
};

#define XML_NODE(x) (&(x)->real_node)
#define BNODE(x) ((BonoboUINode *)(x))

/**
 * bonobo_ui_node_new:
 * @name: The name for the node
 * 
 * Creates a new node with name @name
 * 
 * Return value: a new node pointer
 **/
BonoboUINode*
bonobo_ui_node_new (const char   *name)
{
        return BNODE (xmlNewNode (NULL, name));
}

/**
 * bonobo_ui_node_new_child:
 * @parent: the parent
 * @name: the name of the new child
 * 
 * Create a new node as a child of @parent with name @name
 * 
 * Return value: pointer to the new child
 **/
BonoboUINode*
bonobo_ui_node_new_child (BonoboUINode *parent,
                          const char   *name)
{
        return BNODE (xmlNewChild (XML_NODE (parent), NULL, name, NULL));
}

/**
 * bonobo_ui_node_copy:
 * @node: the node
 * @recursive: whether to dup children too.
 * 
 * Copy an XML node, if @recursive do a deep copy, otherwise just dup the node itself.
 * 
 * Return value: a copy of the node
 **/
BonoboUINode*
bonobo_ui_node_copy (BonoboUINode *node,
                     gboolean recursive)
{
        return BNODE (xmlCopyNode (XML_NODE (node), recursive));
}

/**
 * bonobo_ui_node_free:
 * @node: a node.
 * 
 * Frees the memory associated with the @node and unlink it from the tree
 **/
void
bonobo_ui_node_free (BonoboUINode *node)
{
        xmlFreeNode (XML_NODE (node));
}

/**
 * bonobo_ui_node_set_data:
 * @node: the node
 * @data: user data
 * 
 * Associates some user data with the node pointer
 **/
void
bonobo_ui_node_set_data (BonoboUINode *node,
                         gpointer      data)
{
        XML_NODE (node)->_private = data;
}

/**
 * bonobo_ui_node_get_data:
 * @node: the node
 * 
 * Gets user data associated with @node
 * 
 * Return value: the user data, see bonobo_ui_node_set_data
 **/
gpointer
bonobo_ui_node_get_data (BonoboUINode *node)
{
        return XML_NODE (node)->_private;
}

static xmlAttrPtr
get_attr (xmlNode *node, const char *name)
{
        xmlAttrPtr prop;

        if ((node == NULL) || (name == NULL)) return(NULL);
        /*
         * Check on the properties attached to the node
         */
        prop = node->properties;
        while (prop != NULL) {
                if (!xmlStrcmp(prop->name, name))  {
                        return(prop);
                }
                prop = prop->next;
      }
        
      return(NULL);
}

/**
 * bonobo_ui_node_set_attr:
 * @node: The node
 * @name: the name of the attr
 * @value: the value for the attr
 * 
 * Set the attribute of @name on @node to @value overriding any
 * previous values of that attr.
 **/
void
bonobo_ui_node_set_attr (BonoboUINode *node,
                         const char   *name,
                         const char   *value)
{
        if (value == NULL) {
                xmlAttrPtr attr = get_attr (XML_NODE (node), name);
                if (attr)
                        xmlRemoveProp (attr);
        } else {
                xmlSetProp (XML_NODE (node), name, value);
        }
}

/**
 * bonobo_ui_node_get_attr:
 * @node: the node
 * @name: the name of the attr to get
 * 
 * Fetch the value of an attr of name @name from @node
 * see also: bonobo_ui_node_free_string
 * 
 * Return value: the attr text.
 **/
char*
bonobo_ui_node_get_attr (BonoboUINode *node,
                         const char   *name)
{
        return xmlGetProp (XML_NODE (node), name);
}

/**
 * bonobo_ui_node_has_attr:
 * @node: the node
 * @name: the name of the attr to detect
 * 
 * Determines whether the @node has an attribute of name @name
 * 
 * Return value: TRUE if the attr exists
 **/
gboolean
bonobo_ui_node_has_attr (BonoboUINode *node,
                         const char   *name)
{
        return get_attr (XML_NODE (node), name) != NULL;
}

/**
 * bonobo_ui_node_remove_attr:
 * @node: the node
 * @name: name of the attribute
 * 
 * remove any attribute with name @name from @node
 **/
void
bonobo_ui_node_remove_attr (BonoboUINode *node,
                            const char   *name)
{
        xmlAttrPtr attr = get_attr (XML_NODE (node), name);
        if (attr)
                xmlRemoveProp (attr);
}

/**
 * bonobo_ui_node_add_child:
 * @parent: the parent
 * @child: the new child
 * 
 * Add a @child node to the @parent node ( after the other children )
 **/
void
bonobo_ui_node_add_child (BonoboUINode *parent,
			  BonoboUINode *child)
{
        xmlAddChild (XML_NODE (parent), XML_NODE (child));
}

/**
 * bonobo_ui_node_insert_before:
 * @sibling: the node to insert
 * @prev_sibling: the placeholder for insertion
 * 
 * Insert a @sibling before @prev_sibling in a node list
 **/
void
bonobo_ui_node_insert_before (BonoboUINode *sibling,
                              BonoboUINode *prev_sibling)
{
        xmlAddPrevSibling (XML_NODE (sibling), XML_NODE (prev_sibling));
}

/**
 * bonobo_ui_node_unlink:
 * @node: the node
 * 
 * Unlink @node from its tree, ie. disassociate it with its parent
 **/
void
bonobo_ui_node_unlink (BonoboUINode *node)
{
	xmlUnlinkNode (XML_NODE (node));
}

/**
 * bonobo_ui_node_replace:
 * @old_node: node to be replaced
 * @new_node: node to replace with
 * 
 * Replace @old_node with @new_node in the tree. @old_node is
 * left unlinked and floating with its children.
 **/
void
bonobo_ui_node_replace (BonoboUINode *old_node,
			BonoboUINode *new_node)
{
	/* libxml has these args indisputably backward */
	xmlReplaceNode (XML_NODE (new_node),
			XML_NODE (old_node));
}

/**
 * bonobo_ui_node_set_content:
 * @node: the node
 * @content: the new content
 * 
 * Set the textual content of @node to @content
 **/
void
bonobo_ui_node_set_content (BonoboUINode *node,
                            const char   *content)
{
        xmlNodeSetContent (XML_NODE (node), content);
}

/**
 * bonobo_ui_node_get_content:
 * @node: the node
 * 
 * see also: bonobo_ui_node_free_string
 *
 * Return value: the content of @node
 **/
char *
bonobo_ui_node_get_content (BonoboUINode *node)
{
        return xmlNodeGetContent (XML_NODE (node));
}

/**
 * bonobo_ui_node_next:
 * @node: the node
 *
 * accesses the next node.
 *
 * Return value: the node after @node in the list
 **/
BonoboUINode*
bonobo_ui_node_next (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->next);
}

/**
 * bonobo_ui_node_prev:
 * @node: the node
 * 
 * accesses the previous node.
 *
 * Return value: the node before @node in the list
 **/
BonoboUINode*
bonobo_ui_node_prev (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->prev);
}

/**
 * bonobo_ui_node_children:
 * @node: the node
 * 
 * accesses the node's children.
 *
 * Return value: the first child of @node
 **/
BonoboUINode*
bonobo_ui_node_children (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->xmlChildrenNode);
}

/**
 * bonobo_ui_node_parent:
 * @node: the node
 *
 * accesses the node's parent.
 * 
 * Return value: the parent node of @node
 **/
BonoboUINode*
bonobo_ui_node_parent (BonoboUINode *node)
{
        return BNODE (XML_NODE (node)->parent);
}

/**
 * bonobo_ui_node_get_name:
 * @node: the node
 * 
 * Return value: the name of @node
 **/
const char*
bonobo_ui_node_get_name (BonoboUINode *node)
{
        return XML_NODE (node)->name;
}

/**
 * bonobo_ui_node_has_name:
 * @node: the node
 * @name: a name the node might have
 * 
 * accesses the node's name.
 * 
 * Return value: TRUE if @node has name == @name
 **/
gboolean
bonobo_ui_node_has_name (BonoboUINode *node,
			 const char   *name)
{
        return strcmp (XML_NODE (node)->name, name) == 0;
}

/**
 * bonobo_ui_node_free_string:
 * @str: the string to free.
 * 
 * Frees a string returned by any of the get routines.
 **/
void
bonobo_ui_node_free_string (char *str)
{
        if (str)
                xmlFree (str);
}

/**
 * bonobo_ui_node_to_string:
 * @node: the node tree
 * @recurse: whether to dump its children as well
 * 
 * Convert the Node to its XML string representation
 * see also: bonobo_ui_node_free_string
 * 
 * Return value: the string representation or NULL on error
 **/
char *
bonobo_ui_node_to_string (BonoboUINode *node,
			  gboolean      recurse)
{
	xmlDoc     *doc;
	xmlChar    *mem = NULL;
	int         size;

	doc = xmlNewDoc ("1.0");
	g_return_val_if_fail (doc != NULL, NULL);

	doc->xmlRootNode = XML_NODE(bonobo_ui_node_copy (node, TRUE));
	g_return_val_if_fail (doc->xmlRootNode != NULL, NULL);

	if (!recurse && bonobo_ui_node_children (BNODE (doc->xmlRootNode))) {
		BonoboUINode *tmp;
		while ((tmp = bonobo_ui_node_children (BNODE (doc->xmlRootNode)))) {
			xmlUnlinkNode (XML_NODE(tmp));
			bonobo_ui_node_free (tmp);
		}
	}

	xmlDocDumpMemory (doc, &mem, &size);

	g_return_val_if_fail (mem != NULL, NULL);

	xmlFreeDoc (doc);

	return mem;
}

/**
 * bonobo_ui_node_from_string:
 * @xml: the xml string
 * 
 * Parses a string into an XML tree
 * 
 * Return value: the xml tree.
 **/
BonoboUINode*
bonobo_ui_node_from_string (const char *xml)
{
	/* We have crap error reporting for this function */
	xmlDoc  *doc;
	BonoboUINode *node;
	
	doc = xmlParseDoc ((char *)xml);
	if (!doc)
		return NULL;
	
	node = BNODE (doc->xmlRootNode);
	bonobo_ui_node_strip (&node);

	xmlUnlinkNode (XML_NODE (node));
	doc->xmlRootNode = NULL;
	
	xmlFreeDoc (doc);

	return node;
}

/**
 * bonobo_ui_node_from_file:
 * @fname: the filename containing the xml
 * 
 * Loads and parses the filename into an XML tree
 * 
 * Return value: the xml tree.
 **/
BonoboUINode*
bonobo_ui_node_from_file (const char *fname)
{
	/* Error reporting blows here too (because it blows
	 * in libxml)
	 */
	xmlDoc  *doc;
	BonoboUINode *node;

	g_return_val_if_fail (fname != NULL, NULL);
	
	doc = xmlParseFile (fname);

	g_return_val_if_fail (doc != NULL, NULL);

	node = BNODE (doc->xmlRootNode);
	bonobo_ui_node_strip (&node);

	xmlUnlinkNode (XML_NODE (node));
	doc->xmlRootNode = NULL;

	xmlFreeDoc (doc);

	return node;
}

/**
 * bonobo_ui_node_transparent:
 * @node: the node
 * 
 * Determines whether @node is transparent. A node is
 * transparent if it has no content and either no attributes
 * or a single 'name' attribute.
 * 
 * Return value: TRUE if transparent
 **/
gboolean
bonobo_ui_node_transparent (BonoboUINode *node)
{
	xmlNode *n = XML_NODE (node);
	gboolean ret = FALSE;

	g_return_val_if_fail (n != NULL, TRUE);

	if (n->content) {
		ret = FALSE;

	} else if (!n->properties) {
		if (!strcmp (XML_NODE (node)->name, "placeholder"))
			ret = TRUE;
		else if (!strcmp (XML_NODE (node)->name, "menu"))
			ret = TRUE;

	} else if (!n->properties->next) {
		if (!strcmp (n->properties->name, "name"))
			ret = TRUE;
	}

	return ret;
}

/**
 * bonobo_ui_node_copy_attrs:
 * @src: the attr source node
 * @dest: where to dump the attrs.
 * 
 * This function copies all the attributes from @src to @dest
 * effectively cloning the @src node as @dest
 **/
void
bonobo_ui_node_copy_attrs (BonoboUINode *src,
			   BonoboUINode *dest)
{
	xmlAttr *attr;
	
	for (attr = XML_NODE (src)->properties; attr; attr = attr->next) {
		char *txt = xmlGetProp (XML_NODE (src), attr->name);

		g_assert (txt != NULL);

		xmlSetProp (XML_NODE (dest), attr->name, txt);

		xmlFree (txt);
	}
}

static gboolean
do_strip (xmlNode *node)
{
        xmlNode *l, *next;
	gboolean suspicious = FALSE;

	if (!node)
		return FALSE;

	switch (node->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE:
	case XML_TEXT_NODE:
        case XML_ENTITY_NODE:
        case XML_ENTITY_REF_NODE: {
		xmlAttr *a, *nexta;

		node->nsDef = NULL;
		node->ns = NULL;
		node->doc = NULL;

		for (a = node->properties; a; a = nexta) {
			nexta = a->next;
			a->ns = NULL;
			do_strip (a->val);
		}

		for (l = node->xmlChildrenNode; l; l = next) {
			next = l->next;
			do_strip (l);
		}
		break;
	}

	case XML_ATTRIBUTE_NODE: {
		xmlAttr *attr = (xmlAttr *)node;
		attr->ns = NULL;
		do_strip (attr->val);
		break;
	}

        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
        case XML_NOTATION_NODE:
        case XML_CDATA_SECTION_NODE:
		suspicious = TRUE;
		break;
	}

	if (suspicious) {
/*		g_warning ("node looks suspicious %d: '%s'",
			   node->type,
			   bonobo_ui_node_to_string (BNODE (node), TRUE));*/
		xmlUnlinkNode (node);
		bonobo_ui_node_free (BNODE (node));
		return TRUE;
	} else
		return FALSE;
}

/**
 * bonobo_ui_node_strip:
 * @node: a pointer to the node's pointer
 * 
 *   This function is used to purge unwanted content from
 * a set of nodes, and particularly clean up stray Doc and
 * NS pointers that cause serious trouble later.
 **/
void
bonobo_ui_node_strip (BonoboUINode **node)
{
	BonoboUINode *next, *l;

	for (l = *node; l; l = next) {
		next = bonobo_ui_node_next (l);
		if (l == *node && do_strip (XML_NODE (l)))
			*node = next;
	}
}
