/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Copyright (C) 1998 Miguel de Icaza
 * Copyright (C) 1997 Paolo Molaro
 * Copyright (C) 2000, 2001 Eazel, Inc.
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "gnome-vfs-mime.h"

#include "gnome-vfs-mime-private.h"
#include "gnome-vfs-mime-sniff-buffer-private.h"
#include "gnome-vfs-module-shared.h"
#include "gnome-vfs-ops.h"
#include "gnome-vfs-result.h"
#include "gnome-vfs-uri.h"
#include <ctype.h>
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static gboolean module_inited = FALSE;

static GHashTable *mime_extensions [2] = { NULL, NULL };
static GList      *mime_regexs     [2] = { NULL, NULL };

#define DEFAULT_DATE_TRACKER_INTERVAL	5	/* in milliseconds */

typedef struct {
	char *mime_type;
	regex_t regex;
} RegexMimePair;

typedef struct {
	char *dirname;
	unsigned int valid : 1;
	unsigned int system_dir : 1;
} mime_dir_source_t;

typedef struct {
	char *file_path;
	time_t mtime;
} FileDateRecord;

struct FileDateTracker {
	time_t last_checked;
	guint check_interval;
	GHashTable *records;
};

/* These ones are used to automatically reload mime-types on demand */
static mime_dir_source_t gnome_mime_dir, user_mime_dir;
static FileDateTracker *mime_data_date_tracker;

#ifdef G_THREADS_ENABLED

/* We lock this mutex whenever we modify global state in this module.  */
G_LOCK_DEFINE_STATIC (mime_mutex);

#endif /* G_LOCK_DEFINE_STATIC */


static char *
get_priority (char *def, int *priority)
{
	*priority = 0;

	if (*def == ',') {
		def++;
		if (*def == '1') {
			*priority = 0;
			def++;
		} else if (*def == '2') {
			*priority = 1;
			def++;
		}
	}

	while (*def && *def == ':')
		def++;

	return def;
}

static int
list_find_type (gconstpointer value, gconstpointer type)
{
	return g_strcasecmp((const char *) value, (const char *) type);
}

static void
add_to_key (char *mime_type, char *def)
{
	int priority = 1;
	char *s, *p, *ext;
	GList *list = NULL;

	if (strncmp (def, "ext", 3) == 0){
		char *tokp;

		def += 3;
		def = get_priority (def, &priority);
		s = p = g_strdup (def);

		while ((ext = strtok_r (s, " \t\n\r,", &tokp)) != NULL) {
			list = (GList *) g_hash_table_lookup (mime_extensions [priority], ext);
			if (!g_list_find_custom (list, mime_type, list_find_type)) {
				list = g_list_prepend (list, g_strdup (mime_type));
				g_hash_table_insert (mime_extensions [priority], g_strdup (ext), list);
			}
			s = NULL;
		}
		g_free (p);
	}

	if (strncmp (def, "regex", 5) == 0) {
		RegexMimePair *mp;
		def += 5;
		def = get_priority (def, &priority);

		while (*def != '\0' && isspace ((unsigned char)*def)) {
			def++;
		}

		if (*def == '\0') {
			return;
		}

		/* This was g_new instead of g_new0, but there seems
		 * to be a bug in the Solaris? version of regcomp that
		 * requires an initialized regex or it will crash.
		 */
		mp = g_new0 (RegexMimePair, 1);
		if (regcomp (&mp->regex, def, REG_EXTENDED | REG_NOSUB)) {
			g_free (mp);
			return;
		}
		mp->mime_type = g_strdup (mime_type);

		mime_regexs [priority] = g_list_prepend (mime_regexs [priority], mp);
	}
}

static void
mime_fill_from_file (const char *filename)
{
	FILE *file;
	char buf [1024];
	char *current_key;

	g_assert (filename != NULL);

	file = fopen (filename, "r");

	if (file == NULL) {
		return;
	}
	
	current_key = NULL;
	while (fgets (buf, sizeof (buf), file) != NULL) {
		char *p;

		if (buf [0] == '#') {
			continue;
		}

		/* Trim trailing spaces */
		for (p = buf + strlen (buf) - 1; p >= buf; p--) {
			if (!isspace ((unsigned char)*p) && *p != '\n') {
				break;
			}
			*p = 0;
		}

		if (buf [0] == '\0') {
			continue;
		}

		if (buf [0] == '\t' || buf [0] == ' '){
			if (current_key){
				char *p = buf;

				while (*p && isspace ((unsigned char)*p))
					p++;

				if (*p == 0)
					continue;

				add_to_key (current_key, p);
			}
		} else {
			g_free (current_key);

			current_key = g_strdup (buf);
			if (current_key [strlen (current_key)-1] == ':')
				current_key [strlen (current_key)-1] = 0;
		}
	}

	g_free (current_key);

	fclose (file);

	gnome_vfs_file_date_tracker_start_tracking_file (mime_data_date_tracker, filename);
}

static void
mime_load (mime_dir_source_t *source)
{
	DIR *dir;
	struct dirent *dent;
	const int extlen = sizeof (".mime") - 1;
	char *filename;
	struct stat s;

	g_return_if_fail (source != NULL);
	g_return_if_fail (source->dirname != NULL);

	source->valid = (stat (source->dirname, &s) != -1);

	dir = opendir (source->dirname);
	if (dir == NULL) {
		source->valid = FALSE;
		return;
	}

	if (source->system_dir) {
		filename = g_strconcat (source->dirname, "/gnome-vfs.mime", NULL);
		mime_fill_from_file (filename);
		g_free (filename);
	}

	while (TRUE) {
		int len;
		
		dent = readdir (dir);
		if (dent == NULL) {
			break;
		}
		
		len = strlen (dent->d_name);

		if (len <= extlen) {
			continue;
		}
		
		if (strcmp (dent->d_name + len - extlen, ".mime") != 0) {
			continue;
		}

		if (source->system_dir && strcmp (dent->d_name, "gnome-vfs.mime") == 0) {
			continue;
		}

		if (source->system_dir && strcmp (dent->d_name, "gnome.mime") == 0) {
			/* Ignore the obsolete "official" one so it doesn't override
			 * the new official one.
			 */
			continue;
		}

		if (!source->system_dir && strcmp (dent->d_name, "user.mime") == 0) {
			continue;
		}

		filename = g_strconcat (source->dirname, "/", dent->d_name, NULL);

		mime_fill_from_file (filename);
		g_free (filename);
	}
	closedir (dir);

	if (!source->system_dir) {
		filename = g_strconcat (source->dirname, "/user.mime", NULL);
		mime_fill_from_file (filename);
		g_free (filename);
	}

	gnome_vfs_file_date_tracker_start_tracking_file (mime_data_date_tracker, source->dirname);
}

static gboolean
remove_one_mime_hash_entry (gpointer key, gpointer value, gpointer user_data)
{
	g_free (key);
	g_list_foreach (value, (GFunc) g_free, NULL);
	g_list_free (value);

	return TRUE;
}

static void
mime_extensions_empty (void)
{
	GList *p;
	int i;
	for (i = 0; i < 2; i++) {
		if (mime_extensions [i] != NULL) {
			g_hash_table_foreach_remove (mime_extensions [i], 
						     remove_one_mime_hash_entry, NULL);
		}

		for (p = mime_regexs [i]; p != NULL; p = p->next){
			RegexMimePair *mp = p->data;

			g_free (mp->mime_type);
			regfree (&mp->regex);
			g_free (mp);
		}
		g_list_free (mime_regexs [i]);
		mime_regexs [i] = NULL;
	}
}

static void
maybe_reload (void)
{
	if (!gnome_vfs_file_date_tracker_date_has_changed (mime_data_date_tracker)) {
		return;
	}

	mime_extensions_empty ();

	mime_load (&gnome_mime_dir);
	mime_load (&user_mime_dir);
}

static void
mime_init (void)
{
	mime_extensions [0] = g_hash_table_new (g_str_hash, g_str_equal);
	mime_extensions [1] = g_hash_table_new (g_str_hash, g_str_equal);

	mime_data_date_tracker = gnome_vfs_file_date_tracker_new ();
	
	gnome_mime_dir.dirname = g_strconcat (GNOME_VFS_DATADIR, "/mime-info", NULL);
	gnome_mime_dir.system_dir = TRUE;

	user_mime_dir.dirname = g_strconcat (g_get_home_dir (), "/.gnome/mime-info", NULL);
	user_mime_dir.system_dir = FALSE;

	mime_load (&gnome_mime_dir);
	mime_load (&user_mime_dir);

	module_inited = TRUE;
}

void
gnome_vfs_mime_shutdown (void)
{
	if (!module_inited)
		return;

	gnome_vfs_mime_info_shutdown ();
	gnome_vfs_mime_clear_magic_table ();

	mime_extensions_empty ();
	
	g_hash_table_destroy (mime_extensions[0]);
	g_hash_table_destroy (mime_extensions[1]);

	gnome_vfs_file_date_tracker_free (mime_data_date_tracker);
	
	g_free (gnome_mime_dir.dirname);
	g_free (user_mime_dir.dirname);
}

/**
 * gnome_vfs_mime_type_from_name_or_default:
 * @filename: A filename (the file does not necesarily exist).
 * @defaultv: A default value to be returned if no match is found
 *
 * This routine tries to determine the mime-type of the filename
 * only by looking at the filename from the GNOME database of mime-types.
 *
 * Returns the mime-type of the @filename.  If no value could be
 * determined, it will return @defaultv.
 */
const char *
gnome_vfs_mime_type_from_name_or_default (const char *filename, const char *defaultv)
{
	const gchar *ext;
	char *upext;
	int priority;
	const char *result = defaultv;

	if (filename == NULL) {
		return result;
	}

	G_LOCK (mime_mutex);

	ext = strrchr (filename, '.');
	if (ext != NULL) {
		++ext;
	}
	
	if (!module_inited) {
		mime_init ();
	}

	maybe_reload ();

	for (priority = 1; priority >= 0; priority--){
		GList *l;
		GList *list = NULL ;
		
		if (ext != NULL) {
			
			list = g_hash_table_lookup (mime_extensions [priority], ext);
			if (list != NULL) {
				list = g_list_first( list );
				result = (const char *) list->data;
				break;
			}

			/* Search for UPPER case extension */
			upext = g_strdup (ext);
			g_strup (upext);
			list = g_hash_table_lookup (mime_extensions [priority], upext);
			if (list != NULL) {
				g_free (upext);
				list = g_list_first (list);
				result = (const char *) list->data;
				break;
			}

			/* Final check for lower case */
			g_strdown (upext);
			list = g_hash_table_lookup (mime_extensions [priority], upext);
 			g_free (upext);
			if (list != NULL) {
				list = g_list_first (list);
				result = (const char *) list->data;
				break;
			}
		}

		for (l = mime_regexs [priority]; l; l = l->next){
			RegexMimePair *mp = l->data;

			if (regexec (&mp->regex, filename, 0, 0, 0) == 0) {
				result = mp->mime_type;
				G_UNLOCK (mime_mutex);
				return result;
			}
		}
	}

	G_UNLOCK (mime_mutex);
	return result;
}

/**
 * gnome_vfs_mime_type_from_name:
 * @filename: A filename (the file does not necessarily exist).
 *
 * Determined the mime type for @filename.
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_mime_type_from_name (const gchar * filename)
{
	return gnome_vfs_mime_type_from_name_or_default (filename, GNOME_VFS_MIME_TYPE_UNKNOWN);
}

static const char *
gnome_vfs_get_mime_type_from_uri_internal (GnomeVFSURI *uri)
{
	const char *base_name;

	/* Return a mime type based on the file extension or NULL if no match. */
	base_name = gnome_vfs_uri_get_basename (uri);
	if (base_name == NULL) {
		return NULL;
	}

	return gnome_vfs_mime_type_from_name_or_default (base_name, NULL);
}

const char *
gnome_vfs_get_mime_type_internal (GnomeVFSMimeSniffBuffer *buffer, const char *file_name)
{
	const char *result;

	result = NULL;
	
	if (buffer != NULL) {
		result = gnome_vfs_mime_get_type_from_magic_table (buffer);
		
		if (result != NULL) {
			return result;
		}
		
		/* Special handling of gzip files -- use the file name to make a more
		 * accurate guess of the file type for formats such as gnumeric.gz and
		 * pdf.gz. Without this, these would always get identified as gzip even though
		 * their name would suggest otherwise.
		 * FIXME bugzilla.eazel.com 6867:
		 * Generalize this so that we can have different magic patters
		 * other than gzip do this.
		 */
		if (gnome_vfs_sniff_buffer_looks_like_gzip (buffer, file_name)) {
			return "application/x-gzip";
		}
		
		if (result == NULL) {
			if (gnome_vfs_sniff_buffer_looks_like_text (buffer)) {
				/* Text file -- treat extensions as a more accurate source
				 * of type information.
				 */
				
				if (file_name != NULL) {
					result = gnome_vfs_mime_type_from_name_or_default (file_name, NULL);
				}
	
				if (result != NULL) {
					return result;
				}

				/* Didn't find an extension match, assume plain text. */
				return "text/plain";

			} else if (gnome_vfs_sniff_buffer_looks_like_mp3 (buffer)) {
				return "audio/x-mp3";
			}
		}
	}
	
	if (result == NULL && file_name != NULL) {
		/* No type recognized -- fall back on extensions. */
		result = gnome_vfs_mime_type_from_name_or_default (file_name, NULL);
	}
	
	if (result == NULL) {
		result = GNOME_VFS_MIME_TYPE_UNKNOWN;
	}
	
	return result;
}

/**
 * gnome_vfs_get_mime_type:
 * @uri: a real file or a non-existent uri.
 * @data_size: Size of the data.
 *
 * Tries to guess the mime type of the file represented by @uir.
 * Favors using the file data to the @uri extension.
 * Handles passing @uri of a non-existent file by falling back
 * on returning a type based on the extension.
 *
 * Returns the mime-type for this uri.
 * FIXME: This function will not necessarily return the same mime type as doing a
 * get file info on the text uri.
 * 
 */
const char *
gnome_vfs_get_mime_type (GnomeVFSURI *uri)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;
	GnomeVFSHandle *handle;
	GnomeVFSResult error;

	/* Check for special stat-defined file types first. */
	result = gnome_vfs_get_special_mime_type (uri);
	if (result != NULL) {
		return result;
	}

	error = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ);

	if (error != GNOME_VFS_OK) {
		/* file may not exist, return type based on name only */
		return gnome_vfs_get_mime_type_from_uri_internal (uri);
	}
	
	buffer = gnome_vfs_mime_sniff_buffer_new_from_handle (handle);

	result = gnome_vfs_get_mime_type_internal (buffer, gnome_vfs_uri_get_basename (uri));

	gnome_vfs_mime_sniff_buffer_free (buffer);
	gnome_vfs_close (handle);

	return result;
}

static GnomeVFSResult
file_seek_binder (gpointer context, GnomeVFSSeekPosition whence, 
		  GnomeVFSFileOffset offset)
{
	FILE *file = (FILE *)context;
	int result;
	result = fseek (file, offset, whence);
	if (result < 0) {
		return gnome_vfs_result_from_errno ();
	}
	return GNOME_VFS_OK;
}

static GnomeVFSResult
file_read_binder (gpointer context, gpointer buffer, 
		  GnomeVFSFileSize bytes, GnomeVFSFileSize *bytes_read)
{
	FILE *file = (FILE *)context;	
	*bytes_read = fread (buffer, 1, bytes, file);
	if (*bytes_read < 0) {
		*bytes_read = 0;
		return gnome_vfs_result_from_errno ();
	}

	return GNOME_VFS_OK;
}

/**
 * gnome_vfs_get_file_mime_type:
 * @path: a path of a file.
 * @stat_info: optional stat buffer.
 * @suffix_only: whether or not to do a magic-based lookup.
 *
 * Tries to guess the mime type of the file represented by @path.
 * If @suffix_only is false, uses the mime-magic based lookup first.
 * Handles passing @path of a non-existent file by falling back
 * on returning a type based on the extension.
 *
 * Returns the mime-type for this path.
 */
const char *
gnome_vfs_get_file_mime_type (const char *path, const struct stat *stat_info,
	gboolean suffix_only)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;
	struct stat tmp_stat_buffer;
	FILE *file;

	file = NULL;
	result = NULL;

	/* get the stat info if needed */
	if (stat_info == NULL && stat (path, &tmp_stat_buffer) == 0) {
		stat_info = &tmp_stat_buffer;
	}

	/* single out special file types */
	if (stat_info && !S_ISREG(stat_info->st_mode)) {
		if (S_ISDIR(stat_info->st_mode)) {
			return "x-directory/normal";
		} else if (S_ISCHR(stat_info->st_mode)) {
			return "x-special/device-char";
		} else if (S_ISBLK(stat_info->st_mode)) {
			return "x-special/device-block";
		} else if (S_ISFIFO(stat_info->st_mode)) {
			return "x-special/fifo";
		} else if (S_ISSOCK(stat_info->st_mode)) {
			return "x-special/socket";
		} else {
			/* unknown entry type, return generic file type */
			return GNOME_VFS_MIME_TYPE_UNKNOWN;
		}
	}

	if (!suffix_only) {
		file = fopen(path, "r");
	}

	if (file != NULL) {
		buffer = gnome_vfs_mime_sniff_buffer_new_generic
			(file_seek_binder, file_read_binder, file);

		result = gnome_vfs_get_mime_type_internal (buffer, path);
		gnome_vfs_mime_sniff_buffer_free (buffer);
		fclose (file);
	} else {
		result = gnome_vfs_get_mime_type_internal (NULL, path);
	}

	
	g_assert (result != NULL);
	return result;
}

/**
 * gnome_vfs_get_mime_type_from_uri:
 * @uri: A file uri.
 *
 * Tries to guess the mime type of the file @uri by
 * checking the file name extension. Works on non-existent
 * files.
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_get_mime_type_from_uri (GnomeVFSURI *uri)
{
	const char *result;

	result = gnome_vfs_get_mime_type_from_uri_internal (uri);
	if (result == NULL) {
		/* no type, return generic file type */
		result = GNOME_VFS_MIME_TYPE_UNKNOWN;
	}

	return result;
}

/**
 * gnome_vfs_get_mime_type_from_file_data:
 * @uri: A file uri.
 *
 * Tries to guess the mime type of the file @uri by
 * checking the file data using the magic patterns. Does not handle text files properly
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_get_mime_type_from_file_data (GnomeVFSURI *uri)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;
	GnomeVFSHandle *handle;
	GnomeVFSResult error;

	error = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ);

	if (error != GNOME_VFS_OK) {
		return GNOME_VFS_MIME_TYPE_UNKNOWN;
	}
	
	buffer = gnome_vfs_mime_sniff_buffer_new_from_handle (handle);
	result = gnome_vfs_get_mime_type_internal (buffer, NULL);	
	gnome_vfs_mime_sniff_buffer_free (buffer);
	gnome_vfs_close (handle);

	return result;
}

/**
 * gnome_vfs_get_mime_type_for_data:
 * @data: A pointer to data in memory.
 * @data_size: Size of the data.
 *
 * Tries to guess the mime type of the data in @data
 * using the magic patterns.
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_get_mime_type_for_data (gconstpointer data, int data_size)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;

	buffer = gnome_vfs_mime_sniff_buffer_new_from_existing_data
		(data, data_size);

	result = gnome_vfs_get_mime_type_internal (buffer, NULL);	

	gnome_vfs_mime_sniff_buffer_free (buffer);

	return result;
}

gboolean
gnome_vfs_mime_type_is_supertype (const char *mime_type)
{
	int length;

	if (mime_type == NULL) {
		return FALSE;
	}

	length = strlen (mime_type);

	return length > 2
	       && mime_type[length - 2] == '/' 
	       && mime_type[length - 1] == '*';
}

static char *
extract_prefix_add_suffix (const char *string, const char *separator, const char *suffix)
{
        const char *separator_position;
        int prefix_length;
        char *result;

        separator_position = strstr (string, separator);
        prefix_length = separator_position == NULL
                ? strlen (string)
                : separator_position - string;

        result = g_malloc (prefix_length + strlen (suffix) + 1);
        
        strncpy (result, string, prefix_length);
        result[prefix_length] = '\0';

        strcat (result, suffix);

        return result;
}

/* Returns the supertype for a mime type. Note that if called
 * on a supertype it will return a copy of the supertype.
 */
char *
gnome_vfs_get_supertype_from_mime_type (const char *mime_type)
{
	if (mime_type == NULL) {
		return NULL;
	}
        return extract_prefix_add_suffix (mime_type, "/", "/*");
}

static void
file_date_record_update_mtime (FileDateRecord *record)
{
	struct stat s;

	if (stat (record->file_path, &s) != -1) {
		record->mtime = s.st_mtime;
	}	
}

static FileDateRecord *
file_date_record_new (const char *file_path) {
	FileDateRecord *record;

	record = g_new0 (FileDateRecord, 1);
	record->file_path = g_strdup (file_path);

	file_date_record_update_mtime (record);

	return record;
}

static void
file_date_record_free (FileDateRecord *record)
{
	g_free (record->file_path);
	g_free (record);
}

FileDateTracker *
gnome_vfs_file_date_tracker_new (void)
{
	FileDateTracker *tracker;

	tracker = g_new0 (FileDateTracker, 1);
	tracker->check_interval = DEFAULT_DATE_TRACKER_INTERVAL;
	tracker->records = g_hash_table_new (g_str_hash, g_str_equal);

	return tracker;
}

static gboolean
release_key_and_value (gpointer key, gpointer value, gpointer user_data)
{
	g_free (key);
	file_date_record_free (value);

	return TRUE;
}

void
gnome_vfs_file_date_tracker_free (FileDateTracker *tracker)
{
	g_hash_table_foreach_remove (tracker->records, release_key_and_value, NULL);
	g_hash_table_destroy (tracker->records);
	g_free (tracker);
}

/*
 * Record the current mod date for a specified file, so that we can check
 * later whether it has changed.
 */
void
gnome_vfs_file_date_tracker_start_tracking_file (FileDateTracker *tracker, 
				                 const char *local_file_path)
{
	FileDateRecord *record;

	record = g_hash_table_lookup (tracker->records, local_file_path);
	if (record != NULL) {
		file_date_record_update_mtime (record);
	} else {
		g_hash_table_insert (tracker->records, 
				     g_strdup (local_file_path), 
				     file_date_record_new (local_file_path));
	}
}

static void 
check_and_update_one (gpointer key, gpointer value, gpointer user_data)
{
	FileDateRecord *record;
	gboolean *return_has_changed;
	struct stat s;

	g_assert (key != NULL);
	g_assert (value != NULL);
	g_assert (user_data != NULL);

	record = (FileDateRecord *)value;
	return_has_changed = (gboolean *)user_data;

	if (stat (record->file_path, &s) != -1) {
		if (s.st_mtime != record->mtime) {
			record->mtime = s.st_mtime;
			*return_has_changed = TRUE;
		}
	}
}

gboolean
gnome_vfs_file_date_tracker_date_has_changed (FileDateTracker *tracker)
{
	time_t now;
	gboolean any_date_changed;

	now = time (NULL);

	/* Note that this might overflow once in a blue moon, but the
	 * only side-effect of that would be a slightly-early check
	 * for changes.
	 */
	if (tracker->last_checked + tracker->check_interval >= now) {
		return FALSE;
	}

	any_date_changed = FALSE;

	g_hash_table_foreach (tracker->records, check_and_update_one, &any_date_changed);

	tracker->last_checked = now;

	return any_date_changed;
}
