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
#include <sys/types.h>
#include <zvt/zvtterm.h>

#include "../pygtk/pygtk.h"

static PyObject *_wrap_zvt_term_feed(PyObject *self, PyObject *args) {
  PyObject *term;
  char *text;
  int len;

  if (!PyArg_ParseTuple(args, "O!s#:zvt_term_feed", &PyGtk_Type, &term,
			&text, &len))
    return NULL;
  zvt_term_feed(ZVT_TERM(PyGtk_Get(term)), text, len);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_zvt_term_set_color_scheme(PyObject *self, PyObject *args) {
  PyObject *term, *colours, *item;
  gushort red[18], grn[18], blu[18];
  int i;

  if (!PyArg_ParseTuple(args, "O!O!:zvt_term_set_color_scheme", &PyGtk_Type,
			&term, &PyList_Type, &colours))
    return NULL;
  if (PyList_Size(colours) < 18) {
    PyErr_SetString(PyExc_TypeError, "list must be 18 elements long");
    return NULL;
  }
  for (i = 0; i < 18; i++) {
    item = PyList_GetItem(colours, i);
    if (!PyArg_ParseTuple(PyList_GetItem(colours, i), "hhh",
			  &red[i], &grn[i], &blu[i])) {
      PyErr_Clear();
      PyErr_SetString(PyExc_TypeError,"list items must be like (int,int,int)");
      return NULL;
    }
  }
  zvt_term_set_color_scheme(ZVT_TERM(PyGtk_Get(term)), red, grn, blu);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_zvt_term_get_buffer(PyObject *self, PyObject *args) {
  PyObject *term, *py_ret;
  int len, type, sx, sy, ex, ey;
  char *ret;

  if (!PyArg_ParseTuple(args, "O!iiiii:zvt_term_get_buffer", &PyGtk_Type,&term,
			&type, &sx, &sy, &ex, &ey))
    return NULL;
  ret = zvt_term_get_buffer(ZVT_TERM(PyGtk_Get(term)), &len, type,sx,sy,ex,ey);
  if (ret) {
    py_ret = PyString_FromStringAndSize(ret, len);
    free(ret);
    return py_ret;
  } else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

#include "zvtmodule_impl.c"

static PyMethodDef zvtMethods[] = {
    { "zvt_term_feed", _wrap_zvt_term_feed, 1 },
    { "zvt_term_set_color_scheme", _wrap_zvt_term_set_color_scheme, 1 },
    { "zvt_term_get_buffer", _wrap_zvt_term_get_buffer, 1 },
#include "zvtmodule_defs.c"
    { NULL, NULL, 0 }
};

void init_zvt() {
    PyObject *m, *d;

    m = Py_InitModule("_zvt", zvtMethods);

    init_pygtk();

    if (PyErr_Occurred())
        Py_FatalError("can't initialise module _zvt");
}

