/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-ssl.h
 *
 * Copyright (C) 2001 Ian McKellar
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
/*
 * Authors: Ian McKellar <yakk@yakk.net>
 *   My knowledge of SSL programming is due to reading Jeffrey Stedfast's
 *   excellent SSL implementation in Evolution.
 */

#include <config.h>
#include "gnome-vfs-ssl.h"

#include <string.h>
#include <glib.h>
#include "gnome-vfs.h"
#include "gnome-vfs-private.h"
#include "gnome-vfs-ssl-private.h"

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

typedef struct {
#ifdef HAVE_OPENSSL
	int sockfd;
	SSL *ssl;
#else
#ifdef HAVE_NSS
	PRFileDesc *sockfd;
#else
	char	dummy;
#endif
#endif
} GnomeVFSSSLPrivate;

struct GnomeVFSSSL {
	GnomeVFSSSLPrivate *private;
};

void 
gnome_vfs_ssl_init () {
#ifdef HAVE_OPENSSL
	SSL_library_init ();
#endif
}

gboolean
gnome_vfs_ssl_enabled ()
{
#ifdef HAVE_OPENSSL
	return TRUE;
#else
	return FALSE;
#endif
}

/* FIXME: add *some* kind of cert verification! */
GnomeVFSResult
gnome_vfs_ssl_create (GnomeVFSSSL **handle_return, 
		      const char *host, 
		      unsigned int port)
{
#ifdef HAVE_OPENSSL
	int fd;
	int ret;
	struct hostent *h;
	struct sockaddr_in sin;

        sin.sin_port = htons (port);
	h = gethostbyname (host);

	if (h == NULL) {
		/* host lookup failed */
		return gnome_vfs_result_from_h_errno ();
	}

        sin.sin_family = h->h_addrtype;
        memcpy (&sin.sin_addr, h->h_addr, sizeof (sin.sin_addr));

        fd = socket (h->h_addrtype, SOCK_STREAM, 0);
	if (fd < 0) {
		return gnome_vfs_result_from_errno ();
	}

	ret = connect (fd, (struct sockaddr *)&sin, sizeof (sin));
	if (ret == -1) {
		/* connect failed */
		return gnome_vfs_result_from_errno ();
	}

	return gnome_vfs_ssl_create_from_fd (handle_return, fd);
#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

GnomeVFSResult
gnome_vfs_ssl_create_from_fd (GnomeVFSSSL **handle_return, 
		              gint fd)
{
#ifdef HAVE_OPENSSL
	GnomeVFSSSL *ssl;
	SSL_CTX *ssl_ctx = NULL;
	int ret;

	ssl = g_new0 (GnomeVFSSSL, 1);
	ssl->private = g_new0 (GnomeVFSSSLPrivate, 1);
	ssl->private->sockfd = fd;

        /* SSLv23_client_method will negotiate with SSL v2, v3, or TLS v1 */
        ssl_ctx = SSL_CTX_new (SSLv23_client_method ());

	if (ssl_ctx == NULL) {
		return GNOME_VFS_ERROR_INTERNAL;
	}

        /* FIXME: SSL_CTX_set_verify (ssl_ctx, SSL_VERIFY_PEER, &ssl_verify);*/
        ssl->private->ssl = SSL_new (ssl_ctx);

	if (ssl->private->ssl == NULL) {
		return GNOME_VFS_ERROR_IO;
	}

        SSL_set_fd (ssl->private->ssl, fd);

	ret = SSL_connect (ssl->private->ssl);
	if (ret != 1) {
                SSL_shutdown (ssl->private->ssl);

                if (ssl->private->ssl->ctx)
                        SSL_CTX_free (ssl->private->ssl->ctx);

                SSL_free (ssl->private->ssl);
		g_free (ssl->private);
		g_free (ssl);
		return GNOME_VFS_ERROR_IO;
	}

	*handle_return = ssl;

	return GNOME_VFS_OK;


#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

GnomeVFSResult 
gnome_vfs_ssl_read (GnomeVFSSSL *ssl,
		    gpointer buffer,
		    GnomeVFSFileSize bytes,
		    GnomeVFSFileSize *bytes_read)
{
#if HAVE_OPENSSL
	if (bytes == 0) {
		*bytes_read = 0;
		return GNOME_VFS_OK;
	}

	*bytes_read = SSL_read (ssl->private->ssl, buffer, bytes);

	if (*bytes_read <= 0) {
		*bytes_read = 0;
		return GNOME_VFS_ERROR_GENERIC;
	}
	return GNOME_VFS_OK;
#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

GnomeVFSResult 
gnome_vfs_ssl_write (GnomeVFSSSL *ssl,
		     gconstpointer buffer,
		     GnomeVFSFileSize bytes,
		     GnomeVFSFileSize *bytes_written)
{
#if HAVE_OPENSSL
	if (bytes == 0) {
		*bytes_written = 0;
		return GNOME_VFS_OK;
	}

	*bytes_written = SSL_write (ssl->private->ssl, buffer, bytes);

	if (*bytes_written <= 0) {
		*bytes_written = 0;
		return GNOME_VFS_ERROR_GENERIC;
	}
	return GNOME_VFS_OK;
#else
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
#endif
}

void
gnome_vfs_ssl_destroy (GnomeVFSSSL *ssl) 
{
#if HAVE_OPENSSL
	SSL_shutdown (ssl->private->ssl);
	SSL_CTX_free (ssl->private->ssl->ctx);
	SSL_free (ssl->private->ssl);
	close (ssl->private->sockfd);
#else
#endif
	g_free (ssl->private);
	g_free (ssl);
}

static GnomeVFSSocketImpl ssl_socket_impl = {
	(GnomeVFSSocketReadFunc)gnome_vfs_ssl_read,
	(GnomeVFSSocketWriteFunc)gnome_vfs_ssl_write,
	(GnomeVFSSocketCloseFunc)gnome_vfs_ssl_destroy
};

GnomeVFSSocket *
gnome_vfs_ssl_to_socket (GnomeVFSSSL *ssl)
{
	return gnome_vfs_socket_new (&ssl_socket_impl, ssl);
}
