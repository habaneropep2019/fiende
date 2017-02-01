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

#include <string.h>
#include <ctype.h>
#include <time.h>

#include <gtk/gtk.h>

#include "gladeconfig.h"

#ifdef USE_GNOME
#include <gnome.h>
#endif

#include "editor.h"
#include "glade_keys_dialog.h"
#include "glade_project.h"
#include "glade_project_window.h"
#include "utils.h"
#include "gbwidget.h"
#include "property.h"
#include "load.h"
#include "save.h"

/* These are the standard widget attribute names */
const gchar *GbName = "GtkWidget::name";
const gchar *GbClass = "GtkWidget::class";
const gchar *GbX = "GtkWidget::x";
const gchar *GbY = "GtkWidget::y";
const gchar *GbWidth = "GtkWidget::width";
const gchar *GbHeight = "GtkWidget::height";
const gchar *GbVisible = "GtkWidget::visible";
const gchar *GbSensitive = "GtkWidget::sensitive";
const gchar *GbTooltip = "GtkWidget::tooltip";
const gchar *GbCanDefault = "GtkWidget::can_default";
const gchar *GbHasDefault = "GtkWidget::has_default";
const gchar *GbCanFocus = "GtkWidget::can_focus";
const gchar *GbHasFocus = "GtkWidget::has_focus";
const gchar *GbEvents = "GtkWidget::events";
const gchar *GbExtEvents = "GtkWidget::extension_events";

/* This is just a button to show the accelerators dialog. */
const gchar *GbAccelerators = "GtkWidget::accelerators";

/* Language-Specific properties. */
/* C-specific properties. */
const gchar *GbCSourceFile = "GtkWidget::c_source_file";
const gchar *GbCPublic = "GtkWidget::c_public";

/* C++-specific properties. */
const gchar *GbCxxSeparateFile = "GtkWidget::cxx_separate_file";
const gchar *GbCxxUseHeap = "GtkWidget::cxx_use_heap";
const gchar *GbCxxSeparateClass = "GtkWidget::cxx_separate_class";
const gchar *GbCxxVisibility = "GtkWidget::cxx_visibility";

/* Visibility choices data */
const gchar *GbCxxVisibilityChoices[] =
{N_("private"), N_("protected"), N_("public"), NULL};
const gint GbCxxVisibilityValues[] =
{ 0, 1, 2
};
const gchar *GbCxxVisibilitySymbols[] =
{
  "private",
  "protected",
  "public"
};

/* Widget style properties. */
const gchar *GbStylePropagate = "GtkWidget::style_propagate";
const gchar *GbStyleName = "GtkWidget::style_name";
const gchar *GbStyleFont = "GtkWidget::style_font";

/* Signals page */
const gchar *GbSignalName = "GtkWidget::signal_name";
const gchar *GbSignalHandler = "GtkWidget::signal_handler";
const gchar *GbSignalObject = "GtkWidget::signal_object";
const gchar *GbSignalAfter = "GtkWidget::signal_after";
const gchar *GbSignalData = "GtkWidget::signal_data";

/* Accelerators page */
const gchar *GbAccelKey = "GtkWidget::accelerator_key";
const gchar *GbAccelSignal = "GtkWidget::accelerator_signal";


const gchar *GbStateTabLabels[] =
{ N_("Normal"), N_("Active"), N_("Prelight"),
  N_("Selected"), N_("Insens") };

/* Keys in object data hash used to store color, bg pixmap & filename */
const gchar *GbColorKey = "GbColor";
const gchar *GbBgPixmapKey = "GbBgPixmap";
const gchar *GbBgFilenameKey = "GbBgFilename";
const gchar *GbValueWidgetKey = "GbValue";
const gchar *GbDialogValueKey = "GbDialogValue";
const gchar *GbFilenameValueKey = "GbFilenameValue";
const gchar *GbFontKey = "GbFont";
const gchar *GbFontSpecKey = "GbFontSpec";


/* Extension mode choices data */
const gchar *GbExtensionModeChoices[] =
{"None", "All", "Cursor", NULL};
const gint GbExtensionModeValues[] =
{
  GDK_EXTENSION_EVENTS_NONE,
  GDK_EXTENSION_EVENTS_ALL,
  GDK_EXTENSION_EVENTS_CURSOR
};
const gchar *GbExtensionModeSymbols[] =
{
  "GDK_EXTENSION_EVENTS_NONE",
  "GDK_EXTENSION_EVENTS_ALL",
  "GDK_EXTENSION_EVENTS_CURSOR"
};

/*
 * Private variables
 */

/* Hashtables of all the label & value widgets on the various property pages */
static GHashTable *gb_property_labels = NULL;
static GHashTable *gb_property_values = NULL;
static GHashTable *gb_property_buttons = NULL;

/* Major components of the property window */
#define GB_PAGE_WIDGET	0
#define GB_PAGE_SIGNALS	3
static GtkWidget *win_property = NULL;
static GtkWidget *main_notebook = NULL;
static GtkWidget *property_widget_notebook;
static gint property_language = GLADE_LANGUAGE_C;
static GtkWidget **lang_specific_properties;
static GtkWidget *special_child_property_notebook;
static GtkWidget *apply_button = NULL;
#ifdef GLADE_STYLE_SUPPORT
static GtkWidget *styles_notebook;
#endif

static GtkTooltips *tooltips;

/* The widget whose properties are currently shown (or NULL) */
static GtkWidget *property_widget = NULL;

/* This is used when typing over a widget in the interface to set its label. */
static gboolean typing_over_widget = FALSE;

static GtkStyle *invalid_style;

/* The current table & row used when creating properties */
static GtkWidget *property_table;
static gint property_table_row;

/* This is used when automatically applying properties as they are changed.
   It is on most of the time, except when changes are being made to property
   values which we don't want to result in a callback. */
static gboolean auto_apply;

/* Color selection dialog */
static GtkColorSelectionDialog *colorsel = NULL;
static GtkWidget *color_value = NULL;

/* File selection dialog */
static GtkFileSelection *filesel = NULL;
static GtkWidget *filename_value = NULL;

/* Font selection fialog */
static GtkFontSelectionDialog *fontsel = NULL;

/* Widgets in Accelerators dialog. */
#define ACCEL_MODIFIERS_COL	0
#define ACCEL_KEY_COL		1
#define ACCEL_SIGNAL_COL	2
static GtkWidget *accel_dialog = NULL;
static GtkWidget *accel_clist;
static GtkWidget *accel_control_button;
static GtkWidget *accel_shift_button;
static GtkWidget *accel_alt_button;

/* Widgets on Signals page */
#define SIGNAL_NAME_COL		0
#define SIGNAL_HANDLER_COL	1
#define SIGNAL_DATA_COL		2
#define SIGNAL_AFTER_COL	3
#define SIGNAL_OBJECT_COL	4
static GtkWidget *signal_clist;
/* This holds the last modification time of the GladeSignals currently being
   edited. */
static GMemChunk *signal_mem_chunk;

/* Clipboard used for copying/pasting colors - could possibly use GTK clipbd? */
#define GB_COLOR_CLIPBD_EMPTY    1
#define GB_COLOR_CLIPBD_COLOR    2
#define GB_COLOR_CLIPBD_BGPIXMAP 3
#define GB_COLOR_CLIPBD_STATE    4

#ifdef GLADE_STYLE_SUPPORT
static gint clipbd_state = GB_COLOR_CLIPBD_EMPTY;
static GdkColor clipbd_colors[GB_NUM_STYLE_COLORS];
static GdkPixmap *clipbd_bgpixmap;
static gchar *clipbd_bgfilename;
static GtkWidget *selected_style_widget = NULL;
#endif


/*
 * Private functions
 */

static void property_create ();

static GtkWidget *create_widget_property_page ();
static GtkWidget *create_standard_property_page ();
static GtkWidget *create_special_child_properties ();
static void create_language_specific_properties (GtkWidget *vbox);
#ifdef GLADE_STYLE_SUPPORT
static GtkWidget *create_style_property_page ();
static GtkWidget *create_style_page (const gchar * state);
#endif
static GtkWidget *create_signals_property_page ();

static void property_add (const gchar * property_name,
			  const gchar * label_string,
			  GtkWidget * value,
			  GtkWidget * dialog_button,
			  const gchar * tooltip);

static void on_bool_property_toggle (GtkWidget * value,
				     gpointer data);

static GtkWidget *create_color_preview ();
static gboolean show_color_in_preview (GtkWidget * preview,
				       GdkColor *color);
static void on_color_draw (GtkWidget * widget,
			   gpointer data);
static void on_color_expose_event (GtkWidget * widget,
				   GdkEvent * event,
				   gpointer data);
static void on_color_select (GtkWidget * widget,
			     gpointer data);

static void show_pixmap_in_drawing_area (GtkWidget * drawing_area,
					 GdkPixmap * gdkpixmap);

static gint expose_pixmap (GtkWidget * drawing_area,
			   GdkEventExpose * event,
			   gpointer data);
static void draw_pixmap (GtkWidget * widget,
			 GdkRectangle * area,
			 gpointer data);
static void draw_pixmap_focus (GtkWidget * widget,
			       gpointer data);
static void real_draw_pixmap (GtkWidget * drawing_area);

static void show_events_dialog (GtkWidget * widget,
				gpointer value);
static void on_events_dialog_ok (GtkWidget * widget,
				 GtkWidget * clist);

static void show_keys_dialog (GtkWidget * widget,
			      gpointer value);
static void on_keys_clist_select (GtkWidget * widget,
				  gint row,
				  gint column,
				  GdkEventButton * bevent,
				  gpointer data);
static void on_keys_dialog_ok (GtkWidget * widget,
			       gpointer data);

static void show_signals_dialog (GtkWidget * widget,
				 gpointer value);
static void on_signals_clist_select (GtkWidget * widget,
				     gint row,
				     gint column,
				     GdkEventButton * bevent,
				     gpointer data);
static void on_signals_dialog_ok (GtkWidget * widget,
				  GtkWidget * clist);

static void show_accelerators_dialog (GtkWidget * widget,
				      gpointer value);
static void create_accelerators_dialog ();
static void on_accelerator_add (GtkWidget * widget,
				GtkWidget * clist);
static void on_accelerator_update (GtkWidget * widget,
				   GtkWidget * clist);
static void on_accelerator_delete (GtkWidget * widget,
				   GtkWidget * clist);
static void on_accelerator_clear (GtkWidget * widget,
				  GtkWidget * clist);
static void on_accelerator_select (GtkWidget * clist,
				   gint row,
				   gint column,
				   GdkEventButton * event,
				   gpointer user_data);

static void on_signal_add (GtkWidget * widget,
			   GtkWidget * clist);
static void on_signal_update (GtkWidget * widget,
			      GtkWidget * clist);
static void on_signal_delete (GtkWidget * widget,
			      GtkWidget * clist);
static void on_signal_clear (GtkWidget * widget,
			     GtkWidget * clist);
static void on_signal_select (GtkWidget * clist,
			      gint row,
			      gint column,
			      GdkEventButton * event,
			      gpointer user_data);

#ifdef GLADE_STYLE_SUPPORT
static void on_style_copy (GtkWidget * widget,
			   gpointer data);
static void on_style_copy_all (GtkWidget * widget,
			       gpointer data);
static void on_style_paste (GtkWidget * widget,
			    gpointer data);
#endif

/* Currently unused - its all auto-applied
   static void on_apply                 (GtkWidget      *widget,
   gpointer      data);
 */
static void on_property_changed (GtkWidget * widget,
				 GtkWidget * property);

static void set_pixmap (GtkWidget * drawing_area,
			GdkPixmap * gdkpixmap,
			const gchar * filename);

static void show_colorsel_dialog (GtkWidget * widget,
				  gpointer data);
static void on_colorsel_dialog_ok (GtkWidget * widget,
				   gpointer data);
static gint close_dialog_event (GtkWidget * widget,
				GdkEvent * event,
				GtkWidget * dialog);
static void close_dialog (GtkWidget * widget,
			  GtkWidget * dialog);

static void show_filesel_dialog (GtkWidget * widget,
				 gpointer data);
static void on_filesel_dialog_ok (GtkWidget * widget,
				  gpointer data);

static void show_font_dialog (GtkWidget * widget,
			      gpointer data);
#if 0
static gint get_font_size_from_spec (const gchar * spec);
#endif
static gchar *get_font_name_from_spec (const gchar * spec);
static void on_font_dialog_apply (GtkWidget * widget,
				  GtkFontSelectionDialog * fontsel);
static void on_font_dialog_ok (GtkWidget * widget,
			       GtkFontSelectionDialog * fontsel);

#ifdef GLADE_STYLE_SUPPORT
static void show_style_dialog (GtkWidget * widget,
			       gpointer value);
static gint add_style_to_clist (const gchar * key,
				gpointer data,
				GtkWidget * clist);
static void on_style_clist_select (GtkWidget * widget,
				   gint row,
				   gint column,
				   GdkEventButton * bevent,
				   gpointer data);
static void on_style_dialog_new (GtkWidget * widget,
				 GtkWidget * clist);
static gint create_new_style (GtkWidget * widget,
			      const gchar * name,
			      GbStyle * base_gbstyle);
static void on_style_dialog_ok (GtkWidget * widget,
				GtkWidget * clist);
static void on_style_dialog_copy (GtkWidget * widget,
				  GtkWidget * clist);
static void on_style_dialog_rename (GtkWidget * widget,
				    GtkWidget * clist);
static gint rename_style (GtkWidget * widget,
			  const gchar * name,
			  GbStyle * gbstyle);
static void on_style_dialog_delete (GtkWidget * widget,
				    GtkWidget * clist);
#endif

static void on_toggle_set_x (GtkWidget * widget, gpointer value);
static void on_toggle_set_y (GtkWidget * widget, gpointer value);
static void on_toggle_set_width (GtkWidget * widget, gpointer value);
static void on_toggle_set_height (GtkWidget * widget, gpointer value);

const gchar *GbEventMaskSymbols[GB_EVENT_MASKS_COUNT] =
{
  "GDK_EXPOSURE_MASK",
  "GDK_POINTER_MOTION_MASK",
  "GDK_POINTER_MOTION_HINT_MASK",
  "GDK_BUTTON_MOTION_MASK",
  "GDK_BUTTON1_MOTION_MASK",
  "GDK_BUTTON2_MOTION_MASK",
  "GDK_BUTTON3_MOTION_MASK",
  "GDK_BUTTON_PRESS_MASK",
  "GDK_BUTTON_RELEASE_MASK",
  "GDK_KEY_PRESS_MASK",
  "GDK_KEY_RELEASE_MASK",
  "GDK_ENTER_NOTIFY_MASK",
  "GDK_LEAVE_NOTIFY_MASK",
  "GDK_FOCUS_CHANGE_MASK",
  "GDK_STRUCTURE_MASK",
  "GDK_PROPERTY_CHANGE_MASK",
  "GDK_VISIBILITY_NOTIFY_MASK",
  "GDK_PROXIMITY_IN_MASK",
  "GDK_PROXIMITY_OUT_MASK",
};


const gint GbEventMaskValues[GB_EVENT_MASKS_COUNT] =
{
  GDK_EXPOSURE_MASK,
  GDK_POINTER_MOTION_MASK,
  GDK_POINTER_MOTION_HINT_MASK,
  GDK_BUTTON_MOTION_MASK,
  GDK_BUTTON1_MOTION_MASK,
  GDK_BUTTON2_MOTION_MASK,
  GDK_BUTTON3_MOTION_MASK,
  GDK_BUTTON_PRESS_MASK,
  GDK_BUTTON_RELEASE_MASK,
  GDK_KEY_PRESS_MASK,
  GDK_KEY_RELEASE_MASK,
  GDK_ENTER_NOTIFY_MASK,
  GDK_LEAVE_NOTIFY_MASK,
  GDK_FOCUS_CHANGE_MASK,
  GDK_STRUCTURE_MASK,
  GDK_PROPERTY_CHANGE_MASK,
  GDK_VISIBILITY_NOTIFY_MASK,
  GDK_PROXIMITY_IN_MASK,
  GDK_PROXIMITY_OUT_MASK
};


static const gchar *GbEventMaskDescriptions[GB_EVENT_MASKS_COUNT] =
{
  N_("When the window needs redrawing"),
  N_("When the mouse moves"),
  N_("Mouse movement hints"),
  N_("Mouse movement with any button pressed"),
  N_("Mouse movement with button 1 pressed"),
  N_("Mouse movement with button 2 pressed"),
  N_("Mouse movement with button 3 pressed"),
  N_("Any mouse button pressed"),
  N_("Any mouse button released"),
  N_("Any key pressed"),
  N_("Any key released"),
  N_("When the mouse enters the window"),
  N_("When the mouse leaves the window"),
  N_("Any change in input focus"),
  N_("Any change in window structure"),
  N_("Any change in X Windows property"),
  N_("Any change in visibility"),
  N_("For cursors in XInput-aware programs"),
  N_("For cursors in XInput-aware programs")
};

/*
 * Create the properties window
 */

static void
property_create ()
{
  GtkWidget *vbox1, *label, *child;

  /* Create hash table for widgets containing property values */
  gb_property_labels = g_hash_table_new (g_str_hash, g_str_equal);
  gb_property_values = g_hash_table_new (g_str_hash, g_str_equal);
  gb_property_buttons = g_hash_table_new (g_str_hash, g_str_equal);

  /* Create the mem chunk for storing signal last modification times. */
  signal_mem_chunk = g_mem_chunk_create (time_t, 16, G_ALLOC_ONLY);

  /* Create property window */
  win_property = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_uposition (win_property, 250, 0);
  /*gtk_widget_set_usize (win_property, 240, -1);*/
  /*gtk_window_set_default_size (GTK_WINDOW (win_property), 400, 600);*/
  gtk_widget_set_name (win_property, "GladeObjectInspector");
  gtk_window_set_wmclass (GTK_WINDOW (win_property),
			  "object_inspector", "Glade");
  gtk_widget_realize (win_property);

  gtk_signal_connect (GTK_OBJECT (win_property), "delete_event",
		      GTK_SIGNAL_FUNC (property_hide), NULL);

  gtk_window_set_title (GTK_WINDOW (win_property), _("Properties"));
  gtk_container_set_border_width (GTK_CONTAINER (win_property), 0);

  tooltips = gtk_tooltips_new ();

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (win_property), vbox1);
  gtk_widget_show (vbox1);

  /* Create main notebook for different categories of widget properties */
  main_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (main_notebook), GTK_POS_TOP);
  /*gtk_notebook_set_tab_border (GTK_NOTEBOOK (main_notebook), 2); */
  gtk_box_pack_start (GTK_BOX (vbox1), main_notebook, TRUE, TRUE, 0);
  gtk_widget_show (main_notebook);

  /* Create the pages of the main notebook */
  /* NOTE: If you add/remove pages you need to change the GB_PAGE_SIGNALS
     value at the top of this file */
  label = gtk_label_new (_("Widget"));
  gtk_widget_show (label);
  child = create_widget_property_page ();
  gtk_notebook_append_page (GTK_NOTEBOOK (main_notebook), child, label);

  label = gtk_label_new (_("Packing"));
  gtk_widget_show (label);
  child = create_special_child_properties ();
  gtk_notebook_append_page (GTK_NOTEBOOK (main_notebook), child, label);

  label = gtk_label_new (_("Common"));
  gtk_widget_show (label);
  child = create_standard_property_page ();
  gtk_notebook_append_page (GTK_NOTEBOOK (main_notebook), child, label);

#ifdef GLADE_STYLE_SUPPORT
  label = gtk_label_new (_("Style"));
  gtk_widget_show (label);
  child = create_style_property_page ();
  gtk_notebook_append_page (GTK_NOTEBOOK (main_notebook), child, label);
#endif

  label = gtk_label_new (_("Signals"));
  gtk_widget_show (label);
  child = create_signals_property_page ();
  gtk_notebook_append_page (GTK_NOTEBOOK (main_notebook), child, label);

  /* Create style to use when a property value is invalid - it's not used
     at present. */
  invalid_style = gtk_style_new ();
  invalid_style->fg[GTK_STATE_NORMAL].red = 65535;
  invalid_style->fg[GTK_STATE_NORMAL].green = 0;
  invalid_style->fg[GTK_STATE_NORMAL].blue = 0;

  create_accelerators_dialog ();
}


void
property_show (GtkWidget * widget,
	       gpointer data)
{
  if (!win_property)
    property_create ();
  gtk_widget_show (win_property);
  /* This maps the window, which also de-iconifies it according to ICCCM. */
  gdk_window_show (GTK_WIDGET (win_property)->window);
  gdk_window_raise (GTK_WIDGET (win_property)->window);
  property_set_widget (NULL);
}

gint
property_hide (GtkWidget * widget,
	       gpointer data)
{
  gtk_widget_hide (win_property);
  return TRUE;
}


GtkWidget *
property_get_widget ()
{
  return property_widget;
}


void
property_set_widget (GtkWidget * widget)
{
  gchar buffer[128];
  gchar *name;

  if (widget)
    {
      strcpy (buffer, _("Properties: "));
      name = gtk_widget_get_name (widget);
      strncat (buffer, name, 120 - strlen (buffer));
      gtk_window_set_title (GTK_WINDOW (win_property), buffer);
      gtk_widget_set_sensitive (main_notebook, TRUE);
      if (apply_button)
	gtk_widget_set_sensitive (apply_button, TRUE);
      property_widget = widget;
    }
  else
    {
      gtk_window_set_title (GTK_WINDOW (win_property), _("Properties: <none>"));
      property_hide_gbwidget_page ();
      gtk_widget_set_sensitive (main_notebook, FALSE);
      if (apply_button)
	gtk_widget_set_sensitive (apply_button, FALSE);
      /* Need to clear the property widget before clearing the name property,
         since auto-apply may be on, and hence we may clear the name of the
         currently shown widget. */
      property_widget = NULL;
      property_set_string (GbName, NULL);
      property_set_string (GbClass, NULL);
      /* Make now-empty property window display properly */
      gtk_widget_queue_resize (win_property);
    }

  typing_over_widget = FALSE;
}


static GtkWidget *
create_widget_property_page ()
{
  GtkWidget *page, *table, *vbox, *entry;

  page = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (page),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (page);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (page), vbox);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (page)->child),
				GTK_SHADOW_NONE);
  gtk_widget_show (vbox);

  table = gtk_table_new (9, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, TRUE, 2);

  property_set_table_position (table, 0);
  property_add_string (GbName, _("Name:"),
		       _("The name of the widget"));
  property_add_string (GbClass, _("Class:"),
		       _("The class of the widget"));
  entry = property_get_value_widget (GbClass);
  gtk_entry_set_editable (GTK_ENTRY (entry), FALSE);

  /* Add language-specific options to a notebook at the bottom. */
  create_language_specific_properties (vbox);

  gtk_widget_show (table);

  property_widget_notebook = gtk_notebook_new ();
  gtk_widget_show (property_widget_notebook);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (property_widget_notebook),
			      FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (property_widget_notebook),
				FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), property_widget_notebook,
		      FALSE, TRUE, 0);

  return page;
}


static GtkWidget *
create_standard_property_page ()
{
  GtkWidget *page;

  page = gtk_table_new (9, 3, FALSE);

  property_set_table_position (page, 0);
  property_add_optional_int_range (GbX, _("X:"),
				   _("The left edge of the widget, relative to its parent"),
				   0, 10000, 1, 10, 1, on_toggle_set_x);
  property_add_optional_int_range (GbY, _("Y:"),
				   _("The top edge of the widget, relative to its parent"),
				   0, 10000, 1, 10, 1, on_toggle_set_y);
  property_add_optional_int_range (GbWidth, _("Width:"),
				   _("The width of the widget"),
				   0, 10000, 1, 10, 1, on_toggle_set_width);
  property_add_optional_int_range (GbHeight, _("Height:"),
				   _("The height of the widget"),
				   0, 10000, 1, 10, 1, on_toggle_set_height);

  property_add_bool (GbVisible, _("Visible:"),
		     _("If the widget is initially visible"));
  property_add_bool (GbSensitive, _("Sensitive:"),
		     _("If the widget responds to input"));
  property_add_string (GbTooltip, _("Tooltip:"),
	     _("The tooltip to display if the mouse lingers over the widget"));

  property_add_bool (GbCanDefault, _("Can Default:"),
		     _("If the widget can be the default"));
  property_add_bool (GbHasDefault, _("Has Default:"),
		     _("If the widget is the default"));
  property_add_bool (GbCanFocus, _("Can Focus:"),
		     _("If the widget can accept the input focus"));
  property_add_bool (GbHasFocus, _("Has Focus:"),
		     _("If the widget has the input focus"));

  property_add_dialog (GbEvents, _("Events:"),
		       _("The X events that the widget receives"),
		       FALSE, show_events_dialog);
  property_add_choice (GbExtEvents, _("Ext.Events:"),
		       _("The X Extension events mode"),
		       GbExtensionModeChoices);

  property_add_command (GbAccelerators, _("Accelerators:"),
			_("Defines the signals to emit when keys are pressed"),
			_("Edit..."), show_accelerators_dialog);

  gtk_widget_show (page);
  return page;
}


#ifdef GLADE_STYLE_SUPPORT
static GtkWidget *
create_style_property_page ()
{
  GtkWidget *page, *table, *label, *child, *hbox, *button;
  int i;

  page = gtk_vbox_new (FALSE, 0);

  table = gtk_table_new (3, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (page), table, FALSE, TRUE, 0);

  property_set_table_position (table, 0);
  property_add_bool (GbStylePropagate, _("Propagate:"),
		_("Set True to propagate the style to the widget's children"));
  property_add_dialog (GbStyleName, _("Named Style:"),
	    _("The name of the style, which can be shared by several widgets"),
		       FALSE, show_style_dialog);
  property_add_font (GbStyleFont, _("Font:"),
		     _("The font to use for any text in the widget"));

  /* Create notebook for 5 states */
  styles_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (styles_notebook), GTK_POS_TOP);
  /*gtk_notebook_set_tab_border (GTK_NOTEBOOK (styles_notebook), 2); */
  gtk_box_pack_start (GTK_BOX (page), styles_notebook, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (styles_notebook), 0);

  for (i = 0; i < GB_NUM_STYLE_STATES; i++)
    {
      label = gtk_label_new (_(GbStateTabLabels[i]));
      gtk_widget_show (label);
      child = create_style_page (GbStateNames[i]);
      gtk_notebook_append_page (GTK_NOTEBOOK (styles_notebook), child, label);
    }
  gtk_widget_show (styles_notebook);

  /* Add/Update/Delete buttons at bottom */
  hbox = gtk_hbox_new (TRUE, 5);
  button = gtk_button_new_with_label (_("Copy"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_copy), NULL);

  button = gtk_button_new_with_label (_("Copy All"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_copy_all), NULL);

  button = gtk_button_new_with_label (_("Paste"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_paste), NULL);

  gtk_box_pack_start (GTK_BOX (page), hbox, FALSE, TRUE, 3);
  gtk_widget_show (hbox);

  gtk_widget_show (table);
  gtk_widget_show (page);
  return page;
}


static GtkWidget *
create_style_page (const gchar * state)
{
  GtkWidget *page, *table;
  int i;
  gchar buffer[128];
  gchar *labels[] =
  {N_("Foreground:"), N_("Background:"), N_("Text:"), N_("Base:") };
  gchar *tooltips[] =
  {N_("Foreground color"), N_("Background color"), N_("Text color"),
   N_("Base color") };

  page = gtk_vbox_new (FALSE, 0);

  /* Add the seven colours for this state */
  table = gtk_table_new (8, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (page), table, TRUE, TRUE, 0);
  property_set_table_position (table, 0);
  for (i = 0; i < GB_NUM_STYLE_COLORS; i++)
    {
      sprintf (buffer, "GtkStyle::%s[%s]", GbColorNames[i], state);
      property_add_color (buffer, _(labels[i]), _(tooltips[i]));
    }

  /* Add the background pixmap */
  sprintf (buffer, "GtkStyle::%s[%s]", GbBgPixmapName, state);
  property_add_bgpixmap (buffer, _("Back. Pixmap:"),
		      _("The graphic to use as the background of the widget"));

  gtk_widget_show (table);
  gtk_widget_show (page);
  return page;
}
#endif


static GtkWidget *
create_special_child_properties ()
{
  GtkWidget *page;

  page = gtk_vbox_new (FALSE, 0);

  special_child_property_notebook = gtk_notebook_new ();
  gtk_widget_show (special_child_property_notebook);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (special_child_property_notebook),
			      FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (special_child_property_notebook),
				FALSE);
  gtk_box_pack_start (GTK_BOX (page), special_child_property_notebook,
		      TRUE, TRUE, 0);
  gtk_widget_show (page);
  return page;
}


static void
create_language_specific_properties (GtkWidget *vbox)
{
  GtkWidget *table;
  gint i;

  lang_specific_properties = g_new (GtkWidget*, GladeNumLanguages);
  for (i = 0; i < GladeNumLanguages; i++)
    lang_specific_properties[i] = NULL;

  /* Create table for C-specific properties. */
  table = gtk_table_new (3, 3, FALSE);
  if (property_language == GLADE_LANGUAGE_C)
    gtk_widget_show (table);
  lang_specific_properties[GLADE_LANGUAGE_C] = table;
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

  property_set_table_position (table, 0);
  property_add_filename (GbCSourceFile, _("Source File:"),
			 _("The file to write source code into"));
  property_add_bool (GbCPublic, _("Public:"),
		     _("If the widget is added to the component's data structure"));

  /* Create table for C++-specific properties. */
  table = gtk_table_new (3, 3, FALSE);
  if (property_language == GLADE_LANGUAGE_CPP)
    gtk_widget_show (table);
  lang_specific_properties[GLADE_LANGUAGE_CPP] = table;
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

  property_set_table_position (table, 0);
  property_add_bool (GbCxxSeparateFile, _("Separate File:"),
		     _("Put this widget in a separate source file"));
  property_add_bool (GbCxxUseHeap, _("Use Heap:"),
		     _("If the widget is allocated from the heap rather than from the parent's memory block"));
  property_add_bool (GbCxxSeparateClass, _("Separate Class:"),
		     _("Put this widget's subtree in a separate class"));
  property_add_choice (GbCxxVisibility, _("Visibility:"),
		       _("Visibility of widgets. Public widgets are exported to a global map."),
		       GbCxxVisibilityChoices);

  /* Create table for Ada95-specific properties. */
  table = gtk_table_new (3, 3, FALSE);
  if (property_language == GLADE_LANGUAGE_ADA95)
    gtk_widget_show (table);
  lang_specific_properties[GLADE_LANGUAGE_ADA95] = table;
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

  property_set_table_position (table, 0);
  /* No properties yet. */


  /* Create table for Perl-specific properties. */
  table = gtk_table_new (3, 3, FALSE);
  if (property_language == GLADE_LANGUAGE_PERL)
    gtk_widget_show (table);
  lang_specific_properties[GLADE_LANGUAGE_PERL] = table;
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

  property_set_table_position (table, 0);
  /* No properties yet. */

}



gint
property_add_gbwidget_page (GtkWidget * page)
{
  gtk_notebook_append_page (GTK_NOTEBOOK (property_widget_notebook), page,
			    NULL);
  return g_list_length (GTK_NOTEBOOK (property_widget_notebook)->children) - 1;
}


void
property_hide_gbwidget_page ()
{
  gtk_widget_hide (property_widget_notebook);
}


void
property_show_gbwidget_page (gint page)
{
  gtk_notebook_set_page (GTK_NOTEBOOK (property_widget_notebook), page);
  gtk_widget_show (property_widget_notebook);
}


gint
property_add_child_packing_page (GtkWidget * page)
{
  gtk_notebook_append_page (GTK_NOTEBOOK (special_child_property_notebook),
			    page, NULL);
  return g_list_length (GTK_NOTEBOOK (special_child_property_notebook)->children) - 1;
}


void
property_hide_child_packing_page ()
{
  gtk_widget_hide (special_child_property_notebook);
}


void
property_show_child_packing_page (gint page)
{
  gtk_notebook_set_page (GTK_NOTEBOOK (special_child_property_notebook), page);
  gtk_widget_show (special_child_property_notebook);
}


void
property_show_lang_specific_page	(GladeLanguageType language)
{
  gint i;
  
  property_language = language;
  if (win_property == NULL)
    return;

  for (i = 0; i < GladeNumLanguages; i++)
    {
      if (lang_specific_properties[i])
	{
	  /* FIXME: show the C language properties when we support them. */
	  if (i == language && i != GLADE_LANGUAGE_C)
	    gtk_widget_show (lang_specific_properties[i]);
	  else
	    gtk_widget_hide (lang_specific_properties[i]);
	}
    }
}


#ifdef GLADE_STYLE_SUPPORT
static void
on_style_copy (GtkWidget * widget, gpointer data)
{
  GdkColor *color;

  if (!selected_style_widget)
    {
      glade_util_show_message_box (_("You need to select a color or background to copy"));
      return;
    }
  if ((color = gtk_object_get_data (GTK_OBJECT (selected_style_widget),
				    GbColorKey)))
    {
      clipbd_colors[0] = *color;
      clipbd_state = GB_COLOR_CLIPBD_COLOR;
    }
  else if (GTK_IS_DRAWING_AREA (selected_style_widget))
    {
      clipbd_bgpixmap = gtk_object_get_data (GTK_OBJECT (selected_style_widget),
					     GbBgPixmapKey);
      clipbd_bgfilename = gtk_object_get_data (GTK_OBJECT (selected_style_widget),
					       GbBgFilenameKey);
      clipbd_state = GB_COLOR_CLIPBD_BGPIXMAP;
    }
  else
    {
      g_warning (_("Invalid selection in on_style_copy()"));
    }
}


static void
on_style_copy_all (GtkWidget * widget, gpointer data)
{
  gint page, i;
  GdkColor *color;
  gchar buffer[128];

  /* Find out which state is currently displayed */
  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (styles_notebook));

  for (i = 0; i < GB_NUM_STYLE_COLORS; i++)
    {
      sprintf (buffer, "GtkStyle::%s[%s]", GbColorNames[i],
	       GbStateNames[page]);
      color = property_get_color (buffer, NULL, NULL);
      clipbd_colors[i] = *color;
    }
  sprintf (buffer, "GtkStyle::%s[%s]", GbBgPixmapName, GbStateNames[page]);
  clipbd_bgpixmap = property_get_bgpixmap (buffer, NULL, NULL,
					   &clipbd_bgfilename);

  clipbd_state = GB_COLOR_CLIPBD_STATE;
}


static void
on_style_paste (GtkWidget * widget, gpointer data)
{
  gchar buffer[128];
  gint page, i;
  GtkWidget *value, *preview, *drawing_area, *changed_value = NULL;
  gboolean bgpixmap_changed;
  gchar *filename;

  switch (clipbd_state)
    {
    case GB_COLOR_CLIPBD_EMPTY:
      glade_util_show_message_box (_("You need to copy a color or background pixmap first"));
      break;
    case GB_COLOR_CLIPBD_COLOR:
      if (!selected_style_widget
	  || !GTK_IS_DRAWING_AREA (selected_style_widget))
	{
	  glade_util_show_message_box (_("You need to select a color to paste into"));
	  return;
	}
      show_color_in_preview (selected_style_widget, &clipbd_colors[0]);
      value = selected_style_widget->parent;
      on_property_changed (value, value);
      break;
    case GB_COLOR_CLIPBD_BGPIXMAP:
      if (!selected_style_widget || !GTK_IS_DRAWING_AREA (selected_style_widget))
	{
	  glade_util_show_message_box (_("You need to select a background pixmap to paste into"));
	  return;
	}
      set_pixmap (selected_style_widget, clipbd_bgpixmap, clipbd_bgfilename);
      value = selected_style_widget->parent;
      on_property_changed (value, value);
      break;
    case GB_COLOR_CLIPBD_STATE:
      page = gtk_notebook_get_current_page (GTK_NOTEBOOK (styles_notebook));

      /* We need to find one color or background pixmap which has changed so
         we can call on_property_changed with it, thus recreating the style.
         We don't want to call on_property_changed multiple times. */
      for (i = 0; i < GB_NUM_STYLE_COLORS; i++)
	{
	  sprintf (buffer, "GtkStyle::%s[%s]", GbColorNames[i],
		   GbStateNames[page]);
	  value = (GtkWidget *) g_hash_table_lookup (gb_property_values,
						     buffer);
	  g_return_if_fail (value != NULL);
	  preview = GTK_BIN (value)->child;
	  g_return_if_fail (GTK_IS_DRAWING_AREA (preview));
	  if (show_color_in_preview (preview, &clipbd_colors[i]))
	    changed_value = value;
	}

      sprintf (buffer, "GtkStyle::%s[%s]", GbBgPixmapName, GbStateNames[page]);
      value = (GtkWidget *) g_hash_table_lookup (gb_property_values, buffer);
      g_return_if_fail (value != NULL);
      drawing_area = GTK_BIN (value)->child;
      g_return_if_fail (GTK_IS_DRAWING_AREA (drawing_area));
      filename = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgFilenameKey);
      bgpixmap_changed = FALSE;
      if (filename)
	{
	  if (!clipbd_bgfilename || strcmp (filename, clipbd_bgfilename))
	    bgpixmap_changed = TRUE;
	}
      else
	{
	  if (clipbd_bgfilename)
	    bgpixmap_changed = TRUE;
	}
      if (bgpixmap_changed)
	{
	  property_set_bgpixmap (buffer, clipbd_bgpixmap, clipbd_bgfilename);
	  changed_value = value;
	}

      if (changed_value)
	on_property_changed (changed_value, changed_value);

      break;
    }
}
#endif


/*
 * Color selection dialog
 */

static void
show_colorsel_dialog (GtkWidget * widget, gpointer data)
{
  GdkColor *color;
  gdouble rgb[4];
  GtkWidget *transient_parent ;

  /* Create the dialog if it doesn't exist yet */
  if (!colorsel)
    {
      colorsel = GTK_COLOR_SELECTION_DIALOG (gtk_color_selection_dialog_new (_("Color Selection Dialog")));
      /* Hide the Help button since it is not used */
      gtk_widget_hide (GTK_WIDGET (colorsel->help_button));
      gtk_signal_connect (GTK_OBJECT (colorsel), "delete_event",
			  GTK_SIGNAL_FUNC (close_dialog_event), colorsel);
      gtk_signal_connect (GTK_OBJECT (colorsel->cancel_button), "clicked",
			  GTK_SIGNAL_FUNC (close_dialog), colorsel);
      gtk_signal_connect (GTK_OBJECT (colorsel->ok_button), "clicked",
			  GTK_SIGNAL_FUNC (on_colorsel_dialog_ok), NULL);
      gtk_signal_connect (GTK_OBJECT (colorsel), "key_press_event",
			  GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
			  GINT_TO_POINTER (GladeEscCloses));
      gtk_window_set_wmclass (GTK_WINDOW (colorsel), "color_selection", "Glade");
    }

  color_value = GTK_WIDGET (data);
  g_return_if_fail (GTK_IS_FRAME (color_value));

  color = gtk_object_get_data (GTK_OBJECT (GTK_BIN (color_value)->child),
			       GbColorKey);
  g_return_if_fail (color != NULL);

  rgb[0] = ((gdouble) color->red)   / 0xFFFF;
  rgb[1] = ((gdouble) color->green) / 0xFFFF;
  rgb[2] = ((gdouble) color->blue)  / 0xFFFF;

  gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel->colorsel),
				 rgb);
  gtk_widget_show (GTK_WIDGET (colorsel));
  transient_parent = glade_util_get_toplevel (widget);
  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (colorsel),
				  GTK_WINDOW (transient_parent));
  /* This maps the window, which also de-iconifies it according to ICCCM. */
  gdk_window_show (GTK_WIDGET (colorsel)->window);
  gdk_window_raise (GTK_WIDGET (colorsel)->window);
}


static void
on_colorsel_dialog_ok (GtkWidget * widget, gpointer data)
{
  GdkColor color;
  gdouble rgb[4];
  GtkWidget *preview = GTK_BIN (color_value)->child;

  gtk_color_selection_get_color (GTK_COLOR_SELECTION (colorsel->colorsel),
				 rgb);

  color.red   = (gushort) 0xFFFF * rgb[0];
  color.green = (gushort) 0xFFFF * rgb[1];
  color.blue  = (gushort) 0xFFFF * rgb[2];

  show_color_in_preview (preview, &color);
  close_dialog (widget, GTK_WIDGET (colorsel));
  on_property_changed (color_value, color_value);
}


static gint
close_dialog_event (GtkWidget * widget, GdkEvent * event, GtkWidget * dialog)
{
  close_dialog (widget, dialog);
  return TRUE;
}


static void
close_dialog (GtkWidget * widget, GtkWidget * dialog)
{
  glade_util_close_window (dialog);
}


/*
 * File selection dialog
 */

static void
show_filesel_dialog (GtkWidget * widget, gpointer data)
{
  gchar *filename;
  GtkWidget *transient_parent;
  /* value can be an Entry or Combo (for filename properties) or a Frame (for
     background pixmaps) */
  filename_value = GTK_WIDGET (data);

  /* Create the dialog if it doesn't exist yet */
  if (!filesel)
    {
      filesel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Select File")));
      gtk_signal_connect (GTK_OBJECT (filesel), "delete_event",
			  GTK_SIGNAL_FUNC (close_dialog_event), filesel);
      gtk_signal_connect (GTK_OBJECT (filesel->cancel_button), "clicked",
			  GTK_SIGNAL_FUNC (close_dialog), filesel);
      gtk_signal_connect (GTK_OBJECT (filesel->ok_button), "clicked",
			  GTK_SIGNAL_FUNC (on_filesel_dialog_ok), NULL);
      gtk_signal_connect (GTK_OBJECT (filesel), "key_press_event",
		      GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
		      GINT_TO_POINTER (GladeEscCloses));
      gtk_window_set_wmclass (GTK_WINDOW (filesel), "file_selection", "Glade");
    }

  /* set to current file, if there is one */
  if (GTK_IS_ENTRY (filename_value) || GTK_IS_COMBO (filename_value))
    {
      filename = gtk_object_get_data (GTK_OBJECT (filename_value),
				      GbFilenameValueKey);
    }
  else
    {
      filename = gtk_object_get_data (GTK_OBJECT (GTK_BIN (filename_value)->child),
				      GbBgFilenameKey);
    }
  if (filename != NULL)
    {
      MSG1 ("Setting filename to: %s", filename);
      gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), filename);
    }
  gtk_widget_show (GTK_WIDGET (filesel));
  transient_parent = glade_util_get_toplevel (widget);
  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (filesel),
				  GTK_WINDOW (transient_parent));
  /* This maps the window, which also de-iconifies it according to ICCCM. */
  gdk_window_show (GTK_WIDGET (filesel)->window);
  gdk_window_raise (GTK_WIDGET (filesel)->window);
}


static void
on_filesel_dialog_ok (GtkWidget * widget, gpointer data)
{
  GdkPixmap *gdkpixmap = NULL, *old_gdkpixmap;
  gchar *filename, *old_filename;
  GtkWidget *drawing_area;
  gint filename_len;

  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));

  /* If the filename ends in '/' it means the user wants to reset the
     pixmap to NULL. */
  filename_len = strlen (filename);
  if (filename_len > 0 && filename[filename_len - 1] == '/')
    filename = NULL;

  /* For pixmaps we just show the file basename */
  if (GTK_IS_ENTRY (filename_value) || GTK_IS_COMBO (filename_value))
    {
      g_free (gtk_object_get_data (GTK_OBJECT (filename_value),
				   GbFilenameValueKey));
      gtk_object_set_data (GTK_OBJECT (filename_value), GbFilenameValueKey,
			   g_strdup (filename));
      if (GTK_IS_ENTRY (filename_value))
	gtk_entry_set_text (GTK_ENTRY (filename_value),
			    filename ? g_basename (filename) : "");
      else
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (filename_value)->entry),
			    filename ? g_basename (filename) : "");
      close_dialog (widget, GTK_WIDGET (filesel));
      on_property_changed (filename_value, filename_value);
      return;
    }

  drawing_area = GTK_BIN (filename_value)->child;

  /* For background pixmaps we show them in the property value widget */
  if (filename)
    {
      gdkpixmap = gdk_pixmap_create_from_xpm (drawing_area->window, NULL,
					      &drawing_area->style->bg[GTK_STATE_NORMAL],
					      filename);
      if (!gdkpixmap)
	{
	  glade_util_show_message_box (_("Couldn't create pixmap from file\n"),
				       GTK_WIDGET (filesel));
	  return;
	}
    }

  old_gdkpixmap = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgPixmapKey);
  if (old_gdkpixmap)
    gdk_pixmap_unref (old_gdkpixmap);
  old_filename = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgFilenameKey);
  g_free (old_filename);

  gtk_object_set_data (GTK_OBJECT (drawing_area), GbBgPixmapKey, gdkpixmap);
  gtk_object_set_data (GTK_OBJECT (drawing_area), GbBgFilenameKey,
		       g_strdup (filename));

  close_dialog (widget, GTK_WIDGET (filesel));
  show_pixmap_in_drawing_area (drawing_area, gdkpixmap);
  gtk_widget_queue_draw (drawing_area);
  on_property_changed (filename_value, filename_value);
}


/*

 */


static GtkWidget *
create_signals_property_page ()
{
  GtkWidget *page, *table, *hbox, *button, *scrolled_win;
  gchar *signal_titles[5];
  GList *handler_list = NULL;

  page = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (page);

  /* List of current signal handlers - Signal/Handler/Data/Options */
  signal_titles[0] = _("Signal");
  signal_titles[1] = _("Handler");
  signal_titles[2] = _("Data");
  signal_titles[3] = _("After");
  signal_titles[4] = _("Object");
  signal_clist = gtk_clist_new_with_titles (5, signal_titles);
  gtk_clist_set_column_width (GTK_CLIST (signal_clist), 0, 150);
  gtk_clist_set_column_width (GTK_CLIST (signal_clist), 1, 150);
  gtk_clist_set_column_width (GTK_CLIST (signal_clist), 2, 80);
  gtk_clist_set_column_width (GTK_CLIST (signal_clist), 3, 50);
  gtk_clist_set_column_width (GTK_CLIST (signal_clist), 4, 80);
  gtk_widget_show (signal_clist);
  gtk_signal_connect (GTK_OBJECT (signal_clist), "select_row",
		      GTK_SIGNAL_FUNC (on_signal_select), NULL);

  /* Hide the Data & Object columns. */
#if 0
  gtk_clist_set_column_visibility (GTK_CLIST (signal_clist), 2, FALSE);
  gtk_clist_set_column_visibility (GTK_CLIST (signal_clist), 4, FALSE);
#endif

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  /* This is only here to set a minimum size for the property editor. */
  gtk_widget_set_usize (scrolled_win, 230, -1);
  gtk_container_add (GTK_CONTAINER (scrolled_win), signal_clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (page), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_win);

  /* Mod, Key & Signal fields */
  table = gtk_table_new (3, 3, FALSE);
  gtk_widget_show (table);
  property_set_table_position (table, 0);
  property_add_dialog (GbSignalName, _("Signal:"),
		       _("The signal to add a handler for"), TRUE,
		       show_signals_dialog);

  /* FIXME: These are copied in gbsource.c, and should be language-specific,
     i.e. we may need to change them when the project option changes. */
  handler_list = g_list_append (handler_list, "gtk_widget_show");
  handler_list = g_list_append (handler_list, "gtk_widget_hide");
  handler_list = g_list_append (handler_list, "gtk_widget_grab_focus");
  handler_list = g_list_append (handler_list, "gtk_widget_destroy");
  handler_list = g_list_append (handler_list, "gtk_window_activate_default");
  handler_list = g_list_append (handler_list, "gtk_true");
  handler_list = g_list_append (handler_list, "gtk_false");
  handler_list = g_list_append (handler_list, "gtk_main_quit");
  property_add_combo (GbSignalHandler, _("Handler:"),
		       _("The function to handle the signal"), handler_list);
  g_list_free (handler_list);

  property_add_string (GbSignalData, _("Data:"),
		       _("The data passed to the handler"));
  property_add_string (GbSignalObject, _("Object:"),
		       _("The object which receives the signal"));
  property_add_bool (GbSignalAfter, _("After:"),
		     _("If the handler runs after the class function"));

  /* Hide the data & object properties since they make signals too complicated.
     But we'll leave them in for a while so we don't break old apps. */
#if 0
  property_set_visible (GbSignalData, FALSE);
  property_set_visible (GbSignalObject, FALSE);
#endif

  gtk_box_pack_start (GTK_BOX (page), table, FALSE, TRUE, 5);

  /* Add/Update/Delete buttons at bottom */
  hbox = gtk_hbox_new (TRUE, 2);
  button = gtk_button_new_with_label (_("Add"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_signal_add), signal_clist);

  button = gtk_button_new_with_label (_("Update"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_signal_update), signal_clist);

  button = gtk_button_new_with_label (_("Delete"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_signal_delete), signal_clist);

  button = gtk_button_new_with_label (_("Clear"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_signal_clear), signal_clist);

  gtk_box_pack_start (GTK_BOX (page), hbox, FALSE, TRUE, 5);
  gtk_widget_show (hbox);

  return page;
}


static void
show_accelerators_dialog (GtkWidget * widget,
			  gpointer value)
{
  GtkWidget *transient_parent = glade_util_get_toplevel (widget);
  if (!accel_dialog)
    create_accelerators_dialog ();

  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (accel_dialog),
				  GTK_WINDOW (transient_parent));
  
  gtk_widget_show (accel_dialog);
  if (GTK_WIDGET_REALIZED (accel_dialog))
    {
      gdk_window_show (accel_dialog->window);
      gdk_window_raise (accel_dialog->window);
    }
}


static void
create_accelerators_dialog ()
{
  GtkWidget *vbox, *vbox2;
  gchar *accel_titles[3];
  GtkWidget *hbox, *label, *button, *table, *scrolled_win;
  int row;

#ifdef USE_GNOME
  accel_dialog = gnome_dialog_new (_("Accelerators"), GNOME_STOCK_BUTTON_CLOSE,
				   NULL);
  gnome_dialog_set_close (GNOME_DIALOG (accel_dialog), TRUE);
  gnome_dialog_close_hides (GNOME_DIALOG (accel_dialog), TRUE);
  vbox = GNOME_DIALOG (accel_dialog)->vbox;
  gtk_window_set_wmclass (GTK_WINDOW (accel_dialog), "accelerators", "Glade");
#else
  GtkWidget *buttonbox;

  accel_dialog = gtk_dialog_new ();
  gtk_container_set_border_width (GTK_CONTAINER (accel_dialog), 8);
  gtk_window_set_title (GTK_WINDOW (accel_dialog), _("Accelerators"));
  gtk_window_set_wmclass (GTK_WINDOW (accel_dialog), "accelerators", "Glade");

  buttonbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_END);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (accel_dialog)->action_area),
		     buttonbox);
  gtk_widget_show (buttonbox);

  button = gtk_button_new_with_label (_("Close"));
  gtk_container_add (GTK_CONTAINER (buttonbox), button);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (glade_util_close_window),
			     GTK_OBJECT (accel_dialog));
  gtk_signal_connect (GTK_OBJECT (accel_dialog), "delete_event",
		      GTK_SIGNAL_FUNC (glade_util_close_window_on_delete),
		      NULL);
  gtk_signal_connect (GTK_OBJECT (accel_dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
		      GINT_TO_POINTER (GladeEscCloses));
  vbox = GTK_DIALOG (accel_dialog)->vbox;
#endif

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (vbox), vbox2, TRUE, TRUE, 0);

  /* List of current accelerators - Mods/Keys/Signals */
  accel_titles[0] = _("Mod");
  accel_titles[1] = _("Key");
  accel_titles[2] = _("Signal to emit");
  accel_clist = gtk_clist_new_with_titles (3, accel_titles);
  gtk_clist_set_column_width (GTK_CLIST (accel_clist), 0, 30);
  gtk_clist_set_column_width (GTK_CLIST (accel_clist), 1, 100);
  gtk_clist_set_column_width (GTK_CLIST (accel_clist), 2, 120);
  gtk_widget_show (accel_clist);
  gtk_signal_connect (GTK_OBJECT (accel_clist), "select_row",
		      GTK_SIGNAL_FUNC (on_accelerator_select), NULL);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), accel_clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox2), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_set_usize (scrolled_win, 320, 120);
  gtk_widget_show (scrolled_win);

  /* Mod, Key & Signal fields */
  table = gtk_table_new (3, 3, FALSE);
  gtk_widget_show (table);
  row = 0;
  label = gtk_label_new (_("Modifiers:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1,
		    GTK_FILL, 0, 1, 1);

  hbox = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox);
  accel_control_button = gtk_check_button_new_with_label (_("Ctrl"));
  gtk_widget_show (accel_control_button);
  gtk_box_pack_start (GTK_BOX (hbox), accel_control_button, TRUE, TRUE, 0);
  accel_shift_button = gtk_check_button_new_with_label (_("Shift"));
  gtk_widget_show (accel_shift_button);
  gtk_box_pack_start (GTK_BOX (hbox), accel_shift_button, TRUE, TRUE, 0);
  accel_alt_button = gtk_check_button_new_with_label (_("Alt"));
  gtk_widget_show (accel_alt_button);
  gtk_box_pack_start (GTK_BOX (hbox), accel_alt_button, TRUE, TRUE, 0);
  gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, row, row + 1,
		    GTK_EXPAND | GTK_FILL, 0, 1, 1);

  property_set_table_position (table, ++row);
  property_add_dialog (GbAccelKey, _("Key:"),
		       _("The accelerator key"), TRUE, show_keys_dialog);
  property_add_dialog (GbAccelSignal, _("Signal:"),
		       _("The signal to emit when the accelerator is pressed"),
		       TRUE, show_signals_dialog);
  gtk_box_pack_start (GTK_BOX (vbox2), table, FALSE, TRUE, 5);

  /* Add/Update/Delete buttons at bottom */
  hbox = gtk_hbox_new (TRUE, 2);
  button = gtk_button_new_with_label (_("Add"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_accelerator_add), accel_clist);

  button = gtk_button_new_with_label (_("Update"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_accelerator_update), accel_clist);

  button = gtk_button_new_with_label (_("Delete"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_accelerator_delete), accel_clist);

  button = gtk_button_new_with_label (_("Clear"));
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_accelerator_clear), accel_clist);

  gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, TRUE, 5);
  gtk_widget_show (hbox);
}


static void
on_color_draw (GtkWidget * widget, gpointer data)
{
#ifdef GLADE_STYLE_SUPPORT
  if (widget == selected_style_widget)
    {
      gdk_draw_rectangle (widget->window, widget->style->black_gc, FALSE, 0, 0,
			  widget->allocation.width - 1,
			  widget->allocation.height - 1);
      gdk_draw_rectangle (widget->window, widget->style->white_gc, FALSE, 1, 1,
			  widget->allocation.width - 3,
			  widget->allocation.height - 3);
      gdk_draw_rectangle (widget->window, widget->style->black_gc, FALSE, 2, 2,
			  widget->allocation.width - 5,
			  widget->allocation.height - 5);
    }
#endif
}


static void
on_color_expose_event (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  GdkColor *color = gtk_object_get_data (GTK_OBJECT (widget), GbColorKey);
  g_return_if_fail (color != NULL);
  gdk_window_set_background (widget->window, color);
  gdk_window_clear (widget->window);
  on_color_draw (widget, data);
}


static void
on_color_select (GtkWidget * widget, gpointer data)
{
#ifdef GLADE_STYLE_SUPPORT
  if (selected_style_widget == widget)
    return;
  if (selected_style_widget && selected_style_widget != widget)
    gtk_widget_queue_draw (selected_style_widget);
  selected_style_widget = widget;
#endif
  gtk_widget_queue_draw (widget);
}


/*
 * Creating property widgets
 */

void
property_set_table_position (GtkWidget * table, gint row)
{
  property_table = table;
  property_table_row = row;
}


GtkWidget*
property_get_table_position (gint *row)
{
  *row = property_table_row;
  return property_table;
}


void
property_add_string (const gchar * property_name,
		     const gchar * label,
		     const gchar * tooltip)
{
  GtkWidget *value = gtk_entry_new ();
  gtk_widget_set_usize (value, 80, -1);
  property_add (property_name, label, value, NULL, tooltip);
  gtk_signal_connect (GTK_OBJECT (value), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
}


void
property_add_text (const gchar * property_name,
		   const gchar * label,
		   const gchar * tooltip,
		   gint visible_lines)
{
  GtkWidget *value, *text;
  gint line_height;
  value = gtk_scrolled_window_new (NULL, NULL);
  text = gtk_text_new (NULL, NULL);
  gtk_text_set_editable (GTK_TEXT (text), TRUE);
  gtk_widget_show (text);
  gtk_container_add (GTK_CONTAINER (value), text);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (value),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  line_height = text->style->font->ascent + text->style->font->descent;
  /* We add 8 for the text's border height etc. */
  gtk_widget_set_usize (value, 80, visible_lines * line_height + 8);
  property_add (property_name, label, value, NULL, tooltip);
  gtk_signal_connect (GTK_OBJECT (text), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
}


void
property_add_int (const gchar * property_name,
		  const gchar * label,
		  const gchar * tooltip)
{
  GtkWidget *value = gtk_entry_new ();
  gtk_widget_set_usize (value, 80, -1);
  property_add (property_name, label, value, NULL, tooltip);
  gtk_signal_connect (GTK_OBJECT (value), "activate",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
}


void
property_add_int_range (const gchar * property_name,
			const gchar * label,
			const gchar * tooltip,
			gint min,
			gint max,
			gint step_increment,
			gint page_increment,
			gint climb_rate)
{
  GtkObject *adjustment = gtk_adjustment_new (min, min, max, step_increment,
					    page_increment, page_increment);
  GtkWidget *value = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment),
					  climb_rate, 0);
  gtk_widget_set_usize (value, 80, -1);
  gtk_signal_connect (GTK_OBJECT (value), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
  property_add (property_name, label, value, NULL, tooltip);
}


void
property_add_optional_int_range (const gchar * property_name,
				 const gchar * label,
				 const gchar * tooltip,
				 gint min,
				 gint max,
				 gint step_increment,
				 gint page_increment,
				 gint climb_rate,
				 GtkCallback callback)
{
  GtkObject *adjustment = gtk_adjustment_new (min, min, max, step_increment,
					    page_increment, page_increment);
  GtkWidget *value = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment),
					  climb_rate, 0);
  GtkWidget *dialog_button = gtk_check_button_new ();
  gtk_widget_set_usize (value, 50, -1);
  gtk_signal_connect (GTK_OBJECT (value), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
  gtk_signal_connect (GTK_OBJECT (dialog_button), "toggled",
		      GTK_SIGNAL_FUNC (callback), value);
  property_add (property_name, label, value, dialog_button, tooltip);
}


void
property_add_float (const gchar * property_name,
		    const gchar * label,
		    const gchar * tooltip)
{
  GtkWidget *value = gtk_entry_new ();
  gtk_widget_set_usize (value, 80, -1);
  property_add (property_name, label, value, NULL, tooltip);
  gtk_signal_connect (GTK_OBJECT (value), "activate",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
}


void
property_add_float_range (const gchar * property_name,
			  const gchar * label,
			  const gchar * tooltip,
			  gfloat min,
			  gfloat max,
			  gfloat step_increment,
			  gfloat page_increment,
			  gfloat climb_rate,
			  gint decimal_digits)
{
  GtkObject *adjustment = gtk_adjustment_new (min, min, max, step_increment,
					    page_increment, page_increment);
  GtkWidget *value = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment),
					  climb_rate, decimal_digits);
  gtk_widget_set_usize (value, 80, -1);
  gtk_signal_connect (GTK_OBJECT (value), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
  property_add (property_name, label, value, NULL, tooltip);
}


static void
on_bool_property_toggle (GtkWidget * value, gpointer data)
{
  guint active = GTK_TOGGLE_BUTTON (value)->active;
  GtkWidget *label = GTK_BUTTON (value)->child;
  gtk_label_set_text (GTK_LABEL (label), active ? _("Yes") : _("No"));
  on_property_changed (value, value);
}


void
property_add_bool (const gchar * property_name,
		   const gchar * label,
		   const gchar * tooltip)
{
  GtkWidget *value = gtk_toggle_button_new_with_label (_("No"));
  gtk_signal_connect (GTK_OBJECT (value), "toggled",
		      GTK_SIGNAL_FUNC (on_bool_property_toggle), NULL);
  property_add (property_name, label, value, NULL, tooltip);
}


void
property_add_choice (const gchar * property_name,
		     const gchar * label,
		     const gchar * tooltip,
		     const gchar ** choices)
{
  GtkWidget *value = gtk_option_menu_new ();
  GtkWidget *menu = gtk_menu_new ();

  gint i = 0;
  while (choices[i])
    {
      GtkWidget *menuitem = gtk_menu_item_new_with_label (choices[i]);
      gtk_menu_append (GTK_MENU (menu), menuitem);
      gtk_widget_show (menuitem);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_property_changed), value);
      i++;
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (value), menu);
  property_add (property_name, label, value, NULL, tooltip);
}


void
property_add_combo (const gchar * property_name,
		    const gchar * label,
		    const gchar * tooltip,
		    GList * choices)
{
  GtkWidget *value = gtk_combo_new ();
  gtk_widget_set_usize (GTK_COMBO (value)->entry, 60, -1);
  gtk_widget_set_usize (value, 80, -1);
  if (choices)
    gtk_combo_set_popdown_strings (GTK_COMBO (value), choices);
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (value)->entry), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
  property_add (property_name, label, value, NULL, tooltip);
}


void
property_add_color (const gchar * property_name,
		    const gchar * label,
		    const gchar * tooltip)
{
  GtkWidget *value, *preview, *dialog_button;
  value = gtk_frame_new (NULL);
  preview = create_color_preview ();
  gtk_widget_set_events (preview, gtk_widget_get_events (preview)
			 | GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect_after (GTK_OBJECT (preview), "expose_event",
			    GTK_SIGNAL_FUNC (on_color_expose_event), value);
  gtk_signal_connect_after (GTK_OBJECT (preview), "draw",
			    GTK_SIGNAL_FUNC (on_color_draw), value);
  gtk_signal_connect (GTK_OBJECT (preview), "button_press_event",
		      GTK_SIGNAL_FUNC (on_color_select), value);

  gtk_container_add (GTK_CONTAINER (value), preview);
  dialog_button = gtk_button_new_with_label ("...");
  gtk_signal_connect (GTK_OBJECT (dialog_button), "clicked",
		      GTK_SIGNAL_FUNC (show_colorsel_dialog), value);
  property_add (property_name, label, value, dialog_button, tooltip);
}


void
property_add_bgpixmap (const gchar * property_name,
		       const gchar * label,
		       const gchar * tooltip)
{
  GtkWidget *value, *drawing_area, *dialog_button;
  value = gtk_frame_new (NULL);
  drawing_area = gtk_drawing_area_new ();
  gtk_widget_set_events (drawing_area, gtk_widget_get_events (drawing_area)
			 | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 100, 20);
  gtk_widget_show (drawing_area);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
		      GTK_SIGNAL_FUNC (expose_pixmap), NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "draw",
		      GTK_SIGNAL_FUNC (draw_pixmap), NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "draw_default",
		      GTK_SIGNAL_FUNC (draw_pixmap_focus), NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "draw_focus",
		      GTK_SIGNAL_FUNC (draw_pixmap_focus), NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "button_press_event",
		      GTK_SIGNAL_FUNC (on_color_select), value);
  gtk_container_add (GTK_CONTAINER (value), drawing_area);
  dialog_button = gtk_button_new_with_label ("...");
  gtk_signal_connect (GTK_OBJECT (dialog_button), "clicked",
		      GTK_SIGNAL_FUNC (show_filesel_dialog), value);
  property_add (property_name, label, value, dialog_button, tooltip);
}


void
property_add_dialog (const gchar * property_name,
		     const gchar * label,
		     const gchar * tooltip,
		     gboolean editable,
		     GtkCallback callback)
{
  GtkWidget *value, *dialog_button;
  value = gtk_entry_new ();
  gtk_widget_set_usize (value, 80, -1);
  gtk_entry_set_editable (GTK_ENTRY (value), editable);
  dialog_button = gtk_button_new_with_label ("...");
  gtk_signal_connect (GTK_OBJECT (dialog_button), "clicked",
		      GTK_SIGNAL_FUNC (callback), value);
  gtk_signal_connect (GTK_OBJECT (value), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
  property_add (property_name, label, value, dialog_button, tooltip);
}


void
property_add_filename (const gchar * property_name,
		       const gchar * label,
		       const gchar * tooltip)
{
  GtkWidget *value, *dialog_button;
  value = gtk_entry_new ();
  gtk_widget_set_usize (value, 80, -1);
  gtk_entry_set_editable (GTK_ENTRY (value), FALSE);
  dialog_button = gtk_button_new_with_label ("...");
  gtk_signal_connect (GTK_OBJECT (dialog_button), "clicked",
		      GTK_SIGNAL_FUNC (show_filesel_dialog), value);
  property_add (property_name, label, value, dialog_button, tooltip);
}


void
property_add_filename_with_combo (const gchar * property_name,
				  const gchar * label,
				  const gchar * tooltip,
				  GList * choices)
{
  GtkWidget *value, *dialog_button;
  value = gtk_combo_new ();
  gtk_widget_set_usize (GTK_COMBO (value)->entry, 60, -1);
  gtk_widget_set_usize (value, 80, -1);
  if (choices)
    gtk_combo_set_popdown_strings (GTK_COMBO (value), choices);
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (value)->entry), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
  dialog_button = gtk_button_new_with_label ("...");
  gtk_signal_connect (GTK_OBJECT (dialog_button), "clicked",
		      GTK_SIGNAL_FUNC (show_filesel_dialog), value);
  property_add (property_name, label, value, dialog_button, tooltip);
}



void
property_add_font (const gchar * property_name,
		   const gchar * label,
		   const gchar * tooltip)
{
  GtkWidget *value, *dialog_button;
  value = gtk_entry_new ();
  gtk_widget_set_usize (value, 80, -1);
  gtk_entry_set_editable (GTK_ENTRY (value), FALSE);
  dialog_button = gtk_button_new_with_label ("...");
  gtk_signal_connect (GTK_OBJECT (dialog_button), "clicked",
		      GTK_SIGNAL_FUNC (show_font_dialog), value);
  gtk_signal_connect (GTK_OBJECT (value), "changed",
		      GTK_SIGNAL_FUNC (on_property_changed), value);
  property_add (property_name, label, value, dialog_button, tooltip);
}


void
property_add_command (const gchar * property_name,
		      const gchar * label,
		      const gchar * tooltip,
		      const gchar * command,
		      GtkSignalFunc callback)
{
  GtkWidget *value;
  value = gtk_button_new_with_label (command);
  if (callback)
    {
      gtk_signal_connect (GTK_OBJECT (value), "clicked",
			  GTK_SIGNAL_FUNC (callback), value);
    }
  property_add (property_name, label, value, NULL, tooltip);
}


void
property_add (const gchar * property_name,
	      const gchar * label_string,
	      GtkWidget * value,
	      GtkWidget * dialog_button,
	      const gchar * tooltip)
{
  GtkWidget *label, *eventbox;
  gchar *property_name_copy;
  gint xpad = 1, ypad = 1;

  if (glade_debug_properties)
    g_print ("Property: %s\n", property_name);
  
  if (GTK_IS_OPTION_MENU (value))
    xpad = ypad = 0;

  label = gtk_label_new (label_string);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.3);
  /*gtk_misc_set_padding(GTK_MISC(label), 0, 5); */
  gtk_widget_show (label);

  /* We put the label in the event box so we can set a tooltip. */
  eventbox = gtk_event_box_new ();
  gtk_widget_set_usize (eventbox, 100, -1);
  gtk_widget_show (eventbox);
  gtk_container_add (GTK_CONTAINER (eventbox), label);

  gtk_widget_show (value);
  if (tooltip)
    gtk_tooltips_set_tip (tooltips, eventbox, tooltip, NULL);

  gtk_table_attach (GTK_TABLE (property_table), eventbox, 0, 1,
		    property_table_row, property_table_row + 1,
		    GTK_FILL, GTK_FILL, 1, 1);
  if (dialog_button)
    {
      GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
      gtk_table_attach (GTK_TABLE (property_table), hbox, 1, 3,
			property_table_row, property_table_row + 1,
			GTK_EXPAND | GTK_FILL, 0, xpad, ypad);
      /* Empty checkbuttons are placed before the value widget. */
      if (GTK_IS_CHECK_BUTTON (dialog_button)
	  && GTK_BIN (dialog_button)->child == NULL)
	{
	  gtk_box_pack_start (GTK_BOX (hbox), dialog_button, FALSE, FALSE, 0);
	  gtk_box_pack_start (GTK_BOX (hbox), value, TRUE, TRUE, 0);
	}
      else
	{
	  gtk_box_pack_start (GTK_BOX (hbox), value, TRUE, TRUE, 0);
	  gtk_box_pack_start (GTK_BOX (hbox), dialog_button, FALSE, FALSE, 0);
	}
      gtk_widget_show (dialog_button);
      gtk_widget_show (hbox);
    }
  else
    {
      gtk_table_attach (GTK_TABLE (property_table), value, 1, 3,
			property_table_row, property_table_row + 1,
			GTK_EXPAND | GTK_FILL, 0, xpad, ypad);
    }
  property_table_row++;

  /* Insert property label & value widgets into hash tables */
  property_name_copy = g_strdup (property_name);
  g_hash_table_insert (gb_property_labels, property_name_copy, label);
  g_hash_table_insert (gb_property_values, property_name_copy, value);
  if (dialog_button)
    g_hash_table_insert (gb_property_buttons,
			 property_name_copy,
			 dialog_button);
}


/*
 * Functions for getting/setting properties.
 * NOTE: should also specify whether set properties are copied and
 *       if values returned by get should be freed.
 *       For widgets which use a GtkEntry: string value set is copied,
 *       & don't free string returned by get (it is the actual entry text).
 *
 * NOTE: property_get_*() functions also set the apply flag.
 *       This is used to enable the automatic applying of
 *       properties as they are changed in the property editor. It is set
 *       to TRUE if the property widget matches the to_apply widget.
 */


gchar *
property_get_string (const gchar * property_name,
		     GtkWidget * to_apply,
		     gboolean * apply)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, "");
  return (gtk_entry_get_text (GTK_ENTRY (widget)));
}


void
property_set_string (const gchar * property_name,
		     const gchar * value)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  gtk_entry_set_text (GTK_ENTRY (widget), value ? value : "");
}


/* Note: returned string must be freed with g_free() */
gchar *
property_get_text (const gchar * property_name,
		   GtkWidget * to_apply,
		   gboolean * apply)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  GtkWidget *text = GTK_BIN (widget)->child;
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, "");
  return (gtk_editable_get_chars (GTK_EDITABLE (text), 0, -1));
}


void
property_set_text (const gchar * property_name,
		   const gchar * value)
{
  const gchar *text_to_insert;
  GtkWidget *text;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);

  text = GTK_BIN (widget)->child;
  gtk_editable_delete_text (GTK_EDITABLE (text), 0, -1);
  text_to_insert = value ? value : "";
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL,
		   text_to_insert, strlen (text_to_insert));
}


gint
property_get_int (const gchar * property_name,
		  GtkWidget * to_apply,
		  gboolean * apply)
{
  gchar *text;
  gint value = 0;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, 0);
  if (GTK_IS_SPIN_BUTTON (widget))
    {
      return gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
    }
  else
    {
      text = gtk_entry_get_text (GTK_ENTRY (widget));
      sscanf (text, "%i", &value);
      return value;
    }
}


gint
property_get_optional_int (const gchar * property_name,
			   GtkWidget * to_apply,
			   gboolean * apply,
			   gboolean * is_set)
{
  gchar *text;
  gint value = 0;
  GtkWidget *widget, *button;

  widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
					      property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, 0);

  button = (GtkWidget *) g_hash_table_lookup (gb_property_buttons,
					      property_name);
  g_return_val_if_fail (button != NULL, 0);

  if (is_set)
    *is_set = GTK_TOGGLE_BUTTON (button)->active ? TRUE : FALSE;

  if (GTK_IS_SPIN_BUTTON (widget))
    {
      return gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
    }
  else
    {
      text = gtk_entry_get_text (GTK_ENTRY (widget));
      sscanf (text, "%i", &value);
      return value;
    }
}


void
property_set_int (const gchar * property_name,
		  gint value)
{
  gchar buffer[128];
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  if (GTK_IS_SPIN_BUTTON (widget))
    {
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), value);
    }
  else
    {
      sprintf (buffer, "%i", value);
      gtk_entry_set_text (GTK_ENTRY (widget), buffer);
    }
}


void
property_set_optional_int (const gchar * property_name,
			   gint value,
			   gboolean is_set)
{
  gchar buffer[128];
  GtkWidget *widget, *button;

  widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
					      property_name);
  g_return_if_fail (widget != NULL);

  button = (GtkWidget *) g_hash_table_lookup (gb_property_buttons,
					      property_name);
  g_return_if_fail (button != NULL);

  if (GTK_IS_SPIN_BUTTON (widget))
    {
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), value);
    }
  else
    {
      sprintf (buffer, "%i", value);
      gtk_entry_set_text (GTK_ENTRY (widget), buffer);
    }

  gtk_widget_set_sensitive (widget, is_set);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), is_set);
}


gfloat
property_get_float (const gchar * property_name,
		    GtkWidget * to_apply,
		    gboolean * apply)
{
  gchar *text;
  gfloat value = 0;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, 0);
  if (GTK_IS_SPIN_BUTTON (widget))
    {
      return gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (widget));
    }
  else
    {
      text = gtk_entry_get_text (GTK_ENTRY (widget));
      sscanf (text, "%f", &value);
      return value;
    }
}


void
property_set_float (const gchar * property_name,
		    gfloat value)
{
  gchar buffer[128];
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  if (GTK_IS_SPIN_BUTTON (widget))
    {
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), value);
    }
  else
    {
      sprintf (buffer, "%g", value);
      gtk_entry_set_text (GTK_ENTRY (widget), buffer);
    }
}


gboolean
property_get_bool (const gchar * property_name,
		   GtkWidget * to_apply,
		   gboolean * apply)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, FALSE);
  return (GTK_TOGGLE_BUTTON (widget)->active ? TRUE : FALSE);
}


void
property_set_bool (const gchar * property_name,
		   gint value)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), value);
}


gchar *
property_get_choice (const gchar * property_name,
		     GtkWidget * to_apply,
		     gboolean * apply)
{
  GtkWidget *label;
  gchar *label_text;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, NULL);
  label = GTK_BUTTON (widget)->child;
  gtk_label_get (GTK_LABEL (label), &label_text);
  return label_text;
}


void
property_set_choice (const gchar * property_name,
		     gint value)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  gtk_option_menu_set_history (GTK_OPTION_MENU (widget), value);
}


gchar *
property_get_combo (const gchar * property_name,
		    GtkWidget * to_apply,
		    gboolean * apply)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, "");
  return (gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (widget)->entry)));
}


void
property_set_combo (const gchar * property_name,
		    const gchar * value)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (widget)->entry), value ? value : "");
}


void
property_set_combo_strings (const gchar * property_name,
			    GList * choices)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  if (choices)
    gtk_combo_set_popdown_strings (GTK_COMBO (widget), choices);
  else
    gtk_list_clear_items (GTK_LIST (GTK_COMBO (widget)->list), 0, -1);
}


GdkColor *
property_get_color (const gchar * property_name,
		    GtkWidget * to_apply,
		    gboolean * apply)
{
  GdkColor *color;
  GtkWidget *preview;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, NULL);
  preview = GTK_BIN (widget)->child;
  g_return_val_if_fail (GTK_IS_DRAWING_AREA (preview), NULL);
  color = gtk_object_get_data (GTK_OBJECT (preview), GbColorKey);
  g_return_val_if_fail (color != NULL, NULL);
  return color;
}


void
property_set_color (const gchar * property_name,
		    GdkColor * value)
{
  GtkWidget *preview;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  preview = GTK_BIN (widget)->child;
  g_return_if_fail (GTK_IS_DRAWING_AREA (preview));
  show_color_in_preview (preview, value);
}


GdkPixmap *
property_get_bgpixmap (const gchar * property_name,
		       GtkWidget * to_apply,
		       gboolean * apply,
		       gchar ** filename)
{
  GtkWidget *drawing_area;
  GdkPixmap *gdkpixmap;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, NULL);
  drawing_area = GTK_BIN (widget)->child;
  g_return_val_if_fail (GTK_IS_DRAWING_AREA (drawing_area), NULL);

  gdkpixmap = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgPixmapKey);
  *filename = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgFilenameKey);
  return gdkpixmap;
}


void
property_set_bgpixmap (const gchar * property_name,
		       GdkPixmap * gdkpixmap,
		       const gchar * filename)
{
  GtkWidget *drawing_area;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  drawing_area = GTK_BIN (widget)->child;
  g_return_if_fail (GTK_IS_DRAWING_AREA (drawing_area));

  set_pixmap (drawing_area, gdkpixmap, filename);
}


static void
set_pixmap (GtkWidget * drawing_area, GdkPixmap * gdkpixmap,
	    const gchar * filename)
{
  GdkPixmap *old_gdkpixmap;
  gchar *old_filename, *filename_copy;

  /* free/unref any existing values */
  old_gdkpixmap = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgPixmapKey);
  if (old_gdkpixmap)
    gdk_pixmap_unref (old_gdkpixmap);
  old_filename = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgFilenameKey);
  g_free (old_filename);

  gtk_object_set_data (GTK_OBJECT (drawing_area), GbBgPixmapKey, gdkpixmap);
  filename_copy = filename ? g_strdup (filename) : NULL;
  gtk_object_set_data (GTK_OBJECT (drawing_area), GbBgFilenameKey, filename_copy);

  if (gdkpixmap)
    gdk_pixmap_ref (gdkpixmap);

  show_pixmap_in_drawing_area (drawing_area, gdkpixmap);
}


gpointer
property_get_dialog (const gchar * property_name,
		     GtkWidget * to_apply,
		     gboolean * apply)
{
  gpointer dialog_value;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, NULL);
  dialog_value = gtk_object_get_data (GTK_OBJECT (widget), GbDialogValueKey);
  if (dialog_value)
    return dialog_value;
  else
    return (gtk_entry_get_text (GTK_ENTRY (widget)));
}


void
property_set_dialog (const gchar * property_name,
		     const gchar * string,
		     gconstpointer value)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  if (value)
    gtk_object_set_data (GTK_OBJECT (widget), GbDialogValueKey,
			 (gpointer) value);
  gtk_entry_set_text (GTK_ENTRY (widget), string ? string : "");
}


gchar *
property_get_filename (const gchar * property_name,
		       GtkWidget * to_apply,
		       gboolean * apply)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, "");
  return gtk_object_get_data (GTK_OBJECT (widget), GbFilenameValueKey);
}


void
property_set_filename (const gchar * property_name,
		       const gchar * value)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);
  g_free (gtk_object_get_data (GTK_OBJECT (widget), GbFilenameValueKey));
  gtk_object_set_data (GTK_OBJECT (widget), GbFilenameValueKey,
		       g_strdup (value));
  if (GTK_IS_ENTRY (widget))
    gtk_entry_set_text (GTK_ENTRY (widget), value ? g_basename (value) : "");
  else if (GTK_IS_COMBO (widget))
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (widget)->entry),
			value ? g_basename (value) : "");
  else
    g_warning ("Invalid filename property");
}


GdkFont *
property_get_font (const gchar * property_name,
		   GtkWidget * to_apply,
		   gboolean * apply,
		   gchar ** xlfd_fontname)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  if (apply)
    *apply = (!to_apply || to_apply == widget) ? TRUE : FALSE;
  g_return_val_if_fail (widget != NULL, NULL);
  *xlfd_fontname = gtk_object_get_data (GTK_OBJECT (widget), GbFontSpecKey);
  return (gtk_object_get_data (GTK_OBJECT (widget), GbFontKey));
}


void
property_set_font (const gchar * property_name,
		   GdkFont * font,
		   const gchar * xlfd_fontname)
{
  GdkFont *old_font;
  gchar *old_xlfd_fontname;
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);

  old_font = gtk_object_get_data (GTK_OBJECT (widget), GbFontKey);
  if (old_font)
    gdk_font_unref (old_font);
  if (font)
    gdk_font_ref (font);
  gtk_object_set_data (GTK_OBJECT (widget), GbFontKey, font);
  old_xlfd_fontname = gtk_object_get_data (GTK_OBJECT (widget), GbFontSpecKey);
  g_free (old_xlfd_fontname);
  gtk_object_set_data (GTK_OBJECT (widget), GbFontSpecKey, g_strdup (xlfd_fontname));
  gtk_entry_set_text (GTK_ENTRY (widget), get_font_name_from_spec (xlfd_fontname));
}


/*
 * For getting the widgets used to display properties.
 */
GtkWidget*
property_get_value_widget (const gchar *property_name)
{
  return (GtkWidget *) g_hash_table_lookup (gb_property_values, property_name);
}


/*
 * Setting properties sensitive/insensitive
 */
void
property_set_sensitive (const gchar * property_name, gboolean sensitive)
{
  property_set_sensitive_full (property_name, sensitive, sensitive, TRUE);
}

void
property_set_sensitive_full (const gchar * property_name,
			     gboolean label_sensitive,
			     gboolean value_sensitive,
			     gboolean button_visible)
{
  GtkWidget *value, *label, *button;

  value = (GtkWidget *) g_hash_table_lookup (gb_property_values,
					     property_name);
  g_return_if_fail (value != NULL);
  gtk_widget_set_sensitive (value, value_sensitive);

  label = (GtkWidget *) g_hash_table_lookup (gb_property_labels,
					     property_name);
  if (label)
    gtk_widget_set_sensitive (label, label_sensitive);

  button = (GtkWidget *) g_hash_table_lookup (gb_property_buttons,
					      property_name);
  if (button)
    {
      if (button_visible)
	{
	  gtk_widget_show (button);
	  if (GTK_IS_TOGGLE_BUTTON (button))
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
					  value_sensitive);
	  else
	    gtk_widget_set_sensitive (button, value_sensitive);
	}
      else
	gtk_widget_hide (button);
    }
}


void
property_set_visible (const gchar * property_name,
		      gboolean visible)
{
  GtkWidget *value, *label, *button;

  value = (GtkWidget *) g_hash_table_lookup (gb_property_values,
					     property_name);
  g_return_if_fail (value != NULL);
  if (visible)
    gtk_widget_show (value);
  else
    gtk_widget_hide (value);


  label = (GtkWidget *) g_hash_table_lookup (gb_property_labels,
					     property_name);
  if (label)
    {
      if (visible)
	gtk_widget_show (label);
      else
	gtk_widget_hide (label);
    }

  button = (GtkWidget *) g_hash_table_lookup (gb_property_buttons,
					      property_name);
  if (button)
    {
      if (visible)
	gtk_widget_show (button);
      else
	gtk_widget_hide (button);
    }

  /* Hide the parent of the value/button if it isn't the main table.
     We need this since sometimes the table doesn't resize properly, and we
     are left with a blank row. */
  if (value->parent && !GTK_IS_TABLE (value->parent))
    {
      if (visible)
	gtk_widget_show (value->parent);
      else
	gtk_widget_hide (value->parent);
    }
}


/*
 * Setting properties valid/invalid
 */
void
property_set_valid (const gchar * property_name, gboolean valid)
{
  GtkWidget *widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
							 property_name);
  g_return_if_fail (widget != NULL);

  if (valid)
    gtk_widget_set_style (widget, gtk_widget_get_default_style ());
  else
    gtk_widget_set_style (widget, invalid_style);
}


/*
 * Color previews - for showing style colors
 */

static GtkWidget *
create_color_preview ()
{
  GtkWidget *preview;
  GdkColormap *colormap;
  GdkColor *color;

  preview = gtk_drawing_area_new ();
  gtk_widget_show (preview);

  color = g_new (GdkColor, 1);
  color->red = color->green = color->blue = 0xFFFF;
  gtk_object_set_data (GTK_OBJECT (preview), GbColorKey, color);

  /* Allocate the color. */
  colormap = gtk_widget_get_colormap (preview);
  if (!gdk_color_alloc (colormap, color))
    {
      g_warning ("Couldn't allocate white color");
    }

  return preview;
}


/* Returns TRUE if the color has changed. */
static gboolean
show_color_in_preview (GtkWidget * preview, GdkColor *new_color)
{
  GdkColormap *colormap;
  GdkColor *color;

  color = gtk_object_get_data (GTK_OBJECT (preview), GbColorKey);
  g_return_val_if_fail (color != NULL, FALSE);

  /* If it is the same, return FALSE. */
  if (color->red == new_color->red
      && color->green == new_color->green
      && color->blue == new_color->blue)
    return FALSE;

  /* Allocate the color. */
  colormap = gtk_widget_get_colormap (preview);
  if (!gdk_color_alloc (colormap, new_color))
    {
      /* If we can't allocate the colour, keep the old one. */
      g_warning ("Couldn't allocate color");
      return FALSE;
    }

  /* Free the old color. */
  gdk_colors_free (colormap, &color->pixel, 1, 0);

  *color = *new_color;

  /* The drawing area doesn't get a window until the notebook page is shown! */
  if (preview->window)
    {
      gdk_window_set_background (preview->window, color);
      gdk_window_clear (preview->window);
    }

  return TRUE;
}


/*
 * Pixmap values (displayed as background of drawing area)
 */
static void
show_pixmap_in_drawing_area (GtkWidget * drawing_area, GdkPixmap * gdkpixmap)
{
  g_return_if_fail (GTK_IS_DRAWING_AREA (drawing_area));
  /* The drawing area doesn't get a window until the notebook page is shown! */
  if (drawing_area->window == NULL)
    return;
  if (gdkpixmap)
    {
      gdk_window_set_back_pixmap (drawing_area->window, gdkpixmap, FALSE);
    }
  else
    {
      gdk_window_set_background (drawing_area->window,
				 &drawing_area->style->bg[GTK_STATE_NORMAL]);
    }
  gdk_window_clear (drawing_area->window);
}


static gint
expose_pixmap (GtkWidget * drawing_area, GdkEventExpose * event, gpointer data)
{
  real_draw_pixmap (drawing_area);
  return FALSE;
}


static void
draw_pixmap (GtkWidget * drawing_area, GdkRectangle * area, gpointer data)
{
  real_draw_pixmap (drawing_area);
}


static void
draw_pixmap_focus (GtkWidget * drawing_area, gpointer data)
{
  real_draw_pixmap (drawing_area);
}


static void
real_draw_pixmap (GtkWidget * drawing_area)
{
  GdkPixmap *gdkpixmap;
  g_return_if_fail (GTK_IS_DRAWING_AREA (drawing_area));
  gdkpixmap = gtk_object_get_data (GTK_OBJECT (drawing_area), GbBgPixmapKey);
  show_pixmap_in_drawing_area (drawing_area, gdkpixmap);
  on_color_draw (drawing_area, NULL);
}


/*
 * The Events dialog, used for selecting which X events to receive.
 */

static void
show_events_dialog (GtkWidget * widget, gpointer value)
{
  GtkDialog *dialog;
  GtkWidget *clist, *button, *scrolled_win, *hbbox;
  GtkWidget *transient_parent;
  gchar *titles[2];
  const gchar *row[2];
  int i;
  gchar *event_mask_string;
  gint event_mask_value;

  dialog = GTK_DIALOG (gtk_dialog_new ());
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Select X Events"));
  gtk_window_set_wmclass (GTK_WINDOW (dialog), "select_events", "Glade");
  transient_parent = glade_util_get_toplevel (widget);
  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (dialog),
				  GTK_WINDOW (transient_parent));

  titles[0] = _("Event Mask");
  titles[1] = _("Description");
  clist = gtk_clist_new_with_titles (2, titles);
  gtk_clist_column_titles_passive (GTK_CLIST (clist));
  gtk_clist_set_column_width (GTK_CLIST (clist), 0, 230);
  gtk_clist_set_column_width (GTK_CLIST (clist), 1, 100);
  gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_MULTIPLE);
  gtk_widget_set_usize (clist, 500, 350);
  gtk_widget_show (clist);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (dialog->vbox), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_win);

  /* Insert events & descriptions */
  gtk_clist_freeze (GTK_CLIST (clist));
  for (i = 0; i < GB_EVENT_MASKS_COUNT; i++)
    {
      row[0] = GbEventMaskSymbols[i];
      row[1] = _(GbEventMaskDescriptions[i]);
      gtk_clist_append (GTK_CLIST (clist), (gchar**)row);
    }

  /* Select rows according to current mask setting */
  event_mask_string = gtk_entry_get_text (GTK_ENTRY (value));
  event_mask_value = property_events_string_to_value (event_mask_string);
  for (i = 0; i < GB_EVENT_MASKS_COUNT; i++)
    {
      if (event_mask_value & GbEventMaskValues[i])
	{
	  gtk_clist_select_row (GTK_CLIST (clist), i, 0);
	}
    }
  gtk_clist_thaw (GTK_CLIST (clist));

  /* Buttons at bottom */
  hbbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX (dialog->action_area), hbbox, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbbox), 8);
  gtk_widget_show (hbbox);

#ifdef USE_GNOME
  button = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
#else
  button = gtk_button_new_with_label (_("OK"));
#endif
  gtk_container_add (GTK_CONTAINER (hbbox), button);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  /* Save pointer to value to use when OK pressed */
  gtk_object_set_data (GTK_OBJECT (clist), GbValueWidgetKey, value);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_events_dialog_ok), clist);

#ifdef USE_GNOME
  button = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
#else
  button = gtk_button_new_with_label (_("Cancel"));
#endif
  gtk_container_add (GTK_CONTAINER (hbbox), button);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
		      GINT_TO_POINTER (GladeEscDestroys));
  gtk_widget_show (GTK_WIDGET (dialog));
}


static void
on_events_dialog_ok (GtkWidget * widget, GtkWidget * clist)
{
  gint row, mask_value = 0;
  GtkWidget *value;
  GList *selection = GTK_CLIST (clist)->selection;

  while (selection)
    {
      row = GPOINTER_TO_INT (selection->data);
      mask_value |= GbEventMaskValues[row];
      selection = selection->next;
    }

  value = gtk_object_get_data (GTK_OBJECT (clist), GbValueWidgetKey);
  g_return_if_fail (value != NULL);
  gtk_entry_set_text (GTK_ENTRY (value),
		      property_events_value_to_string (mask_value));
  gtk_widget_destroy (gtk_widget_get_toplevel (clist));
}


/* Converts the events gint to a string of 0s and 1s for displaying */
gchar *
property_events_value_to_string (gint event_mask)
{
  static gchar buf[GB_EVENT_MASKS_COUNT + 2];
  int i;

  for (i = 0; i < GB_EVENT_MASKS_COUNT; i++)
    {
      if (event_mask & GbEventMaskValues[i])
	buf[GB_EVENT_MASKS_COUNT - i - 1] = '1';
      else
	buf[GB_EVENT_MASKS_COUNT - i - 1] = '0';
    }
  buf[GB_EVENT_MASKS_COUNT] = '0';
  buf[GB_EVENT_MASKS_COUNT + 1] = '\0';
  return buf;
}


/* Converts the string of 0s and 1s back to a gint event mask */
gint
property_events_string_to_value (const gchar * event_string)
{
  gint i, value = 0;

  if (strlen (event_string) < GB_EVENT_MASKS_COUNT)
    return 0;
  for (i = 0; i < GB_EVENT_MASKS_COUNT; i++)
    {
      if (event_string[GB_EVENT_MASKS_COUNT - i - 1] == '1')
	value |= GbEventMaskValues[i];
    }
  return value;
}


/*
 * The Accelerators page
 */

static void
on_accelerator_add (GtkWidget * widget, GtkWidget * clist)
{
  gchar modifiers[4];
  gchar *row[3];
  gchar *key, *signal;

  key = property_get_string (GbAccelKey, NULL, NULL);
  if (strlen (key) == 0)
    {
      glade_util_show_message_box (_("You need to set the accelerator key"),
				   widget);
      return;
    }
  signal = property_get_string (GbAccelSignal, NULL, NULL);
  if (strlen (signal) == 0)
    {
      glade_util_show_message_box (_("You need to set the signal to emit"),
				   widget);
      return;
    }

  modifiers[0] = modifiers[1] = modifiers[2] = ' ';
  modifiers[3] = '\0';
  if (GTK_TOGGLE_BUTTON (accel_control_button)->active)
    modifiers[0] = 'C';
  if (GTK_TOGGLE_BUTTON (accel_shift_button)->active)
    modifiers[1] = 'S';
  if (GTK_TOGGLE_BUTTON (accel_alt_button)->active)
    modifiers[2] = 'A';

  row[ACCEL_MODIFIERS_COL] = modifiers;
  row[ACCEL_KEY_COL] = key;
  row[ACCEL_SIGNAL_COL] = signal;
  gtk_clist_append (GTK_CLIST (clist), row);

  /* clear the key & signal fields */
  property_set_string (GbAccelKey, "");
  property_set_string (GbAccelSignal, "");

  on_property_changed (clist, clist);
}


static void
on_accelerator_update (GtkWidget * widget, GtkWidget * clist)
{
  gchar modifiers[4];
  gchar *key, *signal;
  GList *selection = GTK_CLIST (clist)->selection;
  gint row;

  if (!selection)
    return;
  row = GPOINTER_TO_INT (selection->data);

  key = property_get_string (GbAccelKey, NULL, NULL);
  if (strlen (key) == 0)
    {
      glade_util_show_message_box (_("You need to set the accelerator key"),
				   widget);
      return;
    }
  signal = property_get_string (GbAccelSignal, NULL, NULL);
  if (strlen (signal) == 0)
    {
      glade_util_show_message_box (_("You need to set the signal to emit"),
				   widget);
      return;
    }

  modifiers[0] = modifiers[1] = modifiers[2] = ' ';
  modifiers[3] = '\0';
  if (GTK_TOGGLE_BUTTON (accel_control_button)->active)
    modifiers[0] = 'C';
  if (GTK_TOGGLE_BUTTON (accel_shift_button)->active)
    modifiers[1] = 'S';
  if (GTK_TOGGLE_BUTTON (accel_alt_button)->active)
    modifiers[2] = 'A';

  gtk_clist_set_text (GTK_CLIST (clist), row, ACCEL_MODIFIERS_COL, modifiers);
  gtk_clist_set_text (GTK_CLIST (clist), row, ACCEL_KEY_COL, key);
  gtk_clist_set_text (GTK_CLIST (clist), row, ACCEL_SIGNAL_COL, signal);

  on_property_changed (clist, clist);
}


static void
on_accelerator_delete (GtkWidget * widget, GtkWidget * clist)
{
  GList *selection = GTK_CLIST (clist)->selection;
  gint row;

  if (!selection)
    return;
  row = GPOINTER_TO_INT (selection->data);
  gtk_clist_remove (GTK_CLIST (clist), row);
  /* clear the key & signal fields */
  property_set_string (GbAccelKey, "");
  property_set_string (GbAccelSignal, "");

  on_property_changed (clist, clist);
}


static void
on_accelerator_clear (GtkWidget * widget, GtkWidget * clist)
{
  property_set_string (GbAccelKey, "");
  property_set_string (GbAccelSignal, "");

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (accel_control_button), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (accel_shift_button), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (accel_alt_button), FALSE);
}


static void
on_accelerator_select (GtkWidget * clist, gint row, gint column,
		       GdkEventButton * event, gpointer user_data)
{
  gchar *modifiers, *key, *signal;
  gint len;

  gtk_clist_get_text (GTK_CLIST (clist), row, ACCEL_MODIFIERS_COL, &modifiers);
  gtk_clist_get_text (GTK_CLIST (clist), row, ACCEL_KEY_COL, &key);
  gtk_clist_get_text (GTK_CLIST (clist), row, ACCEL_SIGNAL_COL, &signal);

  len = strlen (modifiers);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (accel_control_button),
				len >= 1 && modifiers[0] != ' ');
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (accel_shift_button),
				len >= 2 && modifiers[1] != ' ');
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (accel_alt_button),
				len >= 3 && modifiers[2] != ' ');

  property_set_string (GbAccelKey, key);
  property_set_string (GbAccelSignal, signal);
}


void
property_clear_accelerators ()
{
  gtk_clist_clear (GTK_CLIST (accel_clist));
  /* clear the key & signal fields */
  property_set_string (GbAccelKey, "");
  property_set_string (GbAccelSignal, "");
}


void
property_add_accelerator (GladeAccelerator * accel)
{
  gchar modifiers[4];
  gchar *row[3];

  modifiers[0] = modifiers[1] = modifiers[2] = ' ';
  modifiers[3] = '\0';
  if (accel->modifiers & GDK_CONTROL_MASK)
    modifiers[0] = 'C';
  if (accel->modifiers & GDK_SHIFT_MASK)
    modifiers[1] = 'S';
  /* The Alt key uses GDK_MOD1_MASK */
  if (accel->modifiers & GDK_MOD1_MASK)
    modifiers[2] = 'A';

  row[ACCEL_MODIFIERS_COL] = modifiers;
  row[ACCEL_KEY_COL] = accel->key;
  row[ACCEL_SIGNAL_COL] = accel->signal;
  gtk_clist_append (GTK_CLIST (accel_clist), row);
}


gboolean
property_is_accel_clist (GtkWidget * widget)
{
  return (widget == accel_clist) ? TRUE : FALSE;
}


GList *
property_get_accelerators ()
{
  gint row, len;
  GList *accelerators = NULL;
  GladeAccelerator *accel;
  gchar *modifiers, *key, *signal;

  for (row = 0; row < GTK_CLIST (accel_clist)->rows; row++)
    {
      accel = g_new (GladeAccelerator, 1);

      gtk_clist_get_text (GTK_CLIST (accel_clist), row,
			  ACCEL_MODIFIERS_COL, &modifiers);
      gtk_clist_get_text (GTK_CLIST (accel_clist), row,
			  ACCEL_KEY_COL, &key);
      gtk_clist_get_text (GTK_CLIST (accel_clist), row,
			  ACCEL_SIGNAL_COL, &signal);

      len = strlen (modifiers);
      accel->modifiers = 0;
      if (len >= 1 && modifiers[0] != ' ')
	accel->modifiers |= GDK_CONTROL_MASK;
      if (len >= 2 && modifiers[1] != ' ')
	accel->modifiers |= GDK_SHIFT_MASK;
      if (len >= 3 && modifiers[2] != ' ')
	accel->modifiers |= GDK_MOD1_MASK;

      accel->key = g_strdup (key);
      accel->signal = g_strdup (signal);
      accelerators = g_list_append (accelerators, accel);
    }
  return accelerators;
}


/*
 * The Accelerator Keys dialog for selecting an accelerator key.
 */

static void
show_keys_dialog (GtkWidget * widget, gpointer value)
{
  GladeKeysDialog *dialog;
  GtkWidget *transient_parent;
  
  dialog = GLADE_KEYS_DIALOG (glade_keys_dialog_new ());
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Select Accelerator Key"));
  gtk_window_set_wmclass (GTK_WINDOW (dialog), "accelerator_key", "Glade");
  transient_parent = glade_util_get_toplevel (widget);
  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (dialog),
				  GTK_WINDOW (transient_parent));
  /* Save pointer to value to use when OK pressed */
  gtk_object_set_data (GTK_OBJECT (dialog), GbValueWidgetKey, value);
  gtk_signal_connect_object (GTK_OBJECT (dialog), "delete_event",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));
  gtk_signal_connect (GTK_OBJECT (dialog->clist), "select_row",
		      GTK_SIGNAL_FUNC (on_keys_clist_select), dialog);
  gtk_signal_connect (GTK_OBJECT (dialog->ok_button), "clicked",
		      GTK_SIGNAL_FUNC (on_keys_dialog_ok), dialog);
  gtk_signal_connect_object (GTK_OBJECT (dialog->cancel_button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
		      GINT_TO_POINTER (GladeEscDestroys));
  gtk_widget_show (GTK_WIDGET (dialog));
}


static void
on_keys_dialog_ok (GtkWidget * widget, gpointer data)
{
  GladeKeysDialog *dialog;
  GtkWidget *value;
  gchar *key_symbol;

  dialog = (GladeKeysDialog*) gtk_widget_get_toplevel (widget);
  key_symbol = glade_keys_dialog_get_key_symbol (dialog);
  if (key_symbol)
    {
      value = gtk_object_get_data (GTK_OBJECT (dialog), GbValueWidgetKey);
      g_return_if_fail (value != NULL);
      gtk_entry_set_text (GTK_ENTRY (value), key_symbol);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}


static void
on_keys_clist_select (GtkWidget * widget, gint row, gint column,
		      GdkEventButton * bevent, gpointer data)
{
  if (bevent && bevent->type == GDK_2BUTTON_PRESS)
    on_keys_dialog_ok (widget, NULL);
}


/*
 * The Signals page
 */

static void
on_signal_add (GtkWidget * widget, GtkWidget * clist)
{
  gchar *row_data[5];
  gchar *signal, *handler, *object, *data;
  gboolean after;
  gint row;
  time_t *last_mod_time;

  signal = property_get_string (GbSignalName, NULL, NULL);
  if (strlen (signal) == 0)
    {
      glade_util_show_message_box (_("You need to set the signal name"),
				   widget);
      return;
    }
  handler = property_get_combo (GbSignalHandler, NULL, NULL);
  if (strlen (handler) == 0)
    {
      glade_util_show_message_box (_("You need to set the handler for the signal"), widget);
      return;
    }
  object = property_get_string (GbSignalObject, NULL, NULL);
  after = property_get_bool (GbSignalAfter, NULL, NULL);
  data = property_get_string (GbSignalData, NULL, NULL);

  row_data[SIGNAL_NAME_COL] = signal;
  row_data[SIGNAL_HANDLER_COL] = handler;
  row_data[SIGNAL_OBJECT_COL] = object;
  row_data[SIGNAL_AFTER_COL] = after ? "Y" : "";
  row_data[SIGNAL_DATA_COL] = data;
  row = gtk_clist_append (GTK_CLIST (clist), row_data);

  /* Set the last modification time to the current time. */
  last_mod_time = g_chunk_new (time_t, signal_mem_chunk);
  *last_mod_time = time (NULL);
  if (*last_mod_time == (time_t) -1)
    g_warning ("Error getting current time");
  gtk_clist_set_row_data (GTK_CLIST (signal_clist), row, last_mod_time);

  /* clear the fields */
  property_set_string (GbSignalName, "");
  property_set_combo (GbSignalHandler, "");
  property_set_string (GbSignalObject, "");
  property_set_string (GbSignalData, "");
  property_set_bool (GbSignalAfter, FALSE);

  on_property_changed (clist, clist);
}


static void
on_signal_update (GtkWidget * widget, GtkWidget * clist)
{
  gchar *signal, *handler, *object, *data;
  gchar *old_signal, *old_handler, *old_data;
  gboolean after;
  GList *selection = GTK_CLIST (clist)->selection;
  gint row;
  time_t *last_mod_time;

  if (!selection)
    return;
  row = GPOINTER_TO_INT (selection->data);

  signal = property_get_string (GbSignalName, NULL, NULL);
  if (strlen (signal) == 0)
    {
      glade_util_show_message_box (_("You need to set the signal name"),
				   widget);
      return;
    }
  handler = property_get_combo (GbSignalHandler, NULL, NULL);
  if (strlen (handler) == 0)
    {
      glade_util_show_message_box (_("You need to set the handler for the signal"), widget);
      return;
    }
  object = property_get_string (GbSignalObject, NULL, NULL);
  after = property_get_bool (GbSignalAfter, NULL, NULL);
  data = property_get_string (GbSignalData, NULL, NULL);

  /* We update the last_mod_time if the signal, handler or data have changed.*/
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_NAME_COL, &old_signal);
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_HANDLER_COL, &old_handler);
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_DATA_COL, &old_data);
  if (strcmp (signal, old_signal) || strcmp (handler, old_handler)
      || strcmp (data, old_data))
    {
      last_mod_time = (time_t*) gtk_clist_get_row_data (GTK_CLIST (signal_clist), row);
      *last_mod_time = time (NULL);
      if (*last_mod_time == (time_t) -1)
	g_warning ("Error getting current time");
    }

  gtk_clist_set_text (GTK_CLIST (clist), row, SIGNAL_NAME_COL, signal);
  gtk_clist_set_text (GTK_CLIST (clist), row, SIGNAL_HANDLER_COL, handler);
  gtk_clist_set_text (GTK_CLIST (clist), row, SIGNAL_OBJECT_COL, object);
  gtk_clist_set_text (GTK_CLIST (clist), row, SIGNAL_AFTER_COL,
		      after ? "Y" : "");
  gtk_clist_set_text (GTK_CLIST (clist), row, SIGNAL_DATA_COL, data);

  on_property_changed (clist, clist);
}


static void
on_signal_delete (GtkWidget * widget, GtkWidget * clist)
{
  GList *selection = GTK_CLIST (clist)->selection;
  gint row;

  if (!selection)
    return;
  row = GPOINTER_TO_INT (selection->data);
  gtk_clist_remove (GTK_CLIST (clist), row);
  /* clear the key & signal fields */
  property_set_string (GbSignalName, "");
  property_set_combo (GbSignalHandler, "");
  property_set_string (GbSignalObject, "");
  property_set_string (GbSignalData, "");

  on_property_changed (clist, clist);
}


static void
on_signal_clear (GtkWidget * widget, GtkWidget * clist)
{
  property_set_string (GbSignalName, "");
  property_set_combo (GbSignalHandler, "");
  property_set_string (GbSignalObject, "");
  property_set_string (GbSignalData, "");
  property_set_bool (GbSignalAfter, FALSE);
}


static void
on_signal_select (GtkWidget * clist,
		  gint row,
		  gint column,
		  GdkEventButton * event,
		  gpointer user_data)
{
  gchar *signal, *handler, *object, *after, *data;
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_NAME_COL, &signal);
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_HANDLER_COL, &handler);
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_OBJECT_COL, &object);
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_AFTER_COL, &after);
  gtk_clist_get_text (GTK_CLIST (clist), row, SIGNAL_DATA_COL, &data);
  property_set_string (GbSignalName, signal);
  property_set_combo (GbSignalHandler, handler);
  property_set_string (GbSignalObject, object);
  if (!strcmp (after, "Y"))
    property_set_bool (GbSignalAfter, TRUE);
  else
    property_set_bool (GbSignalAfter, FALSE);
  property_set_string (GbSignalData, data);
}


void
property_clear_signals ()
{
  gtk_clist_clear (GTK_CLIST (signal_clist));
  /* clear the fields */
  property_set_string (GbSignalName, "");
  property_set_combo (GbSignalHandler, "");
  property_set_string (GbSignalObject, "");
  property_set_string (GbSignalData, "");

  g_mem_chunk_reset (signal_mem_chunk);
}


void
property_add_signal (GladeSignal * signal)
{
  gchar *row_data[5];
  time_t *last_mod_time;
  gint row;

  row_data[SIGNAL_NAME_COL] = signal->name ? signal->name : "";
  row_data[SIGNAL_HANDLER_COL] = signal->handler ? signal->handler : "";
  row_data[SIGNAL_OBJECT_COL] = signal->object ? signal->object : "";
  row_data[SIGNAL_AFTER_COL] = signal->after ? "Y" : "";
  row_data[SIGNAL_DATA_COL] = signal->data ? signal->data : "";
  row = gtk_clist_append (GTK_CLIST (signal_clist), row_data);

  last_mod_time = g_chunk_new (time_t, signal_mem_chunk);
  *last_mod_time = signal->last_modification_time;
  gtk_clist_set_row_data (GTK_CLIST (signal_clist), row, last_mod_time);
}


gboolean
property_is_signal_clist (GtkWidget * widget)
{
  return (widget == signal_clist) ? TRUE : FALSE;
}


GList *
property_get_signals ()
{
  gint row;
  GList *signals = NULL;
  GladeSignal *signal;
  gchar *name, *handler, *object, *after, *data;
  time_t *time;

  for (row = 0; row < GTK_CLIST (signal_clist)->rows; row++)
    {
      signal = g_new (GladeSignal, 1);

      gtk_clist_get_text (GTK_CLIST (signal_clist), row,
			  SIGNAL_NAME_COL, &name);
      gtk_clist_get_text (GTK_CLIST (signal_clist), row,
			  SIGNAL_HANDLER_COL, &handler);
      gtk_clist_get_text (GTK_CLIST (signal_clist), row,
			  SIGNAL_OBJECT_COL, &object);
      gtk_clist_get_text (GTK_CLIST (signal_clist), row,
			  SIGNAL_AFTER_COL, &after);
      gtk_clist_get_text (GTK_CLIST (signal_clist), row,
			  SIGNAL_DATA_COL, &data);
      time = gtk_clist_get_row_data (GTK_CLIST (signal_clist), row);

      signal->name = strlen (name) > 0 ? g_strdup (name) : NULL;
      signal->handler = strlen (handler) > 0 ? g_strdup (handler) : NULL;
      signal->object = strlen (object) > 0 ? g_strdup (object) : NULL;
      signal->after = !strcmp (after, "Y") ? TRUE : FALSE;
      signal->data = strlen (data) > 0 ? g_strdup (data) : NULL;
      signal->last_modification_time = *time;
      signals = g_list_append (signals, signal);
    }
  return signals;
}


/*
 * The Signals dialog box for selecting a signal to handle.
 */

static void
show_signals_dialog (GtkWidget * widget, gpointer value)
{
  GtkDialog *dialog;
  GtkWidget *clist, *button, *scrolled_win;
  GtkWidget *transient_parent;
  gint row, i;
  gchar *current_signal, *name;
  gchar *titles[1];
  guint *signals;
  guint nsignals;
  GtkSignalQuery *query_info;
  guint type;
  gpointer class;
  GdkColor *inactive_fg, *inactive_bg;
  GList *items, *elem;

  if (!property_widget)
    return;

  dialog = GTK_DIALOG (gtk_dialog_new ());
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);

  gtk_signal_connect_object (GTK_OBJECT (dialog), "delete_event",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));

  gtk_window_set_title (GTK_WINDOW (dialog), _("Select Signal"));
  gtk_window_set_wmclass (GTK_WINDOW (dialog), "select_signal", "Glade");
  transient_parent = glade_util_get_toplevel (widget);
  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (dialog),
				  GTK_WINDOW (transient_parent));

  titles[0] = _("Signals");
  clist = gtk_clist_new_with_titles (1, titles);
  gtk_clist_column_titles_passive (GTK_CLIST (clist));
  gtk_widget_set_usize (clist, 240, 260);
  gtk_widget_show (clist);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (dialog->vbox), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show(scrolled_win);

  /* Insert list of signals. */
  gtk_clist_freeze (GTK_CLIST (clist));

  current_signal = gtk_entry_get_text (GTK_ENTRY (value));

  /* For custom widgets we only allow standard widget signals to be set. */
  if (GLADE_IS_CUSTOM_WIDGET (property_widget))
    type = gtk_widget_get_type ();
  else
    type = GTK_OBJECT_TYPE (property_widget);

  row = 0;
  inactive_fg = &widget->style->fg[GTK_STATE_INSENSITIVE];
  inactive_bg = &widget->style->bg[GTK_STATE_INSENSITIVE];
  while (type)
    {
      class = gtk_type_class (type);
      signals = GTK_OBJECT_CLASS (class)->signals;
      nsignals = GTK_OBJECT_CLASS (class)->nsignals;

      if (nsignals)
	{
	  /* This groups the signals by class, e.g. 'GtkButton signals'. */
	  name = g_strdup_printf (_("%s signals"), gtk_type_name (type));
	  gtk_clist_append (GTK_CLIST (clist), &name);
	  g_free (name);
	  gtk_clist_set_foreground (GTK_CLIST (clist), row, inactive_fg);
	  gtk_clist_set_background (GTK_CLIST (clist), row, inactive_bg);
	  /* Set this so we know when a row containing a class name is
	     selected. See on_signals_clist_select(). */
	  gtk_clist_set_row_data (GTK_CLIST (clist), row, "ClassName");
	  row++;

	  items = NULL;
	  for (i = 0; i < nsignals; i++)
	    {
	      query_info = gtk_signal_query (signals[i]);
	      name = g_strdup (query_info->signal_name);
	      g_strdelimit (name, NULL, '_');
	      items = g_list_prepend (items, name);
	      g_free (query_info);
	    }

	  items = g_list_sort (items, (GCompareFunc)strcmp);

	  elem = items;
	  while (elem)
	    {
	      name = elem->data;
	      gtk_clist_append (GTK_CLIST (clist), &name);
	      gtk_clist_set_shift (GTK_CLIST (clist), row, 0, 0, 10);
	      if (!strcmp (current_signal, name))
		{
		  gtk_clist_select_row (GTK_CLIST (clist), row, 0);
		}
	      row++;
	      g_free (name);
	      elem = elem->next;
	    }
	  g_list_free (items);
	}

      type = gtk_type_parent (type);
    }

  gtk_clist_thaw (GTK_CLIST (clist));
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
		      GTK_SIGNAL_FUNC (on_signals_clist_select), clist);

#ifdef USE_GNOME
  button = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
#else
  button = gtk_button_new_with_label (_("OK"));
#endif
  gtk_box_pack_start (GTK_BOX (dialog->action_area), button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  /* Save pointer to value to use when OK pressed */
  gtk_object_set_data (GTK_OBJECT (clist), GbValueWidgetKey, value);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_signals_dialog_ok), clist);

#ifdef USE_GNOME
  button = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
#else
  button = gtk_button_new_with_label (_("Cancel"));
#endif
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (dialog->action_area), button, TRUE, TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
		      GINT_TO_POINTER (GladeEscDestroys));
  gtk_widget_show (GTK_WIDGET (dialog));
}


static void
on_signals_dialog_ok (GtkWidget * widget, GtkWidget * clist)
{
  gint row, page;
  GtkWidget *value, *handler;
  GList *selection = GTK_CLIST (clist)->selection;
  gchar *name, *handler_text;

  if (selection)
    {
      row = GPOINTER_TO_INT (selection->data);
      value = gtk_object_get_data (GTK_OBJECT (clist), GbValueWidgetKey);
      g_return_if_fail (value != NULL);
      gtk_clist_get_text (GTK_CLIST (clist), row, 0, &name);
      gtk_entry_set_text (GTK_ENTRY (value), name);

      /* If we're on the Signals page, and the current handler is empty,
         insert an initial value of "on_<widget name>_<signal name>" */
      page = gtk_notebook_get_current_page (GTK_NOTEBOOK (main_notebook));
      if (page == GB_PAGE_SIGNALS)
	{
	  handler = (GtkWidget *) g_hash_table_lookup (gb_property_values,
						       GbSignalHandler);
	  g_return_if_fail (handler != NULL);
	  handler_text = gtk_entry_get_text (GTK_ENTRY (\
				GTK_COMBO (handler)->entry));

	  if (strlen (handler_text) == 0)
	    {
	      gchar buf[128];
	      gchar *widget_name;
	      widget_name = gtk_widget_get_name (property_widget);
	      if (strlen (widget_name) + strlen (name) + 5 < 128)
		{
		  sprintf (buf, "on_%s_%s", widget_name, name);
		  gtk_entry_set_text (GTK_ENTRY ( GTK_COMBO (handler)->entry),
				      buf);
		  gtk_entry_select_region (GTK_ENTRY ( GTK_COMBO (handler)->\
					   entry), 0, -1);
		}
	      gtk_widget_grab_focus (handler);
	    }
	}
    }
  gtk_widget_destroy (gtk_widget_get_toplevel (clist));
}


static void
on_signals_clist_select (GtkWidget * widget, gint row, gint column,
			 GdkEventButton * bevent, gpointer data)
{
  /* Don't allow selection of widget class names */
  if (gtk_clist_get_row_data (GTK_CLIST (widget), row))
    gtk_clist_unselect_row (GTK_CLIST (widget), row, 0);
  if (bevent && bevent->type == GDK_2BUTTON_PRESS)
    {
      on_signals_dialog_ok (widget, widget);
    }
}


/*
   void
   on_apply(GtkWidget *widget, gpointer data)
   {
   if (!property_widget) return;
   gb_widget_apply_properties(property_widget, NULL);
   }
 */

void
property_set_auto_apply (gboolean value)
{
  auto_apply = value;
}


/* This is just used for debugging */
#ifdef GLADE_DEBUG
static void
find_hash_value (const gchar * key, gpointer data, gpointer property)
{
  if (data == property)
    MSG1 ("  applying property: %s", key);
}
#endif

static void
on_property_changed (GtkWidget * widget, GtkWidget * property)
{
  MSG ("In on_property_changed");
  if (property_widget && auto_apply)
    {
#ifdef GLADE_DEBUG
      g_hash_table_foreach (gb_property_values, (GHFunc) find_hash_value,
			    property);
#endif
      gb_widget_apply_properties (property_widget, property);
    }
}


/*
 * Adjustments - handles adding/showing/applying of all 6 properties
 */
void
property_add_adjustment (const gchar * Values[], gint label_type)
{
  const gchar *default_labels[] =
  {N_("Value:"), N_("Min:"), N_("Max:"), N_("Step Inc:"),
   N_("Page Inc:"), N_("Page Size:")};
  const gchar *horz_labels[] =
  {N_("H Value:"), N_("H Min:"), N_("H Max:"), N_("H Step Inc:"),
   N_("H Page Inc:"), N_("H Page Size:")};
  const gchar *vert_labels[] =
  {N_("V Value:"), N_("V Min:"), N_("V Max:"), N_("V Step Inc:"),
   N_("V Page Inc:"), N_("V Page Size:")};
  const gchar *tips[] =
  {
    N_("The initial value"),
    N_("The minimum value"),
    N_("The maximum value"),
    N_("The step increment"),
    N_("The page increment"),
    N_("The page size"),
  };
  const gchar **labels;
  gint i;

  if (label_type == GB_ADJUST_H_LABELS)
    labels = horz_labels;
  else if (label_type == GB_ADJUST_V_LABELS)
    labels = vert_labels;
  else
    labels = default_labels;

  for (i = 0; i < 6; i++)
    {
      if (Values[i])
	{
	  property_add_float (Values[i], _(labels[i]), _(tips[i]));
	}
    }
}


/*
 * The Font dialog
 */

/* This is the default GTK font spec., from gtkstyle.c */
#define GB_DEFAULT_XLFD_FONTNAME \
	"-adobe-helvetica-medium-r-normal--*-120-*-*-*-*-*-*"

static void
show_font_dialog (GtkWidget * widget, gpointer value)
{
  gchar *current_xlfd_fontname;
  GtkWidget *transient_parent;
  
  /* Create the dialog if it doesn't exist yet */
  if (!fontsel)
    {
      fontsel = GTK_FONT_SELECTION_DIALOG (gtk_font_selection_dialog_new (NULL));

      /* The OK/Apply/Cancel button */
      gtk_signal_connect (GTK_OBJECT (fontsel), "delete_event",
			  GTK_SIGNAL_FUNC (close_dialog_event), fontsel);
      gtk_signal_connect (GTK_OBJECT (fontsel->ok_button), "clicked",
			  GTK_SIGNAL_FUNC (on_font_dialog_ok), fontsel);
      gtk_widget_show (fontsel->apply_button);
      gtk_signal_connect (GTK_OBJECT (fontsel->apply_button), "clicked",
			  GTK_SIGNAL_FUNC (on_font_dialog_apply), fontsel);
      gtk_signal_connect (GTK_OBJECT (fontsel->cancel_button), "clicked",
			  GTK_SIGNAL_FUNC (close_dialog), fontsel);
      gtk_window_set_wmclass (GTK_WINDOW (fontsel), "font_selection", "Glade");
      gtk_signal_connect (GTK_OBJECT (fontsel), "key_press_event",
			  GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
			  GINT_TO_POINTER (GladeEscCloses));
    }

  /* Select font according to current setting */
  current_xlfd_fontname = gtk_object_get_data (GTK_OBJECT (value), GbFontSpecKey);
  if (!current_xlfd_fontname || current_xlfd_fontname[0] == '\0')
    current_xlfd_fontname = GB_DEFAULT_XLFD_FONTNAME;

  gtk_font_selection_dialog_set_font_name (fontsel, current_xlfd_fontname);

  /* Save pointer to value to use when OK/Apply pressed */
  gtk_object_set_data (GTK_OBJECT (fontsel), GbValueWidgetKey, value);

  gtk_widget_show (GTK_WIDGET (fontsel));
  transient_parent = glade_util_get_toplevel (widget);
  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (fontsel),
				  GTK_WINDOW (transient_parent));
  /* This maps the window, which also de-iconifies it according to ICCCM. */
  gdk_window_show (GTK_WIDGET (fontsel)->window);
  gdk_window_raise (GTK_WIDGET (fontsel)->window);
}


/* FIXME: This isn't used at present */
#if 0
static gint
get_font_size_from_spec (const gchar * spec)
{
  gint i, size = -1;

  for (i = 0; i < 8; i++)
    {
      spec = strchr (spec, '-');
      if (spec == NULL)
	return -1;
      spec++;
    }
  sscanf (spec, "%i", &size);
  return size;
}
#endif

/* Note: this only works with standard X font specs, e.g.
   -adobe-courier-bold-i-normal--0-0-75-75-m-0-iso8859-1
   It copies the first two fields, changing '-' to ' ' and capitalising the
   first characters of words - after a '-', ' ' or '&'.
   Note: returns pointer to static buffer, so copy it if you want to keep it */
static gchar *
get_font_name_from_spec (const gchar * spec)
{
  static gchar buf[128];
  gint i, dashes_found = 0;
  gboolean word_start = TRUE;
  gchar ch;

  if (spec == NULL)
    return "";

  for (i = 0; i < 127; i++)
    {
      ch = spec[i + 1];
      if (ch == '\0')
	break;
      if (ch == '-')
	{
	  dashes_found++;
	  if (dashes_found == 2)
	    break;
	  ch = ' ';
	}
      if (word_start)
	ch = toupper (ch);
      word_start = (ch == ' ' || ch == '&');
      buf[i] = ch;
    }
  buf[i] = '\0';
  return buf;
}


static void
on_font_dialog_apply (GtkWidget * widget, GtkFontSelectionDialog * fontsel)
{
  GtkWidget *value;
  GdkFont *font, *old_font;
  gchar *xlfd_fontname, *old_xlfd_fontname;

  value = gtk_object_get_data (GTK_OBJECT (fontsel), GbValueWidgetKey);
  g_return_if_fail (value != NULL);

  /* Try to create the font, if the font spec has changed */
  xlfd_fontname = gtk_font_selection_dialog_get_font_name (fontsel);
  font = gtk_font_selection_dialog_get_font (fontsel);
  old_xlfd_fontname = gtk_object_get_data (GTK_OBJECT (value), GbFontSpecKey);

  if (!old_xlfd_fontname
      || (xlfd_fontname && strcmp (xlfd_fontname, old_xlfd_fontname)))
    {
      if (font == NULL)
	{
	  glade_util_show_message_box (_("The requested font is not available."), widget);
	  return;
	}

      old_font = gtk_object_get_data (GTK_OBJECT (value), GbFontKey);
      if (old_font)
	gdk_font_unref (old_font);
      gdk_font_ref (font);
      gtk_object_set_data (GTK_OBJECT (value), GbFontKey, font);
      g_free (old_xlfd_fontname);
      gtk_object_set_data (GTK_OBJECT (value), GbFontSpecKey, g_strdup (xlfd_fontname));
      gtk_entry_set_text (GTK_ENTRY (value), get_font_name_from_spec (xlfd_fontname));
    }
}


static void
on_font_dialog_ok (GtkWidget * widget, GtkFontSelectionDialog * fontsel)
{
  on_font_dialog_apply (widget, fontsel);
  close_dialog (widget, GTK_WIDGET (fontsel));
}


/*
 * The Style dialog, for selecting a style to use/copy for a widget.
 */

#ifdef GLADE_STYLE_SUPPORT
static void
show_style_dialog (GtkWidget * widget, gpointer value)
{
  GtkWidget *dialog;
  GtkWidget *hbox, *clist, *vbbox, *button, *scrolled_win;
  GtkWidget *transient_parent;
  int i;
  gchar *current_style;
  gchar *titles[1];
  gchar *text;
  gchar *row[1];

  dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 3);

  gtk_signal_connect_object (GTK_OBJECT (dialog), "delete_event",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));

  gtk_window_set_title (GTK_WINDOW (dialog), _("Select Named Style"));
  gtk_window_set_wmclass (GTK_WINDOW (dialog), "select_named_style", "Glade");
  transient_parent = glade_util_get_toplevel (transient_for);
  if (GTK_IS_WINDOW (transient_parent))
    gtk_window_set_transient_for (GTK_WINDOW (dialog),
				  GTK_WINDOW (transient_parent));

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (dialog), hbox);
  gtk_widget_show (hbox);

  titles[0] = _("Styles");
  clist = gtk_clist_new_with_titles (1, titles);
  gtk_clist_column_titles_passive (GTK_CLIST (clist));
  gtk_widget_set_usize (clist, 200, 200);
  gtk_widget_show (clist);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), clist);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (hbox), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show(scrolled_win);

  /* Insert styles */
  gtk_clist_freeze (GTK_CLIST (clist));

  /* Add unnamed style first */
  row[0] = GB_STYLE_UNNAMED;
  gtk_clist_append (GTK_CLIST (clist), row);
  g_hash_table_foreach (gb_style_hash, (GHFunc) add_style_to_clist, clist);

  current_style = gtk_entry_get_text (GTK_ENTRY (value));
  if (strlen (current_style) == 0)
    {
      gtk_clist_select_row (GTK_CLIST (clist), 0, 0);
    }
  else
    {
      for (i = 1; i < GTK_CLIST (clist)->rows; i++)
	{
	  gtk_clist_get_text (GTK_CLIST (clist), i, 0, &text);
	  if (!strcmp (current_style, text))
	    {
	      gtk_clist_select_row (GTK_CLIST (clist), i, 0);
	    }
	}
    }

  gtk_clist_thaw (GTK_CLIST (clist));
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
		      GTK_SIGNAL_FUNC (on_style_clist_select), clist);

  /* Save pointer to value to use when a button is pressed */
  gtk_object_set_data (GTK_OBJECT (clist), GbValueWidgetKey, value);

  /* Create all the buttons */
  vbbox = gtk_vbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (vbbox), GTK_BUTTONBOX_START);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbbox), 2);
  gtk_widget_show (vbbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbbox, FALSE, TRUE, 0);

  button = gtk_button_new_with_label (_("New"));
  gtk_box_pack_start (GTK_BOX (vbbox), button, FALSE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_dialog_new), clist);

  button = gtk_button_new_with_label (_("Rename"));
  gtk_box_pack_start (GTK_BOX (vbbox), button, FALSE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_dialog_rename), clist);
  gtk_object_set_data (GTK_OBJECT (clist), "rename_button", button);
  gtk_widget_set_sensitive (button, FALSE);


  button = gtk_button_new_with_label (_("Delete"));
  gtk_box_pack_start (GTK_BOX (vbbox), button, FALSE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_dialog_delete), clist);
  gtk_object_set_data (GTK_OBJECT (clist), "delete_button", button);
  gtk_widget_set_sensitive (button, FALSE);

  button = gtk_button_new_with_label (_("Copy"));
  gtk_box_pack_start (GTK_BOX (vbbox), button, FALSE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_dialog_copy), clist);
  gtk_object_set_data (GTK_OBJECT (clist), "copy_button", button);
  gtk_widget_set_sensitive (button, FALSE);

  button = gtk_button_new_with_label (_("Cancel"));
  gtk_box_pack_start (GTK_BOX (vbbox), button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));
  gtk_widget_show (GTK_WIDGET (dialog));
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (on_key_press),
		      GINT_TO_POINTER (PROP_CLOSE_DESTROYS));

  button = gtk_button_new_with_label (_("OK"));
  gtk_box_pack_start (GTK_BOX (vbbox), button, FALSE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (on_style_dialog_ok), clist);

}


static gint
add_style_to_clist (const gchar * key, gpointer data, GtkWidget * clist)
{
  gint i;
  gchar *text;
  const gchar *row[1];
  row[0] = key;

  /* Leave unnamed style at top */
  for (i = 1; i < GTK_CLIST (clist)->rows; i++)
    {
      gtk_clist_get_text (GTK_CLIST (clist), i, 0, &text);
      if (strcmp (key, text) < 0)
	{
	  gtk_clist_insert (GTK_CLIST (clist), i, (gchar**) row);
	  return i;
	}
    }
  return gtk_clist_append (GTK_CLIST (clist), (gchar**) row);
}


static void
on_style_clist_select (GtkWidget * widget, gint row, gint column,
		       GdkEventButton * bevent, gpointer data)
{
  gchar *text = NULL;
  GtkWidget *copy_button, *rename_button, *delete_button;
  gboolean copy_sensitive = TRUE;
  gboolean rename_sensitive = TRUE;
  gboolean delete_sensitive = TRUE;

  if (bevent && bevent->type == GDK_2BUTTON_PRESS)
    {
      on_style_dialog_ok (widget, widget);
      return;
    }

  copy_button = gtk_object_get_data (GTK_OBJECT (widget), "copy_button");
  g_return_if_fail (copy_button != NULL);
  rename_button = gtk_object_get_data (GTK_OBJECT (widget), "rename_button");
  g_return_if_fail (rename_button != NULL);
  delete_button = gtk_object_get_data (GTK_OBJECT (widget), "delete_button");
  g_return_if_fail (delete_button != NULL);

  /* If unnamed style selected, make copy, rename & delete buttons insensitive,
     else if default style selected, make rename & delete insensitive. */
  gtk_clist_get_text (GTK_CLIST (widget), row, 0, &text);
  /* Added this check since it SEGVed once here. */
  g_return_if_fail (text != NULL);
  if (!strcmp (text, GB_STYLE_UNNAMED))
    {
      copy_sensitive = FALSE;
      rename_sensitive = FALSE;
      delete_sensitive = FALSE;
    }
  else if (!strcmp (text, GB_STYLE_DEFAULT))
    {
      rename_sensitive = FALSE;
      delete_sensitive = FALSE;
    }
  gtk_widget_set_sensitive (copy_button, copy_sensitive);
  gtk_widget_set_sensitive (rename_button, rename_sensitive);
  gtk_widget_set_sensitive (delete_button, delete_sensitive);
}


static void
on_style_dialog_new (GtkWidget * widget, GtkWidget * clist)
{
  GList *selection = GTK_CLIST (clist)->selection;
  gint row;
  gchar *text;
  GladeWidgetData *wdata;
  GbStyle *base_gbstyle = NULL;

  if (selection)
    {
      row = GPOINTER_TO_INT (selection->data);
      gtk_clist_get_text (GTK_CLIST (clist), row, 0, &text);
      if (!strcmp (text, GB_STYLE_UNNAMED))
	{
	  if (property_widget)
	    {
	      wdata = gtk_object_get_data (GTK_OBJECT (property_widget),
					   GB_WIDGET_DATA_KEY);
	      if (wdata)
		base_gbstyle = wdata->gbstyle;
	    }
	}
      else
	{
	  base_gbstyle = (GbStyle *) g_hash_table_lookup (gb_style_hash, text);
	  g_return_if_fail (base_gbstyle != NULL);
	}
    }

  glade_util_show_entry_dialog (_("New Style:"), NULL, clist,
				(GbEntryDialogFunc) create_new_style,
				base_gbstyle);
}


static gint
create_new_style (GtkWidget * widget, const gchar * name, GbStyle * base_gbstyle)
{
  GbStyle *gbstyle, *existing_gbstyle;
  gint row;

  if (strlen (name) == 0)
    {
      glade_util_show_message_box (_("Invalid style name"));
      return FALSE;
    }

  /* Make sure name is unique */
  existing_gbstyle = (GbStyle *) g_hash_table_lookup (gb_style_hash, name);
  if (existing_gbstyle)
    {
      glade_util_show_message_box (_("That style name is already in use"));
      return FALSE;
    }

  if (!base_gbstyle)
    base_gbstyle = gb_widget_default_gb_style;
  g_return_val_if_fail (base_gbstyle != NULL, TRUE);

  gbstyle = gb_widget_copy_gb_style (base_gbstyle);
  g_free (gbstyle->name);
  gbstyle->name = g_strdup (name);
  g_hash_table_insert (gb_style_hash, gbstyle->name, gbstyle);

  /* Add style to clist */
  row = add_style_to_clist (name, NULL, widget);
  gtk_clist_select_row (GTK_CLIST (widget), row, 0);

  return TRUE;
}


static void
on_style_dialog_copy (GtkWidget * widget, GtkWidget * clist)
{
  GladeWidgetData *wdata;
  gint row, i;
  GtkWidget *value;
  GList *selection = GTK_CLIST (clist)->selection;
  gchar *text;
  GbStyle *gbstyle, *other_gbstyle;

  if (selection)
    {
      row = GPOINTER_TO_INT (selection->data);
      value = gtk_object_get_data (GTK_OBJECT (clist), GbValueWidgetKey);
      g_return_if_fail (value != NULL);

      gtk_clist_get_text (GTK_CLIST (clist), row, 0, &text);
      if (!strcmp (text, GB_STYLE_UNNAMED))
	return;
      other_gbstyle = (GbStyle *) g_hash_table_lookup (gb_style_hash, text);
      g_return_if_fail (other_gbstyle != NULL);

      if (property_widget)
	{
	  wdata = gtk_object_get_data (GTK_OBJECT (property_widget),
				       GB_WIDGET_DATA_KEY);
	  g_return_if_fail (wdata != NULL);
	  gbstyle = wdata->gbstyle;

	  /* If widget is using an unnamed GbStyle, just use the selected GbStyle.
	     else copy the selected GbStyle to the current one. */
	  if (wdata->flags & GB_STYLE_IS_UNNAMED)
	    {
	      gb_widget_set_gb_style (widget, other_gbstyle);
	    }
	  else
	    {
	      g_free (gbstyle->xlfd_fontname);
	      gbstyle->xlfd_fontname = g_strdup (other_gbstyle->xlfd_fontname);
	      for (i = 0; i < GB_NUM_STYLE_STATES; i++)
		{
		  g_free (gbstyle->bg_pixmap_filenames[i]);
		  gbstyle->bg_pixmap_filenames[i]
		    = g_strdup (other_gbstyle->bg_pixmap_filenames[i]);
		}
	      gtk_style_unref (gbstyle->style);
	      gbstyle->style = other_gbstyle->style;
	      gtk_style_ref (gbstyle->style);
	      gb_widget_update_gb_styles (gbstyle, gbstyle);
	    }

	  editor_refresh_widget (property_widget);
	  property_set_auto_apply (FALSE);
	  gb_widget_show_style (property_widget);
	  property_set_auto_apply (TRUE);
	}
    }
  gtk_widget_destroy (gtk_widget_get_toplevel (clist));
}


static void
on_style_dialog_rename (GtkWidget * widget, GtkWidget * clist)
{
  gint row;
  GList *selection = GTK_CLIST (clist)->selection;
  gchar *text;
  GbStyle *gbstyle;

  if (selection)
    {
      row = GPOINTER_TO_INT (selection->data);
      gtk_clist_get_text (GTK_CLIST (clist), row, 0, &text);
      if (!strcmp (text, GB_STYLE_UNNAMED) || (!strcmp (text, GB_STYLE_DEFAULT)))
	return;
      gbstyle = (GbStyle *) g_hash_table_lookup (gb_style_hash, text);
      g_return_if_fail (gbstyle != NULL);
      glade_util_show_entry_dialog (_("Rename Style To:"), text, clist,
				    (GbEntryDialogFunc) rename_style, gbstyle);
    }
}


static gint
rename_style (GtkWidget * clist, const gchar * name, GbStyle * gbstyle)
{
  GbStyle *existing_gbstyle;
  gchar *text, *old_name;
  gint i, row;

  if (strlen (name) == 0)
    {
      glade_util_show_message_box (_("Invalid style name"));
      return FALSE;
    }

  /* Make sure name is unique */
  existing_gbstyle = (GbStyle *) g_hash_table_lookup (gb_style_hash, text);
  if (existing_gbstyle == gbstyle)
    return TRUE;
  if (existing_gbstyle)
    {
      glade_util_show_message_box (_("That style name is already in use"));
      return FALSE;
    }

  old_name = gbstyle->name;
  gbstyle->name = g_strdup (name);

  /* Delete old entry in style hash & insert new one */
  g_hash_table_remove (gb_style_hash, old_name);
  g_hash_table_insert (gb_style_hash, gbstyle->name, gbstyle);

  /* Update name in clist */
  for (i = 0; i < GTK_CLIST (clist)->rows; i++)
    {
      gtk_clist_get_text (GTK_CLIST (clist), i, 0, &text);
      if (!strcmp (text, old_name))
	{
	  gtk_clist_remove (GTK_CLIST (clist), i);
	  break;
	}
    }
  row = add_style_to_clist (name, NULL, clist);
  gtk_clist_select_row (GTK_CLIST (clist), row, 0);

  g_free (old_name);

  return TRUE;
}


static void
on_style_dialog_delete (GtkWidget * widget, GtkWidget * clist)
{
  gint row;
  GtkWidget *value;
  GList *selection = GTK_CLIST (clist)->selection;
  gchar *text;
  GbStyle *gbstyle;
  gboolean reshow = FALSE;

  if (selection)
    {
      row = GPOINTER_TO_INT (selection->data);
      value = gtk_object_get_data (GTK_OBJECT (clist), GbValueWidgetKey);
      g_return_if_fail (value != NULL);

      gtk_clist_get_text (GTK_CLIST (clist), row, 0, &text);
      if (!strcmp (text, GB_STYLE_UNNAMED) || (!strcmp (text, GB_STYLE_DEFAULT)))
	return;

      gbstyle = (GbStyle *) g_hash_table_lookup (gb_style_hash, text);
      g_return_if_fail (gbstyle != NULL);

      gtk_clist_remove (GTK_CLIST (clist), row);

      if (property_widget && property_widget->style == gbstyle->style)
	{
	  reshow = TRUE;
	}

      /* Make all widgets which are using the style use the default instead */
      gb_widget_update_gb_styles (gbstyle, gb_widget_default_gb_style);
      gb_widget_destroy_gb_style (gbstyle, TRUE);

      if (reshow)
	{
	  property_set_auto_apply (FALSE);
	  gb_widget_show_style (property_widget);
	  property_set_auto_apply (TRUE);
	}
    }
}


static void
on_style_dialog_ok (GtkWidget * widget, GtkWidget * clist)
{
  GladeWidgetData *wdata;
  gint row;
  GtkWidget *value;
  GList *selection = GTK_CLIST (clist)->selection;
  gchar *text;
  GbStyle *gbstyle;

  if (selection)
    {
      row = GPOINTER_TO_INT (selection->data);
      value = gtk_object_get_data (GTK_OBJECT (clist), GbValueWidgetKey);
      g_return_if_fail (value != NULL);

      gtk_clist_get_text (GTK_CLIST (clist), row, 0, &text);

      if (property_widget)
	{
	  wdata = gtk_object_get_data (GTK_OBJECT (property_widget),
				       GB_WIDGET_DATA_KEY);
	  g_return_if_fail (wdata != NULL);

	  /* If <none> is selected, just set the unnamed style flag, so if any
	     changes are made to the style a new GbStyle is created. */
	  if (!strcmp (text, GB_STYLE_UNNAMED))
	    {
	      if (!(wdata->flags & GB_STYLE_IS_UNNAMED))
		{
		  wdata->flags |= GB_STYLE_IS_UNNAMED;
		  property_set_auto_apply (FALSE);
		  gb_widget_show_style (property_widget);
		  property_set_auto_apply (TRUE);
		}
	    }
	  else
	    {
	      gbstyle = (GbStyle *) g_hash_table_lookup (gb_style_hash, text);
	      g_return_if_fail (gbstyle != NULL);
	      wdata->flags &= ~GB_STYLE_IS_UNNAMED;

	      gb_widget_set_gb_style (property_widget, gbstyle);
	      editor_refresh_widget (property_widget);
	      property_set_auto_apply (FALSE);
	      gb_widget_show_style (property_widget);
	      property_set_auto_apply (TRUE);
	    }
	}
    }
  gtk_widget_destroy (gtk_widget_get_toplevel (clist));
}
#endif


/* Experimental code. */
void
property_redirect_key_press (GdkEventKey *event)
{
  GtkWidget *value_widget;
  GdkEventKey tmp_event;
  gchar *property_name = NULL;

  if (property_widget == NULL)
    return;

  gtk_notebook_set_page (GTK_NOTEBOOK (main_notebook), GB_PAGE_WIDGET);

  /* Make sure subclasses are tested first.
     FIXME: Shouldn't really be using copies of the strings here. */
  if (GTK_IS_ACCEL_LABEL (property_widget))
    property_name = "AccelLabel|GtkLabel::label";
  else if (GTK_IS_LABEL (property_widget))
    property_name = "GtkLabel::label";
  else if (GTK_IS_RADIO_BUTTON (property_widget))
    property_name = "RadioButton|GtkButton::label";
  else if (GTK_IS_CHECK_BUTTON (property_widget))
    property_name = "CheckButton|GtkButton::label";
  else if (GTK_IS_TOGGLE_BUTTON (property_widget))
    property_name = "ToggleButton|GtkButton::label";
  else if (GTK_IS_BUTTON (property_widget))
    property_name = "GtkButton::label";

  if (property_name == NULL)
    return;

  value_widget = (GtkWidget *) g_hash_table_lookup (gb_property_values,
						    property_name);
  g_return_if_fail (value_widget != NULL);

  if (GTK_IS_SCROLLED_WINDOW (value_widget))
    value_widget = GTK_BIN (value_widget)->child;

  if (!GTK_WIDGET_IS_SENSITIVE (value_widget))
    return;

  /* If this is the first key-press, we delete the current text. */
  if (!typing_over_widget)
    {
      /* GtkText can cause crashes, so we only allow short one-line labels to
	 be edited. */
      gchar *text = gtk_editable_get_chars (GTK_EDITABLE (value_widget),
					    0, -1);
      if (strlen (text) > 30 || strchr (text, '\n'))
	{
	  g_free (text);
	  return;
	}
      g_free (text);
      gtk_editable_delete_text (GTK_EDITABLE (value_widget), 0, -1);
      typing_over_widget = TRUE;
    }

  tmp_event.type = event->type;
  tmp_event.window = event->window;
  tmp_event.send_event = TRUE;
  tmp_event.state = event->state;
  tmp_event.keyval = event->keyval;
  tmp_event.length = event->length;
  tmp_event.string = event->string;

  gtk_widget_event (value_widget, (GdkEvent *)&tmp_event);
}


gboolean
property_is_typing_over_widget (void)
{
  return typing_over_widget;
}


/*
 * Callbacks for the buttons to reset the widget width & height.
 * We set the values to 0, which will result in the widget being resized to
 * the default width/height. This then results in the property being updated
 * to show the default size.
 */

static void
update_position_property (GtkWidget * widget, gpointer value, gint flag)
{
  GladeWidgetData *wdata;
  gboolean value_set;
  gint w, h;

  if (property_widget == NULL)
    return;
  wdata = gtk_object_get_data (GTK_OBJECT (property_widget),
			       GB_WIDGET_DATA_KEY);
  g_return_if_fail (wdata != NULL);

  value_set = GTK_TOGGLE_BUTTON (widget)->active ? TRUE : FALSE;
  gtk_widget_set_sensitive (GTK_WIDGET (value), value_set);
  if (value_set)
    wdata->flags |= flag;
  else
    wdata->flags &= ~flag;

  /* X & Y flags can only be changed for windows, and we don't need to update
     those, so we only have to worry about the width & height flags changing.*/
  if (flag == GLADE_WIDTH_SET || flag == GLADE_HEIGHT_SET)
    {
      w = wdata->flags & GLADE_WIDTH_SET ? wdata->width : -1;
      h = wdata->flags & GLADE_HEIGHT_SET ? wdata->height : -1;

      gb_widget_set_usize (property_widget, w, h);
    }
}


static void
on_toggle_set_x (GtkWidget * widget, gpointer value)
{
  update_position_property (widget, value, GLADE_X_SET);
}

static void
on_toggle_set_y (GtkWidget * widget, gpointer value)
{
  update_position_property (widget, value, GLADE_Y_SET);
}

static void
on_toggle_set_width (GtkWidget * widget, gpointer value)
{
  update_position_property (widget, value, GLADE_WIDTH_SET);
}


static void
on_toggle_set_height (GtkWidget * widget, gpointer value)
{
  update_position_property (widget, value, GLADE_HEIGHT_SET);
}
