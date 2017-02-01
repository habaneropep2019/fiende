/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-region.c: A simple wrapper on GdkRegion with rectangle operations.

   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>

#include "eel-region.h"

#include "eel-glib-extensions.h"
#include "eel-art-gtk-extensions.h"

struct EelRegion
{
	GdkRegion *gdk_region;
};

EelRegion *
eel_region_new (void)
{
	EelRegion *region;
	
	region = g_new0 (EelRegion, 1);

	region->gdk_region = gdk_region_new ();

	return region;
}

void
eel_region_free (EelRegion *region)
{
	if (region == NULL) {
		return;
	}

	g_assert (region->gdk_region != NULL);
	gdk_region_destroy (region->gdk_region);
	
	g_free (region);
}

static GdkRegion *
gdk_region_new_from_irect (ArtIRect rectangle)
{
	GdkRegion *region;
 	GdkRegion *empty_region;
	GdkRectangle gdk_rectangle;

	g_return_val_if_fail (!art_irect_empty (&rectangle), NULL);
	
	gdk_rectangle = eel_art_irect_to_gdk_rectangle (rectangle);
	empty_region = gdk_region_new ();
	region = gdk_region_union_with_rect (empty_region, &gdk_rectangle);
	gdk_region_destroy (empty_region);

	return region;
}

void
eel_region_add_rectangle (EelRegion *region,
			  ArtIRect rectangle)
{
	GdkRegion *add_region;
	GdkRegion *new_region;

	g_return_if_fail (region != NULL);
	g_return_if_fail (!art_irect_empty (&rectangle));

	add_region = gdk_region_new_from_irect (rectangle);
	new_region = gdk_regions_union (region->gdk_region, add_region);
	gdk_region_destroy (add_region);
	gdk_region_destroy (region->gdk_region);
	region->gdk_region = new_region;
}

void
eel_region_subtract_rectangle (EelRegion *region,
			       ArtIRect rectangle)
{
	GdkRegion *subtract_region;
	GdkRegion *new_region;

	g_return_if_fail (region != NULL);
	g_return_if_fail (!art_irect_empty (&rectangle));

	subtract_region = gdk_region_new_from_irect (rectangle);
	new_region = gdk_regions_subtract (region->gdk_region, subtract_region);
	gdk_region_destroy (subtract_region);
	gdk_region_destroy (region->gdk_region);
	region->gdk_region = new_region;
}

void
eel_region_set_gc_clip_region (const EelRegion *region,
			       GdkGC *gc)
{
	g_return_if_fail (region != NULL);
	g_return_if_fail (gc != NULL);

	gdk_gc_set_clip_region (gc, region->gdk_region);
}
