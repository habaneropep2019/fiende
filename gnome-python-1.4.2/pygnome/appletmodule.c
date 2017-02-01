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
#include <applet-widget.h>

#include "../pygtk/pygtk.h"

static PyObject *_wrap_applet_widget_init(PyObject *self, PyObject *args) {
  char *app_id, *app_version, **argv;
  int argc, i, res;
  PyObject *av;

  if (!PyArg_ParseTuple(args, "ss:applet_widget_init", &app_id, &app_version))
     return NULL;
  av = PySys_GetObject("argv");
  argc = PyList_Size(av);
  argv = g_new(char *, argc);
  for (i = 0; i < argc; i++)
    argv[i] = PyString_AsString(PyList_GetItem(av, i));

  res = applet_widget_init(app_id, app_version, argc, argv, NULL, 0, NULL);
  g_free(argv);

  return PyInt_FromLong(res);
}

static PyObject *_wrap_applet_widget_gtk_main(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, ":applet_widget_gtk_main"))
    return NULL;
  PyGtk_UnblockThreads();
  applet_widget_gtk_main();
  PyGtk_BlockThreads();

  if (PyErr_Occurred())
    return NULL;
  Py_INCREF(Py_None);
  return Py_None;
}

static void PyGnome_applet_callback(AppletWidget *applet, gpointer data) {
  PyObject *func, *user_data, *args, *ret;
  PyGtk_BlockThreads();
  func = PyTuple_GetItem((PyObject *)data, 0);
  user_data = PyTuple_GetItem((PyObject *)data, 1);

  ret = PyTuple_New(1);
  PyTuple_SetItem(ret, 0, PyGtk_New((GtkObject *)applet));
  args = PySequence_Concat(ret, user_data);
  Py_DECREF(ret);

  ret = PyObject_CallObject(func, args);
  Py_DECREF(args);
  if (ret == NULL) {
    if (PyGtk_FatalExceptions)
      applet_widget_gtk_main_quit();
    else {
      PyErr_Print();
      PyErr_Clear();
    }
  }
  PyGtk_UnblockThreads();
}

static PyObject *_wrap_applet_widget_register_callback(PyObject *self, PyObject *args) {
  PyObject *applet, *func, *user_data;
  char *name, *menutext;

  if (!PyArg_ParseTuple(args, "O!ssOO!:applet_widget_register_callback",
			&PyGtk_Type, &applet, &name, &menutext,
			&func, &PyTuple_Type, &user_data))
    return NULL;
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "forth argument not a function");
    return NULL;
  }
  applet_widget_register_callback(APPLET_WIDGET(PyGtk_Get(applet)),
				  name, menutext, 
				  PyGnome_applet_callback,
				  Py_BuildValue("(OO)", func, user_data));
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_applet_widget_register_stock_callback(PyObject *self, PyObject *args) {
  PyObject *applet, *func, *user_data;
  char *name, *stock_type, *menutext;

  if (!PyArg_ParseTuple(args, "O!sssOO!:applet_widget_register_stock_callback",
			&PyGtk_Type, &applet, &name, &stock_type, &menutext,
			&func, &PyTuple_Type, &user_data))
    return NULL;
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "fifth argument not a function");
    return NULL;
  }
  applet_widget_register_stock_callback(APPLET_WIDGET(PyGtk_Get(applet)),
					name, stock_type, menutext, 
					PyGnome_applet_callback,
					Py_BuildValue("(OO)", func,user_data));
  Py_INCREF(Py_None);
  return Py_None;
}

#include "appletmodule_impl.c"

static PyMethodDef appletMethods[] = {
    { "applet_widget_init", _wrap_applet_widget_init, 1 },
    { "applet_widget_gtk_main", _wrap_applet_widget_gtk_main, 1 },
    { "applet_widget_register_callback", _wrap_applet_widget_register_callback, 1 },
    { "applet_widget_register_stock_callback", _wrap_applet_widget_register_stock_callback, 1 },
#include "appletmodule_defs.c"
    { NULL, NULL, 0 }
};

void init_applet(void) {
    PyObject *m, *d;

    m = Py_InitModule("_applet", appletMethods);

    init_pygtk();

    if (PyErr_Occurred())
        Py_FatalError("can't initialise module _applet");
}
