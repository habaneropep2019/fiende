#ifndef _GP_UNICODE_H_

/*
 * gp-unicode.h - Unicode manipulation functions
 *
 *  Copyright (C) 1999, 2000 Tom Tromey
 *  Copyright 2000 Red Hat, Inc.
 *  Copyright 2000 Helix Code, Inc.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 */

#include <sys/types.h>
#include <glib.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

typedef guint32 gunichar;

/* glib 2.0 stuff follows */

#ifndef _GP_UNICODE_C_
extern char g_utf8_skip[256];
#endif

#define g_utf8_next_char(p) (char *)((p) + g_utf8_skip[*(guchar *)(p)])

gunichar g_utf8_get_char          (const gchar *p);
gchar *  g_utf8_offset_to_pointer  (const gchar *str,
				    gint         offset);
gint     g_utf8_pointer_to_offset (const gchar *str,
				   const gchar *pos);
gchar *  g_utf8_prev_char         (const gchar *p);
gchar *  g_utf8_find_next_char    (const gchar *p,
				   const gchar *end);
gchar *  g_utf8_find_prev_char    (const gchar *str,
				   const gchar *p);

gint g_utf8_strlen (const gchar *p,
		    gint         max);

/* Copies n characters from src to dest */
gchar *g_utf8_strncpy (gchar       *dest,
		       const gchar *src,
		       size_t       n);

/* Convert a single character into UTF-8. outbuf must have at
 * least 6 bytes of space. Returns the number of bytes in the
 * result.
 */
gint      g_unichar_to_utf8 (gunichar    c,
			     char       *outbuf);

/* Validate a UTF8 string, return TRUE if valid, put pointer to
 * first invalid char in **end
 */

gboolean g_utf8_validate (const gchar  *str,
                          gint          max_len,
                          const gchar **end);

END_GNOME_DECLS

#endif
