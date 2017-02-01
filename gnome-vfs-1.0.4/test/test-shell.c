/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-shell.c - A small program to allow testing of a wide variety of
   gnome-vfs functionality

   Copyright (C) 2000 Free Software Foundation

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

   Author: Michael Meeks <mmeeks@gnu.org> 
   
   NB. This code leaks everywhere, don't loose hair.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include <gnome.h>

#include "gnome-vfs.h"

#define TEST_DEBUG 0

static char delim[]=" ";
static char **arg_data = NULL;
static int    arg_cur  = 0;

static char *cur_dir = NULL;

static GHashTable *files = NULL;

FILE *vfserr = NULL;


static gboolean
show_if_error (GnomeVFSResult result, const char *what, const char *what2)
{
	if (result != GNOME_VFS_OK) {
		fprintf (vfserr, "%s%s `%s'\n",
			 what, what2, gnome_vfs_result_to_string (result));
		return TRUE;
	} else
		return FALSE;
}

static void
register_file (const char *str, GnomeVFSHandle *handle)
{
	if (!str)
		fprintf (vfserr, "Need a valid name");
	else
		g_hash_table_insert (files, g_strdup (str), handle);
}

static GnomeVFSHandle *
lookup_file (const char *str)
{
	GnomeVFSHandle *handle;

	if (!str) {
		fprintf (vfserr, "Invalid handle '%s'\n", str);
		return NULL;
	}

	handle = g_hash_table_lookup (files, str);
	if (!handle)
		fprintf (vfserr, "Can't find handle '%s'\n", str);

	return handle;
}

static void
close_file (const char *str)
{
	GnomeVFSResult result;
	gpointer hash_key, value;

	if (!str)
		fprintf (vfserr, "Can't close NULL handles\n");

	else if (g_hash_table_lookup_extended (files, str, &hash_key, &value)) {
		g_hash_table_remove (files, str);

		result = gnome_vfs_close ((GnomeVFSHandle *)value);
		show_if_error (result, "closing ", (char *)hash_key);

		g_free (hash_key);
	} else

		fprintf (vfserr, "Unknown file handle '%s'\n", str);
}

static gboolean
kill_file_cb (gpointer	key,
	      gpointer	value,
	      gpointer	user_data)
{
	GnomeVFSResult result;

	result = gnome_vfs_close (value);
	show_if_error (result, "closing ", key);
	g_free (key);

	return TRUE;
}

static void
close_files (void)
{
	g_hash_table_foreach_remove (files, kill_file_cb, NULL);
	g_hash_table_destroy (files);
}

static void
do_ls (void)
{
	GnomeVFSResult result;
	GList *list, *node;
	GnomeVFSFileInfo *info;

	result = gnome_vfs_directory_list_load (
		&list, cur_dir, GNOME_VFS_FILE_INFO_DEFAULT,
		NULL);
	if (show_if_error (result, "open directory ", cur_dir))
		return;

	for (node = list; node != NULL; node = node->next) {
		char prechar = '\0', postchar = '\0';
		info = node->data;

		if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE) {
			switch (info->type) {
			case GNOME_VFS_FILE_TYPE_DIRECTORY:
				prechar = '[';
				postchar = ']';
				break;
			case GNOME_VFS_FILE_TYPE_UNKNOWN:
				prechar = '?';
				break;
			case GNOME_VFS_FILE_TYPE_FIFO:
				prechar = '|';
				break;
			case GNOME_VFS_FILE_TYPE_SOCKET:
				prechar = '-';
				break;
			case GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE:
			case GNOME_VFS_FILE_TYPE_BLOCK_DEVICE:
				prechar = '@';
				break;
			case GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK:
				prechar = '#';
				break;
			default:
				prechar = '\0';
				break;
			}
			if (!postchar)
				postchar = prechar;
		}
		printf ("%c%s%c", prechar, info->name, postchar);

		if (strlen (info->name) < 40) {
			int i, pad;

			pad = 40 - strlen (info->name) -
				(prechar?1:0) - (postchar?1:0);

			for (i = 0; i < pad; i++)
				printf (" ");
		}
		if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE) {
			long i = info->size;
			printf (" : %ld bytes", i);
		}
		printf ("\n");
	}
	
	gnome_vfs_file_info_list_free (list);
}

static void
list_commands (void)
{
	printf ("command can be one or all of:\n");
	printf ("Main operations:\n");
	printf (" * ls:                     list files\n");
	printf (" * cd:                     enter storage\n");
	printf (" * mv:                     move object\n");
	printf (" * rm:                     remove stream\n");
	printf (" * mkdir:                  make storage\n");
	printf (" * rmdir:                  remove storage\n");
	printf (" * info,stat:              get information on object\n");
	printf (" * cat,type:               dump text file to console\n");
	printf (" * dump:                   dump binary file to console\n");
	printf (" * sync:                   for sinkers\n");
	printf (" * quit,exit,bye:          exit\n");
	printf ("File operations:\n");
	printf (" * open <handle> <name>:   open a file\n");
	printf (" * create <handle> <name>: create a file\n");
	printf (" * close <handle>:         close a file\n");
	printf (" * read <handle> <bytes>:  read bytes from stream\n");
	printf (" * seek <handle> <pos>:    seek set position\n");
}

static gboolean
simple_regexp (const char *regexp, const char *fname)
{
	int      i, j;
	gboolean ret = TRUE;

	g_return_val_if_fail (fname != NULL, FALSE);
	g_return_val_if_fail (regexp != NULL, FALSE);

	for (i = j = 0; regexp [i] && fname [j]; j++, i++) {
		if (regexp [i] == '\\' &&
		    !(i > 0 && regexp [i - 1] == '\\')) {
			j--;
			continue;
		}

		if (regexp [i] == '.' &&
		    !(i > 0 && regexp [i - 1] == '\\'))
			continue;

		if (toupper (regexp [i]) != toupper (fname [j])) {
			ret = FALSE;
			break;
		}
	}

	if (regexp [i] && regexp [i] == '*')
		ret = TRUE;

	else if (!regexp [i] && fname [j])
		ret = FALSE;
	
	else if (!fname [j] && regexp [i])
		ret = FALSE;

/*	if (ret)
	printf ("'%s' matched '%s'\n", regexp, fname);*/

	return ret;
}

static gboolean
validate_path (const char *path)
{
	GnomeVFSResult result;
	GList *list;

	result = gnome_vfs_directory_list_load (
		&list, path, GNOME_VFS_FILE_INFO_DEFAULT,
		NULL);

	if (show_if_error (result, "open directory ", path)) {
		return FALSE;
	}

	gnome_vfs_file_info_list_free (list);

	return TRUE;
}

static char *
get_regexp_name (const char *regexp, const char *path, gboolean dir)
{
	GnomeVFSResult result;
	GList *list, *node;
	GnomeVFSFileInfo *info;
	char *res = NULL;

	result = gnome_vfs_directory_list_load (
		&list, path, GNOME_VFS_FILE_INFO_DEFAULT,
		NULL);

	if (show_if_error (result, "open directory ", path))
		return NULL;

	for (node = list; node != NULL; node = node->next) {
		info = node->data;

		if (simple_regexp (regexp, info->name)) {
			if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE) {
				if ((dir  && info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) ||
				    (!dir && info->type != GNOME_VFS_FILE_TYPE_DIRECTORY)) {
					res = g_strdup (info->name);
					break;
				}
			} else {
				fprintf (vfserr, "Can't cope with no type data");
				res = g_strdup (info->name);
				break;
			}
		}
	}
	gnome_vfs_file_info_list_free (list);

	return res;
}

static void
do_cd (void)
{
	char *p;

	p = arg_data [arg_cur++];

	if (!p) {
		fprintf (vfserr, "Takes a directory argument\n");
		return;
	}

	if (!g_strcasecmp (p, "..")) {
		guint lp;
		char **tmp;
		GString *newp = g_string_new ("");

		tmp = g_strsplit (cur_dir, "/", -1);
		lp  = 0;
		if (!tmp [lp])
			return;

		while (tmp [lp + 1]) {
			g_string_sprintfa (newp, "%s/", tmp [lp]);
			lp++;
		}
		g_free (cur_dir);
		cur_dir = newp->str;
		g_string_free (newp, FALSE);
	} else if (!g_strcasecmp (p, ".")) {
	} else {
		char *newpath;

		if (strchr (p, ':') ||
		    p [0] == '/') {
			if (p [strlen (p) - 1] != '/')
				newpath = g_strconcat (p, "/", NULL);
			else
				newpath = g_strdup (p);
		} else {
			char *ptr;
			
			ptr = get_regexp_name (p, cur_dir, TRUE);
			if (!ptr) {
				fprintf (vfserr, "Can't find '%s'\n", p);
				return;
			}

			newpath = g_strconcat (cur_dir, ptr, "/", NULL);
		}

		if (validate_path (newpath)) {
			g_free (cur_dir);
			cur_dir = newpath;
		} else
			fprintf (vfserr, "Invalid path %s\n", newpath);
	}
}

static char *
get_fname (void)
{
	char *fname, *reg_name, *f;

	if (!arg_data [arg_cur])
		return NULL;
	
	reg_name = arg_data [arg_cur++];
	fname = get_regexp_name (reg_name, cur_dir, FALSE);

	if (!fname)
		fname = reg_name;
	
	if (strchr (fname, ':') ||
	    fname [0] == '/')
		f = g_strdup (fname);
	else if (cur_dir)
		f = g_strconcat (cur_dir, fname, NULL);
	else
		f = g_strdup (fname);

	return f;
}

static void
do_cat (void)
{
	char *from;
	GnomeVFSHandle *from_handle;
	GnomeVFSResult  result;

	from = get_fname ();

	result = gnome_vfs_open (&from_handle, from, GNOME_VFS_OPEN_READ);
	if (show_if_error (result, "open ", from))
		return;

	while (1) {
		GnomeVFSFileSize bytes_read;
		guint8           data [1025];
		
		result = gnome_vfs_read (from_handle, data, 1024, &bytes_read);
		if (show_if_error (result, "read ", from))
			return;

		if (bytes_read == 0)
			break;
		
		if (bytes_read >  0 &&
		    bytes_read <= 1024)
			data [bytes_read] = '\0';
		else {
			data [1024] = '\0';
			g_warning ("Wierd error from vfs_read");
		}
		fprintf (stdout, "%s", data);
	}

	result = gnome_vfs_close (from_handle);
	if (show_if_error (result, "close ", from))
		return;
	fprintf (stdout, "\n");
}

static void
do_rm (void)
{
	char          *fname;
	GnomeVFSResult result;

	fname = get_fname ();

	result = gnome_vfs_unlink (fname);
	if (show_if_error (result, "unlink ", fname))
		return;	
}

static void
do_mkdir (void)
{
	char          *fname;
	GnomeVFSResult result;

	fname = get_fname ();

	result = gnome_vfs_make_directory (fname, GNOME_VFS_PERM_USER_ALL);
	if (show_if_error (result, "mkdir ", fname))
		return;	
}

static void
do_rmdir (void)
{
	char          *fname;
	GnomeVFSResult result;

	fname = get_fname ();

	result = gnome_vfs_remove_directory (fname);
	if (show_if_error (result, "rmdir ", fname))
		return;	
}

static void
do_mv (void)
{
	char          *from, *to;
	char          *msg;
	GnomeVFSResult result;

	from = get_fname ();
	to = get_fname ();
	if (!from || !to) {
		fprintf (vfserr, "mv <from> <to>\n");
		return;
	}

	result = gnome_vfs_move (from, to, FALSE);

	msg = g_strdup_printf ("%s to %s", from, to);
	show_if_error (result, "move ", msg);
	g_free (msg);
}

static void
do_findtrash (void)
{
	char *from;
	char *uri_as_string;
	GnomeVFSResult    result;
	GnomeVFSURI *from_uri;
	GnomeVFSURI *result_vfs_uri;

	from = get_fname ();

	from_uri = gnome_vfs_uri_new (from);
	result = gnome_vfs_find_directory (from_uri, 
					   GNOME_VFS_DIRECTORY_KIND_TRASH, 
					   &result_vfs_uri, 
					   TRUE, TRUE, 0777);

	if (result != GNOME_VFS_OK) {
		fprintf (stdout, "couldn't find or create trash there, error code %d", result);
	} else {
		uri_as_string = gnome_vfs_uri_to_string (result_vfs_uri, GNOME_VFS_URI_HIDE_NONE);
		fprintf (stdout, "trash found or created here: %s", uri_as_string);
		g_free (uri_as_string);
	}

	gnome_vfs_uri_unref (from_uri);
	gnome_vfs_uri_unref (result_vfs_uri);
}

static void
do_info (void)
{
	char             *from;
	GnomeVFSResult    result;
	GnomeVFSFileInfo *info;
	const char *mime_type;

	from = get_fname ();


	info = gnome_vfs_file_info_new ();
	result = gnome_vfs_get_file_info (
		from, info, GNOME_VFS_FILE_INFO_DEFAULT);

	if (show_if_error (result, "getting info on: ", from))
		return;

	fprintf (stdout, "Name: '%s'\n", info->name);
	if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE) {
		fprintf (stdout, "Type: ");
		switch (info->type) {
		case GNOME_VFS_FILE_TYPE_UNKNOWN:
			fprintf (stdout, "unknown");
			break;
		case GNOME_VFS_FILE_TYPE_REGULAR:
			fprintf (stdout, "regular");
			break;
		case GNOME_VFS_FILE_TYPE_DIRECTORY:
			fprintf (stdout, "directory");
			break;
		case GNOME_VFS_FILE_TYPE_FIFO:
			fprintf (stdout, "fifo");
			break;
		case GNOME_VFS_FILE_TYPE_SOCKET:
			fprintf (stdout, "socket");
			break;
		case GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE:
			fprintf (stdout, "char");
			break;
		case GNOME_VFS_FILE_TYPE_BLOCK_DEVICE:
			fprintf (stdout, "block");
			break;
		case GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK:
			fprintf (stdout, "symlink");
			break;
		default:
			fprintf (stdout, "Error; invalid value");
			break;
		}
	} else
		fprintf (stdout, "Type invalid");
	fprintf (stdout, "\n");

	if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE) {
		long i = info->size;
		fprintf (stdout, "Size: %ld bytes", i);
	} else {
		fprintf (stdout, "Size invalid");
	}
	fprintf (stdout, "\n");

	mime_type = gnome_vfs_file_info_get_mime_type (info);

	fprintf (stdout, "Mime Type: %s \n", mime_type);

	fprintf (stdout, "\n");
	/* FIXME bugzilla.eazel.com 2800: hack here; should dump them all */
	    
	gnome_vfs_file_info_unref (info);
}

static void
do_cp (void)
{
	char *from = NULL;
	char *to = NULL;
	GnomeVFSHandle *from_handle = NULL;
	GnomeVFSHandle *to_handle = NULL;
	GnomeVFSResult  result;

	from = get_fname ();

	if (from)
		to = get_fname ();
	else {
		fprintf (vfserr, "cp <from> <to>\n");
		goto out;
	}
       
	result = gnome_vfs_open (&from_handle, from, GNOME_VFS_OPEN_READ);
	if (show_if_error (result, "open ", from))
		goto out;

	result = gnome_vfs_open (&to_handle, to, GNOME_VFS_OPEN_WRITE);
	if (result == GNOME_VFS_ERROR_NOT_FOUND)
		result = gnome_vfs_create (&to_handle, to, GNOME_VFS_OPEN_WRITE, FALSE,
					   GNOME_VFS_PERM_USER_ALL);
	if (show_if_error (result, "open ", to))
		goto out;

	while (1) {
		GnomeVFSFileSize bytes_read;
		GnomeVFSFileSize bytes_written;
		guint8           data [1024];
		
		result = gnome_vfs_read (from_handle, data, 1024, &bytes_read);
		if (show_if_error (result, "read ", from))
			goto out;

		if (bytes_read == 0)
			break;
		
		result = gnome_vfs_write (to_handle, data, bytes_read, &bytes_written);
		if (show_if_error (result, "write ", to))
			goto out;

		if (bytes_read != bytes_written)
			fprintf (vfserr, "Didn't write it all");
	}

 out:
	g_free (from);
	g_free (to);

	if (to_handle) {
		result = gnome_vfs_close (to_handle);
		if (show_if_error (result, "close ", to))
			/* Nothing */;
	}

	if (from_handle) {
		result = gnome_vfs_close (from_handle);
		if (show_if_error (result, "close ", from))
			/* Nothing */;
	}
}

static void
ms_ole_dump (guint8 const *ptr, guint32 len, guint32 offset)
{
	guint32 lp,lp2;
	guint32 off;

	for (lp = 0;lp<(len+15)/16;lp++) {
		printf ("%8x | ", lp*16 + offset);
		for (lp2=0;lp2<16;lp2++) {
			off = lp2 + (lp<<4);
			off<len?printf("%2x ", ptr[off]):printf("XX ");
		}
		printf ("| ");
		for (lp2=0;lp2<16;lp2++) {
			off = lp2 + (lp<<4);
			printf ("%c", off<len?(ptr[off]>'!'&&ptr[off]<127?ptr[off]:'.'):'*');
		}
		printf ("\n");
	}
}

static void
do_dump (void)
{
	char *from;
	GnomeVFSHandle *from_handle;
	GnomeVFSResult  result;
	guint32         offset;

	from = get_fname ();

	result = gnome_vfs_open (&from_handle, from, GNOME_VFS_OPEN_READ);
	if (show_if_error (result, "open ", from))
		return;

	for (offset = 0; 1; ) {
		GnomeVFSFileSize bytes_read;
		guint8           data [1024];
		
		result = gnome_vfs_read (from_handle, data, 1024, &bytes_read);
		if (show_if_error (result, "read ", from))
			return;

		if (bytes_read == 0)
			break;

		ms_ole_dump (data, bytes_read, offset);
		
		offset += bytes_read;
	}

	result = gnome_vfs_close (from_handle);
	if (show_if_error (result, "close ", from))
		return;
}


/*
 * ---------------------------------------------------------------------
 */

static char *
get_handle (void)
{
	if (!arg_data [arg_cur])
		return NULL;

	return arg_data [arg_cur++];
}

static int
get_int (void)
{
	if (!arg_data [arg_cur])
		return 0;

	return atoi (arg_data [arg_cur++]);
}

static void
do_open (void)
{
	char *from, *handle;
	GnomeVFSHandle *from_handle;
	GnomeVFSResult  result;

	handle = get_handle ();
	from = get_fname ();

	if (!handle || !from) {
		fprintf (vfserr, "open <handle> <filename>\n");
		return;
	}

	result = gnome_vfs_open (&from_handle, from, GNOME_VFS_OPEN_READ);
	if (show_if_error (result, "open ", from))
		return;

	register_file (handle, from_handle);
}

static void
do_create (void)
{
	char *from, *handle;
	GnomeVFSHandle *from_handle;
	GnomeVFSResult  result;

	handle = get_handle ();
	from = get_fname ();

	if (!handle || !from) {
		fprintf (vfserr, "create <handle> <filename>\n");
		return;
	}

	result = gnome_vfs_create (&from_handle, from, GNOME_VFS_OPEN_READ,
				   FALSE, GNOME_VFS_PERM_USER_READ |
				   GNOME_VFS_PERM_USER_WRITE);
	if (show_if_error (result, "create ", from))
		return;

	register_file (handle, from_handle);
}

static void
do_read (void)
{
	char            *handle;
	int              length;
	GnomeVFSHandle  *from_handle;
	GnomeVFSResult   result;
	GnomeVFSFileSize bytes_read;
	guint8          *data;

	handle = get_handle ();
	length = get_int ();

	if (length < 0) {
		fprintf (vfserr, "Can't read %d bytes\n", length);
		return;
	}

	from_handle = lookup_file (handle);
	if (!from_handle)
		return;

	data = g_malloc (length);
	result = gnome_vfs_read (from_handle, data, length, &bytes_read);
	if (show_if_error (result, "read ", handle))
		return;

	ms_ole_dump (data, bytes_read, 0);
}

static void
do_seek (void)
{
	char            *handle;
	int              offset;
	GnomeVFSHandle  *from_handle;
	GnomeVFSResult   result;

	handle = get_handle ();
	offset = get_int ();

	if (offset < 0) {
		fprintf (vfserr, "Can't seek to %d bytes offset\n", offset);
		return;
	}

	from_handle = lookup_file (handle);
	if (!from_handle)
		return;

	result = gnome_vfs_seek (from_handle, GNOME_VFS_SEEK_START, offset);
	if (show_if_error (result, "seek ", handle))
		return;
}

static void
do_close (void)
{
	close_file (get_handle ());
}


/*
 * ---------------------------------------------------------------------
 */

int interactive = 0;
const struct poptOption options [] = {
	{ "interactive", 'i', POPT_ARG_NONE, &interactive, 0,
	  "Allow interactive input", NULL  },
	{ NULL, '\0', 0, NULL, 0 }
};

int
main (int argc, char **argv)
{
	poptContext popt_context;
	int exit = 0;
	char *buffer = g_new (char, 1024) ;
	const char **args;
	FILE *instream;

	files = g_hash_table_new (g_str_hash, g_str_equal);

	gnome_init_with_popt_table ("test-vfs", "0.0", argc, argv,
				    options, 0, &popt_context);

	/* make the stupid "SaveYourself" warning not come up */
	gnome_client_disconnect (gnome_master_client ());

	if (interactive)
		vfserr = stderr;
	else
		vfserr = stdout;

	if (!gnome_vfs_init ()) {
		fprintf (vfserr, "Cannot initialize gnome-vfs.\n");
		return 1;
	}

	instream = stdin;
	args = poptGetArgs (popt_context);
	if (!args)
		cur_dir = g_get_current_dir ();
	else
		cur_dir = g_strdup (args [0]);

	if (cur_dir && cur_dir [strlen (cur_dir)] != '/') {
		char *new_dir = g_strconcat (cur_dir, "/", NULL);
		g_free (cur_dir);
		cur_dir = new_dir;
	}
		
	poptFreeContext (popt_context);

	do {
		char *ptr;

		if (interactive) {
			fprintf (stdout,"\n%s > ", cur_dir);
			fflush (stdout);
		}
		fgets (buffer, 1023, stdin);

		if (!buffer || buffer [0] == '#')
			continue;

		arg_data = g_strsplit (g_strchomp (buffer), delim, -1);
		arg_cur  = 0;
		if ((!arg_data || !arg_data[0]) && interactive) continue;
		if (!interactive)
			printf ("Command : '%s'\n", arg_data [0]);
		ptr = arg_data[arg_cur++];
		if (!ptr)
			continue;

		if (g_strcasecmp (ptr, "ls") == 0)
			do_ls ();
		else if (g_strcasecmp (ptr, "cd") == 0)
			do_cd ();
		else if (g_strcasecmp (ptr, "dump") == 0)
			do_dump ();
		else if (g_strcasecmp (ptr, "type") == 0 ||
			 g_strcasecmp (ptr, "cat") == 0)
			do_cat ();
		else if (g_strcasecmp (ptr, "cp") == 0)
			do_cp ();
		else if (g_strcasecmp (ptr, "rm") == 0)
			do_rm ();
		else if (g_strcasecmp (ptr, "mkdir") == 0)
			do_mkdir ();
		else if (g_strcasecmp (ptr, "rmdir") == 0)
			do_rmdir ();
		else if (g_strcasecmp (ptr, "mv") == 0)
			do_mv ();
		else if (g_strcasecmp (ptr, "info") == 0 ||
			 g_strcasecmp (ptr, "stat") == 0)
			do_info ();
		else if (g_strcasecmp (ptr, "findtrash") == 0)
			do_findtrash ();
		else if (g_strcasecmp (ptr, "sync") == 0)
			fprintf (vfserr, "a shell is like a boat, it lists or syncs (RMS)\n");
		else if (g_strcasecmp (ptr,"help") == 0 ||
			 g_strcasecmp (ptr,"?")    == 0 ||
			 g_strcasecmp (ptr,"info") == 0 ||
			 g_strcasecmp (ptr,"man")  == 0)
			list_commands ();
		else if (g_strcasecmp (ptr,"exit") == 0 ||
			 g_strcasecmp (ptr,"quit") == 0 ||
			 g_strcasecmp (ptr,"q")    == 0 ||
			 g_strcasecmp (ptr,"bye") == 0)
			exit = 1;

		/* File ops */
		else if (g_strcasecmp (ptr, "open") == 0)
			do_open ();
		else if (g_strcasecmp (ptr, "create") == 0)
			do_create ();
		else if (g_strcasecmp (ptr, "close") == 0)
			do_close ();
		else if (g_strcasecmp (ptr, "read") == 0)
			do_read ();
		else if (g_strcasecmp (ptr, "seek") == 0)
			do_seek ();
		
		else
			fprintf (vfserr, "Unknown command '%s'", ptr);

		g_strfreev (arg_data);
		arg_data = NULL;
	} while (!exit);

	g_free (buffer);
	g_free (cur_dir);

	close_files ();

	return 0;
}
