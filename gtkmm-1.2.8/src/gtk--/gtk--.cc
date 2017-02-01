/* GTK-- - a C++ wrapper for the Gtk toolkit
 *
 * Copyright (C) 1997 Elliot Lee <sopwith@redhat.com>
 *                    Tero Pulkkinen <terop@modeemi.cs.tut.fi>
 * Currently maintained by Tero Pulkkinen
 *                                                        
 * New Callback stuff written by Phil Dawes (1997)        
 *  using ideas from code by Tero Pulkkinen.
 *
 * Phil's callback stuffs were replaced by something more typesafe. (TP)
 *
 * Added some   adjustment?GTK_ADJUSTMENT(ajdustment->gtkobject):NULL
 * thingyss -- 20.8.1997 Tero Pulkkinen to fix some segv's if you
 * gives NULL's to those.
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
 */

#include "gtk--config.h"
typedef unsigned int guint;

extern const guint gtkmm_major_version = GTKMM_MAJOR_VERSION;
extern const guint gtkmm_minor_version = GTKMM_MINOR_VERSION;
extern const guint gtkmm_micro_version = GTKMM_MICRO_VERSION;



//---------------------------------------------------------------------------


