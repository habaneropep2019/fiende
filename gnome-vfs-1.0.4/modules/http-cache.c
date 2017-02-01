/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* http-cache.c - Property caching for DAV

   Copyright (C) 2000-2001 Eazel, Inc.

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

   Authors: 
		 Michael Fleming <mfleming@eazel.com>
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "http-cache.h"
#include "http-method.h"

#include <string.h>
#include <gnome-vfs.h>
#include <gnome-vfs-method.h>

#include <pthread.h>
#include <libgnomevfs-pthread/gnome-vfs-pthread.h>


/* Cache file info for 5 minutes */
#define US_CACHE_FILE_INFO (1000 * 1000 * 60 * 5)
/* Cache directory listings for 500 ms */
#define US_CACHE_DIRECTORY (1000 * 500)

/* Mutex for cache data structures */
static GnomeVFSRecursiveMutex cache_rlock;

/* Hash maps char * URI ---> FileInfoCacheEntry */
static GHashTable * gl_file_info_cache = NULL;
/* in-order list of cache entries  for expiration */
static GList * gl_file_info_cache_list = NULL;
static GList * gl_file_info_cache_list_last = NULL;

typedef struct {
	gchar *			uri_string;
	GnomeVFSFileInfo *	file_info;
	utime_t			create_time;
	GList *			my_list_node;	/*node for me in gl_file_info_cache_list*/
	GList *			filenames;	/* List of char * basenames for files that are in this 
						 * collection/directory.  Empty for non-directories
						 */
	gboolean		has_filenames:1;/* For directories, FALSE if the cache does not contain 
						 * the directory's children 
						 */
	gboolean		is_dav:1;	/* Did this result from a PROPFIND or a GET ? */
} FileInfoCacheEntry;

static FileInfoCacheEntry * 	http_cache_entry_new ();
static void 			http_cache_entry_free (FileInfoCacheEntry * entry);
static FileInfoCacheEntry *	http_cache_add_no_strdup (gchar *uri_string, GnomeVFSFileInfo *file_info, gboolean is_dav);
static FileInfoCacheEntry *	http_cache_add (const gchar *uri_string, GnomeVFSFileInfo *file_info, gboolean is_dav);

void
http_cache_init (void)
{
	gnome_vfs_pthread_recursive_mutex_init (&cache_rlock);

	gl_file_info_cache = g_hash_table_new (g_str_hash, g_str_equal);
}

void
http_cache_shutdown (void)
{
	GList *node, *node_next;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	for (	node = g_list_first (gl_file_info_cache_list) ; 
		node != NULL; 
		node = node_next
	) {
		node_next = g_list_next (node);
		http_cache_entry_free ((FileInfoCacheEntry*) node->data);
	}

	g_list_free (gl_file_info_cache_list);
	
	g_hash_table_destroy (gl_file_info_cache);

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);

	gnome_vfs_pthread_recursive_mutex_destroy (&cache_rlock);

}

static FileInfoCacheEntry *
http_cache_entry_new (void)
{
	FileInfoCacheEntry *ret;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	ret = g_new0 (FileInfoCacheEntry, 1);
	ret->create_time = http_util_get_utime();
	
	gl_file_info_cache_list = g_list_prepend (gl_file_info_cache_list, ret);

	/* Note that since we've prepended, gl_file_info_cache_list points to us*/

	ret->my_list_node = gl_file_info_cache_list;

	if (gl_file_info_cache_list_last == NULL) {
		gl_file_info_cache_list_last = ret->my_list_node;
	}

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);

	return ret;
}

/* Warning: as this function removes the cache entry from gl_file_info_cache_list, 
 * callee's must be careful when calling this during a list iteration
 */
static void
http_cache_entry_free (FileInfoCacheEntry * entry)
{
	if (entry) {
		GList *node;
		
		gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

		g_hash_table_remove (gl_file_info_cache, entry->uri_string);
		g_free (entry->uri_string);	/* This is the same string as in the hash table */
		gnome_vfs_file_info_unref (entry->file_info);

		if (gl_file_info_cache_list_last == entry->my_list_node) {
			gl_file_info_cache_list_last = g_list_previous (entry->my_list_node);
		}
	
		gl_file_info_cache_list = g_list_remove_link (gl_file_info_cache_list, entry->my_list_node);
		g_list_free_1 (entry->my_list_node);

		for (node = entry->filenames ; node ; node = g_list_next(node)) {
			g_free (node->data);
		}

		g_list_free (entry->filenames); 
		
		g_free (entry);

		gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);
	}
}

void
http_cache_trim (void)
{
	GList *node, *node_previous;
	utime_t utime_expire;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	utime_expire = http_util_get_utime() - US_CACHE_FILE_INFO;

	for (	node = gl_file_info_cache_list_last ; 
		node && (utime_expire > ((FileInfoCacheEntry *)node->data)->create_time) ;
		node = node_previous
	) {
		node_previous = g_list_previous (node);

		DEBUG_HTTP (("Cache: Expire: '%s'",((FileInfoCacheEntry *)node->data)->uri_string));

		http_cache_entry_free ((FileInfoCacheEntry *)(node->data));
	}

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);
}

/* Note: doesn't bother trimming entries, so the check can fast */
GnomeVFSFileInfo *
http_cache_check (const gchar * uri_string)
{
	FileInfoCacheEntry *entry;
	utime_t utime_expire;
	GnomeVFSFileInfo *ret;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	utime_expire = http_util_get_utime() - US_CACHE_FILE_INFO;

	entry = (FileInfoCacheEntry *)g_hash_table_lookup (gl_file_info_cache, uri_string);

	if (entry && (utime_expire > entry->create_time)) {
		entry = NULL;
	}

	if (entry) {
		gnome_vfs_file_info_ref (entry->file_info);

		DEBUG_HTTP (("Cache: Hit: '%s'", entry->uri_string));

		ret = entry->file_info;
	} else {
		ret = NULL;
	}
	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);
	return ret;
}

static gchar *
http_cache_uri_to_string  (GnomeVFSURI *uri)
{
	gchar *uri_string;
	size_t uri_length;

	uri_string = gnome_vfs_uri_to_string (uri,
				      GNOME_VFS_URI_HIDE_USER_NAME
				      | GNOME_VFS_URI_HIDE_PASSWORD
				      | GNOME_VFS_URI_HIDE_TOPLEVEL_METHOD);

	if (uri_string) {
		uri_length = strlen (uri_string);
		/* Trim off trailing '/'s */
		if ( '/' == uri_string[uri_length-1] ) {
			uri_string[uri_length-1] = 0;
		}
	}

	return uri_string;
}

GnomeVFSFileInfo *
http_cache_check_uri (GnomeVFSURI *uri)
{
	gchar *uri_string;
	GnomeVFSFileInfo *ret;

	uri_string = http_cache_uri_to_string (uri);

	ret = http_cache_check (uri_string);
	g_free (uri_string);
	return ret;
}


/* Directory operations demand fresher cache entries */
GnomeVFSFileInfo *
http_cache_check_directory (const gchar * uri_string, GList **p_child_file_info_list)
{
	FileInfoCacheEntry *entry;
	utime_t utime_expire;
	GnomeVFSFileInfo *ret;
	GList *child_file_info_list = NULL;
	gboolean cache_incomplete;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	utime_expire = http_util_get_utime() - US_CACHE_DIRECTORY;

	entry = (FileInfoCacheEntry *)g_hash_table_lookup (gl_file_info_cache, uri_string);

	if (entry && (utime_expire > entry->create_time)) {
		entry = NULL;
	}

	if (entry && entry->has_filenames) {
		DEBUG_HTTP (("Cache: Hit: '%s'",entry->uri_string));

		gnome_vfs_file_info_ref (entry->file_info);
		ret = entry->file_info;
	} else {
		ret = NULL;
	}

	if (ret && p_child_file_info_list != NULL) {
		GList * filename_node;

		cache_incomplete = FALSE;
		
		for (filename_node = entry->filenames ;
			filename_node ; 
			filename_node = g_list_next (filename_node) 
		) {
			char *child_filename;
			FileInfoCacheEntry *child_entry;

			child_filename = g_strconcat (uri_string, "/", (gchar *)filename_node->data, NULL);

			child_entry = (FileInfoCacheEntry *)g_hash_table_lookup (gl_file_info_cache, child_filename);

			/* Other HTTP requests on children can cause them to expire before the parent directory */
			if (child_entry == NULL) {
				cache_incomplete = TRUE;
				break;
			}

			gnome_vfs_file_info_ref (child_entry->file_info);
			child_file_info_list = g_list_prepend (child_file_info_list, child_entry->file_info);

			g_free (child_filename);
		}

		if (cache_incomplete) {
			DEBUG_HTTP (("Cache: Directory was incomplete: '%s'",entry->uri_string));

			gnome_vfs_file_info_unref (ret);
			ret = NULL;
			*p_child_file_info_list = NULL;
		} else {
			*p_child_file_info_list = child_file_info_list;
		}
	}

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);

	return ret;
}

GnomeVFSFileInfo *
http_cache_check_directory_uri (GnomeVFSURI * uri, GList **p_child_file_info_list)
{
	gchar *uri_string;
	GnomeVFSFileInfo *ret;

	uri_string = http_cache_uri_to_string (uri);

	ret = http_cache_check_directory (uri_string, p_child_file_info_list);
	g_free (uri_string);

	return ret;
}

/* Note that this neither strdups uri_string nor calls cache_trim() */
static FileInfoCacheEntry *
http_cache_add_no_strdup (gchar * uri_string, GnomeVFSFileInfo * file_info, gboolean is_dav)
{
	FileInfoCacheEntry *entry_existing;
	FileInfoCacheEntry *entry;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	entry_existing = (FileInfoCacheEntry *)g_hash_table_lookup (gl_file_info_cache, uri_string);

	DEBUG_HTTP (("Cache: Add: '%s'", uri_string));

	if (entry_existing) {
		http_cache_entry_free (entry_existing);
		entry_existing = NULL;
	}

	entry = http_cache_entry_new();

	entry->uri_string =  uri_string; 
	entry->file_info = file_info;
	entry->is_dav = is_dav;
	
	gnome_vfs_file_info_ref (file_info);

	g_hash_table_insert (gl_file_info_cache, entry->uri_string, entry);

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);

	return entry;
}

static FileInfoCacheEntry *
http_cache_add (const gchar * uri_string, GnomeVFSFileInfo * file_info, gboolean is_dav)
{
	http_cache_trim ();
	return http_cache_add_no_strdup (g_strdup (uri_string), file_info, is_dav);
}

void
http_cache_add_uri_and_children (GnomeVFSURI *uri, GnomeVFSFileInfo *file_info, GList *file_info_list)
{
	gchar *uri_string;
	gchar *child_string;
	GList *node;
	FileInfoCacheEntry *parent_entry;

	http_cache_trim();

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	uri_string = http_cache_uri_to_string (uri);

	if (uri_string != NULL) {
		/* Note--can't use no_strdup because we use uri_string below */ 
		parent_entry = http_cache_add (uri_string, file_info, TRUE);

		parent_entry->filenames = NULL;

		for (node = file_info_list ; node != NULL ; node = g_list_next (node)) {
			GnomeVFSFileInfo *child_info;
			gchar * child_name_escaped;

			child_info = (GnomeVFSFileInfo *) node->data;

			child_name_escaped = gnome_vfs_escape_path_string (child_info->name);
			
			child_string = g_strconcat (uri_string, "/", child_name_escaped, NULL);

			parent_entry->filenames = g_list_prepend (
							parent_entry->filenames, 
							child_name_escaped); 
			child_name_escaped = NULL;

			http_cache_add_no_strdup (child_string, child_info, TRUE);
		}
		/* I'm not sure that order matters... */
		parent_entry->filenames = g_list_reverse (parent_entry->filenames);
		parent_entry->has_filenames = TRUE;
	}

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);

	g_free (uri_string);
}

void
http_cache_add_uri (GnomeVFSURI *uri, GnomeVFSFileInfo *file_info, gboolean is_dav)
{
	http_cache_trim ();

	http_cache_add_no_strdup (http_cache_uri_to_string (uri), file_info, is_dav);
}


void
http_cache_invalidate (const gchar * uri_string)
{
	FileInfoCacheEntry *entry;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	entry = (FileInfoCacheEntry *)g_hash_table_lookup (gl_file_info_cache, uri_string);

	if (entry) {
		DEBUG_HTTP (("Cache: Invalidate: '%s'", entry->uri_string));

		http_cache_entry_free (entry);
	}

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);
}

void
http_cache_invalidate_uri (GnomeVFSURI *uri)
{
	gchar *uri_string;

	uri_string = http_cache_uri_to_string (uri);

	if (uri_string) {
		http_cache_invalidate (uri_string);
	}

	g_free (uri_string);
}


/* Invalidates entry and everything cached immediately beneath it */
void
http_cache_invalidate_entry_and_children (const gchar * uri_string)
{
	FileInfoCacheEntry *entry;

	gnome_vfs_pthread_recursive_mutex_lock (&cache_rlock);

	entry = (FileInfoCacheEntry *)g_hash_table_lookup (gl_file_info_cache, uri_string);

	if (entry) {
		GList *node;
		
		DEBUG_HTTP (("Cache: Invalidate Recursive: '%s'", entry->uri_string));

		for (node = entry->filenames ; node ; node = g_list_next (node) ) {
			char *child_filename;
			child_filename = g_strconcat (uri_string, "/", (gchar *)node->data, NULL);
			http_cache_invalidate (child_filename);
			g_free (child_filename);
		}
		
		http_cache_entry_free (entry);
	}

	gnome_vfs_pthread_recursive_mutex_unlock (&cache_rlock);
}

/* Invalidates entry and everything cached immediately beneath it */
void
http_cache_invalidate_uri_and_children (GnomeVFSURI *uri)
{
	gchar * uri_string;

	uri_string = http_cache_uri_to_string (uri);

	if (uri_string) {
		http_cache_invalidate_entry_and_children (uri_string);
	}

	g_free (uri_string);
}

/* Invalidate all of this uri's children and all of its parent's children */
void
http_cache_invalidate_uri_parent (GnomeVFSURI *uri)
{
	gchar * uri_string;
	gchar * last_slash;

	uri_string = http_cache_uri_to_string (uri);

	if (uri_string) {
		http_cache_invalidate_entry_and_children (uri_string);

		last_slash = strrchr (uri_string, (unsigned char)'/');
		if (last_slash) {
			*last_slash = 0;
			http_cache_invalidate_entry_and_children (uri_string);
		}
	}

	g_free (uri_string);
}
