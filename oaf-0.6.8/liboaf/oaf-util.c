/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */


#include "oaf-util.h"
#include <string.h>

/**
 * oaf_server_info_prop_find:
 * @server: the server where to seek the data.
 * @prop_name: the data to seek in the server.
 *
 * Tries to find a server with the given property. Returns
 * NULL if not found.
 *
 * Return value: a pointer to the %OAF_Property structure.
 */
OAF_Property *
oaf_server_info_prop_find (OAF_ServerInfo * server, const char *prop_name)
{
	int i;

	for (i = 0; i < server->props._length; i++) {
		if (!strcmp (server->props._buffer[i].name, prop_name))
			return &server->props._buffer[i];
	}

	return NULL;
}

/**
 * oaf_server_info_prop_lookup:
 * @server: 
 * @propname:
 * @i18n_languages:
 *
 *
 * Return value: 
 */
const char *
oaf_server_info_prop_lookup (OAF_ServerInfo * server, const char *prop_name,
			     GSList * i18n_languages)
{
	GSList *cur;
	OAF_Property *prop;
        const char *retval;
        char *prop_name_buf;
        char short_lang[3];
                     
	if (i18n_languages) {
		for (cur = i18n_languages; cur; cur = cur->next) {
                        prop_name_buf = g_strdup_printf ("%s-%s", prop_name, (char *) cur->data);

			retval = oaf_server_info_prop_lookup (server, prop_name_buf, NULL);
                        g_free (prop_name_buf);

                        if (!retval) {
                                if (strlen ((char *) cur->data) > 2) {
                                        strncpy (short_lang, (char *) cur->data, 2);
                                        prop_name_buf = g_strdup_printf ("%s-%s", prop_name, short_lang);
                                        retval = oaf_server_info_prop_lookup (server, prop_name_buf, NULL);
                                        g_free (prop_name_buf);
                                }
                        }

			if (retval)
				return retval;
		}
	} 

        prop = oaf_server_info_prop_find (server, prop_name);
        if (prop != NULL && prop->v._d == OAF_P_STRING)
                return prop->v._u.value_string;

	return NULL;
}

static void
CORBA_sequence_CORBA_string_copy (CORBA_sequence_CORBA_string *copy, const CORBA_sequence_CORBA_string *original)
{
	int i;

	copy->_maximum = original->_length;
	copy->_length = original->_length;
	copy->_buffer = CORBA_sequence_CORBA_string_allocbuf (original->_length);

	for (i = 0; i < original->_length; i++) {
		copy->_buffer[i] = CORBA_string_dup (original->_buffer[i]);
	}

	CORBA_sequence_set_release (copy, TRUE);
}

void
OAF_PropertyValue_copy (OAF_PropertyValue *copy, const OAF_PropertyValue *original)
{
	copy->_d = original->_d;
	switch (original->_d) {
	case OAF_P_STRING:
		copy->_u.value_string =	CORBA_string_dup (original->_u.value_string);
		break;
	case OAF_P_NUMBER:
		copy->_u.value_number =	original->_u.value_number;
		break;
	case OAF_P_BOOLEAN:
		copy->_u.value_boolean = original->_u.value_boolean;
		break;
	case OAF_P_STRINGV:
		CORBA_sequence_CORBA_string_copy
			(&copy->_u.value_stringv,
			 &original->_u.value_stringv);
		break;
	default:
		g_assert_not_reached ();
	}
}

void
OAF_Property_copy (OAF_Property *copy, const OAF_Property *original)
{
	copy->name = CORBA_string_dup (original->name);
	OAF_PropertyValue_copy (&copy->v, &original->v);
}

void
CORBA_sequence_OAF_Property_copy (CORBA_sequence_OAF_Property *copy, const CORBA_sequence_OAF_Property *original)
{
	int i;

	copy->_maximum = original->_length;
	copy->_length = original->_length;
	copy->_buffer = CORBA_sequence_OAF_Property_allocbuf (original->_length);

	for (i = 0; i < original->_length; i++) {
		OAF_Property_copy (&copy->_buffer[i], &original->_buffer[i]);
	}

	CORBA_sequence_set_release (copy, TRUE);
}

void
OAF_ServerInfo_copy (OAF_ServerInfo *copy, const OAF_ServerInfo *original)
{
	copy->iid = CORBA_string_dup (original->iid);
	copy->server_type = CORBA_string_dup (original->server_type);
	copy->location_info = CORBA_string_dup (original->location_info);
	copy->username = CORBA_string_dup (original->username);
	copy->hostname = CORBA_string_dup (original->hostname);
	copy->domain = CORBA_string_dup (original->domain);
	CORBA_sequence_OAF_Property_copy (&copy->props, &original->props);
}


/**
 * OAF_ServerInfo_duplicate:
 * @original: %ServerInfo to copy.
 *
 * The return value should befreed with CORBA_free (). 
 *
 * Return value: a newly allocated copy of @original.
 */
OAF_ServerInfo *
OAF_ServerInfo_duplicate (const OAF_ServerInfo *original)
{
	OAF_ServerInfo *copy;

	copy = OAF_ServerInfo__alloc ();
	OAF_ServerInfo_copy (copy, original);
	
	return copy;
}


