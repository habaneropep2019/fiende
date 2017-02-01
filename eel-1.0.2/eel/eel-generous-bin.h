/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-generous-bin.h: Subclass of GtkBin that gives all of its
                       allocation to its child.

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

   Author: Darin Adler <darin@eazel.com>
 */

#ifndef EEL_GENEROUS_BIN_H
#define EEL_GENEROUS_BIN_H

#include <gtk/gtkbin.h>

#define EEL_TYPE_GENEROUS_BIN            (eel_generous_bin_get_type ())
#define EEL_GENEROUS_BIN(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_GENEROUS_BIN, EelGenerousBin))
#define EEL_GENEROUS_BIN_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_GENEROUS_BIN, EelGenerousBinClass))
#define EEL_IS_GENEROUS_BIN(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_GENEROUS_BIN))
#define EEL_IS_GENEROUS_BIN_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_GENEROUS_BIN))

typedef struct EelGenerousBin EelGenerousBin;
typedef struct EelGenerousBinClass EelGenerousBinClass;

struct EelGenerousBin {
	GtkBin parent_slot;
};

struct EelGenerousBinClass {
	GtkBinClass parent_slot;
};

GtkType eel_generous_bin_get_type (void);

#endif /* EEL_GENEROUS_BIN_H */
