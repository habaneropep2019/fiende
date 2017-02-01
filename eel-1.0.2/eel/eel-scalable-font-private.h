/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-scalable-font-private.h - Private scalable font things.

   Copyright (C) 1999, 2000 Eazel, Inc.

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

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_SCALABLE_FONT_PRIVATE_H
#define EEL_SCALABLE_FONT_PRIVATE_H

#include <eel/eel-scalable-font.h>

BEGIN_GNOME_DECLS

#define EEL_SCALABLE_FONT_UNDEFINED_HANDLE -1

struct _RsvgFTCtx;

int                eel_scalable_font_get_rsvg_handle  (const EelScalableFont *font);
struct _RsvgFTCtx *eel_scalable_font_get_rsvg_context (const EelScalableFont *font);

END_GNOME_DECLS

#endif /* EEL_SCALABLE_FONT_PRIVATE_H */


