/* gnome-stones - object.h
 *
 * Time-stamp: <1998/11/09 19:34:12 carsten>
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
#include "object.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>



/*****************************************************************************/

/* Hash table of all loaded plugins.  */
static GHashTable *plugin_table= NULL;


/* Initialize a plugin.  At least the object_init element should be
   set.  

   This functions call the 'object_init' function, initializes the
   object hash table.  */
static gboolean
plugin_init (GStonesPlugin *plugin)
{
  gchar *title;
  
  /* Initialize the plugin's object table.  */
  plugin->objects= g_hash_table_new (g_str_hash, g_str_equal);
  if (!plugin->objects)
    {
      printf ("Unable to allocate object table\n");
      return FALSE;
    }
  
  /* Call 'object_init' function.  */
  title= plugin->objects_init (plugin);
  if (title == NULL)
    {
      printf ("objects_init returned error");
      return FALSE;
    }

  plugin->title= g_strdup (title);
  if (plugin->title == NULL)
    {
      printf ("not enough memory");
      return FALSE;
    }

  if (!plugin_table)
    {
      plugin_table= g_hash_table_new (g_str_hash, g_str_equal);
      /* FIXME: add error handling here.  */
    }

  /* Add plugin to hash table of all plugins.  */
  g_hash_table_insert (plugin_table, plugin->title, plugin);

  return TRUE;
}


/* Load a plugin.  */

GStonesPlugin *
plugin_load (const gchar *filename)
{
  GStonesPlugin *plugin;
  
  g_return_val_if_fail (filename != NULL, NULL);
  
  plugin= g_new0 (GStonesPlugin, 1);
  if (!plugin)
    {
      g_print ("allocation error");
      return NULL;
    }

  plugin->handle= g_module_open (filename, 0);
  if (!plugin->handle) 
    {
      printf ("Unable to open module file: %s\n", g_module_error());
      g_free (plugin);
      return NULL;
    }
  
  if (!g_module_symbol (plugin->handle, "objects_init", 
			(gpointer *) &plugin->objects_init))
    {
      printf ("Plugin must contain objects_init function.");
      goto error;
    }

  if (!plugin_init (plugin))
    {
      goto error;
    }

  return plugin;
  
 error:
  g_module_close (plugin->handle);
  g_free (plugin);
  return NULL;
}


void
plugin_load_plugins_in_dir (const gchar *directory)
{
  DIR *d;
  struct dirent *e;
  
  if ((d = opendir (directory)) == NULL)
    return;
  
  while ((e = readdir (d)) != NULL)
    {
      if ((strlen (e->d_name) > 3) &&  
	  (strncmp (e->d_name + strlen (e->d_name) - 3, ".so", 3) == 0))
	{
	  char *objects_name;
	  
	  objects_name = g_strconcat (directory, e->d_name, NULL);
	  plugin_load (objects_name);
	  g_free (objects_name);
	}
    }
  closedir (d);
}


GStonesPlugin *
plugin_find_by_title (const gchar *title)
{
  if (!plugin_table)
    return NULL;
  
  return g_hash_table_lookup (plugin_table, title);
}



/*****************************************************************************/
/* We need at least one object.  The FRAME object.  */


static GStonesObjectDesc frame_description=
{
  "frame",

  NULL,
  NULL,
  
  NULL,
  NULL,
  
  "frame.png",
  NULL,
  0, 0
};


gchar *
default_objects_init (GStonesPlugin *plugin)
{
  object_register (plugin, &frame_description);

  return "default";
}


static GStonesPlugin default_plugin=
{
  NULL,
  NULL,
  NULL,
  default_objects_init
};


void
default_objects_register (void)
{
  /* The frame object has only to be registered once.  */
  if (default_plugin.objects)
    return;

  plugin_init (&default_plugin);
}



/*****************************************************************************/
/* Object handling.  */


GStonesObject *
object_register (GStonesPlugin *plugin, GStonesObjectDesc *description)
{
  guint          num_x;
  guint          num_y;
  guint          x;
  guint          y;
  guint          idx;
  char          *filename;
  char          *pathname;
  GdkImlibImage *image;
  GStonesObject *object;

  object= g_new0 (GStonesObject, 1);
  /* Return 'NULL' if there is no memory left.  */
  if (!object)
    return NULL;

  object->description= description;
  object->plugin     = plugin;

  filename= g_strconcat ("gnome-stones/", description->image_name, NULL);
  pathname= gnome_pixmap_file (filename);
  image   = gdk_imlib_load_image (pathname);
  g_free (pathname);
  g_free (filename);
  
  if (!image)
    {
      char *error= g_strconcat ("Error while loading image ", 
				   description->image_name, 
				   ": file not found!", NULL);
      g_warning (error);
      g_free (error);

      return NULL;
    }

  /* Determine the number of images in the loaded image.  */
  num_x= image->rgb_width/STONE_SIZE;
  num_y= image->rgb_height/STONE_SIZE;

  if (num_x*num_y == 0)
    {
      char *error= g_strconcat ("Error while registering object ", 
				   description->image_name, 
				   ": image contains no data!", NULL);
      g_warning (error);
      g_free (error);

      return NULL;
    }

  object->num_images= num_x*num_y;
  object->image= g_malloc (num_x*num_y*sizeof (GdkPixmap*));
  
  if (object->image == NULL)
    {
      char *error= g_strconcat ("Error while registering object ", 
				   description->image_name, 
				   ": out of memory!", NULL);
      g_warning (error);
      g_free (error);

      return NULL;
    }
  
  object->imlib_image= g_malloc (num_x*num_y*sizeof (GdkImlibImage*));
  
  if (object->image == NULL)
    {
      char *error= g_strconcat ("Error while registering object ", 
				   description->image_name, 
				   ": out of memory!", NULL);
      g_warning (error);
      g_free (error);
      g_free (object->image);

      return NULL;
    }
  
  /* We cut our image into small pieces.  */
  /* gdk_imlib_render (image, image->rgb_width, image->rgb_height); */

  idx= 0;
  for (y= 0; y < num_y; y++)
    for (x= 0; x < num_x; x++, idx++)
      {
	/* FIXME: check error conditions.  */
	object->imlib_image[idx]= 
	  gdk_imlib_crop_and_clone_image (image, x*STONE_SIZE, y*STONE_SIZE,
					  STONE_SIZE, STONE_SIZE);
	
	gdk_imlib_render (object->imlib_image[idx], 
			  object->imlib_image[idx]->rgb_width, 
			  object->imlib_image[idx]->rgb_height);

	object->image[idx]= gdk_imlib_copy_image (object->imlib_image[idx]);
      }
  
  /* Add object to the plugin's object table.  */
  g_hash_table_insert (plugin->objects, description->name, object);

  return object;
}


GStonesObject *
object_find_object_by_name (const gchar *name)
{
  gchar         **parts = NULL;
  GStonesObject  *object= NULL;
  
  /* Check arguments.  */
  g_return_val_if_fail (name, NULL);

  /* We have to register the default objects first.  */
  default_objects_register ();

  /* The object's name must be given in the form 'plugin:object', so we
     start parsing the name.  */
  parts= g_strsplit (name, ":", 3);
  
  if (parts && parts[0] && parts[1] && !parts[2])
    {
      GStonesPlugin *plugin= g_hash_table_lookup (plugin_table, parts[0]);
      
      if (plugin)
	object= g_hash_table_lookup (plugin->objects, parts[1]);
    }
  
  g_strfreev (parts);
  
  return object;
}


GdkPixmap *
object_get_image (GStonesObject *object, gint index)
{
  g_return_val_if_fail (object, NULL);
  /* FIXME: add range checks.  */
  
  if (index == OBJECT_DEFAULT_IMAGE)
    index= object->image_index;
  else if (index == OBJECT_EDITOR_IMAGE)
    index= object->editor_index;
  
  g_return_val_if_fail ((index >= 0) && (index <= object->num_images), NULL);

  return object->image[index];
}


GdkImlibImage *
object_get_imlib_image (GStonesObject *object, gint index)
{
  g_return_val_if_fail (object, NULL);
  
  if (index == OBJECT_DEFAULT_IMAGE)
    index= object->image_index;
  else if (index == OBJECT_EDITOR_IMAGE)
    index= object->editor_index;
  
  g_return_val_if_fail ((index >= 0) && (index <= object->num_images), NULL);

  return object->imlib_image[index];
}


gchar *
object_get_fullname (GStonesObject *object)
{
  g_return_val_if_fail (object, NULL);

  return g_strconcat (object->plugin->title, ":", 
			 object->description->name, NULL);
}

/*****************************************************************************/
/* Object context definitions.  */


struct _GStonesObjContext
{
  /* The object's description data.  */
  GStonesObjectDesc *description;

  /* The options.  */
  GHashTable        *options;

  /* Some private data.  */
  gpointer           private;
};


/* general */
GStonesObjContext *
object_context_new (GStonesObject *object)
{
  GStonesObjContext *context;

  g_return_val_if_fail (object, NULL);
  
  context= g_new0 (GStonesObjContext, 1);
  if (!context)
    return NULL;
 
  context->description= object->description;

  context->options    = g_hash_table_new (g_str_hash, g_str_equal);
  if (!context->options)
    {
      g_free (context);
      return NULL;
    }
  
  return context;
}


static gboolean
object_context_free_option (gchar *name, gchar *value, gpointer user)
{
  g_free (name);
  g_free (value);
  
  return TRUE;
}

void
object_context_free (GStonesObjContext *context)
{
  g_return_if_fail (context);
  
  g_hash_table_foreach_remove 
    (context->options, (GHRFunc) object_context_free_option, NULL);

  g_free (context);
}


/* Setting and getting private data.  */
void     
object_context_set_private_data (GStonesObjContext *context,
				 gpointer           private)
{
  g_return_if_fail (context);
  
  context->private= private;  
}

gpointer 
object_context_private_data (GStonesObjContext *context)
{
  g_return_val_if_fail (context, NULL);

  return context->private;
}


/* Setting and getting options.  */

gboolean
object_context_set_option (GStonesObjContext *context,
			   const gchar       *name,
			   const gchar       *value)
{
  gchar *old_value;
  gchar *old_name;

  g_return_val_if_fail (context, FALSE);
  g_return_val_if_fail (name, FALSE);
  
  if (g_hash_table_lookup_extended (context->options,
				    name,
				    (gpointer) &old_name,
				    (gpointer) &old_value))
    {
      /* Remove old options.  */
      g_hash_table_remove (context->options, name);
      g_free (old_name);
      g_free (old_value);
    }
  
  if (value)
    {
      /* Set a new option.  */
      g_hash_table_insert 
	(context->options, g_strdup (name), g_strdup (value));
    }

  /* FIXME: Add empty memory checks here.  */
  return TRUE;
}


/* A small helper function, needed by
   'object_context_get_string_option'.  */
static GStonesObjectOption *
find_option_by_name (GStonesObjectDesc *desc, const gchar *name)
{
  GStonesObjectOption *option;
  gchar               *tmp_name;  
  
  option= desc->options;

  /* Check, if this object has any options.  */
  if (!option)
    return NULL;

  tmp_name= g_strconcat (name, "=", NULL);

  while (option->name)
    {
      if (strncmp (option->name, tmp_name, strlen (tmp_name)) == 0)
	break;

      /* Check next option.  */
      option++;
    }  

  g_free (tmp_name);

  if (option->name)
    return option;
  else
    return NULL;
}


gchar *
object_context_get_string_option (GStonesObjContext *context,
				  const gchar       *name)
{
  gchar *value;

  g_return_val_if_fail (context, NULL);
  g_return_val_if_fail (name, NULL);
  
  value= g_hash_table_lookup (context->options, name);

  if (value)
    return g_strdup (value);
  else
    {
      /* We try to get the option's default value.  */
      gchar *temp;
      GStonesObjectOption *option= find_option_by_name 
	(context->description, name);
  
      if (!option)
	return NULL;
  
      temp= strchr (option->name, '=');
      /* We can be sure, that tmp is not equal to 'NULL', because otherwise 
	 'object_find_option_by_name' would not have found this option.  */
      temp++;
      return g_strdup (temp);
    }
}

gboolean 
object_context_get_bool_option (GStonesObjContext *context,
				const gchar       *name)
{
  gchar *value= object_context_get_string_option (context, name);
  
  if (value)
    {
      gboolean v= FALSE;
      
      if (!strcasecmp (value, "true"))
	{
	  v= TRUE;
	}
      else if (!strcasecmp (value, "1"))
	{
	  v= TRUE;
	} 

      g_free (value);
      
      return v;
    }
  
  return FALSE;
}

gint     
object_context_get_int_option (GStonesObjContext *context,
			       const gchar       *name)
{
  gchar *value= object_context_get_string_option (context, name);
  
  if (value)
    {
      /* FIXME: Set locale here?  */
      int v= atoi (value);
      
      g_free (value);
      
      return v;
    }

  return 0;
}

gdouble
object_context_get_float_option (GStonesObjContext *context,
				 const gchar       *name)
{
  gchar *value= object_context_get_string_option (context, name);
  
  if (value)
    {
      /* FIXME: Set locale here?  */
      gdouble v= strtod (value, NULL);
      
      g_free (value);
      
      return v;
    }

  return 0;
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */

