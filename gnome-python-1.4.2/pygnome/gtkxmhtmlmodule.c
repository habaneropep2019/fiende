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
#include <gtk-xmhtml/gtk-xmhtml.h>

#include "../pygtk/pygtk.h"
static PyObject *from_string(char *txt) {
  if (txt)
    return PyString_FromString(txt);
  else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}
static PyObject *_wrap_cobject_to_info(PyObject *self, PyObject *args) {
  PyObject *dict, *cobject, *list, *v;
  int i;
  gtk_xmhtml_callback_info *info;
  XmHTMLAnchorCallbackStruct *cb1;
  XmHTMLEventCallbackStruct *cb2;
  XmHTMLDocumentCallbackStruct *cb3;
  XmHTMLFormCallbackStruct *cb4;
  XmHTMLFrameCallbackStruct *cb5;
  XmHTMLImagemapCallbackStruct *cb6;
  XmHTMLLinkCallbackStruct *cb7;

  if (!PyArg_ParseTuple(args, "O!:cobject_to_info", &PyCObject_Type, &cobject))
    return NULL;
  info = PyCObject_AsVoidPtr(cobject);
  dict = PyDict_New();
  PyDict_SetItemString(dict, "reason", v=PyInt_FromLong(info->reason));
  Py_DECREF(v);
  switch (info->reason) {
  case 0:
  case XmCR_HTML_ANCHORTRACK:
    cb1 = (XmHTMLAnchorCallbackStruct *)info;
    PyDict_SetItemString(dict, "url_type", v=PyInt_FromLong(cb1->url_type));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "line", v=PyInt_FromLong(cb1->line));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "href", v=from_string(cb1->href));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "target", v=from_string(cb1->target));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "rel", v=from_string(cb1->rel));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "rev", v=from_string(cb1->rev));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "title", v=from_string(cb1->title));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "is_frame", v=PyInt_FromLong(cb1->is_frame));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "doit", v=PyInt_FromLong(cb1->doit));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "visited", v=PyInt_FromLong(cb1->visited));
    Py_DECREF(v);
    break;
  case XmCR_HTML_EVENT:
  case XmCR_HTML_EVENTDESTROY:
    cb2 = (XmHTMLEventCallbackStruct *)info;
    PyDict_SetItemString(dict, "type", v=PyInt_FromLong(cb2->type));
    Py_DECREF(v);
    break;
  case XmCR_HTML_DOCUMENT:
    cb3 = (XmHTMLDocumentCallbackStruct *)info;
    PyDict_SetItemString(dict, "html32", v=PyInt_FromLong(cb3->html32));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "verified", v=PyInt_FromLong(cb3->verified));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "balanced", v=PyInt_FromLong(cb3->balanced));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "terminated",v=PyInt_FromLong(cb3->terminated));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "pass_level",v=PyInt_FromLong(cb3->pass_level));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "redo", v=PyInt_FromLong(cb3->redo));
    Py_DECREF(v);
    break;
  case XmCR_HTML_FORM:
    cb4 = (XmHTMLFormCallbackStruct *)info;
    PyDict_SetItemString(dict, "action", v=from_string(cb4->action));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "enctype", v=from_string(cb4->enctype));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "method", v=PyInt_FromLong(cb4->method));
    Py_DECREF(v);
    /* XmHTMLFormDataRec XXXX */
    break;
  case XmCR_HTML_FRAMECREATE:
  case XmCR_HTML_FRAMEDESTROY:
  case XmCR_HTML_FRAME:
    cb5 = (XmHTMLFrameCallbackStruct *)info;
    PyDict_SetItemString(dict, "src", v=from_string(cb5->src));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "name", v=from_string(cb5->name));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "html", v=PyGtk_New((GtkObject *)cb5->html));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "doit", v=PyInt_FromLong(cb5->doit));
    Py_DECREF(v);
    break;
  case XmCR_HTML_IMAGEMAPACTIVATE:
  case XmCR_HTML_IMAGEMAP:
    cb6 = (XmHTMLImagemapCallbackStruct *)info;
    PyDict_SetItemString(dict, "x", v=PyInt_FromLong(cb6->x));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "y", v=PyInt_FromLong(cb6->y));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "image_name",
			 v=from_string(cb6->image_name));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "map_name", v=from_string(cb6->map_name));
    Py_DECREF(v);
    PyDict_SetItemString(dict, "map_contents",
			 v=from_string(cb6->map_contents));
    Py_DECREF(v);
    /* XXXX XmImageInfo *image */
    break;
  case XmCR_HTML_LINK:
    cb7 = (XmHTMLLinkCallbackStruct *)info;
    list = PyList_New(cb7->num_link);
    for (i = 0; i < cb7->num_link; i++)
      PyList_SetItem(list, i, Py_BuildValue("(ssss)", cb7->link[i].url,
					    cb7->link[i].rel,
					    cb7->link[i].rev,
					    cb7->link[i].title));
    PyDict_SetItemString(dict, "link", list);
    Py_DECREF(list);
    break;
  }
  return dict;
}

#include "gtkxmhtmlmodule_impl.c"

static PyMethodDef gtkxmhtmlMethods[] = {
    { "cobject_to_info", _wrap_cobject_to_info, 1 },
#include "gtkxmhtmlmodule_defs.c"
    { NULL, NULL, 0 }
};

void init_gtkxmhtml() {
    PyObject *m, *d;

    m = Py_InitModule("_gtkxmhtml", gtkxmhtmlMethods);

    init_pygtk();

    if (PyErr_Occurred())
        Py_FatalError("can't initialise module _zvt");
}
