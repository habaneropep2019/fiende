/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-gconf-extensions.c - Stuff to make GConf easier to use.

   Copyright (C) 2000, 2001 Eazel, Inc.

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
#include "eel-gconf-extensions.h"

#include "eel-glib-extensions.h"
#include "eel-stock-dialogs.h"
#include "eel-string.h"

#include <gconf/gconf-client.h>
#include <gconf/gconf.h>
#include <libgnome/gnome-i18n.h>

static GConfClient *global_gconf_client = NULL;

static void
global_client_free (void)
{
	if (global_gconf_client == NULL) {
		return;
	}
	
	gtk_object_unref (GTK_OBJECT (global_gconf_client));
	global_gconf_client = NULL;
}

/* Public */
GConfClient *
eel_gconf_client_get_global (void)
{
	/* Initialize gconf if needed */
	if (!gconf_is_initialized ()) {
		char *argv[] = { "eel-preferences", NULL };
		GError *error = NULL;
		
		if (!gconf_init (1, argv, &error)) {
			if (eel_gconf_handle_error (&error)) {
				return NULL;
			}
		}
	}
	
	if (global_gconf_client == NULL) {
		global_gconf_client = gconf_client_get_default ();
		g_atexit (global_client_free);
	}
	
	return global_gconf_client;
}

gboolean
eel_gconf_handle_error (GError **error)
{
	char *message;
	static gboolean shown_dialog = FALSE;
	
	g_return_val_if_fail (error != NULL, FALSE);

	if (*error != NULL) {
		g_warning (_("GConf error:\n  %s"), (*error)->message);
		if (! shown_dialog) {
			shown_dialog = TRUE;

			message = g_strdup_printf (_("GConf error:\n  %s\n"
						     "All further errors shown "
						     "only on terminal"),
						   (*error)->message);
			eel_show_error_dialog (message, _("GConf Error"), NULL);
			g_free (message);
		}
		g_error_free (*error);
		*error = NULL;

		return TRUE;
	}

	return FALSE;
}

void
eel_gconf_set_boolean (const char *key,
			    gboolean boolean_value)
{
	GConfClient *client;
	GError *error = NULL;
	
	g_return_if_fail (key != NULL);

	client = eel_gconf_client_get_global ();
	g_return_if_fail (client != NULL);
	
	gconf_client_set_bool (client, key, boolean_value, &error);
	eel_gconf_handle_error (&error);
}

gboolean
eel_gconf_get_boolean (const char *key)
{
	gboolean result;
	GConfClient *client;
	GError *error = NULL;
	
	g_return_val_if_fail (key != NULL, FALSE);
	
	client = eel_gconf_client_get_global ();
	g_return_val_if_fail (client != NULL, FALSE);
	
	result = gconf_client_get_bool (client, key, &error);
	
	if (eel_gconf_handle_error (&error)) {
		result = FALSE;
	}
	
	return result;
}

void
eel_gconf_set_integer (const char *key,
			    int int_value)
{
	GConfClient *client;
	GError *error = NULL;

	g_return_if_fail (key != NULL);

	client = eel_gconf_client_get_global ();
	g_return_if_fail (client != NULL);

	gconf_client_set_int (client, key, int_value, &error);
	eel_gconf_handle_error (&error);
}

int
eel_gconf_get_integer (const char *key)
{
	int result;
	GConfClient *client;
	GError *error = NULL;
	
	g_return_val_if_fail (key != NULL, 0);
	
	client = eel_gconf_client_get_global ();
	g_return_val_if_fail (client != NULL, 0);
	
	result = gconf_client_get_int (client, key, &error);

	if (eel_gconf_handle_error (&error)) {
		result = 0;
	}

	return result;
}

void
eel_gconf_set_string (const char *key,
			   const char *string_value)
{
	GConfClient *client;
	GError *error = NULL;

	g_return_if_fail (key != NULL);

	client = eel_gconf_client_get_global ();
	g_return_if_fail (client != NULL);
	
	gconf_client_set_string (client, key, string_value, &error);
	eel_gconf_handle_error (&error);
}

char *
eel_gconf_get_string (const char *key)
{
	char *result;
	GConfClient *client;
	GError *error = NULL;
	
	g_return_val_if_fail (key != NULL, NULL);
	
	client = eel_gconf_client_get_global ();
	g_return_val_if_fail (client != NULL, NULL);
	
	result = gconf_client_get_string (client, key, &error);
	
	if (eel_gconf_handle_error (&error)) {
		result = g_strdup ("");
	}
	
	return result;
}

void
eel_gconf_set_string_list (const char *key,
				const GSList *slist)
{
	GConfClient *client;
	GError *error;

	g_return_if_fail (key != NULL);

	client = eel_gconf_client_get_global ();
	g_return_if_fail (client != NULL);

	error = NULL;
	gconf_client_set_list (client, key, GCONF_VALUE_STRING,
			       /* Need cast cause of GConf api bug */
			       (GSList *) slist,
			       &error);
	eel_gconf_handle_error (&error);
}

GSList *
eel_gconf_get_string_list (const char *key)
{
	GSList *slist;
	GConfClient *client;
	GError *error;
	
	g_return_val_if_fail (key != NULL, NULL);
	
	client = eel_gconf_client_get_global ();
	g_return_val_if_fail (client != NULL, NULL);
	
	error = NULL;
	slist = gconf_client_get_list (client, key, GCONF_VALUE_STRING, &error);
	if (eel_gconf_handle_error (&error)) {
		slist = NULL;
	}

	return slist;
}

gboolean
eel_gconf_is_default (const char *key)
{
	gboolean result;
	GConfValue *value;
	GError *error = NULL;
	
	g_return_val_if_fail (key != NULL, FALSE);
	
	value = gconf_client_get_without_default  (eel_gconf_client_get_global (), key, &error);

	if (eel_gconf_handle_error (&error)) {
		if (value != NULL) {
			gconf_value_free (value);
		}
		return FALSE;
	}

	result = (value == NULL);
	eel_gconf_value_free (value);
	return result;
}

gboolean
eel_gconf_monitor_add (const char *directory)
{
	GError *error = NULL;
	GConfClient *client;

	g_return_val_if_fail (directory != NULL, FALSE);

	client = gconf_client_get_default ();
	g_return_val_if_fail (client != NULL, FALSE);

	gconf_client_add_dir (client,
			      directory,
			      GCONF_CLIENT_PRELOAD_NONE,
			      &error);
	
	if (eel_gconf_handle_error (&error)) {
		return FALSE;
	}

	return TRUE;
}

gboolean
eel_gconf_monitor_remove (const char *directory)
{
	GError *error = NULL;
	GConfClient *client;

	if (directory == NULL) {
		return FALSE;
	}

	client = gconf_client_get_default ();
	g_return_val_if_fail (client != NULL, FALSE);
	
	gconf_client_remove_dir (client,
				 directory,
				 &error);
	
	if (eel_gconf_handle_error (&error)) {
		return FALSE;
	}
	
	return TRUE;
}

void
eel_gconf_suggest_sync (void)
{
	GConfClient *client;
	GError *error = NULL;

	client = eel_gconf_client_get_global ();
	g_return_if_fail (client != NULL);
	
	gconf_client_suggest_sync (client, &error);
	eel_gconf_handle_error (&error);
}

GConfValue*
eel_gconf_get_value (const char *key)
{
	GConfValue *value = NULL;
	GConfClient *client;
	GError *error = NULL;

	g_return_val_if_fail (key != NULL, NULL);

	client = eel_gconf_client_get_global ();
	g_return_val_if_fail (client != NULL, NULL);

	value = gconf_client_get (client, key, &error);
	
	if (eel_gconf_handle_error (&error)) {
		if (value != NULL) {
			gconf_value_free (value);
			value = NULL;
		}
	}

	return value;
}

static gboolean
simple_value_is_equal (const GConfValue *a,
		       const GConfValue *b)
{
	g_return_val_if_fail (a != NULL, FALSE);
	g_return_val_if_fail (b != NULL, FALSE);

	switch (a->type) {
	case GCONF_VALUE_STRING:
		return eel_str_is_equal (a->d.string_data, b->d.string_data);
		break;

	case GCONF_VALUE_INT:
		return a->d.int_data == b->d.int_data;
		break;

	case GCONF_VALUE_FLOAT:
		return a->d.float_data == b->d.float_data;
		break;

	case GCONF_VALUE_BOOL:
		return a->d.bool_data == b->d.bool_data;
		break;
	default:
		g_assert_not_reached ();
	}
	
	return FALSE;
}

gboolean
eel_gconf_value_is_equal (const GConfValue *a,
			       const GConfValue *b)
{
	GSList *node_a;
	GSList *node_b;

	if (a == NULL && b == NULL) {
		return TRUE;
	}

	if (a == NULL || b == NULL) {
		return FALSE;
	}

	if (a->type != b->type) {
		return FALSE;
	}

	switch (a->type) {
	case GCONF_VALUE_STRING:
	case GCONF_VALUE_INT:
	case GCONF_VALUE_FLOAT:
	case GCONF_VALUE_BOOL:
		return simple_value_is_equal (a, b);
		break;
		
	case GCONF_VALUE_LIST:
		if (a->d.list_data.type != b->d.list_data.type) {
			return FALSE;
		}

		if (a->d.list_data.list == NULL && b->d.list_data.list == NULL) {
			return TRUE;
		}

		if (a->d.list_data.list != NULL || b->d.list_data.list != NULL) {
			return FALSE;
		}

		if (g_slist_length (a->d.list_data.list) != g_slist_length (b->d.list_data.list)) {
			return FALSE;
		}
		
		for (node_a = a->d.list_data.list,node_b = b->d.list_data.list;
		     node_a != NULL && node_b != NULL;
		     node_a = node_a->next, node_b = node_b->next) {
			g_assert (node_a->data != NULL);
			g_assert (node_b->data != NULL);
			if (!simple_value_is_equal (node_a->data, node_b->data)) {
				return FALSE;
			}
		}
		
		return TRUE;
	default:
		/* FIXME: pair ? */
		g_assert (0);
	}
	
	g_assert_not_reached ();
	return FALSE;
}

void
eel_gconf_value_free (GConfValue *value)
{
	if (value == NULL) {
		return;
	}
	
	gconf_value_free (value);
}

guint
eel_gconf_notification_add (const char *key,
				 GConfClientNotifyFunc notification_callback,
				 gpointer callback_data)
{
	guint notification_id;
	GConfClient *client;
	GError *error = NULL;
	
	g_return_val_if_fail (key != NULL, EEL_GCONF_UNDEFINED_CONNECTION);
	g_return_val_if_fail (notification_callback != NULL, EEL_GCONF_UNDEFINED_CONNECTION);

	client = eel_gconf_client_get_global ();
	g_return_val_if_fail (client != NULL, EEL_GCONF_UNDEFINED_CONNECTION);
	
	notification_id = gconf_client_notify_add (client,
						   key,
						   notification_callback,
						   callback_data,
						   NULL,
						   &error);
	
	if (eel_gconf_handle_error (&error)) {
		if (notification_id != EEL_GCONF_UNDEFINED_CONNECTION) {
			gconf_client_notify_remove (client, notification_id);
			notification_id = EEL_GCONF_UNDEFINED_CONNECTION;
		}
	}
	
	return notification_id;
}

void
eel_gconf_notification_remove (guint notification_id)
{
	GConfClient *client;

	if (notification_id == EEL_GCONF_UNDEFINED_CONNECTION) {
		return;
	}
	
	client = eel_gconf_client_get_global ();
	g_return_if_fail (client != NULL);

	gconf_client_notify_remove (client, notification_id);
}

GSList *
eel_gconf_value_get_string_list (const GConfValue *value)
{
 	GSList *result;
 	const GSList *slist;
 	const GSList *node;
	const char *string;
	const GConfValue *next_value;

	if (value == NULL) {
		return NULL;
	}

	g_return_val_if_fail (value->type == GCONF_VALUE_LIST, NULL);
	g_return_val_if_fail (gconf_value_get_list_type (value) == GCONF_VALUE_STRING, NULL);

	slist = gconf_value_get_list (value);
	result = NULL;
	for (node = slist; node != NULL; node = node->next) {
		next_value = node->data;
		g_return_val_if_fail (next_value != NULL, NULL);
		g_return_val_if_fail (next_value->type == GCONF_VALUE_STRING, NULL);
		string = gconf_value_get_string (next_value);
		result = g_slist_append (result, g_strdup (string));
	}
	return result;
}

void
eel_gconf_value_set_string_list (GConfValue *value,
				 const GSList *string_list)
{
 	const GSList *node;
	GConfValue *next_value;
 	GSList *value_list;

	g_return_if_fail (value->type == GCONF_VALUE_LIST);
	g_return_if_fail (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);

	value_list = NULL;
	for (node = string_list; node != NULL; node = node->next) {
		next_value = gconf_value_new (GCONF_VALUE_STRING);
		gconf_value_set_string (next_value, node->data);
		value_list = g_slist_append (value_list, next_value);
	}

	gconf_value_set_list (value, value_list);

	for (node = value_list; node != NULL; node = node->next) {
		gconf_value_free (node->data);
	}
	g_slist_free (value_list);
}
