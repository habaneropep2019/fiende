/* gnome-stones - object.h
 *
 * Time-stamp: <1998/11/09 19:21:37 carsten>
 *
 * Copyright (C) 1998 Carsten Schaar
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef OBJECT_H
#define OBJECT_H

#include <config.h>
#include <gnome.h>
#include "types.h"


/*****************************************************************************/

/* The Gnome-Stones objects are located in - so called - plugins.  The
   following functions handle the loading and management of these
   plugins.  */

GStonesPlugin *plugin_load                 (const gchar *filename);
void           plugin_load_plugins_in_dir  (const gchar *directory);

GStonesPlugin *plugin_find_by_title        (const gchar *title);



/*****************************************************************************/
/* Object declarations.  */


#define OBJECT_DEFAULT_IMAGE -1
#define OBJECT_EDITOR_IMAGE -2


GStonesObject *object_register            (GStonesPlugin     *plugin, 
					   GStonesObjectDesc *object);

GStonesObject *object_find_object_by_name (const gchar *name);

GdkPixmap     *object_get_image           (GStonesObject *object, gint index);
GdkImlibImage *object_get_imlib_image     (GStonesObject *object, gint index);

/* Returns the object's fullname, i. e. '<pluginname>:<objectname>'.  */
gchar         *object_get_fullname        (GStonesObject *object);


/*****************************************************************************/
/* Object context declarations.  */


/* general */
GStonesObjContext *object_context_new     (GStonesObject *object);
void     object_context_free              (GStonesObjContext *context);

/* Setting and getting private data.  */
void     object_context_set_private_data  (GStonesObjContext *context,
					   gpointer           private);
gpointer object_context_private_data      (GStonesObjContext *context);

/* Setting and getting options.  */

gboolean object_context_set_option        (GStonesObjContext *context,
					   const gchar       *name,
					   const gchar       *value);

gchar   *object_context_get_string_option (GStonesObjContext *context,
					   const gchar       *name);
gboolean object_context_get_bool_option   (GStonesObjContext *context,
					   const gchar       *name);
gint     object_context_get_int_option    (GStonesObjContext *context,
					   const gchar       *name);
gdouble  object_context_get_float_option  (GStonesObjContext *context,
					   const gchar       *name);


/*****************************************************************************/

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
