/* GnomeCard - a graphical contact manager.
 *
 * pix.c: This file is part of GnomeCard.
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <gnome.h>

#include "pix.h"

pix *null_pix, *crd_pix, *ident_pix, *geo_pix, *org_pix, *exp_pix, *sec_pix;
pix *phone_pix, *email_pix, *addr_pix, *expl_pix, *org_pix;

extern pix *pix_new(char **xpm)
{
	GdkImlibImage *image;
	pix *new_pix;
	
	new_pix = g_malloc(sizeof(pix));
	
	image = gdk_imlib_create_image_from_xpm_data(xpm);
	gdk_imlib_render (image, 16, 16);
	new_pix->pixmap = gdk_imlib_move_image (image);
	new_pix->mask = gdk_imlib_move_mask (image);
	gdk_imlib_destroy_image (image);
	gdk_window_get_size(new_pix->pixmap,
			    &(new_pix->width), &(new_pix->height));
	
	return new_pix;
}
