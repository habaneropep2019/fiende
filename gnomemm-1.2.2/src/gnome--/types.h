/* 
 * Copyright 2000 Karl Nelson
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

#ifndef GNOMEMM_TYPES_H
#define GNOMEMM_TYPES_H

#include <libgnomeui/gnome-types.h>
#include <sigc++/signal_system.h>
#include <gtk--/base.h>

namespace Gnome {

typedef SigC::Slot0<void>        Callback;
typedef SigC::Slot0<gdouble>     ProgressCallback;
typedef SigC::Slot1<void, gint>   ReplyCallback;
typedef SigC::Slot1<void, Gtk::string> StringCallback;

using Gtk::nstring;

} /* namespace Gnome */

#endif /* GNOMEMM_TYPES_H */

