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

#include <gtk/gtkaccellabel.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/accellabel.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "AccelLabel|GtkLabel::label";
static gchar *Justify = "AccelLabel|GtkLabel::justify";
static gchar *Wrap = "AccelLabel|GtkLabel::wrap";
static gchar *XAlign = "AccelLabel|GtkMisc::xalign";
static gchar *YAlign = "AccelLabel|GtkMisc::yalign";
static gchar *XPad = "AccelLabel|GtkMisc::xpad";
static gchar *YPad = "AccelLabel|GtkMisc::ypad";

/* Note: Fill doesn't currently work with labels */
static const gchar *GbJustifyChoices[] =
{"Left", "Right", "Center", "Fill", NULL};
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


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the funtion in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkAccelLabel, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget*
gb_accel_label_new (GbWidgetNewData *data)
{
  return gtk_accel_label_new (data->name);
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_accel_label_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
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
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_accel_label_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gchar *label_text;
  gint i;

  label_text = glade_util_get_label_text (widget);
  gb_widget_output_translatable_text (data, Label, label_text);
  g_free (label_text);

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
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_accel_label_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *label, *justify;
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
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkAccelLabel, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_accel_label_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_accel_label_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gchar *label_text;
  gint i;

  if (data->create_widget)
    {
      gtk_label_get (GTK_LABEL (widget), &label_text);
      source_add (data, "  %s = gtk_accel_label_new (%s);\n", data->wname,
		  source_make_string (label_text, data->use_gettext));
    }

  gb_widget_write_standard_source (widget, data);

  if (GTK_LABEL (widget)->jtype != GTK_JUSTIFY_CENTER)
    {
      for (i = 0; i < sizeof (GbJustifyValues) / sizeof (GbJustifyValues[0]);
	   i++)
	{
	  if (GbJustifyValues[i] == GTK_LABEL (widget)->jtype)
	    source_add (data, "  gtk_label_set_justify (GTK_LABEL (%s), %s);\n",
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
GbWidget*
gb_accel_label_init ()
{
  /* Initialise the GTK type */
  gtk_accel_label_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = accellabel_xpm;
  gbwidget.tooltip = _("Label with Accelerator");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_accel_label_new;
  gbwidget.gb_widget_create_properties	= gb_accel_label_create_properties;
  gbwidget.gb_widget_get_properties	= gb_accel_label_get_properties;
  gbwidget.gb_widget_set_properties	= gb_accel_label_set_properties;
  gbwidget.gb_widget_write_source	= gb_accel_label_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_accel_label_create_popup_menu;
*/

  return &gbwidget;
}

