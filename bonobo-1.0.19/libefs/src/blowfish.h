/* blowfish.h - C implementation of the Blowfish algorithm. 

   Copyright (C) 1997 Paul Kocher

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

   Author: Paul Kocher <pck@netcom.com>
*/

#ifndef _EFS_BLOWFISH_H_
#define _EFS_BLOWFISH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

typedef struct {
  unsigned long P[16 + 2];
  unsigned long S[4][256];
} BlowfishCTX;


void blowfish_init      (BlowfishCTX *ctx, gchar *key, gint keylen);
void blowfish_encrypt   (BlowfishCTX *ctx, guint32 *xl, guint32 *xr);
void blowfish_decrypt   (BlowfishCTX *ctx, guint32 *xl, guint32 *xr);

#ifdef __cplusplus
}
#endif

#endif /* _EFS_BLOWFISH_H_ */
