/* http-method.h - HTTP access method for the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

#ifndef HTTP_METHOD_H
#define HTTP_METHOD_H

typedef gint64 utime_t;

utime_t http_util_get_utime (void);

gchar * http_util_base64 (const gchar *text);

#undef DEBUG_HTTP_ENABLE

#ifdef DEBUG_HTTP_ENABLE

#define DEBUG_HTTP(x) http_debug_printf x
void http_debug_printf(char *fmt, ...) G_GNUC_PRINTF (1,2);
/* #define ANALYZE_HTTP(x) my_debug_printf (x) */
#define ANALYZE_HTTP(x) 

#else /* DEBUG_HTTP_ENABLE */

#define DEBUG_HTTP(x)
#define ANALYZE_HTTP(x) 

#endif /* DEBUG_HTTP_ENABLE */

#endif /* HTTP_METHOD_H */
