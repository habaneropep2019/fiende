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
#include <sysmodule.h>

#include <locale.h>

#include <libgnomeui/libgnomeui.h>

#include "../pygtk/pygtk.h"
#include "../pygtk/pygdkimlib.h"

/* This is needed because a NULL signal handler is taken to mean that the
 * python signal handler should be called using the user data as a function.
 * This function is used to prevent seg faults where we really don't want a
 * python signal handler invoked.
 */

static void PyGnome_UISignalConnect(GnomeUIInfo *uiinfo, char *signal_name,
				    GnomeUIBuilderData *uibdata) {
  PyObject *data;
  if (uiinfo->moreinfo == NULL)
    return;
  if (uiinfo->user_data == NULL) {
    data = uiinfo->moreinfo;
    Py_INCREF(data);
  } else {
    data = PyTuple_New(2);
    PyTuple_SetItem(data, 0, uiinfo->moreinfo);
    Py_INCREF((PyObject *)uiinfo->moreinfo);
    PyTuple_SetItem(data, 1, uiinfo->user_data);
    Py_INCREF((PyObject *)uiinfo->user_data);
  }
  /* default signal handler will free this new object */
  gtk_signal_connect(GTK_OBJECT(uiinfo->widget), signal_name, NULL, data);
}

static GnomeUIBuilderData PyGnome_UIBuilder = {
  PyGnome_UISignalConnect,
  NULL,               /* these are basically ignored when */
  FALSE,
  NULL,
  NULL
};

static GnomeUIInfo *list_to_ui_info(PyObject *list, gboolean inc_uibd) {
  GnomeUIInfo *ret, *inf, *sub;
  PyObject *item, *moreinfo, *ac_mods;
  int len, i;
  int type, pix_type;
  char buf[100];

  if (!PyList_Check(list)) {
    PyErr_SetString(PyExc_TypeError, "object not a list");
    return NULL;
  }
  len = PyList_Size(list);
  if (inc_uibd) {
    inf = g_new(GnomeUIInfo, len + 2);
    inf[0].type = GNOME_APP_UI_BUILDER_DATA;
    inf[0].label = inf[0].hint = NULL;
    inf[0].moreinfo = &PyGnome_UIBuilder;
    inf[0].user_data = NULL;
    ret = &inf[1];
  } else
    ret = inf = g_new(GnomeUIInfo, len + 1);
  ret[len].type = GNOME_APP_UI_ENDOFINFO; /* set terminator */
  
  for (i = 0; i < len; i++) {
    item = PyList_GetItem(list, i);
    if (!PyTuple_Check(item) || !PyArg_ParseTuple(item, "izzOOiziO",
						  &(ret[i].type),
						  &(ret[i].label),
						  &(ret[i].hint),
						  &moreinfo,
						  &(ret[i].user_data),
						  &(ret[i].pixmap_type),
						  &(ret[i].pixmap_info),
						  &(ret[i].accelerator_key),
						  &ac_mods) ||
	PyGtkFlag_get_value(GTK_TYPE_GDK_MODIFIER_TYPE, ac_mods,
			    (gint *)&(ret[i].ac_mods))) {
      g_free(inf);
      return NULL;
    }
    if ((PyObject *)ret[i].user_data == Py_None)
      ret[i].user_data = NULL;
    else
      Py_INCREF((PyObject *)ret[i].user_data);
    type = ret[i].type; pix_type = ret[i].pixmap_type;
    if (type < GNOME_APP_UI_ENDOFINFO ||
	type == GNOME_APP_UI_BUILDER_DATA ||
	type > GNOME_APP_UI_SUBTREE_STOCK ||
	pix_type < GNOME_APP_PIXMAP_NONE ||
	pix_type > GNOME_APP_PIXMAP_FILENAME ||
	pix_type == GNOME_APP_PIXMAP_DATA) {
      sprintf(buf, "bad info or pixmap type for label \"%s\"", ret[i].label);
      PyErr_SetString(PyExc_TypeError, buf);
      g_free(inf);
      return NULL;
    }
    switch(type) {
    case GNOME_APP_UI_ENDOFINFO:
    case GNOME_APP_UI_SEPARATOR:
      ret[i].moreinfo = NULL;
      break;
    case GNOME_APP_UI_ITEM:
    case GNOME_APP_UI_TOGGLEITEM:
      if (moreinfo == Py_None) {
        ret[i].moreinfo = NULL;
        break;
      }
      if (!PyCallable_Check(moreinfo)) {
	sprintf(buf, "additional info not callable (label \"%s\")",
		ret[i].label);
	PyErr_SetString(PyExc_TypeError, buf);
	g_free(inf);
	return NULL;
      }
      ret[i].moreinfo = moreinfo;
      Py_INCREF(moreinfo);
      break;
    case GNOME_APP_UI_RADIOITEMS:
      sub = list_to_ui_info(moreinfo, FALSE);
      if (sub == NULL) {
	g_free(inf);
	return NULL;
      }
      ret[i].moreinfo = sub;
      break;
    case GNOME_APP_UI_SUBTREE:
    case GNOME_APP_UI_SUBTREE_STOCK:
      sub = list_to_ui_info(moreinfo, FALSE);
      if (sub == NULL) {
	g_free(inf);
	return NULL;
      }
      ret[i].moreinfo = sub;
      break;
    case GNOME_APP_UI_HELP:
      if (moreinfo != Py_None && !PyString_Check(moreinfo)) {
	PyErr_SetString(PyExc_TypeError, "additional info should be a string for help");
	g_free(inf);
	return NULL;
      }
      if (moreinfo == Py_None)
	ret[i].moreinfo = NULL;
      else
	ret[i].moreinfo = PyString_AsString(moreinfo);
      break;
    case GNOME_APP_UI_ITEM_CONFIGURABLE:
      if (moreinfo == Py_None) {
        ret[i].moreinfo = NULL;
        break;
      }
      if (!PyCallable_Check(moreinfo)) {
	sprintf(buf, "additional info not callable (label \"%s\")",
		ret[i].label);
	PyErr_SetString(PyExc_TypeError, buf);
	g_free(inf);
	return NULL;
      }
      ret[i].moreinfo = moreinfo;
      Py_INCREF(moreinfo);
      break;
    }
  }
  return inf;
}

static void free_ui_info(GnomeUIInfo *inf) {
  int i = 0;
  
  while (inf[i].type != GNOME_APP_UI_ENDOFINFO) {
    if (inf[i].type == GNOME_APP_UI_RADIOITEMS ||
	inf[i].type == GNOME_APP_UI_SUBTREE)
      if (inf[i].moreinfo !=  NULL)
	free_ui_info(inf[i].moreinfo);
    if ((inf[i].type == GNOME_APP_UI_ITEM ||
	 inf[i].type == GNOME_APP_UI_TOGGLEITEM ||
	 inf[i].type == GNOME_APP_UI_ITEM_CONFIGURABLE) &&
	inf[i].moreinfo != NULL)
      Py_DECREF((PyObject *)inf[i].moreinfo);
    if (inf[i].user_data != NULL)
      Py_DECREF((PyObject *)inf[i].user_data);
    i++;
  }
  g_free(inf);
}
/* --- */

static PyObject *imlib_fromarg(gpointer boxed) {
  return PyGdkImlibImage_New(boxed);
}
static int imlib_toarg(gpointer *boxed, PyObject *obj) {
  if (!PyGdkImlibImage_Check(obj))
    return 1;
  *boxed = PyGdkImlibImage_Get(obj);
  return 0;
}

static PyObject *points_fromarg(gpointer boxed) {
  GnomeCanvasPoints *points = boxed;
  PyObject *ret;
  int i;

  ret = PyTuple_New(points->num_points*2);
  for (i = 0; i < points->num_points*2; i++)
    PyTuple_SetItem(ret, i, PyFloat_FromDouble(points->coords[i]));
  gnome_canvas_points_free(points);
  return ret;
}

static int points_toarg(gpointer *boxed, PyObject *obj) {
  static GnomeCanvasPoints *points = NULL;
  int len, i;

  /* obj must be a sequence of even length */
  if (!PySequence_Check(obj) || (len=PySequence_Length(obj)) & 1 == 1)
    return 1;

  /* XXXX - this hack means we don't need to worry about freeing the
   * GnomeCanvasPoints structure after the function call.  It will not
   * work if more than one GnomeCanvasPoints structure needs to be
   * passed in. */
  if (points) gnome_canvas_points_free(points);
  points = gnome_canvas_points_new(len/2);
  for (i = 0; i < len; i++) {
    PyObject *item = PySequence_GetItem(obj, i);
    Py_DECREF(item);
    item = PyNumber_Float(item);
    if (!item) return 1;
    points->coords[i] = PyFloat_AsDouble(item);
    Py_DECREF(item);
  }
  *boxed = points;
  return 0;
}

static PyObject *_register_types() {
  static gboolean called = FALSE;

  if (!called) {
    called = TRUE;
  
    PyGtk_RegisterBoxed(GTK_TYPE_GDK_IMLIB_IMAGE,
			imlib_fromarg, imlib_toarg);
    PyGtk_RegisterBoxed(GTK_TYPE_GNOME_CANVAS_POINTS,
			points_fromarg, points_toarg);
  }
  Py_INCREF(Py_None);
  return Py_None;
}

/* we won't worry about all that popt stuff right now ... */
static PyObject *_wrap_gnome_init(PyObject *self, PyObject *args) {
  char *app_id, *app_version, **argv;
  int argc, i;
  PyObject *av;

  if (!PyArg_ParseTuple(args, "ss:gnome_init", &app_id, &app_version))
     return NULL;
  av = PySys_GetObject("argv");
  argc = PyList_Size(av);
  argv = g_new(char *, argc);
  for (i = 0; i < argc; i++)
    argv[i] = PyString_AsString(PyList_GetItem(av, i));

  gnome_init(app_id, app_version, argc, argv);
  g_free(argv);

  /* set LC_NUMERIC back to C, as required by Python */
  setlocale(LC_NUMERIC, "C");

  Py_DECREF(_register_types());

  /* some other stuff that needs to be inited */
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_about_new(PyObject *self, PyObject *args) {
  char *title, *version, *copyright, **authors, *comments, *logo;
  PyObject *list, *item, *ret;
  int len, i;

  if (!PyArg_ParseTuple(args, "zzzOzz", &title, &version, &copyright,
			&list, &comments, &logo))
    return NULL;
  if (!PySequence_Check(list)) {
    PyErr_SetString(PyExc_TypeError, "forth argument not a sequence");
    return NULL;
  }
  len = PySequence_Length(list);
  authors = g_new(char *, len+1);
  authors[len] = NULL;
  for (i = 0; i < len; i++) {
    item = PySequence_GetItem(list, i);
    Py_DECREF(item);
    if (!PyString_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "sequence member is not a string");
      g_free(authors);
      return NULL;
    }
    authors[i] = PyString_AsString(item);
  }
  ret= PyGtk_New((GtkObject *)gnome_about_new(title, version, copyright,
					      (const char **)authors,
					      comments, logo));
  g_free(authors);
  return ret;
}

static PyObject *_wrap_gnome_app_create_menus(PyObject *self, PyObject *args) {
  PyObject *app, *list;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O!O:gnome_app_create_menus", &PyGtk_Type, &app,
			&list))
    return NULL;
  if ((uiinfo = list_to_ui_info(list, FALSE)) == NULL)
    return NULL;
  gnome_app_create_menus_custom(GNOME_APP(PyGtk_Get(app)), uiinfo,
				&PyGnome_UIBuilder);
  free_ui_info(uiinfo);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_app_create_toolbar(PyObject *self, PyObject *args) {
  PyObject *app, *list;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O!O:gnome_app_create_toolbar", &PyGtk_Type, &app,
			&list))
    return NULL;
  if ((uiinfo = list_to_ui_info(list, FALSE)) == NULL)
    return NULL;
  gnome_app_create_toolbar_custom(GNOME_APP(PyGtk_Get(app)), uiinfo,
				  &PyGnome_UIBuilder);
  free_ui_info(uiinfo);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_app_insert_menus(PyObject *self, PyObject *args) {
  PyObject *app, *list;
  char *path;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O!sO:gnome_app_insert_menus", &PyGtk_Type, &app,
			&path, &list))
    return NULL;
  if ((uiinfo = list_to_ui_info(list, FALSE)) == NULL)
    return NULL;
  gnome_app_insert_menus_custom(GNOME_APP(PyGtk_Get(app)), path, uiinfo,
				&PyGnome_UIBuilder);
  free_ui_info(uiinfo);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_app_install_menu_hints(PyObject *self, PyObject *args) {
  PyObject *app, *list;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O!O:gnome_app_install_menu_hints", &PyGtk_Type,
			&app, &list))
    return NULL;
  if ((uiinfo = list_to_ui_info(list, FALSE)) == NULL)
    return NULL;
  gnome_app_install_menu_hints(GNOME_APP(PyGtk_Get(app)), uiinfo);
  free_ui_info(uiinfo);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_canvas_new(PyObject *self, PyObject *args) {
  GtkWidget *ret;

  if (!PyArg_ParseTuple(args, ":gnome_canvas_new"))
    return NULL;
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  gtk_widget_push_visual(gdk_imlib_get_visual());
  ret = gnome_canvas_new();
  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();
  return PyGtk_New((GtkObject *)ret);
}

static PyObject *_wrap_gnome_canvas_new_aa(PyObject *self, PyObject *args) {
  GtkWidget *ret;

  if (!PyArg_ParseTuple(args, ":gnome_canvas_new_aa"))
    return NULL;
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  gtk_widget_push_visual(gdk_rgb_get_visual());
  ret = gnome_canvas_new_aa();
  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();
  return PyGtk_New((GtkObject *)ret);
}

static PyObject *_wrap_gnome_canvas_get_scroll_region(PyObject *self,
						 PyObject *args) {
  PyObject *obj;
  double x1, y1, x2, y2;
  if (!PyArg_ParseTuple(args, "O!:gnome_canvas_get_scroll_region", &PyGtk_Type,
			&obj))
    return NULL;
  gnome_canvas_get_scroll_region(GNOME_CANVAS(PyGtk_Get(obj)), &x1, &y1,
				 &x2,&y2);
  return Py_BuildValue("(dddd)", x1, y1, x2, y2);
}

static PyObject *_wrap_gnome_canvas_get_scroll_offsets(PyObject *self,
						  PyObject *args) {
  PyObject *obj;
  int cx, cy;
  if (!PyArg_ParseTuple(args, "O!:gnome_canvas_get_scroll_offsets", &PyGtk_Type,
			&obj))
    return NULL;
  gnome_canvas_get_scroll_offsets(GNOME_CANVAS(PyGtk_Get(obj)), &cx, &cy);
  return Py_BuildValue("(ii)", cx, cy);
}

static PyObject *_wrap_gnome_canvas_w2c(PyObject *self, PyObject *args) {
  PyObject *obj;
  int cx, cy;
  double wx = 0.0, wy = 0.0;
  if (!PyArg_ParseTuple(args, "O!dd:gnome_canvas_w2c", &PyGtk_Type, &obj,
			&wx, &wy))
    return NULL;
  gnome_canvas_w2c(GNOME_CANVAS(PyGtk_Get(obj)), wx, wy, &cx, &cy);
  return Py_BuildValue("(ii)", cx, cy);
}

static PyObject *_wrap_gnome_canvas_c2w(PyObject *self, PyObject *args) {
  PyObject *obj;
  int cx = 0, cy = 0;
  double wx, wy;
  if (!PyArg_ParseTuple(args, "O!ii:gnome_canvas_c2w", &PyGtk_Type, &obj,
			&cx, &cy))
    return NULL;
  gnome_canvas_c2w(GNOME_CANVAS(PyGtk_Get(obj)), cx, cy, &wx, &wy);
  return Py_BuildValue("(dd)", wx, wy);
}

static PyObject *_wrap_gnome_canvas_get_color(PyObject *self, PyObject *args) {
  PyObject *obj;
  char *spec;
  GdkColor color;
  if (!PyArg_ParseTuple(args, "O!s:gnome_canvas_get_color", &PyGtk_Type, &obj,
			&spec));
  if (gnome_canvas_get_color(GNOME_CANVAS(PyGtk_Get(obj)), spec, &color))
    return PyGdkColor_New(&color);
  PyErr_SetString(PyExc_TypeError, "invalid colour specification");
  return NULL;
} 

static PyObject *_wrap_gnome_canvas_get_item_at(PyObject *self, PyObject *args) {
    double x, y;
    PyObject *canvas;
    GnomeCanvasItem *item;

    if (!PyArg_ParseTuple(args, "O!dd:gnome_canvas_get_item_at", &PyGtk_Type,
			  &canvas, &x, &y))
        return NULL;
    item = gnome_canvas_get_item_at(GNOME_CANVAS(PyGtk_Get(canvas)), x, y);
    if (item)
        return PyGtk_New((GtkObject *)item);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *_wrap_gnome_canvas_item_new(PyObject *self, PyObject *args) {
  PyObject *parent, *dict, *ret;
  GtkType type;
  gint nargs;
  GtkArg *arg;
  if (!PyArg_ParseTuple(args, "O!iO!:gnome_canvas_item_new", &PyGtk_Type,
			&parent, &type, &PyDict_Type, &dict))
    return NULL;
  arg = PyDict_AsGtkArgs(dict, type, &nargs);
  if (arg == NULL && nargs != 0)
    return NULL;
  ret = PyGtk_New((GtkObject *)gnome_canvas_item_newv(
				GNOME_CANVAS_GROUP(PyGtk_Get(parent)),
				type, nargs, arg));
  g_free(arg);
  return ret;
}
static PyObject *_wrap_gnome_canvas_item_set(PyObject *self, PyObject *args) {
  PyObject *obj, *dict;
  GtkType type;
  gint nargs;
  GtkArg *arg;
  if (!PyArg_ParseTuple(args, "O!O!:gnome_canvas_item_set", &PyGtk_Type,
			&obj, &PyDict_Type, &dict))
    return NULL;
  type = GTK_OBJECT_TYPE(PyGtk_Get(obj));
  arg = PyDict_AsGtkArgs(dict, type, &nargs);
  if (arg == NULL && nargs != 0)
    return NULL;
  gnome_canvas_item_setv(GNOME_CANVAS_ITEM(PyGtk_Get(obj)), nargs, arg);
  g_free(arg);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_canvas_item_affine_relative(PyObject *self, PyObject *args) {
  PyObject *item, *py_affine;
  double affine[6];
  int i;

  if (!PyArg_ParseTuple(args, "O!O:gnome_canvas_item_affine_relative",
			&PyGtk_Type, &item, &py_affine))
    return NULL;
  if (!PySequence_Check(py_affine) || PySequence_Length(py_affine) != 6) {
    PyErr_SetString(PyExc_TypeError, "second argument must be a sequence, length 6");
    return NULL;
  }
  for (i = 0; i < 6; i++) {
    PyObject *sitem = PySequence_GetItem(py_affine, i);
    Py_DECREF(sitem);
    sitem = PyNumber_Float(sitem);
    if (sitem)
      affine[i] = PyFloat_AsDouble(sitem);
    else {
      PyErr_Clear();
      PyErr_SetString(PyExc_TypeError, "sequence item not a float");
      return NULL;
    }
    Py_DECREF(sitem);
  }
  gnome_canvas_item_affine_relative(GNOME_CANVAS_ITEM(PyGtk_Get(item)),affine);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_canvas_item_affine_absolute(PyObject *self, PyObject *args) {
  PyObject *item, *py_affine;
  double affine[6];
  int i;

  if (!PyArg_ParseTuple(args, "O!O:gnome_canvas_item_affine_absolute",
			&PyGtk_Type, &item, &py_affine))
    return NULL;
  if (!PySequence_Check(py_affine) || PySequence_Length(py_affine) != 6) {
    PyErr_SetString(PyExc_TypeError, "second argument must be a sequence, length 6");
    return NULL;
  }
  for (i = 0; i < 6; i++) {
    PyObject *sitem = PySequence_GetItem(py_affine, i);
    Py_DECREF(sitem);
    sitem = PyNumber_Float(sitem);
    if (sitem)
      affine[i] = PyFloat_AsDouble(sitem);
    else {
      PyErr_Clear();
      PyErr_SetString(PyExc_TypeError, "sequence item not a float");
      return NULL;
    }
    Py_DECREF(sitem);
  }
  gnome_canvas_item_affine_absolute(GNOME_CANVAS_ITEM(PyGtk_Get(item)),affine);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_canvas_item_w2i(PyObject *self, PyObject *args) {
  PyObject *item;
  double x, y;

  if (!PyArg_ParseTuple(args, "O!dd:gnome_canvas_item_w2i", &PyGtk_Type,
			&item, &x, &y))
    return NULL;
  gnome_canvas_item_w2i(GNOME_CANVAS_ITEM(PyGtk_Get(item)), &x, &y);
  return Py_BuildValue("(dd)", x, y);
}

static PyObject *_wrap_gnome_canvas_item_i2w(PyObject *self, PyObject *args) {
  PyObject *item;
  double x, y;

  if (!PyArg_ParseTuple(args, "O!dd:gnome_canvas_item_i2w", &PyGtk_Type,
			&item, &x, &y))
    return NULL;
  gnome_canvas_item_i2w(GNOME_CANVAS_ITEM(PyGtk_Get(item)), &x, &y);
  return Py_BuildValue("(dd)", x, y);
}

static PyObject *_wrap_gnome_canvas_item_get_bounds(PyObject *self, PyObject *args) {
  PyObject *item;
  double x1, y1, x2, y2;

  if (!PyArg_ParseTuple(args, "O!:gnome_canvas_item_get_bounds", &PyGtk_Type,
			&item))
    return NULL;
  gnome_canvas_item_get_bounds(GNOME_CANVAS_ITEM(PyGtk_Get(item)),
			       &x1, &y1, &x2, &y2);
  return Py_BuildValue("(dddd)", x1, y1, x2, y2);
}

static PyObject *_wrap_gnome_canvas_group_children(PyObject *self, PyObject *args) {
  PyObject *group, *ret;
  GList *children;
  
  if (!PyArg_ParseTuple(args, "O!:gnome_canvas_group_children", &PyGtk_Type,
			&group))
    return NULL;
  children = GNOME_CANVAS_GROUP(PyGtk_Get(group))->item_list;
  ret = PyList_New(0);
  while (children) {
    PyList_Append(ret, PyGtk_New(children->data));
    children = children->next;
  }
  return ret;
}

static int read_vector(PyObject *obj, int *argc, char ***argv) {
  int i;
  if (obj == Py_None) {
    *argc = 0;
    *argv = NULL;
    return 0;
  } else if (!PySequence_Check(obj)) {
    PyErr_SetString(PyExc_TypeError, "argument not a sequence or None");
    return 1;
  }
  *argc = PySequence_Length(obj);
  *argv = g_new(char *, *argc);
  for (i = 0; i < *argc; i++) {
    PyObject *item = PySequence_GetItem(obj, i);
    Py_DECREF(item);
    if (!PyString_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "sequence item not a string");
      g_free(*argv);
      return 1;
    }
    (*argv)[i] = PyString_AsString(item);
  }
  return 0;
}

static PyObject *_wrap_gnome_client_set_clone_command(PyObject *self, PyObject *args) {
  PyObject *client, *av = Py_None;
  int argc;
  char **argv = NULL;
  if (!PyArg_ParseTuple(args, "O!|O:gnome_client_set_clone_command",
			&PyGtk_Type, &client, &av))
    return NULL;
  if (read_vector(av, &argc, &argv))
    return NULL;
  gnome_client_set_clone_command(GNOME_CLIENT(PyGtk_Get(client)), argc, argv);
  if (argv) g_free(argv);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_client_set_discard_command(PyObject *self, PyObject *args) {
  PyObject *client, *av = Py_None;
  int argc;
  char **argv = NULL;
  if (!PyArg_ParseTuple(args, "O!|O:gnome_client_set_discard_command",
			&PyGtk_Type, &client, &av))
    return NULL;
  if (read_vector(av, &argc, &argv))
    return NULL;
  gnome_client_set_discard_command(GNOME_CLIENT(PyGtk_Get(client)), argc,argv);
  if (argv) g_free(argv);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_client_set_restart_command(PyObject *self, PyObject *args) {
  PyObject *client, *av = Py_None;
  int argc;
  char **argv = NULL;
  if (!PyArg_ParseTuple(args, "O!|O:gnome_client_set_restart_command",
			&PyGtk_Type, &client, &av))
    return NULL;
  if (read_vector(av, &argc, &argv))
    return NULL;
  gnome_client_set_restart_command(GNOME_CLIENT(PyGtk_Get(client)), argc,argv);
  if (argv) g_free(argv);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_client_set_resign_command(PyObject *self, PyObject *args) {
  PyObject *client, *av = Py_None;
  int argc;
  char **argv = NULL;
  if (!PyArg_ParseTuple(args, "O!|O:gnome_client_set_resign_command",
			&PyGtk_Type, &client, &av))
    return NULL;
  if (read_vector(av, &argc, &argv))
    return NULL;
  gnome_client_set_resign_command(GNOME_CLIENT(PyGtk_Get(client)), argc, argv);
  if (argv) g_free(argv);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_client_set_shutdown_command(PyObject *self, PyObject *args) {
  PyObject *client, *av = Py_None;
  int argc;
  char **argv = NULL;
  if (!PyArg_ParseTuple(args, "O!|O:gnome_client_set_shutdown_command",
			&PyGtk_Type, &client, &av))
    return NULL;
  if (read_vector(av, &argc, &argv))
    return NULL;
  gnome_client_set_shutdown_command(GNOME_CLIENT(PyGtk_Get(client)),
				    argc, argv);
  if (argv) g_free(argv);
  Py_INCREF(Py_None);
  return Py_None;
}

static void PyGnome_interact_function(gpointer a, PyObject *func, int nargs,
				      GtkArg *args) {
  PyObject *ret;
  /* this is the correct implementation, but gnome_client is done wrong */
  /*ret = PyObject_CallFunction(func, "(Oii)",
			      PyGtk_New(GTK_VALUE_OBJECT(args[0])),
			      GTK_VALUE_INT(args[1]),
			      GTK_VALUE_ENUM(args[2])); */
  PyGtk_BlockThreads();
  ret = PyObject_CallFunction(func, "(Oii)",
			      PyGtk_New(*GTK_RETLOC_OBJECT(args[1])),
			      *GTK_RETLOC_INT(args[2]),
			      *GTK_RETLOC_ENUM(args[3]));
  if (ret == NULL) {
    if (PyGtk_FatalExceptions)
        gtk_main_quit();
    else {
        PyErr_Print();
        PyErr_Clear();
    }
    PyGtk_UnblockThreads();
    return;
  }
  Py_DECREF(ret);
  PyGtk_UnblockThreads();
}

static PyObject *_wrap_gnome_client_request_interaction(PyObject *self,
							PyObject *args) {
  PyObject *client, *func, *py_dialog;
  GnomeDialogType dialog;
  if (!PyArg_ParseTuple(args, "O!OO", &PyGtk_Type, &client, &py_dialog, &func))
    return NULL;
  if (PyGtkEnum_get_value(GTK_TYPE_GNOME_DIALOG_TYPE,py_dialog,(gint*)&dialog))
    return NULL;
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "third argument not callbable");
    return NULL;
  }
  Py_INCREF(func);
  gnome_client_request_interaction_interp(GNOME_CLIENT(PyGtk_Get(client)),
			dialog,
			(GtkCallbackMarshal) PyGnome_interact_function,
			func,
			(GtkDestroyNotify) PyGtk_DestroyNotify);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_color_picker_get_d(PyObject *self, PyObject *args) {
  PyObject *obj;
  double r, g, b, a;
  if (!PyArg_ParseTuple(args, "O!:gnome_color_picker_get_d", &PyGtk_Type, &obj))
    return NULL;
  gnome_color_picker_get_d(GNOME_COLOR_PICKER(PyGtk_Get(obj)), &r, &g, &b, &a);
  return Py_BuildValue("(dddd)", r, g, b, a);
}

static PyObject *_wrap_gnome_color_picker_get_i8(PyObject *self, PyObject *args) {
  PyObject *obj;
  guint8 r, g, b, a;
  if (!PyArg_ParseTuple(args, "O!:gnome_color_picker_get_i8", &PyGtk_Type,&obj))
    return NULL;
  gnome_color_picker_get_i8(GNOME_COLOR_PICKER(PyGtk_Get(obj)), &r, &g, &b,&a);
  return Py_BuildValue("(iiii)", (int)r, (int)g, (int)b, (int)a);
}

static PyObject *_wrap_gnome_color_picker_get_i16(PyObject *self, PyObject *args) {
  PyObject *obj;
  gushort r, g, b, a;
  if (!PyArg_ParseTuple(args, "O!:gnome_color_picker_get_i16",&PyGtk_Type,&obj))
    return NULL;
  gnome_color_picker_get_i16(GNOME_COLOR_PICKER(PyGtk_Get(obj)), &r, &g,&b,&a);
  return Py_BuildValue("(iiii)", (int)r, (int)g, (int)b, (int)a);
}

static PyObject *_wrap_gnome_dentry_edit_save_file(PyObject *self, PyObject *args) {
  PyObject *obj;
  char *fname;
  GnomeDesktopEntry *de;
  if (!PyArg_ParseTuple(args, "O!s:gnome_dentry_edit_save_file", &PyGtk_Type,
			&obj, &fname))
    return NULL;
  de = gnome_dentry_get_dentry(GNOME_DENTRY_EDIT(PyGtk_Get(obj)));
  de->location = g_strdup(fname);
  gnome_desktop_entry_save(de);
  gnome_desktop_entry_free(de);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_dialog_button_connect(PyObject *self, PyObject *args) {
  PyObject *obj, *callback, * extra, * data;
  int button;

  if (!PyArg_ParseTuple(args, "O!iO:gnome_dialog_button_connect", &PyGtk_Type,
			&obj, &button, &callback))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "third argument not callable");
    return NULL;
  }

  extra = PyTuple_New(0);

  if (extra == NULL)
    return NULL;
  data = Py_BuildValue("(ON)", callback, extra);

  gnome_dialog_button_connect(GNOME_DIALOG(PyGtk_Get(obj)), button, NULL,
			      data);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_file_entry_get_full_path(PyObject *self, 
						      PyObject *args) {
    PyObject *py_ret, *fentry;
    int file_must_exist;
    char *ret;

    if (!PyArg_ParseTuple(args, "O!i:gnome_file_entry_get_full_path", 
			  &PyGtk_Type, &fentry, &file_must_exist))
        return NULL;
    ret = gnome_file_entry_get_full_path(GNOME_FILE_ENTRY(PyGtk_Get(fentry)), 
					 file_must_exist);
    if (ret == NULL) {
      py_ret = Py_None;
      Py_INCREF (Py_None);
    }
    else {
      py_ret = PyString_FromString(ret);
      g_free(ret);
    }
    return py_ret;
}

static PyObject *_wrap_gnome_icon_list_set_icon_data(PyObject *self,
						     PyObject *args) {
  PyObject *ilist, *data;
  int pos;
  if (!PyArg_ParseTuple(args, "O!iO:gnome_icon_list_set_icon_data", &PyGtk_Type,
			&ilist, &pos, &data))
    return NULL;
  Py_INCREF(data);
  gnome_icon_list_set_icon_data_full(GNOME_ICON_LIST(PyGtk_Get(ilist)), pos,
				     data, (GtkDestroyNotify)
				     PyGtk_DestroyNotify);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_icon_list_find_icon_from_data(PyObject *self, PyObject *args) {
  PyObject *ilist, *data;

  if (!PyArg_ParseTuple(args, "O!O:gnome_icon_list_find_icon_from_data",
			&PyGtk_Type, &ilist, &data))
    return NULL;
  return PyInt_FromLong(gnome_icon_list_find_icon_from_data(
			GNOME_ICON_LIST(PyGtk_Get(ilist)), data));
}

static PyObject *_wrap_gnome_icon_list_get_icon_data(PyObject *self, PyObject *args) {
  PyObject *ilist, *data;
  int pos;

  if (!PyArg_ParseTuple(args, "O!i:gnome_icon_list_get_icon_data",
			&PyGtk_Type, &ilist, &pos))
    return NULL;
  data = gnome_icon_list_get_icon_data(GNOME_ICON_LIST(PyGtk_Get(ilist)), pos);
  Py_INCREF(data);
  return data;
}

static PyObject *_wrap_gnome_mdi_set_menubar_template(PyObject *self, PyObject *args) {
  PyObject *mdi, *list;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O!O:gnome_mdi_set_menubar_template", &PyGtk_Type,
			&mdi, &list))
    return NULL;
  if ((uiinfo = list_to_ui_info(list, TRUE)) == NULL)
    return NULL;
  gnome_mdi_set_menubar_template(GNOME_MDI(PyGtk_Get(mdi)), uiinfo);
  /* we don't free the uiinfo */
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_mdi_set_toolbar_template(PyObject *self, PyObject *args) {
  PyObject *mdi, *list;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O!O:gnome_mdi_set_toolbar_template", &PyGtk_Type,
			&mdi, &list))
    return NULL;
  if ((uiinfo = list_to_ui_info(list, TRUE)) == NULL)
    return NULL;
  gnome_mdi_set_toolbar_template(GNOME_MDI(PyGtk_Get(mdi)), uiinfo);
  /* we don't free the uiinfo */
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_mdi_child_set_menu_template(PyObject *self, PyObject *args) {
  PyObject *mdi, *list;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O!O:gnome_mdi_child_set_menu_template",
			&PyGtk_Type, &mdi, &list))
    return NULL;
  if ((uiinfo = list_to_ui_info(list, TRUE)) == NULL)
    return NULL;
  gnome_mdi_child_set_menu_template(GNOME_MDI_CHILD(PyGtk_Get(mdi)), uiinfo);
  /* we don't free the uiinfo */
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_mdi_generic_child_set_view_creator(PyObject *self, PyObject *args) {
  PyObject *mdi, *func, *data = NULL;

  if (!PyArg_ParseTuple(args, "O!O|O:gnome_mdi_generic_child_set_view_creator",
			&PyGtk_Type, &mdi, &func, &data))
    return NULL;
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(func);
  if (data) {
    PyObject *tmp = PyTuple_New(2);
    PyTuple_SetItem(tmp, 0, func);
    Py_INCREF(data);
    PyTuple_SetItem(tmp, 1, data);
    func = tmp;
  }
  gnome_mdi_generic_child_set_view_creator_full(GNOME_MDI_GENERIC_CHILD(PyGtk_Get(mdi)),
						NULL,
						PyGtk_CallbackMarshal,
						func,
						PyGtk_DestroyNotify);
  Py_INCREF(Py_None);
  return Py_None;
}
						
static PyObject *_wrap_gnome_mdi_generic_child_set_menu_creator(PyObject *self, PyObject *args) {
  PyObject *mdi, *func, *data = NULL;

  if (!PyArg_ParseTuple(args, "O!O|O:gnome_mdi_generic_child_set_menu_creator",
			&PyGtk_Type, &mdi, &func, &data))
    return NULL;
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(func);
  if (data) {
    PyObject *tmp = PyTuple_New(2);
    PyTuple_SetItem(tmp, 0, func);
    Py_INCREF(data);
    PyTuple_SetItem(tmp, 1, data);
    func = tmp;
  }
  gnome_mdi_generic_child_set_menu_creator_full(GNOME_MDI_GENERIC_CHILD(PyGtk_Get (mdi)),
						NULL,
						PyGtk_CallbackMarshal,
						func,
						PyGtk_DestroyNotify);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_mdi_generic_child_set_config_func(PyObject *self, PyObject *args) {
  PyObject *mdi, *func, *data = NULL;

  if (!PyArg_ParseTuple(args, "O!O|O:gnome_mdi_generic_child_set_config_func",
			&PyGtk_Type, &mdi, &func, &data))
    return NULL;
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(func);
  if (data) {
    PyObject *tmp = PyTuple_New(2);
    PyTuple_SetItem(tmp, 0, func);
    Py_INCREF(data);
    PyTuple_SetItem(tmp, 1, data);
    func = tmp;
  }
  gnome_mdi_generic_child_set_config_func_full(GNOME_MDI_GENERIC_CHILD(PyGtk_Get (mdi)),
					       NULL,
					       PyGtk_CallbackMarshal,
					       func,
					       PyGtk_DestroyNotify);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_mdi_generic_child_set_label_func(PyObject *self, PyObject *args) {
  PyObject *mdi, *func, *data = NULL;

  if (!PyArg_ParseTuple(args, "O!O|O:gnome_mdi_generic_child_set_label_func",
			&PyGtk_Type, &mdi, &func, &data))
    return NULL;
  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(func);
  if (data) {
    PyObject *tmp = PyTuple_New(2);
    PyTuple_SetItem(tmp, 0, func);
    Py_INCREF(data);
    PyTuple_SetItem(tmp, 1, data);
    func = tmp;
  }
  gnome_mdi_generic_child_set_label_func_full(GNOME_MDI_GENERIC_CHILD(PyGtk_Get (mdi)),
					      NULL,
					      PyGtk_CallbackMarshal,
					      func,
					      PyGtk_DestroyNotify);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_popup_menu_new(PyObject *self, PyObject *args) {
  PyObject *list, *ret;
  GnomeUIInfo *uiinfo;

  if (!PyArg_ParseTuple(args, "O:gnome_popup_menu_new", &list))
    return NULL;
  /* include a GNOME_APP_UI_BUILDER_DATA entry to override this code's
   * none interp-friendlyness */
  if ((uiinfo = list_to_ui_info(list, TRUE)) == NULL)
    return NULL;
  ret = PyGtk_New((GtkObject *)gnome_popup_menu_new(uiinfo));
  free_ui_info(uiinfo);
  return ret;
}

static PyObject *_wrap_gnome_popup_menu_attach(PyObject *self,PyObject *args) {
  PyObject *popup, *widget;

  if (!PyArg_ParseTuple(args, "O!O!:gnome_popup_menu_attach", &PyGtk_Type,
			&popup, &PyGtk_Type, &widget))
    return NULL;
  gnome_popup_menu_attach(GTK_WIDGET(PyGtk_Get(popup)),
			  GTK_WIDGET(PyGtk_Get(widget)), NULL);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_widget_add_help_with_uidata(PyObject *self,
							 PyObject *args) {
  PyObject *widget, *list;
  char *help;
  GnomeUIInfo *menuinfo;

  if (!PyArg_ParseTuple(args, "O!sO:gnome_widget_add_help_with_uidata",
			&PyGtk_Type, &widget, &help, &list))
    return NULL;
  if ((menuinfo = list_to_ui_info(list, TRUE)) == NULL)
    return NULL;
  gnome_widget_add_help_with_uidata(GTK_WIDGET(PyGtk_Get(widget)), help,
				    menuinfo, NULL);
  free_ui_info(menuinfo);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_scores_new(PyObject *self, PyObject *args) {
  PyObject *list, *item;
  guint num, i, clear;
  gchar **names;
  gfloat *scores;
  time_t *times;
  GtkWidget *ret;
  if (!PyArg_ParseTuple(args, "O!i:gnome_scores_new", &PyList_Type, &list,
			&clear))
    return NULL;
  num = PyList_Size(list);
  names = g_new(gchar *, num);
  scores = g_new(gfloat, num);
  times = g_new(time_t, num);
  for (i = 0; i < num; i++) {
    item = PyList_GetItem(list, i);
    if (!PyArg_ParseTuple(item, "sfi", &names[i], &scores[i], &times[i])) {
      PyErr_Clear();
      g_free(names);
      g_free(scores);
      g_free(times);
      PyErr_SetString(PyExc_TypeError,"list items should be (name,score,time");
      return NULL;
    }
  }
  ret = gnome_scores_new(num, names, scores, times, clear);
  g_free(names);
  g_free(scores);
  g_free(times);
  return PyGtk_New((GtkObject *)ret);
}

static PyObject *_wrap_gnome_stock_menu_accel(PyObject *self, PyObject *args) {
  char *type;
  guchar key;
  guint8 mod;
  gboolean ret;

  if (!PyArg_ParseTuple(args, "s:gnome_stock_menu_accel", &type))
    return NULL;
  ret = gnome_stock_menu_accel(type, &key, &mod);
  if (!ret) {
    PyErr_SetString(PyExc_KeyError, type);
    return NULL;
  }
  return Py_BuildValue("(ci)", key, mod);
}

/* --- gtkcauldron.h --- */
typedef struct {
  int type;
  union {
    char *ch_pp;
    int int_p;
    double dbl_p;
    PyObject *tuple; /* of form (callback, userdata). Type set to CALLBACK */
  } d;
} cauldron_result;
typedef struct {
  PyObject *args;
  int arg_len, pos, num_results, num_rets;
  cauldron_result *results;
} cauldron_data;

static GtkWidget *PyGtk_cauldron_callback(GtkWidget *self, PyObject *tuple) {
  PyObject *ret, *params;
  GtkWidget *wret;

  PyGtk_BlockThreads();
  ret = PyTuple_New(1);
  PyTuple_SetItem(ret, 0, PyGtk_New((GtkObject *)self));
  params = PySequence_Concat(ret, PyTuple_GetItem(tuple, 1));
  Py_DECREF(ret);

  ret = PyObject_CallObject(PyTuple_GetItem(tuple, 0), params);
  Py_DECREF(params);
  if (ret == NULL) {
    if (PyGtk_FatalExceptions)
        gtk_main_quit();
    else {
        PyErr_Print();
        PyErr_Clear();
    }
    PyGtk_UnblockThreads();
    return NULL;
  }

  if (ret == Py_None) { /* this should only be returned by 'c' type callbacks*/
    Py_DECREF(ret);
    PyGtk_UnblockThreads();
    return NULL;
  } else if (PyGtk_Check(ret)) {
    wret = GTK_WIDGET(PyGtk_Get(ret));
    /* the returned widget should still have its floating reference set */
    Py_DECREF(ret);
    PyGtk_UnblockThreads();
    return wret;
  } else if ((params = PyObject_GetAttrString(ret, "_o")) &&
	     PyGtk_Check(params)) { /* for Gtkinter */
    wret = GTK_WIDGET(PyGtk_Get(params));
    Py_DECREF(params);
    Py_DECREF(ret);
    PyGtk_UnblockThreads();
    return wret;
  }
  Py_XDECREF(params);
  Py_DECREF(ret);
  PyGtk_UnblockThreads();
  return NULL;
}

static void next_arg(gint type, cauldron_data *data, void *result) {
  PyObject *item;

  /* there doesn't seem to be any error reporting mechanism -- just fudge it */
  if (data->pos >= data->arg_len) {
    *((int *)result) = 0;
    return;
  }
  item = PyTuple_GetItem(data->args, data->pos++);
  switch (type) {
  case GTK_CAULDRON_TYPE_CHAR_P:
    if (PyString_Check(item))
      *((gchar **)result) = PyString_AsString(item);
    else {
      g_warning("expected string argument, got %s", item->ob_type->tp_name);
      *((gchar **)result) = "";
    }
    break;
  case GTK_CAULDRON_TYPE_CHAR_P_P:
    data->results[data->num_results].type = type;
    if (PyString_Check(item))
      data->results[data->num_results].d.ch_pp = PyString_AsString(item);
    else {
      g_warning("expected string argument, got %s", item->ob_type->tp_name);
      data->results[data->num_results].d.ch_pp = "";
    }
    data->num_rets++;
    *((gchar ***)result) = &(data->results[data->num_results++].d.ch_pp);
    break;
  case GTK_CAULDRON_TYPE_INT:
    if (PyInt_Check(item))
      *((gint *)result) = PyInt_AsLong(item);
    else {
      g_warning("expected int argument, got %s", item->ob_type->tp_name);
      *((gint *)result) = 0;
    }
    break;
  case GTK_CAULDRON_TYPE_INT_P:
    data->results[data->num_results].type = type;
    if (PyInt_Check(item))
      data->results[data->num_results].d.int_p = PyInt_AsLong(item);
    else {
      g_warning("expected int argument, got %s", item->ob_type->tp_name);
      data->results[data->num_results].d.int_p = 0;
    }
    data->num_rets++;
    *((gint **)result) = &(data->results[data->num_results++].d.int_p);
    break;
  case GTK_CAULDRON_TYPE_DOUBLE:
    if (PyFloat_Check(item))
      *((gdouble *)result) = PyFloat_AsDouble(item);
    else {
      g_warning("expected float argument, got %s", item->ob_type->tp_name);
      *((gdouble *)result) = 0.0;
    }
    break;
  case GTK_CAULDRON_TYPE_DOUBLE_P:
    data->results[data->num_results].type = type;
    if (PyFloat_Check(item))
      data->results[data->num_results].d.dbl_p = PyFloat_AsDouble(item);
    else {
      g_warning("expected float argument, got %s", item->ob_type->tp_name);
      data->results[data->num_results].d.dbl_p = 0.0;
    }
    data->num_rets++;
    *((gdouble **)result) = &(data->results[data->num_results++].d.dbl_p);
    break;
  case GTK_CAULDRON_TYPE_CALLBACK:
    data->results[data->num_results].type = type;
    data->results[data->num_results].d.tuple = PyTuple_New(2);
    Py_INCREF(item);
    PyTuple_SetItem(data->results[data->num_results++].d.tuple, 0, item);
    *((gpointer*)result) = PyGtk_cauldron_callback;
    break;
  case GTK_CAULDRON_TYPE_USERDATA_P:
    g_assert(data->results[data->num_results-1].type == GTK_CAULDRON_TYPE_CALLBACK);
    Py_INCREF(item);
    PyTuple_SetItem(data->results[data->num_results-1].d.tuple, 1, item);
    *((PyObject **)result) = data->results[data->num_results-1].d.tuple;
    break;
  }
}

static PyObject *_wrap_gtk_dialog_cauldron(PyObject *self, PyObject *args) {
  PyObject *ret;
  char *title, *fmt;
  int options, i, j;
  cauldron_data data;
  char *result;

  if (!PyArg_ParseTuple(args, "sisO!:gtk_dialog_cauldron", &title,
			&options, &fmt, &PyTuple_Type, &(data.args)))
    return NULL;
  data.arg_len = PyTuple_Size(data.args);
  data.pos = data.num_results = 0;
  data.num_rets = 1; /* we always return at least a string */
  data.results = g_new(cauldron_result, data.arg_len);
  result = gtk_dialog_cauldron_parse(title, options, fmt,
				     (GtkCauldronNextArgCallback)next_arg,
				     &data, NULL);
  if (!result) result = "";
  if (data.num_rets == 1)
    ret = PyString_FromString(result);
  else {
    ret = PyTuple_New(data.num_rets);
    j = 0;
    PyTuple_SetItem(ret, j, PyString_FromString(result));
    for (i = 0; i < data.num_results; i++) {
      switch (data.results[i].type) {
      case GTK_CAULDRON_TYPE_CHAR_P_P:
	PyTuple_SetItem(ret, ++j,PyString_FromString(data.results[i].d.ch_pp));
	break;
      case GTK_CAULDRON_TYPE_INT_P:
	PyTuple_SetItem(ret, ++j, PyInt_FromLong(data.results[i].d.int_p));
	break;
      case GTK_CAULDRON_TYPE_DOUBLE_P:
	PyTuple_SetItem(ret, ++j, PyFloat_FromDouble(data.results[i].d.dbl_p));
	break;
      case GTK_CAULDRON_TYPE_CALLBACK:
	Py_DECREF(data.results[i].d.tuple);
	break;
      default:
	g_assert_not_reached();
	Py_INCREF(Py_None);
	PyTuple_SetItem(ret, ++j, Py_None);
      }
    }
  }
  g_free(data.results);
  return ret;
}
/* --- end of gtkcauldron.h --- */

static void PyGnome_StringCallback(gchar *string, gpointer data) {
  PyObject *ret, *func = (PyObject *)data;

  PyGtk_BlockThreads();
  ret = PyObject_CallFunction(func, "s", string);
  if (ret)
    Py_DECREF(ret);
  else {
    if (PyGtk_FatalExceptions)
      gtk_main_quit();
    else {
      PyErr_Print();
      PyErr_Clear();
    }
  }
  Py_DECREF(func); /* callback only called once */
  PyGtk_UnblockThreads();
}

static void PyGnome_ReplyCallback(gint reply, gpointer data) {
  PyObject *ret, *func = (PyObject *)data;

  PyGtk_BlockThreads();
  ret = PyObject_CallFunction(func, "i", reply);
  if (ret)
    Py_DECREF(ret);
  else {
    if (PyGtk_FatalExceptions)
      gtk_main_quit();
    else {
      PyErr_Print();
      PyErr_Clear();
    }
  }
  Py_DECREF(func); /* callback only called once */
  PyGtk_UnblockThreads();
}

static PyObject *_wrap_gnome_question_dialog(PyObject *self, PyObject *args) {
  char *string;
  PyObject *callback;

  if (!PyArg_ParseTuple(args, "sO:gnome_question_dialog", &string, &callback))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak */
  return PyGtk_New((GtkObject *)gnome_question_dialog(string,
						      PyGnome_ReplyCallback,
						      callback));
}

static PyObject *_wrap_gnome_question_dialog_parented(PyObject *self, PyObject *args) {
  char *string;
  PyObject *callback, *parent;

  if (!PyArg_ParseTuple(args, "sOO!:gnome_question_dialog_parented", &string,
			&callback, &PyGtk_Type, &parent))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak */
  return PyGtk_New((GtkObject *)gnome_question_dialog_parented(
				string, PyGnome_ReplyCallback, callback,
				GTK_WINDOW(PyGtk_Get(parent))));
}

static PyObject *_wrap_gnome_ok_cancel_dialog(PyObject *self, PyObject *args) {
  char *string;
  PyObject *callback;

  if (!PyArg_ParseTuple(args, "sO:gnome_ok_cancel_dialog", &string, &callback))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible referennce leak */
  return PyGtk_New((GtkObject *)gnome_ok_cancel_dialog(string,
						       PyGnome_ReplyCallback,
						       callback));
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_ok_cancel_dialog_parented(PyObject *self, PyObject *args) {
  char *string;
  PyObject *callback, *parent;

  if (!PyArg_ParseTuple(args, "sOO!:gnome_ok_cancel_dialog_parented", &string,
			&callback, &PyGtk_Type, &parent))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "second argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak */
  return PyGtk_New((GtkObject *)gnome_ok_cancel_dialog_parented(
				string, PyGnome_ReplyCallback, callback,
				GTK_WINDOW(PyGtk_Get(parent))));
}

static PyObject *_wrap_gnome_request_dialog(PyObject *self, PyObject *args) {
  int password, max_length;
  char *prompt, *default_text;
  PyObject *callback, *py_parent;
  GtkWindow *parent = NULL;

  if (!PyArg_ParseTuple(args, "isziOO:gnome_request_dialog", &password,
			&prompt, &default_text, &max_length, &callback,
			&py_parent))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "6th argument must be callable");
    return NULL;
  }
  if (PyGtk_Check(py_parent))
    parent = GTK_WINDOW(PyGtk_Get(py_parent));
  else if (py_parent != Py_None) {
    PyErr_SetString(PyExc_TypeError, "7th arg must be a window or None");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak */
  return PyGtk_New((GtkObject *)gnome_request_dialog(password, prompt,
						     default_text, max_length,
						     PyGnome_StringCallback,
						     callback, parent));
}

static PyObject *_wrap_gnome_app_question(PyObject *self, PyObject *args) {
  PyObject *app, *callback;
  char *question;

  if (!PyArg_ParseTuple(args, "O!sO:gnome_app_question", &PyGtk_Type, &app,
			&question, &callback))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "3rd argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak :( */
  gnome_app_question(GNOME_APP(PyGtk_Get(app)), question,
		     PyGnome_ReplyCallback, callback);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_app_ok_cancel(PyObject *self, PyObject *args) {
  PyObject *app, *callback;
  char *question;

  if (!PyArg_ParseTuple(args, "O!sO:gnome_app_ok_cancel", &PyGtk_Type, &app,
			&question, &callback))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "3rd argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak :( */
  gnome_app_ok_cancel(GNOME_APP(PyGtk_Get(app)), question,
		      PyGnome_ReplyCallback, callback);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_app_request_string(PyObject *self, PyObject *args) {
  PyObject *app, *callback;
  char *question;

  if (!PyArg_ParseTuple(args, "O!sO:gnome_app_request_string", &PyGtk_Type,
			&app, &question, &callback))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "3rd argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak :( */
  gnome_app_request_string(GNOME_APP(PyGtk_Get(app)), question,
			   PyGnome_StringCallback, callback);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_app_request_password(PyObject *self, PyObject *args) {
  PyObject *app, *callback;
  char *question;

  if (!PyArg_ParseTuple(args, "O!sO:gnome_app_request_password", &PyGtk_Type,
			&app, &question, &callback))
    return NULL;
  if (!PyCallable_Check(callback)) {
    PyErr_SetString(PyExc_TypeError, "3rd argument must be callable");
    return NULL;
  }
  Py_INCREF(callback); /* possible reference leak :( */
  gnome_app_request_password(GNOME_APP(PyGtk_Get(app)), question,
			     PyGnome_StringCallback, callback);
  Py_INCREF(Py_None);
  return Py_None;
}

#include "gnomeuimodule_impl.c"

static PyMethodDef gnomeuiMethods[] = {
    { "_register_types", _register_types, 1 },
    { "gnome_init", _wrap_gnome_init, 1 },
    { "gnome_about_new", _wrap_gnome_about_new, 1 },
    { "gnome_app_create_menus", _wrap_gnome_app_create_menus, 1 },
    { "gnome_app_create_toolbar", _wrap_gnome_app_create_toolbar, 1 },
    { "gnome_app_insert_menus", _wrap_gnome_app_insert_menus, 1 },
    { "gnome_app_install_menu_hints", _wrap_gnome_app_install_menu_hints, 1 },
    { "gnome_canvas_new", _wrap_gnome_canvas_new, 1 },
    { "gnome_canvas_new_aa", _wrap_gnome_canvas_new_aa, 1 },
    { "gnome_canvas_get_scroll_region", _wrap_gnome_canvas_get_scroll_region, 1 },
    { "gnome_canvas_get_scroll_offsets", _wrap_gnome_canvas_get_scroll_offsets, 1 },
    { "gnome_canvas_w2c", _wrap_gnome_canvas_w2c, 1 },
    { "gnome_canvas_c2w", _wrap_gnome_canvas_c2w, 1 },
    { "gnome_canvas_get_color", _wrap_gnome_canvas_get_color, 1 },
    { "gnome_canvas_get_item_at", _wrap_gnome_canvas_get_item_at, 1 },
    { "gnome_canvas_item_new", _wrap_gnome_canvas_item_new, 1 },
    { "gnome_canvas_item_set", _wrap_gnome_canvas_item_set, 1 },
    { "gnome_canvas_item_affine_relative", _wrap_gnome_canvas_item_affine_relative, 1 },
    { "gnome_canvas_item_affine_absolute", _wrap_gnome_canvas_item_affine_absolute, 1 },
    { "gnome_canvas_item_i2w", _wrap_gnome_canvas_item_i2w, 1 },
    { "gnome_canvas_item_w2i", _wrap_gnome_canvas_item_w2i, 1 },
    { "gnome_canvas_item_get_bounds", _wrap_gnome_canvas_item_get_bounds, 1 },
    { "gnome_canvas_group_children", _wrap_gnome_canvas_group_children, 1 },
    { "gnome_client_set_clone_command", _wrap_gnome_client_set_clone_command, 1 },
    { "gnome_client_set_discard_command", _wrap_gnome_client_set_discard_command, 1 },
    { "gnome_client_set_restart_command", _wrap_gnome_client_set_restart_command, 1 },
    { "gnome_client_set_resign_command", _wrap_gnome_client_set_resign_command, 1 },
    { "gnome_client_set_shutdown_command", _wrap_gnome_client_set_shutdown_command, 1 },
    { "gnome_client_request_interaction", _wrap_gnome_client_request_interaction, 1 },
    { "gnome_color_picker_get_d", _wrap_gnome_color_picker_get_d, 1 },
    { "gnome_color_picker_get_i8", _wrap_gnome_color_picker_get_i8, 1 },
    { "gnome_color_picker_get_i16", _wrap_gnome_color_picker_get_i16, 1 },
    { "gnome_dentry_edit_save_file", _wrap_gnome_dentry_edit_save_file, 1 },
    { "gnome_dialog_button_connect", _wrap_gnome_dialog_button_connect, 1 },
    { "gnome_file_entry_get_full_path", _wrap_gnome_file_entry_get_full_path, 1 },
    { "gnome_icon_list_set_icon_data", _wrap_gnome_icon_list_set_icon_data, 1 },
    { "gnome_icon_list_find_icon_from_data", _wrap_gnome_icon_list_find_icon_from_data, 1 },
    { "gnome_icon_list_get_icon_data", _wrap_gnome_icon_list_get_icon_data, 1 },
    { "gnome_mdi_set_menubar_template", _wrap_gnome_mdi_set_menubar_template, 1 },
    { "gnome_mdi_set_toolbar_template", _wrap_gnome_mdi_set_toolbar_template, 1 },
    { "gnome_mdi_child_set_menu_template", _wrap_gnome_mdi_child_set_menu_template, 1 },
    { "gnome_mdi_generic_child_set_view_creator", _wrap_gnome_mdi_generic_child_set_view_creator, 1 },
    { "gnome_mdi_generic_child_set_menu_creator", _wrap_gnome_mdi_generic_child_set_menu_creator, 1 },
    { "gnome_mdi_generic_child_set_config_func", _wrap_gnome_mdi_generic_child_set_config_func, 1 },
    { "gnome_mdi_generic_child_set_label_func", _wrap_gnome_mdi_generic_child_set_label_func, 1 },
    { "gnome_popup_menu_new", _wrap_gnome_popup_menu_new, 1 },
    { "gnome_popup_menu_attach", _wrap_gnome_popup_menu_attach, 1 },
    { "gnome_widget_add_help_with_uidata", _wrap_gnome_widget_add_help_with_uidata, 1 },
    { "gnome_scores_new", _wrap_gnome_scores_new, 1 },
    { "gnome_stock_menu_accel", _wrap_gnome_stock_menu_accel, 1 },
    { "gtk_dialog_cauldron", _wrap_gtk_dialog_cauldron, 1 },
    { "gnome_question_dialog", _wrap_gnome_question_dialog, 1 },
    { "gnome_question_dialog_parented", _wrap_gnome_question_dialog_parented, 1 },
    { "gnome_ok_cancel_dialog", _wrap_gnome_ok_cancel_dialog, 1 },
    { "gnome_ok_cancel_dialog_parented", _wrap_gnome_ok_cancel_dialog_parented, 1 },
    { "gnome_request_dialog", _wrap_gnome_request_dialog, 1 },
    { "gnome_app_question", _wrap_gnome_app_question, 1 },
    { "gnome_app_ok_cancel", _wrap_gnome_app_ok_cancel, 1 },
    { "gnome_app_request_string", _wrap_gnome_app_request_string, 1 },
    { "gnome_app_request_password", _wrap_gnome_app_request_password, 1 },
#include "gnomeuimodule_defs.c"
    { NULL, NULL, 0 }
};

void init_gnomeui() {
    PyObject *m, *d;

    m = Py_InitModule("_gnomeui", gnomeuiMethods);

    init_pygtk();
    init_pygdkimlib();

    if (PyErr_Occurred())
        Py_FatalError("can't initialise module _gnomeui");
}
