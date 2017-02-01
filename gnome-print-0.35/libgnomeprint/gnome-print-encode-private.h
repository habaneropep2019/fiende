/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Copyright (C) 2000 Jose M Celorio.
 *
 *  Authors: Chema Celorio <chema@celorio.com
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _GNOME_PRINT_ENCODE_PRIVATE_H
#define _GNOME_PRINT_ENCODE_PRIVATE_H

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

int gnome_print_encode_blank    (const guchar *in, gint in_size);
int gnome_print_encode_rlc      (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_tiff     (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_drow     (const guchar *in, guchar *out, gint in_size, guchar *seed);
int gnome_print_encode_hex      (const guchar *in, guchar *out, gint in_size);
int gnome_print_decode_hex      (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_ascii85  (const guchar *in, guchar *out, gint in_size);
int gnome_print_decode_ascii85  (const guchar *in, guchar *out, gint in_size);
int gnome_print_encode_deflate  (const guchar *in, guchar *out, gint in_size, gint out_size);

int gnome_print_encode_rlc_wcs     (gint size);
int gnome_print_encode_tiff_wcs    (gint size);
int gnome_print_encode_drow_wcs    (gint size);
int gnome_print_encode_hex_wcs     (gint size);
int gnome_print_decode_hex_wcs     (gint size);
int gnome_print_encode_ascii85_wcs (gint size);
int gnome_print_decode_ascii85_wcs (gint size);
int gnome_print_encode_deflate_wcs (gint size);

/* temp, just for comparing speed */
int gnome_print_encode_ascii85_ (const guchar *in, guchar *out, gint in_size);

/* gnome-print-encode virtual stopwatch */
#if 0 /* disabled by chema 20-oct-2000 */
void gnome_print_encode_timer_start (void);
void gnome_print_encode_timer_end (void);
#endif

END_GNOME_DECLS

#endif /* ! _GNOME_PRINT_ENCODE_PRIVATE_H */
