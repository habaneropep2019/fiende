/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* http-authn.c - Basic authentication handling for HTTP

   Copyright (C) 2001 Eazel, Inc.

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

#include "http-authn.h"
#include "http-method.h"

#include <string.h>
#include <gnome-vfs.h>
#include <gnome-vfs-method.h>
#include <stdio.h>

/* Authentication session information
 * Key: host:port/path
 * Value: Header line including <cr><lf>
 */

static GHashTable * gl_authn_table = NULL;
static GMutex *gl_mutex = NULL;

void
http_authn_init (void)
{
	gl_authn_table = g_hash_table_new (g_str_hash, g_str_equal);
	gl_mutex = g_mutex_new ();
}

static void /* GHFunc */
hfunc_free_string (gpointer key, gpointer value, gpointer user_data)
{
	g_free (key);
	g_free (value);
}

void
http_authn_shutdown (void)
{
	g_hash_table_foreach (gl_authn_table, hfunc_free_string, NULL);
	g_hash_table_destroy (gl_authn_table);
	gl_authn_table = NULL;

	g_mutex_free (gl_mutex);
	gl_mutex = NULL;
}

/* watch it here: # and ? may be legal in the authority field of a URI
 * (eg: username:password)
 * so don't feed the authority field in here
 */
static char *
strip_uri_query_and_fragment (const char *in)
{
	char *ret;
	char *separator;

	ret = g_strdup (in);

	separator = strchr (ret, '#'); 

	if (separator) {
		*separator = '\0';
	}

	separator = strchr (ret, '?');

	if (separator) {
		*separator = '\0';
	}

	ret = g_realloc (ret, strlen (ret) + 1);
	return ret;
}

static char *
http_authn_get_key_string_from_uri (GnomeVFSURI *uri)
{
	size_t len;
	char *uri_string;
	char *uri_string_canonical;
	char *ret;

	uri_string = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_USER_NAME 
						   | GNOME_VFS_URI_HIDE_PASSWORD 
						   | GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER);

	uri_string_canonical = gnome_vfs_make_uri_canonical (uri_string);
	
	ret = strip_uri_query_and_fragment (uri_string_canonical);

	/* strip any trailing / */
	len = strlen (ret);
	if (ret [len-1] == '/') {
		ret [len-1] = '\0';
	}

	g_free (uri_string_canonical);
	uri_string_canonical = NULL;
	g_free (uri_string);
	uri_string = NULL;

	return ret;
}

char *
http_authn_session_get_header_for_uri (GnomeVFSURI *uri)
{
	char *key_string;
	char *marker;
	char *authn_header;
	char *ret;

	key_string = http_authn_get_key_string_from_uri (uri);

	g_mutex_lock (gl_mutex);

	ret = NULL;
	while (NULL != strrchr (key_string, '/')) {
		authn_header = (char *) g_hash_table_lookup (gl_authn_table, key_string);
		if (authn_header != NULL) {
			ret = g_strdup (authn_header);
			break;
		}

		/* Try the next level up in the heirarchy */
		marker = strrchr (key_string, '/');
		*marker = '\0';
	}

	g_mutex_unlock (gl_mutex);

	g_free (key_string);
	key_string = NULL;
	
	return ret;
}

void
http_authn_session_add_credentials (GnomeVFSURI *uri, const char *username, const char *password)
{
	char *key_string;
	char *orig_key;
	char *orig_value;
	char *credentials;
	char *credentials_encoded;
	
	g_return_if_fail (uri != NULL);

	key_string = http_authn_get_key_string_from_uri (uri);

	credentials = credentials_encoded = NULL;

	if (username != NULL) {
		credentials = g_strdup_printf ("%s:%s", username, 
						 password == NULL
							? "" : password);
		credentials_encoded = http_util_base64 (credentials);
	}

	g_mutex_lock (gl_mutex);

	if (g_hash_table_lookup_extended (gl_authn_table, key_string, (gpointer) &orig_key, (gpointer) &orig_value)) {
		g_hash_table_remove (gl_authn_table, orig_key);
		g_free (orig_key);
		orig_key = NULL;
		g_free (orig_value);
		orig_value = NULL;
	}

	if (credentials_encoded != NULL) {
		g_hash_table_insert (gl_authn_table, key_string, g_strdup_printf ("Authorization: Basic %s\r\n", credentials_encoded)); 
		key_string = NULL;
	}
	
	g_mutex_unlock (gl_mutex);

	g_free (key_string);
	g_free (credentials);
	g_free (credentials_encoded);
}


static gint /* GCompareFunc */
http_authn_glist_find_header (gconstpointer a, gconstpointer b)
{
	if ( NULL != a && NULL != b) {
		return strncasecmp ( (const char *)a, (const char *)b, strlen((const char *)b));
	} else {
		return -1;
	}
}

/* "quoted-string" rfc 2616 section 2.1 */
/* tolerant of lack of quotes (switches to space-delimited) */
static char *
http_authn_parse_quoted_string (const char *in, const char **p_out)
{
	gboolean quote_next;
	gboolean space_delimited;
	GString *unquoted;
	char *ret;

	ret = NULL;

	if (p_out != NULL) {
		*p_out = NULL;
	}

	if (*in != '"') {
		space_delimited = TRUE;
	} else {
		space_delimited = FALSE;
		in++;
	}

	quote_next = FALSE;
	unquoted = g_string_new ("");
	for (; *in != '\0' 
	       && (space_delimited 
	       		? (*in != ' ' && *in != '\t') 
	       		: (*in != '"' || quote_next))
	     ; in++) {
		if (!quote_next && *in == '\\') {
			quote_next = TRUE;
		} else {
			quote_next = FALSE;
			g_string_append_c (unquoted, *in);
		}
	}

	if (p_out != NULL) {
		if (*in != '\0') {
			*p_out = in+1;
		} else {
			*p_out = in;
		}
	}

	ret = unquoted->str;
	g_string_free (unquoted, FALSE);

	return ret;
}

gboolean
http_authn_parse_response_header_basic (enum AuthnHeaderType type, GList *response_headers, /* OUT */ char **p_realm)
{
	const char *header_name;
	char *header;
	const char *marker;
	GList *node;
	gboolean ret;

	g_return_val_if_fail (p_realm != NULL, FALSE);

	*p_realm = NULL;
	ret = FALSE;
	
	switch (type) {
	case AuthnHeader_WWW:
		header_name = "WWW-Authenticate:";
		break;
	
	case AuthnHeader_Proxy:
		header_name = "Proxy-Authenticate:";
		break;
	default:
		g_return_val_if_fail (FALSE, FALSE);
	}

	/* Apparently, there can be more than one authentication header
	 * (see RFC 2617 section 1.2 paragraph beginning with "Note:"
	 */
	node = g_list_find_custom (response_headers, (gpointer) header_name, http_authn_glist_find_header);
	for (; node != NULL 
	     ; node = g_list_find_custom (g_list_next (node), (gpointer) header_name, http_authn_glist_find_header)) {

		header = (char *)node->data;

		/* skip through the header name */
		marker = strchr (header, (unsigned char)':');

		if (marker == NULL) {
			continue;
		}

		marker++;

		/* skip to the auth-scheme */
		for (; *marker != '\0' 
			&& (*marker == ' ' || *marker == '\t') 
		     ; marker++);

		if (0 != strncasecmp ("Basic", marker, strlen ("Basic"))) {
			continue;
		}

		marker += strlen ("Basic");

		while (*marker != '\0') {
			for (; *marker != '\0' 
				&& (*marker == ' ' || *marker == '\t' || *marker == ',') 
			     ; marker++);

			if (0 == strncasecmp ("realm=", marker, strlen ("realm="))) {
				marker += strlen ("realm=");
				*p_realm = http_authn_parse_quoted_string (marker, &marker);
				break;
			}
		}

		if (*p_realm == NULL) {
			*p_realm = strdup ("");
		}

		ret = TRUE;
		break;
	}

	return ret;
}


char *
http_authn_get_header_for_uri (GnomeVFSURI *uri)
{
	char *result;
	GnomeVFSToplevelURI *toplevel_uri;

	toplevel_uri = gnome_vfs_uri_get_toplevel (uri);

	result = NULL;
	
	/* If authn info was passed in the URI, then default to that */
	if(toplevel_uri != NULL && toplevel_uri->user_name) {
		gchar *raw;
		gchar *enc;

		raw = g_strdup_printf("%s:%s", toplevel_uri->user_name,
				toplevel_uri->password?toplevel_uri->password:"");

		enc = http_util_base64(raw);
		
		result = g_strdup_printf("Authorization: Basic %s\r\n", enc);
		g_free(enc);
		g_free(raw);
	} else {
		result = http_authn_session_get_header_for_uri ((GnomeVFSURI *)toplevel_uri);
	}

	return result;
}

#define VERIFY_STRING_RESULT(function, expected) \
	G_STMT_START {											\
		char *result = function; 								\
		if (!((result == NULL && expected == NULL)						\
		      || (result != NULL && expected != NULL && strcmp (result, (char *)expected) == 0))) {	\
			test_failed ("%s:%s:%s: returned '%s' expected '%s'", __FILE__, __LINE__, #function, result, expected);	\
		}											\
	} G_STMT_END

static gboolean at_least_one_test_failed = FALSE;

static void
test_failed (const char *format, ...)
{
	va_list arguments;
	char *message;

	va_start (arguments, format);
	message = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	fprintf (stderr, "test failed: %s\n", message);
	at_least_one_test_failed = TRUE;
}

static void
test_parse_header (guint line, enum AuthnHeaderType type, const char *realm_expected, gboolean result_expected, ...)
{
	va_list arguments;
	GList *header_list;
	const char *header;
	char *realm;
	gboolean result;
	
	va_start (arguments, result_expected);

	header_list = NULL;
	
	for (header = va_arg (arguments, const char *) 
	     ; header != NULL
	     ; header = va_arg (arguments, const char *)) {

		header_list = g_list_prepend (header_list, (gpointer)header);
	}

	header_list = g_list_reverse (header_list);
	va_end (arguments);

	result = http_authn_parse_response_header_basic (type, header_list, &realm);

	if (! (result == result_expected
	       && ((realm == NULL && realm_expected == NULL) 
	    	    || (realm != NULL && realm_expected != NULL 
	    	    	&& 0 == strcmp (realm, realm_expected))))) {

		test_failed ("%s:%u:http_authn_parse_response_header_basic failed, expected (%d,%s) but got (%d, %s)\n",
			__FILE__, line, result_expected, realm_expected, result, realm);
	}
}

/* Hook for testing code to flush credentials */
void
http_authentication_test_flush_credentials (void);

void
http_authentication_test_flush_credentials (void)
{
	g_hash_table_foreach (gl_authn_table, hfunc_free_string, NULL);
	g_hash_table_destroy (gl_authn_table);
	gl_authn_table = g_hash_table_new (g_str_hash, g_str_equal);
}

gboolean
http_authn_self_test (void)
{
	GnomeVFSURI *uri;

	at_least_one_test_failed = FALSE;

	fprintf (stderr, "self-test: http-authn\n");
	
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("/foo/bar"), "/foo/bar");
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("/foo/bar?query"), "/foo/bar");
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("/foo/bar?query#fragment"), "/foo/bar");
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("/foo/bar#fragment"), "/foo/bar");
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("#fragment"), "");
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("?query#fragment"), "");
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("?query"), "");
	VERIFY_STRING_RESULT (strip_uri_query_and_fragment ("?query#fragment"), "");

	uri = gnome_vfs_uri_new ("http://host/path");
	VERIFY_STRING_RESULT (http_authn_get_key_string_from_uri (uri), "http://host/path");
	gnome_vfs_uri_unref (uri);

	/* FIXME I think make_uri_canonical should remove default ports */
	uri = gnome_vfs_uri_new ("http://host:80/path");
	VERIFY_STRING_RESULT (http_authn_get_key_string_from_uri (uri), "http://host:80/path");
	gnome_vfs_uri_unref (uri);

	uri = gnome_vfs_uri_new ("http://host:80/path/");
	VERIFY_STRING_RESULT (http_authn_get_key_string_from_uri (uri), "http://host:80/path");
	gnome_vfs_uri_unref (uri);

	uri = gnome_vfs_uri_new ("http://user:pass@host:80/path/?query#foo");
	VERIFY_STRING_RESULT (http_authn_get_key_string_from_uri (uri), "http://host:80/path");
	gnome_vfs_uri_unref (uri);

	VERIFY_STRING_RESULT (http_authn_parse_quoted_string ("\"quoted string\"", NULL), "quoted string");
	VERIFY_STRING_RESULT (http_authn_parse_quoted_string ("\"quoted\\\"str\\\\ing\"", NULL), "quoted\"str\\ing");
	VERIFY_STRING_RESULT (http_authn_parse_quoted_string ("unquoted-string", NULL), "unquoted-string");
	VERIFY_STRING_RESULT (http_authn_parse_quoted_string ("\"\"", NULL), "");
	VERIFY_STRING_RESULT (http_authn_parse_quoted_string ("", NULL), "");

	test_parse_header (__LINE__, AuthnHeader_WWW, "realm" , TRUE, "WWW-Authenticate: Basic realm=\"realm\"", NULL);
	test_parse_header (__LINE__, AuthnHeader_WWW, "realm" , TRUE, "WWW-Authenticate: Digest crap=\"crap\"", "WWW-Authenticate: Basic realm=\"realm\"", NULL);
	test_parse_header (__LINE__, AuthnHeader_WWW, "realm" , TRUE, "WWW-Authenticate: Basic realm=\"realm\"", "WWW-Authenticate: Digest crap=\"crap\"", NULL);
	test_parse_header (__LINE__, AuthnHeader_WWW, "" , TRUE, "WWW-Authenticate: Basic", "WWW-Authenticate: Digest crap=\"crap\"", "Proxy-Authenticate: Basic realm=\"crap\"", NULL);
	test_parse_header (__LINE__, AuthnHeader_WWW, NULL , FALSE, "WWW-Authenticate: Digest crap=\"crap\"", "Proxy-Authenticate: Basic realm=\"crap\"", NULL);

	test_parse_header (__LINE__, AuthnHeader_Proxy, "realm" , TRUE, "WWW-Authenticate: Basic", "WWW-Authenticate: Digest crap=\"crap\"", "Proxy-Authenticate: Basic realm=\"realm\"", NULL);
	test_parse_header (__LINE__, AuthnHeader_Proxy, "realm" , TRUE, "WWW-Authenticate: Basic", "WWW-Authenticate: Digest crap=\"crap\"", "proxy-authenticate: basic Realm=\"realm\"", NULL);

	uri = gnome_vfs_uri_new ("http://host/path/");

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri), NULL);

	http_authn_session_add_credentials (uri, "myuser", "mypasswd");

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri),
		g_strconcat ("Authorization: Basic ", http_util_base64("myuser:mypasswd"), "\r\n", NULL));

	gnome_vfs_uri_unref (uri);
	uri = gnome_vfs_uri_new ("http://host/path/?query#foo");

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri), 
		g_strconcat ("Authorization: Basic ", http_util_base64("myuser:mypasswd"), "\r\n", NULL));

	http_authn_session_add_credentials (uri, "newuser", "newpasswd");

	gnome_vfs_uri_unref (uri);
	uri = gnome_vfs_uri_new ("http://host/path");

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri),
		g_strconcat ("Authorization: Basic ", http_util_base64("newuser:newpasswd"), "\r\n", NULL));

	gnome_vfs_uri_unref (uri);
	uri = gnome_vfs_uri_new ("http://host/");

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri), NULL);

	gnome_vfs_uri_unref (uri);
	uri = gnome_vfs_uri_new ("http://user:passwd@host/path");

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri),
		g_strconcat ("Authorization: Basic ", http_util_base64("user:passwd"), "\r\n", NULL));

	gnome_vfs_uri_unref (uri);
	uri = gnome_vfs_uri_new ("http://anotherhost/path");

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri), NULL);

	http_authn_session_add_credentials (uri, "newuser", "newpasswd");
	http_authn_session_add_credentials (uri, NULL, NULL);

	VERIFY_STRING_RESULT (http_authn_get_header_for_uri (uri), NULL);

	return !at_least_one_test_failed;
}
