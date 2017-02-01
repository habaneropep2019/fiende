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
#include "libgnome/libgnome.h"
#include "libgnome/gnome-fileconvert.h"
#include "libgnome/gnome-history.h"

static PyObject *_wrap_gnome_config_get_string(PyObject *self, PyObject *args) {
  char *path, *ret;

  if (!PyArg_ParseTuple(args, "s:gnome_config_get_string", &path))
    return NULL;
  ret = gnome_config_get_string(path);
  if (ret) {
    PyObject *py_ret = PyString_FromString(ret);
    g_free(ret);
    return py_ret;
  }
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_config_private_get_string(PyObject *self, PyObject *args) {
  char *path, *ret;

  if (!PyArg_ParseTuple(args, "s:gnome_config_private_get_string", &path))
    return NULL;
  ret = gnome_config_private_get_string(path);
  if (ret) {
    PyObject *py_ret = PyString_FromString(ret);
    g_free(ret);
    return py_ret;
  }
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_config_get_vector(PyObject *self, PyObject *args) {
  char *path;
  int argc, i;
  char **argv;
  PyObject *ret;

  if (!PyArg_ParseTuple(args, "s:gnome_config_get_vector", &path))
    return NULL;
  gnome_config_get_vector(path, &argc, &argv);
  ret = PyTuple_New(argc);
  for (i = 0; i < argc; i++)
    PyTuple_SetItem(ret, i, PyString_FromString(argv[i]));
  return ret;
}

static PyObject *_wrap_gnome_config_private_get_vector(PyObject *self, PyObject *args) {
  char *path;
  int argc, i;
  char **argv;
  PyObject *ret;

  if (!PyArg_ParseTuple(args, "s:gnome_config_private_get_vector", &path))
    return NULL;
  gnome_config_private_get_vector(path, &argc, &argv);
  ret = PyTuple_New(argc);
  for (i = 0; i < argc; i++)
    PyTuple_SetItem(ret, i, PyString_FromString(argv[i]));
  return ret;
}

static PyObject *_wrap_gnome_config_set_vector(PyObject *self, PyObject *args) {
  char *path;
  PyObject *seq, *item;
  int argc, i;
  char **argv;
  if (!PyArg_ParseTuple(args, "sO:gnome_config_set_vector", &path, &seq))
    return NULL;
  if (!PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "second argument not a sequence");
    return NULL;
  }
  argc = PySequence_Length(seq);
  argv = g_new(char *, argc);
  for (i = 0; i < argc; i++) {
    item = PySequence_GetItem(seq, i);
    Py_DECREF(item);
    if (!PyString_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "sequence member not a string");
      g_free(argv);
      return NULL;
    }
    argv[i] = PyString_AsString(item);
  }
  gnome_config_set_vector(path, argc, (const char **)argv);
  g_free(argv);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_config_private_set_vector(PyObject *self, PyObject *args) {
  char *path;
  PyObject *seq, *item;
  int argc, i;
  char **argv;
  if (!PyArg_ParseTuple(args, "sO:gnome_config_private_set_vector",
			&path, &seq))
    return NULL;
  if (!PySequence_Check(seq)) {
    PyErr_SetString(PyExc_TypeError, "second argument not a sequence");
    return NULL;
  }
  argc = PySequence_Length(seq);
  argv = g_new(char *, argc);
  for (i = 0; i < argc; i++) {
    item = PySequence_GetItem(seq, i);
    Py_DECREF(item);
    if (!PyString_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "sequence member not a string");
      g_free(argv);
      return NULL;
    }
    argv[i] = PyString_AsString(item);
  }
  gnome_config_private_set_vector(path, argc, (const char **)argv);
  g_free(argv);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_config_section_contents(PyObject *self,PyObject *args) {
  char *path, *key, *value;
  void *iterator;
  PyObject *ret, *v;
  
  if (!PyArg_ParseTuple(args, "s:gnome_config_section_contents", &path))
    return NULL;
  iterator = gnome_config_init_iterator(path);
  ret = PyDict_New();
  while (iterator) {
    iterator = gnome_config_iterator_next(iterator, &key, &value);
    PyDict_SetItemString(ret, key, v=PyString_FromString(value));
    Py_DECREF(v);
  }
  return ret;
}

static PyObject *_wrap_gnome_config_enum_sections(PyObject *self, PyObject *args) {
  char *path, *key;
  void *iterator;
  PyObject *ret;
  if (!PyArg_ParseTuple(args, "s:gnome_config_enum_sections", &path))
    return NULL;
  iterator = gnome_config_init_iterator_sections(path);
  ret = PyList_New(0);
  while (iterator) {
    iterator = gnome_config_iterator_next(iterator, &key, NULL);
    PyList_Append(ret, PyString_FromString(key));
  }
  return ret;
}

static PyObject *_wrap_gnome_config_private_section_contents(PyObject *self,PyObject *args) {
  char *path, *key, *value;
  void *iterator;
  PyObject *ret, *v;
  
  if (!PyArg_ParseTuple(args, "s:gnome_config_private_section_contents",&path))
    return NULL;
  iterator = gnome_config_private_init_iterator(path);
  ret = PyDict_New();
  while (iterator) {
    iterator = gnome_config_iterator_next(iterator, &key, &value);
    PyDict_SetItemString(ret, key, v=PyString_FromString(value));
    Py_DECREF(v);
  }
  return ret;
}

static PyObject *_wrap_gnome_config_private_enum_sections(PyObject *self, PyObject *args) {
  char *path, *key;
  void *iterator;
  PyObject *ret;
  if (!PyArg_ParseTuple(args, "s:gnome_config_private_enum_sections", &path))
    return NULL;
  iterator = gnome_config_private_init_iterator_sections(path);
  ret = PyList_New(0);
  while (iterator) {
    iterator = gnome_config_iterator_next(iterator, &key, NULL);
    PyList_Append(ret, PyString_FromString(key));
  }
  return ret;
}

static PyObject *_wrap_gnome_file_convert_stream(PyObject *self, PyObject *args) {
  PyObject *in_file;
  int fd, out_fd;
  char *fromtype, *totype;
  FILE *fp;

  if (!PyArg_ParseTuple(args, "O!ss:gnome_file_convert_stream", &PyFile_Type,
                        &in_file, &fromtype, &totype))
    return NULL;
  fd = fileno(PyFile_AsFile(in_file));
  out_fd = gnome_file_convert_fd(fd, fromtype, totype);
  if (fd < 0 || (fp = fdopen(out_fd, "rb")) == NULL) {
    PyErr_SetString(PyExc_IOError, "can't make conversion");
    return NULL;
  }
  return PyFile_FromFile(fp, "<conversion>", "rb", fclose);
}

static PyObject *_wrap_gnome_file_convert(PyObject *self, PyObject *args) {
  char *filename, *fromtype, *totype;
  int fd;
  FILE *fp;
  if (!PyArg_ParseTuple(args, "sss:gnome_file_convert", &filename, &fromtype,
                        &totype))
    return NULL;
  fd = gnome_file_convert(filename, fromtype, totype);
  if (fd < 0 || (fp = fdopen(fd, "rb")) == NULL) {
    PyErr_SetString(PyExc_IOError, "can't make conversion");
    return NULL;
  }
  return PyFile_FromFile(fp, "<conversion>", "rb", fclose);
}

static PyObject *_wrap_gnome_help_display(PyObject *self, PyObject *args) {
  GnomeHelpMenuEntry ref;
  if (!PyArg_ParseTuple(args, "ss:gnome_help_display", &(ref.name),
                        &(ref.path)))
    return NULL;
  gnome_help_display(NULL, &ref);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_help_goto(PyObject *self, PyObject *args) {
  char *file;
  if (!PyArg_ParseTuple(args, "s:gnome_help_goto", &file))
    return NULL;
  gnome_help_goto(NULL, file);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_history_get_recently_used(PyObject *self,
						       PyObject *args) {
  GList *list;
  PyObject *py_list;
  GnomeHistoryEntry entry;
  if (!PyArg_ParseTuple(args, ":gnome_history_get_recently_used"))
    return NULL;
  list = gnome_history_get_recently_used();
  py_list = PyList_New(0);
  while (list) {
    entry = list->data;
    PyList_Append(py_list, Py_BuildValue("(ssss)", entry->filename,
                                         entry->filetype, entry->creator,
                                         entry->desc));
    list = g_list_next(list);
  }
  gnome_history_free_recently_used_list(list);
  return py_list;
}

/* translate return value */
static PyObject *metadata_set_ret(int retval) {
  if (retval == GNOME_METADATA_IO_ERROR) {
    PyErr_SetString(PyExc_IOError, "low-level communications/storage error");
    return NULL;
  } else if (retval == GNOME_METADATA_NOT_FOUND) {
    PyErr_SetString(PyExc_KeyError, "metadata attribute not found");
    return NULL;
  } else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}
static PyObject *_wrap_gnome_metadata_set(PyObject *self, PyObject *args) {
  char *file, *key, *value;
  int vallen;
  if (!PyArg_ParseTuple(args, "sss#:gnome_metadata_set", &file, &key, &value,
			&vallen))
    return NULL;
  return metadata_set_ret(gnome_metadata_set(file, key, vallen, value));
}
static PyObject *_wrap_gnome_metadata_remove(PyObject *self, PyObject *args) {
  char *file, *key;
  if (!PyArg_ParseTuple(args, "ss:gnome_metadata_remove", &file, &key))
    return NULL;
  return metadata_set_ret(gnome_metadata_remove(file, key));
}
static PyObject *_wrap_gnome_metadata_list(PyObject *self, PyObject *args) {
  char *file, **ret, **tmp;
  PyObject *list;
  if (!PyArg_ParseTuple(args, "s:gnome_metadata_list", &file))
    return NULL;
  ret = gnome_metadata_list(file);
  list = PyList_New(0);
  for (tmp = ret; *tmp != NULL; tmp++)
    PyList_Append(list, PyString_FromString(*tmp));
  g_strfreev(ret);
  return list;
}
static PyObject *_wrap_gnome_metadata_get(PyObject *self, PyObject *args) {
  char *file, *key, *value;
  int valuelen;
  PyObject *ret;
  if (!PyArg_ParseTuple(args, "ss:gnome_metadata_get", &file, &key))
    return NULL;
  ret = metadata_set_ret(gnome_metadata_get(file, key, &valuelen, &value));
  if (ret == NULL) return NULL;
  Py_DECREF(ret);
  ret = PyString_FromStringAndSize(value, valuelen);
  g_free(value);
  return ret;
}
static PyObject *_wrap_gnome_metadata_get_fast(PyObject *self, PyObject *args) {
  char *file, *key, *value;
  int valuelen;
  PyObject *ret;
  if (!PyArg_ParseTuple(args, "ss:gnome_metadata_get_fast", &file, &key))
    return NULL;
  ret = metadata_set_ret(gnome_metadata_get_fast(file, key, &valuelen,&value));
  if (ret == NULL) return NULL;
  Py_DECREF(ret);
  ret = PyString_FromStringAndSize(value, valuelen);
  g_free(value);
  return ret;
}
static PyObject *_wrap_gnome_metadata_rename(PyObject *self, PyObject *args) {
  char *file, *to;
  if (!PyArg_ParseTuple(args, "ss:gnome_metadata_rename", &file, &to))
    return NULL;
  return metadata_set_ret(gnome_metadata_rename(file, to));
}
static PyObject *_wrap_gnome_metadata_copy(PyObject *self, PyObject *args) {
  char *file, *to;
  if (!PyArg_ParseTuple(args, "ss:gnome_metadata_copy", &file, &to))
    return NULL;
  return metadata_set_ret(gnome_metadata_copy(file, to));
}
static PyObject *_wrap_gnome_metadata_delete(PyObject *self, PyObject *args) {
  char *file;
  if (!PyArg_ParseTuple(args, "s:gnome_metadata_delete", &file))
    return NULL;
  return metadata_set_ret(gnome_metadata_delete(file));
}
static PyObject *_wrap_gnome_metadata_regex_add(PyObject *self, PyObject *args) {
  char *regex, *key, *value;
  int vallen;
  if (!PyArg_ParseTuple(args, "sss#:gnome_metadata_regex_add", &regex, &key,
			&value, &vallen))
    return NULL;
  gnome_metadata_regex_add(regex, key, vallen, value);
  Py_INCREF(Py_None);
  return Py_None;
}
static PyObject *_wrap_gnome_metadata_regex_remove(PyObject *self, PyObject *args) {
  char *regex, *key;
  if (!PyArg_ParseTuple(args, "ss:gnome_metadata_regex_remove", &regex, &key))
    return NULL;
  gnome_metadata_regex_remove(regex, key);
  Py_INCREF(Py_None);
  return Py_None;
}
static PyObject *_wrap_gnome_metadata_type_add(PyObject *self, PyObject *args) {
  char *type, *key, *value;
  int vallen;
  if (!PyArg_ParseTuple(args, "sss#:gnome_metadata_type_add", &type, &key,
			&value, &vallen))
    return NULL;
  gnome_metadata_type_add(type, key, vallen, value);
  Py_INCREF(Py_None);
  return Py_None;
}
static PyObject *_wrap_gnome_metadata_type_remove(PyObject *self, PyObject *args) {
  char *type, *key;
  if (!PyArg_ParseTuple(args, "ss:gnome_metadata_type_remove", &type, &key))
    return NULL;
  gnome_metadata_type_remove(type, key);
  Py_INCREF(Py_None);
  return Py_None;
}
static PyObject *_wrap_gnome_metadata_lock(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, ":gnome_metadata_lock"))
    return NULL;
  gnome_metadata_lock();
  Py_INCREF(Py_None);
  return Py_None;
}
static PyObject *_wrap_gnome_metadata_unlock(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, ":gnome_metadata_unlock"))
    return NULL;
  gnome_metadata_unlock();
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_mime_get_keys(PyObject *self, PyObject *args) {
  char *mime_type;
  GList *ret, *tmp;
  PyObject *list;

  if (!PyArg_ParseTuple(args, "s:gnome_mime_get_keys", &mime_type))
    return NULL;
  ret = gnome_mime_get_keys(mime_type);
  list = PyList_New(0);
  for (tmp = ret; tmp != NULL; tmp = tmp->next)
    PyList_Append(list, PyString_FromString(tmp->data));
  g_list_free(ret);
  return list;
}

static PyObject *_wrap_gnome_score_log(PyObject *self, PyObject *args) {
  double score;
  char *level = NULL;
  int high_to_low;
  if (!PyArg_ParseTuple(args, "fzi:gnome_score_log", &score, &level,
                        &high_to_low))
    return NULL;
  return PyInt_FromLong(gnome_score_log(score, level, high_to_low));
}

static PyObject *_wrap_gnome_score_get_notable(PyObject*self, PyObject *args) {
  char *gamename = NULL, *level = NULL, **names;
  gfloat *scores;
  time_t *scoretimes;
  PyObject *ret;
  int len, i;
  if (!PyArg_ParseTuple(args, "zz:gnome_score_get_notable", &gamename, &level))
    return NULL;
  len = gnome_score_get_notable(gamename, level, &names, &scores, &scoretimes);
  ret = PyList_New(len);
  for (i = 0; i < len; i++) {
    PyList_SetItem(ret, i, Py_BuildValue("(sfi)", names[i], scores[i],
                                         scoretimes[i]));
    g_free(names[i]);
  }
  g_free(names);
  g_free(scores);
  g_free(scoretimes);
  return ret;
}

static PyObject *_wrap_gnome_triggers_add_trigger(PyObject *self, PyObject *args) {
  int len, i;
  GnomeTrigger nt;
  PyObject *two, *supinf, *item;
  char **supinfo;

  if (!PyArg_ParseTuple(args, "iOsO!:gnome_triggers_add_trigger", &(nt.type),
			&two, &(nt.level), &PyTuple_Type, &supinf))
    return NULL;
  switch (nt.type) {
  case GTRIG_NONE:
    break;
  /*GTRIG_FUNCTION:*/
  case GTRIG_COMMAND:
    if (!PyString_Check(two)) {
      PyErr_SetString(PyExc_TypeError, "second arg not a string");
      return NULL;
    }
    nt.u.command = PyString_AsString(two);
    break;
  case GTRIG_MEDIAPLAY:
    if (!PyString_Check(two)) {
      PyErr_SetString(PyExc_TypeError, "second arg not a string");
      return NULL;
    }
    nt.u.media.file = PyString_AsString(two);
    break;
  default:
    PyErr_SetString(PyExc_TypeError, "unsupported trigger type");
    return NULL;
  }
  len = PyTuple_Size(supinf);
  supinfo = g_new(char *, len+1);
  supinfo[len] = NULL;
  for (i = 0; i < len; i++) {
    item = PyTuple_GetItem(supinf, i);
    if (!PyString_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "sequence member not a string");
      g_free(supinfo);
      return NULL;
    }
    supinfo[i] = PyString_AsString(item);
  }
  gnome_triggers_vadd_trigger(&nt, supinfo);
  g_free(supinfo);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_wrap_gnome_triggers_do(PyObject *self, PyObject *args) {
  PyObject *supinf, *item;
  int len, i;
  char *msg, *level, **supinfo;

  if (!PyArg_ParseTuple(args, "ssO!:gnome_triggers_do", &msg, &level,
			&PyTuple_Type, &supinf))
    return NULL;
  len = PyTuple_Size(supinf);
  supinfo = g_new(char *, len+1);
  supinfo[len] = NULL;
  for (i = 0; i < len; i++) {
    item = PyTuple_GetItem(supinf, i);
    if (!PyString_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "sequence member not a string");
      g_free(supinfo);
      return NULL;
    }
    supinfo[i] = PyString_AsString(item);
  }
  gnome_triggers_vdo(msg, level, (const char **)supinfo);
  g_free(supinfo);
  Py_INCREF(Py_None);
  return Py_None;
}

#include "gnomemodule_impl.c"

static PyMethodDef _gnomeMethods[] = {
    { "gnome_config_get_string", _wrap_gnome_config_get_string, 1 },
    { "gnome_config_private_get_string", _wrap_gnome_config_private_get_string, 1 },
    { "gnome_config_get_vector", _wrap_gnome_config_get_vector, 1 },
    { "gnome_config_private_get_vector", _wrap_gnome_config_private_get_vector, 1 },
    { "gnome_config_set_vector", _wrap_gnome_config_set_vector, 1 },
    { "gnome_config_private_set_vector", _wrap_gnome_config_private_set_vector, 1 },
    { "gnome_config_section_contents", _wrap_gnome_config_section_contents, 1 },
    { "gnome_config_enum_sections", _wrap_gnome_config_enum_sections, 1 },
    { "gnome_config_private_section_contents", _wrap_gnome_config_private_section_contents, 1 },
    { "gnome_config_private_enum_sections", _wrap_gnome_config_private_enum_sections, 1 },
    { "gnome_file_convert_stream", _wrap_gnome_file_convert_stream, 1 },
    { "gnome_file_convert", _wrap_gnome_file_convert, 1 },
    { "gnome_help_display", _wrap_gnome_help_display, 1 },
    { "gnome_help_goto", _wrap_gnome_help_goto, 1 },
    { "gnome_history_get_recently_used", _wrap_gnome_history_get_recently_used, 1 },
    { "gnome_metadata_set", _wrap_gnome_metadata_set, 1 },
    { "gnome_metadata_remove", _wrap_gnome_metadata_remove, 1 },
    { "gnome_metadata_list", _wrap_gnome_metadata_list, 1 },
    { "gnome_metadata_get", _wrap_gnome_metadata_get, 1 },
    { "gnome_metadata_get_fast", _wrap_gnome_metadata_get_fast, 1 },
    { "gnome_metadata_rename", _wrap_gnome_metadata_rename, 1 },
    { "gnome_metadata_copy", _wrap_gnome_metadata_copy, 1 },
    { "gnome_metadata_delete", _wrap_gnome_metadata_delete, 1 },
    { "gnome_metadata_regex_add", _wrap_gnome_metadata_regex_add, 1 },
    { "gnome_metadata_regex_remove", _wrap_gnome_metadata_regex_remove, 1 },
    { "gnome_metadata_type_add", _wrap_gnome_metadata_type_add, 1 },
    { "gnome_metadata_type_remove", _wrap_gnome_metadata_type_remove, 1 },
    { "gnome_metadata_lock", _wrap_gnome_metadata_lock, 1 },
    { "gnome_metadata_unlock", _wrap_gnome_metadata_unlock, 1 },
    { "gnome_mime_get_keys", _wrap_gnome_mime_get_keys, 1 },
    { "gnome_score_log", _wrap_gnome_score_log, 1 },
    { "gnome_score_get_notable", _wrap_gnome_score_get_notable, 1 },
    { "gnome_triggers_add_trigger", _wrap_gnome_triggers_add_trigger, 1 },
    { "gnome_triggers_do", _wrap_gnome_triggers_do, 1 },
#include "gnomemodule_defs.c"
    { NULL, NULL, 0 }
};

void init_gnome() {
    PyObject *m, *d;

    m = Py_InitModule("_gnome", _gnomeMethods);
    d = PyModule_GetDict(m);

    if (PyErr_Occurred())
        Py_FatalError("can't initialise module _gnome");
}

