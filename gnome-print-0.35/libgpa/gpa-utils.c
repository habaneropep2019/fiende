/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Some of this functions have been copied from gnumeric.
 *
 * Authors :
 *  Chema Celorio <chema@celorio.com>
 *  Gnumeric Hackers 
 */

#include "gpa-defs.h"

#include <glib.h>

#include "gpa-utils.h"
#include "gpa-private.h"


static void
gpa_hash_item_copy (gpointer key_in, gpointer value_in, gpointer data)
{
	GHashTable *new_values;
	gchar *key;
	gchar *value;
	gchar *new_key;
	gchar *new_value;

	key   = (gchar *) key_in;
	value = (gchar *) value_in;
	new_values = (GHashTable *) data;

	new_key   = g_strdup (key);
	new_value = g_strdup (value);

	g_hash_table_insert (new_values, new_key, new_value);

}	

GHashTable*
gpa_hash_copy (GHashTable *values)
{
	GHashTable *new_values;

	new_values = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_foreach (values, gpa_hash_item_copy, new_values);

	return new_values;
}

static gboolean
gpa_hash_item_free (gpointer key_in, gpointer value_in, gpointer data)
{
	gchar *key;
	gchar *value;

	key   = (gchar *) key_in;
	value = (gchar *) value_in;

	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	g_free (key);
	g_free (value);
	
	key   = NULL;
	value = NULL;

	return TRUE;
}	

gboolean 
gpa_hash_free (GHashTable *values)
{
	if (values == NULL)
		return TRUE;

	g_hash_table_foreach_remove (values, gpa_hash_item_free, NULL);

	g_hash_table_destroy (values);

	return TRUE;
}


static void
gpa_hash_item_write (gpointer key_in, gpointer value_in, gpointer data)
{
	xmlNodePtr item;
	xmlNodePtr node;
	gchar *key;
	gchar *value;

	key   = (gchar *) key_in;
	value = (gchar *) value_in;
	node  = (xmlNodePtr) data;

	item = xmlNewChild (node, NULL, key, value);

	/*
	gpa_xml_set_value (item, GPA_TAG_NAME, key);
	gpa_xml_set_value (item, GPA_TAG_CONTENT, value);
	*/

}	

xmlNodePtr
gpa_hash_write (XmlParseContext *context, GHashTable *values, const gchar *name)
{
	xmlNodePtr node;

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (values  != NULL, NULL);
	g_return_val_if_fail (name    != NULL, NULL);
		
	node = xmlNewDocNode (context->doc, context->ns, name, NULL);

	g_hash_table_foreach (values, gpa_hash_item_write, node);
	
	return node;
}

gboolean
gpa_hash_item_set (GHashTable *hash, const gchar *key, const gchar *content)
{
	g_return_val_if_fail (key     != NULL, FALSE);
	g_return_val_if_fail (content != NULL, FALSE);

	if (hash == NULL) {
		g_warning ("Hash value not found. Trying to set :\"%s\" Key:\"%s\"",
			   content, key);
		return FALSE;
	}

	/* FIXME: We need to free the string inside the hash */
	g_hash_table_remove (hash, key);
	g_hash_table_insert (hash, g_strdup (key), g_strdup (content));
	
	return TRUE;
}


const gchar *
gpa_hash_item_get (GHashTable *hash, const gchar *key)
{
	gchar *content;
	
	g_return_val_if_fail (key != NULL,    FALSE);

	if (hash == NULL) {
		g_warning ("Hash value not found. Key:\"%s\"",
			   key);
		return FALSE;
	}

	content = g_hash_table_lookup (hash, key);

	return content;
}

typedef struct {
	gboolean success;
	gint size;
	const GpaKnownNodes *nodes;
	const gchar *section;
} known_nodes_structure;

static void
gpa_hash_verify_item (gpointer key_in, gpointer value_in, gpointer data)
{
	known_nodes_structure *nodes_struct;
	gboolean success;
	gchar *key;
	gchar *value;
	gint size;
	gint n;

	debug (FALSE, "");

	key   = (gchar *) key_in;
	value = (gchar *) value_in;
	nodes_struct = (known_nodes_structure *) data;
	
	size    = nodes_struct->size;
	success = nodes_struct->success;

	for (n = 0; n < size; n++) {
		if (nodes_struct->nodes[n].name == NULL)
			break;
		if (strcmp (key, nodes_struct->nodes[n].name) == 0)
			break;
	}

	if (n == size) {
		gpa_error ("Invalid Value \"%s\", inside \"%s\"", key, nodes_struct->section);
		nodes_struct->success = FALSE;
	}
}

gboolean
gpa_hash_verify (GHashTable *hash,
		 const GpaKnownNodes *nodes,
		 gboolean xtra_flag,
		 const gchar *section)
{
	known_nodes_structure *nodes_struct;
	gboolean success = TRUE;
	gint size = 0;
	gint n;

	debug (FALSE, "");

	g_return_val_if_fail (hash != NULL, FALSE);

	/* First, verify that the values that are in the hash are Valid */
	while (nodes[size++].name != NULL);
	size--;
	
	nodes_struct = g_new (known_nodes_structure, 1);
	nodes_struct->success = TRUE;
	nodes_struct->nodes = nodes;
	nodes_struct->size  = size;
	nodes_struct->section = section;
	
	g_hash_table_foreach (hash, gpa_hash_verify_item, nodes_struct);
	
	success = nodes_struct->success;
	
	g_free (nodes_struct);

	/* Second, verify that the required values are in the hash */
	for (n = 0; n < size; n++) {
		if (!nodes[n].required)
			continue;
		if (nodes[n].xtra_flag || !xtra_flag) {
			if (g_hash_table_lookup (hash, nodes[n].name) ==NULL) {
				gpa_error ("Could not find the required value \"%s\" in the "
					   "\"%s\" hash\n",
					   nodes[n].name, section);
				return FALSE;
			}
		}
	}


	return success;
}


gchar *
gpa_utils_convert_date_to_string (time_t clock)
{
	struct tm *now;
	gchar *date;

	now = localtime (&clock);
		
	date = g_strdup_printf ("%04d%02d%02d%02d%02d%02d",
				now->tm_year + 1900,
				now->tm_mon + 1,
				now->tm_mday,
				now->tm_hour,
				now->tm_min,
				now->tm_sec);

	return date;
}

