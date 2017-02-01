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

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>

#include "gladeconfig.h"

#include "glade_project.h"
#include "gbwidget.h"
#include "load.h"
#include "utils.h"

#define BUFFER_INCREMENT_SIZE	1024

#define MAX_ENTITY_LEN	16


static GladeStatusCode real_load_project_file (GladeProject *project,
					       GList **errors);
static gchar load_next_char (GbWidgetSetArgData * data);
static void load_buffer_expand_and_add_char (GbBuffControl * data,
					     gchar ch);
static void load_entity (GbWidgetSetArgData * data);
static void load_ensure_widgets_named (GtkWidget    *widget,
				       GladeProject *project);
static gint load_get_property_line_number (GbWidgetSetArgData * data,
					   const gchar * tag_name);


/* Use a few macros for efficiency. */
#define load_buffer_add(data, ch) \
  if (ch != '&') \
    { \
      load_buffer_add_char (&data->buffer, ch); \
    } \
  else \
    load_entity (data)

#define load_buffer_add_char(buf, ch) \
  if ((buf)->pos != (buf)->space) (buf)->ptr[(buf)->pos++] = ch; \
  else load_buffer_expand_and_add_char (buf, ch);


/* We need this to make sure that numbers are read in a portable syntax,
   instead of using the current locale. This code is from glibc info docs.
   We also set the timezone temporarily to GMT so that we can read in dates
   easily. */
GladeStatusCode
load_project_file (GladeProject *project,
		   GList       **errors)
{
  gchar *saved_locale, *saved_timezone;
  GladeStatusCode status;

  *errors = NULL;

  /* Set the locale to "C". */
  saved_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
  setlocale (LC_NUMERIC, "C");

  /* Set the timezone to "UTC". */
  saved_timezone = glade_util_set_timezone ("UTC");

  /* Now save the project XML file. */
  status = real_load_project_file (project, errors);

  /* Reset the timezone. */
  glade_util_reset_timezone (saved_timezone);

  /* Reset the locale. */
  setlocale (LC_NUMERIC, saved_locale);
  g_free (saved_locale);

  return status;
}


void
load_init_before_read (gint		   space,
		       GbWidgetSetArgData *data)
{
  data->status = GLADE_STATUS_OK;
  data->line_number = 1;
  
  /* Note: we start with a buffer pos of 1 since 0 is needed to represent an
     empty value when retrieving indices from the properties hash. */
  gb_init_buffer_struct(1, space, &data->buffer);

  data->token_type = GB_TOKEN_START_TAG;
  gb_init_load_properties (&data->properties);
  gb_init_load_properties (&data->child_properties);

  data->signals = NULL;
  data->accelerators = NULL;
#ifdef GLADE_STYLE_SUPPORT
  data->gbstyle = NULL;
#endif
  data->error_messages = NULL;
}

/* FIXME: handle XML comments - could be a problem if in the middle of CDATA */

static GladeStatusCode
real_load_project_file (GladeProject *project,
			GList       **errors)
{
  GbWidgetSetArgData data;
  gboolean toplevel = TRUE;

  data.project = project;
  data.filename = project->xml_filename;
  data.xml_buffer = NULL;

  data.fp = fopen (data.filename, "r");
  if (data.fp == NULL)
    {
      *errors = g_list_append (*errors, g_strdup (_("Couldn't open file.\n")));
      return GLADE_STATUS_FILE_OPEN_ERROR;
    }

  load_init_before_read (BUFFER_INCREMENT_SIZE, &data);

  load_token_skip_whitespace (&data);
  while (data.status == GLADE_STATUS_OK)
    {
      toplevel = FALSE;

      if (data.token_type == GB_TOKEN_END_TAG)
	{
	  if (!strcmp (data.buffer.ptr + data.token, "GTK-Interface"))
	    {
	      MSG1 ("End of Document body element: %s",
		    data.buffer.ptr + data.token);
	      toplevel = TRUE;
	      break;
	    }
	}

      if (data.token_type != GB_TOKEN_START_TAG)
	{
	  data.status = GLADE_STATUS_START_TAG_EXPECTED;
	  break;
	}

      /* This will load an entire component, recursively. */
      if (!strcmp (data.buffer.ptr + data.token, "widget"))
	{
	  gb_widget_load (NULL, &data, NULL);
	  if (data.status != GLADE_STATUS_OK)
	    break;
	}
      else if (!strcmp (data.buffer.ptr + data.token, "project"))
	{
	  glade_project_load_options (data.project, &data);
	  if (data.status != GLADE_STATUS_OK)
	    break;
	}
      else if (!strcmp (data.buffer.ptr + data.token, "style"))
	{
	  gb_widget_load_style (&data);
	  if (data.status != GLADE_STATUS_OK)
	    break;
	}
      else if (!strcmp (data.buffer.ptr + data.token, "GTK-Interface"))
	{
	  MSG1 ("Document body element: %s", data.buffer.ptr + data.token);
	}
      else if (*(data.buffer.ptr + data.token) == '?')
	{
	  MSG1 ("Processing instruction: %s", data.buffer.ptr + data.token);
	}
      else
	{
	  data.status = GLADE_STATUS_INVALID_ENTITY;
	  break;
	}

      load_token_skip_whitespace (&data);
      toplevel = TRUE;
    }

  g_free (data.buffer.ptr);
  gb_free_load_properties (&data.properties);
  gb_free_load_properties (&data.child_properties);
  fclose (data.fp);

  /* Now we need to ensure that all widgets have names. In particular the
     titles of CLists & CTrees, since sometimes it is necessary to create
     these will loading. */
  glade_project_foreach_component (data.project,
				   (GtkCallback) load_ensure_widgets_named,
				   data.project);
  
  /* If we got EOF at the top level then that is OK. */
  if (data.status == GLADE_STATUS_EOF && toplevel)
    data.status = GLADE_STATUS_OK;

  if (data.status != GLADE_STATUS_OK)
    load_add_error_message (&data, data.line_number,
			    glade_get_error_message (data.status), NULL);

  *errors = data.error_messages;
  return data.status;
}


/* This skips any initial whitespace, then reads a start tag, data, and an end
   tag. Note that even though this function may return the START_TAG_EXPECTED
   error code, that often just means we've reached the end of a group of
   properties and is not actually an error. */
void
load_element (GbWidgetSetArgData * data, gint * element, gint * cdata)
{
  load_token_skip_whitespace (data);
  if (data->status != GLADE_STATUS_OK)
    {
      MSG ("Load error");
      return;
    }
  if (data->token_type != GB_TOKEN_START_TAG)
    {
      data->status = GLADE_STATUS_START_TAG_EXPECTED;
      MSG ("Load error");
      return;
    }
  *element = data->token;

  load_token (data);
  if (data->status != GLADE_STATUS_OK)
    {
      MSG ("Load error");
      return;
    }
  if (data->token_type == GB_TOKEN_DATA)
    {
      *cdata = data->token;

      load_token (data);
      if (data->status != GLADE_STATUS_OK)
	{
	  MSG ("Load error");
	  return;
	}
    }
  else
    {
      /* Empty tag - use special buffer position, 0. */
      *cdata = 0;
    }

  if (data->token_type != GB_TOKEN_END_TAG
      || strcmp (data->buffer.ptr + *element, data->buffer.ptr + data->token))
    {
      data->status = GLADE_STATUS_END_TAG_EXPECTED;
      MSG2 ("start tag:%s end tag:%s", data->buffer.ptr + *element,
	    data->buffer.ptr + data->token);
      MSG ("Load error");
      return;
    }
  MSG2 ("Loaded element: %s, data: %s", data->buffer.ptr + *element,
	data->buffer.ptr + *cdata);
}


/* Returns one token of XML - a GB_TOKEN_START_TAG, GB_TOKEN_END_TAG or
   GB_TOKEN_DATA. */
void
load_token (GbWidgetSetArgData * data)
{
  gchar ch;

  data->token = data->buffer.pos;

  /* If the last token was data, it finished with a '<'. */
  if (data->token_type == GB_TOKEN_DATA)
    ch = '<';
  else
    {
      ch = load_next_char (data);
      if (data->status != GLADE_STATUS_OK)
	return;
    }

  if (ch == '<')
    {
      ch = load_next_char (data);
      if (data->status != GLADE_STATUS_OK)
	return;
      if (ch != '/')
	{
	  data->token_type = GB_TOKEN_START_TAG;
	  load_buffer_add (data, ch);
	  if (data->status != GLADE_STATUS_OK)
	    return;
	}
      else
	{
	  data->token_type = GB_TOKEN_END_TAG;
	}

      for (;;)
	{
	  ch = load_next_char (data);
	  if (data->status != GLADE_STATUS_OK)
	    return;
	  if (ch == '>')
	    {
	      load_buffer_add_char (&data->buffer, '\0');
	      return;
	    }
	  else
	    {
	      load_buffer_add (data, ch);
	      if (data->status != GLADE_STATUS_OK)
		return;
	    }
	}
    }
  else
    {
      data->token_type = GB_TOKEN_DATA;
      load_buffer_add (data, ch);
      if (data->status != GLADE_STATUS_OK)
	return;

      for (;;)
	{
	  ch = load_next_char (data);
	  if (data->status != GLADE_STATUS_OK)
	    return;
	  if (ch == '<')
	    {
	      load_buffer_add_char (&data->buffer, '\0');
	      return;
	    }
	  else
	    {
	      load_buffer_add (data, ch);
	      if (data->status != GLADE_STATUS_OK)
		return;
	    }
	}
    }
}


/* This skips any whitespace token and returns the next token. */
void
load_token_skip_whitespace (GbWidgetSetArgData * data)
{
  gchar *pos;

  load_token (data);
  if (data->status != GLADE_STATUS_OK || data->token_type != GB_TOKEN_DATA)
    return;

  /* Return if the CDATA is not all whitespace. */
  pos = data->buffer.ptr + data->token;
  while (*pos)
    {
      if (*pos != ' ' && *pos != '\n' && *pos != '\r' && *pos != '\t')
	return;
      pos++;
    }

  load_token (data);
}


/* This returns the next character from the file, and sets the status if
   eof or an error occurred. It also increments the line number if a '\n' is
   read. */
static gchar
load_next_char (GbWidgetSetArgData * data)
{
  gchar ch = ' ';
  gint next_char;

  /* If we are pasting form the clipboard, we get the characters from the
     xml buffer. */
  if (data->xml_buffer)
    {
      if (data->xml_buffer_pos == data->xml_buffer_len)
	data->status = GLADE_STATUS_EOF;
      else
	ch = data->xml_buffer[data->xml_buffer_pos++];
    }
  else
    {
      next_char = getc (data->fp);
      if (next_char != EOF)
	ch = (gchar) next_char;
      else
	{
	  ch = ' ';
	  if (feof (data->fp))
	    data->status = GLADE_STATUS_EOF;
	  else
	    data->status = GLADE_STATUS_FILE_READ_ERROR;
	}
    }

  if (ch == '\n')
    {
      data->line_number++;
      MSG1 ("Line: %i", data->line_number);
    }

  return ch;
}


static gboolean
read_entity(GbWidgetSetArgData * data, gchar *entity)
{
  gint i;
  gchar ch;

  for (i = 0; i < MAX_ENTITY_LEN - 2; i++)
  {
    ch = load_next_char (data);
    if (data->status != GLADE_STATUS_OK)
    {
      entity[i] = '\0';
      return FALSE;
    }
    
    entity[i] = ch;
    if (ch == ';')
    {
      entity[i + 1] = '\0';
      return TRUE;
    }    
  }  
  entity[i] = '\0';
  return FALSE;
}

static void
load_entity (GbWidgetSetArgData * data)
{
  gchar entity[MAX_ENTITY_LEN];
  gboolean is_entity;
  gchar ch;

  is_entity = read_entity (data, entity);
  if (!is_entity)
    {
      data->status = GLADE_STATUS_INVALID_ENTITY;
      return;
    }

  if (!g_strcasecmp (entity, "lt;"))
    ch = '<';
  else if (!g_strcasecmp (entity, "gt;"))
    ch = '>';
  else if (!g_strcasecmp (entity, "amp;"))
    ch = '&';
  else if (!g_strcasecmp (entity, "quot;"))
    ch = '"';
  else
    {
      data->status = GLADE_STATUS_INVALID_ENTITY;
      return;
    }

  load_buffer_add_char (&data->buffer, ch);
}


/* This adds the char to the buffer, expanding the buffer if necessary. */
static void
load_buffer_expand_and_add_char (GbBuffControl * data, gchar ch)
{
  data->space += BUFFER_INCREMENT_SIZE;
  data->ptr = g_realloc (data->ptr, data->space);
  data->ptr[data->pos++] = ch;
}


/* This releases any data in the buffer, and is meant to be called after a
   widget has read all of it's properties from the buffer. */
void
load_buffer_release (GbWidgetSetArgData * data)
{
  /* Note: we start with a buffer pos of 1 since 0 is needed to represent an
     empty value when retrieving indices from the properties hash. */
  data->buffer.pos = 1;
}



gchar *
load_get_value (GbWidgetSetArgData * data,
		const gchar * property_name)
{
  GbLoadedProperty *properties;
  gint nproperties;
  const gchar *tag_name, *prop;
  gint i;

  tag_name = property_name;
  while (*tag_name && (*tag_name != ':' || *(tag_name + 1) != ':'))
     tag_name++;
  if (*tag_name)
    tag_name += 2;
  else
    tag_name = property_name;

  if (data->loading_type == GB_STANDARD_PROPERTIES)
    {
      properties = data->properties.ptr;
      nproperties = data->properties.n;
    }
  else
    {
      properties = data->child_properties.ptr;
      nproperties = data->child_properties.n;
    }

  for (i = 0; i < nproperties; i++)
    {
      prop = data->buffer.ptr + properties[i].tag_index;
      if (prop[0] == tag_name[0] && !strcmp (prop, tag_name))
	{
	  data->apply = TRUE;
	  return data->buffer.ptr + properties[i].cdata_index;
	}
    }

  data->apply = FALSE;
  return NULL;
}


void
load_add_error_message (GbWidgetSetArgData * data,
			gint line_number,
			const gchar *message,
			const gchar *value)
{
  gchar *error_message;

  error_message = g_strdup_printf (_("Line %i - %s %s %s\n"),
				   line_number, message,
				   value ? ":" : "", value ? value : "");

  data->error_messages = g_list_append (data->error_messages, error_message);
}


void
load_add_error_message_with_tag (GbWidgetSetArgData * data,
				 GladeErrorLineType line,
				 const gchar * message,
				 const gchar * tag_name,
				 const gchar * value)
{
  gchar *error_message, *real_tag_name;
  gint line_number;

  real_tag_name = glade_util_find_start_of_tag_name (tag_name);

  if (line == GLADE_LINE_PROPERTY)
    line_number = load_get_property_line_number (data, real_tag_name);
  else
    line_number = data->line_number;

  error_message = g_strdup_printf (_("Line %i - %s:\n  %s = %s\n"),
				   line_number, message,
				   real_tag_name, value ? value : "");

  data->error_messages = g_list_append (data->error_messages, error_message);
}


static gint
load_get_property_line_number (GbWidgetSetArgData * data,
			       const gchar * tag_name)
{
  GbLoadedProperty *properties;
  gint nproperties, i;
  gchar *prop;

  properties = data->properties.ptr;
  nproperties = data->properties.n;
  for (i = 0; i < nproperties; i++)
    {
      prop = data->buffer.ptr + properties[i].tag_index;
      if (!strcmp (prop, tag_name))
	return properties[i].line_number;
    }

  /* Now try the child properties. */
  properties = data->child_properties.ptr;
  nproperties = data->child_properties.n;
  for (i = 0; i < nproperties; i++)
    {
      prop = data->buffer.ptr + properties[i].tag_index;
      if (!strcmp (prop, tag_name))
	return properties[i].line_number;
    }
  return data->line_number;
}


/* FIXME: Should the functions returning strings return NULL or "" ? */

gchar *
load_string (GbWidgetSetArgData * data,
	     const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? value : "";
}


gchar *
load_text (GbWidgetSetArgData * data,
	   const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? value : "";
}


gint
load_int (GbWidgetSetArgData * data,
	  const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? atoi (value) : 0;
}


gfloat
load_float (GbWidgetSetArgData * data,
	    const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? atof (value) : 0;
}


gboolean
load_bool (GbWidgetSetArgData * data,
	   const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  gboolean result = load_parse_bool (data, value);
  if (data->status == GLADE_STATUS_INVALID_VALUE)
    {
      load_add_error_message_with_tag (data, GLADE_LINE_PROPERTY,
				       _("Invalid boolean value"),
				       property_name, value);
      data->status = GLADE_STATUS_OK;
      result = FALSE;
    }
  return result;
}


gboolean
load_parse_bool (GbWidgetSetArgData * data,
		 const gchar * value)
{
  if (value != NULL)
    {
      if (!g_strcasecmp (value, "true") || !strcmp (value, "1"))
	return TRUE;
      else if (!g_strcasecmp (value, "false") || !strcmp (value, "0"))
	return FALSE;
      else
	{
	  data->status = GLADE_STATUS_INVALID_VALUE;
	  MSG1 ("===Invalid boolean property: %s", value);
	}
    }
  return FALSE;
}


gchar *
load_choice (GbWidgetSetArgData * data,
	     const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? value : "";
}


gchar *
load_combo (GbWidgetSetArgData * data,
	    const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? value : "";
}


GdkColor *
load_color (GbWidgetSetArgData * data,
	    const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  GdkColor *result = load_parse_color (data, value);
  if (data->status == GLADE_STATUS_INVALID_VALUE)
    {
      load_add_error_message_with_tag (data, GLADE_LINE_PROPERTY,
				       _("Invalid color"),
				       property_name, value);
      data->status = GLADE_STATUS_OK;
      /* If an error occurs return white. */
      result->red = 0xFFFF;
      result->green = 0xFFFF;
      result->blue = 0xFFFF;
    }
  return result;
}


/* Colors are currently saved as r,g,b where rgb are 0-255.
   But we may switch to using RGB:RRRR/GGGG/BBBB so we can use 16-bit values.*/
GdkColor *
load_parse_color (GbWidgetSetArgData * data,
		  const gchar * value)
{
  static GdkColor color;
  gint matched, red, green, blue;

  if (value == NULL)
    return NULL;
  matched = sscanf (value, "%i,%i,%i", &red, &green, &blue);
  if (matched != 3)
    {
      data->status = GLADE_STATUS_INVALID_VALUE;
      return NULL;
    }

  /* This effectively multiplies my 257 so we cover the range properly.
     e.g. 00 -> 0000, 01 -> 0101, ff -> ffff. */
  color.red = (red << 8) + red;
  color.green = (green << 8) + green;
  color.blue = (blue << 8) + blue;
  return &color;
}


GdkPixmap *
load_bgpixmap (GbWidgetSetArgData * data,
	       const gchar * property_name,
	       gchar ** filename)
{
  /*GdkPixmap *gdkpixmap;*/
  gchar *value = load_get_value (data, property_name);
  *filename = value;
  if (value)
    {
      /* FIXME: What do I do here? We have no widget. Could use the parent,
         or load the pixmap in a realize callback. */
      /*
         gdkpixmap = gdk_pixmap_create_from_xpm (data->holding_widget->window, NULL,
         &data->holding_widget->style->bg[GTK_STATE_NORMAL],
         value);
         if (!gdkpixmap)
         load_add_error_message_with_tag (data, GLADE_LINE_PROPERTY,
					  "Couldn't load pixmap",
					  property_name, value);

         return gdkpixmap;
       */
    }

  return NULL;
}


gpointer
load_dialog (GbWidgetSetArgData * data,
	     const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? value : "";
}


gchar *
load_filename (GbWidgetSetArgData * data,
	       const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? value : "";
}


/* If we are loading the XML file, we convert any relative filenames to
   absolute ones, based on the project directory and/or pixmaps directory
   options. The returned filename should be freed when no longer needed. */
gchar *
load_pixmap_filename (GbWidgetSetArgData * data,
		      const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  gchar *pixmaps_dir;

  if (value == NULL)
    return NULL;

  if (data->xml_buffer == NULL)
    {
      pixmaps_dir = glade_project_get_pixmaps_directory (data->project);
      g_return_val_if_fail (pixmaps_dir != NULL, NULL);
      g_return_val_if_fail (pixmaps_dir[0] != '\0', NULL);
      return glade_util_make_absolute_path (pixmaps_dir, value);
    }
  else
    return g_strdup (value);
}


GdkFont *
load_font (GbWidgetSetArgData * data,
	   const gchar * property_name,
	   gchar ** xlfd_fontname)
{
  GdkFont *font;
  gchar *value = load_get_value (data, property_name);
  *xlfd_fontname = value;
  if (value)
    {
      font = gdk_font_load (value);
      if (font == NULL)
	load_add_error_message_with_tag (data, GLADE_LINE_PROPERTY,
					 _("Couldn't load font"),
					 property_name, value);
      return font;
    }
  return NULL;
}


time_t
load_date (GbWidgetSetArgData * data,
	   const gchar * property_name)
{
  gchar *value = load_get_value (data, property_name);
  return value ? load_parse_date (data, value) : 0;
}


/* This parses a date in the RFC1123 format (an update of RFC822),
   e.g. 'Sun, 06 Nov 1994 08:49:37 GMT'. */
time_t
load_parse_date (GbWidgetSetArgData * data,
		 const gchar * value)
{
  struct tm t;
  gchar day[4], month[4];
  gint matched, i;
  time_t time;

  /* Terminate the strings to be careful. */
  day[0] = '\0';
  month[0] = '\0';

  MSG1 ("Trying to match date: %s", value);
  matched = sscanf (value, "%3s, %2d %3s %4d %2d:%2d:%2d GMT",
		    &day[0], &t.tm_mday, &month[0], &t.tm_year,
		    &t.tm_hour, &t.tm_min, &t.tm_sec);
  if (matched != 7)
    {
      MSG1 ("ERROR parsing date, matched: %i", matched);
      data->status = GLADE_STATUS_INVALID_VALUE;
      return 0;
    }

  /* The tm_year field starts from 1900 so we have to subtract that. */
  t.tm_year -= 1900;

  /* Find the month. */
  t.tm_mon = -1;
  for (i = 0; i < 12; i++)
    {
      if (!strcmp (GladeMonthNames[i], month))
	{
	  t.tm_mon = i;
	  break;
	}
    }

  /* Find the day. */
  t.tm_wday = -1;
  for (i = 0; i < 7; i++)
    {
      if (!strcmp (GladeDayNames[i], day))
	{
	  t.tm_wday = i;
	  break;
	}
    }

  if (t.tm_mon == -1 || t.tm_wday == -1)
    {
      MSG ("ERROR parsing date");
      data->status = GLADE_STATUS_INVALID_VALUE;
      return 0;
    }

  t.tm_isdst = -1;
  /* Note that we don't need to set t.tm_yday (or t.tm_wday really).
     They are recomputed by mktime.
     Note also that mktime works since we have already set the timezone to GMT
     in load_project_file(). */
  time = mktime (&t);
  if (time == -1)
    {
      MSG ("ERROR parsing date");
      data->status = GLADE_STATUS_INVALID_VALUE;
      return 0;
    }
  return time;
}


static void
load_ensure_widgets_named (GtkWidget    *widget,
			   GladeProject *project)
{
  glade_project_ensure_widgets_named (project, widget);
}
