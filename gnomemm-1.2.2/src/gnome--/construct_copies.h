/* 
 * Copyright 2001 The Gnome-- development team.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef GNOMEMM_CONSTRUCT_COPIES_H
#define GNOMEMM_CONSTRUCT_COPIES_H

#include <gnome.h>
#include <libgnomeui/gnome-druid-page-finish.h>
#include <libgnomeui/gnome-druid-page-standard.h>
#include <libgnomeui/gnome-druid-page-start.h>

//These functions are not declared publicly in GNOME.
//Therefore they are copy-and-pasted here.
//There are also copies of a few private functions that are used by the _construct functions.
//This should not be necessary with GNOME 2, but it doesn't look like GNOME 1.2 is going to be fixed.

namespace Gnome {
 
void
gnome_druid_page_finish_construct (GnomeDruidPageFinish *druid_page_finish);

void
gnome_druid_page_standard_construct (GnomeDruidPageStandard *druid_page_standard);

void
gnome_druid_page_start_construct (GnomeDruidPageStart *druid_page_start);

void
gnome_message_box_constructv (GnomeMessageBox* message_box, 
            const gchar           *message,
		        const gchar           *message_box_type,
		      	const gchar 	     **buttons);

void
gtk_clock_construct(GtkClock* clock, GtkClockType type);


} /* namespace Gnome */

#endif /* GNOMEMM_CONSTRUCT_COPIES_H */
