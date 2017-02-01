/* -*- Mode: C; c-basic-offset: 4 -*- */
/* pygtk - python bindings for GTK+
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
#include <gtkhtml/gtkhtml.h>
#include <gtkhtml/gtkhtml-embedded.h>

#include "../pygtk/pygtk.h"
#include "gtkhtml_impl.c"

static PyObject *
_wrap_gtk_html_begin (PyObject *self, PyObject *args)
{
    PyObject *html, *handle;
    GtkHTMLStream *stream;
    
    if (!PyArg_ParseTuple(args, "O!:gtk_html_begin",
			  &PyGtk_Type, &html))
	return NULL;
    stream = gtk_html_begin (GTK_HTML (PyGtk_Get(html)));
    
    handle = PyCObject_FromVoidPtr((void *) stream, NULL);

    if (!handle)
	return NULL;

    return (PyObject *) handle;
}

static PyObject *
_wrap_gtk_html_begin_content (PyObject *self, PyObject *args)
{
    gchar *content_type;
    PyObject *html, *handle;
    GtkHTMLStream *stream;
    
    if (!PyArg_ParseTuple(args, "O!s:gtk_html_begin_content",
			  &PyGtk_Type, &html, &content_type))
	return NULL;
    stream = gtk_html_begin_content (GTK_HTML (PyGtk_Get(html)), content_type);
    
    handle = PyCObject_FromVoidPtr((void *) stream, NULL);

    if (!handle)
	return NULL;

    return (PyObject *) handle;
}

static PyObject *
_wrap_gtk_html_write (PyObject *self, PyObject *args)
{
    char *buf;
    int length;
    PyObject *html, *handle;
    GtkHTMLStream *stream;
    
    if (!PyArg_ParseTuple(args, "O!O!s#:gtk_html_write",
			  &PyGtk_Type, &html,
			  &PyCObject_Type, &handle,
			  &buf, &length))
	return NULL;

    stream = (GtkHTMLStream *) PyCObject_AsVoidPtr (handle);
    gtk_html_write (GTK_HTML (PyGtk_Get(html)), stream, buf, length);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_html_end (PyObject *self, PyObject *args)
{
    char *buf;
    int status;
    PyObject *html, *handle;
    GtkHTMLStream *stream;
    
    if (!PyArg_ParseTuple(args, "O!O!i:gtk_html_end",
			  &PyGtk_Type, &html,
			  &PyCObject_Type, &handle,
			  &status))
	return NULL;

    stream = (GtkHTMLStream *) PyCObject_AsVoidPtr (handle);
    gtk_html_end (GTK_HTML (PyGtk_Get(html)), stream, status);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_html_load_from_string (PyObject *self, PyObject *args)
{
    char *buf;
    int length;
    PyObject *html;
    
    if (!PyArg_ParseTuple(args, "O!s#:gtk_html_load_from_string",
			  &PyGtk_Type, &html,
			  &buf, &length))
	return NULL;

    gtk_html_load_from_string (GTK_HTML (PyGtk_Get(html)), buf, length);
    
    Py_INCREF(Py_None);
    return Py_None;
}

/* need this so it won't freak out on a NULL return */
static PyObject *
_wrap_gtk_html_embedded_get_parameter(PyObject *self, PyObject *args)
{
    char *param;
    char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!s:gtk_html_embedded_get_parameter", &PyGtk_Type, &ge, &param))
        return NULL;
    value = gtk_html_embedded_get_parameter(GTK_HTML_EMBEDDED(PyGtk_Get(ge)), param);
    if (value == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyString_FromString(value);
}

static PyObject *
_wrap_gtk_html_embedded_get_object_classid(PyObject *self, PyObject *args)
{
    char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!:gtk_html_embedded_get_object_classid", &PyGtk_Type, &ge))
        return NULL;
    value = GTK_HTML_EMBEDDED(PyGtk_Get(ge))->classid;
    if (value == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyString_FromString(value);
}

static PyObject *
_wrap_gtk_html_embedded_get_object_name(PyObject *self, PyObject *args)
{
    char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!:gtk_html_embedded_get_object_name", &PyGtk_Type, &ge))
        return NULL;
    value = GTK_HTML_EMBEDDED(PyGtk_Get(ge))->name;
    if (value == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyString_FromString(value);
}

static PyObject *
_wrap_gtk_html_embedded_get_object_type(PyObject *self, PyObject *args)
{
    char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!:gtk_html_embedded_get_object_type", &PyGtk_Type, &ge))
        return NULL;
    value = GTK_HTML_EMBEDDED(PyGtk_Get(ge))->type;
    if (value == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyString_FromString(value);
}

static PyObject *
_wrap_gtk_html_embedded_get_object_data(PyObject *self, PyObject *args)
{
    char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!:gtk_html_embedded_get_object_data", &PyGtk_Type, &ge))
        return NULL;
    value = GTK_HTML_EMBEDDED(PyGtk_Get(ge))->data;
    if (value == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyString_FromString(value);
}

static PyObject *
_wrap_gtk_html_embedded_set_object_classid(PyObject *self, PyObject *args)
{
    const char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!z:gtk_html_embedded_set_object_classid", &PyGtk_Type, &ge, &value))
        return NULL;
    g_free (GTK_HTML_EMBEDDED(PyGtk_Get(ge))->classid);
    GTK_HTML_EMBEDDED(PyGtk_Get(ge))->classid = g_strdup (value);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_html_embedded_set_object_name(PyObject *self, PyObject *args)
{
    const char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!z:gtk_html_embedded_set_object_name", &PyGtk_Type, &ge, &value))
        return NULL;
    g_free (GTK_HTML_EMBEDDED(PyGtk_Get(ge))->name);
    GTK_HTML_EMBEDDED(PyGtk_Get(ge))->name = g_strdup (value);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_html_embedded_set_object_type(PyObject *self, PyObject *args)
{
    const char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!z:gtk_html_embedded_set_object_type", &PyGtk_Type, &ge, &value))
        return NULL;
    g_free (GTK_HTML_EMBEDDED(PyGtk_Get(ge))->type);
    GTK_HTML_EMBEDDED(PyGtk_Get(ge))->type = g_strdup (value);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_html_embedded_set_object_data(PyObject *self, PyObject *args)
{
    const char *value;
    PyObject *ge;

    if (!PyArg_ParseTuple(args, "O!z:gtk_html_embedded_set_object_data", &PyGtk_Type, &ge, &value))
        return NULL;
    g_free (GTK_HTML_EMBEDDED(PyGtk_Get(ge))->data);
    GTK_HTML_EMBEDDED(PyGtk_Get(ge))->data = g_strdup (value);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef gtkhtmlMethods[] = {
    { "gtk_html_begin", _wrap_gtk_html_begin, 1 },
    { "gtk_html_begin_content", _wrap_gtk_html_begin_content, 1 },
    { "gtk_html_write", _wrap_gtk_html_write, 1 },
    { "gtk_html_end", _wrap_gtk_html_end, 1 },
    { "gtk_html_load_from_string", _wrap_gtk_html_load_from_string, 1 },
    { "gtk_html_embedded_get_parameter", _wrap_gtk_html_embedded_get_parameter, 1 },
    { "gtk_html_embedded_get_object_classid", _wrap_gtk_html_embedded_get_object_classid, 1 },
    { "gtk_html_embedded_get_object_name", _wrap_gtk_html_embedded_get_object_name, 1 },
    { "gtk_html_embedded_get_object_type", _wrap_gtk_html_embedded_get_object_type, 1 },
    { "gtk_html_embedded_get_object_data", _wrap_gtk_html_embedded_get_object_data, 1 },
    { "gtk_html_embedded_set_object_classid", _wrap_gtk_html_embedded_set_object_classid, 1 },
    { "gtk_html_embedded_set_object_name", _wrap_gtk_html_embedded_set_object_name, 1 },
    { "gtk_html_embedded_set_object_type", _wrap_gtk_html_embedded_set_object_type, 1 },
    { "gtk_html_embedded_set_object_data", _wrap_gtk_html_embedded_set_object_data, 1 },
#include "gtkhtml_defs.c"
    { NULL, NULL, 0 }
};

void init_gtkhtml(void) {
    PyObject *m, *d;

    m = Py_InitModule("_gtkhtml", gtkhtmlMethods);
    
    init_pygtk();
    gdk_rgb_init();

    if (PyErr_Occurred())
	Py_FatalError("can't initialise module _gtkhtml");
}
