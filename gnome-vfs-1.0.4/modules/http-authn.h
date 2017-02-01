/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* http-authn.h - Basic authentication handling for HTTP

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

#ifndef HTTP_AUTHN_H
#define HTTP_AUTHN_H

#include <glib.h>
#include <libgnomevfs/gnome-vfs-uri.h>

enum AuthnHeaderType {
	AuthnHeader_WWW,
	AuthnHeader_Proxy
};

void	 http_authn_init 					(void);

void	 http_authn_shutdown					(void);

char *	 http_authn_session_get_header_for_uri			(GnomeVFSURI *uri);

void	 http_authn_session_add_credentials			(GnomeVFSURI *uri,
								 const char * username,
								 const char * password);

gboolean http_authn_parse_response_header_basic			(enum AuthnHeaderType type,
								 GList *response_headers, 
								 /* OUT */ char **p_realm);

char *	 http_authn_get_header_for_uri 				(GnomeVFSURI *uri);

gboolean http_authn_self_test 					(void);


#endif /* HTTP_AUTHN_H */
