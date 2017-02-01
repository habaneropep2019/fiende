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

#include "config.h"

#include <string.h>
#include <locale.h>
#include <stdlib.h>

#ifdef USE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif

#include "gladeconfig.h"

#include "editor.h"
#include "glade_clipboard.h"
#include "gbwidget.h"
#include "load.h"
#include "tree.h"
#include "utils.h"

typedef struct _GladeClipboardItem  GladeClipboardItem;

struct _GladeClipboardItem
{
  GladeProject *project;
  GtkType type;
  gchar *xml_data;
  gboolean names_unique;
};


static GtkWindowClass *parent_class = NULL;

static void glade_clipboard_class_init (GladeClipboardClass * klass);
static void glade_clipboard_init (GladeClipboard * clipboard);

static void glade_clipboard_cut_or_copy (GladeClipboard *clipboard,
					 GladeProject   *project,
					 GtkWidget      *widget,
					 gboolean	 cut);
static void glade_clipboard_add	(GladeClipboard *clipboard,
				 GladeProject   *project,
				 gboolean	 names_unique,
				 GtkWidget	*widget,
				 gchar		*xml_data);
static GladeClipboardItem* glade_clipboard_get_current_item (GladeClipboard *clipboard);

static void glade_clipboard_on_project_destroy (GladeProject *project,
						GladeClipboardItem *item);


#define GLADE_PASTE_BUFFER_INCREMENT	1024


guint
glade_clipboard_get_type (void)
{
  static guint glade_clipboard_type = 0;

  if (!glade_clipboard_type)
    {
      GtkTypeInfo glade_clipboard_info =
      {
	"GladeClipboard",
	sizeof (GladeClipboard),
	sizeof (GladeClipboardClass),
	(GtkClassInitFunc) glade_clipboard_class_init,
	(GtkObjectInitFunc) glade_clipboard_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
	(GtkClassInitFunc) NULL,
      };

      glade_clipboard_type = gtk_type_unique (gtk_window_get_type (),
					      &glade_clipboard_info);
    }
  return glade_clipboard_type;
}


static void
glade_clipboard_class_init (GladeClipboardClass * klass)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass *) klass;

  parent_class = gtk_type_class (gtk_window_get_type ());
}


static void
glade_clipboard_init (GladeClipboard * clipboard)
{
  GtkWidget *vbox, *scrolled_win;

  gtk_window_set_title (GTK_WINDOW (clipboard), _ ("Clipboard"));
  gtk_window_set_policy (GTK_WINDOW (clipboard), FALSE, TRUE, FALSE);
  gtk_window_set_wmclass (GTK_WINDOW (clipboard), "clipboard", "Glade");
  gtk_window_set_default_size (GTK_WINDOW (clipboard), 150, 200);

  vbox = gtk_vbox_new (FALSE, 4);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (clipboard), vbox);

  clipboard->clist = gtk_clist_new (1);
  gtk_clist_set_row_height (GTK_CLIST (clipboard->clist), 23);
  gtk_widget_set_usize (clipboard->clist, 100, 100);
  gtk_clist_set_column_width (GTK_CLIST (clipboard->clist), 0, 100);
  gtk_clist_set_selection_mode (GTK_CLIST (clipboard->clist),
				GTK_SELECTION_BROWSE);
  gtk_widget_show (clipboard->clist);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), clipboard->clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_win);

  gtk_signal_connect (GTK_OBJECT (clipboard), "delete_event",
		      GTK_SIGNAL_FUNC (glade_util_close_window_on_delete),
		      NULL);
}


GtkWidget *
glade_clipboard_new (void)
{
  return GTK_WIDGET (gtk_type_new (glade_clipboard_get_type ()));
}


void
glade_clipboard_cut (GladeClipboard *clipboard,
		     GladeProject   *project,
		     GtkWidget      *widget)
{
  glade_clipboard_cut_or_copy (clipboard, project, widget, TRUE);
}


void
glade_clipboard_copy (GladeClipboard *clipboard,
		      GladeProject   *project,
		      GtkWidget      *widget)
{
  glade_clipboard_cut_or_copy (clipboard, project, widget, FALSE);
}


static void
glade_clipboard_cut_or_copy (GladeClipboard *clipboard,
			     GladeProject   *project,
			     GtkWidget      *widget,
			     gboolean	     cut)
{
  GbWidgetGetArgData data;
  gchar *old_locale, *saved_locale;
  GList *selection;

  if (widget == NULL)
    {
      selection = editor_get_selection ();
      if (selection)
	widget = GTK_WIDGET (selection->data);
    }

  if (widget == NULL || GB_IS_PLACEHOLDER (widget))
    return;

  data.project = project;
  data.action = GB_SAVING;
  data.copying_to_clipboard = TRUE;
  data.error = NULL;

  /* Initialize the output buffer. */
  data.buffer = g_string_sized_new (1024);
  data.indent = 0;

  /* We don't need the translatable strings. */
  data.save_translatable_strings = FALSE;
  data.translatable_strings = NULL;

  old_locale = setlocale (LC_NUMERIC, NULL);
  saved_locale = g_strdup (old_locale);
  setlocale (LC_NUMERIC, "C");

  gb_widget_save (widget, &data);

  setlocale (LC_NUMERIC, saved_locale);
  g_free (saved_locale);

  if (data.error == NULL)
    {
      glade_clipboard_add (GLADE_CLIPBOARD (clipboard), project, cut,
			   widget, data.buffer->str);
      if (cut)
	editor_delete_widget (widget);
    }
  else
    /* This shouldn't happen. */
    g_warning ("Error saving widget to clipboard");

  g_string_free (data.buffer, TRUE);
}


void
glade_clipboard_paste (GladeClipboard *clipboard,
		       GladeProject   *project,
		       GtkWidget      *widget)
{
  GladeClipboardItem *item;
  GbWidgetSetArgData data;
  GtkWidget *parent, *new_widget = NULL;
  gchar *saved_locale, *saved_timezone, *child_name;
  GList *selection;

  item = glade_clipboard_get_current_item (clipboard);
  if (item == NULL)
    return;

  if (widget == NULL)
    {
      selection = editor_get_selection ();
      if (selection)
	widget = GTK_WIDGET (selection->data);
    }

  /* We can only paste toplevel components, i.e. windows and menus, into the
     project, and not into other widgets. */
  if (widget == NULL)
    {
      if (!gtk_type_is_a (item->type, gtk_window_get_type ())
	  && !gtk_type_is_a (item->type, gtk_menu_get_type ()))
	{
	  glade_util_show_message_box (_("You need to select a widget to paste into"), widget);
	  return;
	}
    }
  else
    {
      if (gtk_type_is_a (item->type, gtk_window_get_type ())
	  || gtk_type_is_a (item->type, gtk_menu_get_type ()))
	widget = NULL;
    }

  if (widget)
    {
      /* Don't allow pasting into any windows/dialogs. */
      if (GTK_IS_WINDOW (widget)
	  || GTK_IS_DIALOG (widget)
	  || GTK_IS_COLOR_SELECTION_DIALOG (widget)
	  || GTK_IS_INPUT_DIALOG (widget)
	  || GTK_IS_FONT_SELECTION_DIALOG (widget)
	  || GTK_IS_FILE_SELECTION (widget)
#ifdef USE_GNOME
	  || GNOME_IS_APP (widget)
	  || GNOME_IS_DIALOG (widget)
#endif
	  )
	{
	  glade_util_show_message_box (_("You can't paste into windows or dialogs."), widget);
	  return;
	}

      /* SPECIAL CODE: Don't allow pasting into dialog widgets. */
      child_name = gb_widget_get_child_name (widget);
      if (child_name)
	{
	  if (!strcmp (child_name, "FileSel:ok_button")
	      || !strcmp (child_name, "FileSel:cancel_button")
	      || !strcmp (child_name, "ColorSel:ok_button")
	      || !strcmp (child_name, "ColorSel:cancel_button")
	      || !strcmp (child_name, "ColorSel:help_button")
	      || !strcmp (child_name, "FontSel:ok_button")
	      || !strcmp (child_name, "FontSel:cancel_button")
	      || !strcmp (child_name, "FontSel:apply_button")
	      || !strcmp (child_name, "InputDialog:save_button")
	      || !strcmp (child_name, "InputDialog:close_button")

	      || !strcmp (child_name, "Dialog:vbox")
	      || !strcmp (child_name, "Dialog:action_area")
	      || !strcmp (child_name, "GnomeDialog:vbox")
	      || !strcmp (child_name, "GnomeDialog:action_area")
	      )
	    {
	      glade_util_show_message_box (_("You can't paste into the selected widget, since\nit is created automatically by its parent."), widget);
	      return;
	    }
	}

      /* Only allow menuitems to be pasted into menus or menubars. */
      if (GTK_IS_MENU_SHELL (widget)
	  && !gtk_type_is_a (item->type, gtk_menu_item_get_type ()))
	{
	  glade_util_show_message_box (_("Only menu items can be pasted into a menu or menu bar."), widget);
	  return;
	}

      /* Only allow menuitems to replace menuitems. */
      if (GTK_IS_MENU_ITEM (widget)
	  && !gtk_type_is_a (item->type, gtk_menu_item_get_type ()))
	{
	  glade_util_show_message_box (_("Only menu items can be pasted into a menu or menu bar."), widget);
	  return;
	}

      /* Only buttons can be pasted into a GnomeDialog action area. */
      if (widget->parent)
	{
	  child_name = gb_widget_get_child_name (widget->parent);
	  if (child_name && !strcmp (child_name, "GnomeDialog:action_area")
	      && !gtk_type_is_a (item->type, gtk_button_get_type ()))
	    {
	      glade_util_show_message_box (_("Only buttons can be pasted into a GnomeDialog action area."), widget);
	      return;
	    }
	}

#ifdef USE_GNOME
      if (GNOME_IS_DOCK (widget))
	{
	  if (!gtk_type_is_a (item->type, gnome_dock_item_get_type ()))
	    {
	      glade_util_show_message_box (_("Only GnomeDockItem widgets can be pasted into a GnomeDock."), widget);
	      return;
	    }
	}

      if (GNOME_IS_DOCK_ITEM (widget))
	{
	  if (!gtk_type_is_a (item->type, gnome_dock_item_get_type ()))
	    {
	      glade_util_show_message_box (_("Only GnomeDockItem widgets can be pasted over a GnomeDockItem."), widget);
	      return;
	    }
	  glade_util_show_message_box (_("Sorry - pasting over a GnomeDockItem is not implemented yet."), widget);
	  return;
	}

      if (gtk_type_is_a (item->type, gnome_dock_item_get_type ()))
	{
	  if (!GNOME_IS_DOCK (widget) && !GNOME_IS_DOCK_ITEM (widget))
	    {
	      glade_util_show_message_box (_("GnomeDockItem widgets can only be pasted into a GnomeDock."), widget);
	      return;
	    }
	}
#endif
    }

  data.project = project;
  data.fp = NULL;
  data.filename = NULL;
  data.xml_buffer = item->xml_data;
  data.xml_buffer_pos = 0;
  data.xml_buffer_len = strlen (data.xml_buffer);

  /* We normally create new names for the widgets pasted, discarding the names
     in the XML data. However, if the widgets were cut to the clipboard, and
     we are pasting into the same project for the first time, then we use
     the original names. */
  data.discard_names = TRUE;
  if (project == item->project)
    {
      if (item->names_unique)
	{
	  data.discard_names = FALSE;
	  item->names_unique = FALSE;
	}
    }

  /* We always try to replace the selected widget, unless it is a GtkFixed,
     GtkLayout, or GtkPacker, in which case we add the pasted widget as a
     child. If we didn't do this, then it would be quite difficult to paste
     children into these widgets. I did consider adding the widget as a child
     when pasting into other containers like boxes and tables, but then we
     have a problem with composite widgets, where we may accidentally paste
     inside the composite (e.g. inside its toplevel vbox).
     For Gnome, we also allow pasting GnomeDockItems into GnomeDocks. */
  parent = NULL;
  data.replacing_widget = NULL;
  if (widget)
    {
      if (GTK_IS_FIXED (widget)
	  || GTK_IS_LAYOUT (widget)
	  || GTK_IS_PACKER (widget)
#ifdef USE_GNOME
	  || GNOME_IS_DOCK (widget)
#endif
	  )
	{
	  parent = widget;
	}
      else
	{
	  parent = widget->parent;
	  data.replacing_widget = widget;
	}
    }

  load_init_before_read (GLADE_PASTE_BUFFER_INCREMENT, &data);

  tree_freeze ();

  /* Set the locale to "C". */
  saved_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
  setlocale (LC_NUMERIC, "C");

  /* Set the timezone to "UTC". */
  saved_timezone = glade_util_set_timezone ("UTC");

  /* Now parse the clipboard data. */
  load_token_skip_whitespace (&data);
  if (data.status == GLADE_STATUS_OK)
    {
      if (!strcmp (data.buffer.ptr + data.token, "widget"))
	new_widget = gb_widget_load (NULL, &data, parent);
      else
	data.status = GLADE_STATUS_INVALID_ENTITY;
    }

  /* Reset the timezone. */
  glade_util_reset_timezone (saved_timezone);

  /* Reset the locale. */
  setlocale (LC_NUMERIC, saved_locale);
  g_free (saved_locale);

  tree_thaw ();

  g_free (data.buffer.ptr);
  gb_free_load_properties (&data.properties);
  gb_free_load_properties (&data.child_properties);

  if (data.status != GLADE_STATUS_OK)
    {
      /* I don't think this should happen, except due to bugs. */
      g_warning ("Error pasting from clipboard");
      return;
    }

  /* If a window was pasted, show it. */
  if (GTK_IS_WINDOW (new_widget))
    glade_project_show_component (project, new_widget);
}


/* This adds the XML for a widget (and any descendants) to the clipboard.
   If the widget has been cut and is later pasted into the same project,
   then the same widget names can be used. Otherwise new widget names have
   to be created. The widget parameter is used to get the widget's name and
   class and to look up its pixmap to display in the clipboard.
   The xml_data parameter is copied. */
static void
glade_clipboard_add	(GladeClipboard *clipboard,
			 GladeProject   *project,
			 gboolean	 names_unique,
			 GtkWidget	*widget,
			 gchar		*xml_data)
{
  GladeClipboardItem *item;
  GbWidget *gbwidget;
  gchar *name;

  name = gtk_widget_get_name (widget);
  gbwidget = gb_widget_lookup (widget);
  g_return_if_fail (gbwidget != NULL);

  item = g_new (GladeClipboardItem, 1);
  item->project = project;
  item->type = GTK_OBJECT_TYPE (widget);
  item->xml_data = g_strdup (xml_data);
  item->names_unique = names_unique;

  /* Connect to the project's destroy signal to set the pointer to NULL, so
     we never have invalid pointers. */
  gtk_signal_connect (GTK_OBJECT (project), "destroy",
		      glade_clipboard_on_project_destroy,
		      item);

  gtk_clist_insert (GTK_CLIST (clipboard->clist), 0, &name);
  gtk_clist_set_row_data (GTK_CLIST (clipboard->clist), 0, item);

  gtk_clist_set_pixtext (GTK_CLIST (clipboard->clist), 0, 0, name, 3,
			 gbwidget->gdkpixmap, gbwidget->mask);

  gtk_clist_select_row (GTK_CLIST (clipboard->clist), 0, 0);
}


/* This returns the currently-selected GladeClipboardItem, or NULL if no item
   is currently selected (i.e. the clipboard is empty). */
static GladeClipboardItem*
glade_clipboard_get_current_item	(GladeClipboard *clipboard)
{
  GList *selection;

  selection = GTK_CLIST (clipboard->clist)->selection;
  if (selection == NULL)
    return NULL;

  return (GladeClipboardItem*) gtk_clist_get_row_data (GTK_CLIST (clipboard->clist), GPOINTER_TO_INT (selection->data));
}


static void
glade_clipboard_on_project_destroy (GladeProject *project,
				    GladeClipboardItem *item)
{
  if (item)
    item->project = NULL;
}
