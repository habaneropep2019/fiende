/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* test-mime.c - Test program for the GNOME Virtual File System.

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

   Authors: 
   	Darin Adler <darin@eazel.com>
	Ian McKellar <yakk@yakk.net.au>
*/

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include "gnome-vfs.h"
#include "gnome-vfs-private-utils.h"

#define TEST_ASSERT(expression, message) \
	G_STMT_START { if (!(expression)) test_failed message; } G_STMT_END

static void
stop_after_log (const char *domain, GLogLevelFlags level, 
	const char *message, gpointer data)
{
	void (* saved_handler) (int);
	
	g_log_default_handler (domain, level, message, data);

	saved_handler = signal (SIGINT, SIG_IGN);
	raise (SIGINT);
	signal (SIGINT, saved_handler);
}

static void
make_asserts_break (const char *domain)
{
	g_log_set_handler
		(domain, 
		 (GLogLevelFlags) (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING),
		 stop_after_log, NULL);
}

static gboolean at_least_one_test_failed = FALSE;

static void
test_failed (const char *format, ...)
{
	va_list arguments;
	char *message;

	va_start (arguments, format);
	message = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	g_message ("test failed: %s", message);
	at_least_one_test_failed = TRUE;
}

static void
test_make_canonical_path (const char *input,
		          const char *expected_output)
{
	char *output;

	output = gnome_vfs_make_path_name_canonical (input);

	if (strcmp (output, expected_output) != 0) {
		test_failed ("test_make_canonical_path (%s) resulted in %s instead of %s",
			     input, output, expected_output);
	}

	g_free (output);
}

static void
test_uri_to_string (const char *input,
		    const char *expected_output,
		    GnomeVFSURIHideOptions hide_options)
{
	GnomeVFSURI *uri;
	char *output;

	uri = gnome_vfs_uri_new (input);
	if (uri == NULL) {
		output = g_strdup ("NULL");
	} else {
		output = gnome_vfs_uri_to_string (uri, hide_options);
		gnome_vfs_uri_unref (uri);
	}

	if (strcmp (output, expected_output) != 0) {
		test_failed ("gnome_vfs_uri_to_string (%s, %d) resulted in %s instead of %s",
			     input, hide_options, output, expected_output);
	}

	g_free (output);
}

static void
test_make_canonical (const char *input,
		     const char *expected_output)
{
	char *output;

	output = gnome_vfs_make_uri_canonical (input);
	if (output == NULL) {
		output = g_strdup ("NULL");
	}

	if (strcmp (output, expected_output) != 0) {
		test_failed ("test_make_canonical (%s) resulted in %s instead of %s",
			     input, output, expected_output);
	}

	g_free (output);
}

static void
test_uri_match (const char *uri_string_1, const char *uri_string_2, gboolean expected_result)
{
	GnomeVFSURI *uri1;
	GnomeVFSURI *uri2;
	
	uri1 = gnome_vfs_uri_new (uri_string_1);
	uri2 = gnome_vfs_uri_new (uri_string_2);
	
	if (gnome_vfs_uri_equal (uri1, uri2) != expected_result) {
		test_failed ("test_uri_match (%s, %s) resulted in a %s instead of %s",
			uri_string_1, uri_string_2,
			expected_result ? "mismatch" : "match",
			expected_result ? "match" : "mismatch");
	}
	
	gnome_vfs_uri_unref (uri2);
	gnome_vfs_uri_unref (uri1);
}

static void
test_file_path_to_uri_string (const char *input,
			      const char *expected_output,
			      GnomeVFSURIHideOptions hide_options)
{
	GnomeVFSURI *uri, *resulting_uri;
	char *output;
	char *unescaped_output;

	uri = gnome_vfs_uri_new ("file:/");
	resulting_uri = gnome_vfs_uri_append_path (uri, input);
	gnome_vfs_uri_unref (uri);

	output = gnome_vfs_uri_to_string (resulting_uri, hide_options);
	gnome_vfs_uri_unref (resulting_uri);

	unescaped_output = gnome_vfs_unescape_string (output, "/");
	g_free (output);

	if (strcmp (unescaped_output, expected_output) != 0) {
		test_failed ("gnome_vfs_uri_to_string (%s, %d) resulted in %s instead of %s",
			     input, hide_options, unescaped_output, expected_output);
	}

	g_free (unescaped_output);
}

static void
test_uri_has_fragment_id (const char *input,
		          const char *expected_output)
{
	GnomeVFSURI *uri;
	char *output;
	
	uri = gnome_vfs_uri_new (input);
	if (uri == NULL) {
		output = g_strdup ("NULL");
	} else {
		output = g_strdup (gnome_vfs_uri_get_fragment_identifier (uri));
	}

	if (strcmp (output, expected_output) != 0) {
		test_failed ("test_uri_has_fragment_id (%s) resulted in %s instead of %s",
			     input, output, expected_output);
	}

	g_free (output);
	gnome_vfs_uri_unref (uri);
}

static void
test_uri_parent (const char *input,
		 const char *expected_output)
{
	GnomeVFSURI *uri, *parent;
	char *output;

	uri = gnome_vfs_uri_new (input);
	if (uri == NULL) {
		output = g_strdup ("URI NULL");
	} else {
		parent = gnome_vfs_uri_get_parent (uri);
		gnome_vfs_uri_unref (uri);
		if (parent == NULL) {
			output = g_strdup ("NULL");
		} else {
			output = gnome_vfs_uri_to_string (parent, GNOME_VFS_URI_HIDE_NONE);
			gnome_vfs_uri_unref (parent);
		}
	}

	if (strcmp (output, expected_output) != 0) {
		test_failed ("gnome_vfs_uri_parent (%s) resulted in %s instead of %s",
			     input, output, expected_output);
	}

	g_free (output);
}

static void
test_uri_has_parent (const char *input,
		     const char *expected_output)
{
	GnomeVFSURI *uri;
	const char *output;
	gboolean has;

	uri = gnome_vfs_uri_new (input);
	if (uri == NULL) {
		output = "URI NULL";
	} else {
		has = gnome_vfs_uri_has_parent (uri);
		gnome_vfs_uri_unref (uri);
		output = has ? "TRUE" : "FALSE";
	}

	if (strcmp (output, expected_output) != 0) {
		test_failed ("gnome_vfs_uri_has_parent (%s) resulted in %s instead of %s",
			     input, output, expected_output);
	}
}

/*
 * Ensure that gnome_vfs_uri_{get_host_name,get_scheme,get_user_name,get_password} 
 * return expected results
 */  
static void
test_uri_part (const char *input, 
	       const char *expected_output,
	       const char *(*func_gnome_vfs_uri)(const GnomeVFSURI *)
	       )
{
	GnomeVFSURI *uri;
	const char *output;

	uri = gnome_vfs_uri_new (input);
	if (NULL == uri) {
		output = "URI NULL";
	} else {
		output = func_gnome_vfs_uri(uri);
		if ( NULL == output ) {
			output = "NULL";
		}
	}

	if (strcmp (output, expected_output) != 0) {
		test_failed ("gnome_vfs_uri_{?} (%s) resulted in %s instead of %s",
			     input, output, expected_output);
	}

	if ( NULL != uri ) {
		gnome_vfs_uri_unref (uri);
	}

}

/*
 * Ensure that gnome_vfs_uri_get_host_port
 * return expected results
 */  
static void
test_uri_host_port (const char *input, 
		    guint expected_port)
{
	GnomeVFSURI *uri;
	gboolean success = FALSE;
	guint port;

	port = 0;
	uri = gnome_vfs_uri_new (input);
	if (NULL != uri) {
		port = gnome_vfs_uri_get_host_port(uri);
	 	if (expected_port == port) {
			success = TRUE;
			gnome_vfs_uri_unref (uri);
		}
	}

	if (!success) {
		test_failed ("gnome_vfs_uri_get_host_port (%s) resulted in %u instead of %u",
			     input, port, expected_port);
	}
}

static void
test_uri_is_parent_common (const char *parent, const char *item, gboolean deep, gboolean expected_result)
{
	GnomeVFSURI *item_uri;
	GnomeVFSURI *parent_uri;
	gboolean result;
	
	item_uri = gnome_vfs_uri_new (item);
	parent_uri = gnome_vfs_uri_new (parent);
	
	result = gnome_vfs_uri_is_parent (parent_uri, item_uri, deep);
	
	if (result != expected_result) {
		test_failed ("gnome_vfs_uri_is_parent (%s, %s) resulted in \"%s\" instead of \"%s\"",
			     parent, item, result ? "TRUE" : "FALSE", expected_result ? "TRUE" : "FALSE");
	}
}

static void
test_uri_is_parent_deep (const char *parent, const char *item, gboolean expected_result)
{
	test_uri_is_parent_common (parent, item, TRUE, expected_result);
}

static void
test_uri_is_parent_shallow (const char *parent, const char *item, gboolean expected_result)
{
	test_uri_is_parent_common (parent, item, FALSE, expected_result);
}

#define VERIFY_STRING_RESULT(function, expected) \
	G_STMT_START {											\
		char *result = function; 								\
		if (!((result == NULL && expected == NULL)						\
		      || (result != NULL && expected != NULL && strcmp (result, (char *)expected) == 0))) {	\
			test_failed ("%s: returned '%s' expected '%s'", #function, result, expected);	\
		}											\
	} G_STMT_END

int
main (int argc, char **argv)
{
	make_asserts_break ("GLib");
	make_asserts_break ("GnomeVFS");

	/* Initialize the libraries we use. */
	g_thread_init (NULL);
	gnome_vfs_init ();

	/* Test the "make canonical" call for pathnames. */
	test_make_canonical_path ("", "");
	test_make_canonical_path ("/", "/");
	test_make_canonical_path ("/.", "/");
	test_make_canonical_path ("/./.", "/");
	test_make_canonical_path ("/.//.", "/");
	test_make_canonical_path ("/.///.", "/");
	test_make_canonical_path ("a", "a");
	test_make_canonical_path ("/a/b/..", "/a");
	test_make_canonical_path ("a///", "a/");
	test_make_canonical_path ("./a", "a");
	test_make_canonical_path ("../a", "../a");
	test_make_canonical_path ("..//a", "../a");
	test_make_canonical_path ("a/.", "a");
	test_make_canonical_path ("/a/.", "/a");
	test_make_canonical_path ("/a/..", "/");
	test_make_canonical_path ("a//.", "a");
	test_make_canonical_path ("./a/.", "a");
	test_make_canonical_path (".//a/.", "a");
	test_make_canonical_path ("./a//.", "a");
	test_make_canonical_path ("a/..", "");
	test_make_canonical_path ("a//..", "");
	test_make_canonical_path ("./a/..", "");
	test_make_canonical_path (".//a/..", "");
	test_make_canonical_path ("./a//..", "");
	test_make_canonical_path (".//a//..", "");
	test_make_canonical_path ("a/b/..", "a");
	test_make_canonical_path ("./a/b/..", "a");
	test_make_canonical_path ("/./a/b/..", "/a");
	test_make_canonical_path ("/a/./b/..", "/a");
	test_make_canonical_path ("/a/b/./..", "/a");
	test_make_canonical_path ("/a/b/../.", "/a");
	test_make_canonical_path ("a/b/../..", "");
	test_make_canonical_path ("./a/b/../..", "");
	test_make_canonical_path ("././a/b/../..", "");
	test_make_canonical_path ("a/b/c/../..", "a");
	test_make_canonical_path ("a/b/c/../../d", "a/d");
	test_make_canonical_path ("a/b/../../d", "d");
	test_make_canonical_path ("a/../../d", "../d");
	test_make_canonical_path ("a/b/.././.././c", "c");
	test_make_canonical_path ("a/.././.././b/c", "../b/c");
	test_make_canonical_path ("\\", "\\");

	test_uri_to_string ("", "NULL", GNOME_VFS_URI_HIDE_NONE);

	test_uri_to_string ("http://www.eazel.com", "http://www.eazel.com", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("http://www.eazel.com/", "http://www.eazel.com/", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("http://www.eazel.com/dir", "http://www.eazel.com/dir", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("http://www.eazel.com/dir/", "http://www.eazel.com/dir/", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("http://yakk:womble@www.eazel.com:42/blah/", "http://yakk:womble@www.eazel.com:42/blah/", GNOME_VFS_URI_HIDE_NONE);

	test_uri_to_string ("http://yakk:womble@www.eazel.com:42/blah/", "http://:womble@www.eazel.com:42/blah/", GNOME_VFS_URI_HIDE_USER_NAME);
	test_uri_to_string ("FILE://", "file:", GNOME_VFS_URI_HIDE_NONE);

	test_uri_to_string ("file:///trash", "file:///trash", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("file:///Users/mikef", "file:///Users/mikef", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/trash", "file:///trash", GNOME_VFS_URI_HIDE_NONE);

	/* test URI parts */
	test_uri_part ("http://www.eazel.com:80/", "http", gnome_vfs_uri_get_scheme);
	test_uri_part ("http://www.eazel.com:80/", "www.eazel.com", gnome_vfs_uri_get_host_name);
	test_uri_part ("http://www.eazel.com:80/", "NULL", gnome_vfs_uri_get_user_name);
	test_uri_part ("http://www.eazel.com:80/", "NULL", gnome_vfs_uri_get_password);
	test_uri_part ("http://www.eazel.com:80/", "/", gnome_vfs_uri_get_path);

	test_uri_host_port ("http://www.eazel.com/", 0);
	test_uri_host_port ("http://www.eazel.com:80/", 80);

	/* Now--same thing w/o trailing / */
	test_uri_part ("http://www.eazel.com:80", "http", gnome_vfs_uri_get_scheme);
	test_uri_part ("http://www.eazel.com:80", "www.eazel.com", gnome_vfs_uri_get_host_name);
	test_uri_part ("http://www.eazel.com:80", "NULL", gnome_vfs_uri_get_user_name);
	test_uri_part ("http://www.eazel.com:80", "NULL", gnome_vfs_uri_get_password);
	test_uri_part ("http://www.eazel.com:80", "/", gnome_vfs_uri_get_path);

	test_uri_host_port ("http://www.eazel.com", 0);
	test_uri_host_port ("http://www.eazel.com:80", 80);

	/* now same thing with all the parts */
	test_uri_part ("http://yakk:womble@www.eazel.com:42/blah/", "http", gnome_vfs_uri_get_scheme );
	test_uri_part ("http://yakk:womble@www.eazel.com:42/blah/", "www.eazel.com", gnome_vfs_uri_get_host_name );
	test_uri_part ("http://yakk:womble@www.eazel.com:42/blah/", "yakk", gnome_vfs_uri_get_user_name );
	test_uri_part ("http://yakk:womble@www.eazel.com:42/blah/", "womble", gnome_vfs_uri_get_password );
	test_uri_host_port ("http://yakk:womble@www.eazel.com:42/blah/", 42);
	test_uri_part ("http://yakk:womble@www.eazel.com:42/blah/", "/blah/", gnome_vfs_uri_get_path );

	test_uri_part ("http://foo.com/query?email=user@host", "/query?email=user@host", gnome_vfs_uri_get_path);
	test_uri_part ("http://user@host:42/query?email=user@host", "host", gnome_vfs_uri_get_host_name);
	test_uri_part ("http://@:/path", "NULL", gnome_vfs_uri_get_host_name);
	test_uri_part ("http://@::/path", "NULL", gnome_vfs_uri_get_host_name);
	test_uri_part ("http://::/path", "NULL", gnome_vfs_uri_get_host_name);
	test_uri_part ("http://@pass/path", "pass", gnome_vfs_uri_get_host_name);

	test_uri_parent ("", "URI NULL");
	test_uri_parent ("http://www.eazel.com", "NULL");
	test_uri_parent ("http://www.eazel.com/", "NULL");
	test_uri_parent ("http://www.eazel.com/dir", "http://www.eazel.com/");
	test_uri_parent ("http://www.eazel.com/dir/", "http://www.eazel.com/");
	test_uri_parent ("http://yakk:womble@www.eazel.com:42/blah/", "http://yakk:womble@www.eazel.com:42/");
	test_uri_parent ("file:", "NULL");
	test_uri_parent ("http:", "NULL");
	test_uri_parent ("file:/", "NULL");
	test_uri_parent ("FILE://", "NULL");
	test_uri_parent ("pipe:gnome-info2html2 as", "URI NULL");
	test_uri_parent ("file:///my/document.html#fragment", "file:///my");

	test_uri_is_parent_shallow ("file:///path", "file:///path/child", TRUE);
	test_uri_is_parent_shallow ("file:///bla", "file:///path/child", FALSE);
	test_uri_is_parent_shallow ("file:///path1/path2", "file:///path1/path2/child", TRUE);
	test_uri_is_parent_shallow ("ftp://foobar.com", "ftp://foobar.com/child", TRUE);
	test_uri_is_parent_shallow ("ftp://foobar.com/", "ftp://foobar.com/child", TRUE);
	test_uri_is_parent_shallow ("ftp://foobar.com/path", "ftp://foobar.com/path/child", TRUE);
	test_uri_is_parent_shallow ("file:///path1/path2", "file:///path1/path2/path3/path4/path5/child", FALSE);

	test_uri_is_parent_deep ("file:///path", "file:///path/child", TRUE);
	test_uri_is_parent_deep ("file:///path1/path2", "file:///path1/path2/child", TRUE);
	test_uri_is_parent_deep ("ftp://foobar.com", "ftp://foobar.com/child", TRUE);
	test_uri_is_parent_deep ("ftp://foobar.com/", "ftp://foobar.com/child", TRUE);
	test_uri_is_parent_deep ("ftp://foobar.com/path", "ftp://foobar.com/path/child", TRUE);

	test_uri_is_parent_deep ("file:///path", "file:///path/path1/child", TRUE);
	test_uri_is_parent_deep ("file:///path1/path2", "file:///path1/path2/path3/child", TRUE);
	test_uri_is_parent_deep ("file:///path1/path2", "file:///path1/path2/path3/path4/path5/child", TRUE);
	test_uri_is_parent_deep ("ftp://foobar.com", "ftp://foobar.com/path1/child", TRUE);
	test_uri_is_parent_deep ("ftp://foobar.com/", "ftp://foobar.com/path1/child", TRUE);
	test_uri_is_parent_deep ("ftp://foobar.com/path1", "ftp://foobar.com/path1/path2/child", TRUE);

	test_uri_has_parent ("", "URI NULL");
	test_uri_has_parent ("http://www.eazel.com", "FALSE");
	test_uri_has_parent ("http://www.eazel.com/", "FALSE");
	test_uri_has_parent ("http://www.eazel.com/dir", "TRUE");
	test_uri_has_parent ("http://www.eazel.com/dir/", "TRUE");
	test_uri_has_parent ("http://yakk:womble@www.eazel.com:42/blah/", "TRUE");
	test_uri_has_parent ("file:", "FALSE");
	test_uri_has_parent ("http:", "FALSE");
	test_uri_has_parent ("file:/", "FALSE");
	test_uri_has_parent ("FILE://", "FALSE");
	test_uri_has_parent ("pipe:gnome-info2html2 as", "URI NULL");

	/* Test uri canonicalization */
	test_uri_to_string ("/////", "file:///", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/.", "file:///", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/./.", "file:///", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/.///.", "file:///", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/..", "file:///", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/b/..", "file:///a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/b//..", "file:///a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/./a/b/..", "file:///a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/./b/..", "file:///a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/b/./..", "file:///a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a///b//..", "file:///a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/b/../..", "file:///", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/b/c/../..", "file:///a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/../b/..", "file:///", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/../b/../c", "file:///c", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/a/../b/../c", "file:///c", GNOME_VFS_URI_HIDE_NONE);

	test_make_canonical ("file:///%3F", "file:///%3F");
	test_make_canonical ("file:///%78", "file:///x");
	test_make_canonical ("file:///?", "file:///%3F");
	test_make_canonical ("file:///&", "file:///%26");
	test_make_canonical ("file:///x", "file:///x");
	test_make_canonical ("glorb:///%3F", "glorb:///%3F");
	test_make_canonical ("glorb:///%78", "glorb:///x");
	test_make_canonical ("glorb:///?", "glorb:///%3F");
	test_make_canonical ("glorb:///&", "glorb:///%26");
	test_make_canonical ("glorb:///x", "glorb:///x");
	test_make_canonical ("http:///%3F", "http:///%3F");
	test_make_canonical ("http:///%78", "http:///x");
	test_make_canonical ("http:///?", "http:///?");
	test_make_canonical ("http:///&", "http:///&");
	test_make_canonical ("http:///x", "http:///x");
	test_make_canonical ("pipe:foo", "pipe:foo");
	test_make_canonical ("eazel-services:///%3F", "eazel-services:///%3F");
	test_make_canonical ("eazel-services:///%78", "eazel-services:///x");
	test_make_canonical ("eazel-services:///?", "eazel-services:///?");
	test_make_canonical ("eazel-services:///&", "eazel-services:///&");
	test_make_canonical ("eazel-services:///x", "eazel-services:///x");

	test_make_canonical ("eazel-install://anonymous@/product_name=gnucash", "eazel-install://anonymous@/product_name%3Dgnucash");
	test_make_canonical ("eazel-install://:password@/product_name=gnucash", "eazel-install://:password@/product_name%3Dgnucash");

	test_make_canonical ("http://www.eazel.com/query?email=email@eazel.com", "http://www.eazel.com/query?email=email@eazel.com");

	/* test proper case-sensitivity handling */
	test_make_canonical ("HTTP://WWW.ZOO.COM", "http://www.zoo.com");
	test_make_canonical ("HTTP://WWW.ZOO.COM/", "http://www.zoo.com/");
	test_make_canonical ("HTTP://WWW.ZOO.COM/ED", "http://www.zoo.com/ED");
	test_uri_match ("http://www.zoo.com/ed", "HTTP://WWW.ZOO.COM/ed", TRUE);
	test_uri_match ("http://www.zoo.com/ed", "http://www.zoo.com/ED", FALSE);

	test_uri_match ("http://ed:ed@www.zoo.com/ed", "http://ed:ed@www.zoo.com/ed", TRUE);
	test_uri_match ("http://ed:ed@www.zoo.com/ed", "HTTP://ed:ed@www.zoo.com/ed", TRUE);
	test_uri_match ("http://ed:ed@www.zoo.com/ed", "http://ED:ed@www.zoo.com/ed", FALSE);
	test_uri_match ("http://ed:ed@www.zoo.com/ed", "http://ed:ED@www.zoo.com/ed", FALSE);
	test_uri_match ("http://ed:ed@www.zoo.com/ed", "http://ed:ed@WWW.zoo.com/ed", TRUE);
	test_uri_match ("http://ed:ed@www.zoo.com/ed", "http://ed:ed@www.ZOO.com/ed", TRUE);
	test_uri_match ("http://ed:ed@www.zoo.com/ed", "http://ed:ed@www.zoo.COM/ed", TRUE);
	test_uri_match ("http://ed:ed@www.zoo.com/ed", "http://ed:ed@www.zoo.com/ED", FALSE);

	test_uri_match ("/tmp/foo", "/tmp/foo", TRUE);
	test_uri_match ("file:/tmp/foo", "file:/TMP/foo", FALSE);
	test_uri_match ("/tmp/foo", "/TMP/foo", FALSE);

	/* Test chained uris */
	test_uri_to_string ("/tmp/t.efs#http:///foobar/", "file:///tmp/t.efs#http:/foobar/", GNOME_VFS_URI_HIDE_NONE);
	test_uri_parent ("/tmp/t.efs#http:/", "file:///tmp/t.efs");
	test_uri_to_string ("/tmp/t.efs#zip:/", "file:///tmp/t.efs#zip:/", GNOME_VFS_URI_HIDE_NONE);
	test_uri_parent ("/tmp/t.efs#zip:/", "file:///tmp/t.efs");
	test_uri_to_string ("/tmp/t.efs#unknownmethod:/", "file:///tmp/t.efs", GNOME_VFS_URI_HIDE_NONE);

	/* Test fragment identifiers. */
	test_uri_to_string ("/tmp/#junk", "file:///tmp/#junk", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("/tmp/#junk", "file:///tmp/", GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER);
	test_uri_to_string ("/tmp/#junk#", "file:///tmp/#junk#", GNOME_VFS_URI_HIDE_NONE);
	test_uri_has_fragment_id ("/tmp/#junk", "junk");
	test_uri_has_fragment_id ("/tmp/#junk#", "junk#");

	/* test a escaping->unescaping round trip for funny characters */
	test_file_path_to_uri_string ("/tmp/#backup_file#", "file:///tmp/#backup_file#", GNOME_VFS_URI_HIDE_NONE);
	test_file_path_to_uri_string ("/tmp/percent%percent", "file:///tmp/percent%percent", GNOME_VFS_URI_HIDE_NONE);

	/* FIXME bugzilla.eazel.com 4101: Why append a slash in this case, but not in the http://www.eazel.com case? */
	test_uri_to_string ("http://www.eazel.com:80", "http://www.eazel.com:80/", GNOME_VFS_URI_HIDE_NONE);

	/* FIXME bugzilla.eazel.com 3829: illegal */
	test_uri_to_string ("foo", "file:foo", GNOME_VFS_URI_HIDE_NONE);

	/* FIXME bugzilla.eazel.com 4102: illegal? */
	test_uri_to_string ("file:foo", "file:foo", GNOME_VFS_URI_HIDE_NONE);

	/* FIXME bugzilla.eazel.com 3830: This turns a good path with
	 * a redundant "/" in it into a completely different one.
	 */
	test_uri_to_string ("//foo", "file://foo", GNOME_VFS_URI_HIDE_NONE);

	/* FIXME bugzilla.eazel.com 7774: Are any of these right?
	 * Perhaps they should all return NULL?
	 */
	test_uri_to_string (".", "file:", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("./a", "file:a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("../a", "file:../a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("../a", "file:../a", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("../../a", "file:a", GNOME_VFS_URI_HIDE_NONE);

	/* FIXME bugzilla.eazel.com 2801: Do we want GnomeVFSURI to
         * just refuse to deal with URIs that we don't have a module
         * for?
	 */
	test_uri_to_string ("glorp:", "NULL", GNOME_VFS_URI_HIDE_NONE);
	test_uri_parent ("glorp:", "URI NULL");

	test_uri_to_string ("file:", "file:", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("http:", "http:", GNOME_VFS_URI_HIDE_NONE);
	test_uri_to_string ("file:/", "file:///", GNOME_VFS_URI_HIDE_NONE);

	/* FIXME bugzilla.eazel.com 6022: At least for http, we don't
         * want to do canonicalizing after the ?. All these results
         * are presumably wrong because of that.
	 */
	test_uri_to_string ("http://www.eazel.com?///xxx", "http://www.eazel.com?/xxx", GNOME_VFS_URI_HIDE_NONE);
	test_uri_parent ("http://www.eazel.com?///xxx", "http://www.eazel.com?/");
	test_uri_parent ("http://www.eazel.com/dir?xxx/yyy", "http://www.eazel.com/dir?xxx");

	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri (""), NULL);
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("/#"), NULL);
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file:/path"), "/path");
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file://path"), NULL);
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file:///path"), "/path");
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file:////path"), "//path");
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file:///my/document.html"), "/my/document.html");
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file:///my/document.html#fragment"), NULL);
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file:///my/docu%20ment%23/path"), "/my/docu ment#/path");
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("file:///my/docu%20ment%23/path/foo.html.gz#gunzip:///#fragment"), NULL);
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("/my/document.html"), NULL);
	VERIFY_STRING_RESULT (gnome_vfs_get_local_path_from_uri ("http://my/document.html"), NULL);
	
	/* Report to "make check" on whether it all worked or not. */
	return at_least_one_test_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
