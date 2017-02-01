/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-font-manager.h - Functions for managing fonts.

   Copyright (C) 2000 Eazel, Inc.

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

   Authors: Pavel Cisler <pavel@eazel.com>,
            Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_FONT_MANAGER_H
#define EEL_FONT_MANAGER_H

#include <libgnome/gnome-defs.h>
#include <glib.h>

BEGIN_GNOME_DECLS

typedef enum {
	EEL_FONT_POSTSCRIPT = 1,
	EEL_FONT_TRUE_TYPE
} EelFontType;

/*
 * A callback which can be invoked for each font available in the system.
 */
typedef gboolean (*EelFontManagerCallback) (const char *font_file_name,
					    EelFontType font_type,
					    const char *foundry,
					    const char *family,
					    const char *weight,
					    const char *slant,
					    const char *set_width,
					    const char *char_set,
					    gpointer callback_data);

void     eel_font_manager_for_each_font         (EelFontManagerCallback  callback,
						 gpointer                callback_data);
char *   eel_font_manager_get_default_font      (void);
char *   eel_font_manager_get_default_bold_font (void);
gboolean eel_font_manager_file_is_scalable_font (const char             *file_name);
char *   eel_font_manager_get_bold              (const char             *plain_font);
char *   eel_font_manager_get_italic            (const char             *plain_font);
gboolean eel_font_manager_weight_is_bold        (const char             *weight);

END_GNOME_DECLS

#endif /* EEL_FONT_MANAGER_H */


