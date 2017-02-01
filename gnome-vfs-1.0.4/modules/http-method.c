/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* http-method.c - The HTTP method implementation for the GNOME Virtual File
   System.

   Copyright (C) 1999 Free Software Foundation
   Copyright (C) 2000-2001 Eazel, Inc

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
		 Ettore Perazzoli <ettore@gnu.org> (core HTTP)
		 Ian McKellar <yakk@yakk.net> (WebDAV/PUT)
		 Michael Fleming <mfleming@eazel.com> (Caching, Cleanup)
		 The friendly GNU Wget sources
	*/

/* TODO:
   - Handle redirection.
   - Handle persistent connections.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>

#include <stdlib.h> /* for atoi */

#if GNOME_PLATFORM_VERSION < 1095000
#include <gnome-xml/parser.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/xmlmemory.h>
#else
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#endif

#include <sys/time.h>


#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-mime-sniff-buffer.h"
#include "gnome-vfs-module.h"
#include "gnome-vfs-module-callback-module-api.h"
#include "gnome-vfs-standard-callbacks.h"
#include "gnome-vfs-socket-buffer.h"
#include "gnome-vfs-socket.h"
#include "gnome-vfs-ssl.h"

#include "http-method.h"
#include "http-cache.h"
#include "http-authn.h"

#define EAZEL_XML_NS "http://services.eazel.com/namespaces"

#ifdef DEBUG_HTTP_ENABLE
void
http_debug_printf (char *fmt, ...)
{
	va_list args;
	gchar * out;

	g_assert (fmt);

	va_start (args, fmt);

	out = g_strdup_vprintf (fmt, args);

	fprintf (stderr, "HTTP: [0x%08x] [0x%08x] %s\n", (unsigned int) http_util_get_utime (), (unsigned int) pthread_self(), out);

	g_free (out);
	va_end (args);
}

#define DEBUG_HTTP(x) my_debug_printf x

/* #define ANALYZE_HTTP(x) my_debug_printf (x) */
#define ANALYZE_HTTP(x) 

#endif /* DEBUG_HTTP_ENABLE */

/* What do we qualify ourselves as?  */
/* FIXME bugzilla.eazel.com 1160: "gnome-vfs/1.0.0" may not be good. */
#define USER_AGENT_STRING 	"gnome-vfs/" VERSION

/* Custom User-Agent environment variable */
#define CUSTOM_USER_AGENT_VARIABLE "GNOME_VFS_HTTP_USER_AGENT"

/* Standard HTTP[S] port.  */
#define DEFAULT_HTTP_PORT 	80
#define DEFAULT_HTTPS_PORT 	443

/* Standard HTTP proxy port */
#define DEFAULT_HTTP_PROXY_PORT 8080

/* GConf paths and keys */
#define PATH_GCONF_GNOME_VFS "/system/gnome-vfs"
#define ITEM_GCONF_HTTP_PROXY_PORT "http-proxy-port"
#define ITEM_GCONF_HTTP_PROXY_HOST "http-proxy-host"
#define KEY_GCONF_HTTP_PROXY_PORT (PATH_GCONF_GNOME_VFS "/" ITEM_GCONF_HTTP_PROXY_PORT)
#define KEY_GCONF_HTTP_PROXY_HOST (PATH_GCONF_GNOME_VFS "/" ITEM_GCONF_HTTP_PROXY_HOST)

#define ITEM_GCONF_USE_HTTP_PROXY "use-http-proxy"
#define KEY_GCONF_USE_HTTP_PROXY (PATH_GCONF_GNOME_VFS "/" ITEM_GCONF_USE_HTTP_PROXY)

#define KEY_GCONF_HTTP_AUTH_USER (PATH_GCONF_GNOME_VFS "/" "http-proxy-authorization-user")
#define KEY_GCONF_HTTP_AUTH_PW (PATH_GCONF_GNOME_VFS "/" "http-proxy-authorization-password")
#define KEY_GCONF_HTTP_USE_AUTH (PATH_GCONF_GNOME_VFS "/" "use-http-proxy-authorization")


/* Some status code validation macros.  */
#define HTTP_20X(x)        (((x) >= 200) && ((x) < 300))
#define HTTP_PARTIAL(x)    ((x) == HTTP_STATUS_PARTIAL_CONTENTS)
#define HTTP_REDIRECTED(x) (((x) == HTTP_STATUS_MOVED_PERMANENTLY)	\
			    || ((x) == HTTP_STATUS_MOVED_TEMPORARILY))

/* HTTP/1.1 status codes from RFC2068, provided for reference.  */
/* Successful 2xx.  */
#define HTTP_STATUS_OK			200
#define HTTP_STATUS_CREATED		201
#define HTTP_STATUS_ACCEPTED		202
#define HTTP_STATUS_NON_AUTHORITATIVE	203
#define HTTP_STATUS_NO_CONTENT		204
#define HTTP_STATUS_RESET_CONTENT	205
#define HTTP_STATUS_PARTIAL_CONTENTS	206

/* Redirection 3xx.  */
#define HTTP_STATUS_MULTIPLE_CHOICES	300
#define HTTP_STATUS_MOVED_PERMANENTLY	301
#define HTTP_STATUS_MOVED_TEMPORARILY	302
#define HTTP_STATUS_SEE_OTHER		303
#define HTTP_STATUS_NOT_MODIFIED	304
#define HTTP_STATUS_USE_PROXY		305

/* Client error 4xx.  */
#define HTTP_STATUS_BAD_REQUEST		400
#define HTTP_STATUS_UNAUTHORIZED	401
#define HTTP_STATUS_PAYMENT_REQUIRED	402
#define HTTP_STATUS_FORBIDDEN		403
#define HTTP_STATUS_NOT_FOUND		404
#define HTTP_STATUS_METHOD_NOT_ALLOWED	405
#define HTTP_STATUS_NOT_ACCEPTABLE	406
#define HTTP_STATUS_PROXY_AUTH_REQUIRED 407
#define HTTP_STATUS_REQUEST_TIMEOUT	408
#define HTTP_STATUS_CONFLICT		409
#define HTTP_STATUS_GONE		410
#define HTTP_STATUS_LENGTH_REQUIRED	411
#define HTTP_STATUS_PRECONDITION_FAILED	412
#define HTTP_STATUS_REQENTITY_TOO_LARGE 413
#define HTTP_STATUS_REQURI_TOO_LARGE	414
#define HTTP_STATUS_UNSUPPORTED_MEDIA	415
#define HTTP_STATUS_LOCKED		423

/* Server errors 5xx.  */
#define HTTP_STATUS_INTERNAL		500
#define HTTP_STATUS_NOT_IMPLEMENTED	501
#define HTTP_STATUS_BAD_GATEWAY		502
#define HTTP_STATUS_UNAVAILABLE		503
#define HTTP_STATUS_GATEWAY_TIMEOUT	504
#define HTTP_STATUS_UNSUPPORTED_VERSION 505
#define HTTP_STATUS_INSUFFICIENT_STORAGE 507

/*
 * Static Variables
 */

/* Global variables used by the HTTP proxy config */ 
static GConfClient * gl_client = NULL;
static GMutex *gl_mutex = NULL;		/* This mutex protects preference values
					 * and ensures serialization of authentication
					 * hook callbacks
					 */
static gchar *gl_http_proxy = NULL;
static gchar *gl_http_proxy_auth = NULL;

typedef struct {
	GnomeVFSSocketBuffer *socket_buffer;
	char *uri_string;
	GnomeVFSURI *uri;
	/* The list of headers returned with this response, newlines removed */
	GList *response_headers;

	/* File info for this file */
	GnomeVFSFileInfo *file_info;

	/* Bytes read so far.  */
	GnomeVFSFileSize bytes_read;

	/* Bytes to be written... */
	GByteArray *to_be_written;

	/* List of GnomeVFSFileInfo from a directory listing */
	GList *files;

	/* The last HTTP status code returned */
	guint server_status;
} HttpFileHandle;

static GnomeVFSResult resolve_409 		(GnomeVFSMethod *method,
						 GnomeVFSURI *uri,
						 GnomeVFSContext *context);
static void 	proxy_set_authn			(const char *username,
						 const char *password);
static void	proxy_unset_authn 		(void);
static gboolean invoke_callback_basic_authn	(HttpFileHandle *handle, 
			     			 enum AuthnHeaderType authn_which,
			     			 gboolean previous_attempt_failed);
static gboolean check_authn_retry_request 	(HttpFileHandle * http_handle,
			   			 enum AuthnHeaderType authn_which,
			   			 const char *prev_authn_header);


static GnomeVFSFileInfo *
defaults_file_info_new (void)
{
	GnomeVFSFileInfo *ret;

	/* Fill up the file info structure with default values */
	/* Default to REGULAR unless we find out later via a PROPFIND that it's a collection */

	ret = gnome_vfs_file_info_new();

	ret->type = GNOME_VFS_FILE_TYPE_REGULAR;
	ret->flags = GNOME_VFS_FILE_FLAGS_NONE;

	ret->valid_fields |= 
		GNOME_VFS_FILE_INFO_FIELDS_TYPE
			| GNOME_VFS_FILE_INFO_FIELDS_FLAGS;


	return ret;
}

static HttpFileHandle *
http_file_handle_new (GnomeVFSSocketBuffer *socket_buffer,
		      GnomeVFSURI *uri)
{
	HttpFileHandle *result;

	result = g_new0 (HttpFileHandle, 1);

	result->socket_buffer = socket_buffer;
	result->uri_string = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE );
	result->uri = uri;
	gnome_vfs_uri_ref(result->uri);

	result->file_info = defaults_file_info_new();
	result->file_info->name = gnome_vfs_uri_extract_short_name (uri);

	return result;
}

static void
http_file_handle_destroy (HttpFileHandle *handle)
{
	if (handle == NULL) {
		return;
	}

	gnome_vfs_uri_unref(handle->uri);
	gnome_vfs_file_info_unref (handle->file_info);
	g_free (handle->uri_string);
	if (handle->to_be_written) {
		g_byte_array_free(handle->to_be_written, TRUE);
	}

	g_list_foreach (handle->response_headers, (GFunc) g_free, NULL);
	g_list_free (handle->response_headers);

	g_list_foreach(handle->files, (GFunc)gnome_vfs_file_info_unref, NULL);
	g_list_free(handle->files);

	g_free (handle);
}

/* The following comes from GNU Wget with minor changes by myself.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */
/* Parse the HTTP status line, which is of format:

   HTTP-Version SP Status-Code SP Reason-Phrase

   The function returns the status-code, or -1 if the status line is
   malformed.  The pointer to reason-phrase is returned in RP.  */
static gboolean
parse_status (const char *cline,
	      guint *status_return)
{
	/* (the variables must not be named `major' and `minor', because
	   that breaks compilation with SunOS4 cc.)  */
	guint mjr, mnr;
	guint statcode;
	const guchar *p, *line;

	line = (const guchar *)cline;

	/* The standard format of HTTP-Version is: `HTTP/X.Y', where X is
	   major version, and Y is minor version.  */
	if (strncmp (line, "HTTP/", 5) == 0) {
		line += 5;
		
		/* Calculate major HTTP version.  */
		p = line;
		for (mjr = 0; isdigit (*line); line++)
			mjr = 10 * mjr + (*line - '0');
		if (*line != '.' || p == line)
			return FALSE;
		++line;
		
		/* Calculate minor HTTP version.  */
		p = line;
		for (mnr = 0; isdigit (*line); line++)
			mnr = 10 * mnr + (*line - '0');
		if (*line != ' ' || p == line)
			return -1;
		/* Wget will accept only 1.0 and higher HTTP-versions.  The value of
		   minor version can be safely ignored.  */
		if (mjr < 1)
			return FALSE;
		++line;
	} else if (strncmp (line, "ICY ", 4) == 0) {
		/* FIXME: workaround for broken ShoutCast and IceCast status replies.
		 * They send things like "ICY 200 OK" instead of "HTTP/1.0 200 OK".
		 * Is there a better way to handle this? 
		 */
		mjr = 1;
		mnr = 0;
		line += 4;
	} else {
		return FALSE;
	}
	
	/* Calculate status code.  */
	if (!(isdigit (*line) && isdigit (line[1]) && isdigit (line[2])))
		return -1;
	statcode = 100 * (*line - '0') + 10 * (line[1] - '0') + (line[2] - '0');

	*status_return = statcode;
	return TRUE;
}

static GnomeVFSResult
http_status_to_vfs_result (guint status)
{
	if (HTTP_20X (status))
		return GNOME_VFS_OK;

	/* FIXME bugzilla.eazel.com 1163 */
	/* mfleming--I've improved the situation slightly, but more
	 * test cases need to be written to ensure that HTTP (esp DAV) does compatibile
	 * things with the normal file method
	 */

	switch (status) {
	case HTTP_STATUS_PRECONDITION_FAILED:
		/* This mapping is certainly true for MOVE with Overwrite: F, otherwise not so true */
		return GNOME_VFS_ERROR_FILE_EXISTS;
	case HTTP_STATUS_UNAUTHORIZED:
	case HTTP_STATUS_PROXY_AUTH_REQUIRED:
	case HTTP_STATUS_FORBIDDEN:
		/* Note that FORBIDDEN can also be returned on a MOVE in a case which
		 * should be VFS_ERROR_BAD_PARAMETERS
		 */
		return GNOME_VFS_ERROR_ACCESS_DENIED;
	case HTTP_STATUS_NOT_FOUND:
		return GNOME_VFS_ERROR_NOT_FOUND;
	case HTTP_STATUS_METHOD_NOT_ALLOWED:
		/* Note that METHOD_NOT_ALLOWED is also returned in a PROPFIND in a case which
		 * should be FILE_EXISTS.  This is handled in do_make_directory
		 */
	case HTTP_STATUS_BAD_REQUEST:
	case HTTP_STATUS_NOT_IMPLEMENTED:
	case HTTP_STATUS_UNSUPPORTED_VERSION:
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	case HTTP_STATUS_CONFLICT:
		/* _CONFLICT's usually happen when collection paths don't exist */
		return GNOME_VFS_ERROR_NOT_FOUND;
	case HTTP_STATUS_LOCKED:
		/* Maybe we need a separate GNOME_VFS_ERROR_LOCKED? */
		return GNOME_VFS_ERROR_DIRECTORY_BUSY;
	case HTTP_STATUS_INSUFFICIENT_STORAGE:
		return GNOME_VFS_ERROR_NO_SPACE;
	default:
		return GNOME_VFS_ERROR_GENERIC;
	}
}

/* Header parsing routines.  */

static gboolean
header_value_to_number (const char *header_value,
			gulong *number)
{
	const char *p;
	gulong result;

	p = header_value;

	for (result = 0; isdigit ((unsigned char)*p); p++)
		result = 10 * result + (*p - '0');
	if (*p)
		return FALSE;

	*number = result;

	return TRUE;
}

static gboolean
set_content_length (HttpFileHandle *handle,
		    const char *value)
{
	gboolean result;
	gulong size;

	result = header_value_to_number (value, &size);
	if (! result)
		return FALSE;

	DEBUG_HTTP (("Expected size is %lu.", size));
	handle->file_info->size = size;
	handle->file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
	return TRUE;
}

static gboolean
set_content_type (HttpFileHandle *handle,
		  const char *value)
{
	char *p;

	g_free (handle->file_info->mime_type);

	if((p=strchr(value, ';')))
		handle->file_info->mime_type = g_strndup (value, p-value);
	else
		handle->file_info->mime_type = g_strdup (value);

	handle->file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
	return TRUE;
}

static gboolean
set_last_modified (HttpFileHandle *handle,
		   const char *value)
{
	time_t time;

	if (! gnome_vfs_atotm (value, &time))
		return FALSE;

	handle->file_info->mtime = time;
	handle->file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MTIME;
	return TRUE;
}

static gboolean
set_access_time (HttpFileHandle *handle,
		 const char *value)
{
	time_t time;

	if (! gnome_vfs_atotm (value, &time))
		return FALSE;

	handle->file_info->atime = time;
	handle->file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_ATIME;
	return TRUE;
}

struct _Header {
	const char *name;
	gboolean (* set_func) (HttpFileHandle *handle, const char *value);
};
typedef struct _Header Header;

static Header headers[] = {
	{ "Content-Length", set_content_length },
	{ "Content-Type", set_content_type },
	{ "Last-Modified", set_last_modified },
	{ "Date", set_access_time },
	{ NULL, NULL }
};

static const char *
check_header (const char *header,
	      const char *name)
{
	const char *p, *q;

	for (p = header, q = name; *p != '\0' && *q != '\0'; p++, q++) {
		if (tolower (*p) != tolower (*q))
			break;
	}

	if (*q != '\0' || *p != ':')
		return NULL;

	p++;			/* Skip ':'.  */
	while (*p == ' ' || *p == '\t')
		p++;

	return p;
}

static gboolean
parse_header (HttpFileHandle *handle,
	      const char *header)
{
	guint i;

	for (i = 0; headers[i].name != NULL; i++) {
		const char *value;

		value = check_header (header, headers[i].name);
		if (value != NULL)
			return (* headers[i].set_func) (handle, value);
	}

	/* Simply ignore headers we don't know.  */
	return TRUE;
}

/* Header/status reading.  */

static GnomeVFSResult
get_header (GnomeVFSSocketBuffer *socket_buffer,
	    GString *s)
{
	GnomeVFSResult result;
	GnomeVFSFileSize bytes_read;
	guint count;

	ANALYZE_HTTP ("==> +get_header");

	g_string_truncate (s, 0);

	count = 0;
	while (1) {
		char c;

		/* ANALYZE_HTTP ("==> +get_header read"); */
		result = gnome_vfs_socket_buffer_read (socket_buffer, &c, 1,
				&bytes_read);
		/* ANALYZE_HTTP ("==> -get_header read"); */

		if (result != GNOME_VFS_OK) {
			return result;
		}
		if (bytes_read == 0) {
			return GNOME_VFS_ERROR_EOF;
		}

		if (c == '\n') {
			/* Handle continuation lines.  */
			if (count != 0 && (count != 1 || s->str[0] != '\r')) {
				char next;

				result = gnome_vfs_socket_buffer_peekc (
						socket_buffer, &next);
				if (result != GNOME_VFS_OK) {
					return result;
				}
				
				if (next == '\t' || next == ' ') {
					if (count > 0
					    && s->str[count - 1] == '\r')
						s->str[count - 1] = '\0';
					continue;
				}
			}

			if (count > 0 && s->str[count - 1] == '\r')
				s->str[count - 1] = '\0';
			break;
		} else {
			g_string_append_c (s, c);
		}

		count++;
	}
	

	ANALYZE_HTTP ("==> -get_header");

	return GNOME_VFS_OK;
}

/* rename this function? */
static GnomeVFSResult
create_handle (GnomeVFSURI *uri,
	       GnomeVFSSocketBuffer *socket_buffer,
	       GnomeVFSContext *context,
	       /* OUT */ HttpFileHandle **p_handle)
{
	GString *header_string;
	GnomeVFSResult result;
	guint server_status;

	g_return_val_if_fail (p_handle != NULL, GNOME_VFS_ERROR_INTERNAL);

	*p_handle = http_file_handle_new (socket_buffer, uri);

	header_string = g_string_new (NULL);

	ANALYZE_HTTP ("==> +create_handle");

	/* This is the status report string, which is the first header.  */
	result = get_header (socket_buffer, header_string);
	if (result != GNOME_VFS_OK) {
		goto error;
	}

	if (!parse_status (header_string->str, &server_status)) {
		/* An unparsable status line is fatal */
		result = GNOME_VFS_ERROR_GENERIC;
		goto error;
	}

	(*p_handle)->server_status = server_status;

	ANALYZE_HTTP ("==> +create_handle: fetching headers");

	/* Header fetching loop.  */
	for (;;) {
		result = get_header (socket_buffer, header_string);
		if (result != GNOME_VFS_OK) {
			break;
		}

		/* Empty header ends header section.  */
		if (header_string->str[0] == '\0') {
			break;
		}

		(*p_handle)->response_headers = g_list_prepend ((*p_handle)->response_headers, 
							g_strdup (header_string->str));

		/* We don't really care if we successfully parse the
		 * header or not. It might be nice to tell someone we
		 * found a header we can't parse, but it's not clear
		 * who would be interested or how we tell them. In the
		 * past we would return NOT_FOUND if any header could
		 * not be parsed, but that seems wrong.
		 */
		parse_header (*p_handle, header_string->str);
	}

	ANALYZE_HTTP ("==> -create_handle: fetching headers");

	if (result != GNOME_VFS_OK) {
		goto error;
	}

	if (! HTTP_20X (server_status) && !HTTP_REDIRECTED(server_status)) {
		result = http_status_to_vfs_result (server_status);
		goto error;
	}

	result = GNOME_VFS_OK;
 error:
	g_string_free (header_string, TRUE);

	ANALYZE_HTTP ("==> -create_handle");
	return result;
}

/*
 * Here's how the gconf gnome-vfs HTTP proxy variables
 * are intended to be used
 *
 * /system/gnome-vfs/use-http-proxy	
 * 	Type: boolean
 *	If set to TRUE, the client should use an HTTP proxy to connect to all
 *	servers that are not "localhost".  The proxy is specified in other
 *	gconf variables below
 *
 * /system/gnome-vfs/http-proxy-host
 *	Type: string
 *	The hostname of the HTTP proxy this client should use.  If use-http-proxy
 *	is TRUE, this should be set.  If it is not set, the application should
 *	behave as if use-http-proxy is was set to FALSE.
 *
 * /system/gnome-vfs/http-proxy-port
 *	Type: int
 *	The port number on the HTTP proxy host that the client should connect to
 *	If use-http-proxy and http-proxy-host are set but this is not set, 
 *	the application should use a default port value of 8080
 *
 * /system/gnome-vfs/http-proxy-authorization-user
 *	Type: string
 *	Username to pass to an authenticating HTTP proxy.
 *
 * /system/gnome-vfs/http-proxy-authorization-password
 *	Type: string
 *	Password to pass to an authenticating HTTP proxy.
 *  
 * /system/gnome-vfs/use-http-proxy-authorization
 *	Type: boolean
 * 	TRUE if the client should pass http-proxy-authorization-user and
 *	http-proxy-authorization-password an HTTP proxy
 */

/**
 * sig_gconf_value_changed 
 * GTK signal function for when HTTP proxy GConf key has changed.
 */
static void
sig_gconf_value_changed (GConfClient* client, 
			 const char* key, 
			 GConfValue* value)
{
	if (strcmp (key, KEY_GCONF_USE_HTTP_PROXY) == 0
	    || strcmp (key, KEY_GCONF_HTTP_PROXY_HOST) == 0
	    || strcmp (key, KEY_GCONF_HTTP_PROXY_PORT) == 0) {
		gboolean use_proxy_value;
		char *proxy_host;
		int proxy_port;
		
		g_mutex_lock (gl_mutex);
		
		/* Check and see if we are using the proxy */
		use_proxy_value = gconf_client_get_bool (gl_client, KEY_GCONF_USE_HTTP_PROXY, NULL);
		proxy_host = gconf_client_get_string (gl_client, KEY_GCONF_HTTP_PROXY_HOST, NULL);
		proxy_port = gconf_client_get_int (gl_client, KEY_GCONF_HTTP_PROXY_PORT, NULL);
		
		g_free (gl_http_proxy);
		gl_http_proxy = NULL;
		
		if (use_proxy_value && proxy_host !=NULL) {
			if (0 != proxy_port && 0xffff >= (unsigned) proxy_port) {
				gl_http_proxy = g_strdup_printf ("%s:%u", proxy_host, (unsigned)proxy_port);
			} else {
				gl_http_proxy = g_strdup_printf ("%s:%u", proxy_host, (unsigned)DEFAULT_HTTP_PROXY_PORT);
			}
			DEBUG_HTTP (("New HTTP proxy: '%s'", gl_http_proxy));
		} else {
			DEBUG_HTTP (("HTTP proxy unset"));
		}
		
		g_free (proxy_host);
		proxy_host = NULL;
		
		g_mutex_unlock (gl_mutex);
	} else if (strcmp (key, KEY_GCONF_HTTP_AUTH_USER) == 0
	    || strcmp (key, KEY_GCONF_HTTP_AUTH_PW) == 0
	    || strcmp (key, KEY_GCONF_HTTP_USE_AUTH) == 0) {
		gboolean use_proxy_auth;
		char *auth_user;
		char *auth_pw;

		g_mutex_lock (gl_mutex);
		
		use_proxy_auth = gconf_client_get_bool (gl_client, KEY_GCONF_HTTP_USE_AUTH, NULL);
		auth_user = gconf_client_get_string (gl_client, KEY_GCONF_HTTP_AUTH_USER, NULL);
		auth_pw = gconf_client_get_string (gl_client, KEY_GCONF_HTTP_AUTH_PW, NULL);

		if (use_proxy_auth) {
			proxy_set_authn (auth_user, auth_pw);
			DEBUG_HTTP (("New HTTP proxy auth user: '%s'", auth_user));
		} else {
			proxy_unset_authn ();
			DEBUG_HTTP (("HTTP proxy auth unset"));
		}

		g_free (auth_user);
		g_free (auth_pw);

		g_mutex_unlock (gl_mutex);
	}
}

/**
 * host_port_from_string
 * splits a <host>:<port> formatted string into its separate components
 */
static gboolean
host_port_from_string (const char *http_proxy,
		       char **p_proxy_host, 
		       guint *p_proxy_port)
{
	char *port_part;
	
	port_part = strchr (http_proxy, ':');
	
	if (port_part && '\0' != ++port_part && p_proxy_port) {
		*p_proxy_port = (guint) strtoul (port_part, NULL, 10);
	} else if (p_proxy_port) {
		*p_proxy_port = DEFAULT_HTTP_PROXY_PORT;
	}
	
	if (p_proxy_host) {
		if ( port_part != http_proxy ) {
			*p_proxy_host = g_strndup (http_proxy, port_part - http_proxy - 1);
		} else {
			return FALSE;
		}
	}

	return TRUE;
}

static gboolean
proxy_should_for_hostname (const char *hostname)
{
	struct in_addr in, in_loop, in_mask;
	gboolean ret;

	ret = TRUE;

	/* Don't force "localhost" or 127.x.x.x through the proxy */
	/* This is a special case that we'd like to generalize into a gconf config */

	inet_aton("127.0.0.0", &in_loop); 
	inet_aton("255.0.0.0", &in_mask); 

	if (hostname != NULL 
		&& (strcasecmp (hostname, "localhost") == 0 || (inet_aton (hostname, &in) != 0
			&& ((in.s_addr & in_mask.s_addr) == in_loop.s_addr)))) {
		ret = FALSE;
	}

	return ret;
}

static char *
proxy_get_authn_header_for_uri_nolock (GnomeVFSURI * uri)
{
	char * ret;

	ret = NULL;

	/* FIXME this needs to be atomic */	
	if (gl_http_proxy_auth != NULL) {
		ret = g_strdup_printf ("Proxy-Authorization: Basic %s\r\n", gl_http_proxy_auth);
	}

	return ret;
}

static char *
proxy_get_authn_header_for_uri (GnomeVFSURI * uri)
{
	char * ret;

	g_mutex_lock (gl_mutex);

	ret = proxy_get_authn_header_for_uri_nolock (uri);

	g_mutex_unlock (gl_mutex);
	
	return ret;
}

/**
 * proxy_for_uri
 * Retrives an appropriate HTTP proxy for a given toplevel uri
 * Currently, only a single HTTP proxy is implemented (there's no way for
 * specifying non-proxy domain names's).  Returns FALSE if the connect should
 * take place directly
 */
static gboolean
proxy_for_uri (
	GnomeVFSToplevelURI * toplevel_uri,
	gchar **p_proxy_host,		/* Callee must free */
	guint *p_proxy_port)		/* Callee must free */
{
	gboolean ret;
	
	ret = proxy_should_for_hostname (toplevel_uri->host_name);

	g_mutex_lock (gl_mutex);

	if (ret && gl_http_proxy != NULL) {
		ret = host_port_from_string (gl_http_proxy, p_proxy_host, p_proxy_port);
	} else {
		p_proxy_host = NULL;
		p_proxy_port = NULL;
		ret = FALSE;
	}

	g_mutex_unlock (gl_mutex);

	return ret;
}

static void
proxy_set_authn (const char *username, const char *password)
{
	char * credentials;

	g_free (gl_http_proxy_auth);
	gl_http_proxy_auth = NULL;

	credentials = g_strdup_printf ("%s:%s", 
			username == NULL ? "" : username, 
			password == NULL ? "" : password);

	gl_http_proxy_auth = http_util_base64 (credentials);

	g_free (credentials);
}

static void
proxy_unset_authn (void)
{
	g_free (gl_http_proxy_auth);
	gl_http_proxy_auth = NULL;
}


static GnomeVFSResult
https_proxy (GnomeVFSSocket **socket_return,
	     gchar *proxy_host,
	     gint proxy_port,
	     gchar *server_host,
	     gint server_port)
{
	/* use CONNECT to do https proxying. It goes something like this:
	 * >CONNECT server:port HTTP/1.0
	 * >
	 * <HTTP/1.0 200 Connection-established
	 * <Proxy-agent: Apache/1.3.19 (Unix) Debian/GNU
	 * <
	 * and then we've got an open connection.
	 *
	 * So we sent "CONNECT server:port HTTP/1.0\r\n\r\n"
	 * Check the HTTP status.
	 * Wait for "\r\n\r\n"
	 * Start doing the SSL dance.
	 */

	GnomeVFSResult result;
	GnomeVFSInetConnection *http_connection;
	GnomeVFSSocket *http_socket;
	GnomeVFSSocket *https_socket;
	GnomeVFSSSL *ssl;
	char *buffer;
	GnomeVFSFileSize bytes;
	guint status_code;
	gint fd;

	result = gnome_vfs_inet_connection_create (&http_connection, 
			proxy_host, proxy_port, NULL);

	if (result != GNOME_VFS_OK) {
		return result;
	}

	fd = gnome_vfs_inet_connection_get_fd (http_connection);

	http_socket = gnome_vfs_inet_connection_to_socket (http_connection);

	buffer = g_strdup_printf ("CONNECT %s:%d HTTP/1.0\r\n\r\n",
			server_host, server_port);
	result = gnome_vfs_socket_write (http_socket, buffer, strlen(buffer),
			&bytes);
	g_free (buffer);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_socket_close (http_socket);
		return result;
	}

	buffer = proxy_get_authn_header_for_uri (NULL); /* FIXME need uri */
	if (buffer != NULL) {
		result = gnome_vfs_socket_write (http_socket, buffer, 
				strlen(buffer), &bytes);
		g_free (buffer);
	}

	if (result != GNOME_VFS_OK) {
		gnome_vfs_socket_close (http_socket);
		return result;
	}

	bytes = 8192;
	buffer = g_malloc0 (bytes);

	result = gnome_vfs_socket_read (http_socket, buffer, bytes-1, &bytes);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_socket_close (http_socket);
		g_free (buffer);
		return result;
	}

	if (!parse_status (buffer, &status_code)) {
		gnome_vfs_socket_close (http_socket);
		g_free (buffer);
		return GNOME_VFS_ERROR_PROTOCOL_ERROR;
	}

	result = http_status_to_vfs_result (status_code);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_socket_close (http_socket);
		g_free (buffer);
		return result;
	}

	/* okay - at this point we've read some stuff from the socket.. */
	/* FIXME: for now we'll assume thats all the headers and nothing but. */

	g_free (buffer);

	result = gnome_vfs_ssl_create_from_fd (&ssl, fd);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_socket_close (http_socket);
		return result;
	}

	https_socket = gnome_vfs_ssl_to_socket (ssl);

	*socket_return = https_socket;

	return GNOME_VFS_OK;
}



static GnomeVFSResult
connect_to_uri (
	GnomeVFSToplevelURI *toplevel_uri, 
	/* OUT */ GnomeVFSSocketBuffer **p_socket_buffer,
	/* OUT */ gboolean * p_proxy_connect)
{
	guint host_port;
	char *proxy_host;
	guint proxy_port;
	GnomeVFSResult result;
	GnomeVFSCancellation * cancellation;
	GnomeVFSInetConnection *connection;
	GnomeVFSSSL *ssl;
	GnomeVFSSocket *socket;
	gboolean https = FALSE;

	cancellation = gnome_vfs_context_get_cancellation (
				gnome_vfs_context_peek_current ());

	g_return_val_if_fail (p_socket_buffer != NULL, GNOME_VFS_ERROR_INTERNAL);
	g_return_val_if_fail (p_proxy_connect != NULL, GNOME_VFS_ERROR_INTERNAL);
	g_return_val_if_fail (toplevel_uri != NULL, GNOME_VFS_ERROR_INTERNAL);

	if (!strcasecmp (gnome_vfs_uri_get_scheme (&toplevel_uri->uri), 
				"https")) {
		if (!gnome_vfs_ssl_enabled ()) {
			return GNOME_VFS_ERROR_NOT_SUPPORTED;
		}
		https = TRUE;
	}

	if (toplevel_uri->host_port == 0) {
		if (https) {
			host_port = DEFAULT_HTTPS_PORT;
		} else {
			host_port = DEFAULT_HTTP_PORT;
		}
	} else {
		host_port = toplevel_uri->host_port;
	}

	ANALYZE_HTTP ("==> +Making connection");

	if (toplevel_uri->host_name == NULL) {
		result = GNOME_VFS_ERROR_INVALID_URI;
		goto error;
	}

	if (proxy_for_uri (toplevel_uri, &proxy_host, &proxy_port)) {
		if (https) {
			*p_proxy_connect = FALSE;

			result = https_proxy (&socket, proxy_host, proxy_port,
					toplevel_uri->host_name, host_port);

			g_free (proxy_host);
			proxy_host = NULL;

			if (result != GNOME_VFS_OK) {
				return result;
			}

		} else {
			*p_proxy_connect = TRUE;

			result = gnome_vfs_inet_connection_create (&connection,
							proxy_host,
							proxy_port, 
							cancellation);
			if (result != GNOME_VFS_OK) {
				return result;
			}
			socket = gnome_vfs_inet_connection_to_socket 
								(connection);

			g_free (proxy_host);
			proxy_host = NULL;
		}
	} else {
		*p_proxy_connect = FALSE;

		if (https) {
			result = gnome_vfs_ssl_create (&ssl, 
					toplevel_uri->host_name, host_port);

			if (result != GNOME_VFS_OK) {
				return result;
			}
			socket = gnome_vfs_ssl_to_socket (ssl);
		} else {
			result = gnome_vfs_inet_connection_create (&connection,
						   toplevel_uri->host_name,
						   host_port,
						   cancellation);
			if (result != GNOME_VFS_OK) {
				return result;
			}
			socket = gnome_vfs_inet_connection_to_socket 
								(connection);
		}
	}

	*p_socket_buffer = gnome_vfs_socket_buffer_new (socket);

	if (*p_socket_buffer == NULL) {
		gnome_vfs_socket_close (socket);
		return GNOME_VFS_ERROR_INTERNAL;
	}

	ANALYZE_HTTP ("==> -Making connection");

error:
	return result;
}

static GString *
build_request (const char * method, GnomeVFSToplevelURI * toplevel_uri, gboolean proxy_connect)
{
	gchar *uri_string = NULL;
	GString *request;
	GnomeVFSURI *uri;
	gchar *user_agent;

	uri = (GnomeVFSURI *)toplevel_uri;

	if (proxy_connect) {
		uri_string = gnome_vfs_uri_to_string (uri,
						      GNOME_VFS_URI_HIDE_USER_NAME
						      | GNOME_VFS_URI_HIDE_PASSWORD);

	} else {
		uri_string = gnome_vfs_uri_to_string (uri,
						      GNOME_VFS_URI_HIDE_USER_NAME
						      | GNOME_VFS_URI_HIDE_PASSWORD
						      | GNOME_VFS_URI_HIDE_HOST_NAME
						      | GNOME_VFS_URI_HIDE_HOST_PORT
						      | GNOME_VFS_URI_HIDE_TOPLEVEL_METHOD);
	}

	/* Request line.  */
	request = g_string_new ("");

	g_string_sprintfa (request, "%s %s%s HTTP/1.0\r\n", method, uri_string,
		strlen(gnome_vfs_uri_get_path (uri)) == 0 ? "/" : "" );

	DEBUG_HTTP (("-->Making request '%s %s'", method, uri_string));
	
	g_free (uri_string);
	uri_string = NULL;

	/* `Host:' header.  */
	if(toplevel_uri->host_port && toplevel_uri->host_port != 0) {
		g_string_sprintfa (request, "Host: %s:%d\r\n",
			   toplevel_uri->host_name, toplevel_uri->host_port);
	} else {
		g_string_sprintfa (request, "Host: %s:80\r\n",
			   toplevel_uri->host_name);
	}

	/* `Accept:' header.  */
	g_string_append (request, "Accept: */*\r\n");

	/* `User-Agent:' header.  */
	user_agent = getenv (CUSTOM_USER_AGENT_VARIABLE);

	if(user_agent == NULL) {
		user_agent = USER_AGENT_STRING;
	}

	g_string_sprintfa (request, "User-Agent: %s\r\n", user_agent);

	return request;
}

static GnomeVFSResult
xmit_request (GnomeVFSSocketBuffer *socket_buffer, 
	      GString *request, 
	      GByteArray *data)
{
	GnomeVFSResult result;
	GnomeVFSFileSize bytes_written;

	ANALYZE_HTTP ("==> Writing request and header");

	/* Transmit the request headers.  */
	result = gnome_vfs_socket_buffer_write (socket_buffer, request->str, 
			request->len, &bytes_written);

	if (result != GNOME_VFS_OK) {
		goto error;
	}

	/* Transmit the body */
	if(data && data->data) {
		ANALYZE_HTTP ("==> Writing data");
		
		result = gnome_vfs_socket_buffer_write (socket_buffer, 
				data->data, data->len, &bytes_written);
	}

	if (result != GNOME_VFS_OK) {
		goto error;
	}

	result = gnome_vfs_socket_buffer_flush (socket_buffer);	

error:
	return result;
}

static GnomeVFSResult
make_request (HttpFileHandle **handle_return,
	      GnomeVFSURI *uri,
	      const gchar *method,
	      GByteArray *data,
	      gchar *extra_headers,
	      GnomeVFSContext *context)
{
	GnomeVFSSocketBuffer *socket_buffer;
	GnomeVFSResult result;
	GnomeVFSToplevelURI *toplevel_uri;
	GString *request;
	gboolean proxy_connect;
	char *authn_header_request;
	char *authn_header_proxy;

	g_return_val_if_fail (handle_return != NULL, GNOME_VFS_ERROR_INTERNAL);
 	*handle_return = NULL;

	ANALYZE_HTTP ("==> +make_request");

	request 		= NULL;
	proxy_connect 		= FALSE;
	authn_header_request	= NULL;
	authn_header_proxy	= NULL;
	
	toplevel_uri = (GnomeVFSToplevelURI *) uri;

	for (;;) {
		g_free (authn_header_request);
		g_free (authn_header_proxy);

		socket_buffer = NULL;
		result = connect_to_uri (toplevel_uri, &socket_buffer, 
				&proxy_connect);
		
		if (result != GNOME_VFS_OK) {
			break;
		}
		
		request = build_request (method, toplevel_uri, proxy_connect);

		authn_header_request = http_authn_get_header_for_uri (uri);

		if (authn_header_request != NULL) {
			g_string_append (request, authn_header_request);
		}

		if (proxy_connect) {
			authn_header_proxy = proxy_get_authn_header_for_uri (uri);

			if (authn_header_proxy != NULL) {
				g_string_append (request, authn_header_proxy);
			}
		}
		
		/* `Content-Length' header.  */
		if (data != NULL) {
			g_string_sprintfa (request, "Content-Length: %d\r\n", data->len);
		}
		
		/* Extra headers. */
		if (extra_headers != NULL) {
			g_string_append(request, extra_headers);
		}

		/* Empty line ends header section.  */
		g_string_append (request, "\r\n");

		result = xmit_request (socket_buffer, request, data);
		g_string_free (request, TRUE);
		request = NULL;

		if (result != GNOME_VFS_OK) {
			break;
		}

		/* Read the headers and create our internal HTTP file handle.  */
		result = create_handle (uri, socket_buffer, context, handle_return);
		socket_buffer = NULL;

		if (result == GNOME_VFS_OK) {
			break;
		}

		if ((*handle_return)->server_status == HTTP_STATUS_UNAUTHORIZED) {
			if (! check_authn_retry_request (*handle_return, AuthnHeader_WWW, authn_header_request)) {
				break;
			}
		} else if ((*handle_return)->server_status == HTTP_STATUS_PROXY_AUTH_REQUIRED) {
			if (! check_authn_retry_request (*handle_return, AuthnHeader_WWW, authn_header_proxy)) {
				break;
			}
		} else {
			break;
		}
		http_file_handle_destroy (*handle_return);
		*handle_return = NULL;
	}

	g_free (authn_header_request);
	g_free (authn_header_proxy);

	if (result != GNOME_VFS_OK && *handle_return != NULL) {
		http_file_handle_destroy (*handle_return);
		*handle_return = NULL;
	}

 	if (request != NULL) {
		g_string_free (request, TRUE);
	}
	
	if (socket_buffer != NULL) {
		gnome_vfs_socket_buffer_destroy (socket_buffer, TRUE);
	}
	
	ANALYZE_HTTP ("==> -make_request");
	return result;
}

static void
http_handle_close (HttpFileHandle *handle, 
		   GnomeVFSContext *context)
{
	ANALYZE_HTTP ("==> +http_handle_close");
	
	if (handle != NULL) {
		if (handle->socket_buffer) {
			gnome_vfs_socket_buffer_flush (handle->socket_buffer);
			gnome_vfs_socket_buffer_destroy (handle->socket_buffer,
							 TRUE);
			handle->socket_buffer = NULL;
		}

		http_file_handle_destroy (handle);
	}
	
	ANALYZE_HTTP ("==> -http_handle_close");
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	HttpFileHandle *handle;
	GnomeVFSResult result = GNOME_VFS_OK;
	
	g_return_val_if_fail (uri->parent == NULL, GNOME_VFS_ERROR_INVALID_URI);
	g_return_val_if_fail (!(mode & GNOME_VFS_OPEN_READ && 
				mode & GNOME_VFS_OPEN_WRITE),
			      GNOME_VFS_ERROR_INVALID_OPEN_MODE);

	ANALYZE_HTTP ("==> +do_open");
	DEBUG_HTTP (("+Open URI: '%s' mode:'%c'", gnome_vfs_uri_to_string(uri, 0), 
		     mode & GNOME_VFS_OPEN_READ ? 'R' : 'W'));
	
	if (mode & GNOME_VFS_OPEN_READ) {
		result = make_request (&handle, uri, "GET", NULL, NULL,
				       context);
	} else {
		handle = http_file_handle_new(NULL, uri); /* shrug */
	}
	if (result == GNOME_VFS_OK) {
		*method_handle = (GnomeVFSMethodHandle *) handle;
	} else {
		*method_handle = NULL;
	}

	DEBUG_HTTP (("-Open (%d) handle:0x%08x", result, (unsigned int)handle));
	ANALYZE_HTTP ("==> -do_open");
	
	return result;
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
	/* try to write a zero length file - this appears to be the 
	 * only reliable way of testing if a put will succeed. 
	 * Xythos can apparently tell us if we have write permission by
	 * playing with LOCK, but mod_dav cannot. */
	HttpFileHandle *handle;
	GnomeVFSResult result;
	GByteArray *bytes = g_byte_array_new();
	
	ANALYZE_HTTP ("==> +do_create");
	DEBUG_HTTP (("+Create URI: '%s'", gnome_vfs_uri_get_path (uri)));

	http_cache_invalidate_uri_parent (uri);

	/* Don't ignore exclusive; it should check first whether
	   the file exists, since the http protocol default is to 
	   overwrite by default */
	/* FIXME we've stopped using HEAD -- we should use GET instead  */
	/* FIXME we should check the cache here */
	if (exclusive) {
		
		ANALYZE_HTTP ("==> Checking to see if file exists");
		
		result = make_request (&handle, uri, "HEAD", NULL, NULL,
				       context);
		http_handle_close (handle, context);
		
		if (result != GNOME_VFS_OK &&
		    result != GNOME_VFS_ERROR_NOT_FOUND) {
			return result;
		}
		if (result == GNOME_VFS_OK) {
			return GNOME_VFS_ERROR_FILE_EXISTS;
		}
	}
	
	ANALYZE_HTTP ("==> Creating initial file");
	
      	result = make_request (&handle, uri, "PUT", bytes, NULL, context);
	http_handle_close(handle, context);
	
	if (result != GNOME_VFS_OK) {
		/* the PUT failed */
		
		/* FIXME bugzilla.eazel.com 5131
		 * If you PUT a file with an invalid name to Xythos, it 
		 * returns a 403 Forbidden, which is different from the behaviour
		 * in MKCOL or MOVE.  Unfortunately, it is not possible to discern whether 
		 * that 403 Forbidden is being returned because of invalid characters in the name
		 * or because of permissions problems
		 */  

		if (result == GNOME_VFS_ERROR_NOT_FOUND) {
			result = resolve_409 (method, uri, context);
		}

		return result;
	}

	/* clean up */
	g_byte_array_free (bytes, TRUE);
	
	/* FIXME bugzilla.eazel.com 1159: do we need to do something more intelligent here? */
	result = do_open (method, method_handle, uri, GNOME_VFS_OPEN_WRITE, context);

	DEBUG_HTTP (("-Create (%d) handle:0x%08x", result, (unsigned int)handle));
	ANALYZE_HTTP ("==> -do_create");

	return result;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	HttpFileHandle *old_handle;
	HttpFileHandle *new_handle;
	GnomeVFSResult result;
	
	ANALYZE_HTTP ("==> +do_close");
	DEBUG_HTTP (("+Close handle:0x%08x", (unsigned int)method_handle));

	old_handle = (HttpFileHandle *) method_handle;
	
	/* if the handle was opened in write mode then:
	 * 1) there won't be a connection open, and
	 * 2) there will be data to_be_written...
	 */
	if (old_handle->to_be_written != NULL) {
		GnomeVFSURI *uri = old_handle->uri;
		GByteArray *bytes = old_handle->to_be_written;
		GnomeVFSMimeSniffBuffer *sniff_buffer;
		char *extraheader = NULL;
		const char *mime_type = NULL;
		
		sniff_buffer = 
			gnome_vfs_mime_sniff_buffer_new_from_existing_data (bytes->data, 
									    bytes->len);

		if (sniff_buffer != NULL) {
			mime_type = 
				gnome_vfs_get_mime_type_for_buffer (
						sniff_buffer);
			if (mime_type != NULL) {
				extraheader = g_strdup_printf(
						"Content-type: %s\r\n", 
						mime_type);
			}
			gnome_vfs_mime_sniff_buffer_free (sniff_buffer);

		}

		http_cache_invalidate_uri (uri);

		ANALYZE_HTTP ("==> doing PUT");
		result = make_request (&new_handle, uri, "PUT", bytes, 
				       extraheader, context);
		g_free (extraheader);
		http_handle_close (new_handle, context);
	} else {
		result = GNOME_VFS_OK;
	}
	
	http_handle_close (old_handle, context);
	
	DEBUG_HTTP (("-Close (%d)", result));
	ANALYZE_HTTP ("==> -do_close");
	
	return result;
}
	
static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_read,
	  GnomeVFSContext *context)
{
	HttpFileHandle *handle;

	DEBUG_HTTP (("+Write handle:0x%08x", (unsigned int)method_handle));

	handle = (HttpFileHandle *) method_handle;

	if(handle->to_be_written == NULL) {
		handle->to_be_written = g_byte_array_new();
	}
	handle->to_be_written = g_byte_array_append(handle->to_be_written, buffer, num_bytes);
	*bytes_read = num_bytes;
	
	DEBUG_HTTP (("-Write (0)"));
	
	return GNOME_VFS_OK;
}


static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	HttpFileHandle *handle;
	GnomeVFSResult result;

	ANALYZE_HTTP ("==> +do_read");
	DEBUG_HTTP (("+Read handle=0x%08x", (unsigned int) method_handle));

	handle = (HttpFileHandle *) method_handle;

	if (handle->file_info->flags & GNOME_VFS_FILE_INFO_FIELDS_SIZE) {
		GnomeVFSFileSize max_bytes;

		max_bytes = handle->file_info->size - handle->bytes_read;
		num_bytes = MIN (max_bytes, num_bytes);
	}

	result = gnome_vfs_socket_buffer_read (handle->socket_buffer, buffer, 
			num_bytes, bytes_read);
	
	if (*bytes_read == 0) {
		return GNOME_VFS_ERROR_EOF;
	}				       

	handle->bytes_read += *bytes_read;

	DEBUG_HTTP (("-Read (%d)", result));
	ANALYZE_HTTP ("==> -do_read");

	return result;
}

/* Directory handling - WebDAV servers only */

static void
process_propfind_propstat (xmlNodePtr node, 
			   GnomeVFSFileInfo *file_info)
{
	xmlNodePtr l;
	gboolean treat_as_directory;

	treat_as_directory = FALSE;

	while (node != NULL) {
		if (strcmp ((char *)node->name, "prop") != 0) {
			/* node name != "prop" - prop is all we care about */
			node = node->next;
			continue;
		}
		/* properties of the file */
		l = node->xmlChildrenNode;
		while (l != NULL) {
			char *node_content_xml = xmlNodeGetContent(l);
			if (node_content_xml) {
				if (strcmp ((char *)l->name, "getcontenttype") == 0) {
					file_info->valid_fields |= 
						GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
					
					if (!file_info->mime_type) {
						file_info->mime_type = g_strdup(node_content_xml);
					}
				} else if (strcmp ((char *)l->name, "getcontentlength") == 0){
					file_info->valid_fields |= 
						GNOME_VFS_FILE_INFO_FIELDS_SIZE;
					file_info->size = atoi(node_content_xml);
				} else if (strcmp((char *)l->name, "getlastmodified") == 0) {
					if (gnome_vfs_atotm (node_content_xml, &(file_info->mtime))) {
						file_info->ctime = file_info->mtime;
						file_info->valid_fields |= 
							GNOME_VFS_FILE_INFO_FIELDS_MTIME 
							| GNOME_VFS_FILE_INFO_FIELDS_CTIME;
					}
				} else if (strcmp ((char *)l->name, "nautilus-treat-as-directory") == 0
					   && l->ns != NULL && l->ns->href != NULL
					   && strcmp (l->ns->href, EAZEL_XML_NS) == 0
					   && strcasecmp (node_content_xml, "TRUE") == 0) {
					treat_as_directory = TRUE;
				}
				/* Unfortunately, we don't have a mapping for "creationdate" */

				xmlFree (node_content_xml);
				node_content_xml = NULL;
			}
			if (strcmp ((char *)l->name, "resourcetype") == 0) {
				file_info->valid_fields |= 
					GNOME_VFS_FILE_INFO_FIELDS_TYPE;
				file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
				
				if (l->xmlChildrenNode && l->xmlChildrenNode->name 
				    && strcmp ((char *)l->xmlChildrenNode->name, "collection") == 0) {
					file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
				}
			}
			l = l->next;
		}
		node = node->next;
	}
	
	/* If this is a DAV collection, do we tell nautilus to treat it
	 * as a directory or as a web page?
	 */
	if (file_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE
	    && file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
		g_free (file_info->mime_type);
		if (treat_as_directory) {
			file_info->mime_type = g_strdup ("x-directory/webdav-prefer-directory");
			file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		} else {
			file_info->mime_type = g_strdup ("x-directory/webdav");
			file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	}
	
	
	if ((file_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE) == 0) {
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		file_info->mime_type = g_strdup (gnome_vfs_mime_type_from_name_or_default (file_info->name, "text/plain"));
	}

	if ((file_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE) == 0) {
		/* Is this a reasonable assumption ? */
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
	}
}

/* The problem here: Eazel Vault returns "https:" URI's which GnomeVFS doesn't recognize
 * So, if it's an https: uri scheme, just change it to "http" and we'll all be happy
 */
static GnomeVFSURI *
propfind_href_to_vfs_uri (const char *propfind_href_uri)
{
#ifdef THIS_IS_EVIL_AND_NOBODY_NEEDS_IT_ANY_MORE
	size_t https_len = strlen ("https:");
	GnomeVFSURI *ret;

	if (strncmp (propfind_href_uri, "https:", https_len) == 0) {
		char *new_uri;
		new_uri = g_strconcat ("http:", https_len + propfind_href_uri, NULL);
		ret = gnome_vfs_uri_new (new_uri);
		g_free (new_uri);
	} else {
		ret = gnome_vfs_uri_new (propfind_href_uri);
	}
	
	return ret;
#else
	return gnome_vfs_uri_new (propfind_href_uri);
#endif
}

/* a strcmp that doesn't barf on NULLs */
static gint
null_handling_strcmp (const char *a, const char *b) 
{
	if ((a == NULL) != (b == NULL)) {
		return 1;
	}
	
	if (a == NULL && b == NULL) {
		return 0;
	}
	
	return strcmp (a, b);
}

#if 0
static unsigned char
unhex_char (char to_unhex)
{
	unsigned char ret = 0;
	
	if (to_unhex >= 'A' && to_unhex <= 'F') {	
		ret = to_unhex - 'A' + 0xA;	
	} else if (to_unhex >= 'a' && to_unhex <= 'f') {
		ret = to_unhex - 'a' + 0xa;	
	} else if (to_unhex >= '0' && to_unhex <= '9') {
		ret = to_unhex - '0';
	}
	
	return ret;
}

static char *
unescape_unreserved_chars (const char *in_string)
{
	/* RFC 2396 section 2.2 */
	static const char * reserved_chars = "%;/?:@&=+$,";

	char *ret, *write_char;
	const char * read_char;

	if (in_string == NULL) {
		return NULL;
	}
	
	ret = g_new (char, strlen (in_string) + 1);

	for (read_char = in_string, write_char = ret ; *read_char != '\0' ; read_char++) {
		if (read_char[0] == '%' 
		    && isxdigit ((unsigned char) read_char[1]) 
		    && isxdigit ((unsigned char) read_char[2])) {
			char unescaped;
			
			unescaped = (unsigned char) ((unhex_char (read_char[1]) << 4) | unhex_char (read_char[2]));
			
			if (strchr (reserved_chars, (unsigned char)unescaped)) {
				*write_char++ = *read_char++;
				*write_char++ = *read_char++;
				*write_char++ = *read_char; /*The last ++ is done in the for statement */
			} else {
				*write_char++ = unescaped;
				read_char += 2; /*The last ++ is done in the for statement */ 
			}
		} else {
			*write_char++ = *read_char;
		}
	}
	*write_char++ = '\0';
	
	if (strlen (in_string) != strlen (ret)) {
		DEBUG_HTTP (("unescape_unreserved from '%s' to '%s'"));
	}
	
	
	return ret;
}

#endif /* 0 */

static xmlNodePtr
find_child_node_named (xmlNodePtr node, 
		       const char *child_node_name)
{
	xmlNodePtr child;

	child = node->xmlChildrenNode;

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		if (0 == strcmp (child->name, child_node_name)) {
			return child;
		}
	}

	return NULL;
}

static guint
get_propstat_status (xmlNodePtr propstat_node, 
		     guint *p_status_code)
{
	xmlNodePtr status_node;
	char *status_string;
	gboolean ret;

	status_node = find_child_node_named (propstat_node, "status");

	if (status_node != NULL) {
		status_string =	xmlNodeGetContent (status_node);
		ret = parse_status (status_string, p_status_code);
		xmlFree (status_string);
	} else {
		ret = FALSE;
	}
	
	return ret;
}

static GnomeVFSFileInfo *
process_propfind_response(xmlNodePtr n,
			  GnomeVFSURI *base_uri)
{
	GnomeVFSFileInfo *file_info = defaults_file_info_new ();
	GnomeVFSURI *second_base = gnome_vfs_uri_append_path (base_uri, "/");
	guint status_code;
	
	file_info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE;
	
	while (n != NULL) {
		if (strcmp ((char *)n->name, "href") == 0) {
			char *nodecontent = xmlNodeGetContent (n);
			GnomeVFSResult rv;
			
			rv = gnome_vfs_remove_optional_escapes (nodecontent);
			
			if (nodecontent != NULL && *nodecontent != '\0' && rv == GNOME_VFS_OK) {
				gint len;
				GnomeVFSURI *uri = propfind_href_to_vfs_uri (nodecontent);
				
				if (uri != NULL) {
					if ((0 == null_handling_strcmp (base_uri->text, uri->text)) ||
					    (0 == null_handling_strcmp (second_base->text, uri->text))) {
						file_info->name = NULL; /* this file is the . directory */
					} else {
						/* extract_short_name returns unescaped */
						file_info->name = gnome_vfs_uri_extract_short_name (uri);
						gnome_vfs_uri_unref (uri);
						
						len = strlen (file_info->name) -1;
						if (file_info->name[len] == '/') {
							/* trim trailing `/` - it confuses stuff */
							file_info->name[len] = '\0';
						}
					}
				} else {
					g_warning ("Can't make URI from href in PROPFIND '%s'; silently skipping", nodecontent);
				}
			} else {
				g_warning ("got href without contents in PROPFIND response");
			}

			xmlFree (nodecontent);
		} else if (strcmp ((char *)n->name, "propstat") == 0) {
			if (get_propstat_status (n, &status_code) && status_code == 200) {
				process_propfind_propstat (n->xmlChildrenNode, file_info);
			}
		}
		n = n->next;
	}

	gnome_vfs_uri_unref (second_base);

	return file_info;
}



static GnomeVFSResult
make_propfind_request (HttpFileHandle **handle_return,
	GnomeVFSURI *uri,
	gint depth,
	GnomeVFSContext *context)
{
	GnomeVFSResult result = GNOME_VFS_OK;
	GnomeVFSFileSize bytes_read, num_bytes=(64*1024);
	char *buffer = g_malloc(num_bytes);
	xmlParserCtxtPtr parserContext;
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	char *extraheaders = g_strdup_printf("Depth: %d\r\n", depth);
	gboolean found_root_node_props;

	GByteArray *request = g_byte_array_new();
	char *request_str = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
		"<D:propfind xmlns:D=\"DAV:\" xmlns:ns1000=\"" EAZEL_XML_NS "\">"
		"<D:prop>"
                "<D:creationdate/>"
                "<D:getcontentlength/>"
                "<D:getcontenttype/>"
                "<D:getlastmodified/>"
                "<D:resourcetype/>"
                "<ns1000:nautilus-treat-as-directory/>"
		/*"<D:allprop/>"*/
		"</D:prop>"
		"</D:propfind>";

	ANALYZE_HTTP ("==> +make_propfind_request");

	request = g_byte_array_append(request, request_str, 
			strlen(request_str));

	parserContext = xmlCreatePushParserCtxt(NULL, NULL, "", 0, "PROPFIND");

	if (depth > 0) {
		http_cache_invalidate_uri_and_children (uri);
	}

	result = make_request (handle_return, uri, "PROPFIND", request, 
			       extraheaders, context);
	
	/* FIXME bugzilla.eazel.com 3834: It looks like some http
	 * servers (eg, www.yahoo.com) treat PROPFIND as a GET and
	 * return a 200 OK. Others may return access denied errors or
	 * redirects or any other legal response. This case probably
	 * needs to be made more robust.
	 */
	if (result == GNOME_VFS_OK && (*handle_return)->server_status != 207) { /* Multi-Status */
		DEBUG_HTTP (("HTTP server returned an invalid PROPFIND response: %d", (*handle_return)->server_status));
		result = GNOME_VFS_ERROR_NOT_SUPPORTED;
	}
	
	if (result == GNOME_VFS_OK) {
		do {
			result = do_read (NULL, (GnomeVFSMethodHandle *) *handle_return,
					  buffer, num_bytes, &bytes_read, context);
			
			if (result != GNOME_VFS_OK ) {
				break;
			}
			
			xmlParseChunk (parserContext, buffer, bytes_read, 0);
			buffer[bytes_read]=0;
		} while (bytes_read > 0);
	}

	if (result == GNOME_VFS_ERROR_EOF) {
		result = GNOME_VFS_OK;
	}
	
	if (result != GNOME_VFS_OK) {
		goto cleanup;
	}
	
	xmlParseChunk (parserContext, "", 0, 1);

	doc = parserContext->myDoc;
	if (doc == NULL) {
		result = GNOME_VFS_ERROR_GENERIC;
		goto cleanup;
	}
	
	cur = doc->xmlRootNode;
	
	if (strcmp ((char *)cur->name, "multistatus") != 0) {
		DEBUG_HTTP (("Couldn't find <multistatus>.\n"));
		result = GNOME_VFS_ERROR_GENERIC;
		goto cleanup;
	}

	cur = cur->xmlChildrenNode;
	
	found_root_node_props = FALSE;
	while (cur != NULL) {
		if (strcmp ((char *)cur->name, "response") == 0) {
			GnomeVFSFileInfo *file_info =
				process_propfind_response (cur->xmlChildrenNode, uri);
			
			if (file_info->name != NULL) { 
				(*handle_return)->files = g_list_append ((*handle_return)->files, file_info);
			} else {
				/* This response refers to the root node */
				/* Abandon the old information that came from create_handle*/
				
				file_info->name = (*handle_return)->file_info->name;
				(*handle_return)->file_info->name = NULL;
				gnome_vfs_file_info_unref ((*handle_return)->file_info);
				(*handle_return)->file_info = file_info;
				found_root_node_props = TRUE;
			}
			
		} else {
			DEBUG_HTTP(("expecting <response> got <%s>\n", cur->name));
		}
		cur = cur->next;
	}
	
	if (!found_root_node_props) {
		DEBUG_HTTP (("Failed to find root request node properties during propfind"));
		result = GNOME_VFS_ERROR_GENERIC;
		goto cleanup;
	}

	/*
	 * RFC 2518
	 * Section 8.1, final line
	 * "The results of this method [PROPFIND] SHOULD NOT be cached"
	 * Well, at least its not "MUST NOT"
	 */
	
	if (depth == 0) {
		http_cache_add_uri (uri, (*handle_return)->file_info, TRUE);
	} else {
		http_cache_add_uri_and_children (uri, (*handle_return)->file_info, (*handle_return)->files);
	}

cleanup:
	g_free(buffer);
	g_free(extraheaders);
	xmlFreeParserCtxt(parserContext);
	
	if (result != GNOME_VFS_OK) {
		http_handle_close (*handle_return, context);
		*handle_return = NULL;
	}
	
	ANALYZE_HTTP ("==> -make_propfind_request");
	
	return result;
}

static GnomeVFSResult
do_open_directory(GnomeVFSMethod *method,
		  GnomeVFSMethodHandle **method_handle,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfoOptions options,
		  const GnomeVFSDirectoryFilter *filter,
		  GnomeVFSContext *context) 
{
	/* TODO move to using the gnome_vfs_file_info_list family of functions */
	GnomeVFSResult result;
	HttpFileHandle *handle = NULL;
	GnomeVFSFileInfo * file_info_cached;
	GList *child_file_info_cached_list = NULL;

	ANALYZE_HTTP ("==> +do_open_directory");
	DEBUG_HTTP (("+Open_Directory options: %d dirfilter: 0x%08x URI: '%s'", options, (unsigned int) filter, gnome_vfs_uri_to_string( uri, 0)));

	/* Check the cache--is this even a directory?  
	 * (Nautilus, in particular, seems to like to make this call on non directories
	 */

	file_info_cached = http_cache_check_uri (uri);

	if (file_info_cached) {
		if (GNOME_VFS_FILE_TYPE_DIRECTORY != file_info_cached->type) {
			ANALYZE_HTTP ("==> Cache Hit (Negative)");	
			gnome_vfs_file_info_unref (file_info_cached);
			result = GNOME_VFS_ERROR_NOT_A_DIRECTORY;
			goto error;
		}
		gnome_vfs_file_info_unref (file_info_cached);
		file_info_cached = NULL;
	}

	
	/* The check for directory contents is more stringent */
	file_info_cached = http_cache_check_directory_uri (uri, &child_file_info_cached_list);

	if (file_info_cached) {
		handle = http_file_handle_new (NULL, uri);
		handle->file_info = file_info_cached;
		handle->files = child_file_info_cached_list;
		result = GNOME_VFS_OK;
	} else {
		result = make_propfind_request(&handle, uri, 1, context);
		/* mfleming -- is this necessary?  Most DAV server's I've seen don't have the horrible
		 * lack-of-trailing-/-is-a-301 problem for PROPFIND's
		 */
		if (result == GNOME_VFS_ERROR_NOT_FOUND) { /* 404 not found */
			if (uri->text != NULL && *uri->text != '\0'
			   && uri->text[strlen (uri->text) - 1] != '/') {
				GnomeVFSURI *tmpuri = gnome_vfs_uri_append_path (uri, "/");
				result = do_open_directory (method, (GnomeVFSMethodHandle **)&handle, tmpuri, options, filter, context);
				gnome_vfs_uri_unref (tmpuri);

			}
		}

		if (result == GNOME_VFS_OK
		    && handle->file_info->type != GNOME_VFS_FILE_TYPE_DIRECTORY) {
			result = GNOME_VFS_ERROR_NOT_A_DIRECTORY;
			http_handle_close (handle, context);
			handle = NULL;
		}
	}
	
	*method_handle = (GnomeVFSMethodHandle *)handle;

error:
	DEBUG_HTTP (("-Open_Directory (%d) handle:0x%08x", result, (unsigned int)handle));
	ANALYZE_HTTP ("==> -do_open_directory");
	
	return result;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context) 
{
	HttpFileHandle *handle;
	
	DEBUG_HTTP (("+Close_Directory"));
	
	handle = (HttpFileHandle *) method_handle;
	
	http_handle_close(handle, context);

	DEBUG_HTTP (("-Close_Directory (0) handle:0x%08x", (unsigned int) method_handle));

	return GNOME_VFS_OK;
}
       
static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
       GnomeVFSMethodHandle *method_handle,
       GnomeVFSFileInfo *file_info,
       GnomeVFSContext *context)
{
	HttpFileHandle *handle;
	GnomeVFSResult result;

	DEBUG_HTTP (("+Read_Directory handle:0x%08x", (unsigned int) method_handle));

	handle = (HttpFileHandle *) method_handle;
	
	if (handle->files && g_list_length (handle->files)) {
		GnomeVFSFileInfo *original_info = g_list_nth_data (handle->files, 0);
		gboolean found_entry = FALSE;
		
		/* mfleming -- Why is this check here?  Does anyone set original_info->name to NULL? */
		if (original_info->name != NULL && original_info->name[0]) {
			gnome_vfs_file_info_copy (file_info, original_info); 
			found_entry = TRUE;
		}
		
		/* remove our GnomeVFSFileInfo from the list */
		handle->files = g_list_remove (handle->files, original_info);
		gnome_vfs_file_info_unref (original_info);
	
		/* mfleming -- Is this necessary? */
		if (found_entry) {
			result = GNOME_VFS_OK;
		} else {
			result = do_read_directory (method, method_handle, file_info, context);
		}
	} else {
		result = GNOME_VFS_ERROR_EOF;
	}

	DEBUG_HTTP (("-Read_Directory (%d)", result));
	return result;
}
 

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
	HttpFileHandle *handle;
	GnomeVFSResult result;
	GnomeVFSFileInfo * file_info_cached;

	ANALYZE_HTTP ("==> +do_get_file_info");
	DEBUG_HTTP (("+Get_File_Info options: %d URI: '%s'", options, gnome_vfs_uri_to_string( uri, 0)));

	file_info_cached = http_cache_check_uri (uri);

	if (file_info_cached != NULL) {
		gnome_vfs_file_info_copy (file_info, file_info_cached);
		gnome_vfs_file_info_unref (file_info_cached);
		ANALYZE_HTTP ("==> Cache Hit");	
		result = GNOME_VFS_OK;
	} else {
		/*
		 * Start off by making a PROPFIND request.  Fall back to a HEAD if it fails
		 */
		
		result = make_propfind_request (&handle, uri, 0, context);
		
		/* Note that theoretically we could not bother with this request if we get a 404 back,
		 * but since some servers seem to return wierd things on PROPFIND (mostly 200 OK's...)
		 * I'm not going to count on the PROPFIND response....
		 */ 
		if (result == GNOME_VFS_OK) {
			gnome_vfs_file_info_copy (file_info, handle->file_info);
			http_handle_close (handle, context);
			handle = NULL;
		} else {
			g_assert (handle == NULL); /* Make sure we're not leaking some old one */
			
			/* Lame buggy servers (eg: www.mozilla.org,
			 * www.corel.com)) return an HTTP error for a
			 * HEAD where a GET would succeed. In these
			 * cases lets try to do a GET.
			 */
			if (result != GNOME_VFS_OK) {
				g_assert (handle == NULL); /* Make sure we're not leaking some old one */

				ANALYZE_HTTP ("==> do_get_file_info: do GET ");

				result = make_request (&handle, uri, "GET", NULL, NULL, context);
				if (result == GNOME_VFS_OK) {
					gnome_vfs_file_info_copy (file_info, handle->file_info);
					http_cache_add_uri (uri, handle->file_info, FALSE);
					http_handle_close (handle, context);
				}

				/* If we get a redirect, we should be
				 * basing the MIME type on the type of
				 * the page we'll be redirected
				 * too. Maybe we even want to take the
				 * "follow_links" setting into account.
				 */
				/* FIXME: For now we treat all
				 * redirects as if they lead to a
				 * text/html. That works pretty well,
				 * but it's not correct.
				 */
				if (handle != NULL && HTTP_REDIRECTED (handle->server_status)) {
					g_free (file_info->mime_type);
					file_info->mime_type = g_strdup ("text/html");
				}
			}
			
			if (result == GNOME_VFS_ERROR_NOT_FOUND) { /* 404 not found */
				/* FIXME bugzilla.eazel.com 3835: mfleming: Is this code really appropriate?
				 * In any case, it doesn't seem to be appropriate for a DAV-enabled
				 * server, since they don't seem to send 301's when you PROPFIND collections
				 * without a trailing '/'.
				 */
				if (uri->text != NULL && *uri->text != '\0' 
				    && uri->text[strlen(uri->text)-1] != '/') {
					GnomeVFSURI *tmpuri = gnome_vfs_uri_append_path (uri, "/");
					
					result = do_get_file_info (method, tmpuri, file_info, options, context);
					gnome_vfs_uri_unref (tmpuri);
				}
			}
		}
	}
	
	DEBUG_HTTP (("-Get_File_Info (%d)", result));
	ANALYZE_HTTP ("==> -do_get_file_info");
	
	return result;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
	GnomeVFSResult result;
	
	DEBUG_HTTP (("+Get_File_Info_From_Handle"));
	
	result = do_get_file_info (method, ((HttpFileHandle *)method_handle)->uri, 
				   file_info, options, context);

	DEBUG_HTTP (("-Get_File_Info_From_Handle"));
	
	return result;
}

static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)
{
	DEBUG_HTTP (("+Is_Local"));
	return FALSE;
}

static GnomeVFSResult 
do_make_directory (GnomeVFSMethod *method, 
		   GnomeVFSURI *uri,
                   guint perm, 
		   GnomeVFSContext *context) 
{
	/* MKCOL /path HTTP/1.0 */

	HttpFileHandle *handle;
	GnomeVFSResult result;

	DEBUG_HTTP (("+Make_Directory URI: '%s'", gnome_vfs_uri_to_string (uri, 0)));
	ANALYZE_HTTP ("==> +do_make_directory");

	/*
	 * MKCOL returns a 405 if you try to MKCOL on something that
	 * already exists.  Of course, we don't know whether that means that 
	 * the server doesn't support DAV or the collection already exists.
	 * So we do a PROPFIND first to find out
	 */
	/* FIXME check cache here */
	result = make_propfind_request(&handle, uri, 0, context);

	if (result == GNOME_VFS_OK) {
		result = GNOME_VFS_ERROR_FILE_EXISTS;
	} else {
		/* Make sure we're not leaking an old one */
		g_assert (handle == NULL);
		
		if (result == GNOME_VFS_ERROR_NOT_FOUND) {
			http_cache_invalidate_uri_parent (uri);
			result = make_request (&handle, uri, "MKCOL", NULL, NULL, context);
		}
	}
	http_handle_close (handle, context);
	
	if (result == GNOME_VFS_ERROR_NOT_FOUND) {
		result = resolve_409 (method, uri, context);
	}

	ANALYZE_HTTP ("==> -do_make_directory");
	DEBUG_HTTP (("-Make_Directory (%d)", result));

	return result;
}

static GnomeVFSResult 
do_remove_directory(GnomeVFSMethod *method, 
		    GnomeVFSURI *uri, 
		    GnomeVFSContext *context) 
{
	/* DELETE /path HTTP/1.0 */
	HttpFileHandle *handle;
	GnomeVFSResult result;

	ANALYZE_HTTP ("==> +do_remove_directory");
	DEBUG_HTTP (("+Remove_Directory URI: '%s'", gnome_vfs_uri_to_string (uri, 0)));

	http_cache_invalidate_uri_parent (uri);

	/* FIXME this should return GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY if the
	 * directory is not empty
	 */
	result = make_request (&handle, uri, "DELETE", NULL, NULL,
			       context);
	http_handle_close (handle, context);
	
	DEBUG_HTTP (("-Remove_Directory (%d)", result));
	ANALYZE_HTTP ("==> -do_remove_directory");
	
	return result;
}

static gboolean 
is_same_fs (const GnomeVFSURI *a, 
	    const GnomeVFSURI *b)
{
	return null_handling_strcmp (gnome_vfs_uri_get_scheme (a), gnome_vfs_uri_get_scheme (b)) == 0
		&& null_handling_strcmp (gnome_vfs_uri_get_host_name (a), gnome_vfs_uri_get_host_name (b)) == 0
	  	&& null_handling_strcmp (gnome_vfs_uri_get_user_name (a), gnome_vfs_uri_get_user_name (b)) == 0
	  	&& null_handling_strcmp (gnome_vfs_uri_get_password (a), gnome_vfs_uri_get_password (b)) == 0
		&& (gnome_vfs_uri_get_host_port (a) == gnome_vfs_uri_get_host_port (b));
}

static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{

	/*
	 * MOVE /path1 HTTP/1.0
	 * Destination: /path2
	 * Overwrite: (T|F)
	 */

	HttpFileHandle *handle;
	GnomeVFSResult result;

	char *destpath, *destheader;

	ANALYZE_HTTP ("==> +do_move");
	DEBUG_HTTP (("+Move URI: '%s' Dest: '%s'", 
		gnome_vfs_uri_to_string (old_uri, 0), 
		gnome_vfs_uri_to_string (new_uri, 0)));

	if (!is_same_fs (old_uri, new_uri)) {
		return GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM;
	}	
	
	destpath = gnome_vfs_uri_to_string (new_uri, GNOME_VFS_URI_HIDE_USER_NAME|GNOME_VFS_URI_HIDE_PASSWORD);
	destheader = g_strdup_printf ("Destination: %s\r\nOverwrite: %c\r\n", destpath, force_replace ? 'T' : 'F' );

	result = make_request (&handle, old_uri, "MOVE", NULL, destheader, context);
	http_handle_close (handle, context);
	handle = NULL;

	if (result == GNOME_VFS_ERROR_NOT_FOUND) {
		result = resolve_409 (method, new_uri, context);
	}

	http_cache_invalidate_uri_parent (old_uri);
	http_cache_invalidate_uri_parent (new_uri);

	DEBUG_HTTP (("-Move (%d)", result));
	ANALYZE_HTTP ("==> -do_move");

	return result;
}


static GnomeVFSResult 
do_unlink(GnomeVFSMethod *method,
	GnomeVFSURI *uri,
	  GnomeVFSContext *context)
{
	GnomeVFSResult result;

	/* FIXME need to make sure this fails on directories */
	ANALYZE_HTTP ("==> +do_unlink");
	DEBUG_HTTP (("+Unlink URI: '%s'", gnome_vfs_uri_to_string (uri, 0)));
	result = do_remove_directory (method, uri, context);
	DEBUG_HTTP (("-Unlink (%d)", result));
	ANALYZE_HTTP ("==> -do_unlink");
	
	return result;
}

static GnomeVFSResult 
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *a,
		  GnomeVFSURI *b,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	*same_fs_return = is_same_fs (a, b);
	
	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{
	GnomeVFSURI *parent_uri, *new_uri;
	GnomeVFSResult result;
	
	/* FIXME: For now, we only support changing the name. */
	if ((mask & ~(GNOME_VFS_SET_FILE_INFO_NAME)) != 0) {
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}
	
	/* FIXME bugzilla.eazel.com 645: Make sure this returns an
	 * error for incoming names with "/" characters in them,
	 * instead of moving the file.
	 */
	
	/* Share code with do_move. */
	parent_uri = gnome_vfs_uri_get_parent (uri);
	if (parent_uri == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	new_uri = gnome_vfs_uri_append_file_name (parent_uri, info->name);
	gnome_vfs_uri_unref (parent_uri);
	result = do_move (method, uri, new_uri, FALSE, context);
	gnome_vfs_uri_unref (new_uri);
	return result;
}

static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	NULL, /* seek */
	NULL, /* tell */
	NULL, /* truncate_handle */
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
	NULL, /* truncate */
	NULL, /* find_directory */
	NULL  /* create_symbolic_link */
};

GnomeVFSMethod *
vfs_module_init (const char *method_name, 
		 const char *args)
{
	char *argv[] = {"dummy"};
	int argc = 1;
	GError *gconf_error = NULL;
	GConfValue *proxy_value;

	LIBXML_TEST_VERSION
		
	/* Ensure GConf is initialized.  If more modules start to rely on
	 * GConf, then this should probably be moved into a more 
	 * central location
	 */

	if (!gconf_is_initialized ()) {
		/* auto-initializes OAF if necessary */
		gconf_init (argc, argv, NULL);
	}

	/* ensure GTK is inited for gconf-client. */
#if GNOME_PLATFORM_VERSION < 1095000
	gtk_type_init ();
#else
	gtk_type_init (G_TYPE_DEBUG_NONE);
#endif

	gl_client = gconf_client_get_default ();

#if GNOME_PLATFORM_VERSION < 1095000
	gtk_object_ref (GTK_OBJECT (gl_client));
	gtk_object_sink (GTK_OBJECT (gl_client));
#endif

	gl_mutex = g_mutex_new ();
	
	gconf_client_add_dir (gl_client, PATH_GCONF_GNOME_VFS, GCONF_CLIENT_PRELOAD_NONE, &gconf_error);

	if (gconf_error) {
		DEBUG_HTTP (("GConf error during client_add_dir '%s'", gconf_error->message));
		g_error_free (gconf_error);
		gconf_error = NULL;
	}

#if GNOME_PLATFORM_VERSION < 1095000
	gtk_signal_connect (GTK_OBJECT (gl_client), "value_changed", (GtkSignalFunc) sig_gconf_value_changed, NULL);
#endif

	/* Load the http proxy setting */	
	proxy_value = gconf_client_get (gl_client, KEY_GCONF_USE_HTTP_PROXY, &gconf_error);

	if (gconf_error != NULL) {
		DEBUG_HTTP (("GConf error during client_get '%s'", gconf_error->message));
		g_error_free (gconf_error);
		gconf_error = NULL;
	} else if (proxy_value != NULL) {
		sig_gconf_value_changed (gl_client, KEY_GCONF_USE_HTTP_PROXY, proxy_value);
		gconf_value_free (proxy_value);
	}

	proxy_value = gconf_client_get (gl_client, KEY_GCONF_HTTP_USE_AUTH, &gconf_error);

	if (gconf_error != NULL) {
		DEBUG_HTTP (("GConf error during client_get '%s'", gconf_error->message));
		g_error_free (gconf_error);
		gconf_error = NULL;
	} else if (proxy_value != NULL) {
		sig_gconf_value_changed (gl_client, KEY_GCONF_HTTP_USE_AUTH, proxy_value);
		gconf_value_free (proxy_value);
	}

	http_authn_init ();
	http_cache_init ();

	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
#if GNOME_PLATFORM_VERSION < 1095000
	gtk_signal_disconnect_by_func (GTK_OBJECT(gl_client), (GtkSignalFunc) sig_gconf_value_changed, NULL);
#endif

#if GNOME_PLATFORM_VERSION < 1095000
	gtk_object_destroy (GTK_OBJECT (gl_client));
	gtk_object_unref (GTK_OBJECT (gl_client));
#else
	g_object_unref (G_OBJECT (gl_client));
#endif

	http_authn_shutdown ();
	
	http_cache_shutdown();

	g_mutex_free (gl_mutex);

	gl_client = NULL;
}

/* A "409 Conflict" currently maps to GNOME_VFS_ERROR_NOT_FOUND because it can be returned
 * when the parent collection/directory does not exist.  Unfortunately, Xythos also returns
 * this code when the destination filename of a PUT, MKCOL, or MOVE contains illegal characters, 
 * eg "my*file:name".
 * 
 * The only way to resolve this is to ask...
 */

static GnomeVFSResult
resolve_409 (GnomeVFSMethod *method, GnomeVFSURI *uri, GnomeVFSContext *context)
{
	GnomeVFSFileInfo *file_info;
	GnomeVFSURI *parent_dest_uri;
	GnomeVFSResult result;


	ANALYZE_HTTP ("==> +resolving 409");

	file_info = gnome_vfs_file_info_new ();
	parent_dest_uri = gnome_vfs_uri_get_parent (uri);

	if (parent_dest_uri != NULL) {
		result = do_get_file_info (method,
					   parent_dest_uri,
					   file_info,
					   GNOME_VFS_FILE_INFO_DEFAULT,
					   context);

		gnome_vfs_file_info_unref (file_info);
		file_info = NULL;
		
		gnome_vfs_uri_unref (parent_dest_uri);
		parent_dest_uri = NULL;
	} else {
		result = GNOME_VFS_ERROR_NOT_FOUND;
	}
	
	if (result == GNOME_VFS_OK) {
		/* The destination filename contains characters that are not allowed
		 * by the server.  This is a bummer mapping, but EINVAL is what
		 * the Linux FAT filesystems return on bad filenames, so at least
		 * its not without precedent...
		 */ 
		result = GNOME_VFS_ERROR_BAD_PARAMETERS;
	} else {
		/* The destination's parent path does not exist */
		result = GNOME_VFS_ERROR_NOT_FOUND;
	}

	ANALYZE_HTTP ("==> -resolving 409");

	return result;
}

static gboolean
invoke_callback_basic_authn (HttpFileHandle *handle, 
			     enum AuthnHeaderType authn_which,
			     gboolean previous_attempt_failed)
{
	GnomeVFSModuleCallbackAuthenticationIn in_args;
	GnomeVFSModuleCallbackAuthenticationOut out_args;
	gboolean ret;

	ret = FALSE;
	
	memset (&in_args, 0, sizeof (in_args));
	memset (&out_args, 0, sizeof (out_args));

	in_args.previous_attempt_failed = previous_attempt_failed;
		
	in_args.uri = gnome_vfs_uri_to_string (handle->uri, GNOME_VFS_URI_HIDE_NONE);

	ret = http_authn_parse_response_header_basic (authn_which, handle->response_headers, &in_args.realm);
		
	if (!ret) {
		goto error;
	}

	DEBUG_HTTP (("Invoking %s authentication callback for uri %s",
		authn_which == AuthnHeader_WWW ? "basic" : "proxy", in_args.uri));

	in_args.auth_type = AuthTypeBasic;

	ret = gnome_vfs_module_callback_invoke (authn_which == AuthnHeader_WWW 
						? GNOME_VFS_MODULE_CALLBACK_AUTHENTICATION
						: GNOME_VFS_MODULE_CALLBACK_HTTP_PROXY_AUTHENTICATION, 
						&in_args, sizeof (in_args), 
						&out_args, sizeof (out_args)); 

	if (!ret) {
		DEBUG_HTTP (("No callback registered"));
		goto error;
	}

	ret = (out_args.username != NULL);

	if (!ret) {
		DEBUG_HTTP (("No username provided by callback"));
		goto error;
	}

	DEBUG_HTTP (("Back from authentication callback, adding credentials"));

	if (authn_which == AuthnHeader_WWW) {
		http_authn_session_add_credentials (handle->uri, out_args.username, out_args.password);
	} else /* if (authn_which == AuthnHeader_Proxy) */ {
		proxy_set_authn (out_args.username, out_args.password);
	}
error:
	g_free (in_args.uri);
	g_free (in_args.realm);
	g_free (out_args.username);
	g_free (out_args.password);

	return ret;
}

static int
strcmp_allow_nulls (const char *s1, const char *s2)
{
	return strcmp (s1 == NULL ? "" : s1, s2 == NULL ? "" : s2);
}


/* Returns TRUE if the given URL has changed authentication credentials
 * from the last request (eg, another thread updated the authn information) 
 * or if the application provided new credentials via a callback
 *
 * prev_authn_header is NULL if the previous request contained no authn information.
 */

gboolean
check_authn_retry_request (HttpFileHandle * http_handle,
			   enum AuthnHeaderType authn_which,
			   const char *prev_authn_header)
{
	gboolean ret;
	char *current_authn_header;

	current_authn_header = NULL;
	
	g_mutex_lock (gl_mutex);

	if (authn_which == AuthnHeader_WWW) {
		current_authn_header = http_authn_get_header_for_uri (http_handle->uri);
	} else if (authn_which == AuthnHeader_Proxy) {
		current_authn_header = proxy_get_authn_header_for_uri_nolock (http_handle->uri);
	} else {
		g_assert_not_reached ();
	}

	ret = FALSE;
	if (0 == strcmp_allow_nulls (current_authn_header, prev_authn_header)) {
		ret = invoke_callback_basic_authn (http_handle, authn_which, prev_authn_header == NULL);
	} else {
		ret = TRUE;
	}

	g_mutex_unlock (gl_mutex);

	g_free (current_authn_header);

	return ret;
} 


utime_t
http_util_get_utime (void)
{
    struct timeval tmp;
    gettimeofday (&tmp, NULL);
    return (utime_t)tmp.tv_usec + ((gint64)tmp.tv_sec) * 1000000LL;
}


/* BASE64 code ported from neon (http://www.webdav.org/neon) */
static const gchar b64_alphabet[65] = {
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/=" };

gchar *
http_util_base64 (const gchar *text)
{
	/* The tricky thing about this is doing the padding at the end,
	 * doing the bit manipulation requires a bit of concentration only */
	gchar *buffer, *point;
	gint inlen, outlen;

	/* Use 'buffer' to store the output. Work out how big it should be...
	 * This must be a multiple of 4 bytes 
	 */

	inlen = strlen (text);
	outlen = (inlen*4)/3;
	if ((inlen % 3) > 0) { /* got to pad */
		outlen += 4 - (inlen % 3);
	}

	buffer = g_malloc (outlen + 1); /* +1 for the \0 */

	/* now do the main stage of conversion, 3 bytes at a time,
	 * leave the trailing bytes (if there are any) for later
	 */

	for (point=buffer; inlen>=3; inlen-=3, text+=3) {
		*(point++) = b64_alphabet[ (*text)>>2 ];
		*(point++) = b64_alphabet[ ((*text)<<4 & 0x30) | (*(text+1))>>4 ];
		*(point++) = b64_alphabet[ ((*(text+1))<<2 & 0x3c) | (*(text+2))>>6 ];
		*(point++) = b64_alphabet[ (*(text+2)) & 0x3f ];
	}

	/* Now deal with the trailing bytes */
	if (inlen) {
		/* We always have one trailing byte */
		*(point++) = b64_alphabet[ (*text)>>2 ];
		*(point++) = b64_alphabet[ ( ((*text)<<4 & 0x30) |
									 (inlen==2?(*(text+1))>>4:0) ) ];
		*(point++) = (inlen==1?'=':b64_alphabet[ (*(text+1))<<2 & 0x3c ] );
		*(point++) = '=';
	}

	/* Null-terminate */
	*point = '\0';

	return buffer;
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

#define VERIFY_BOOLEAN_RESULT(function, expected) \
	G_STMT_START {											\
		gboolean result = function; 								\
		if (! ((result && expected) || (!result && !expected))) {				\
			test_failed ("%s: returned '%d' expected '%d'", #function, (int)result, (int)expected);	\
		}											\
	} G_STMT_END


static gboolean
http_self_test (void)
{
	g_message ("self-test: http\n");

	VERIFY_BOOLEAN_RESULT (proxy_should_for_hostname ("localhost"), FALSE);
	VERIFY_BOOLEAN_RESULT (proxy_should_for_hostname ("LocalHost"), FALSE);
	VERIFY_BOOLEAN_RESULT (proxy_should_for_hostname ("127.0.0.1"), FALSE);
	VERIFY_BOOLEAN_RESULT (proxy_should_for_hostname ("127.127.0.1"), FALSE);
	VERIFY_BOOLEAN_RESULT (proxy_should_for_hostname ("www.yahoo.com"), TRUE);

	return !at_least_one_test_failed;
}

gboolean vfs_module_self_test (void);

gboolean
vfs_module_self_test (void)
{
	gboolean ret;

	ret = TRUE;

	ret = http_authn_self_test () && ret;

	ret = http_self_test () && ret;

	return ret;
}

