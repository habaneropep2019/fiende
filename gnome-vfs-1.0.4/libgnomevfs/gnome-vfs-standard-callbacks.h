/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*

   Copyright (C) 2001 Eazel, Inc

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

   Author: Michael Fleming <mfleming@eazel.com>
*/

#ifndef GNOME_VFS_STANDARD_CALLBACKS_H
#define GNOME_VFS_STANDARD_CALLBACKS_H

#include <glib.h>

/*
 * defined callback structures
 */

/*
 * hook name: "simple-authentication"
 * In arg: GnomeVFSModuleCallbackAuthenticationIn *
 * Out arg: GnomeVFSModuleCallbacAuthenticationOut *
 * 
 * Called when access to a URI requires a username/password
 */
#define GNOME_VFS_MODULE_CALLBACK_AUTHENTICATION "simple-authentication"

/*
 * hook name: "http:proxy-authentication"
 * In arg: GnomeVFSModuleCallbackAuthenticationIn *
 * Out arg: GnomeVFSModuleCallbackAuthenticationOut *
 * 
 * Called when access to an HTTP proxy requires a username/password
 */
#define GNOME_VFS_MODULE_CALLBACK_HTTP_PROXY_AUTHENTICATION "http:proxy-authentication"

typedef struct {
	char *uri;		/* Full URI of operation */
	char *realm;		/* for HTTP auth, NULL for others */
	gboolean previous_attempt_failed;
				/* TRUE if there were credentials specified
				 * for this request, but they resulted in
				 * an authorization error. 
				 * ("you gave me the wrong pw!")
				 * 
				 * FALSE if there were no credentials specified
				 * but they are required to continue
				 * 
				 */
	enum {
		AuthTypeBasic,	/* Password will be transmitted unencrypted */
		AuthTypeDigest	/* Digest is transferred, not plaintext credentials */		
	} auth_type;
} GnomeVFSModuleCallbackAuthenticationIn;

typedef struct {
	char *username;		/* will be freed by g_free,
				 * NULL indicates no auth should be provided;
				 * if the request requires authn, the operation
				 * will fail with a GNOME_VFS_ERROR_ACCESS_DENIED
				 * code
				 */
	char *password;		/* will be freed by g_free */
} GnomeVFSModuleCallbackAuthenticationOut;

/*
 * hook name: "status-message"
 * In arg: GnomeVFSModuleCallbackStatusMessageIn *
 * Out arg: GnomeVFSModuleCallbackStatusMessageOut *
 * 
 * Called when a GnomeVFS API or module has a status message to return to
 * the user.
 */

#define GNOME_VFS_MODULE_CALLBACK_STATUS_MESSAGE "status-message"

typedef struct {
	char *uri;		/* Full URI of operation */
	char *message;		/* A message indicating the current state or
				 * NULL if there is no message */
	int percentage;		/* Percentage indicating completeness 0-100 or
				 * -1 if there is no progress percentage to
				 * report */
} GnomeVFSModuleCallbackStatusMessageIn;

typedef struct {
	int dummy; /* empty structs not allowed */
} GnomeVFSModuleCallbackStatusMessageOut;

#endif /* GNOME_VFS_STANDARD_CALLBACKS_H */
