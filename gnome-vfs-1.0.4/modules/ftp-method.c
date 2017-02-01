/* -*- Mode: C; tab-width: 8; indent-tabs-mode: 8; c-basic-offset: 8 -*- */

/* ftp-method.c - VFS modules for FTP

   Copyright (C) 2000 Ian McKellar, Eazel Inc.

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

   Author: Ian McKellar <yakk@yakk.net> */

/* see RFC 959 for protocol details */

/* SOME INVALID ASSUMPTIONS I HAVE MADE:
 * All FTP servers return UNIX ls style responses to LIST,
 */

/* TODO */
/* FIXME bugzilla.eazel.com 1463: Make koobera.math.uic.edu and 
   Make NetPresenz work (eg: uniserver.uwa.edu.au)*/
/* FIXME bugzilla.eazel.com 1465: FtpUri / FtpConnectionUri refcounting or something. */
/* FIXME bugzilla.eazel.com 1466: do_get_file_info_from_handle */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h> /* for isspace */
#include <stdlib.h> /* for atoi */
#include <stdio.h> /* for sscanf */
#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <gtk/gtk.h>
#ifdef HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

#include "gnome-vfs-context.h"
#include "gnome-vfs-iobuf.h"
#include "gnome-vfs-inet-connection.h"
#include "gnome-vfs-method.h"
#include "gnome-vfs-module.h"
#include "gnome-vfs-module-shared.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-parse-ls.h"
#include "gnome-vfs-utils.h"

#include "ftp-method.h"

/* Standard FTP proxy port */
#define DEFAULT_FTP_PROXY_PORT 8080

/* maximum size of response we're expecting to get */
#define MAX_RESPONSE_SIZE 4096 

/* macros for the checking of FTP response codes */
#define IS_100(X) ((X) >= 100 && (X) < 200)
#define IS_200(X) ((X) >= 200 && (X) < 300)
#define IS_300(X) ((X) >= 300 && (X) < 400)
#define IS_400(X) ((X) >= 400 && (X) < 500)
#define IS_500(X) ((X) >= 500 && (X) < 600)

static const char PROXY_KEY[] = "/system/gnome-vfs/http-proxy";
static const char USE_PROXY_KEY[] = "/system/gnome-vfs/use-http-proxy";


static GnomeVFSResult do_open	         (GnomeVFSMethod               *method,
					  GnomeVFSMethodHandle         **method_handle,
					  GnomeVFSURI                   *uri,
					  GnomeVFSOpenMode               mode,
					  GnomeVFSContext               *context);
static gboolean       do_is_local        (GnomeVFSMethod               *method,
					  const GnomeVFSURI             *uri);
static GnomeVFSResult do_open_directory  (GnomeVFSMethod                *method,
					  GnomeVFSMethodHandle         **method_handle,
					  GnomeVFSURI                   *uri,
					  GnomeVFSFileInfoOptions        options,
					  const GnomeVFSDirectoryFilter *filter,
					  GnomeVFSContext               *context);
static GnomeVFSResult do_close_directory (GnomeVFSMethod               *method,
					  GnomeVFSMethodHandle          *method_handle,
					  GnomeVFSContext               *context);
static GnomeVFSResult do_read_directory  (GnomeVFSMethod               *method,
		                          GnomeVFSMethodHandle          *method_handle,
		                          GnomeVFSFileInfo              *file_info,
		                          GnomeVFSContext               *context);

	/* FIXME bugzilla.eazel.com 1137: implement filters */

guint                 ftp_connection_uri_hash  (gconstpointer c);
gint                  ftp_connection_uri_equal (gconstpointer c, gconstpointer d);
static GnomeVFSResult ftp_connection_acquire   (GnomeVFSURI *uri, 
		                                FtpConnection **connection, 
		                                GnomeVFSContext *context);
static void           ftp_connection_release   (FtpConnection *conn);


static const char *anon_user = "anonymous";
static const char *anon_pass = "nobody@gnome.org";
static const int   control_port = 21;


/* A GHashTable of GLists of FtpConnections */
static GHashTable *spare_connections = NULL;
G_LOCK_DEFINE_STATIC (spare_connections);
static gint total_connections = 0;
static gint allocated_connections = 0;

#if 0
struct CachedDirectoryListing {
	GList *file_info_list;
	time_t expires;
};

/* A GHashTable of directory listings */
static GHashTable *cached_directory_listings = NULL;
G_LOCK_DEFINE_STATIC (cached_directory_listings);

static void dircache_setup() {
	/* NOTE: only call this when you have the lock */
	if (cached_directory_listings == NULL)
		cached_directory_listings = g_hash_table_new 
			(gnome_vfs_uri_hash, gnome_vfs_uri_hequal);
}

static void dircache_expire(GnomeVFSURI *uri) {
	GList *list;
	G_LOCK (cached_directory_listings);
	dircache_setup();
	list = g_hash_table_lookup(cached_directory_listings, uri);
	G_UNLOCK (cached_directory_listings);
}
#endif


#if ENABLE_FTP_DEBUG

#define ftp_debug(c,g) FTP_DEBUG((c),(g),__FILE__, __LINE__, __PRETTY_FUNCTION__)
static void 
FTP_DEBUG (FtpConnection *conn, 
	   gchar *text, 
	   gchar *file, 
	   gint line, 
	   gchar *func) 
{
	if (conn) {
		g_print ("%s:%d (%s) [ftp conn=%p]\n %s\n", file, line, 
			 func, conn, text);
	} else {
		g_print ("%s:%d (%s) [ftp]\n %s\n", file, line, func, text);
	}

	g_free (text);
}

#else 

#define ftp_debug(c,g) (g)

#endif

static GnomeVFSResult 
ftp_response_to_vfs_result (FtpConnection *conn) 
{
	gint response = conn->response_code;

	switch (response) {
	case 421: 
	case 426: 
	  return GNOME_VFS_ERROR_CANCELLED;
	case 425:
		/*FIXME this looks like a bad mapping.  
		 * 425 is "could not open data connection" which
		 * probably doesn't have anything to do with file permissions
		 */ 
	  return GNOME_VFS_ERROR_ACCESS_DENIED;
	case 530:
	case 331:
	case 332:
	case 532:
	  return GNOME_VFS_ERROR_LOGIN_FAILED;
	case 450:
	case 451:
	case 551:
	  return GNOME_VFS_ERROR_NOT_FOUND;
	case 550:
	  return conn->fivefifty;
	case 452:
	case 552:
	  return GNOME_VFS_ERROR_NO_SPACE;
	case 553:
	  return GNOME_VFS_ERROR_BAD_FILE;
	}

	/* is this the correct interpretation of this error? */
	if (IS_100 (response)) return GNOME_VFS_OK;
	if (IS_200 (response)) return GNOME_VFS_OK;
	/* is this the correct interpretation of this error? */
	if (IS_300 (response)) return GNOME_VFS_OK;
	if (IS_400 (response)) return GNOME_VFS_ERROR_GENERIC;
	if (IS_500 (response)) return GNOME_VFS_ERROR_INTERNAL;

	return GNOME_VFS_ERROR_GENERIC;

}

static GnomeVFSResult read_response_line(FtpConnection *conn, gchar **line) {
	GnomeVFSFileSize bytes = MAX_RESPONSE_SIZE, bytes_read;
	gchar *ptr, *buf = g_malloc (MAX_RESPONSE_SIZE+1);
	gint line_length;
	GnomeVFSResult result = GNOME_VFS_OK;

	while (!strstr (conn->response_buffer->str, "\r\n")) {
		/* we don't have a full line. Lets read some... */
		/*ftp_debug (conn,g_strdup_printf ("response `%s' is incomplete", conn->response_buffer->str));*/
		bytes_read = 0;
		result = gnome_vfs_iobuf_read (conn->iobuf, buf,
					       bytes, &bytes_read);
		buf[bytes_read] = '\0';
		/*ftp_debug (conn,g_strdup_printf ("read `%s'", buf));*/
		conn->response_buffer = g_string_append (conn->response_buffer,
							 buf);
		if (result != GNOME_VFS_OK) {
		        g_warning ("Error `%s' during read\n", 
				   gnome_vfs_result_to_string(result));
			g_free (buf);
			return result;
		}
	}

	g_free (buf);

	ptr = strstr (conn->response_buffer->str, "\r\n");
	line_length = ptr - conn->response_buffer->str;

	*line = g_strndup (conn->response_buffer->str, line_length);

	g_string_erase (conn->response_buffer, 0 , line_length + 2);

	return result;
}

static GnomeVFSResult
get_response (FtpConnection *conn)
{
	/* all that should be pending is a response to the last command */
	GnomeVFSResult result;

	/*ftp_debug (conn,g_strdup_printf ("get_response(%p)",  conn));*/

	while (TRUE) {
		gchar *line = NULL;
		result = read_response_line (conn, &line);

		if (result != GNOME_VFS_OK) {
			g_free (line);
			g_warning ("Error reading response line.");
			return result;
		}

#ifdef FTP_RESPONSE_DEBUG
		g_print ("FTP: %s\n", line);
#endif

		/* response needs to be at least: "### x"  - I think*/
		if (isdigit ((unsigned char) line[0]) &&
		    isdigit ((unsigned char) line[1]) &&
		    isdigit ((unsigned char) line[2]) &&
		    isspace ((unsigned char) line[3])) {

			conn->response_code = (line[0] - '0') * 100 + (line[1] - '0') * 10 + (line[2] - '0');

			if (conn->response_message) g_free (conn->response_message);
			conn->response_message = g_strdup (line+4);

#if 0
			ftp_debug (conn,g_strdup_printf ("got response %d (%s)", 
							 conn->response_code, conn->response_message));
#endif

			g_free (line);

			return ftp_response_to_vfs_result (conn);

		}

		/* hmm - not a valid line - lets ignore it :-) */
		g_free (line);

	}

	return GNOME_VFS_OK; /* should never be reached */

}

static GnomeVFSResult do_control_write (FtpConnection *conn, 
					gchar *command) 
{
        gchar *actual_command = g_strdup_printf ("%s\r\n", command);
	GnomeVFSFileSize bytes = strlen (actual_command), bytes_written;
	GnomeVFSResult result = gnome_vfs_iobuf_write (conn->iobuf,
						       actual_command, bytes, &bytes_written);
#if 0
	ftp_debug (conn, g_strdup_printf ("sent \"%s\\r\\n\"", command));
#endif
	gnome_vfs_iobuf_flush (conn->iobuf);

	if(result != GNOME_VFS_OK) {
		g_free (actual_command);
		return result;
	}

	if(bytes != bytes_written) {
		g_free (actual_command);
		return result;
	}

	g_free (actual_command);

	return result;
}

static GnomeVFSResult 
do_basic_command (FtpConnection *conn, 
		  gchar *command) 
{
	GnomeVFSResult result = do_control_write(conn, command);

	if (result != GNOME_VFS_OK) {
		return result;
	}

	result = get_response (conn);

	return result;
}

static GnomeVFSResult 
do_path_command (FtpConnection *conn, 
		 gchar *command,
		 GnomeVFSURI *uri) 
{
	const char *path;
	char *actual_command;
	GnomeVFSResult result;

	/* as some point we may need to make this execute a CD and then
  	 * a command using the basename rather than the full path. I am yet
	 * to come across such a system.
	 */
	path = gnome_vfs_unescape_string (uri->text, G_DIR_SEPARATOR_S);

	if (path == NULL || strlen (path) == 0) {
		path = ".";
	}

	actual_command = g_strdup_printf ("%s %s", command, path);

	result = do_basic_command (conn, actual_command);
	g_free (actual_command);
	return result;
}

static GnomeVFSResult 
do_path_command_completely (gchar *command, 
                            GnomeVFSURI *uri, 
			    GnomeVFSContext *context,
			    GnomeVFSResult fivefifty) 
{
	FtpConnection *conn;
	GnomeVFSResult result;

	result = ftp_connection_acquire (uri, &conn, context);
	if (result != GNOME_VFS_OK) {
		return result;
	}

	conn->fivefifty = fivefifty;
	result = do_path_command (conn, command, uri);
	ftp_connection_release (conn);

	return result;
}

static GnomeVFSResult
do_transfer_command (FtpConnection *conn, gchar *command, GnomeVFSContext *context) 
{
	char *host = NULL;
	gint port;
	GnomeVFSResult result;

	/* Image mode (binary to the uninitiated) */
	do_basic_command (conn, "TYPE I");

	/* FIXME bugzilla.eazel.com 1464: implement non-PASV mode */

	/* send PASV */
	do_basic_command (conn, "PASV");

	/* parse response */
	{
	        gint a1, a2, a3, a4, p1, p2;
		gchar *ptr, *response = g_strdup (conn->response_message);
		ptr = strchr (response, '(');
		if (!ptr ||
		    (sscanf (ptr+1,"%d,%d,%d,%d,%d,%d", &a1, &a2, &a3, 
			     &a4, &p1, &p2) != 6)) {
			g_free (response);
			return GNOME_VFS_ERROR_CORRUPTED_DATA;
		}

		host = g_strdup_printf ("%d.%d.%d.%d", a1, a2, a3, a4);
		port = p1*256 + p2;

		g_free (response);

	}

	/* connect */
	result = gnome_vfs_inet_connection_create (&conn->data_connection,
						   host, 
						   port,
						   context ? gnome_vfs_context_get_cancellation(context) : NULL);

	if (host != NULL) {
	        g_free (host);
	}
	if (result != GNOME_VFS_OK) {
		return result;
	}

	conn->data_iobuf = gnome_vfs_inet_connection_get_iobuf (conn->data_connection);

	if (conn->iobuf == NULL) {
		gnome_vfs_inet_connection_destroy (conn->data_connection, NULL);
		return GNOME_VFS_ERROR_GENERIC;
	}

	result = do_control_write (conn, command);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_iobuf_destroy (conn->data_iobuf);
		gnome_vfs_inet_connection_destroy (conn->data_connection, NULL);
		return result;
	}

	result = get_response (conn);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_iobuf_destroy (conn->data_iobuf);
		gnome_vfs_inet_connection_destroy (conn->data_connection, NULL);
		return result;
	}

	return result;
}

static GnomeVFSResult
do_path_transfer_command (FtpConnection *conn, gchar *command, GnomeVFSURI *uri, GnomeVFSContext *context) 
{
	const char *path;
	char *actual_command;
	GnomeVFSResult result;
	/* as some point we may need to make this execute a CD and then
  	 * a command using the basename rather than the full path. I am yet
	 * to come across such a system.
	 */
	path = gnome_vfs_unescape_string (uri->text, G_DIR_SEPARATOR_S);

	if (path == NULL || strlen (path) == 0) {
		path = "/";
	}

	actual_command = g_strdup_printf ("%s %s", command, path);

	result = do_transfer_command (conn, actual_command, context);
	g_free (actual_command);
	return result;
}


static GnomeVFSResult 
end_transfer (FtpConnection *conn) 
{
	GnomeVFSResult result;

	/*ftp_debug (conn, g_strdup ("end_transfer()"));*/

	if(conn->data_iobuf) {
		gnome_vfs_iobuf_flush (conn->data_iobuf);
		gnome_vfs_iobuf_destroy (conn->data_iobuf);
		conn->data_iobuf = NULL;
	}

	if (conn->data_connection) {
	        gnome_vfs_inet_connection_destroy (conn->data_connection, NULL);
		conn->data_connection = NULL;
	}

	result = get_response (conn);

	return result;

}

static GnomeVFSResult 
ftp_connection_create (FtpConnection **connptr, GnomeVFSURI *uri, GnomeVFSContext *context) 
{
	FtpConnection *conn = g_new (FtpConnection, 1);
	GnomeVFSResult result;
	gchar *tmpstring;
	gint port = control_port;
	const gchar *user = anon_user;
	const gchar *pass = anon_pass;
	
	conn->uri = gnome_vfs_uri_dup (uri);
	conn->cwd = NULL;
	conn->data_connection = NULL;
	conn->data_iobuf = NULL;
	conn->response_buffer = g_string_new ("");
	conn->response_message = NULL;
	conn->response_code = -1;
	conn->anonymous = TRUE;
	conn->fivefifty = GNOME_VFS_ERROR_NOT_FOUND;

	if (gnome_vfs_uri_get_host_port (uri)) {
		port = gnome_vfs_uri_get_host_port (uri);
	}

	if (gnome_vfs_uri_get_user_name (uri)) {
		user = gnome_vfs_uri_get_user_name (uri);
		conn->anonymous = FALSE;
	}

	if (gnome_vfs_uri_get_password (uri)) {
		pass = gnome_vfs_uri_get_password (uri);
	}

	result = gnome_vfs_inet_connection_create (&conn->inet_connection, 
						   gnome_vfs_uri_get_host_name (uri), 
						   port, 
						   context ? gnome_vfs_context_get_cancellation(context) : NULL);
	
	if (result != GNOME_VFS_OK) {
	        g_warning ("gnome_vfs_inet_connection_create (\"%s\", %d) = \"%s\"",
			   gnome_vfs_uri_get_host_name (uri),
			   gnome_vfs_uri_get_host_port (uri),
			   gnome_vfs_result_to_string (result));
		g_string_free (conn->response_buffer, TRUE);
		g_free (conn);
		return result;
	}

	conn->iobuf = gnome_vfs_inet_connection_get_iobuf (conn->inet_connection);

	if (conn->iobuf == NULL) {
		g_warning ("gnome_vfs_inet_connection_get_iobuf () failed");
		gnome_vfs_inet_connection_destroy (conn->inet_connection, NULL);
		g_string_free (conn->response_buffer, TRUE);
		g_free (conn);
		return GNOME_VFS_ERROR_GENERIC;
	}

	result = get_response (conn);

	if (result != GNOME_VFS_OK) { 
		g_warning ("ftp server (%s:%d) said `%d %s'", 
			   gnome_vfs_uri_get_host_name (uri),
			   gnome_vfs_uri_get_host_port (uri), 
			   conn->response_code, conn->response_message);
		g_string_free (conn->response_buffer, TRUE);
		g_free (conn);
		return result;
	}

	tmpstring = g_strdup_printf ("USER %s", user);
	result = do_basic_command (conn, tmpstring);
	g_free (tmpstring);

	if (IS_300 (conn->response_code)) {
		tmpstring = g_strdup_printf ("PASS %s", pass);
		result = do_basic_command (conn, tmpstring);
		g_free (tmpstring);
	}

	if (result != GNOME_VFS_OK) {
		/* login failed */
		g_warning ("FTP server said: \"%d %s\"\n", conn->response_code,
			   conn->response_message);
		gnome_vfs_iobuf_destroy (conn->iobuf);
		gnome_vfs_inet_connection_destroy (conn->inet_connection, NULL);
		g_free (conn);
		return result;
	}

	/* okay, we should be connected now */

	/* Image mode (binary to the uninitiated) */

	do_basic_command (conn, "TYPE I");

	/* Get the system type */

	do_basic_command (conn, "SYST");
	conn->server_type=g_strdup(conn->response_message);

	*connptr = conn;

	ftp_debug (conn, g_strdup ("created"));

	total_connections++;

	return GNOME_VFS_OK;
}

static void
ftp_connection_destroy (FtpConnection *conn) 
{

	if (conn->inet_connection) 
	        gnome_vfs_inet_connection_destroy (conn->inet_connection, NULL);

	if (conn->iobuf) 
	        gnome_vfs_iobuf_destroy (conn->iobuf);

	gnome_vfs_uri_unref (conn->uri);
	g_free (conn->cwd);

	if (conn->response_buffer) 
	        g_string_free(conn->response_buffer, TRUE);
	g_free (conn->response_message);
	g_free (conn->server_type);

	if (conn->data_connection) 
		gnome_vfs_inet_connection_destroy(conn->data_connection, NULL);

	if (conn->data_iobuf) 
	        gnome_vfs_iobuf_destroy (conn->data_iobuf);

	g_free (conn->dirlist);
	g_free (conn->dirlistptr);
	g_free (conn);
	total_connections--;
}

/* g_str_hash and g_str_equal seem to fail with null arguments */

static gint 
my_str_hash (const gchar *c) 
{
	if (c) 
	        return g_str_hash (c);
	else return 0;
}

static gint 
my_str_equal (gconstpointer c, 
	      gconstpointer d) 
{
	if ((c && !d) || (d &&!c)) 
		return FALSE;
	if (!c && !d) 
		return TRUE;
	return g_str_equal (c,d);
}

/* hash the bits of a GnomeVFSURI that distingush FTP connections */
guint
ftp_connection_uri_hash (gconstpointer c) 
{
	GnomeVFSURI *uri = (GnomeVFSURI *) c;

	return my_str_hash (gnome_vfs_uri_get_host_name (uri)) + 
		my_str_hash (gnome_vfs_uri_get_user_name (uri)) +
		my_str_hash (gnome_vfs_uri_get_password (uri)) +
		gnome_vfs_uri_get_host_port (uri);
}

/* test the equality of the bits of a GnomeVFSURI that distingush FTP 
 * connections */
gint 
ftp_connection_uri_equal (gconstpointer c, 
			  gconstpointer d) 
{
	GnomeVFSURI *uri1 = (GnomeVFSURI *)c;
	GnomeVFSURI *uri2 = (GnomeVFSURI *) d;

	return my_str_equal (gnome_vfs_uri_get_host_name(uri1),
			     gnome_vfs_uri_get_host_name (uri2)) &&
		my_str_equal (gnome_vfs_uri_get_user_name (uri1),
			      gnome_vfs_uri_get_user_name (uri2)) &&
		my_str_equal (gnome_vfs_uri_get_password (uri1),
			      gnome_vfs_uri_get_password (uri2)) &&
		gnome_vfs_uri_get_host_port (uri1) == 
		gnome_vfs_uri_get_host_port (uri2);
}

static GnomeVFSResult 
ftp_connection_acquire (GnomeVFSURI *uri, FtpConnection **connection, GnomeVFSContext *context) 
{
	GList *possible_connections;
	FtpConnection *conn = NULL;
	GnomeVFSResult result = GNOME_VFS_OK;

	G_LOCK (spare_connections);

	if (spare_connections == NULL) {
		spare_connections = g_hash_table_new (ftp_connection_uri_hash, 
						      ftp_connection_uri_equal);
	}

	possible_connections = g_hash_table_lookup (spare_connections, uri);

	if (possible_connections) {
		/* spare connection(s) found */
		conn = (FtpConnection *) possible_connections->data;
#if 0
		ftp_debug (conn, strdup ("found a connection"));
#endif
		possible_connections = g_list_remove (possible_connections, conn);

		if(!g_hash_table_lookup (spare_connections, uri)) {
			/* uri will be used as a key in the hashtable */
			uri = gnome_vfs_uri_dup (uri);
		}

		g_hash_table_insert (spare_connections, uri, possible_connections);

		/* make sure connection hasn't timed out */
		result = do_basic_command(conn, "PWD");
		if (result != GNOME_VFS_OK) {
			ftp_connection_destroy (conn);
			result = ftp_connection_create (&conn, uri, context);
		}

	} else {
		result = ftp_connection_create (&conn, uri, context);
	}

	G_UNLOCK (spare_connections);

	*connection = conn;

	if(result == GNOME_VFS_OK) allocated_connections++;

	return result;
}


static void 
ftp_connection_release (FtpConnection *conn) 
{
	GList *possible_connections;
	GnomeVFSURI *uri;

	g_return_if_fail (conn);

	/* reset the 550 result */
	conn->fivefifty = GNOME_VFS_ERROR_NOT_FOUND;

	G_LOCK (spare_connections);
	if (spare_connections == NULL) 
		spare_connections = 
			g_hash_table_new (ftp_connection_uri_hash, 
					  ftp_connection_uri_equal);

	possible_connections = g_hash_table_lookup (spare_connections, 
						    conn->uri);
#if 0
	ftp_debug (conn, g_strdup_printf ("releasing [len = %d]", 
					  g_list_length (possible_connections)));
#endif
	possible_connections = g_list_append (possible_connections, conn);

	if (g_hash_table_lookup (spare_connections, conn->uri)) {
		uri = conn->uri; /* no need to duplicate uri */
	} else {
		/* uri will be used as key */
		uri = gnome_vfs_uri_dup (conn->uri); 
	}
	g_hash_table_insert (spare_connections, uri, possible_connections);
	allocated_connections--;

	G_UNLOCK(spare_connections);
}

gboolean 
do_is_local (GnomeVFSMethod *method, 
	     const GnomeVFSURI *uri)
{
	return FALSE;
}


static GnomeVFSResult 
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context) 
{
	GnomeVFSResult result;
	FtpConnection *conn;

	result = ftp_connection_acquire (uri, &conn, context);
	if (result != GNOME_VFS_OK) 
		return result;

	if (mode == GNOME_VFS_OPEN_READ) {
		conn->operation = FTP_READ;
		result = do_path_transfer_command (conn, "RETR", uri, context);
	} else if (mode == GNOME_VFS_OPEN_WRITE) {
		conn->operation = FTP_WRITE;
		conn->fivefifty = GNOME_VFS_ERROR_ACCESS_DENIED;
		result = do_path_transfer_command (conn, "STOR", uri, context);
		conn->fivefifty = GNOME_VFS_ERROR_NOT_FOUND;
	} else {
		g_warning ("Unsupported open mode %d\n", mode);
		ftp_connection_release (conn);
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	}
	if (result == GNOME_VFS_OK) {
		*method_handle = (GnomeVFSMethodHandle *) conn;
	} else {
		*method_handle = NULL;
		ftp_connection_release (conn);
	}
	return result;
}

static GnomeVFSResult
do_create (GnomeVFSMethod *method,
     GnomeVFSMethodHandle **method_handle,
     GnomeVFSURI *uri,
     GnomeVFSOpenMode mode,
     gboolean exclusive,
     guint perm,
     GnomeVFSContext *context) {
	return do_open(method, method_handle, uri, mode, context);
}

static GnomeVFSResult 
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context) 
{
	FtpConnection *conn = (FtpConnection *) method_handle;

	GnomeVFSResult result = end_transfer (conn);

	ftp_connection_release (conn);

	return result;
}

static GnomeVFSResult 
do_read (GnomeVFSMethod *method, 
	 GnomeVFSMethodHandle *method_handle, 
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes, 
	 GnomeVFSFileSize *bytes_read, 
	 GnomeVFSContext *context) 
{
	FtpConnection *conn = (FtpConnection * )method_handle;

#if 0
	/*
	if (conn->operation != FTP_READ) {
		g_print ("attempted to read when conn->operation = %d\n", conn->operation);
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}*/
	g_print ("do_read (%p)\n", method_handle);
#endif

	return gnome_vfs_iobuf_read (conn->data_iobuf, buffer, num_bytes, bytes_read);
}

static GnomeVFSResult 
do_write (GnomeVFSMethod *method, 
	  GnomeVFSMethodHandle *method_handle, 
	  gconstpointer buffer, 
	  GnomeVFSFileSize num_bytes, 
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context) 
{
	FtpConnection *conn = (FtpConnection *) method_handle;
	GnomeVFSResult result;

#if 0
	g_print ("do_write ()\n");
#endif

	if (conn->operation != FTP_WRITE) 
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	
	result = gnome_vfs_iobuf_write (conn->data_iobuf, buffer, num_bytes, 
				      bytes_written);
	return result;
}


static gboolean 
ls_to_file_info (gchar *ls, GnomeVFSFileInfo *file_info, 
		GnomeVFSFileInfoOptions options) 
{
	struct stat s;
	gchar *filename = NULL, *linkname = NULL;
	const char *mime_type;

	gnome_vfs_parse_ls_lga (ls, &s, &filename, &linkname);

	/* g_print ("filename: %s, linkname: %s\n", filename, linkname); */

	if (filename) {

		gnome_vfs_stat_to_file_info (file_info, &s);
		
		/* FIXME: This is a hack, but we can't change
		   the above API until after Gnome 1.4.  Ideally, we
		   would give the stat_to_file_info function this
		   information.  Also, there may be more fields here that are not 
		   valid that we haven't dealt with.  */
		file_info->valid_fields -= GNOME_VFS_FILE_INFO_FIELDS_DEVICE;
		file_info->valid_fields -= GNOME_VFS_FILE_INFO_FIELDS_INODE;
		file_info->valid_fields -= GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE;
		file_info->io_block_size = 0;

		file_info->name = g_strdup (g_basename(filename));

		if(*(file_info->name) == '\0') {
			g_free (file_info->name);
			file_info->name = g_strdup ("/");
		}

		if(linkname) {
			file_info->symlink_name = linkname;
			file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME;
			file_info->flags |= GNOME_VFS_FILE_FLAGS_SYMLINK;
		}

		if (file_info->type == GNOME_VFS_FILE_TYPE_REGULAR) {
			mime_type = gnome_vfs_mime_type_from_name_or_default (file_info->name, GNOME_VFS_MIME_TYPE_UNKNOWN);
			/*ftp_debug (conn, g_strdup_printf ("mimetype = %s", mime_type));*/
		} else {
			mime_type = gnome_vfs_mime_type_from_mode (s.st_mode);
		}
		file_info->mime_type = g_strdup (mime_type);
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

		/*ftp_debug (conn, g_strdup_printf ("info got name `%s'", file_info->name));*/

		g_free (filename);

		return TRUE;
	} else {
		return FALSE;
	}
}


#if 0
static GnomeVFSResult
internal_get_file_info (GnomeVFSMethod *method,
			GnomeVFSURI *uri,
			GnomeVFSFileInfo *file_info,
			GnomeVFSFileInfoOptions options,
			GnomeVFSContext *context) 
{
	FtpConnection *conn;
	/* FIXME bugzilla.eazel.com 1463 */
	GnomeVFSResult result;
	GnomeVFSFileSize num_bytes = 1024, bytes_read;
	gchar buffer[num_bytes+1];

	result = ftp_connection_acquire(uri, &conn);
	if (result != GNOME_VFS_OK) {
		return result;
	}

#if 0
	g_print ("do_get_file_info()\n");
#endif

	if(strstr(conn->server_type,"MACOS")) {
		/* don't ask for symlinks from MacOS servers */
		do_path_transfer_command (conn, "LIST -ld", uri, context);
	} else {
		do_path_transfer_command (conn, "LIST -ldL", uri, context);
	}

	result = gnome_vfs_iobuf_read (conn->data_iobuf, buffer, 
				       num_bytes, &bytes_read);

	if (result != GNOME_VFS_OK) {
		/*ftp_debug (conn, g_strdup ("gnome_vfs_iobuf_read failed"));*/
		ftp_connection_release (conn);
		return result;
	}

	result = end_transfer (conn);

	/* FIXME bugzilla.eazel.com 2793: check return? */

	ftp_connection_release (conn);

	if (result != GNOME_VFS_OK) {
		/*ftp_debug (conn,g_strdup ("LIST for get_file_info failed."));*/
		return result;
	}

	if (bytes_read>0) {

		buffer[bytes_read] = '\0';
		file_info->valid_fields = 0; /* make sure valid_fields is 0 */

		if (ls_to_file_info (buffer, file_info)) {
			return GNOME_VFS_OK;
		}

	}
	
	return GNOME_VFS_ERROR_NOT_FOUND;

}
#endif

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context) 
{
	GnomeVFSURI *parent = gnome_vfs_uri_get_parent (uri);
	GnomeVFSResult result;

	if (parent == NULL) {
		FtpConnection *conn;
		/* this is a request for info about the root directory */

		/* is the host there? */
		result = ftp_connection_acquire (uri, &conn, context);
		
		if (result != GNOME_VFS_OK) {
			/* doesn't look like it */
			return result;
		}

		ftp_connection_release (conn);

		file_info->name = g_strdup ("/");
		file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		file_info->mime_type = g_strdup ("x-directory/normal");
		file_info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
			GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		return GNOME_VFS_OK;
	} else {
		GnomeVFSMethodHandle *method_handle;
		gchar *name;

		result = do_open_directory (method, &method_handle, parent,
					    options, NULL, context);

		gnome_vfs_uri_unref (parent);

		if (result != GNOME_VFS_OK) {
			return result;
		}

	       	name = gnome_vfs_uri_extract_short_name (uri);

		while (result == GNOME_VFS_OK) {
			result = do_read_directory (method, method_handle, 
						    file_info, context);
			if (result == GNOME_VFS_OK) {
				if (file_info->name && !strcmp (file_info->name,
								name)) {
					g_free (name);
					do_close_directory(
							method, method_handle,
						       	context);
					return GNOME_VFS_OK;
				}

				gnome_vfs_file_info_clear (file_info);
			}
		}
		do_close_directory(method, method_handle, context);
	}

	return GNOME_VFS_ERROR_NOT_FOUND;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
	return do_get_file_info (method,
				 ((FtpConnection *)method_handle)->uri, 
				 file_info, options, context);
}

static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)
{
	/* FIXME bugzilla.eazel.com 1137: implement filters */

	FtpConnection *conn;
	GnomeVFSResult result;
	GnomeVFSFileSize num_bytes = 1024, bytes_read;
	gchar buffer[num_bytes+1];
	GString *dirlist = g_string_new ("");

	result = ftp_connection_acquire (uri, &conn, context);
	if (result != GNOME_VFS_OK) {
		g_string_free (dirlist, TRUE);
		return result;
	}

	/*g_print ("do_open_directory () in uri: %s\n", gnome_vfs_uri_get_path(uri));*/

	/* LIST does not return an error if called on a file, but CWD
	 * should. This allows us to have proper gnome-vfs semantics.
	 * does the cwd break other things though?  ie, are
	 * connections stateless?
	 */
	conn->fivefifty = GNOME_VFS_ERROR_NOT_A_DIRECTORY;
	result = do_path_command (conn, "CWD", uri);
	if (result != GNOME_VFS_OK) {
		ftp_connection_release (conn);
		return result;
	}

	if(strstr(conn->server_type,"MACOS")) {
		/* don't ask for symlinks from MacOS servers */
		result = do_transfer_command (conn, "LIST", context);
	} else {
		result = do_transfer_command (conn, "LIST -L", context);
	}

	if (result != GNOME_VFS_OK) {
		g_warning ("opendir failed because \"%s\"", 
			   gnome_vfs_result_to_string (result));
		ftp_connection_release (conn);
		g_string_free (dirlist, TRUE);
		return result;
	}

	while (result == GNOME_VFS_OK) {
		result = gnome_vfs_iobuf_read (conn->data_iobuf, buffer, 
					       num_bytes, &bytes_read);
		if (result == GNOME_VFS_OK && bytes_read > 0) {
			buffer[bytes_read] = '\0';
			dirlist = g_string_append (dirlist, buffer);
		} else {
			break;
		}
	} 

	result = end_transfer (conn);

	if(result != GNOME_VFS_OK) g_warning ("end_transfer (conn) failed!!!!");

	conn->dirlist = g_strdup (dirlist->str);
	conn->dirlistptr = conn->dirlist;
	conn->file_info_options = options;

	g_string_free (dirlist,TRUE);

	*method_handle = (GnomeVFSMethodHandle *) conn;

	return result;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context) 
{
	FtpConnection *conn = (FtpConnection *) method_handle;

#if 0
	g_print ("do_close_directory ()\n");
#endif

	g_free (conn->dirlist);
	conn->dirlist = NULL;
	conn->dirlistptr = NULL;
	ftp_connection_release (conn);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	FtpConnection *conn = (FtpConnection *) method_handle;

	if (!conn->dirlistptr || *(conn->dirlistptr) == '\0')
		return GNOME_VFS_ERROR_EOF;

	while (TRUE) {
		gboolean success = ls_to_file_info (conn->dirlistptr, file_info, conn->file_info_options);

		/* permissions aren't valid */
		file_info->valid_fields &= ~GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS;

		if ((conn->file_info_options & 
					GNOME_VFS_FILE_INFO_FOLLOW_LINKS) && 
				(file_info->type == 
				 GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK)) {
			/* Need to follow symbolic links to match behavior Nautilus
			 * requires for sensible display (otherwise symlinks all appear
			 * broken)
			 */
#if 0
		       	g_print("[debug] expand symlink for: %s\n", file_info->name);
#endif
		}


		if (*(conn->dirlistptr) == '\0')
			return GNOME_VFS_ERROR_EOF;

		/* go till we find \r\n */
		while (conn->dirlistptr &&
		       *conn->dirlistptr && 
		       *conn->dirlistptr != '\r' && 
		       *conn->dirlistptr != '\n') {
			conn->dirlistptr++;
		}
		/* go past \r\n */
		while (conn->dirlistptr && *conn->dirlistptr &&
		       isspace ((unsigned char) (*conn->dirlistptr))) {
			conn->dirlistptr++;
		}

		if(success) break;
	}

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
      GnomeVFSURI *a,
      GnomeVFSURI *b,
      gboolean *same_fs_return,
      GnomeVFSContext *context)
{
	*same_fs_return = ftp_connection_uri_equal (a,b);
	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method, GnomeVFSURI *uri, guint perm, GnomeVFSContext *context)
{
	GnomeVFSResult result;
	gchar *chmod_command;

	result = do_path_command_completely ("MKD", uri, context, 
		GNOME_VFS_ERROR_ACCESS_DENIED);

	if (result == GNOME_VFS_OK) {
		/* try to set the permissions */
		/* this is a non-standard extension, so we'll just do our
		 * best. We can ignore error codes. */
		chmod_command = g_strdup_printf("SITE CHMOD %o", perm);
		do_path_command_completely (chmod_command, uri, context,
			GNOME_VFS_ERROR_ACCESS_DENIED);
		g_free(chmod_command);
	}

	return result;
}


static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	return do_path_command_completely ("RMD", uri, context, 
		GNOME_VFS_ERROR_ACCESS_DENIED);
}


static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	GnomeVFSResult result;
	GnomeVFSFileInfo *p_file_info;

	if (!force_replace) {
		p_file_info = gnome_vfs_file_info_new ();
		result = do_get_file_info (method, new_uri, p_file_info, GNOME_VFS_FILE_INFO_DEFAULT, context);
		gnome_vfs_file_info_unref (p_file_info);
		p_file_info = NULL;

		if (result == GNOME_VFS_OK) {
			return GNOME_VFS_ERROR_FILE_EXISTS;
		}
	}
	

	if (ftp_connection_uri_equal (old_uri, new_uri)) {
		FtpConnection *conn;
		GnomeVFSResult result;

		result = ftp_connection_acquire (old_uri, &conn, context);
		if (result != GNOME_VFS_OK) {
			return result;
		}
		result = do_path_command (conn, "RNFR", old_uri);
		
		if (result == GNOME_VFS_OK) {
			conn->fivefifty = GNOME_VFS_ERROR_ACCESS_DENIED;
			result = do_path_command (conn, "RNTO", new_uri);
			conn->fivefifty = GNOME_VFS_ERROR_NOT_FOUND;
		}

		ftp_connection_release (conn);

		return result;
	} else {
		return GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM;
	}
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method, GnomeVFSURI *uri, GnomeVFSContext *context)
{
	return do_path_command_completely ("DELE", uri, context,
		GNOME_VFS_ERROR_ACCESS_DENIED);
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
	NULL, /* truncate */
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
	NULL /* create_symbolic_link */
};

GnomeVFSMethod *
vfs_module_init (const char *method_name, 
		 const char *args)
{
#ifdef HAVE_GCONF
	char *argv[] = {"vfs-ftp-method"};
	int argc = 1;

	/* Ensure GConf is initialized.  If more modules start to rely on
	 * GConf, then this should probably be moved into a more 
	 * central location
	 */

	if (!gconf_is_initialized ()) {
		/* auto-initializes OAF if necessary */
		gconf_init (argc, argv, NULL);
	}
#endif
	
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
}
