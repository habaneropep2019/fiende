/* bonobo-ui-icon-modify.h: Icon modifier for the Bonobo UI engine
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Author: Federico Mena-Quintero <federico@ximian.com>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _BONOBO_UI_ICON_MODIFY_H_
#define _BONOBO_UI_ICON_MODIFY_H_

#include <gdk-pixbuf/gdk-pixbuf.h>


GdkPixbuf *bonobo_ui_icon_modify (GdkPixbuf *source, double saturation,
				  gboolean pixelate, double pixelation_dark_factor);


#endif
