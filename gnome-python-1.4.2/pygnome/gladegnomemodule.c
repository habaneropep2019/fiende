/* -*- Mode: C; c-basic-offset: 4 -*- */
/* PyGTK libglade module - python bindings for libglade
 * Copyright (C) 1999-2002 James Henstridge <james@daa.com.au>
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

#include <Python.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#include "../pygtk/pygtk.h"

static PyMethodDef gladegnomeMethods[] = {
    { NULL, NULL, 0 }
};


void init_gladegnome(void) {
    PyObject *m, *d;

    m = Py_InitModule("_gladegnome", gladegnomeMethods);

    init_pygtk();

    glade_gnome_init();

    if (PyErr_Occurred())
        Py_FatalError("can't initialise module _gladegnome");
}
