/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998-1999  Damon Chaplin
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
#include <errno.h>

#include "gladeconfig.h"
#include "glade.h"
#include "glade_clipboard.h"
#include "glade_project.h"
#include "gbwidget.h"
#include "editor.h"
#include "load.h"
#include "property.h"
#include "palette.h"
#include "tree.h"
#include "utils.h"

/* This is the global current directory, set in glade_init(). It is used in
   case we use chdir() anywhere within Glade. We used to do that in source.c
   but that's been changed. */
gchar *glade_current_directory;

/* This is the global clipboard. */
GtkWidget *glade_clipboard;

void
glade_init (void)
{
  /* Initialize debugging flags from the GLADE_DEBUG environment variable. */
  glade_debug_init ();

  glade_current_directory = g_get_current_dir ();

  gb_widgets_init ();
  editor_init ();
  tree_init ();
  glade_clipboard = glade_clipboard_new ();
}


/* Simple user interface functions. */
void
glade_show_project_window (void)
{

}


void
glade_hide_project_window (void)
{

}


void
glade_show_palette (void)
{
  palette_show (NULL, NULL);
}


void
glade_hide_palette (void)
{
  palette_hide (NULL, NULL);
}


void
glade_show_property_editor (void)
{
  property_show (NULL, NULL);
}


void
glade_hide_property_editor (void)
{
  property_hide (NULL, NULL);
}


void
glade_show_widget_tree (void)
{
  tree_show (NULL, NULL);
}


void
glade_hide_widget_tree (void)
{
  tree_hide (NULL, NULL);
}


void
glade_show_clipboard	(void)
{
  gtk_widget_show (glade_clipboard);
  gdk_window_show (GTK_WIDGET (glade_clipboard)->window);
  gdk_window_raise (GTK_WIDGET (glade_clipboard)->window);
}


void
glade_hide_clipboard	(void)
{
  glade_util_close_window (glade_clipboard);
}


void
glade_show_widget_tooltips (gboolean show)
{
  gb_widget_set_show_tooltips (show);
}


void
glade_show_grid	(gboolean show)
{
  editor_set_show_grid (show);
}


void
glade_snap_to_grid (gboolean snap)
{
  editor_set_snap_to_grid (snap);
}


#if 0
/* Changed editor_show_grid_settings_dialog and
   editor_show_grid_settings_dialog to take a widget parameter, to use for
   selecting a transient parent.
   These functions don't know about any widgets. Since they're unused,
   they're commented out. I guess it would be even better to remove them
   outright. */
void
glade_show_grid_settings (void)
{
  editor_show_grid_settings_dialog ();
}


void
glade_show_snap_settings (void)
{
  editor_show_snap_settings_dialog ();
}
#endif


gchar*
glade_get_error_message	(GladeStatusCode status)
{
  switch (status)
    {
    case GLADE_STATUS_OK:
      return _("OK");
    case GLADE_STATUS_ERROR:
      return _("Error");

    case GLADE_STATUS_SYSTEM_ERROR:
      return _("System Error");

      /* File related errors. */
    case GLADE_STATUS_FILE_OPEN_ERROR:
      return _("Error opening file");
    case GLADE_STATUS_FILE_READ_ERROR:
      return _("Error reading file");
    case GLADE_STATUS_FILE_WRITE_ERROR:
      return _("Error writing file");

    case GLADE_STATUS_INVALID_DIRECTORY:
      return _("Invalid directory");

      /* XML Parsing errors. */
    case GLADE_STATUS_INVALID_VALUE:
      return _("Invalid value");
    case GLADE_STATUS_INVALID_ENTITY:
      return _("Invalid XML entity");
    case GLADE_STATUS_START_TAG_EXPECTED:
      return _("Start tag expected");
    case GLADE_STATUS_END_TAG_EXPECTED:
      return _("End tag expected");
    case GLADE_STATUS_DATA_EXPECTED:
      return _("Character data expected");
    case GLADE_STATUS_CLASS_ID_MISSING:
      return _("Class id missing");
    case GLADE_STATUS_CLASS_UNKNOWN:
      return _("Class unknown");
    case GLADE_STATUS_INVALID_COMPONENT:
      return _("Invalid component");
    case GLADE_STATUS_EOF:
      return _("Unexpected end of file");

    default:
      return _("Unknown error code");
    }
}


/*************************************************************************
 * GladeError - an ADT to represent an error which occurred in Glade.
 *              Currently it is only used for writing source, but it may be
 *		extended for all errors, since GladeStatus doesn't really
 *		provide enough detail to show the user useful messages.
 *************************************************************************/

GladeError*
glade_error_new			(void)
{
  GladeError *error;

  error = g_new (GladeError, 1);
  error->status = GLADE_STATUS_OK;
  error->system_errno = 0;
  error->message = NULL;

  return error;
}


/* Creates a GladeError with the given Glade status code and the printf-like
   message and arguments. */
GladeError*
glade_error_new_general		(GladeStatusCode  status,
				 gchar		 *message,
				 ...)
{
  GladeError *error;
  va_list args;

  error = glade_error_new ();
  error->status = status;

  va_start (args, message);
  error->message = g_strdup_vprintf (message, args);
  va_end (args);

  return error;
}


/* Creates a GladeError using the system errno and the printf-like
   message and arguments. This must be called immediately after the error
   is detected, so that errno is still valid and can be copied into the
   GladeError. */
GladeError*
glade_error_new_system		(gchar		 *message,
				 ...)
{
  GladeError *error;
  va_list args;

  error = glade_error_new ();
  error->status = GLADE_STATUS_SYSTEM_ERROR;
  error->system_errno = errno;

  va_start (args, message);
  error->message = g_strdup_vprintf (message, args);
  va_end (args);

  return error;
}


/* Frees the GladeError and its contents. */
void
glade_error_free			(GladeError      *error)
{
  g_free (error->message);
  g_free (error);
}
