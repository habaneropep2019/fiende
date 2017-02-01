/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* Test-method.c: Gnome-VFS testing method

   Copyright (C) 2000 Eazel

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

   Authors: Seth Nickell      (seth@eazel.com) */

/* Create a settings file and put it at PREFIX/etc/vfs/Test-conf.xml,
 * then point gnome-vfs clients to test:<uri_minus_scheme> which will
 * translate into the "real" method (e.g. test:///home/seth).
 *
 * The delay is in milliseconds.
 *
 * Here's a sample config file (pointing to the file method):
 *
 *    <?xml version="1.0"?>
 *        <TestModule method="file">
 *	      <function name="open_directory" delay="2000"/>
 *        </TestModule>
 * */

#include <config.h>

#if GNOME_PLATFORM_VERSION < 1095000
#include <gnome-xml/parser.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/xmlmemory.h>
#else
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#endif

#if GNOME_PLATFORM_VERSION < 1095000
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#endif
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-private.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	char *operation_name;
        int delay;
	gboolean skip;
	gboolean override_result;
	GnomeVFSResult overridden_result_value;
} OperationSettings;

#define NUM_RESULT_STRINGS 41

static gboolean properly_initialized;

static char *test_method_name;
static GList *settings_list;

static const char * const
result_strings[NUM_RESULT_STRINGS] = {
	"GNOME_VFS_OK",
	"GNOME_VFS_ERROR_NOT_FOUND",
	"GNOME_VFS_ERROR_GENERIC",
	"GNOME_VFS_ERROR_INTERNAL",
	"GNOME_VFS_ERROR_BAD_PARAMETERS",
	"GNOME_VFS_ERROR_NOT_SUPPORTED",
	"GNOME_VFS_ERROR_IO",
	"GNOME_VFS_ERROR_CORRUPTED_DATA",
	"GNOME_VFS_ERROR_WRONG_FORMAT",
	"GNOME_VFS_ERROR_BAD_FILE",
	"GNOME_VFS_ERROR_TOO_BIG",
	"GNOME_VFS_ERROR_NO_SPACE",
	"GNOME_VFS_ERROR_READ_ONLY",
	"GNOME_VFS_ERROR_INVALID_URI",
	"GNOME_VFS_ERROR_NOT_OPEN",
	"GNOME_VFS_ERROR_INVALID_OPEN_MODE",
	"GNOME_VFS_ERROR_ACCESS_DENIED",
	"GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES",
	"GNOME_VFS_ERROR_EOF",
	"GNOME_VFS_ERROR_NOT_A_DIRECTORY",
	"GNOME_VFS_ERROR_IN_PROGRESS",
	"GNOME_VFS_ERROR_INTERRUPTED",
	"GNOME_VFS_ERROR_FILE_EXISTS",
	"GNOME_VFS_ERROR_LOOP",
	"GNOME_VFS_ERROR_NOT_PERMITTED",
	"GNOME_VFS_ERROR_IS_DIRECTORY",
	"GNOME_VFS_ERROR_NO_MEMORY",
	"GNOME_VFS_ERROR_HOST_NOT_FOUND",
	"GNOME_VFS_ERROR_INVALID_HOST_NAME",
	"GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS",
	"GNOME_VFS_ERROR_LOGIN_FAILED",
	"GNOME_VFS_ERROR_CANCELLED",
	"GNOME_VFS_ERROR_DIRECTORY_BUSY",
	"GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY",
	"GNOME_VFS_ERROR_TOO_MANY_LINKS",
	"GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM",
	"GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM",
	"GNOME_VFS_ERROR_NAME_TOO_LONG",
	"GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE",
	"GNOME_VFS_NUM_ERRORS"
};

/* Module entry points. */
GnomeVFSMethod *vfs_module_init     (const char     *method_name,
				        const char     *args);
void            vfs_module_shutdown  (GnomeVFSMethod *method);

static GnomeVFSURI *
translate_uri (GnomeVFSURI *uri)
{
	GnomeVFSURI *translated_uri;
	char *uri_text;
	char *translated_uri_text;
	char *no_method;

	uri_text = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	no_method = strchr (uri_text, ':');
	
	if (test_method_name != NULL) {
	  translated_uri_text = g_strconcat (test_method_name, 
					     no_method, NULL);
	} else {
	  translated_uri_text = NULL;
	}

	if (translated_uri_text != NULL) {
	  translated_uri = gnome_vfs_uri_new (translated_uri_text);
	} else {
	  translated_uri = NULL;
	}

	g_free (translated_uri_text);
	g_free (uri_text);

	return translated_uri;
}

/* reads the configuration file and returns TRUE if there are special options for
   this execution of the operation.

   if TRUE is returned then result will contain the result the operation should return
   and perform_operation will be TRUE if the operation should execute the underlying
   operation anyway
*/
static const OperationSettings *
get_operation_settings (const char *function_identifier)
{
	static OperationSettings empty_settings;
        GList *node;
	OperationSettings *settings;

	for (node = settings_list; node != NULL; node = node->next) {
	        settings = node->data;
		if (g_strcasecmp (settings->operation_name, function_identifier) == 0) {
			return settings;
		}
	}
	
	return &empty_settings;
}

static const OperationSettings *
start_operation (const char *name,
		 GnomeVFSURI **uri,
		 GnomeVFSURI **saved_uri)
{
	const OperationSettings *settings;

	settings = get_operation_settings (name);
	usleep (settings->delay * 1000);
	
	if (uri != NULL) {
		*saved_uri = *uri;
		*uri = translate_uri (*uri);
	}
	return settings;
}

static GnomeVFSResult
finish_operation (const OperationSettings *settings,
		  GnomeVFSResult result,
		  GnomeVFSURI **uri,
		  GnomeVFSURI **saved_uri)
{
	if (uri != NULL) {
		gnome_vfs_uri_unref (*uri);
		*uri = *saved_uri;
	}

	if (settings->override_result) {
		return settings->overridden_result_value;
	}
	return result;
}

#define PERFORM_OPERATION(name, operation)			\
do {								\
	const OperationSettings *settings;			\
	GnomeVFSURI *saved_uri;					\
	GnomeVFSResult result;					\
								\
        if (!properly_initialized) {                            \
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;   \
        }                                                       \
                                                                \
	settings = start_operation (#name, &uri, &saved_uri);	\
	if (settings->skip) {					\
		result = GNOME_VFS_OK;				\
	} else {						\
		result = operation;				\
	}							\
	return finish_operation (settings, result,		\
				 &uri, &saved_uri);		\
} while (0)


#define PERFORM_OPERATION_NO_URI(name, operation)		\
do {								\
	const OperationSettings *settings;			\
	GnomeVFSResult result;					\
								\
        if (!properly_initialized) {                            \
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;   \
        }                                                       \
                                                                \
	settings = start_operation (#name, NULL, NULL);		\
	if (settings->skip) {					\
		result = GNOME_VFS_OK;				\
	} else {						\
		result = operation;				\
	}							\
	return finish_operation (settings, result,		\
				 NULL, NULL);			\
} while (0)

static gboolean
parse_result_text (const char *result_text,
		   GnomeVFSResult *result_code)
{
	int i;

	for (i = 0; i < NUM_RESULT_STRINGS; i++) {
		if (g_strcasecmp (result_text, result_strings[i]) == 0) {
			*result_code = i;
			return TRUE;
		}
	}
	
	return FALSE;
}

static gboolean
load_settings (const char *filename) 
{
	xmlDocPtr doc;
	xmlNodePtr node;
	char *name;
	OperationSettings *operation;
	char *str;

	doc = xmlParseFile (filename); 

	if (doc == NULL
	    || doc->xmlRootNode == NULL
	    || doc->xmlRootNode->name == NULL
	    || g_strcasecmp (doc->xmlRootNode->name, "testmodule") != 0) {
		xmlFreeDoc(doc);
		return FALSE;
	}

	test_method_name = xmlGetProp (doc->xmlRootNode, "method");
	
	for (node = doc->xmlRootNode->xmlChildrenNode; node != NULL; node = node->next) {
		name = xmlGetProp (node, "name");
		if (name == NULL) {
			continue;
		}

		operation = g_new0 (OperationSettings, 1);
		operation->operation_name = name;

		str = xmlGetProp (node, "delay");
		if (str != NULL) {
			sscanf (str, "%d", &operation->delay);
		}
		xmlFree (str);

		str = xmlGetProp(node, "execute_operation");
		if (str != NULL && g_strcasecmp (str, "FALSE") == 0) {
			operation->skip = TRUE;
		}
		xmlFree (str);

		str = xmlGetProp (node, "result");
		if (str != NULL) {
			operation->override_result = parse_result_text
				(str, &operation->overridden_result_value);
		}
		xmlFree (str);
		
		settings_list = g_list_prepend (settings_list, operation);
	}
	return TRUE;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{	
	PERFORM_OPERATION (open, gnome_vfs_open_uri_cancellable ((GnomeVFSHandle **) method_handle, uri, mode, context));
}

static GnomeVFSResult
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	/* FIXME bugzilla.eazel.com 3837: Not implemented. */
	return GNOME_VFS_ERROR_INTERNAL;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (close, gnome_vfs_close_cancellable ((GnomeVFSHandle *) method_handle, context));
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (read, gnome_vfs_read_cancellable ((GnomeVFSHandle *) method_handle, buffer, num_bytes, bytes_read, context));
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (write, gnome_vfs_write_cancellable((GnomeVFSHandle *) method_handle, buffer, num_bytes, bytes_written, context));
}

static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition whence,
	 GnomeVFSFileOffset offset,
	 GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (seek, gnome_vfs_seek_cancellable ((GnomeVFSHandle *) method_handle, whence, offset, context));
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset *offset_return)
{
	PERFORM_OPERATION_NO_URI (tell, gnome_vfs_tell ((GnomeVFSHandle *) method_handle, offset_return));
}


static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)
{
	PERFORM_OPERATION (open_directory, gnome_vfs_directory_open_from_uri ((GnomeVFSDirectoryHandle **) method_handle, uri, options, filter));
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (close_directory, gnome_vfs_directory_close ((GnomeVFSDirectoryHandle *) method_handle));
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (read_directory, gnome_vfs_directory_read_next ((GnomeVFSDirectoryHandle *) method_handle, file_info));
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
	PERFORM_OPERATION (get_file_info, gnome_vfs_get_file_info_uri_cancellable (uri, file_info, options, context));
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (get_file_info_from_handle, gnome_vfs_get_file_info_from_handle_cancellable ((GnomeVFSHandle *) method_handle, file_info, options, context));
}

static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)
{
	/* FIXME bugzilla.eazel.com 3837: Not implemented. */
	return TRUE;
}

static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   guint perm,
		   GnomeVFSContext *context)
{
	PERFORM_OPERATION (make_directory, gnome_vfs_make_directory_for_uri_cancellable (uri, perm, context));
}

static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	PERFORM_OPERATION (remove_directory, gnome_vfs_remove_directory_from_uri_cancellable (uri, context));
}

static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	/* FIXME bugzilla.eazel.com 3837: Not implemented. */
	return gnome_vfs_move_uri_cancellable (old_uri, new_uri, force_replace, context);
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method,
	   GnomeVFSURI *uri,
	   GnomeVFSContext *context)
{
	PERFORM_OPERATION (unlink, gnome_vfs_unlink_from_uri_cancellable (uri, context));
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *a,
		  GnomeVFSURI *b,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	/* FIXME bugzilla.eazel.com 3837: Not implemented. */
	return gnome_vfs_check_same_fs_uris_cancellable (a, b, same_fs_return, context);
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{	
	PERFORM_OPERATION (set_file_info, gnome_vfs_set_file_info_cancellable (uri, info, mask, context));
}

static GnomeVFSResult
do_truncate (GnomeVFSMethod *method,
	     GnomeVFSURI *uri,
	     GnomeVFSFileSize where,
	     GnomeVFSContext *context)
{
	PERFORM_OPERATION (truncate, gnome_vfs_truncate_uri_cancellable (uri, where, context));
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)
{
	PERFORM_OPERATION_NO_URI (truncate_handle, gnome_vfs_truncate_handle_cancellable ((GnomeVFSHandle *) method_handle, where, context));
}

static GnomeVFSResult
do_find_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   GnomeVFSFindDirectoryKind kind,
		   GnomeVFSURI **result_uri,
		   gboolean create_if_needed,
		   gboolean find_if_needed,
		   guint permissions,
		   GnomeVFSContext *context)
{
	PERFORM_OPERATION (find_directory, gnome_vfs_find_directory_cancellable (uri, kind, result_uri, create_if_needed, find_if_needed, permissions, context));
}

static GnomeVFSResult
do_create_symbolic_link (GnomeVFSMethod *method,
			 GnomeVFSURI *uri,
			 const char *target_reference,
			 GnomeVFSContext *context)
{
	PERFORM_OPERATION (create_symbolic_link, gnome_vfs_create_symbolic_link_cancellable (uri, target_reference, context));
}

static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	do_seek,
	do_tell,
	do_truncate_handle,
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	do_make_directory,
	do_remove_directory,
	do_move,
	do_unlink,
	do_check_same_fs,
        do_set_file_info,
	do_truncate,
	do_find_directory,
	do_create_symbolic_link
};


GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
	LIBXML_TEST_VERSION
	
	if (load_settings (PREFIX "/etc/vfs/Test-conf.xml") == FALSE) {

	  // FIXME: we probably shouldn't use printf to output the message
	  printf (_("Didn't find a valid settings file at %s\n"), 
		  PREFIX "/etc/vfs/Test-conf.xml");
	  properly_initialized = FALSE;
	} else {
	  properly_initialized = TRUE;
	}

	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	GList *node;
	OperationSettings *settings;

	for (node = settings_list; node != NULL; node = node->next) {
	        settings = node->data;
		xmlFree (settings->operation_name);
		g_free (settings);
	}
	g_list_free (settings_list);
	xmlFree (test_method_name);
}
