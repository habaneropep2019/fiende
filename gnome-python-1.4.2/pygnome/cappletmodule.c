/* gnome-python - python bindings for gnome-libs
 * Copyright (C) 1998-2002 James Henstridge <james@daa.com.au>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <Python.h>
#include <capplet-widget.h>

#include "../pygtk/pygtk.h"

static PyObject *_wrap_gnome_capplet_init(PyObject *self, PyObject *args) {
  char *app_id, *app_version, **argv;
  int argc, i, res;
  PyObject *av;

  if (!PyArg_ParseTuple(args, "ss:gnome_capplet_init", &app_id, &app_version))
     return NULL;
  av = PySys_GetObject("argv");
  argc = PyList_Size(av);
  argv = g_new(char *, argc);
  for (i = 0; i < argc; i++)
    argv[i] = PyString_AsString(PyList_GetItem(av, i));

  res = gnome_capplet_init(app_id, app_version, argc, argv, NULL, 0, NULL);
  g_free(argv);

  return PyInt_FromLong(res);
}

static PyObject *_wrap_capplet_gtk_main(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, ":capplet_gtk_main"))
    return NULL;
  PyGtk_UnblockThreads();
  capplet_gtk_main();
  PyGtk_BlockThreads();

  if (PyErr_Occurred())
    return NULL;
  Py_INCREF(Py_None);
  return Py_None;
}

#include "cappletmodule_impl.c"

static PyMethodDef cappletMethods[] = {
    { "gnome_capplet_init", _wrap_gnome_capplet_init, 1 },
    { "capplet_gtk_main", _wrap_capplet_gtk_main, 1 },
#include "cappletmodule_defs.c"
    { NULL, NULL, 0 }
};

void init_capplet(void) {
    PyObject *m, *d;

    m = Py_InitModule("_capplet", cappletMethods);

    init_pygtk();

    if (PyErr_Occurred())
        Py_FatalError("can't initialise module _applet");
}
