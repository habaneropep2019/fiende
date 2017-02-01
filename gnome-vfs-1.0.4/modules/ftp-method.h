/* ftp-method.h - VFS modules for FTP

   Copyright (C) 2000 Ian McKellar

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


#ifndef FTP_METHOD_H
#define FTP_METHOD_H

#include "gnome-vfs-module.h"

typedef struct {
	GnomeVFSMethodHandle method_handle;
	GnomeVFSInetConnection *inet_connection;
	GnomeVFSIOBuf *iobuf;
	GnomeVFSURI *uri;
	gchar *cwd;
	GString *response_buffer;
	gchar *response_message;
	gint response_code;
	GnomeVFSInetConnection *data_connection;
	GnomeVFSIOBuf *data_iobuf;
	enum {
		FTP_NOTHING,
		FTP_READ,
		FTP_WRITE,
		FTP_READDIR
	} operation;
	gchar *dirlist;
	gchar *dirlistptr;
	gchar *server_type; /* the response from TYPE */
	gboolean anonymous;
	GnomeVFSResult fivefifty; /* the result to return for an FTP 550 */
	GnomeVFSFileInfoOptions file_info_options;
} FtpConnection;

#endif /* FTP_METHOD_H */
