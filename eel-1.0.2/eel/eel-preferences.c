/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences.c - Preference peek/poke/notify implementation.

   Copyright (C) 1999, 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>
#include "eel-preferences.h"

#include "eel-gconf-extensions.h"
#include "eel-lib-self-check-functions.h"
#include "eel-enumeration.h"
#include "eel-glib-extensions.h"
#include "eel-string-list.h"
#include "eel-string.h"
#include <gconf/gconf-client.h>
#include <gconf/gconf.h>
#include <gtk/gtksignal.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-dialog-util.h>

#define DEFAULT_USER_LEVEL	EEL_USER_LEVEL_INTERMEDIATE
#define USER_LEVEL_KEY		"/apps/nautilus/user_level"

/* An enumeration used for updating auto-storage variables in a type-specific way. 
 * FIXME: there is another enumeration like this in eel-global-preferences.c,
 * used for different purposes but in a related way. Should we combine them?
 */
typedef enum {
	PREFERENCE_BOOLEAN = 1,
	PREFERENCE_INTEGER,
	PREFERENCE_STRING,
	PREFERENCE_STRING_LIST
} PreferenceType;

/*
 * PreferencesEntry:
 *
 * A structure to manage preference hash table nodes.
 * Preferences are hash tables.  The hash key is the preference name
 * (a string).  The  hash value is a pointer of the following struct:
 */
typedef struct {
	char *name;
	char *description;
	PreferenceType type;
	gboolean invisible;
	GList *callback_list;
	gboolean callbacks_blocked;
	GList *auto_storage_list;
	int gconf_connection_id;
	char *enumeration_id;
	GConfValue *cached_value;
	int visible_user_level;
	GConfValue *default_values[3];
} PreferencesEntry;

/*
 * PreferencesCallbackEntry:
 *
 * A structure to manage callback lists.  A callback list is a GList.
 * The callback_data in each list node is a pointer to the following 
 * struct:
 */
typedef struct {
	EelPreferencesCallback callback;
	gpointer callback_data;
} PreferencesCallbackEntry;

static const char *user_level_names_for_display[] = {
	N_("Beginner"),
	N_("Intermediate"),
	N_("Advanced")
};

static const char *user_level_names_for_storage[] = {
	"novice",
	"intermediate",
	"advanced"
};

static const char *      preferences_peek_storage_path                   (void);
static gboolean          preferences_preference_is_gconf_key             (const char               *name);
static gboolean          preferences_preference_is_user_level            (const char               *name);
static gboolean          preferences_preference_is_default               (const char               *name);
static char *            preferences_key_make                            (const char               *name);
static void              preferences_user_level_changed_notice           (GConfClient              *client,
									  guint                     connection_id,
									  GConfEntry               *gconf_entry,
									  gpointer                  user_data);
static void              preferences_something_changed_notice            (GConfClient              *client,
									  guint                     connection_id,
									  GConfEntry               *gconf_entry,
									  gpointer                  user_data);
static void              preferences_global_table_check_changes_function (gpointer                  key,
									  gpointer                  value,
									  gpointer                  callback_data);
static GHashTable  *     preferences_global_table_get_global             (void);
static void              preferences_callback_entry_free                 (PreferencesCallbackEntry *callback_entry);
static void              preferences_entry_update_auto_storage           (PreferencesEntry         *entry);
static void              preferences_global_table_free                   (void);
static const char *      preferences_peek_user_level_name_for_storage    (int                       user_level);
static PreferencesEntry *preferences_global_table_lookup_or_insert       (const char               *name);
static const GConfValue *preferences_find_first_non_null_default_value   (const char               *name,
									  int                       user_level);

static guint user_level_changed_connection_id = EEL_GCONF_UNDEFINED_CONNECTION;
static GHashTable *global_table = NULL;
static char *storage_path = NULL;
static gboolean initialized = FALSE;

static int
preferences_gconf_value_get_int (const GConfValue *value)
{
	if (value == NULL) {
		return 0;
	}
	g_return_val_if_fail (value->type == GCONF_VALUE_INT, 0);
	return gconf_value_get_int (value);
}

static gboolean
preferences_gconf_value_get_bool (const GConfValue *value)
{
	if (value == NULL) {
		return FALSE;
	}
	g_return_val_if_fail (value->type == GCONF_VALUE_BOOL, FALSE);
	return gconf_value_get_bool (value);
}

static char *
preferences_gconf_value_get_string (const GConfValue *value)
{
	if (value == NULL) {
		return g_strdup ("");
	}
	g_return_val_if_fail (value->type == GCONF_VALUE_STRING, g_strdup (""));
	return g_strdup (gconf_value_get_string (value));
}

static EelStringList *
preferences_gconf_value_get_string_list (const GConfValue *value)
{
	GSList *slist;
	EelStringList *result;

	if (value == NULL) {
		return eel_string_list_new (TRUE);
	}
	g_return_val_if_fail (value->type == GCONF_VALUE_LIST, eel_string_list_new (TRUE));
	g_return_val_if_fail (gconf_value_get_list_type (value) == GCONF_VALUE_STRING, eel_string_list_new (TRUE));

	slist = eel_gconf_value_get_string_list (value);
	result = eel_string_list_new_from_g_slist (slist, TRUE);
	eel_g_slist_free_deep (slist);
 	return result;
}

static const char *
preferences_peek_storage_path (void)
{
	g_return_val_if_fail (storage_path != NULL, NULL);

	return storage_path;
}

static void
preferences_set_storage_path (const char *new_storage_path)
{
	g_return_if_fail (eel_strlen (new_storage_path) > 0);

	/* Make sure the path is indeed different */
	if (eel_str_is_equal (new_storage_path, storage_path)) {
		return;
	}

	/* Free the preference hash table */
	preferences_global_table_free ();

	/* Stop monitoring the old path */
	eel_gconf_monitor_remove (storage_path);

	g_free (storage_path);
	storage_path = g_strdup (new_storage_path);

	/* Start monitoring the new path */
	eel_gconf_monitor_add (storage_path);
}

static gboolean
preferences_is_initialized (void)
{
	return initialized;
}

static GConfValue *
preferences_get_value (const char *name)
{
	GConfValue *result;
	char *key;
	const GConfValue *default_value;

	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (preferences_is_initialized (), 0);

	/* If the preference is default (no value is stored for it) or
	 * it is not visible in the current user level, return the
	 * default_value */
	if (preferences_preference_is_default (name)
	    || !eel_preferences_visible_in_current_user_level (name)) {
		default_value = preferences_find_first_non_null_default_value (name, eel_preferences_get_user_level ());
		return (default_value != NULL)
			? gconf_value_copy ((GConfValue *) default_value)
			: NULL;
	}

	key = preferences_key_make (name);
	result = eel_gconf_get_value (key);
	g_free (key);

	return result;
}

static const char *
preferences_peek_user_level_name_for_storage (int user_level)
{
	user_level = eel_preferences_user_level_clamp (user_level);
	
	return user_level_names_for_storage[user_level];
}

/* If the preference name begind with a "/", we interpret 
 * it as a straight gconf key. */
static gboolean
preferences_preference_is_gconf_key (const char *name)
{
	g_return_val_if_fail (name != NULL, FALSE);
	
	if (eel_str_has_prefix (name, "/")) {
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
preferences_preference_is_user_level (const char *name)
{
	g_return_val_if_fail (name != NULL, FALSE);
	
	return eel_str_is_equal (name, USER_LEVEL_KEY)
		|| eel_str_is_equal (name, "user_level");
}

static char *
preferences_key_make (const char *name)
{
	g_return_val_if_fail (name != NULL, NULL);
	
	if (!preferences_preference_is_gconf_key (name)) {
		return g_strdup (name);
	}

	/* Otherwise, we prefix it with the path */
	return g_strdup_printf ("%s/%s", preferences_peek_storage_path (), name);
}

/* Find the first non NULL default_value that is less than or 
 * equal to the given user level */
static const GConfValue *
preferences_find_first_non_null_default_value (const char *name,
					       int user_level)
{
	const GConfValue *result;
	PreferencesEntry *entry;
	gboolean done;

	g_return_val_if_fail (name != NULL, NULL);

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	user_level = eel_preferences_user_level_clamp (user_level);

	done = FALSE;
	while (!done) {
		result = entry->default_values[user_level];
		done = (user_level == 0) || (result != NULL);
		if (!done) {
			user_level--;
		}
	}

	return result;
}

static gboolean
preferences_preference_is_default (const char *name)
{
	gboolean result;
	char *key;
	
	g_return_val_if_fail (name != NULL, FALSE);
	
	key = preferences_key_make (name);
	result = eel_gconf_is_default (key);
	g_free (key);

	return result;
}

static void
preferences_block_callbacks (const char *name)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	entry->callbacks_blocked = TRUE;
}

static void
preferences_unblock_callbacks (const char *name)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	entry->callbacks_blocked = FALSE;
}

/* Public preferences functions */
int
eel_preferences_get_visible_user_level (const char *name)
{
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (preferences_is_initialized (), FALSE);

	return preferences_global_table_lookup_or_insert (name)->visible_user_level;
}

void
eel_preferences_set_visible_user_level (const char *name,
					int visible_user_level)
{
	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());

	preferences_global_table_lookup_or_insert (name)->visible_user_level =
		eel_preferences_user_level_clamp (visible_user_level);
}

gboolean
eel_preferences_get_is_invisible (const char *name)
{
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (preferences_is_initialized (), FALSE);

	return preferences_global_table_lookup_or_insert (name)->invisible;
}

void
eel_preferences_set_is_invisible (const char *name,
				  gboolean is_invisible)
{
	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());

	preferences_global_table_lookup_or_insert (name)->invisible = is_invisible;
}

void
eel_preferences_set_boolean (const char *name,
			     gboolean boolean_value)
{
	char *key;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());
	
	key = preferences_key_make (name);
	eel_gconf_set_boolean (key, boolean_value);
	g_free (key);

	eel_gconf_suggest_sync ();
}

gboolean
eel_preferences_get_boolean (const char *name)
{
 	gboolean result;
	GConfValue *value;

	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (preferences_is_initialized (), 0);

	value = preferences_get_value (name);
	result = preferences_gconf_value_get_bool (value);
	eel_gconf_value_free (value);
	
	return result;
}

void
eel_preferences_set_integer (const char *name,
			     int int_value)
{
	char *key;
	int old_value;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());
	
	key = preferences_key_make (name);
	old_value = eel_preferences_get_integer (name);
	
	if (int_value != old_value) {
		eel_gconf_set_integer (key, int_value);
		eel_gconf_suggest_sync ();
	}
	g_free (key);
}

int
eel_preferences_get_integer (const char *name)
{
 	int result;
	GConfValue *value;

	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (preferences_is_initialized (), 0);

	value = preferences_get_value (name);
	result = preferences_gconf_value_get_int (value);
	eel_gconf_value_free (value);

	return result;
}

void
eel_preferences_set (const char *name,
		     const char *string_value)
{
	char *key;
	char *old_value;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());

	key = preferences_key_make (name);
	old_value = eel_preferences_get (name);

	if (strcmp (string_value, old_value) != 0) {
		eel_gconf_set_string (key, string_value);
		
		eel_gconf_suggest_sync ();
	}
	g_free (key);
}

char *
eel_preferences_get (const char *name)
{
 	char *result;
	GConfValue *value;

	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (preferences_is_initialized (), 0);

	value = preferences_get_value (name);
	result = preferences_gconf_value_get_string (value);
	eel_gconf_value_free (value);
	
	return result;
}

void
eel_preferences_set_string_list (const char *name,
				 const EelStringList *string_list_value)
{
	char *key;
	GSList *slist;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());

	slist = eel_string_list_as_g_slist (string_list_value);
	key = preferences_key_make (name);
	eel_gconf_set_string_list (key, slist);
	g_free (key);

	eel_g_slist_free_deep (slist);

	eel_gconf_suggest_sync ();
}

static gboolean
string_list_is_valid (const EelStringList *string_list,
		      const char *enumeration_id)
{
	guint i;
	char *nth;
	gboolean bad;

	g_return_val_if_fail (string_list != NULL, FALSE);
	g_return_val_if_fail (enumeration_id != NULL, FALSE);

	bad = FALSE;
	for (i = 0; i < eel_string_list_get_length (string_list) && !bad; i++) {
		nth = eel_string_list_nth (string_list, i);
		bad = !eel_enumeration_id_contains_name (enumeration_id, nth);
		g_free (nth);
	}

	return !bad;
}

EelStringList *
eel_preferences_get_string_list (const char *name)
{
 	EelStringList *result;
	GConfValue *value;
	PreferencesEntry *entry;
	const GConfValue *default_value;

	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (preferences_is_initialized (), 0);

	value = preferences_get_value (name);
	result = preferences_gconf_value_get_string_list (value);
	eel_gconf_value_free (value);
	
	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	/* No enumeration_id so we're done */
	if (entry->enumeration_id == NULL) {
		return result;
	}

	/* Do a sanity check on the validity of the values */
	if (string_list_is_valid (result, entry->enumeration_id)) {
		return result;
	}

	/* Forget the bad value and use the default instead */
	eel_string_list_free (result);

	default_value = preferences_find_first_non_null_default_value (name, eel_preferences_get_user_level ());
	result = preferences_gconf_value_get_string_list (default_value);
	
 	/* Go the extra mile and fix the problem for the user */
	preferences_block_callbacks (name);
	eel_preferences_set_string_list (name, result);
	preferences_unblock_callbacks (name);

	return result;
}

int
eel_preferences_get_user_level (void)
{
	char *user_level;
	int result;

	g_return_val_if_fail (preferences_is_initialized (), 0);

	user_level = eel_gconf_get_string (USER_LEVEL_KEY);

	if (eel_str_is_equal (user_level, "advanced")) {
		result = EEL_USER_LEVEL_ADVANCED;
	} else if (eel_str_is_equal (user_level, "intermediate")) {
		result = EEL_USER_LEVEL_INTERMEDIATE;
	} else if (eel_str_is_equal (user_level, "novice")) {
		result = EEL_USER_LEVEL_NOVICE;
	} else {
		result = DEFAULT_USER_LEVEL;
	}
	
	g_free (user_level);
	return result;
}

void
eel_preferences_set_user_level (int user_level)
{
	g_return_if_fail (preferences_is_initialized ());
	g_return_if_fail (eel_preferences_user_level_is_valid (user_level));
	
	user_level = eel_preferences_user_level_clamp (user_level);

	eel_gconf_set_string (USER_LEVEL_KEY, user_level_names_for_storage[user_level]);

	eel_gconf_suggest_sync ();
}

void
eel_preferences_default_set_integer (const char *name,
				     int user_level,
				     int int_value)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());
	g_return_if_fail (eel_preferences_user_level_is_valid (user_level));

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	if (entry->default_values[user_level] == NULL) {
		entry->default_values[user_level] = gconf_value_new (GCONF_VALUE_INT);
	}
	gconf_value_set_int (entry->default_values[user_level], int_value);
}

int
eel_preferences_default_get_integer (const char *name,
				     int user_level)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (preferences_is_initialized (), 0);
	g_return_val_if_fail (eel_preferences_user_level_is_valid (user_level), 0);
	
	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	return preferences_gconf_value_get_int (entry->default_values[user_level]);
}

void
eel_preferences_default_set_boolean (const char *name,
				     int user_level,
				     gboolean boolean_value)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());
	g_return_if_fail (eel_preferences_user_level_is_valid (user_level));

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	if (entry->default_values[user_level] == NULL) {
		entry->default_values[user_level] = gconf_value_new (GCONF_VALUE_BOOL);
	}
	gconf_value_set_bool (entry->default_values[user_level], boolean_value);
}

gboolean
eel_preferences_default_get_boolean (const char *name,
				     int user_level)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (preferences_is_initialized (), FALSE);
	g_return_val_if_fail (eel_preferences_user_level_is_valid (user_level), FALSE);
	
	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	return preferences_gconf_value_get_bool (entry->default_values[user_level]);
}

void
eel_preferences_default_set_string (const char *name,
				    int user_level,
				    const char *string_value)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());
	g_return_if_fail (eel_preferences_user_level_is_valid (user_level));

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	if (entry->default_values[user_level] == NULL) {
		entry->default_values[user_level] = gconf_value_new (GCONF_VALUE_STRING);
	}
	gconf_value_set_string (entry->default_values[user_level], string_value);
}

char *
eel_preferences_default_get_string (const char *name,
				    int user_level)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (preferences_is_initialized (), FALSE);
	g_return_val_if_fail (eel_preferences_user_level_is_valid (user_level), FALSE);
	
	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	return preferences_gconf_value_get_string (entry->default_values[user_level]);
}

void
eel_preferences_default_set_string_list (const char *name,
					 int user_level,
					 const EelStringList *string_list_value)
{
	PreferencesEntry *entry;
	GSList *slist;

	g_return_if_fail (name != NULL);
	g_return_if_fail (preferences_is_initialized ());
	g_return_if_fail (eel_preferences_user_level_is_valid (user_level));

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	if (entry->default_values[user_level] == NULL) {
		entry->default_values[user_level] = gconf_value_new (GCONF_VALUE_LIST);
		gconf_value_set_list_type (entry->default_values[user_level], GCONF_VALUE_STRING);
	}
	
	slist = eel_string_list_as_g_slist (string_list_value);
	eel_gconf_value_set_string_list (entry->default_values[user_level], slist);
	eel_g_slist_free_deep (slist);
}

EelStringList *
eel_preferences_default_get_string_list (const char *name,
					 int user_level)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (preferences_is_initialized (), FALSE);
	g_return_val_if_fail (eel_preferences_user_level_is_valid (user_level), FALSE);
	
	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	return preferences_gconf_value_get_string_list (entry->default_values[user_level]);
}

/**
 * preferences_callback_entry_invoke_function
 *
 * A function that invokes a callback from the given struct.  It is meant to be fed to 
 * g_list_foreach ()
 * @data: The list data privately maintained by the GList.
 * @callback_data: The callback_data privately maintained by the GList.
 **/
static void
preferences_callback_entry_invoke_function (gpointer data,
					    gpointer callback_data)
{
	PreferencesCallbackEntry *callback_entry;
	
	g_return_if_fail (data != NULL);
	
	callback_entry = data;

 	(* callback_entry->callback) (callback_entry->callback_data);
}

/**
 * preferences_entry_invoke_callbacks_if_needed
 *
 * @entry: A PreferencesEntry
 *
 * This function checks the cached value in the entry with the current
 * value of the preference.  If the value has changed, then callbacks
 * are invoked and auto storage updated.
 *
 * We need this check because even though the GConf value of a preference
 * could indeed have changed, its representation on the Eel side
 * of things could still be the same.  The best example of this is 
 * user level changes, where the value of the preference on the Eel
 * end of things is determined by visibility.
 **/
static void
preferences_entry_invoke_callbacks_if_needed (PreferencesEntry *entry)
{
	GConfValue *new_value;
	
	g_return_if_fail (entry != NULL);

	new_value = preferences_get_value (entry->name);

	/* If the values are the same, then we dont need to invoke any callbacks */
	if (eel_gconf_value_is_equal (entry->cached_value, new_value)) {
		eel_gconf_value_free (new_value);
		return;
	}

	/* Update the auto storage preferences */
	if (entry->auto_storage_list != NULL) {
		preferences_entry_update_auto_storage (entry);			
	}

	/* Store the new cached value */
	eel_gconf_value_free (entry->cached_value);
	entry->cached_value = new_value;

	/* Dont invoke callbacks if the entry is blocked */
	if (entry->callbacks_blocked) {
		return;
	}
	
	/* Invoke callbacks for this entry if any */
	if (entry->callback_list != NULL) {
		g_list_foreach (entry->callback_list,
				preferences_callback_entry_invoke_function,
				NULL);
	}
}

static void
update_auto_string (gpointer data, gpointer callback_data)
{
	char **storage;
	const char *value;

	g_return_if_fail (data != NULL);
	g_return_if_fail (callback_data != NULL);

	storage = (char **)data;
	value = (const char *)callback_data;

	g_free (*storage);
	*(char **)storage = g_strdup (value);
}

static void
update_auto_string_list (gpointer data, gpointer callback_data)
{
	EelStringList **storage;
	const EelStringList *value;
	
	g_return_if_fail (data != NULL);
	g_return_if_fail (callback_data != NULL);

	storage = (EelStringList **)data;
	value = (const EelStringList *)callback_data;
	
	eel_string_list_free (*storage);
	*(EelStringList **)storage = eel_string_list_copy (value);
}

static void
update_auto_integer_or_boolean (gpointer data, gpointer callback_data)
{
	g_return_if_fail (data != NULL);

	*(int *)data = GPOINTER_TO_INT (callback_data);
}

static void
preferences_entry_update_auto_storage (PreferencesEntry *entry)
{
	char *new_string_value;
	EelStringList *new_string_list_value;
	int new_int_value;
	gboolean new_boolean_value;

	switch (entry->type) {
	case PREFERENCE_STRING:
		new_string_value = eel_preferences_get (entry->name);
		g_list_foreach (entry->auto_storage_list,
				update_auto_string,
				new_string_value);
		g_free (new_string_value);
		break;
	case PREFERENCE_STRING_LIST:
		new_string_list_value = eel_preferences_get_string_list (entry->name);
		g_list_foreach (entry->auto_storage_list,
				update_auto_string_list,
				new_string_list_value);
		eel_string_list_free (new_string_list_value);
		break;
	case PREFERENCE_INTEGER:
		new_int_value = eel_preferences_get_integer (entry->name);
		g_list_foreach (entry->auto_storage_list,
				update_auto_integer_or_boolean,
				GINT_TO_POINTER (new_int_value));
		break;
	case PREFERENCE_BOOLEAN:
		new_boolean_value = eel_preferences_get_boolean (entry->name);
		g_list_foreach (entry->auto_storage_list,
				update_auto_integer_or_boolean,
				GINT_TO_POINTER (new_boolean_value));
		break;
	default:
		g_warning ("unexpected preferences type %d in preferences_entry_update_auto_storage", entry->type);
	}
}

static void
preferences_something_changed_notice (GConfClient *client, 
				      guint connection_id, 
				      GConfEntry *entry, 
				      gpointer notice_data)
{
	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->key != NULL);
	g_return_if_fail (notice_data != NULL);

	preferences_entry_invoke_callbacks_if_needed (notice_data);
}

static void
preferences_global_table_check_changes_function (gpointer key,
						 gpointer value,
						 gpointer user_data)
{
	PreferencesEntry *entry;

	g_return_if_fail (key != NULL);
	g_return_if_fail (value != NULL);

	entry = value;

	g_return_if_fail (entry->name != NULL);

	/* We dont worry about the 'user_level' itself for recursive reasons */
	if (preferences_preference_is_user_level (entry->name)) {
		return;
	}

	preferences_entry_invoke_callbacks_if_needed (entry);
}

static void
preferences_entry_update_cached_value (PreferencesEntry *entry)
{
	g_return_if_fail (entry != NULL);

	eel_gconf_value_free (entry->cached_value);

	entry->cached_value = preferences_get_value (entry->name);
}

static void
preferences_user_level_changed_notice (GConfClient *client, 
				       guint connection_id, 
				       GConfEntry *gconf_entry, 
				       gpointer user_data)
{
	g_return_if_fail (gconf_entry != NULL);
	g_return_if_fail (gconf_entry->key != NULL);
	g_return_if_fail (eel_str_has_suffix (gconf_entry->key, "user_level"));
	
	g_hash_table_foreach (preferences_global_table_get_global (),
			      preferences_global_table_check_changes_function,
			      NULL);
}

static void
preferences_entry_ensure_gconf_connection (PreferencesEntry *entry)
{
	char *key;
	
	/*
	 * We install only one gconf notification for each preference entry.
	 * Otherwise, we would invoke the installed callbacks more than once
	 * per registered callback.
	 */
	if (entry->gconf_connection_id != EEL_GCONF_UNDEFINED_CONNECTION) {
		return;
	}
		
	g_return_if_fail (entry->name != NULL);

	key = preferences_key_make (entry->name);

	entry->gconf_connection_id = eel_gconf_notification_add (key,
								 preferences_something_changed_notice,
								 entry);
	g_free (key);

	g_return_if_fail (entry->gconf_connection_id != EEL_GCONF_UNDEFINED_CONNECTION);

	/* Update the cached value.
	 * From now onwards the cached value will be updated 
	 * each time preferences_something_changed_notice() triggers
	 * so that it can be later compared with new values to 
	 * determine if the gconf value is different from the 
	 * Eel value.
	 */
	preferences_entry_update_cached_value (entry);
}

/**
 * preferences_entry_add_callback
 *
 * Add a callback to a pref node.  Callbacks are fired whenever
 * the pref value changes.
 * @preferences_entry: The hash node.
 * @callback: The user-supplied callback.
 * @callback_data: The user-supplied closure.
 **/
static void
preferences_entry_add_callback (PreferencesEntry *entry,
				EelPreferencesCallback callback,
				gpointer callback_data)
{
	PreferencesCallbackEntry *callback_entry;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (callback != NULL);

	callback_entry = g_new0 (PreferencesCallbackEntry, 1);
	callback_entry->callback = callback;
	callback_entry->callback_data = callback_data;
	
	g_return_if_fail (callback_entry != NULL);
	
	entry->callback_list = g_list_append (entry->callback_list, callback_entry);

	preferences_entry_ensure_gconf_connection (entry);
}

/**
 * preferences_entry_add_auto_storage
 *
 * Add an auto-storage variable to a pref node.  The variable will
 * be updated to match the pref value whenever the pref 
 * the pref value changes.
 * @preferences_entry: The hash node.
 * @storage: The user-supplied location at which to store the value.
 * @type: Which type of variable this is.
 **/
static void
preferences_entry_add_auto_storage (PreferencesEntry *entry,
				    gpointer storage,
				    PreferenceType type)
{
	g_return_if_fail (entry != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (entry->type == 0 || entry->type == type);
	g_return_if_fail (g_list_find (entry->auto_storage_list, storage) == NULL);

	entry->type = type;
	
	entry->auto_storage_list = g_list_append (entry->auto_storage_list, storage);

	preferences_entry_ensure_gconf_connection (entry);
}

static void
preferences_entry_check_remove_connection (PreferencesEntry *entry)
{
	/*
	 * If there are no callbacks or auto-storage variables left in the entry, 
	 * remove the gconf notification.
	 */
	if (entry->callback_list != NULL || entry->auto_storage_list != NULL) {
		return;
	}

	eel_gconf_notification_remove (entry->gconf_connection_id);
	entry->gconf_connection_id = EEL_GCONF_UNDEFINED_CONNECTION;
}

/**
 * preferences_entry_remove_callback
 *
 * remove a callback from a pref entry.  Both the callback and the callback_data must
 * match in order for a callback to be removed from the entry.
 * @preferences_entry: The hash entry.
 * @callback: The user-supplied callback.
 * @callback_data: The user-supplied closure.
 **/
static void
preferences_entry_remove_callback (PreferencesEntry *entry,
				   EelPreferencesCallback callback,
				   gpointer callback_data)
{
	GList *new_list;
	const GList *node;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (callback != NULL);
	g_return_if_fail (entry->callback_list != NULL);
	
	new_list = g_list_copy (entry->callback_list);
	
	for (node = new_list; node != NULL; node = node->next) {
		PreferencesCallbackEntry *callback_entry = node->data;
		
		g_return_if_fail (callback_entry != NULL);
		
		if (callback_entry->callback == callback &&
		    callback_entry->callback_data == callback_data) {
			entry->callback_list = g_list_remove (entry->callback_list, 
							      callback_entry);
			
			preferences_callback_entry_free (callback_entry);
		}
	}

	g_list_free (new_list);

	preferences_entry_check_remove_connection (entry);
}

/**
 * preferences_entry_remove_auto_storage
 *
 * remove an auto-storage variable from a pref entry.
 * @preferences_entry: The hash entry.
 * @storage: The user-supplied location.
 **/
static void
preferences_entry_remove_auto_storage (PreferencesEntry *entry,
				       gpointer storage)
{
	GList *new_list;
	const GList *node;
	gpointer storage_in_entry;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (entry->auto_storage_list != NULL);
	
	new_list = g_list_copy (entry->auto_storage_list);
	
	for (node = new_list; node != NULL; node = node->next) {
		storage_in_entry = node->data;
		
		g_return_if_fail (storage_in_entry != NULL);
		
		if (storage_in_entry == storage) {
			entry->auto_storage_list = g_list_remove (entry->auto_storage_list, 
							          storage);

			switch (entry->type) {
			case PREFERENCE_STRING:
				update_auto_string (storage, NULL);
				break;
			case PREFERENCE_STRING_LIST:
				update_auto_string_list (storage, NULL);
				break;
			case PREFERENCE_BOOLEAN:
			case PREFERENCE_INTEGER:
				update_auto_integer_or_boolean (storage, 0);
				break;
			default:
				g_warning ("unexpected preference type %d in preferences_entry_remove_auto_storage", entry->type);
			}
		}
	}

	g_list_free (new_list);

	preferences_entry_check_remove_connection (entry);
}

/**
 * preferences_callback_entry_free
 *
 * Free a callback info struct.
 * @preferences_callback_entry: The struct to free.
 **/
static void
preferences_callback_entry_free (PreferencesCallbackEntry *callback_entry)
{
	g_return_if_fail (callback_entry != NULL);

	callback_entry->callback = NULL;
	callback_entry->callback_data = NULL;

	g_free (callback_entry);
}

/**
 * preferences_callback_entry_free_func
 *
 * A function that frees a callback info struct.  It is meant to be fed to 
 * g_list_foreach ()
 * @data: The list data privately maintained by the GList.
 * @callback_data: The callback_data privately maintained by the GList.
 **/
static void
preferences_callback_entry_free_func (gpointer	data,
				      gpointer	callback_data)
{
	g_return_if_fail (data != NULL);
	
	preferences_callback_entry_free (data);
}

/**
 * preferences_entry_free
 *
 * Free a preference hash node's members along with the node itself.
 * @preferences_hash_node: The node to free.
 **/
static void
preferences_entry_free (PreferencesEntry *entry)
{
	g_return_if_fail (entry != NULL);

	eel_gconf_notification_remove (entry->gconf_connection_id);
	entry->gconf_connection_id = EEL_GCONF_UNDEFINED_CONNECTION;

	g_list_free (entry->auto_storage_list);
	eel_g_list_free_deep_custom (entry->callback_list,
				     preferences_callback_entry_free_func,
				     NULL);
	
	entry->auto_storage_list = NULL;
	entry->callback_list = NULL;

	g_free (entry->name);
	g_free (entry->description);
	g_free (entry->enumeration_id);

	eel_gconf_value_free (entry->cached_value);
	eel_gconf_value_free (entry->default_values[0]);
	eel_gconf_value_free (entry->default_values[1]);
	eel_gconf_value_free (entry->default_values[2]);

	g_free (entry);
}

/**
 * preferences_entry_free_func
 *
 * A function that frees a pref hash node.  It is meant to be fed to 
 * g_hash_table_foreach ()
 * @key: The hash key privately maintained by the GHashTable.
 * @value: The hash value privately maintained by the GHashTable.
 * @callback_data: The callback_data privately maintained by the GHashTable.
 **/
static void
preferences_entry_free_func (gpointer key,
			     gpointer value,
			     gpointer callback_data)
{
	g_assert (value != NULL);

	preferences_entry_free (value);
}

static void
preferences_global_table_free (void)
{
	if (global_table == NULL) {
		return;
	}
	
	g_hash_table_foreach (global_table, preferences_entry_free_func, NULL);
	g_hash_table_destroy (global_table);
	global_table = NULL;

	g_free (storage_path);
	storage_path = NULL;
}

static GHashTable *
preferences_global_table_get_global (void)
{
	static gboolean at_exit_handler_added = FALSE;

	if (global_table == NULL) {
		global_table = g_hash_table_new (g_str_hash, g_str_equal);

		if (!at_exit_handler_added) {
			at_exit_handler_added = TRUE;
			g_atexit (preferences_global_table_free);
		}
	}
	
	return global_table;
}

static PreferencesEntry *
preferences_global_table_lookup (const char *name)
{
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (preferences_global_table_get_global () != NULL, NULL);
	
	return g_hash_table_lookup (preferences_global_table_get_global (), name);
}

static PreferencesEntry *
preferences_global_table_insert (const char *name)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (preferences_global_table_get_global () != NULL, NULL);
	g_return_val_if_fail (preferences_global_table_lookup (name) == NULL, NULL);
	
	entry = g_new0 (PreferencesEntry, 1);
	entry->name = g_strdup (name);

	g_hash_table_insert (preferences_global_table_get_global (), entry->name, entry);

	g_return_val_if_fail (entry == preferences_global_table_lookup (name), NULL);

	/* Update the cached value for the first time.
	 * 
	 * We need to do this because checks for value changes
	 * happen not only as a result of callbacks triggering, but 
	 * also as a result of user_level changes.  When a user level
	 * changes, all the preferences entries are iterated to invoke
	 * callbacks for those that changed as a result.
	 *
	 * See preferences_global_table_check_changes_function().
	 */
	preferences_entry_update_cached_value (entry);

	return entry;
}

static PreferencesEntry *
preferences_global_table_lookup_or_insert (const char *name)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, NULL);
	
	entry = preferences_global_table_lookup (name);

	if (entry != NULL) {
		return entry;
	}

	entry = preferences_global_table_insert (name);
	g_assert (entry != NULL);

	return entry;
}

void
eel_preferences_add_callback (const char *name,
			      EelPreferencesCallback callback,
			      gpointer callback_data)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (callback != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	preferences_entry_add_callback (entry, callback, callback_data);
}

void
eel_preferences_add_auto_string (const char *name,
				 const char **storage)
{
	PreferencesEntry *entry;
	char *value;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	preferences_entry_add_auto_storage (entry, storage, PREFERENCE_STRING);

	value = eel_preferences_get (entry->name);
	update_auto_string (storage, value);
	g_free (value);
}

void
eel_preferences_add_auto_string_list (const char *name,
				      const EelStringList **storage)
{
	PreferencesEntry *entry;
	EelStringList *value;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	preferences_entry_add_auto_storage (entry, storage, PREFERENCE_STRING_LIST);

	value = eel_preferences_get_string_list (entry->name);
	update_auto_string_list (storage, value);
	eel_string_list_free (value);
}

void
eel_preferences_add_auto_integer (const char *name,
				  int *storage)
{
	PreferencesEntry *entry;
	int value;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	preferences_entry_add_auto_storage (entry, storage, PREFERENCE_INTEGER);

	value = eel_preferences_get_integer (entry->name);
	update_auto_integer_or_boolean (storage, GINT_TO_POINTER (value));
}

void
eel_preferences_add_auto_boolean (const char *name,
				  gboolean *storage)
{
	PreferencesEntry *entry;
	gboolean value;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	preferences_entry_add_auto_storage (entry, storage, PREFERENCE_BOOLEAN);

	value = eel_preferences_get_boolean (entry->name);
	update_auto_integer_or_boolean (storage, GINT_TO_POINTER (value));
}

void
eel_preferences_remove_auto_string (const char *name,
				    const char **storage)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup (name);
	if (entry == NULL) {
		g_warning ("Trying to remove auto-string for %s without adding it first.", name);
		return;
	}

	preferences_entry_remove_auto_storage (entry, storage);
}

void
eel_preferences_remove_auto_string_list (const char *name,
					 const EelStringList **storage)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup (name);
	if (entry == NULL) {
		g_warning ("Trying to remove auto-string for %s without adding it first.", name);
		return;
	}

	preferences_entry_remove_auto_storage (entry, storage);
}

void
eel_preferences_remove_auto_integer (const char *name,
				     int *storage)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup (name);
	if (entry == NULL) {
		g_warning ("Trying to remove auto-integer for %s without adding it first.", name);
		return;
	}

	preferences_entry_remove_auto_storage (entry, storage);
}

void
eel_preferences_remove_auto_boolean (const char *name,
				     gboolean *storage)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (storage != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup (name);
	if (entry == NULL) {
		g_warning ("Trying to remove auto-boolean for %s without adding it first.", name);
		return;
	}

	preferences_entry_remove_auto_storage (entry, storage);
}

typedef struct
{
	char *name;
	EelPreferencesCallback callback;
	gpointer callback_data;
} WhileAliveData;

static void
preferences_while_alive_disconnector (GtkObject *object, gpointer callback_data)
{
	WhileAliveData *data;

	g_return_if_fail (GTK_IS_OBJECT (object));
	g_return_if_fail (callback_data != NULL);

	data = callback_data;

	eel_preferences_remove_callback (data->name,
					 data->callback,
					 data->callback_data);

	g_free (data->name);
	g_free (data);
}

void
eel_preferences_add_callback_while_alive (const char *name,
					  EelPreferencesCallback callback,
					  gpointer callback_data,
					  GtkObject *alive_object)
{
	WhileAliveData *data;

	g_return_if_fail (name != NULL);
	g_return_if_fail (callback != NULL);
	g_return_if_fail (GTK_IS_OBJECT (alive_object));
	g_return_if_fail (preferences_is_initialized ());

	data = g_new (WhileAliveData, 1);
	data->name = g_strdup (name);
	data->callback = callback;
	data->callback_data = callback_data;

	eel_preferences_add_callback (name, callback, callback_data);

	gtk_signal_connect (alive_object,
			    "destroy",
			    GTK_SIGNAL_FUNC (preferences_while_alive_disconnector),
			    data);
}

void
eel_preferences_remove_callback (const char *name,
				 EelPreferencesCallback callback,
				 gpointer callback_data)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (callback != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup (name);

	if (entry == NULL) {
		g_warning ("Trying to remove a callback for %s without adding it first.", name);
		return;
	}
	
	preferences_entry_remove_callback (entry, callback, callback_data);
}

void
eel_preferences_set_description (const char *name,
				 const char *description)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (description != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	g_free (entry->description);
	entry->description = g_strdup (description);
}

char *
eel_preferences_get_description (const char *name)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (preferences_is_initialized (), NULL);

	entry = preferences_global_table_lookup_or_insert (name);

	return g_strdup (entry->description ? entry->description : "");
}

void
eel_preferences_set_enumeration_id (const char *name,
				    const char *enumeration_id)
{
	PreferencesEntry *entry;

	g_return_if_fail (name != NULL);
	g_return_if_fail (enumeration_id != NULL);
	g_return_if_fail (preferences_is_initialized ());

	entry = preferences_global_table_lookup_or_insert (name);
	g_assert (entry != NULL);

	g_free (entry->enumeration_id);
	entry->enumeration_id = g_strdup (enumeration_id);
}

char *
eel_preferences_get_enumeration_id (const char *name)
{
	PreferencesEntry *entry;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (preferences_is_initialized (), NULL);

	entry = preferences_global_table_lookup_or_insert (name);

	return entry->enumeration_id ? g_strdup (entry->enumeration_id) : NULL;
}

char *
eel_preferences_get_user_level_name_for_display (int user_level)
{
	g_return_val_if_fail (preferences_is_initialized (), NULL);

	user_level = eel_preferences_user_level_clamp (user_level);
	
	return g_strdup (_(user_level_names_for_display[user_level]));
}

char *
eel_preferences_get_user_level_name_for_storage (int user_level)
{
	g_return_val_if_fail (preferences_is_initialized (), NULL);

	return g_strdup (preferences_peek_user_level_name_for_storage (user_level));
}

int
eel_preferences_user_level_clamp (int user_level)
{
	g_return_val_if_fail (preferences_is_initialized (), 0);

	return CLAMP (user_level, EEL_USER_LEVEL_NOVICE, EEL_USER_LEVEL_ADVANCED);
}

gboolean
eel_preferences_user_level_is_valid (int user_level)
{
	g_return_val_if_fail (preferences_is_initialized (), FALSE);

	return user_level == eel_preferences_user_level_clamp (user_level);
}

gboolean
eel_preferences_monitor_directory (const char *directory)
{
	g_return_val_if_fail (preferences_is_initialized (), FALSE);

	return eel_gconf_monitor_add (directory);
}

gboolean
eel_preferences_visible_in_current_user_level (const char *name)
{
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (preferences_is_initialized (), FALSE);

	return eel_preferences_get_visible_user_level (name)
		<= eel_preferences_get_user_level ();
}

gboolean
eel_preferences_is_visible (const char *name)
{
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (preferences_is_initialized (), FALSE);

	if (!eel_preferences_visible_in_current_user_level (name)) {
		return FALSE;
	}

	return !preferences_global_table_lookup_or_insert (name)->invisible;
}

static void
preferences_remove_user_level_notice (void)
{
	eel_gconf_notification_remove (user_level_changed_connection_id);
	user_level_changed_connection_id = EEL_GCONF_UNDEFINED_CONNECTION;
}

void
eel_preferences_initialize (const char *path)
{
	g_return_if_fail (eel_strlen (path) > 0);
	
	if (initialized) {
		return;
	}

	initialized = TRUE;

	user_level_changed_connection_id = eel_gconf_notification_add (USER_LEVEL_KEY,
								       preferences_user_level_changed_notice,
								       NULL);

	g_atexit (preferences_remove_user_level_notice);

	preferences_set_storage_path (path);
}

#if !defined (EEL_OMIT_SELF_CHECK)

#define CHECK_BOOLEAN(name__, value__)								\
G_STMT_START {											\
	eel_preferences_set_boolean ((name__), (value__));					\
	EEL_CHECK_BOOLEAN_RESULT (eel_preferences_get_boolean (name__), (value__));	\
} G_STMT_END

#define CHECK_INTEGER(name__, value__)								\
G_STMT_START {											\
	eel_preferences_set_integer ((name__), (value__));					\
	EEL_CHECK_INTEGER_RESULT (eel_preferences_get_integer (name__), (value__));	\
} G_STMT_END

#define CHECK_STRING(name__, value__)							\
G_STMT_START {										\
	eel_preferences_set ((name__), (value__));					\
	EEL_CHECK_STRING_RESULT (eel_preferences_get (name__), (value__));		\
} G_STMT_END

void
eel_self_check_preferences (void)
{
	/* Disabled until I can debug why these seemingly harmless tests 
	 * dont work. -re
	 */
#if 0
	int original_user_level;

	original_user_level = eel_preferences_get_user_level ();

 	EEL_CHECK_INTEGER_RESULT (eel_preferences_get_integer ("self-check/neverset/i"), 0);
 	EEL_CHECK_STRING_RESULT (eel_preferences_get ("self-check/neverset/s"), "");
 	EEL_CHECK_BOOLEAN_RESULT (eel_preferences_get_boolean ("self-check/neverset/b"), FALSE);

	eel_preferences_set_user_level (0);

	/* FIXME: Fails if you add the commented-out lines. */
	/* CHECK_INTEGER ("self-check/i", 0); */
	CHECK_INTEGER ("self-check/i", 666);
	/* CHECK_BOOLEAN ("self-check/b", FALSE); */
	CHECK_BOOLEAN ("self-check/b", TRUE);
	/* CHECK_STRING ("self-check/s", ""); */
	CHECK_STRING ("self-check/s", "foo");

	/* Restore the original user level */
	eel_preferences_set_user_level (original_user_level);
#endif
}

#endif /* !EEL_OMIT_SELF_CHECK */
