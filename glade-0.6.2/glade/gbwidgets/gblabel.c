
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

#include <math.h>
#include <string.h>

#include <gtk/gtkcombo.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtklist.h>
#include <gtk/gtklistitem.h>
#include <gtk/gtkeventbox.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/label.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "GtkLabel::label";
static gchar *Justify = "GtkLabel::justify";
static gchar *Wrap = "GtkLabel::wrap";
static gchar *XAlign = "Label|GtkMisc::xalign";
static gchar *YAlign = "Label|GtkMisc::yalign";
static gchar *XPad = "Label|GtkMisc::xpad";
static gchar *YPad = "Label|GtkMisc::ypad";

static gchar *AccelTarget = "GtkLabel::focus_target";

/* This is only saved to the XML, and only when AccelTarget is set to Auto.
   It is not loaded or used in Glade. */
static gchar *DefaultAccelTarget = "GtkLabel::default_focus_target";


static const gchar *GbJustifyChoices[] =
{
  "Left",
  "Right",
  "Center",
  "Fill",
  NULL
};
static const gint GbJustifyValues[] =
{
  GTK_JUSTIFY_LEFT,
  GTK_JUSTIFY_RIGHT,
  GTK_JUSTIFY_CENTER,
  GTK_JUSTIFY_FILL
};
static const gchar *GbJustifySymbols[] =
{
  "GTK_JUSTIFY_LEFT",
  "GTK_JUSTIFY_RIGHT",
  "GTK_JUSTIFY_CENTER",
  "GTK_JUSTIFY_FILL"
};


static void gb_label_get_focus_targets (GtkWidget * widget,
					GList ** focus_targets);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkLabel, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_label_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;

  new_widget = gtk_label_new (data->name);

  /* If we are creating a new label in a table or in an event box in a table
     set it to left-aligned, since that is what is usually wanted. */
  if (data->action == GB_CREATING && data->parent
      && (GTK_IS_TABLE (data->parent)
	  || (GTK_IS_EVENT_BOX (data->parent) && data->parent->parent
	      && GTK_IS_TABLE (data->parent->parent))))
    {
      gtk_misc_set_alignment (GTK_MISC (new_widget), 0.0, 0.5);
    }

  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_label_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  GtkWidget *combo;

  property_add_text (Label, _("Label:"), _("The text to display"), 2);
  property_add_choice (Justify, _("Justify:"),
		       _("The justification of the lines of the label"),
		       GbJustifyChoices);
  property_add_bool (Wrap, _("Wrap Text:"),
		     _("If the text is wrapped to fit within the width of the label"));
  property_add_float_range (XAlign, _("X Align:"),
			    _("The horizontal alignment of the entire label"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_float_range (YAlign, _("Y Align:"),
			    _("The vertical alignment of the entire label"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_int_range (XPad, _("X Pad:"), _("The horizontal padding"),
			  0, 1000, 1, 10, 1);
  property_add_int_range (YPad, _("Y Pad:"), _("The vertical padding"),
			  0, 1000, 1, 10, 1);
  property_add_combo (AccelTarget, _("Focus Target:"),
		      _("The widget to set the keyboard focus to when the underlined accelerator key is used"),
		      NULL);
  combo = property_get_value_widget (AccelTarget);
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (combo)->entry), FALSE);
  gtk_combo_set_value_in_list (GTK_COMBO (combo), TRUE, TRUE);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_label_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gchar *label_text;
  gint i;

  label_text = glade_util_get_label_text (widget);
  gb_widget_output_translatable_text (data, Label, label_text);

  for (i = 0; i < sizeof (GbJustifyValues) / sizeof (GbJustifyValues[0]); i++)
    {
      if (GbJustifyValues[i] == GTK_LABEL (widget)->jtype)
	gb_widget_output_choice (data, Justify, i, GbJustifySymbols[i]);
    }
  gb_widget_output_bool (data, Wrap, GTK_LABEL (widget)->wrap);
  gb_widget_output_float (data, XAlign, GTK_MISC (widget)->xalign);
  gb_widget_output_float (data, YAlign, GTK_MISC (widget)->yalign);
  gb_widget_output_int (data, XPad, GTK_MISC (widget)->xpad);
  gb_widget_output_int (data, YPad, GTK_MISC (widget)->ypad);

  /* Labels not in buttons may have a focus target widget. */
  if (!glade_util_find_parent_button (widget))
    {
      gchar *accel_target;

      accel_target = gtk_object_get_data (GTK_OBJECT (widget), AccelTarget);

      /* If we're showing we need to display the list of possible focus target
	 widgets. We walk the tree of widgets in this component, and if a
	 widget has CAN_FOCUS set, we add it to the list. */
      if (data->action == GB_SHOWING)
	{
	  GList *focus_targets = NULL, *standard_items = NULL;
	  GtkWidget *item, *combo;

	  property_set_visible (AccelTarget, TRUE);

	  gb_label_get_focus_targets (gtk_widget_get_toplevel (widget),
				      &focus_targets);
	  property_set_combo_strings (AccelTarget, focus_targets);
	  g_list_free (focus_targets);

	  combo = property_get_value_widget (AccelTarget);

	  item = gtk_list_item_new_with_label (_("Auto"));
	  gtk_widget_show (item);
	  standard_items = g_list_append (standard_items, item);

	  item = gtk_list_item_new ();
	  gtk_widget_set_sensitive (item, FALSE);
	  gtk_widget_show (item);
	  standard_items = g_list_append (standard_items, item);
	  gtk_combo_set_item_string (GTK_COMBO (combo), GTK_ITEM (item), "");

	  gtk_list_prepend_items (GTK_LIST (GTK_COMBO (combo)->list),
				  standard_items);

	  if (!accel_target)
	    {
	      accel_target = _("Auto");
	    }
	  gb_widget_output_combo (data, AccelTarget, accel_target);
	}
      else
	{
	  /* When saving, we only save the property if it has been set. */
	  if (accel_target)
	    {
	      /* First check that the widget is still there, and if it isn't
		 just skip it. */
	      if (glade_util_find_widget (gtk_widget_get_toplevel (widget),
					  accel_target))
		{
		  gb_widget_output_combo (data, AccelTarget, accel_target);
		}
	    }
	  else
	    {
	      /* If no target has been set, and the label has an underlined
		 key, we try to find a default target and save that. */
	      if (strchr (label_text, '_'))
		{
		  GtkWidget *accel_target;
		  gchar *signal_name;

		  accel_target = glade_util_find_default_accelerator_target (widget, &signal_name);
		  if (accel_target)
		    {
		      gb_widget_output_string (data, DefaultAccelTarget, gtk_widget_get_name (accel_target));
		    }
		}
	    }
	}
    }
  else
    {
      if (data->action == GB_SHOWING)
	{
	  property_set_visible (AccelTarget, FALSE);
	}
    }

  g_free (label_text);
}


static void
gb_label_get_focus_targets (GtkWidget * widget,
			    GList ** focus_targets)
{
  if (GTK_WIDGET_CAN_FOCUS (widget) && GB_IS_GB_WIDGET (widget))
    {
      *focus_targets = g_list_insert_sorted (*focus_targets,
					     gtk_widget_get_name (widget),
					     g_str_equal);
    }

  if (GTK_IS_CONTAINER (widget))
    {
      gtk_container_forall (GTK_CONTAINER (widget),
			    (GtkCallback) gb_label_get_focus_targets,
			    focus_targets);
    }
}


/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_label_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *label, *justify, *accel_target;
  gfloat xalign, yalign;
  gint xpad, ypad, i;
  gboolean set_label = FALSE, free_label = FALSE, wrap;
  gboolean set_alignment = FALSE, set_padding = FALSE;

  label = gb_widget_input_text (data, Label);
  /* We use parse_uline so letters can be underlined. */
  if (data->apply)
    set_label = TRUE;
  if (data->action == GB_APPLYING)
    free_label = TRUE;

  justify = gb_widget_input_choice (data, Justify);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbJustifyValues) / sizeof (GbJustifyValues[0]);
	   i++)
	{
	  if (!strcmp (justify, GbJustifyChoices[i])
	      || !strcmp (justify, GbJustifySymbols[i]))
	    {
	      gtk_label_set_justify (GTK_LABEL (widget), GbJustifyValues[i]);
	      set_label = TRUE;
	      break;
	    }
	}
    }

  wrap = gb_widget_input_bool (data, Wrap);
  if (data->apply)
    {
      gtk_label_set_line_wrap (GTK_LABEL (widget), wrap);
      set_label = TRUE;
    }

  /* GTK+ 1.2 bug workaround. If we change the justify property, we need to
     reset the label or it isn't displayed correctly. */
  if (set_label)
    {
      if (!label)
	{
	  label = glade_util_get_label_text (widget);
	  free_label = TRUE;
	}

      gtk_label_parse_uline (GTK_LABEL (widget), label);
    }
  if (free_label)
    g_free (label);

  xalign = gb_widget_input_float (data, XAlign);
  if (data->apply)
    set_alignment = TRUE;
  else
    xalign = GTK_MISC (widget)->xalign;

  yalign = gb_widget_input_float (data, YAlign);
  if (data->apply)
    set_alignment = TRUE;
  else
    yalign = GTK_MISC (widget)->yalign;

  if (set_alignment)
    gtk_misc_set_alignment (GTK_MISC (widget), xalign, yalign);

  xpad = gb_widget_input_int (data, XPad);
  if (data->apply)
    set_padding = TRUE;
  else
    xpad = GTK_MISC (widget)->xpad;

  ypad = gb_widget_input_int (data, YPad);
  if (data->apply)
    set_padding = TRUE;
  else
    ypad = GTK_MISC (widget)->ypad;

  if (set_padding)
    gtk_misc_set_padding (GTK_MISC (widget), xpad, ypad);

  /* Labels not in buttons may have a focus target widget. */
  accel_target = gb_widget_input_combo (data, AccelTarget);
  if (data->apply)
    {
      if (!glade_util_find_parent_button (widget))
	{
	  if (!strcmp (accel_target, _("Auto")))
	    accel_target = NULL;

	  gtk_object_set_data_full (GTK_OBJECT (widget), AccelTarget,
				    g_strdup (accel_target),
				    accel_target ? g_free : NULL);
	}
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkLabel, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_label_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_label_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gchar *label_text, *signal_name, *target_name;
  GtkWidget *accel_target = NULL;
  gint i;

  if (data->create_widget)
    {
      label_text = glade_util_get_label_text (widget);
      /* If there is an underlined accelerator, set up the accel signal. */
      if (strchr (label_text, '_'))
	{
	  if (!glade_util_find_parent_button (widget))
	    {
	      target_name = gtk_object_get_data (GTK_OBJECT (widget),
						 AccelTarget);
	      if (target_name)
		accel_target = glade_util_find_widget (gtk_widget_get_toplevel (widget), target_name);
	      signal_name = "grab_focus";
	    }

	  if (!accel_target)
	    {
	      accel_target = glade_util_find_default_accelerator_target (widget, &signal_name);
	    }

	  if (accel_target)
	    {
	      source_add_decl (data, "  guint %s_key;\n", data->real_wname);
	      source_add (data, "  %s = gtk_label_new (\"\");\n", data->wname);
	      source_add (data,
			  "  %s_key = gtk_label_parse_uline (GTK_LABEL (%s),\n"
			  "                                   %s);\n",
			  data->real_wname, data->wname,
			  source_make_string (label_text, data->use_gettext));

	      data->need_accel_group = TRUE;
	      target_name = gtk_widget_get_name (accel_target);
	      target_name = source_create_valid_identifier (target_name);
	      source_add_to_buffer (data, GLADE_ACCELERATORS,
				    "  gtk_widget_add_accelerator (%s, \"%s\", accel_group,\n"
				    "                              %s_key, GDK_MOD1_MASK, (GtkAccelFlags) 0);\n",
				    target_name, signal_name,
				    data->real_wname);
	      g_free (target_name);
	    }
	  else
	    {
	      source_add (data, "  %s = gtk_label_new (\"\");\n", data->wname);
	      source_add (data,
			  "  gtk_label_parse_uline (GTK_LABEL (%s),\n"
			  "                         %s);\n",
			  data->wname,
			  source_make_string (label_text, data->use_gettext));
	    }
	}
      else
	{
	  source_add (data, "  %s = gtk_label_new (%s);\n", data->wname,
		      source_make_string (label_text, data->use_gettext));
	}
      g_free (label_text);
    }

  gb_widget_write_standard_source (widget, data);

  if (GTK_LABEL (widget)->jtype != GTK_JUSTIFY_CENTER)
    {
      for (i = 0; i < sizeof (GbJustifyValues) / sizeof (GbJustifyValues[0]);
	   i++)
	{
	  if (GbJustifyValues[i] == GTK_LABEL (widget)->jtype)
	    source_add (data,
			"  gtk_label_set_justify (GTK_LABEL (%s), %s);\n",
			data->wname, GbJustifySymbols[i]);
	}
    }

  if (GTK_LABEL (widget)->wrap)
    source_add (data, "  gtk_label_set_line_wrap (GTK_LABEL (%s), TRUE);\n",
		data->wname);

  if (fabs (GTK_MISC (widget)->xalign - 0.5) > 0.0001
      || fabs (GTK_MISC (widget)->yalign - 0.5) > 0.0001)
    source_add (data, "  gtk_misc_set_alignment (GTK_MISC (%s), %g, %g);\n",
	 data->wname, GTK_MISC (widget)->xalign, GTK_MISC (widget)->yalign);

  if (GTK_MISC (widget)->xpad != 0 || GTK_MISC (widget)->ypad != 0)
    source_add (data, "  gtk_misc_set_padding (GTK_MISC (%s), %i, %i);\n",
	     data->wname, GTK_MISC (widget)->xpad, GTK_MISC (widget)->ypad);
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_label_init ()
{
  /* Initialise the GTK type */
  gtk_label_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = label_xpm;
  gbwidget.tooltip = _("Label");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_label_new;
  gbwidget.gb_widget_create_properties = gb_label_create_properties;
  gbwidget.gb_widget_get_properties = gb_label_get_properties;
  gbwidget.gb_widget_set_properties = gb_label_set_properties;
  gbwidget.gb_widget_write_source = gb_label_write_source;
/*
   gbwidget.gb_widget_create_popup_menu = gb_label_create_popup_menu;
 */

  return &gbwidget;
}
