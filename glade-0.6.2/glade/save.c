/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998  Damon Chaplin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "gladeconfig.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <time.h>
#include <errno.h>
#include <locale.h>

#include "glade_project.h"
#include "gbwidget.h"
#include "save.h"
#include "utils.h"


/* An internal struct to pass data to the save_component() and
   save_named_style_callback() callbacks. */
typedef struct _GladeSaveCallbackData GladeSaveCallbackData;
struct _GladeSaveCallbackData
{
  GbWidgetGetArgData *data;
  FILE *fp;
};

static GladeError* save_project_file_internal (GladeProject *project);

static void save_component (GtkWidget * item,
			    GladeSaveCallbackData * save_data);

#ifdef GLADE_STYLE_SUPPORT
static void save_named_style_callback (const gchar * name,
				       GbStyle * gbstyle,
				       GladeSaveCallbackData * save_data);
static void save_named_style (const gchar * name,
			      GbStyle * gbstyle,
			      GbWidgetGetArgData * data,
			      FILE *fp);
#endif

static void save_buffer_add_string (GString * buffer,
				    const gchar * string);
static void save_buffer_flush (GbWidgetGetArgData * data,
			       FILE *fp);

static void save_indent (GbWidgetGetArgData * data);

static void save_add_translatable_string (GbWidgetGetArgData * data,
					  const gchar * string);
static void save_translatable_strings (GbWidgetGetArgData * data);


/* We need this to make sure that numbers are output in a portable syntax,
   instead of using the current locale. This code is from glibc info docs. */
GladeError*
save_project_file (GladeProject *project)
{
  gchar *old_locale, *saved_locale;
  GladeError *error;
     
  old_locale = setlocale (LC_NUMERIC, NULL);
  saved_locale = g_strdup (old_locale);
  setlocale (LC_NUMERIC, "C");
  error = save_project_file_internal (project);
  setlocale (LC_NUMERIC, saved_locale);
  g_free (saved_locale);
  return error;
}


/* The main function to output the XML. It creates and initializes the
   GbWidgetGetArgData, opens the file, outputs the project options, the
   named styles, and then each component (window/dialog) in the interface. */
static GladeError*
save_project_file_internal (GladeProject *project)
{
  GbWidgetGetArgData data;
  GladeSaveCallbackData save_data;
  gchar *filename, *backup_filename;
  FILE *fp;
  int status;

  MSG ("Saving project");

  data.project = project;
  data.action = GB_SAVING;
  data.copying_to_clipboard = FALSE;
  data.error = NULL;

  filename = glade_project_get_xml_filename (project);

  /* Backup the file, if it already exists. */
  if (glade_util_file_exists (filename))
    {
      backup_filename = g_strdup_printf ("%s.bak", filename);
#if defined (__EMX__) || defined (_WIN32)
      /* for OS/2 rename dosn't work if the dest. file exist ! remove it! */
      status = remove (backup_filename);
#endif
      status = rename (filename, backup_filename);

      if (status == -1)
	{
	  data.error = glade_error_new_system (_("Couldn't rename file:\n  %s\nto:\n  %s\n"), filename, backup_filename);
	  g_free (backup_filename);
	  return data.error;
	}
      g_free (backup_filename);
    }

  fp = fopen (filename, "w");
  if (fp == NULL)
    return glade_error_new_system (_("Couldn't create file:\n  %s\n"),
				   filename);

  /* Initialize the output buffer. */
  data.buffer = g_string_sized_new (1024);
  data.indent = 0;

  /* See if we need the translatable strings file output. */
  data.save_translatable_strings = glade_project_get_output_translatable_strings (data.project);
  data.translatable_strings = NULL;
  if (data.save_translatable_strings)
    data.translatable_strings = g_string_sized_new (1024);

  /* Output the XML version info and our root element '<GTK-Interface>'. */
  fprintf (fp, "<?xml version=\"1.0\"?>\n<GTK-Interface>\n\n");

  /* Save the general project information. */
  glade_project_save_options (data.project, &data);
  save_buffer_flush (&data, fp);

  /* Set up the struct to pass data to the callbacks. */
  save_data.data = &data;
  save_data.fp = fp;

#ifdef GLADE_STYLE_SUPPORT
  if (!data.error)
    {
      /* Save default gbstyle first. */
      MSG ("Saving styles");
      save_named_style (gb_widget_default_gb_style->name,
			gb_widget_default_gb_style, &data, fp);
      /* Now save all the other named styles. */
      g_hash_table_foreach (gb_style_hash, (GHFunc) save_named_style_callback,
			    &save_data);
    }
#endif

  if (!data.error)
    {
      MSG ("Saving components");
      glade_project_foreach_component (data.project,
				       (GtkCallback) save_component,
				       &save_data);
    }

  /* Finish the root element. */
  if (!data.error)
    {
      fprintf (fp, "\n</GTK-Interface>\n");
    }

  /* Save the translatable strings file, if needed. */
  if (!data.error)
    {
      if (data.save_translatable_strings)
	save_translatable_strings (&data);
    }

  /* Free any memory used while saving. */
  g_string_free (data.buffer, TRUE);
  if (data.translatable_strings)
    g_string_free (data.translatable_strings, TRUE);

  fclose (fp);

  return data.error;
}


/* This is called when iterating over the components in a project, to output
   each component. It simply calls gb_widget_save() to recursively save the
   XML for the component into the buffer, and it then flushes the buffer to
   the output file. */
static void
save_component (GtkWidget * component,
		GladeSaveCallbackData * save_data)
{
  /* If an error has occurred, we return. */
  if (save_data->data->error)
    return;

  gb_widget_save (component, save_data->data);
  save_buffer_flush (save_data->data, save_data->fp);
}


/* This is called when iterating over the GHashTable of named styles.
   If the style isn't the default style it is output here. The default
   style is output first, since for all other styles we only output the
   differences from the default. */
#ifdef GLADE_STYLE_SUPPORT
static void
save_named_style_callback (const gchar * name,
			   GbStyle * gbstyle,
			   GladeSaveCallbackData * save_data)
{
  /* If an error has occurred, or this is the default GbStyle, we return. */
  if (save_data->data->error || gbstyle == gb_widget_default_gb_style)
    {
      return;
    }

  save_named_style (name, gbstyle, save_data->data, save_data->fp);
}


/* Outputs a named style. */
static void
save_named_style (const gchar * name,
		  GbStyle * gbstyle,
		  GbWidgetGetArgData * data,
		  FILE *fp)
{
  gboolean save_all = FALSE;

  MSG1 ("Saving style: %s", name);
  /* If this is the default style, only save it if it is different to the
     GTK default style, and if it is make sure we save everything. */
  if (gbstyle == gb_widget_default_gb_style)
    {
      if (gbstyle->style == gtk_widget_get_default_style ())
	return;
      else
	save_all = TRUE;
    }
  gb_widget_save_style (gbstyle, data, save_all);
  save_buffer_flush (data, fp);
}
#endif


/* Adds a start tag, e.g. "<widget>", to the output buffer. */
void
save_start_tag (GbWidgetGetArgData * data, const gchar * tag_name)
{
  save_indent (data);
  g_string_append_c (data->buffer, '<');
  g_string_append (data->buffer, tag_name);
  g_string_append (data->buffer, ">\n");
  data->indent++;
}


/* Adds an end tag, e.g. "</widget>", to the output buffer. */
void
save_end_tag (GbWidgetGetArgData * data, const gchar * tag_name)
{
  data->indent--;
  save_indent (data);
  g_string_append (data->buffer, "</");
  g_string_append (data->buffer, tag_name);
  g_string_append (data->buffer, ">\n");
}


/* Starts a new line in the output buffer (without indenting). */
void
save_newline (GbWidgetGetArgData * data)
{
  g_string_append_c (data->buffer, '\n');
}


/* These functions are called to save different types of widget properties.
   They all convert the property to a string representation and call
   save_string() to output it. The tag_name is usually the long name of the
   property, e.g. "GtkLabel::justify", so we cut out the first part and output
   <justify>...</justify>. */

void
save_string (GbWidgetGetArgData * data, const gchar * tag_name,
	     const gchar * tag_value)
{
  gchar *tag_name_start;

  if (tag_value == NULL)
    return;

  tag_name_start = glade_util_find_start_of_tag_name (tag_name);
  save_indent (data);
  g_string_append_c (data->buffer, '<');
  g_string_append (data->buffer, tag_name_start);
  g_string_append_c (data->buffer, '>');
  save_buffer_add_string (data->buffer, tag_value);
  g_string_append (data->buffer, "</");
  g_string_append (data->buffer, tag_name_start);
  g_string_append (data->buffer, ">\n");
}


void
save_translatable_string (GbWidgetGetArgData * data, const gchar * tag_name,
			  const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
  if (data->save_translatable_strings)
    save_add_translatable_string (data, tag_value);
}


void
save_text (GbWidgetGetArgData * data, const gchar * tag_name,
	   const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
}


void
save_translatable_text (GbWidgetGetArgData * data, const gchar * tag_name,
			const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
  if (data->save_translatable_strings)
    save_add_translatable_string (data, tag_value);
}


/* This is like save_translatable_text() except it splits the text into lines
   when adding to the translatable strings file. This is used for option menu
   items and combo items. */
void
save_translatable_text_in_lines (GbWidgetGetArgData * data,
				 const gchar * tag_name,
				 const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
  if (data->save_translatable_strings)
    {
      gchar *items, *pos, *items_end;

      items = pos = g_strdup (tag_value);
      items_end = &items[strlen (items)];

      while (pos < items_end)
	{
	  gchar *item_end = strchr (pos, '\n');
	  if (item_end == NULL)
	    item_end = items_end;
	  *item_end = '\0';

	  save_add_translatable_string (data, pos);

	  pos = item_end + 1;
	}

      g_free (items);
    }
}


void
save_int (GbWidgetGetArgData * data, const gchar * tag_name,
	  const gint tag_value)
{
  gchar buf[32];
  sprintf (buf, "%i", tag_value);
  save_string (data, tag_name, buf);
}


void
save_float (GbWidgetGetArgData * data, const gchar * tag_name,
	    gfloat tag_value)
{
  gchar buf[32];
  sprintf (buf, "%g", tag_value);
  save_string (data, tag_name, buf);
}


void
save_bool (GbWidgetGetArgData * data, const gchar * tag_name, gint tag_value)
{
  save_string (data, tag_name, tag_value ? "True" : "False");
}


void
save_choice (GbWidgetGetArgData * data, const gchar * tag_name,
	     const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
}


void
save_combo (GbWidgetGetArgData * data, const gchar * tag_name,
	    const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
}


/* Colors are currently saved as r,g,b where rgb are 0-255.
   But we may switch to using RGB:RRRR/GGGG/BBBB so we can use 16-bit values.*/
void
save_color (GbWidgetGetArgData * data, const gchar * tag_name,
	    GdkColor * tag_value)
{
  gchar buf[32];
  guint16 red = tag_value->red >> 8;
  guint16 green = tag_value->green >> 8;
  guint16 blue = tag_value->blue >> 8;
  sprintf (buf, "%i,%i,%i", red, green, blue);
  save_string (data, tag_name, buf);
}


void
save_bgpixmap (GbWidgetGetArgData * data, const gchar * tag_name,
	       const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
}


void
save_dialog (GbWidgetGetArgData * data, const gchar * tag_name,
	     const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
}


void
save_filename (GbWidgetGetArgData * data, const gchar * tag_name,
	       const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
}


void
save_pixmap_filename (GbWidgetGetArgData * data, const gchar * tag_name,
		      const gchar * tag_value)
{
  if (data->copying_to_clipboard)
    {
      /* When saving to the clipboard, we just save the full path.
	 This means it will still work if we paste into other projects. */
      save_string (data, tag_name, tag_value);
    }
  else
    {
      /* When saving the XML file, we save only the basename of the pixmap
	 files, since they should all be in the pixmaps directory. */
      gchar *pixmaps_dir, *filename;

      pixmaps_dir = glade_project_get_pixmaps_directory (data->project);
      g_return_if_fail (pixmaps_dir != NULL);
      g_return_if_fail (pixmaps_dir[0] != '\0');

      if (tag_value == NULL || tag_value[0] == '\0')
	{
	  filename = NULL;
	}
      else
	{
	  filename = g_basename (tag_value);
	}

      save_string (data, tag_name, filename);
    }
}


void
save_font (GbWidgetGetArgData * data, const gchar * tag_name,
	   const gchar * tag_value)
{
  save_string (data, tag_name, tag_value);
}

/* This saves a date in the RFC1123 format (an update of RFC822),
   e.g. 'Sun, 06 Nov 1994 08:49:37 GMT'. */
void
save_date (GbWidgetGetArgData * data, const gchar * tag_name,
	   time_t tag_value)
{
  time_t time;
  struct tm *t;
  gchar buffer[32];

  time = tag_value;
  t = gmtime (&time);
  sprintf (buffer, "%s, %02d %s %04d %02d:%02d:%02d GMT",
	   GladeDayNames[t->tm_wday], t->tm_mday, GladeMonthNames[t->tm_mon],
	   t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
  save_string (data, tag_name, buffer);
}


/* Adds a string to the output buffer, converting special characters to
   entities, e.g. "<" is output as "&lt;". */
static void
save_buffer_add_string (GString * buffer, const gchar * string)
{
  gchar ch;

  while ((ch = *string++))
    {
      if (ch == '<')
	g_string_append (buffer, "&lt;");
      else if (ch == '>')
	g_string_append (buffer, "&gt;");
      else if (ch == '&')
	g_string_append (buffer, "&amp;");
      else if (ch == '"')
	g_string_append (buffer, "&quot;");
      else
	g_string_append_c (buffer, ch);
    }
}


/* Outputs the contents of the buffer to the file and resets the buffer. */
static void
save_buffer_flush (GbWidgetGetArgData * data,
		   FILE *fp)
{
  gint bytes_written;

  bytes_written = fwrite (data->buffer->str, sizeof (gchar), data->buffer->len,
			  fp);
  if (bytes_written != data->buffer->len)
    {
      MSG2 ("Bytes: %i Written: %i", data->buffer->len, bytes_written);
      data->error = glade_error_new_system (_("Error writing XML file\n"));
    }

  /* Reset the output buffer. */
  g_string_truncate (data->buffer, 0);
  data->indent = 0;
}


/* Outputs tabs & spaces to indent the line according to the current
   indentation level. Tabs are used to cut down on the file size a bit. */
static void
save_indent (GbWidgetGetArgData * data)
{
  gint i, ntabs, nspaces;

  ntabs = (data->indent * 2) / 8;
  nspaces = (data->indent * 2) % 8;

  for (i = 0; i < ntabs; i++)
    g_string_append_c (data->buffer, '\t');

  for (i = 0; i < nspaces; i++)
    g_string_append_c (data->buffer, ' ');
}


/*
 * Translatable string functions.
 */

/* This adds a translatable string to the buffer, wrapping it in the N_()
   macro so xgettext can find it. */
static void
save_add_translatable_string (GbWidgetGetArgData * data,
			      const gchar * string)
{
  GString *buffer;
  gchar escape_buffer[16];
  const gchar *p;

  /* If it is an empty string don't bother outputting it. */
  if (!string || string[0] == '\0')
    return;

  buffer = data->translatable_strings;
  g_string_append (buffer, "gchar *s = N_(\"");

  /* Step through each character of the given string, adding it to our GString
     buffer, converting it so that it is valid in a literal C string. */
  for (p = string; *p; p++)
    {
      switch (*p)
	{
	case '\n':
	  g_string_append (buffer, "\\n\"\n              \"");
	  break;
	case '\r':
	  g_string_append (buffer, "\\r");
	  break;
	case '\t':
	  g_string_append (buffer, "\\t");
	  break;
	case '\\':
	  g_string_append (buffer, "\\\\");
	  break;
	case '"':
	  g_string_append (buffer, "\\\"");
	  break;
	default:
	  if (isprint (*p))
	    {
	      g_string_append_c (buffer, *p);
	    }
	  else
	    {
	      sprintf (escape_buffer, "\\%02o", (guchar) *p);
	      g_string_append (buffer, escape_buffer);
	    }
	  break;
	}
    }

  g_string_append (buffer, "\");\n");
}


/* This outputs the file containing all the translatable strings. */
static void
save_translatable_strings (GbWidgetGetArgData * data)
{
  gchar *filename;
  FILE *fp;

  filename = glade_project_get_translatable_strings_file (data->project);

  fp = fopen (filename, "w");
  if (fp == NULL)
    {
      data->error = glade_error_new_system (_("Couldn't create file:\n  %s\n"),
					    filename);
      return;
    }

  fprintf (fp,
	   _("/*\n"
	     " * Translatable strings file generated by Glade.\n"
	     " * Add this file to your project's POTFILES.in.\n"
	     " * DO NOT compile it as part of your application.\n"
	     " */\n"
	     "\n"));

  fprintf (fp, "%s", data->translatable_strings->str);

  fclose (fp);
}
