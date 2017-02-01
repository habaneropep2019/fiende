/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-enumeration.c: Enumeration data structure.

   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>

#include "eel-enumeration.h"

#include "eel-glib-extensions.h"
#include "eel-lib-self-check-functions.h"
#include "eel-string.h"

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>

static gboolean suppress_duplicate_registration_warning;

struct EelEnumeration
{
	char *id;
	EelStringList *names;
	EelStringList *descriptions;
	GList *values;
};

/**
 * eel_enumeration_new:
 *
 * Return value: A newly constructed empty enumeration.
 */
EelEnumeration *
eel_enumeration_new (const char *id)
{
	EelEnumeration *enumeration;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '\0', NULL);
	
	enumeration = g_new0 (EelEnumeration, 1);

	enumeration->id = g_strdup (id);

	return enumeration;
}

EelEnumeration *
eel_enumeration_copy (const EelEnumeration *enumeration)
{
	EelEnumeration *copy;

	g_return_val_if_fail (enumeration != NULL, NULL);

	copy = g_new0 (EelEnumeration, 1);

	copy->id = g_strdup (enumeration->id);
	copy->names = eel_string_list_copy (enumeration->names);
	copy->descriptions = eel_string_list_copy (enumeration->descriptions);
	copy->values = g_list_copy (enumeration->values);

	return copy;
}

void
eel_enumeration_free (EelEnumeration *enumeration)
{
	if (enumeration == NULL) {
		return;
	}
	
	eel_string_list_free (enumeration->names);
	eel_string_list_free (enumeration->descriptions);
	g_list_free (enumeration->values);
	g_free (enumeration->id);
	g_free (enumeration);
}

void
eel_enumeration_insert (EelEnumeration *enumeration,
			const char *name,
			const char *description,
			int value)
{
	g_return_if_fail (enumeration != NULL);
	g_return_if_fail (name != NULL);

	if (enumeration->names == NULL) {
		enumeration->names = eel_string_list_new (TRUE);
	}
	
	if (enumeration->descriptions == NULL) {
		enumeration->descriptions = eel_string_list_new (TRUE);
	}

	eel_string_list_insert (enumeration->names, name);
	eel_string_list_insert (enumeration->descriptions, description ? description : "");
	enumeration->values = g_list_append (enumeration->values, GINT_TO_POINTER (value));
}

char *
eel_enumeration_get_id (const EelEnumeration *enumeration)
{
	g_return_val_if_fail (enumeration != NULL, NULL);

	return g_strdup (enumeration->id);
}

char *
eel_enumeration_get_nth_name (const EelEnumeration *enumeration,
			      guint n)
{
	g_return_val_if_fail (enumeration != NULL, NULL);
	g_return_val_if_fail (n < eel_string_list_get_length (enumeration->names), NULL);

	return eel_string_list_nth (enumeration->names, n);
}

char *
eel_enumeration_get_nth_description (const EelEnumeration *enumeration,
				     guint n)
{
	g_return_val_if_fail (enumeration != NULL, NULL);
	g_return_val_if_fail (n < eel_string_list_get_length (enumeration->descriptions), NULL);

	return eel_string_list_nth (enumeration->descriptions, n);
}

char *
eel_enumeration_get_nth_description_translated (const EelEnumeration *enumeration,
						guint n)
{
	char *untranslated_description;
	const char *translated_description;

	g_return_val_if_fail (enumeration != NULL, NULL);
	g_return_val_if_fail (n < eel_string_list_get_length (enumeration->descriptions), NULL);

	untranslated_description = eel_string_list_nth (enumeration->descriptions, n);
	g_return_val_if_fail (untranslated_description != NULL, NULL);

	translated_description = gettext (untranslated_description);

 	/* If not translation is found, return untranslated property as-is. */
 	if (translated_description == untranslated_description) {
 		return untranslated_description;
 	}

	g_free (untranslated_description);

	return g_strdup (translated_description);
}

int
eel_enumeration_get_nth_value (const EelEnumeration *enumeration,
			       guint n)
{
	g_return_val_if_fail (enumeration != NULL, 0);
	g_return_val_if_fail (n < g_list_length (enumeration->values), 0);
	
	return GPOINTER_TO_INT (g_list_nth_data (enumeration->values, n));
}

guint
eel_enumeration_get_length (const EelEnumeration *enumeration)
{
	g_return_val_if_fail (enumeration != NULL, 0);

	return eel_string_list_get_length (enumeration->names);
}

EelEnumeration *
eel_enumeration_new_from_tokens (const char *id,
				 const char *names,
				 const char *descriptions,
				 const char *values,
				 const char *delimiter)
{
	EelEnumeration *enumeration;
	EelStringList *name_list;
	EelStringList *description_list;
	EelStringList *value_list;
	guint i;
	int value;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '\0', NULL);
	g_return_val_if_fail (names != NULL, NULL);
	g_return_val_if_fail (names[0] != '\0', NULL);
	g_return_val_if_fail (values != NULL, NULL);
	g_return_val_if_fail (values[0] != '\0', NULL);
	g_return_val_if_fail (delimiter != NULL, NULL);
	g_return_val_if_fail (delimiter[0] != '\0', NULL);

	enumeration = eel_enumeration_new (id);

	name_list = eel_string_list_new_from_tokens (names, delimiter, TRUE);
	value_list = eel_string_list_new_from_tokens (values, delimiter, TRUE);

	if (eel_string_list_get_length (name_list)
	    != eel_string_list_get_length (value_list)) {
		g_warning ("names and values have different lengths.");
		eel_string_list_free (name_list);
		eel_string_list_free (value_list);
		return NULL;
	}

	description_list = 
		descriptions != NULL ? 
		eel_string_list_new_from_tokens (descriptions, delimiter, TRUE) :
		NULL;

	if (description_list != NULL) {
		if (eel_string_list_get_length (name_list)
		    != eel_string_list_get_length (description_list)) {
			g_warning ("names and descriptions have different lengths.");
			eel_string_list_free (name_list);
			eel_string_list_free (value_list);
			eel_string_list_free (description_list);
			return NULL;
		}
	}

	enumeration->names = name_list;

	if (description_list == NULL) {
		description_list = eel_string_list_new (TRUE);

		for (i = 0; i < eel_string_list_get_length (name_list); i++) {
			eel_string_list_insert (description_list, "");
		}
	}
	
	enumeration->names = name_list;
	enumeration->descriptions = description_list;

	for (i = 0; i < eel_string_list_get_length (name_list); i++) {
		if (!eel_string_list_nth_as_integer (value_list, i, &value)) {
			g_warning ("Could not convert value '%d' to an integer.  Using 0.", i);
			value = 0;
		}

		enumeration->values = g_list_append (enumeration->values, GINT_TO_POINTER (value));
	}
	eel_string_list_free (value_list);
	
	return enumeration;
}

int
eel_enumeration_get_name_position (const EelEnumeration *enumeration,
				   const char *name)
{
	g_return_val_if_fail (enumeration != NULL, EEL_STRING_LIST_NOT_FOUND);
	g_return_val_if_fail (name != NULL, EEL_STRING_LIST_NOT_FOUND);

	if (enumeration->names == NULL) {
		return EEL_STRING_LIST_NOT_FOUND;
	}

	return eel_string_list_get_index_for_string (enumeration->names, name);
}

int
eel_enumeration_get_description_position (const EelEnumeration *enumeration,
					  const char *description)
{
	g_return_val_if_fail (enumeration != NULL, EEL_STRING_LIST_NOT_FOUND);
	g_return_val_if_fail (description != NULL, EEL_STRING_LIST_NOT_FOUND);

	if (enumeration->descriptions == NULL) {
		return EEL_STRING_LIST_NOT_FOUND;
	}

	return eel_string_list_get_index_for_string (enumeration->descriptions, description);
}

int
eel_enumeration_get_value_position (const EelEnumeration *enumeration,
				    int value)
{
	GList *node;
	int pos;

	g_return_val_if_fail (enumeration != NULL, EEL_STRING_LIST_NOT_FOUND);

	if (enumeration->values == NULL) {
		return EEL_STRING_LIST_NOT_FOUND;
	}

	for (node = enumeration->values, pos = 0; node != NULL; node = node->next, pos++) {
		if (GPOINTER_TO_INT (node->data) == value) {
			return pos;
		}
	}

	return EEL_STRING_LIST_NOT_FOUND;
}

gboolean
eel_enumeration_contains_name (const EelEnumeration *enumeration,
			       const char *name)
{
	g_return_val_if_fail (enumeration != NULL, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);

	return eel_string_list_contains (enumeration->names, name);
}

EelStringList *
eel_enumeration_get_names (const EelEnumeration *enumeration)
{
	g_return_val_if_fail (enumeration != NULL, NULL);

	if (enumeration->names == NULL) {
		return NULL;
	}

	return eel_string_list_copy (enumeration->names);
}

void
eel_enumeration_insert_entries (EelEnumeration *enumeration,
				const EelEnumerationEntry entries[])
{
	guint i;

	g_return_if_fail (enumeration != NULL);
	g_return_if_fail (entries != NULL);

	for (i = 0; entries[i].name != NULL; i++) {
		eel_enumeration_insert (enumeration,
					entries[i].name,
					entries[i].description,
					entries[i].value);
	}
}

static GHashTable *enumeration_table = NULL;

typedef struct
{
	char *id;
	EelEnumeration *enumeration;
} TableEntry;

static void
enumeration_table_free_one_node (gpointer key,
				 gpointer value,
				 gpointer callback_data)
{
	TableEntry *entry;

	g_return_if_fail (key != NULL);
	g_return_if_fail (value != NULL);

	entry = value;

	g_free (entry->id);
	eel_enumeration_free (entry->enumeration);
	g_free (entry);
}

static void
enumeration_table_free (void)
{
	if (enumeration_table != NULL) {
		g_hash_table_foreach (enumeration_table, enumeration_table_free_one_node, NULL);
		g_hash_table_destroy (enumeration_table);
	}
	
	enumeration_table = NULL;
}

static GHashTable *
enumeration_table_get (void)
{
	if (enumeration_table != NULL) {
		return enumeration_table;
	}

	enumeration_table = g_hash_table_new (g_str_hash, g_str_equal);

	g_atexit (enumeration_table_free);

	return enumeration_table;
}

static TableEntry *
enumeration_table_lookup (const char *id)
{
	GHashTable *table;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '\0', NULL);
	g_return_val_if_fail (enumeration_table != NULL, NULL);
	
	table = enumeration_table_get ();
	g_return_val_if_fail (table != NULL, NULL);

	return g_hash_table_lookup (table, id);
}

static void
enumeration_register (const char *id,
		      const EelEnumerationEntry entries[])
{
	GHashTable *table;
	TableEntry *entry;

	g_return_if_fail (id != NULL);
	g_return_if_fail (id[0] != '\0');
	g_return_if_fail (entries != NULL);
	
	table = enumeration_table_get ();
	g_return_if_fail (table != NULL);
	
	if (enumeration_table_lookup (id) != NULL) {
		if (!suppress_duplicate_registration_warning) {
			g_warning ("Trying to register duplicate enumeration '%s'.", id);
		}

		return;
	}

	entry = g_new0 (TableEntry, 1);
	entry->id = g_strdup (id);
	entry->enumeration = eel_enumeration_new (entry->id);

	eel_enumeration_insert_entries (entry->enumeration, entries);

	g_hash_table_insert (table, entry->id, entry);
	g_assert (enumeration_table_lookup (entry->id) == entry);
}

void
eel_enumeration_register (const EelEnumerationInfo info_array[])
{
	guint i;

	g_return_if_fail (info_array != NULL);

	for (i = 0; info_array[i].id && info_array[i].entries != NULL; i++) {
		enumeration_register (info_array[i].id, info_array[i].entries);
	}
}

EelEnumeration *
eel_enumeration_lookup (const char *id)
{
	TableEntry *entry;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '\0', NULL);

	entry = enumeration_table_lookup (id);

	return entry != NULL ? eel_enumeration_copy (entry->enumeration) : NULL;
}

char *
eel_enumeration_id_get_nth_name (const char *id,
				 guint n)
{
	TableEntry *entry;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '\0', NULL);

	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->enumeration != NULL, NULL);

	g_return_val_if_fail (n < eel_enumeration_get_length (entry->enumeration), NULL);

	return eel_enumeration_get_nth_name (entry->enumeration, n);
}

char *
eel_enumeration_id_get_nth_description (const char *id,
					guint n)
{
	TableEntry *entry;
	
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '\0', NULL);
	
	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->enumeration != NULL, NULL);

	g_return_val_if_fail (n < eel_enumeration_get_length (entry->enumeration), NULL);
	
	return eel_enumeration_get_nth_description (entry->enumeration, n);
}

char *
eel_enumeration_id_get_nth_description_translated (const char *id,
						   guint n)
{
	TableEntry *entry;
	
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '\0', NULL);
	
	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->enumeration != NULL, NULL);

	g_return_val_if_fail (n < eel_enumeration_get_length (entry->enumeration), NULL);
	
	return eel_enumeration_get_nth_description_translated (entry->enumeration, n);
}

int
eel_enumeration_id_get_nth_value (const char *id,
				  guint n)
{
	TableEntry *entry;
	
	g_return_val_if_fail (id != NULL, 0);
	g_return_val_if_fail (id[0] != '\0', 0);
	
	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, 0);
	g_return_val_if_fail (entry->enumeration != NULL, 0);

	g_return_val_if_fail (n < eel_enumeration_get_length (entry->enumeration), 0);

	return eel_enumeration_get_nth_value (entry->enumeration, n);
}

guint
eel_enumeration_id_get_length (const char *id)
{
	TableEntry *entry;
	
	g_return_val_if_fail (id != NULL, 0);
	g_return_val_if_fail (id[0] != '\0', 0);
	
	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, 0);
	g_return_val_if_fail (entry->enumeration != NULL, 0);

	return eel_enumeration_get_length (entry->enumeration);
}

int
eel_enumeration_id_get_name_position (const char *id,
				      const char *name)
{
	TableEntry *entry;

	g_return_val_if_fail (id != NULL, -1);
	g_return_val_if_fail (id[0] != '\0', -1);
	g_return_val_if_fail (name != NULL, -1);
	g_return_val_if_fail (name[0] != '\0', -1);

	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, -1);
	g_return_val_if_fail (entry->enumeration != NULL, -1);

	return eel_enumeration_get_name_position (entry->enumeration, name);
}

int
eel_enumeration_id_get_description_position (const char *id,
					     const char *description)
{
	TableEntry *entry;

	g_return_val_if_fail (id != NULL, -1);
	g_return_val_if_fail (id[0] != '\0', -1);
	g_return_val_if_fail (description != NULL, -1);
	g_return_val_if_fail (description[0] != '\0', -1);

	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, -1);
	g_return_val_if_fail (entry->enumeration != NULL, -1);

	return eel_enumeration_get_description_position (entry->enumeration, description);
}

int
eel_enumeration_id_get_value_position (const char *id,
				       int value)
{
	TableEntry *entry;

	g_return_val_if_fail (id != NULL, -1);
	g_return_val_if_fail (id[0] != '\0', -1);

	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, -1);
	g_return_val_if_fail (entry->enumeration != NULL, -1);

	return eel_enumeration_get_value_position (entry->enumeration, value);
}

gboolean
eel_enumeration_id_contains_name (const char *id,
				  const char *name)
{
	TableEntry *entry;

	g_return_val_if_fail (id != NULL, FALSE);
	g_return_val_if_fail (id[0] != '\0', FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (name[0] != '\0', FALSE);

	entry = enumeration_table_lookup (id);
	g_return_val_if_fail (entry != NULL, -1);
	g_return_val_if_fail (entry->enumeration != NULL, -1);

	return eel_enumeration_contains_name (entry->enumeration, name);
}

#if !defined (EEL_OMIT_SELF_CHECK)

#define CHECK_ENUMERATION_ENTRY(enumeration, i, name, description, value) \
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_nth_name (enumeration, i), name); \
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_nth_name (enumeration, i), name); \
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_nth_description (enumeration, i), description); \
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_nth_value (enumeration, i), value); \
	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_name_position (enumeration, name), i); \
	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_value_position (enumeration, value), i);

static EelEnumerationEntry speed_tradeoff_enum_entries[] = {
	{ "always",	    "Always",		10 },
	{ "local_only",	    "Local Files Only",	20 },
	{ "never",	    "Never",		30 },
	{ NULL, NULL, 0 }
};

static EelEnumerationEntry standard_zoom_levels_enum_entries[] = {
	{ "smallest",	    "25%",	25 },
	{ "smaller",	    "50%",	50 },
	{ "small",	    "75%",	75 },
	{ "standard",	    "100%",	100 },
	{ "large",	    "150%",	150 },
	{ "larger",	    "200%",	200 },
	{ "largest",	    "400%",	400 },
	{ NULL, NULL, 0 }
};

static EelEnumerationEntry file_size_enum_entries[] = {
	{ "102400",	    "100 K",	102400 },
	{ "512000",	    "500 K",	512000 },
	{ "1048576",	    "1 MB",	1048576 },
	{ "3145728",	    "3 MB",	3145728 },
	{ "5242880",	    "5 MB",	5242880 },
	{ "10485760",	    "10 MB",	10485760 },
	{ "104857600",	    "100 MB",	104857600 },
	{ NULL, NULL, 0 }
};

static EelEnumerationInfo enumerations[] = {
	{ "speed_tradeoffs",		speed_tradeoff_enum_entries },
	{ "standard_zoom_levels",	standard_zoom_levels_enum_entries },
	{ "file_size",			file_size_enum_entries },
	{ NULL, NULL }
};

void
eel_self_check_enumeration (void)
{
	EelEnumeration *e;
	EelEnumeration *copy;
	EelStringList *names;
	EelStringList *names_test;
	guint i;
	guint j;

	/***/
	e = eel_enumeration_new ("id");
	eel_enumeration_insert (e, "foo", NULL, 0);
	CHECK_ENUMERATION_ENTRY (e, 0, "foo", "", 0);
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_id (e), "id");
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 1);

	eel_enumeration_insert (e, "bar", NULL, 1);
	CHECK_ENUMERATION_ENTRY (e, 1, "bar", "", 1);
	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 2);

	eel_enumeration_free (e);

	/***/
	e = eel_enumeration_new_from_tokens ("id",
					     "single",
					     NULL,
					     "1",
					     ",");
	
	CHECK_ENUMERATION_ENTRY (e, 0, "single", "", 1);
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_id (e), "id");
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 1);
	eel_enumeration_free (e);

	/***/
	e = eel_enumeration_new_from_tokens ("id",
					     "apple,orange,banana",
					     NULL,
					     "1,2,3",
					     ",");
	
	CHECK_ENUMERATION_ENTRY (e, 0, "apple", "", 1);
	CHECK_ENUMERATION_ENTRY (e, 1, "orange", "", 2);
	CHECK_ENUMERATION_ENTRY (e, 2, "banana", "", 3);
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_id (e), "id");
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 3);
	eel_enumeration_free (e);

	/***/
	e = eel_enumeration_new_from_tokens ("id",
					     "foo",
					     NULL,
					     "666",
					     ",");
	CHECK_ENUMERATION_ENTRY (e, 0, "foo", "", 666);
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_id (e), "id");
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 1);
	eel_enumeration_free (e);

	/***/
	e = eel_enumeration_new_from_tokens ("id",
					     "one,two,---,three",
					     "One,Two,---,Three",
					     "1,2,0,3",
					     ",");
	CHECK_ENUMERATION_ENTRY (e, 0, "one", "One", 1);
	CHECK_ENUMERATION_ENTRY (e, 1, "two", "Two", 2);
	CHECK_ENUMERATION_ENTRY (e, 2, "---", "---", 0);
	CHECK_ENUMERATION_ENTRY (e, 3, "three", "Three", 3);
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 4);
	eel_enumeration_free (e);

	/***/
	e = eel_enumeration_new_from_tokens ("id",
					     "red,green,blue",
					     "Red Desc,Green Desc,Blue Desc",
					     "10,20,30",
					     ",");
	
	CHECK_ENUMERATION_ENTRY (e, 0, "red", "Red Desc", 10);
	CHECK_ENUMERATION_ENTRY (e, 1, "green", "Green Desc", 20);
	CHECK_ENUMERATION_ENTRY (e, 2, "blue", "Blue Desc", 30);
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_id (e), "id");
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 3);
	
 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_contains_name (e, "red"), TRUE);
 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_contains_name (e, "green"), TRUE);
 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_contains_name (e, "blue"), TRUE);
 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_contains_name (e, "pink"), FALSE);
	
	copy = eel_enumeration_copy (e);
	eel_enumeration_free (e);

	CHECK_ENUMERATION_ENTRY (copy, 0, "red", "Red Desc", 10);
	CHECK_ENUMERATION_ENTRY (copy, 1, "green", "Green Desc", 20);
	CHECK_ENUMERATION_ENTRY (copy, 2, "blue", "Blue Desc", 30);
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_id (copy), "id");
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (copy), 3);
	eel_enumeration_free (copy);

	/***/
	e = eel_enumeration_new_from_tokens ("id",
					     "red,foo:green,bar:blue,baz",
					     "Red,Desc:Green,Desc:Blue,Desc",
					     "10:20:30",
					     ":");

	CHECK_ENUMERATION_ENTRY (e, 0, "red,foo", "Red,Desc", 10);
	CHECK_ENUMERATION_ENTRY (e, 1, "green,bar", "Green,Desc", 20);
	CHECK_ENUMERATION_ENTRY (e, 2, "blue,baz", "Blue,Desc", 30);
 	EEL_CHECK_STRING_RESULT (eel_enumeration_get_id (e), "id");
 	EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), 3);

	names = eel_enumeration_get_names (e);
	names_test = eel_string_list_new_from_tokens ("red,foo:green,bar:blue,baz", ":", TRUE);

 	EEL_CHECK_BOOLEAN_RESULT (eel_string_list_equals (names, names_test), TRUE);

	eel_string_list_free (names);
	eel_string_list_free (names_test);
	eel_enumeration_free (e);

	/***/
	suppress_duplicate_registration_warning = TRUE;
	eel_enumeration_register (enumerations);
	suppress_duplicate_registration_warning = FALSE;

	for (j = 0; enumerations[j].id != NULL; j++) {
		e = eel_enumeration_lookup (enumerations[j].id);
		g_return_if_fail (e != NULL);

		for (i = 0; enumerations[j].entries[i].name != NULL; i++) {
			CHECK_ENUMERATION_ENTRY (e,
						 i,
						 enumerations[j].entries[i].name,
						 enumerations[j].entries[i].description,
						 enumerations[j].entries[i].value);

			EEL_CHECK_STRING_RESULT (eel_enumeration_id_get_nth_name (enumerations[j].id, i),
						  enumerations[j].entries[i].name);

			EEL_CHECK_STRING_RESULT (eel_enumeration_id_get_nth_description (enumerations[j].id, i),
						 enumerations[j].entries[i].description);

			EEL_CHECK_INTEGER_RESULT (eel_enumeration_id_get_nth_value (enumerations[j].id, i),
						  enumerations[j].entries[i].value);
		}
		EEL_CHECK_INTEGER_RESULT (eel_enumeration_get_length (e), i);
		EEL_CHECK_INTEGER_RESULT (eel_enumeration_id_get_length (enumerations[j].id), i);
		
		eel_enumeration_free (e);
	}

 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_id_contains_name ("speed_tradeoffs", "always"), TRUE);
 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_id_contains_name ("speed_tradeoffs", "local_only"), TRUE);
 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_id_contains_name ("speed_tradeoffs", "never"), TRUE);
 	EEL_CHECK_BOOLEAN_RESULT (eel_enumeration_id_contains_name ("speed_tradeoffs", "sometimes"), FALSE);
}

#endif /* !EEL_OMIT_SELF_CHECK */
