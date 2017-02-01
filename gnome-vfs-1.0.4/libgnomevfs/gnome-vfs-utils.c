/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-utils.c - Private utility functions for the GNOME Virtual
   File System.

   Copyright (C) 1999 Free Software Foundation
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

   Authors: Ettore Perazzoli <ettore@comm2000.it>
   	    John Sullivan <sullivan@eazel.com> 
            Darin Adler <darin@eazel.com>
*/

#include <config.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include "gnome-vfs-utils.h"
#include "gnome-vfs-private-utils.h"
#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif

#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#elif HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#define KILOBYTE_FACTOR 1024.0
#define MEGABYTE_FACTOR (1024.0 * 1024.0)
#define GIGABYTE_FACTOR (1024.0 * 1024.0 * 1024.0)

gchar*
gnome_vfs_format_file_size_for_display (GnomeVFSFileSize bytes)
{
	if (bytes < (GnomeVFSFileSize) KILOBYTE_FACTOR) {
		if (bytes == 1)
			return g_strdup (_("1 byte"));
		else
			return g_strdup_printf (_("%u bytes"),
						       (guint) bytes);
	} else {
		gdouble displayed_size;

		if (bytes < (GnomeVFSFileSize) MEGABYTE_FACTOR) {
			displayed_size = (gdouble) bytes / KILOBYTE_FACTOR;
			return g_strdup_printf (_("%.1f K"),
						       displayed_size);
		} else if (bytes < (GnomeVFSFileSize) GIGABYTE_FACTOR) {
			displayed_size = (gdouble) bytes / MEGABYTE_FACTOR;
			return g_strdup_printf (_("%.1f MB"),
						       displayed_size);
		} else {
			displayed_size = (gdouble) bytes / GIGABYTE_FACTOR;
			return g_strdup_printf (_("%.1f GB"),
						       displayed_size);
		}
	}
}

typedef enum {
	UNSAFE_ALL        = 0x1,  /* Escape all unsafe characters   */
	UNSAFE_ALLOW_PLUS = 0x2,  /* Allows '+'  */
	UNSAFE_PATH       = 0x4,  /* Allows '/' and '?' and '&' and '='  */
	UNSAFE_DOS_PATH   = 0x8,  /* Allows '/' and '?' and '&' and '=' and ':' */
	UNSAFE_HOST       = 0x10, /* Allows '/' and ':' and '@' */
	UNSAFE_SLASHES    = 0x20  /* Allows all characters except for '/' and '%' */
} UnsafeCharacterSet;

static const guchar acceptable[96] =
{ /* X0   X1   X2   X3   X4   X5   X6   X7   X8   X9   XA   XB   XC   XD   XE   XF */
    0x00,0x3F,0x20,0x20,0x20,0x00,0x2C,0x3F,0x3F,0x3F,0x3F,0x22,0x20,0x3F,0x3F,0x1C, /* 2X  !"#$%&'()*+,-./   */
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x38,0x20,0x20,0x2C,0x20,0x2C, /* 3X 0123456789:;<=>?   */
    0x30,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F, /* 4X @ABCDEFGHIJKLMNO   */
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x20,0x3F, /* 5X PQRSTUVWXYZ[\]^_   */
    0x20,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F, /* 6X `abcdefghijklmno   */
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x3F,0x20  /* 7X pqrstuvwxyz{|}~DEL */
};

enum {
	RESERVED = 1,
	UNRESERVED,
	DELIMITERS,
	UNWISE,
	CONTROL,
	SPACE	
};

static const guchar uri_character_kind[128] =
{
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   , 
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    /* ' '        !          "          #          $          %          &          '      */
    SPACE     ,UNRESERVED,DELIMITERS,DELIMITERS,RESERVED  ,DELIMITERS,RESERVED  ,UNRESERVED,
    /*  (         )          *          +          ,          -          .          /      */
    UNRESERVED,UNRESERVED,UNRESERVED,RESERVED  ,RESERVED  ,UNRESERVED,UNRESERVED,RESERVED  , 
    /*  0         1          2          3          4          5          6          7      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  8         9          :          ;          <          =          >          ?      */
    UNRESERVED,UNRESERVED,RESERVED  ,RESERVED  ,DELIMITERS,RESERVED  ,DELIMITERS,RESERVED  ,
    /*  @         A          B          C          D          E          F          G      */
    RESERVED  ,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  H         I          J          K          L          M          N          O      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  P         Q          R          S          T          U          V          W      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  X         Y          Z          [          \          ]          ^          _      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNWISE    ,UNWISE    ,UNWISE    ,UNWISE    ,UNRESERVED,
    /*  `         a          b          c          d          e          f          g      */
    UNWISE    ,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  h         i          j          k          l          m          n          o      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  p         q          r          s          t          u          v          w      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  x         y          z         {           |          }          ~         DEL     */
    UNRESERVED,UNRESERVED,UNRESERVED,UNWISE    ,UNWISE    ,UNWISE    ,UNRESERVED,CONTROL
};


/*  Below modified from libwww HTEscape.c */

#define HEX_ESCAPE '%'

/*  Escape undesirable characters using %
 *  -------------------------------------
 *
 * This function takes a pointer to a string in which
 * some characters may be unacceptable unescaped.
 * It returns a string which has these characters
 * represented by a '%' character followed by two hex digits.
 *
 * This routine returns a g_malloced string.
 */

static const gchar hex[16] = "0123456789ABCDEF";

static gchar *
gnome_vfs_escape_string_internal (const gchar *string, 
				  UnsafeCharacterSet mask)
{
#define ACCEPTABLE(a) ((a)>=32 && (a)<128 && (acceptable[(a)-32] & use_mask))

	const gchar *p;
	gchar *q;
	gchar *result;
	guchar c;
	gint unacceptable;
	UnsafeCharacterSet use_mask;

	g_return_val_if_fail (mask == UNSAFE_ALL
			      || mask == UNSAFE_ALLOW_PLUS
			      || mask == UNSAFE_PATH
			      || mask == UNSAFE_DOS_PATH
			      || mask == UNSAFE_HOST
			      || mask == UNSAFE_SLASHES, NULL);

	if (string == NULL) {
		return NULL;
	}
	
	unacceptable = 0;
	use_mask = mask;
	for (p = string; *p != '\0'; p++) {
		c = *p;
		if (!ACCEPTABLE (c)) {
			unacceptable++;
		}
		if ((use_mask == UNSAFE_HOST) && 
		    (unacceptable || (c == '/'))) {
			/* when escaping a host, if we hit something that needs to be escaped, or we finally
			 * hit a path separator, revert to path mode (the host segment of the url is over).
			 */
			use_mask = UNSAFE_PATH;
		}
	}
	
	result = g_malloc (p - string + unacceptable * 2 + 1);

	use_mask = mask;
	for (q = result, p = string; *p != '\0'; p++){
		c = *p;
		
		if (!ACCEPTABLE (c)) {
			*q++ = HEX_ESCAPE; /* means hex coming */
			*q++ = hex[c >> 4];
			*q++ = hex[c & 15];
		} else {
			*q++ = c;
		}
		if ((use_mask == UNSAFE_HOST) &&
		    (!ACCEPTABLE (c) || (c == '/'))) {
			use_mask = UNSAFE_PATH;
		}
	}
	
	*q = '\0';
	
	return result;
}

gchar *
gnome_vfs_escape_string (const gchar *file_name)
{
	return gnome_vfs_escape_string_internal (file_name, UNSAFE_ALL);
}

gchar *
gnome_vfs_escape_path_string (const gchar *path)
{
	return gnome_vfs_escape_string_internal (path, UNSAFE_PATH);
}

gchar *
gnome_vfs_escape_host_and_path_string (const gchar *path)
{
	return gnome_vfs_escape_string_internal (path, UNSAFE_HOST);
}

gchar *
gnome_vfs_escape_slashes (const gchar *string)
{
	return gnome_vfs_escape_string_internal (string, UNSAFE_SLASHES);
}

char *
gnome_vfs_escape_set (const char *string,
	              const char *match_set)
{
	char *result;
	const char *scanner;
	char *result_scanner;
	int escape_count;

	escape_count = 0;

	if (string == NULL) {
		return NULL;
	}

	if (match_set == NULL) {
		return g_strdup (string);
	}
	
	for (scanner = string; *scanner != '\0'; scanner++) {
		if (strchr(match_set, *scanner) != NULL) {
			/* this character is in the set of characters 
			 * we want escaped.
			 */
			escape_count++;
		}
	}
	
	if (escape_count == 0) {
		return g_strdup (string);
	}

	/* allocate two extra characters for every character that
	 * needs escaping and space for a trailing zero
	 */
	result = g_malloc (scanner - string + escape_count * 2 + 1);
	for (scanner = string, result_scanner = result; *scanner != '\0'; scanner++) {
		if (strchr(match_set, *scanner) != NULL) {
			/* this character is in the set of characters 
			 * we want escaped.
			 */
			*result_scanner++ = HEX_ESCAPE;
			*result_scanner++ = hex[*scanner >> 4];
			*result_scanner++ = hex[*scanner & 15];
			
		} else {
			*result_scanner++ = *scanner;
		}
	}

	*result_scanner = '\0';

	return result;
}

char *
gnome_vfs_expand_initial_tilde (const char *path)
{
	char *slash_after_user_name, *user_name;
	struct passwd *passwd_file_entry;

	g_return_val_if_fail (path != NULL, NULL);

	if (path[0] != '~') {
		return g_strdup (path);
	}
	
	if (path[1] == '/' || path[1] == '\0') {
		return g_strconcat (g_get_home_dir (), &path[1], NULL);
	}

	slash_after_user_name = strchr (&path[1], '/');
	if (slash_after_user_name == NULL) {
		user_name = g_strdup (&path[1]);
	} else {
		user_name = g_strndup (&path[1],
				       slash_after_user_name - &path[1]);
	}
	passwd_file_entry = getpwnam (user_name);
	g_free (user_name);

	if (passwd_file_entry == NULL || passwd_file_entry->pw_dir == NULL) {
		return g_strdup (path);
	}

	return g_strconcat (passwd_file_entry->pw_dir,
			    slash_after_user_name,
			    NULL);
}

static int
hex_to_int (gchar c)
{
	return  c >= '0' && c <= '9' ? c - '0'
		: c >= 'A' && c <= 'F' ? c - 'A' + 10
		: c >= 'a' && c <= 'f' ? c - 'a' + 10
		: -1;
}

static int
unescape_character (const char *scanner)
{
	int first_digit;
	int second_digit;

	first_digit = hex_to_int (*scanner++);
	if (first_digit < 0) {
		return -1;
	}

	second_digit = hex_to_int (*scanner++);
	if (second_digit < 0) {
		return -1;
	}

	return (first_digit << 4) | second_digit;
}

/*  Decode %xx escaped characters
**  -----------------------------
**
** This function takes a pointer to a string in which some
** characters may have been encoded in %xy form, where xy is
** the ASCII hex code for character 16x+y.
*/

gchar *
gnome_vfs_unescape_string (const gchar *escaped, const gchar *illegal_characters)
{
	const gchar *in;
	gchar *out, *result;
	gint character;
	
	if (escaped == NULL) {
		return NULL;
	}

	result = g_malloc (strlen (escaped) + 1);
	
	out = result;
	for (in = escaped; *in != '\0'; in++) {
		character = *in;
		if (*in == HEX_ESCAPE) {
			character = unescape_character (in + 1);

			/* Check for an illegal character. We consider '\0' illegal here. */
			if (character <= 0
			    || (illegal_characters != NULL
				&& strchr (illegal_characters, (char)character) != NULL)) {
				g_free (result);
				return NULL;
			}
			in += 2;
		}
		*out++ = (char)character;
	}
	
	*out = '\0';
	g_assert (out - result <= strlen (escaped));
	return result;
	
}

/**
 * gnome_vfs_unescape_for_display:
 * @escaped: The string encoded with escaped sequences
 * 
 * Similar to gnome_vfs_unescape_string, but it returns something
 * semi-intelligable to a user even upon receiving traumatic input
 * such as %00 or URIs in bad form.
 * 
 * See also: gnome_vfs_unescape_string.
 * 
 * Return value: A pointer to a g_malloc'd string with all characters
 *               replacing their escaped hex values
 *
 * WARNING: You should never use this function on a whole URI!  It
 * unescapes reserved characters, and can result in a mangled URI
 * that can not be re-entered.  For example, it unescapes "#" "&" and "?",
 * which have special meanings in URI strings.
 **/
gchar *
gnome_vfs_unescape_string_for_display (const gchar *escaped)
{
	const gchar *in, *start_escape;
	gchar *out, *result;
	gint i,j;
	gchar c;
	gint invalid_escape;

	if (escaped == NULL) {
		return NULL;
	}

	result = g_malloc (strlen (escaped) + 1);
	
	out = result;
	for (in = escaped; *in != '\0'; ) {
		start_escape = in;
		c = *in++;
		invalid_escape = 0;
		
		if (c == HEX_ESCAPE) {
			/* Get the first hex digit. */
			i = hex_to_int (*in++);
			if (i < 0) {
				invalid_escape = 1;
				in--;
			}
			c = i << 4;
			
			if (invalid_escape == 0) {
				/* Get the second hex digit. */
				i = hex_to_int (*in++);
				if (i < 0) {
					invalid_escape = 2;
					in--;
				}
				c |= i;
			}
			if (invalid_escape == 0) {
				/* Check for an illegal character. */
				if (c == '\0') {
					invalid_escape = 3;
				}
			}
		}
		if (invalid_escape != 0) {
			for (j = 0; j < invalid_escape; j++) {
				*out++ = *start_escape++;
			}
		} else {
			*out++ = c;
		}
	}
	
	*out = '\0';
	g_assert (out - result <= strlen (escaped));
	return result;
}

/**
 * gnome_vfs_remove_optional_escapes:
 * @uri: an escaped uri
 * 
 * Scans the uri and converts characters that do not have to be 
 * escaped into an un-escaped form. The characters that get treated this
 * way are defined as unreserved by the RFC.
 * 
 * Return value: an error value if the uri is found to be malformed.
 **/
GnomeVFSResult
gnome_vfs_remove_optional_escapes (char *uri)
{
	guchar *scanner;
	int character;
	int length;

	if (uri == NULL) {
		return GNOME_VFS_OK;
	}
	
	length = strlen (uri);

	for (scanner = uri; *scanner != '\0'; scanner++, length--) {
		if (*scanner == HEX_ESCAPE) {
			character = unescape_character (scanner + 1);
			if (character < 0) {
				/* invalid hexadecimal character */
				return GNOME_VFS_ERROR_INVALID_URI;
			}

			if (uri_character_kind [character] == UNRESERVED) {
				/* This character does not need to be escaped, convert it
				 * to a non-escaped form.
				 */
				*scanner = (guchar)character;
				g_assert (length >= 3);

				/* Shrink the string covering up the two extra digits of the
				 * escaped character. Include the trailing '\0' in the copy
				 * to keep the string terminated.
				 */
				memmove (scanner + 1, scanner + 3, length - 2);
			} else {
				/* This character must stay escaped, skip the entire
				 * escaped sequence
				 */
				scanner += 2;
			}
			length -= 2;

		} else if (*scanner > 127
			|| uri_character_kind [*scanner] == DELIMITERS
			|| uri_character_kind [*scanner] == UNWISE
			|| uri_character_kind [*scanner] == CONTROL) {
			/* It is illegal for this character to be in an un-escaped form
			 * in the uri.
			 */
			return GNOME_VFS_ERROR_INVALID_URI;
		}
	}
	return GNOME_VFS_OK;
}

char *
gnome_vfs_make_uri_canonical (const char *original_uri_text)
{
	/* For now use a sub-optimal implementation involving a
	 * conversion to GnomeVFSURI and back.
	 */
	GnomeVFSURI *uri;
	char *result;

	uri = gnome_vfs_uri_new_private (original_uri_text, TRUE, TRUE, FALSE);
	if (uri == NULL) {
		return NULL;;
	} 

	result = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	gnome_vfs_uri_unref (uri);

	return result;
}


/**
 * gnome_vfs_make_canonical_pathname:
 * @path: a file path, relative or absolute
 * 
 * Calls gnome_vfs_canonicalize_pathname, allocating storage for the result and
 * providing for a cleaner memory management.
 * 
 * Return value: a canonical version of @path
 **/
gchar *
gnome_vfs_make_path_name_canonical (const gchar *path)
{
	char *path_clone;
	char *result;

	path_clone = g_strdup (path);
	result = gnome_vfs_canonicalize_pathname (path_clone);
	if (result != path_clone) {
		g_free (path_clone);
		return g_strdup (result);
	}

	return path_clone;
}

void
gnome_vfs_list_deep_free (GList *list)
{
	GList *p;

	if (list == NULL)
		return;

	for (p = list; p != NULL; p = p->next) {
		g_free (p->data);
	}
	g_list_free (list);
}

/**
 * gnome_vfs_get_local_path_from_uri:
 * 
 * Return a local path for a file:/// URI.
 *
 * Return value: the local path 
 * NULL is returned on error or if the uri isn't a file: URI
 * without a fragment identifier (or chained URI).
 **/
char *
gnome_vfs_get_local_path_from_uri (const char *uri)
{
	const char *path_part;

	if (!gnome_vfs_istr_has_prefix (uri, "file:/")) {
		return NULL;
	}
	
	path_part = uri + strlen ("file:");
	if (strchr (path_part, '#') != NULL) {
		return NULL;
	}
	
	if (gnome_vfs_istr_has_prefix (path_part, "///")) {
		path_part += 2;
	} else if (gnome_vfs_istr_has_prefix (path_part, "//")) {
		return NULL;
	}

	return gnome_vfs_unescape_string (path_part, "/");
}

/**
 * gnome_vfs_get_uri_from_local_path:
 * 
 * Return a file:/// URI for a local path.
 *
 * Return value: the URI (NULL for some bad errors).
 **/
char *
gnome_vfs_get_uri_from_local_path (const char *local_path)
{
	char *escaped_path, *result;
	
	if (local_path == NULL) {
		return NULL;
	}

	g_return_val_if_fail (local_path[0] == '/', NULL);

	escaped_path = gnome_vfs_escape_path_string (local_path);
	result = g_strconcat ("file://", escaped_path, NULL);
	g_free (escaped_path);
	return result;
}

/* gnome_vfs_get_volume_free_space
 * 
 * Return total amount of free space on a volume.
 * This only works for local file systems with
 * the file: scheme.
 */
GnomeVFSResult
gnome_vfs_get_volume_free_space (const GnomeVFSURI *vfs_uri, GnomeVFSFileSize *size)
{	
	GnomeVFSFileSize free_blocks, block_size;
       	int statfs_result;
	const char *path, *scheme;
	char *unescaped_path;
	GnomeVFSResult ret;
#if HAVE_STATVFS
	struct statvfs statfs_buffer;
#else
	struct statfs statfs_buffer;
#endif

 	*size = 0;

	/* We can't check non local systems */
	if (!gnome_vfs_uri_is_local (vfs_uri)) {
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

	path = gnome_vfs_uri_get_path (vfs_uri);

	unescaped_path = gnome_vfs_unescape_string (path, G_DIR_SEPARATOR_S);
	
	scheme = gnome_vfs_uri_get_scheme (vfs_uri);
	
        /* We only handle the file scheme for now */
	if (g_strcasecmp (scheme, "file") != 0 || !gnome_vfs_istr_has_prefix (path, "/")) {
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

#if HAVE_STATVFS
	statfs_result = statvfs (unescaped_path, &statfs_buffer);
#else
	statfs_result = statfs (unescaped_path, &statfs_buffer);   
#endif  

	if (statfs_result == 0) {
		ret = GNOME_VFS_OK;
	} else {
		ret = gnome_vfs_result_from_errno ();
	}
	
	g_return_val_if_fail (statfs_result == 0, FALSE);
	block_size = statfs_buffer.f_bsize; 
	free_blocks = statfs_buffer.f_bavail;

	*size = block_size * free_blocks;

	g_free (unescaped_path);
	
	return ret;
}

/**
 * hack_file_exists
 * @filename: pathname to test for existance.
 *
 * Returns true if filename exists
 */
/* FIXME: Why is this here? Why not use g_file_exists in libgnome/gnome-util.h?
 * (I tried to simply replace but there were strange include dependencies, maybe
 * that's why this function exists.)
 */
static int
hack_file_exists (const char *filename)
{
	struct stat s;

	g_return_val_if_fail (filename != NULL,FALSE);
    
	return stat (filename, &s) == 0;
}


char *
gnome_vfs_icon_path_from_filename (const char *relative_filename)
{
	const char *gnome_var;
	char *full_filename;
	char **paths, **temp_paths;

	gnome_var = g_getenv ("GNOME_PATH");

	if (gnome_var == NULL) {
		gnome_var = GNOME_VFS_PREFIX;
	}

	paths = g_strsplit (gnome_var, ":", 0); 

	for (temp_paths = paths; *temp_paths != NULL; temp_paths++) {
		full_filename = g_strconcat (*temp_paths, "/share/pixmaps/", relative_filename, NULL);
		if (hack_file_exists (full_filename)) {
			g_strfreev (paths);
			return full_filename;
		}
		g_free (full_filename);
		full_filename = NULL;
	}

	g_strfreev (paths);
	return NULL;
}


static char *
strdup_to (const char *string, const char *end)
{
	if (end == NULL) {
		return g_strdup (string);
	}
	return g_strndup (string, end - string);
}

static gboolean
is_executable_file (const char *path)
{
	struct stat stat_buffer;

	/* Check that it exists. */
	if (stat (path, &stat_buffer) != 0) {
		return FALSE;
	}

	/* Check that it is a file. */
	if (!S_ISREG (stat_buffer.st_mode)) {
		return FALSE;
	}

	/* Check that it's executable. */
	if (access (path, X_OK) != 0) {
		return FALSE;
	}

	return TRUE;
}


static gboolean
executable_in_path (const char *executable_name)
{
	const char *path_list, *piece_start, *piece_end;
	char *piece, *raw_path, *expanded_path;
	gboolean is_good;

	path_list = g_getenv ("PATH");

	for (piece_start = path_list; ; piece_start = piece_end + 1) {
		/* Find the next piece of PATH. */
		piece_end = strchr (piece_start, ':');
		piece = strdup_to (piece_start, piece_end);
		g_strstrip (piece);
		
		if (piece[0] == '\0') {
			is_good = FALSE;
		} else {
			/* Try out this path with the executable. */
			raw_path = g_strconcat (piece, "/", executable_name, NULL);
			expanded_path = gnome_vfs_expand_initial_tilde (raw_path);
			g_free (raw_path);
			
			is_good = is_executable_file (expanded_path);
			g_free (expanded_path);
		}
		
		g_free (piece);
		
		if (is_good) {
			return TRUE;
		}

		if (piece_end == NULL) {
			return FALSE;
		}
	}
}

static char *
get_executable_name_from_command_string (const char *command_string)
{
	/* FIXME bugzilla.eazel.com 2757: 
	 * We need to handle quoting here for the full-path case */
	return g_strstrip (strdup_to (command_string, strchr (command_string, ' ')));
}

/* Returns TRUE if commmand_string starts with the full path for an executable
 * file, or starts with a command for an executable in $PATH.
 */
gboolean
gnome_vfs_is_executable_command_string (const char *command_string)
{
	char *executable_name;
	char *executable_path;
	gboolean found;

	/* Check whether command_string is a full path for an executable. */
	if (command_string[0] == '/') {

		/* FIXME bugzilla.eazel.com 2757:
		 * Because we don't handle quoting, we can check for full
		 * path including spaces, but no parameters, and full path
		 * with no spaces with or without parameters. But this will
		 * fail for quoted full path with spaces, and parameters.
		 */

		/* This works if command_string contains a space, but not
		 * if command_string has parameters.
		 */
		if (is_executable_file (command_string)) {
			return TRUE;
		}

		/* This works if full path has no spaces, with or without parameters */
		executable_path = get_executable_name_from_command_string (command_string);
		found = is_executable_file (executable_path);
		g_free (executable_path);

		return found;
	}
	
	executable_name = get_executable_name_from_command_string (command_string);
	found = executable_in_path (executable_name);
	g_free (executable_name);

	return found;
}
