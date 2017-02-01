/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oafd: OAF CORBA dameon.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 1999, 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Elliot Lee <sopwith@redhat.com>
 *           Maciej Stachowiak <mjs@eazel.com>
 *
 */

#include "oafd.h"
#include "oaf-i18n.h"
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <parser.h>      /* gnome-xml */
#include <xmlmemory.h>   /* gnome-xml */


static gboolean od_string_to_boolean      (const char *str);
static gboolean od_filename_has_extension (const char *filename,
                                           const char *extension);


static void
od_entry_read_props (OAF_ServerInfo *server, xmlNodePtr node)
{
	int i, n;
	xmlNodePtr sub;
	OAF_Property *curprop;

	for (n = 0, sub = node->xmlChildrenNode; sub; sub = sub->next) {
		if (sub->type != XML_ELEMENT_NODE) {
			continue;
                }

		if (strcasecmp (sub->name, "oaf_attribute") != 0 &&
                    strcasecmp (sub->name, "oaf_property") != 0) {
			continue;
                }
                
		n++;
	}

	server->props._length = n;
	server->props._buffer = g_new0 (OAF_Property, n);

        curprop = server->props._buffer;

	for (i = 0, sub = node->xmlChildrenNode; i < n; sub = sub->next, i++) {
		char *type, *valuestr;

		type = xmlGetProp (sub, "type");
		if (type == NULL) {
			continue;
                }

		valuestr = xmlGetProp (sub, "name");
		if (valuestr == NULL) {
			free (type);
			continue;
		}
		if(valuestr[0] == '_') {
			g_error("%s is an invalid property name "
				"- property names beginning with '_' are reserved",
				valuestr);
                }
		curprop->name = CORBA_string_dup (valuestr);
		free (valuestr);

		if (strcasecmp (type, "stringv") == 0) {
			int j, o;
			xmlNodePtr sub2;

			curprop->v._d = OAF_P_STRINGV;

			for (o = 0, sub2 = sub->xmlChildrenNode; sub2;
			     sub2 = sub2->next) {
				if (sub2->type != XML_ELEMENT_NODE) {
					continue;
                                }
				if (strcasecmp (sub2->name, "item") != 0) {
					continue;
                                }

				o++;
			}

			curprop->v._u.value_stringv._length = o;
			curprop->v._u.value_stringv._buffer =
				CORBA_sequence_CORBA_string_allocbuf (o);

			for (j = 0, sub2 = sub->xmlChildrenNode; 
                             j < o;
			     sub2 = sub2->next, j++) {
				valuestr = xmlGetProp (sub2, "value");
				if (valuestr) {
					curprop->v._u.
						value_stringv._buffer[j] =
						CORBA_string_dup (valuestr);
				} else {
					g_warning
						(_("Property '%s' has no value"),
						 curprop->name);
					curprop->v._u.
						value_stringv._buffer[j] =
						CORBA_string_dup ("");
				}
				xmlFree (valuestr);
			}

		} else if (strcasecmp (type, "number") == 0) {
			valuestr = xmlGetProp (sub, "value");

			curprop->v._d = OAF_P_NUMBER;
			curprop->v._u.value_number = atof (valuestr);

			xmlFree (valuestr);
		} else if (strcasecmp (type, "boolean") == 0) {
			valuestr = xmlGetProp (sub, "value");
			curprop->v._d = OAF_P_BOOLEAN;
			curprop->v._u.value_boolean =
				od_string_to_boolean (valuestr);
			xmlFree (valuestr);
		} else {
			valuestr = xmlGetProp (sub, "value");
			/* Assume string */
			curprop->v._d = OAF_P_STRING;
			if (valuestr != NULL) {
				curprop->v._u.value_string =
					CORBA_string_dup (valuestr);
			} else {
                                g_warning (_("Property '%s' has no value"),
					   curprop->name);
				curprop->v._u.value_string =
					CORBA_string_dup ("");
			}
			xmlFree (valuestr);
		}
                
		free (type);
                
		curprop++;
	}
}

static char *
od_validate (const char *iid, const char *type, const char *location)
{
        int i;

        if (iid == NULL) {
                return g_strdup (_("a NULL iid is not valid"));
        }

        if (type == NULL) {
                return g_strdup_printf (_("iid %s has a NULL type"), iid);
        }

        if (location == NULL) {
                return g_strdup_printf (_("iid %s has a NULL location"), iid);
        }

        for (i = 0; iid && iid [i]; i++) {
                char c = iid [i];

                if (c == ',' || c == '[' || c == ']' ||
                    /* Reserved for future expansion */
                    c == '!' || c == '#' || c == '|') {
                        return g_strdup_printf (_("invalid character '%c' in iid '%s'"),
                                                c, iid);
                }
        }

        return NULL;
}




static void
od_process_server_xml_node (xmlNodePtr node,
                            GSList    **entries,
                            const char *host, 
                            const char *domain)
{
        GSList *cur;
        OAF_ServerInfo *server;
        char *iid, *type, *location, *error;
        gboolean already_there;
        
        if (node->type != XML_ELEMENT_NODE) {
                /* syslog error */
                return;
        }
        
        /* I'd love to use XML namespaces, but unfortunately they appear
         * to require putting complicated stuff into the .oaf file, 
         * and even more complicated stuff to use. 
         */
        
        if (strcasecmp (node->name, "oaf_server")) {
                /* FIXME: syslog the error */
                return;
        }
                
        iid = xmlGetProp (node, "iid");
        type = xmlGetProp (node, "type");
        location = xmlGetProp (node, "location");
        
        error = od_validate (iid, type, location);

        if (error != NULL) {
                /* FIXME: should syslog */
                g_print ("%s", error);
                        
                g_free (error);
                xmlFree (iid);
                xmlFree (type);
                xmlFree (location);

                return;
        }
                
        /* make sure the component has not been already read. If so,
           do not add this entry to the entries list */
        already_there = FALSE;
        
        for (cur = *entries; cur != NULL; cur = cur->next) {
                if (strcmp (((OAF_ServerInfo *) cur->data)->iid, iid) == 0) {
                        already_there = TRUE;
                        break;
                }
        }
        
        if (already_there == FALSE) {
                server = g_new0 (OAF_ServerInfo, 1);
                
                server->iid = CORBA_string_dup (iid);
                server->server_type = CORBA_string_dup (type);
                server->location_info = CORBA_string_dup (location);
                server->hostname = CORBA_string_dup (host);
                server->domain = CORBA_string_dup (domain);
                server->username = CORBA_string_dup (g_get_user_name ());
                
                od_entry_read_props (server, node);
                
                *entries = g_slist_prepend (*entries, server);
        }
                
        xmlFree (iid);
        xmlFree (type);
        xmlFree (location);
}



static void
od_load_file (const char *file,
              GSList    **entries,
              const char *host, 
              const char *domain)
{
        xmlNodePtr node;
        xmlDocPtr document;
       
        document = xmlParseFile (file);
        
        if (document == NULL) {
                /* FIXME: syslog the error */
                return;
        }
        
        if (strcasecmp (document->xmlRootNode->name, "oaf_info") == 0) {
                node = document->xmlRootNode->xmlChildrenNode;
        } else {
                node = document->xmlRootNode;
        }
        
        while (node != NULL) {
                od_process_server_xml_node (node, entries, host, domain);
                node = node->next;
        }
        
        xmlFreeDoc (document);
}

static void
od_load_directory (const char *directory,
                   GSList    **entries,
                   const char *host, 
                   const char *domain)
{
	DIR *directory_handle;
	struct dirent *directory_entry;
        char *pathname;

        
        /* FIXME: Should be a syslog message. */
        /* g_print (_("Trying dir %s\n"), directory); */

        directory_handle = opendir (directory);

        if (directory_handle == NULL) {
                /* FIXME */
                return;
        }
        
        for (directory_entry = readdir (directory_handle);
             directory_entry != NULL;
             directory_entry = readdir (directory_handle)) {
                pathname = g_strdup_printf ("%s/%s", directory, directory_entry->d_name);

                if (od_filename_has_extension (pathname, ".oaf") ||
                    od_filename_has_extension (pathname, ".oafinfo")) {
                        od_load_file (pathname, entries, host, domain);
                }

		g_free (pathname);
        }

        closedir (directory_handle);
}


void
OAF_ServerInfo_load (char **directories,
                     OAF_ServerInfoList   *servers,
		     GHashTable **iid_to_server_info_map,
		     const char *host, const char *domain)
{
	GSList *entries;
        int length;
        GSList *p;
	int i, j; 

	g_return_if_fail (directories);
	g_return_if_fail (iid_to_server_info_map);

        entries = NULL;

	if (*iid_to_server_info_map != NULL) {
		g_hash_table_destroy (*iid_to_server_info_map);
        }

	*iid_to_server_info_map = g_hash_table_new (g_str_hash, g_str_equal);

        /* Load each directory */
	for (i = 0; directories[i] != NULL; i++) {
                od_load_directory (directories[i], &entries, host, domain);
	}

	/* Now convert 'entries' into something that the server can store and pass back */
	length = g_slist_length (entries);

	servers->_buffer = CORBA_sequence_OAF_ServerInfo_allocbuf (length);
        servers->_length = length;

	g_hash_table_freeze (*iid_to_server_info_map);

	for (j = 0, p = entries; j < length; j++, p = p->next) {
		memcpy (&servers->_buffer[j], p->data, sizeof (OAF_ServerInfo));
		g_hash_table_insert (*iid_to_server_info_map, servers->_buffer[j].iid, &servers->_buffer[j]);
	}

	g_hash_table_thaw (*iid_to_server_info_map);

        g_slist_foreach (entries, (GFunc) g_free, NULL);
        g_slist_free (entries);
}




static gboolean 
od_string_to_boolean (const char *str)
{
	if (strcasecmp (str, "true") == 0
	    || strcasecmp (str, "yes") == 0
	    || strcmp (str, "1") == 0) {
		return TRUE;
	} else {
		return FALSE;
        }
}


static gboolean
od_filename_has_extension (const char *filename,
                           const char *extension)
{
        char *last_dot;
        
        last_dot = strrchr (filename, '.');

        return last_dot != NULL && strcmp (last_dot, extension) == 0;
}
