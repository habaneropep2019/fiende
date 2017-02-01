/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* file-method.c - Local file access method for the GNOME Virtual File
   System.

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

   Authors: 
  	Ian McKellar <yakk@yakk.net.au>
  	Grahame Bowland <gbowland@gbowland.com>

   Ref: RFC1094 - this attempts to be a valid client-side implementation of 
                  NFS version 2.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include <string.h>

#include <libgnomevfs/gnome-vfs-mime.h>
#include <libgnomevfs/gnome-vfs-module.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-module-shared.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <libgnomevfs/gnome-vfs-cancellation.h>
#include <libgnomevfs/gnome-vfs-context.h>

/* this is a work-around for some systems, which define these 
 * symbols without guards
 */
#undef MIN
#undef MAX

#include "nfs-method.h"
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <rpc/xdr.h>
#include <arpa/inet.h>

/* the timeout used when making RPC calls */
#define NFS_TIMEOUT 10
/* how many times to retry a given RPC call */
#define NFS_RETRY 5
/* define this for verbose debugging */
// #define NFS_VERBOSE_DEBUG

/* until there's a way to specify this at run time (configuration option)
 * define this at compile time. The run down - use TCP if you're in a bad 
 * environment where UDP packets will go astray (the internet). If you're 
 * on a nice short ethernet then you want UDP.
 *
 * Can this be done in gconf? 'expert' users might want to change it.
 */
#define NFS_PROTO	NFS_UDP

/* a list of all cached server connections
 * 
 * any access to this should first attempt to acquire a mutex on it.
 */
GList *server_connection_list = NULL;
G_LOCK_DEFINE_STATIC (server_connection_list);

/* Module entry points. */
GnomeVFSMethod *vfs_module_init     (const char     *method_name,
				        const char     *args);
void            vfs_module_shutdown  (GnomeVFSMethod *method);

static GnomeVFSResult
nfs_get_attr(GnomeVFSURI *uri, NfsServerConnection *conn, GnomeVFSFileInfo *info);

/* work around for inconsistent paths */
static void
nfs_strip_last_slash(char *c)
{
	int len;
	
	g_assert(c != NULL);

	len = strlen(c);
	if ((len > 1) && (c[len - 1] == '/')) {
		c[len - 1] = '\0';
	}
}


/* a wrapper for the RPC function clnt_call. This handles doing a retry on 
 * certain errors, etc. The final argument, `success', should be a pointer 
 * to an integer that will be zero if the call was completely successful. 
 * it may also be NULL, in which case it is ignored.
 */
static enum clnt_stat 
nfs_clnt_call(CLIENT *clnt, u_long procnum, xdrproc_t inproc, char *in,
	      xdrproc_t outproc, char *out, struct timeval tout, enum nfsstat *success)
{
	int retry_count = 0;
	enum clnt_stat rv = 0;

	while (retry_count < NFS_RETRY) {
		/* try the call */
		rv = clnt_call(clnt, procnum, inproc, in, outproc, out, tout);
		if (rv == RPC_SUCCESS && !*success) {
			/* all good, break from this loop */
			break;
#ifdef ECOMM
/* no ECOMM on FreeBSD.
 * You may want to check that with autoconf or leave like that.
 */
		} else if (*success == ECOMM) {
			/* this error is evil, only way around it is 
			   to destroy the RPC connection and start over
			*/

			/* FIXME
			 * do something intelligent, don't just fail out
			 * FIXME
			 */
#endif
		} else if ((rv == RPC_CANTSEND) || /* can't send */ 
			   (rv == RPC_CANTRECV) || /* can't receive */
			   (rv == RPC_TIMEDOUT) || /* timed out */
			   (rv == RPC_SYSTEMERROR) || /* generic other problem at server */
			   (rv == RPC_PMAPFAILURE) || /* portmapper failed in its call */
			   (rv == RPC_CANTDECODEARGS) || /* can't decode arguments */
			   (rv == RPC_CANTENCODEARGS) || /* can't encode arguments */
			   (rv == RPC_CANTDECODERES) || /* can't decored results */
			   (rv == RPC_FAILED) || /* unspecified error */ 
			   ((success != NULL) && (*success))) /* not okay */ {
			/* retry... this error is one we can recover from
			 * something like a lost UDP packet or a cosmic ray :)
			 */
			g_print("NFS_METHOD: retrying due to an error (retry_count = %d.\n", retry_count);
		} else {
			/* unknown error probably no point retrying, all bad */
			g_print("Error fatal, giving up.\n");
			return rv;
		}
		retry_count++;
	}
	return rv;
}

static GnomeVFSResult
rpc_init_udp(NfsServerConnection *c)
{
	/* create UDP clients */
	if ((c->mount_client = clntudp_create(c->mount_server_addr, 
					MOUNTPROG, MOUNTVERS, 
					c->mount_timeval, 
					&c->mount_sock)) == NULL) {
		clnt_pcreateerror("clntudp_create");
		return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	}
	if ((c->nfs_client = clntudp_create(c->nfs_server_addr, 
					NFS_PROGRAM, NFS_VERSION, 
					c->nfs_timeval, 
					&c->nfs_sock)) == NULL) {
		clnt_pcreateerror("clntudp_create");
		return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	}
	return GNOME_VFS_OK;
}

static GnomeVFSResult
rpc_init_tcp(NfsServerConnection *c)
{
	/* create TCP clients */
	if ((c->mount_client = clnttcp_create(c->mount_server_addr,
					MOUNTPROG, MOUNTVERS,
					&c->mount_sock, 0, 0)) == NULL) {
		clnt_pcreateerror("clnttcp_create");
		return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	}
	if ((c->nfs_client = clnttcp_create(c->nfs_server_addr,
					NFS_PROGRAM, NFS_VERSION,
					&c->nfs_sock, 0, 0)) == NULL) {
		clnt_pcreateerror("clnttcp_create");
		return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	}
	return GNOME_VFS_OK;
}

static GnomeVFSResult
server_connection_acquire(GnomeVFSURI *uri, NfsServerConnection **conn)
{
	NfsServerConnection *new, *cached;
	int retval;
	const char *hostname = gnome_vfs_uri_get_host_name(uri);
	GList *next;

	g_assert(uri != NULL);
	g_assert(conn != NULL);

	G_LOCK (server_connection_list);

	/* make sure we don't try to contact a NULL host! */
	if (hostname == NULL || hostname[0] == '\0') {
		return GNOME_VFS_ERROR_INVALID_URI;
	}
	
	/* examine the available (cached) connections */
	next = server_connection_list;
	while (next) {
		cached = (NfsServerConnection *)next->data;
		if (!g_strcasecmp(cached->hostname, hostname)) {
			/* match */
			*conn = cached;
			G_UNLOCK(server_connection_list);
			return GNOME_VFS_OK;
		}
		next = next->next;
	}

	new = g_new(NfsServerConnection, 1);

	new->hostname = g_strdup(gnome_vfs_uri_get_host_name(uri));
	new->nfs_timeval.tv_sec = NFS_TIMEOUT;
	new->nfs_timeval.tv_usec = 0;
	new->mount_timeval.tv_sec = NFS_TIMEOUT;
	new->mount_timeval.tv_usec = 0;
	new->nfs_server_addr = g_malloc(sizeof(struct sockaddr_in));
	new->mount_server_addr = g_malloc(sizeof(struct sockaddr_in));
	new->mount_sock = RPC_ANYSOCK;
	new->nfs_sock = RPC_ANYSOCK;
	new->nfs_sock_mutex = g_mutex_new();
	new->mount_sock_mutex = g_mutex_new();
	new->file_handle_hash = g_hash_table_new(g_str_hash, g_str_equal);
	new->file_handle_hash_mutex = g_mutex_new();

	/* DNS lookup on the server */
	if ((new->hp = gethostbyname(new->hostname)) == NULL) {
		G_UNLOCK (server_connection_list);
		return GNOME_VFS_ERROR_HOST_NOT_FOUND;
	}

        /* we need two of these, mainly because by having sin_port be zero
         * we get auto-magical discovery of the remote port number.
	 */
	memcpy((caddr_t)&new->mount_server_addr->sin_addr, 
			new->hp->h_addr, new->hp->h_length);
	new->mount_server_addr->sin_family = AF_INET;
	new->mount_server_addr->sin_port = 0;
	memcpy((caddr_t)&new->nfs_server_addr->sin_addr, 
			new->hp->h_addr, new->hp->h_length);
	new->nfs_server_addr->sin_family = AF_INET;
	new->nfs_server_addr->sin_port = 0;

	new->proto = NFS_PROTO;

	if (NFS_PROTO == NFS_UDP) {
		if ((retval = rpc_init_udp(new)) != GNOME_VFS_OK) {
			G_UNLOCK(server_connection_list);
			g_free(new);
			return retval;
		}
	} else {
		if ((retval = rpc_init_tcp(new)) != GNOME_VFS_OK) {
			G_UNLOCK(server_connection_list);
			g_free(new);
			return retval;
		}
	}
	
	/* use basic UNIX auth. NFS v2 trusts _hosts_ - if the filesystem 
	 * has any sort of auth turned on, you'll need to come out of a 
	 * low port. Some OSes (Tru64 Unix) won't even let you mount a 
	 * public export without this.
	 * 
	 * TODO: investigate supporting other methods (kerberos?)
	 */
	new->mount_client->cl_auth = authunix_create_default();
	new->nfs_client->cl_auth = authunix_create_default();

	/* cache the server connection */
	server_connection_list = g_list_append(server_connection_list, (gpointer)new);

	*conn = new;
	G_UNLOCK (server_connection_list);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
fhandle_recurse_lookup (GnomeVFSURI *uri, NfsServerConnection *conn, NfsFileHandle *fh, NfsFileHandle **to_fh)
{
	diropargs args;
	diropres res;
	enum clnt_stat clnt_stat;
	NfsFileHandle *new = g_malloc(sizeof(struct NfsFileHandle));
	int retval;
	
	g_assert(uri != NULL);
	g_assert(conn != NULL);
	g_assert(fh != NULL);

	args.name = gnome_vfs_uri_extract_short_name(uri);
	memset((char *)&args.dir, 0, sizeof(nfs_fh));
	memcpy(args.dir.data, fh->handle.data, NFS_FHSIZE * sizeof(char));

	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_LOOKUP, 
			(xdrproc_t)xdr_diropargs, (caddr_t)&args,
			(xdrproc_t)xdr_diropres, (caddr_t)&res,
			conn->nfs_timeval, &res.status)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "lookup");
		retval = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);

	if (res.status) {
		retval = gnome_vfs_result_from_errno_code(res.status);
		goto error;
	}

	memcpy(new->handle.data, res.diropres_u.diropres.file.data, NFS_FHSIZE * sizeof(char));
	new->uri = uri;
	new->mounted = FALSE;
	retval = GNOME_VFS_OK;

error:
	if (retval == GNOME_VFS_OK) {
		*to_fh = new;
	} else {
		g_free(new);
		*to_fh = NULL;
	}
	return retval;
}

static GnomeVFSResult
fhandle_recurse (GnomeVFSURI *uri, NfsServerConnection *conn, NfsFileHandle **fh)
{
	GnomeVFSURI *parent;
	NfsFileHandle *p, *f;
	int retval;
	char *path;
	
	g_assert(uri != NULL);
	g_assert(conn != NULL);
	g_assert(fh != NULL);

	path = g_strdup(gnome_vfs_uri_get_path(uri));
	nfs_strip_last_slash(path);

	f = (NfsFileHandle *)g_hash_table_lookup(conn->file_handle_hash, (gpointer)path);
	if (!f) {
		if (!gnome_vfs_uri_has_parent(uri)) {
			/* we don't have a parent - can't go up for a file handle in
			 * the cache, so return a generic error
			 */
			retval = GNOME_VFS_ERROR_GENERIC;
			goto error;
		} else {
			/* get a cached file handle on our parent, if possible */
			parent = gnome_vfs_uri_get_parent(uri);
			retval = fhandle_recurse(parent, conn, &p);
			if ((retval != GNOME_VFS_OK) || (p == NULL)) {
				goto error;
			}
			/* wow, we now have a file handle on our parent - do a 
			 * LOOKUP and get our file handle
			 */
			retval = fhandle_recurse_lookup(uri, conn, p, &f);
			if ((retval != GNOME_VFS_OK) || (f == NULL)) {
				goto error;
			}
			/* now that we have the file handle, save it for posterity */

			g_hash_table_insert(conn->file_handle_hash, (gpointer)path, (gpointer)f);
			
			*fh = f;
			retval = GNOME_VFS_OK;
		}
	} else {
		*fh = f;
		retval = GNOME_VFS_OK;
	}

error:
	if (retval != GNOME_VFS_OK) {
		*fh = NULL;
		g_free(path);
	} else {
		*fh = f;
	}
	return retval;
}

static GnomeVFSResult
fhandle_acquire (GnomeVFSURI *uri, NfsServerConnection *conn, NfsFileHandle **fh)
{
	char *path;
	NfsFileHandle *f = NULL;
	fhstatus s;
	enum clnt_stat clnt_stat;
	int retval;

	g_assert(uri != NULL);
	g_assert(conn != NULL);

	/* lock the file handle hash */
	g_mutex_lock(conn->file_handle_hash_mutex);

	path = g_strdup(gnome_vfs_uri_get_path(uri));
	nfs_strip_last_slash(path);

	/* now, try and get a file handle for ourselves.. */
	retval = fhandle_recurse(uri, conn, &f);

	if (!f) {
		f = g_malloc(sizeof(struct NfsFileHandle));

		/* this call informs the server we want a mount on the specified 
		 * export, and gives us a file handle.
		 */
		memset((char *)&s, 0, sizeof(s));
		g_mutex_lock(conn->mount_sock_mutex);
		if ((clnt_stat = nfs_clnt_call(conn->mount_client, MOUNTPROC_MNT, 
					       (xdrproc_t)xdr_dirpath, 
					       (caddr_t)&path, 
					       (xdrproc_t)xdr_fhstatus, 
					       (caddr_t)&s, 
					       conn->mount_timeval, 
					       &s.fhs_status)) != RPC_SUCCESS) {
			clnt_perror(conn->mount_client, "MOUNTPROC_MNT");
			G_UNLOCK (server_connection_list);
			retval = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
			g_mutex_unlock(conn->mount_sock_mutex);
			goto error;
		}
		g_mutex_unlock(conn->mount_sock_mutex);
		if (s.fhs_status) {
			G_UNLOCK (server_connection_list);
			retval = gnome_vfs_result_from_errno_code(s.fhs_status);
			goto error;
		}
		memcpy(f->handle.data, s.fhstatus_u.fhs_fhandle, NFS_FHSIZE * sizeof(char));
		f->uri = uri;
		f->mounted = TRUE;
		/* cache the file handle */
		g_hash_table_insert(conn->file_handle_hash, (gpointer)path, (gpointer)f);
		retval = GNOME_VFS_OK;
	}

error:
	if (retval == GNOME_VFS_OK) {
		*fh = f;
	} else {
		g_free(path);
	}
	/* unlock the file handle hash */
	g_mutex_unlock(conn->file_handle_hash_mutex);
	return retval;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)

{
	GnomeVFSResult result;
	NfsOpenHandle *handle = g_new(NfsOpenHandle, 1);

	if ((mode != GNOME_VFS_OPEN_READ) && (mode != GNOME_VFS_OPEN_WRITE)) {
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	}
	
	result = server_connection_acquire(uri, &handle->conn);
	if (result != GNOME_VFS_OK) {
		goto error;
	}
	result = fhandle_acquire(uri, handle->conn, &handle->handle);
	if (result != GNOME_VFS_OK) {	
		goto error;
	}

	handle->uri = uri;
	if (mode == GNOME_VFS_OPEN_READ) {
		handle->operation = NFS_READ;
	} else if (mode == GNOME_VFS_OPEN_WRITE) {
		handle->operation = NFS_WRITE;
	}
	handle->position = 0;

	result = GNOME_VFS_OK;

error:
	if (result == GNOME_VFS_OK) {
		*method_handle = (GnomeVFSMethodHandle *)handle;
	} else {
		*method_handle = NULL;
		g_free(handle);
	}

	return result;
}

static GnomeVFSResult
nfs_create (GnomeVFSURI *uri,
           NfsServerConnection *conn,
	   NfsFileHandle *fh,
	   guint perm, 
	   NfsFileHandle **fh_res)
{
	GnomeVFSResult result;
	createargs c;
	diropres res;
	enum clnt_stat clnt_stat;

	g_print("NFS_METHOD: nfs_create -- %s\n", gnome_vfs_uri_get_path(uri));
	memcpy(c.where.dir.data, fh->handle.data, NFS_FHSIZE * sizeof(char));
	c.where.name = g_strdup(gnome_vfs_uri_get_basename(uri));
	c.attributes.mode = perm;
	c.attributes.uid = getuid();
	c.attributes.gid = getgid();
	c.attributes.size = 0;
	gettimeofday((struct timeval *)&c.attributes.atime, (struct timezone *)NULL);
	gettimeofday((struct timeval *)&c.attributes.mtime, (struct timezone *)NULL);
	
	g_print("NFS_METHOD: Creating file: %s\n", c.where.name);

	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_CREATE,
			(xdrproc_t)xdr_createargs, (caddr_t)&c,
			(xdrproc_t)xdr_diropres, (caddr_t)&res, 
			conn->nfs_timeval, &res.status)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "create");
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		g_mutex_unlock(conn->nfs_sock_mutex);
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);

	if(res.status != 0) {
		g_print("NFS_METHOD: create error... %s\n", strerror(res.status));
		result = gnome_vfs_result_from_errno_code(res.status);
		goto error;
	}
	g_print("NFS_METHOD: File created!\n");
	result = GNOME_VFS_OK;

error:
	return GNOME_VFS_OK;
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
	GnomeVFSResult result;
	NfsOpenHandle *handle = g_new(NfsOpenHandle, 1);
	GnomeVFSURI *parent;
	NfsFileHandle *parent_handle;

	parent = gnome_vfs_uri_get_parent(uri);

	if ((mode != GNOME_VFS_OPEN_READ) && (mode != GNOME_VFS_OPEN_WRITE)) {
		result = GNOME_VFS_ERROR_INVALID_OPEN_MODE;
		goto error;
	}
	
	result = server_connection_acquire(uri, &handle->conn);
	if (result != GNOME_VFS_OK) {
		goto error;
	}
	result = fhandle_acquire(parent, handle->conn, &parent_handle);
	if (result != GNOME_VFS_OK) {	
		goto error;
	}
	result = nfs_create(uri, handle->conn, handle->handle, perm, &handle->handle);
	if (result != GNOME_VFS_OK) {	
		goto error;
	}

	handle->uri = uri;
	if (mode == GNOME_VFS_OPEN_READ) {
		handle->operation = NFS_READ;
	} else if (mode == GNOME_VFS_OPEN_WRITE) {
		handle->operation = NFS_WRITE;
	}

	handle->position = 0;

error:
	if (result == GNOME_VFS_OK) {
		*method_handle = (GnomeVFSMethodHandle *)handle;
	} else {
		*method_handle = NULL;
		g_free(handle);
	}

	return result;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)

{
	NfsOpenHandle *handle;

	handle = (NfsOpenHandle *)method_handle;
	g_free(handle);

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
	GnomeVFSResult result;
	NfsOpenHandle *h;
	enum clnt_stat clnt_stat;
	readargs r;
	readres res;
	readokres o;
	fattr f;


	h = (NfsOpenHandle *)method_handle;

#ifdef NFS_VERBOSE_DEBUG
	g_print("NFS_METHOD: do_read %s at offset %d (%lld bytes)\n", 
			gnome_vfs_uri_to_string(h->uri, 0), 
			h->position, 
			(unsigned long long)num_bytes);
#endif

	memset((char *)&res, 0, sizeof(res));
	memset((char *)&r, 0, sizeof(r));
	memcpy(r.file.data, h->handle->handle.data, sizeof(char) * NFS_FHSIZE);
	r.offset = h->position;
	r.count = num_bytes;

	g_mutex_lock(h->conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(h->conn->nfs_client, NFSPROC_READ,
		(xdrproc_t)xdr_readargs, (caddr_t)&r,
		(xdrproc_t)xdr_readres, (caddr_t)&res,
		h->conn->nfs_timeval, &res.status)) != RPC_SUCCESS) {
		clnt_perror(h->conn->nfs_client, "read");
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		g_mutex_unlock(h->conn->nfs_sock_mutex);
		goto error;
	}
	g_mutex_unlock(h->conn->nfs_sock_mutex);
	if (res.status) {
		g_print("NFS_METHOD: read error: %s\n", strerror(res.status));
		result = gnome_vfs_result_from_errno_code(res.status);
		goto error;
	}
	o = res.readres_u.reply;
	f = o.attributes;
	memcpy(buffer, o.data.data_val, o.data.data_len);
	*bytes_read = o.data.data_len;
	if ((o.data.data_len == 0) || (h->position >= f.size)) {
		result = GNOME_VFS_ERROR_EOF;
		goto error;
	}
	h->position += o.data.data_len;

	result = GNOME_VFS_OK;

error:
	if (result != GNOME_VFS_OK) {
		*bytes_read = 0;
	}
	if (o.data.data_val) {
		free(o.data.data_val);
	}
#ifdef NFS_VERBOSE_DEBUG
	g_print("NFS_METHOD: do_read %s complete (%lld bytes read)\n", 
			gnome_vfs_uri_to_string(h->uri, 0), 
			(unsigned long long)*bytes_read);
#endif
	return result;
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)


{
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition whence,
	 GnomeVFSFileOffset offset,
	 GnomeVFSContext *context)

{
	NfsOpenHandle *h = (NfsOpenHandle *)method_handle;
	NfsServerConnection *conn;
	GnomeVFSFileInfo file_info;
	int newpos;
	int result;

	result = server_connection_acquire(h->uri, &conn);
	if (result != GNOME_VFS_OK) {
		goto error;
	}
	result = nfs_get_attr(h->uri, conn, &file_info);
	if (result != GNOME_VFS_OK) {
		goto error;
	}

	switch (whence) {
		case GNOME_VFS_SEEK_START:
			newpos = offset;
			break;
		case GNOME_VFS_SEEK_CURRENT:
			newpos = h->position + offset;
			break;
		case GNOME_VFS_SEEK_END:
			newpos = file_info.size - offset;
			break;
		default:
			return GNOME_VFS_ERROR_GENERIC;
	}

	if ((newpos > 0) && (newpos < file_info.size)) {
		h->position = newpos;
		result = GNOME_VFS_OK;
	} else {
		// FIXME - what should this be?
		result = GNOME_VFS_ERROR_GENERIC;
	}

	result = GNOME_VFS_OK;

error:
	return result;
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset *offset_return)


{
	NfsOpenHandle *h = (NfsOpenHandle *)method_handle;

	*offset_return = h->position;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)

{
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
do_truncate (GnomeVFSMethod *method,
	     GnomeVFSURI *uri,
	     GnomeVFSFileSize where,
	     GnomeVFSContext *context)

{
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

#if 0
static GnomeVFSResult 
nfs_export_list(GnomeVFSURI *uri, 
		GList **exportlist) 
{
	CLIENT *client;
	struct timeval timeval = { NFS_TIMEOUT, 0 };
	char *argp;
	exports e;
	groups g;

	printf("NFS export list called.\n");

	client = clnt_create(gnome_vfs_uri_get_host_name(uri), 
			MOUNTPROG, MOUNTVERS, "udp");
	if (client == NULL) {
		clnt_pcreateerror("gnfs_create_exports");
		return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	}
	client->cl_auth = authunix_create_default();
	memset(&e, 0, sizeof(e));
	if (nfs_clnt_call(client, MOUNTPROC_EXPORT, 
		(xdrproc_t)xdr_void, (char *)&argp, 
		(xdrproc_t)xdr_exports, (char *)&e, timeval, NULL) != RPC_SUCCESS) {
		return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	}
	while (e != NULL) {
		GnomeVFSFileInfo *info = gnome_vfs_file_info_new();
		char *name;
		const char *basename;
		basename = gnome_vfs_uri_get_basename(uri);
		if (basename[0]=='/' && basename[1]) {
			name = e->ex_dir+1;
		} else {
			name = e->ex_dir;
		}

		info->name = g_strdup(name);
		info->mime_type = g_strdup("x-directory/normal");
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->flags = GNOME_VFS_FILE_FLAGS_SYMLINK;
		info->symlink_name = g_strdup(name);
		info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
			GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE |
			GNOME_VFS_FILE_INFO_FIELDS_FLAGS |
			GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME;
		g_print("NFS_METHOD: got: %s (%s) ", e->ex_dir, name);
		g = e->ex_groups;
		while (g != NULL) {
			/* TODO guess permissions */
			g_print("NFS_METHOD: %s ", g->gr_name);
			g = g->gr_next;
		}
		e = e->ex_next;
		g_print("\n");

		*exportlist = g_list_append(*exportlist, info);
	}
	printf("NFS_METHOD: And returned.\n");

	return GNOME_VFS_OK;
}
#endif 

static GnomeVFSResult
nfs_attr_to_file_info(fattr f, GnomeVFSFileInfo *info)
{
	switch (f.type) {
	case NFREG:      /* regular file */
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		break;
	case NFDIR:      /* directory */
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		break;
	case NFBLK:      /* block special */
		info->type = GNOME_VFS_FILE_TYPE_BLOCK_DEVICE;
		break;
	case NFCHR:      /* character special */
		info->type = GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE;
		break;
	case NFLNK:      /* symbolic link */
		info->type = GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK;
		break;
	case NFSOCK:     /* unix domain sockets */
		info->type = GNOME_VFS_FILE_TYPE_SOCKET;
		break;
	case NFFIFO:     /* named pipe */
		info->type = GNOME_VFS_FILE_TYPE_FIFO;
		break;

	case NFBAD:      /* unused */
        case NFNON:      /* non-file */
	default:
		info->type = GNOME_VFS_FILE_TYPE_UNKNOWN;
	}

	info->permissions = f.mode;
	info->device = f.fsid;
	info->inode = f.fileid;
	info->link_count = f.nlink;
	info->uid = f.uid;
	info->gid = f.gid;
	info->size = f.size;
	info->block_count = f.blocks;
	info->io_block_size = f.blocksize;

	info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE |
		GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS |
		GNOME_VFS_FILE_INFO_FIELDS_FLAGS |
		GNOME_VFS_FILE_INFO_FIELDS_DEVICE |
		GNOME_VFS_FILE_INFO_FIELDS_INODE |
		GNOME_VFS_FILE_INFO_FIELDS_LINK_COUNT |
		GNOME_VFS_FILE_INFO_FIELDS_SIZE |
		GNOME_VFS_FILE_INFO_FIELDS_BLOCK_COUNT |
		GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
nfs_get_attr(GnomeVFSURI *uri, NfsServerConnection *conn, GnomeVFSFileInfo *info)
{
	GnomeVFSResult result;
	enum clnt_stat clnt_stat;
	attrstat a;
	nfs_fh h;
	NfsFileHandle *f;


	if (fhandle_acquire(uri, conn, &f) != GNOME_VFS_OK) {
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		goto error;
	}

	memset((char *)&h, 0, sizeof(nfs_fh));
	memcpy(h.data, f->handle.data, NFS_FHSIZE * sizeof(char));
	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_GETATTR, 
			(xdrproc_t)xdr_nfs_fh, (caddr_t)&h,
			(xdrproc_t)xdr_attrstat, (caddr_t)&a,
			conn->nfs_timeval, &a.status)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "getattr");
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);
	if (a.status) {
		g_print("NFS_METHOD: getattr error - %s\n", strerror(a.status));
		result = gnome_vfs_result_from_errno_code(a.status);
		goto error;
	}
	result = nfs_attr_to_file_info(a.attrstat_u.attributes, info);
error:
	return result;
}

static GnomeVFSResult
nfs_lookup(NfsServerConnection *conn, NfsFileHandle *fh, char *name, GnomeVFSFileInfo *info)
{
	enum clnt_stat clnt_stat;
	diropargs args;
	diropres res;
	GnomeVFSResult result;

	args.name = name;
	memset((char *)&args.dir, 0, sizeof(nfs_fh));
	memcpy(args.dir.data, fh->handle.data, NFS_FHSIZE * sizeof(char));

	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_LOOKUP, 
			(xdrproc_t)xdr_diropargs, (caddr_t)&args,
			(xdrproc_t)xdr_diropres, (caddr_t)&res,
			conn->nfs_timeval, &res.status)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "lookup");
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		g_mutex_unlock(conn->nfs_sock_mutex);
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);

	if (res.status) {
		g_print("NFS_METHOD: lookup error - %s\n", strerror(res.status));
		result = gnome_vfs_result_from_errno_code(res.status);
		goto error;
	}

	if (info) {
		result = nfs_attr_to_file_info(
				res.diropres_u.diropres.attributes, info);
	}

	result = GNOME_VFS_OK;

error:
	return result;
}

//#define DIRSIZE (1024*64)
#define DIRSIZE (1024*8)

#if 0
static GnomeVFSResult
nfs_file_list_do(NfsServerConnection *conn, NfsFileHandle *dir, GList **list, readdirargs *rdargs, readdirres *rdres)
{
	dirlist *rddirlist;
	entry *next;
	enum clnt_stat clnt_stat;
	GnomeVFSResult result = GNOME_VFS_OK;

	memcpy(rdargs->dir.data, dir->handle.data, NFS_FHSIZE * sizeof(char));
	memset(&rddirlist, 0, sizeof(rddirlist));

	/* this code does a READDIR */
	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_READDIR,
			(xdrproc_t)xdr_readdirargs, (caddr_t)rdargs,
			(xdrproc_t)xdr_readdirres, (caddr_t)rdres, 
			conn->nfs_timeval, &rdres.status)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "readdir");
		g_print("NFS_METHOD: %d - bailing\n", clnt_stat);
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		g_mutex_unlock(conn->nfs_sock_mutex);
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);
	if(rdres->status != 0) {
		g_print("NFS_METHOD: readdir error... %s\n", strerror(rdres->status));
		result = gnome_vfs_result_from_errno_code(rdres->status);
		goto error;
	}

	rddirlist = &rdres->readdirres_u.reply;
	next = rddirlist->entries;
	while(next) {
		GnomeVFSFileInfo *info;

		/* special case; nautilus dies if we return these
		 * specifically, a do_open_directory is done on '.' and then 
		 * nautilus dies on a hash table assert.
		 */
		if (!strcmp(next->name, ".") || !strcmp(next->name, "..")) {
			next = next->nextentry;
			continue;
		}
		
		info = gnome_vfs_file_info_new();

		info->name = g_strdup(next->name);
		nfs_lookup(conn, dir, next->name, info);
		if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			info->mime_type = g_strdup("x-directory/normal");
		} else {
			info->mime_type = g_strdup( 
				gnome_vfs_mime_type_from_name_or_default( 
						next->name, "text/plain"));
		}
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		*list = g_list_append(*list, info);
		memcpy(rdargs->cookie, &next->cookie, NFS_COOKIESIZE);
		next = next->nextentry;
	}
error:
	return result;
}
#endif

#if 0
static GnomeVFSResult
nfs_file_list(NfsServerConnection *conn, NfsFileHandle *dir, GList **list)
{
	GnomeVFSResult result = GNOME_VFS_OK;
	readdirargs rdargs;
	readdirres rdres;

	memset(&rdargs, 0, sizeof(rdargs));
	memset(&rdres, 0, sizeof(rdres));

	rdargs.count = DIRSIZE;
	while (1) {
		result = nfs_file_list_do(conn, dir, list, &rdargs, &rdres);
		if (result != GNOME_VFS_OK) {
			goto error;
		}
		if (rdres.readdirres_u.reply.eof) {
			break;
		}
	}
error:
	return result;
}
#endif

static GnomeVFSResult
nfs_file_list(NfsServerConnection *conn, NfsFileHandle *dir, GList **list) 
{
	GnomeVFSResult result;
	enum clnt_stat clnt_stat;
	readdirargs *rdargs=g_new(readdirargs, 1);
	readdirres *rdres=g_malloc(DIRSIZE);
	entry *next;
	dirlist *rddirlist;


	memset((char *)&rdargs->dir, 0, sizeof(nfs_fh));
	memcpy(rdargs->dir.data, dir->handle.data, NFS_FHSIZE * sizeof(char));
	memset(rdargs->cookie, 0, NFS_COOKIESIZE);

	rdargs->count = DIRSIZE;

	while (1) {
		bzero(rdres, DIRSIZE);

		/* this code does a READDIR */
		g_mutex_lock(conn->nfs_sock_mutex);
		if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_READDIR,
					       (xdrproc_t)xdr_readdirargs, 
					       (caddr_t)rdargs,
					       (xdrproc_t)xdr_readdirres, 
					       (caddr_t)rdres, 
					       conn->nfs_timeval, 
					       &rdres->status)) != RPC_SUCCESS) {
			clnt_perror(conn->nfs_client, "readdir");
			g_print("NFS_METHOD: %d - bailing\n", clnt_stat);
			result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
			g_mutex_unlock(conn->nfs_sock_mutex);
			goto error;
		}
		g_mutex_unlock(conn->nfs_sock_mutex);
		if(rdres->status != 0) {
			g_print("NFS_METHOD: readdir error... %s\n", strerror(rdres->status));
			result = gnome_vfs_result_from_errno_code(rdres->status);
			goto error;
		}

		rddirlist = &rdres->readdirres_u.reply;
		next = rddirlist->entries;
		while(next) {
			GnomeVFSFileInfo *info;

			/* special case; nautilus dies if we return these
			 * specifically, a do_open_directory is done on '.' and then 
			 * nautilus dies on a hash table assert.
			 */
			if (!strcmp(next->name, ".") || !strcmp(next->name, "..")) {
				next = next->nextentry;
				continue;
			}
		
			info = gnome_vfs_file_info_new();

			info->name = g_strdup(next->name);
			nfs_lookup(conn, dir, next->name, info);
			if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
				info->mime_type = g_strdup("x-directory/normal");
			} else {
				info->mime_type = g_strdup( 
					gnome_vfs_mime_type_from_name_or_default( 
						next->name, "text/plain"));
			}
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
			*list = g_list_append(*list, info);
			memcpy(rdargs->cookie, &next->cookie, NFS_COOKIESIZE);
			next = next->nextentry;
		}

		if(rddirlist->eof) {
			/* we've finished traversing the directory list */
			break;
		}
	}

	result = GNOME_VFS_OK;

error:
	g_free(rdargs);
	g_free(rdres);
	return result;
}

static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)

{
	NfsServerConnection *conn;
	NfsFileHandle *f;
       	GnomeVFSResult result;
	NfsDirectoryHandle *handle = g_new(NfsDirectoryHandle, 1);

	handle->files = NULL;

	result = server_connection_acquire(uri, &conn);
	if (result != GNOME_VFS_OK) goto error;
	result = fhandle_acquire(uri, conn, &f);
	if (result != GNOME_VFS_OK) goto error;

	result = nfs_file_list(conn, f, &handle->files);
	if (result != GNOME_VFS_OK) goto error;

	result = GNOME_VFS_OK;

error:	
	if (result != GNOME_VFS_OK) {
		g_free(handle);
	} else {
		handle->uri = uri;
		gnome_vfs_uri_ref(handle->uri);
//		gnome_vfs_file_info_list_ref(handle->files);
		handle->pointer = handle->files;
		*method_handle = (GnomeVFSMethodHandle *)handle;
	}

	return result;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	NfsDirectoryHandle *handle = (NfsDirectoryHandle *)method_handle;
	gnome_vfs_uri_unref(handle->uri);
	gnome_vfs_file_info_list_unref(handle->files);
	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	NfsDirectoryHandle *handle = (NfsDirectoryHandle *)method_handle;
	if (handle->pointer) {
		/* is this leaking refs? */
		gnome_vfs_file_info_copy(file_info, 
				(GnomeVFSFileInfo *)handle->pointer->data);
		handle->pointer = handle->pointer->next;

		if (file_info->type == GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK) {
			printf("symbollic link debug: %s\n", file_info->symlink_name);
		} else {
//			printf("Not a symlink: %s\n", file_info->name);
		}

		return GNOME_VFS_OK;
	} else {
		return GNOME_VFS_ERROR_EOF;
	}
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)

{
	GnomeVFSResult result;

	if (uri->text == NULL || uri->text[0] == '\0') {
		file_info->name = g_strdup(gnome_vfs_uri_get_host_name(uri));
		file_info->mime_type = g_strdup("x-directory/normal");
		file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		file_info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
			GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		return GNOME_VFS_OK;
	} else {
		NfsServerConnection *conn;
		result = server_connection_acquire(uri, &conn);
		if (result != GNOME_VFS_OK) return result;

		result = nfs_get_attr(uri, conn, file_info);
		if (result != GNOME_VFS_OK) return result;

		file_info->name = gnome_vfs_uri_extract_short_name(uri);
		if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			if (file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
				file_info->mime_type = 
					g_strdup("x-directory/normal");
			} else {
				file_info->mime_type = 
					g_strdup("text/plain");
			}
			file_info->valid_fields |= 
				GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
		return GNOME_VFS_OK;
	}
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)


{
	NfsServerConnection *conn;
	NfsOpenHandle *h = (NfsOpenHandle *)method_handle;
	GnomeVFSResult result;

	result = server_connection_acquire(h->uri, &conn);
	if (result == GNOME_VFS_OK) return result;

	result = nfs_get_attr(h->uri, conn, file_info);
	if (result == GNOME_VFS_OK) return result;

	file_info->name = gnome_vfs_uri_extract_short_name(h->uri);
	if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
		if (file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			file_info->mime_type = 
				g_strdup("x-directory/normal");
		} else {
			file_info->mime_type = 
				g_strdup("text/plain");
		}
		file_info->valid_fields |= 
			GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
	}
	g_print("NFS_METHOD: get_file_info worked...\n");
	return GNOME_VFS_OK;
}

static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)


{
	/* should always consider it to not be local */
	return FALSE;
}

static GnomeVFSResult
nfs_mkdir (GnomeVFSURI *uri,
           NfsServerConnection *conn,
	   NfsFileHandle *fh,
	   guint perm)
{
	GnomeVFSResult result;
	createargs c;
	diropres res;
	enum clnt_stat clnt_stat;

	memcpy(c.where.dir.data, fh->handle.data, NFS_FHSIZE * sizeof(char));
	c.where.name = g_strdup(gnome_vfs_uri_get_basename(uri));
	c.attributes.mode = perm;
	c.attributes.uid = getuid();
	c.attributes.gid = getgid();
	printf("creating directory with uid, gid = %d, %d\n", c.attributes.uid, c.attributes.gid);
	c.attributes.size = -1;
	gettimeofday((struct timeval *)&c.attributes.atime, (struct timezone *)NULL);
	gettimeofday((struct timeval *)&c.attributes.mtime, (struct timezone *)NULL);

	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_MKDIR,
			(xdrproc_t)xdr_createargs, (caddr_t)&c,
			(xdrproc_t)xdr_diropres, (caddr_t)&res, 
			conn->nfs_timeval, &res.status)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "mkdir");
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		g_mutex_unlock(conn->nfs_sock_mutex);
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);
	g_free(c.where.name);
	if (res.status != 0) {
		g_print("NFS_METHOD: mkdir error... %s\n", strerror(res.status));
		result = gnome_vfs_result_from_errno_code(res.status);
		goto error;
	}
	g_print("NFS_METHOD: Directory created! (%d, %d)\n", res.diropres_u.diropres.attributes.uid, res.diropres_u.diropres.attributes.gid);

	result = GNOME_VFS_OK;

error:
	return result;

}

static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   guint perm,
		   GnomeVFSContext *context)
{
	GnomeVFSResult result;
	GnomeVFSURI *parent;

	g_print("NFS_METHOD: make directory(%s)\n", gnome_vfs_uri_to_string(uri,0));
	if (uri->text == NULL || uri->text[0] == '\0') {
		/* remove the root? */
		return GNOME_VFS_ERROR_GENERIC;
	} else {
		NfsServerConnection *conn;
		NfsFileHandle *fh;

		if (!gnome_vfs_uri_has_parent(uri)) {
			return GNOME_VFS_ERROR_GENERIC;
		}
		parent = gnome_vfs_uri_get_parent(uri);

		result = server_connection_acquire(parent, &conn);
		if (result != GNOME_VFS_OK) return result;
		result = fhandle_acquire(parent, conn, &fh);
		if (result != GNOME_VFS_OK) return result;

		result = nfs_mkdir(uri, conn, fh, perm);
		if (result != GNOME_VFS_OK) return result;

		g_print("NFS_METHOD: mkdir worked...\n");
		return GNOME_VFS_OK;
	}
}

static GnomeVFSResult
nfs_rmdir (GnomeVFSURI *uri,
           NfsServerConnection *conn,
	   NfsFileHandle *f)
{
	GnomeVFSResult result;
	diropargs d;
	nfsstat s;
	enum clnt_stat clnt_stat;

	memcpy(d.dir.data, f->handle.data, NFS_FHSIZE * sizeof(char));
	d.name = NULL;
	d.name = (char *)gnome_vfs_uri_get_basename(uri);

	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_RMDIR,
			(xdrproc_t)xdr_diropargs, (caddr_t)&d,
			(xdrproc_t)xdr_nfsstat, (caddr_t)&s, 
			conn->nfs_timeval, &s)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "readdir");
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		g_mutex_unlock(conn->nfs_sock_mutex);
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);
	if(s != 0) {
		g_print("NFS_METHOD: rmdir error... %s\n", strerror(s));
		result = gnome_vfs_result_from_errno_code(s);
		goto error;
	}
	g_print("NFS_METHOD: Directory removed!\n");
	result = GNOME_VFS_OK;
error:
	return result;
}

static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	GnomeVFSResult result;
	GnomeVFSURI *parent;

	g_print("NFS_METHOD: remove directory(%s)\n", gnome_vfs_uri_to_string(uri,0));
	if (uri->text == NULL || uri->text[0] == '\0') {
		/* remove the root? */
		return GNOME_VFS_ERROR_GENERIC;
	} else {
		NfsServerConnection *conn;
		NfsFileHandle *fh;

		if (!gnome_vfs_uri_has_parent(uri)) {
			return GNOME_VFS_ERROR_GENERIC;
		}
		parent = gnome_vfs_uri_get_parent(uri);

		result = server_connection_acquire(parent, &conn);
		if (result != GNOME_VFS_OK) return result;
		result = fhandle_acquire(parent, conn, &fh);
		if (result != GNOME_VFS_OK) return result;

		result = nfs_rmdir(uri, conn, fh);
		if (result != GNOME_VFS_OK) return result;

		g_print("NFS_METHOD: rmdir worked...\n");
		return GNOME_VFS_OK;
	}
}

static GnomeVFSResult
do_find_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *near_uri,
		   GnomeVFSFindDirectoryKind kind,
		   GnomeVFSURI **result_uri,
		   gboolean create_if_needed,
		   gboolean find_if_needed,
		   guint permissions,
		   GnomeVFSContext *context)
{
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
nfs_unlink (GnomeVFSURI *uri,
           NfsServerConnection *conn,
	   NfsFileHandle *f)
{
	GnomeVFSResult result;
	diropargs d;
	nfsstat s;
	enum clnt_stat clnt_stat;

	memcpy(d.dir.data, f->handle.data, NFS_FHSIZE * sizeof(char));
	d.name = NULL;
	d.name = (char *)gnome_vfs_uri_get_basename(uri);

	g_mutex_lock(conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(conn->nfs_client, NFSPROC_REMOVE,
			(xdrproc_t)xdr_diropargs, (caddr_t)&d,
			(xdrproc_t)xdr_nfsstat, (caddr_t)&s, 
			conn->nfs_timeval, &s)) != RPC_SUCCESS) {
		clnt_perror(conn->nfs_client, "readdir");
		result = GNOME_VFS_ERROR_GENERIC;
		g_mutex_unlock(conn->nfs_sock_mutex);
		goto error;
	}
	g_mutex_unlock(conn->nfs_sock_mutex);
	g_print("NFS_METHOD: status = %d\n", s);
	if(s != 0) {
		g_print("NFS_METHOD: remove error... %s\n", strerror(s));
		result = gnome_vfs_result_from_errno_code(s);
		goto error;
	}

	result = GNOME_VFS_OK;
error:
	return result;
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method,
	   GnomeVFSURI *uri,
	   GnomeVFSContext *context)
{
	GnomeVFSResult result;
	GnomeVFSURI *parent;

	g_print("NFS_METHOD: remove file(%s)\n", gnome_vfs_uri_to_string(uri,0));
	if (uri->text == NULL || uri->text[0] == '\0') {
		/* remove the root? */
		return GNOME_VFS_ERROR_GENERIC;
	} else {
		NfsServerConnection *conn;
		NfsFileHandle *f;

		if (!gnome_vfs_uri_has_parent(uri)) {
			return GNOME_VFS_ERROR_GENERIC;
		}
		parent = gnome_vfs_uri_get_parent(uri);

		result = server_connection_acquire(parent, &conn);
		if (result != GNOME_VFS_OK) return result;
		result = fhandle_acquire(parent, conn, &f);
		if (result != GNOME_VFS_OK) return result;

		result = nfs_unlink(uri, conn, f);
		if (result != GNOME_VFS_OK) return result;

		g_print("NFS_METHOD: unlink worked...\n");
		return GNOME_VFS_OK;
	}
}


static GnomeVFSResult
nfs_rename (GnomeVFSURI *from, GnomeVFSURI *to,
           NfsServerConnection *from_conn, NfsServerConnection *to_conn, 
	   NfsFileHandle *from_dir, NfsFileHandle *to_dir)
{
	GnomeVFSResult result;
	renameargs r;
	nfsstat s;
	enum clnt_stat clnt_stat;


	memcpy(r.from.dir.data, from_dir->handle.data, NFS_FHSIZE * sizeof(char));
	memcpy(r.to.dir.data, to_dir->handle.data, NFS_FHSIZE * sizeof(char));

	r.from.name = (char *)gnome_vfs_uri_get_basename(from);
	r.to.name = (char *)gnome_vfs_uri_get_basename(to);

	g_mutex_lock(from_conn->nfs_sock_mutex);
	if ((clnt_stat = nfs_clnt_call(from_conn->nfs_client, NFSPROC_RENAME,
			(xdrproc_t)xdr_renameargs, (caddr_t)&r,
			(xdrproc_t)xdr_nfsstat, (caddr_t)&s, 
			from_conn->nfs_timeval, &s)) != RPC_SUCCESS) {
		clnt_perror(from_conn->nfs_client, "readdir");
		result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
		goto error;
	}
	g_mutex_unlock(from_conn->nfs_sock_mutex);
	g_print("NFS_METHOD: status = %d\n", s);
	if(s != 0) {
		g_print("NFS_METHOD: mkdir error... %s\n", strerror(s));
		result = gnome_vfs_result_from_errno_code(s);
		goto error;
	}
	g_print("NFS_METHOD: File moved!\n");

	result = GNOME_VFS_OK;
error:
	return result;
}


static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	GnomeVFSResult result;
	GnomeVFSURI *from_parent, *to_parent;


	g_print("NFS_METHOD: move file(%s) to (%s)\n", gnome_vfs_uri_to_string(old_uri,0), 
		gnome_vfs_uri_to_string(new_uri,0));
	if ((new_uri->text == NULL || new_uri->text[0] == '\0')
		|| (old_uri->text == NULL || old_uri->text[0] == '\0')) {
		/* huh */
		return GNOME_VFS_ERROR_GENERIC;
	} else {
		NfsServerConnection *from_conn;
		NfsServerConnection *to_conn;
		NfsFileHandle *from_handle;
		NfsFileHandle *to_handle;

		if (!gnome_vfs_uri_has_parent(old_uri)) {
			return GNOME_VFS_ERROR_GENERIC;
		}
		if (!gnome_vfs_uri_has_parent(new_uri)) {
			return GNOME_VFS_ERROR_GENERIC;
		}
		from_parent = gnome_vfs_uri_get_parent(old_uri);
		to_parent = gnome_vfs_uri_get_parent(new_uri);

		result = server_connection_acquire(from_parent, &from_conn);
		if (result != GNOME_VFS_OK) return result;
		result = server_connection_acquire(to_parent, &to_conn);
		if (result != GNOME_VFS_OK) return result;
		result = fhandle_acquire(from_parent, from_conn, &from_handle);
		if (result != GNOME_VFS_OK) return result;
		result = fhandle_acquire(from_parent, to_conn, &to_handle);
		if (result != GNOME_VFS_OK) return result;

		result = nfs_rename(old_uri, new_uri, from_conn, to_conn, from_handle, to_handle);
		if (result != GNOME_VFS_OK) return result;

		g_print("NFS_METHOD: move worked...\n");
		return GNOME_VFS_OK;
	}
}

static GnomeVFSResult
do_create_symbolic_link (GnomeVFSMethod *method,
			 GnomeVFSURI *uri,
			 const char *target_reference,
			 GnomeVFSContext *context)
{
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *a,
		  GnomeVFSURI *b,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}


static void
nfs_hash_foreach (gpointer key, gpointer value, gpointer user_data)
{
	NfsFileHandle *f;
	NfsServerConnection *c = (NfsServerConnection *)user_data;
	enum clnt_stat clnt_stat;
	const char *path;

	f = (NfsFileHandle *)value;
	path = gnome_vfs_uri_get_path(f->uri);
	if (f->mounted) {
		/* unmount this */
		if ((clnt_stat = nfs_clnt_call(c->mount_client, MOUNTPROC_UMNT,
			(xdrproc_t)xdr_dirpath, (char *)&path,
			(xdrproc_t)xdr_void, 0,
			c->mount_timeval, NULL)) != RPC_SUCCESS) {
			clnt_perror(c->mount_client, "mountproc_umnt");
		}
	}
	g_free(f->uri);
	g_free(f);
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
	return &method;
}


void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	GList *next;
	NfsServerConnection *c;

	printf("<-- module shutdown called -->\n");
	next = server_connection_list;
	while (next) {
		c = (NfsServerConnection *)next->data;
		g_hash_table_foreach(c->file_handle_hash, nfs_hash_foreach, (gpointer)c);
		auth_destroy(c->mount_client->cl_auth);
		auth_destroy(c->nfs_client->cl_auth);
		clnt_destroy(c->mount_client);
		clnt_destroy(c->nfs_client);
		g_free(c->hostname);
		g_free(c->hp);
		g_free(c->mount_server_addr);
		g_free(c->nfs_server_addr);
		g_mutex_free(c->nfs_sock_mutex);
		g_mutex_free(c->mount_sock_mutex);
		g_mutex_free(c->file_handle_hash_mutex);
		g_free(c);
	}
}


