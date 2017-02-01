/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998  Damon Chaplin
 *
 *  Update 3/17/99 - Jay Johnston:
 *  - added property adjustments
 *    - style
 *    - orientation
 *    - activity mode
 *    - show text
 *  - added sample animation (most of that code was taken from the Gtk v1.2
 *      tutorial by Tony Gale <gale@gtk.org> and Ian Main <imain@gtk.org> )
 *  - added dynamic popup menus which change depending on whether the animation
 *      is active or not
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

#include <gtk/gtkprogressbar.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/progressbar.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Value = "ProgressBar|GtkProgress::value";
static gchar *MinValue = "ProgressBar|GtkProgress::lower";
static gchar *MaxValue = "ProgressBar|GtkProgress::upper";
static gchar *ActivityMode = "ProgressBar|GtkProgress::activity_mode";
static gchar *Style = "GtkProgressBar::bar_style";
static gchar *Orientation = "GtkProgressBar::orientation";
static gchar *ShowText = "ProgressBar|GtkProgress::show_text";
static gchar *XAlign = "GtkProgressBar::text_xalign";
static gchar *YAlign = "GtkProgressBar::text_yalign";
static gchar *Format = "GtkProgressBar::format";


static const gchar *GbStyleChoices[] =
{"Continuous", "Discrete", NULL};

static const gint GbStyleValues[] =
{
  GTK_PROGRESS_CONTINUOUS,
  GTK_PROGRESS_DISCRETE
};
static const gchar *GbStyleSymbols[] =
{
  "GTK_PROGRESS_CONTINUOUS",
  "GTK_PROGRESS_DISCRETE"
};


static const gchar *GbOrientationChoices[] =
{"Left to Right", "Right to Left", "Bottom to Top", "Top to Bottom", NULL};

static const gint GbOrientationValues[] =
{
  GTK_PROGRESS_LEFT_TO_RIGHT,
  GTK_PROGRESS_RIGHT_TO_LEFT,
  GTK_PROGRESS_BOTTOM_TO_TOP,
  GTK_PROGRESS_TOP_TO_BOTTOM
};
static const gchar *GbOrientationSymbols[] =
{
  "GTK_PROGRESS_LEFT_TO_RIGHT",
  "GTK_PROGRESS_RIGHT_TO_LEFT",
  "GTK_PROGRESS_BOTTOM_TO_TOP",
  "GTK_PROGRESS_TOP_TO_BOTTOM"
};


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkProgressBar, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_progress_bar_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;

  new_widget = gtk_progress_bar_new ();
  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */

static void
gb_progress_bar_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_float (Value, _("Value:"), _("The initial value"));
  property_add_float (MinValue, _("Minimum Value:"), _("The minimum value"));
  property_add_float (MaxValue, _("Maximum Value:"), _("The maximum value"));

  property_add_choice (Style, _("Style:"),
		       _("The style of the progress bar"),
		       GbStyleChoices);
  property_add_choice (Orientation, _("Orientation:"),
		       _("The orientation of the progress bar's contents"),
		       GbOrientationChoices);

  property_add_bool (ActivityMode, _("Activity Mode:"), 
                     _("If the progress bar should act like the front of Kit's car"));
  property_add_bool (ShowText, _("Show Text:"), 
                     _("If the text should be shown in the progress bar"));
  property_add_string (Format, _("Format:"),
		       _("The format of the text, similar to sprintf. "
			 "Use %P, %V, %L and %U to insert the percentage, "
			 "value, lower bound and upper bound respectively. "
			 "Use %1V or %2V to show 1 or 2 decimal places."));
  property_add_float_range (XAlign, _("X Align:"),
			    _("The horizontal alignment of the text"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_float_range (YAlign, _("Y Align:"),
			    _("The vertical alignment of the text"),
			    0, 1, 0.01, 0.1, 0.01, 2);
}
 



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */

static void
gb_progress_bar_get_properties(GtkWidget *widget, GbWidgetGetArgData * data)
{
  GtkAdjustment *adjustment;
  gint i;

  adjustment = GTK_PROGRESS (widget)->adjustment;

  gb_widget_output_float (data, Value, adjustment->value);
  gb_widget_output_float (data, MinValue, adjustment->lower);
  gb_widget_output_float (data, MaxValue, adjustment->upper);

  for (i = 0; i < sizeof (GbStyleValues) / sizeof (GbStyleValues[0]); i++)
    {
      if (GbStyleValues[i] == GTK_PROGRESS_BAR (widget)->bar_style)
	gb_widget_output_choice (data, Style, i, GbStyleSymbols[i]);
    }

  for (i = 0; i < sizeof (GbOrientationValues) / sizeof (GbOrientationValues[0]); i++)
    {
      if (GbOrientationValues[i] == GTK_PROGRESS_BAR (widget)->orientation)
	gb_widget_output_choice (data, Orientation, i, GbOrientationSymbols[i]);
    }
  
    gb_widget_output_bool (data, ActivityMode, GTK_PROGRESS (widget)->activity_mode);

    gb_widget_output_bool (data, ShowText, GTK_PROGRESS (widget)->show_text);
    gb_widget_output_translatable_string (data, Format,
					  GTK_PROGRESS (widget)->format);
    gb_widget_output_float (data, XAlign, GTK_PROGRESS (widget)->x_align);
    gb_widget_output_float (data, YAlign, GTK_PROGRESS (widget)->y_align);
}




/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */

static void
gb_progress_bar_set_properties(GtkWidget * widget, GbWidgetSetArgData * data)
{
  GtkAdjustment *adjustment;
  gfloat value, min, max, xalign, yalign;
  gboolean activityMode, showText;
  gboolean configure_progress = FALSE, set_alignment = FALSE;
  gchar *style, *orientation, *format;
  gint i;
 
  adjustment = GTK_PROGRESS (widget)->adjustment;

  value = gb_widget_input_float (data, Value);
  if (data->apply)
    configure_progress = TRUE;
  else
    value = adjustment->value;

  min = gb_widget_input_float (data, MinValue);
  if (data->apply)
    configure_progress = TRUE;
  else
    min = adjustment->lower;

  max = gb_widget_input_float (data, MaxValue);
  if (data->apply)
    configure_progress = TRUE;
  else
    max = adjustment->upper;

  if (configure_progress)
    {
      /* Make sure we use valid values. */
      max = MAX (max, min);
      value = MAX (value, min);
      value = MIN (value, max);
      gtk_progress_configure (GTK_PROGRESS (widget), value, min, max);
    }


  style = gb_widget_input_choice (data, Style);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbStyleValues) / sizeof (GbStyleValues[0]); i++)
	{
	  if (!strcmp (style, GbStyleChoices[i])
	      || !strcmp (style, GbStyleSymbols[i]))
	    {
	      gtk_progress_bar_set_bar_style (GTK_PROGRESS_BAR (widget),
					      GbStyleValues[i]);
	      /* FIXME: GTK+ 1.2 workaround. If we don't do this it doesn't
		 recalculate the number of blocks to show. */
	      gtk_adjustment_value_changed (GTK_PROGRESS (widget)->adjustment);
	      break;
	    }
	}
    }

  orientation = gb_widget_input_choice (data, Orientation);

  if (data->apply)
    {
      for (i = 0;
	   i < sizeof (GbOrientationValues) / sizeof (GbOrientationValues[0]);
	   i++)
	{
	  if (!strcmp (orientation, GbOrientationChoices[i])
	      || !strcmp (orientation, GbOrientationSymbols[i]))
	    {
	      gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (widget),
						GbOrientationValues[i]);
	      break;
	    }
	}
    }

  activityMode = gb_widget_input_bool (data, ActivityMode);
  if (data->apply)
    gtk_progress_set_activity_mode (GTK_PROGRESS (widget), activityMode);

  showText = gb_widget_input_bool (data, ShowText);
  if (data->apply)
    gtk_progress_set_show_text (GTK_PROGRESS (widget), showText);

  format = gb_widget_input_string (data, Format);
  if (data->apply)
    gtk_progress_set_format_string (GTK_PROGRESS (widget), format);

  xalign = gb_widget_input_float (data, XAlign);
  if (data->apply)
    set_alignment = TRUE;
  else
    xalign = GTK_PROGRESS (widget)->x_align;

  yalign = gb_widget_input_float (data, YAlign);
  if (data->apply)
    set_alignment = TRUE;
  else
    yalign = GTK_PROGRESS (widget)->y_align;

  if (set_alignment)
    gtk_progress_set_text_alignment (GTK_PROGRESS (widget), xalign, yalign);
}


/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_progress_bar_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GtkAdjustment *adjustment;
  gint i;

  adjustment = GTK_PROGRESS (widget)->adjustment;

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_progress_bar_new ();\n", data->wname);
    }
  gb_widget_write_standard_source (widget, data);

  if (adjustment->value != 0 || adjustment->lower != 0
      || adjustment->upper != 100)
    {
      source_add (data,
		  "  gtk_progress_configure (GTK_PROGRESS (%s), %g, %g, %g);\n", 
		  data->wname, adjustment->value,
		  adjustment->lower, adjustment->upper);
    }

  if (GTK_PROGRESS_BAR (widget)->bar_style != GTK_PROGRESS_CONTINUOUS)
    {
      for (i = 0; i < sizeof (GbStyleValues) / sizeof (GbStyleValues[0]); i++)
        {
          if (GbStyleValues[i] == GTK_PROGRESS_BAR (widget)->bar_style)
            source_add (data,
		    "  gtk_progress_bar_set_bar_style (GTK_PROGRESS_BAR (%s), %s);\n", 
                        data->wname, GbStyleSymbols[i]);
        }
    }

  if (GTK_PROGRESS_BAR (widget)->orientation != GTK_PROGRESS_LEFT_TO_RIGHT)
    {
      for (i = 0; i < sizeof (GbOrientationValues) / sizeof (GbOrientationValues[0]); i++)
        {
          if (GbOrientationValues[i] == GTK_PROGRESS_BAR (widget)->orientation)
            source_add (data,
		    "  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (%s), %s);\n", 
                        data->wname, GbOrientationSymbols[i]);
        }
    }

  if (GTK_PROGRESS (widget)->activity_mode)
    {
      source_add( data,
                  "  gtk_progress_set_activity_mode (GTK_PROGRESS (%s), TRUE);\n",
                  data->wname);
    }

  if (GTK_PROGRESS (widget)->show_text)
    {
      source_add( data,
                  "  gtk_progress_set_show_text (GTK_PROGRESS (%s), TRUE);\n",
                  data->wname);
    }

  if (strcmp (GTK_PROGRESS (widget)->format, "%P %%"))
    {
      source_add( data,
                  "  gtk_progress_set_format_string (GTK_PROGRESS (%s), %s);\n",
                  data->wname,
		  source_make_string (GTK_PROGRESS (widget)->format,
				      data->use_gettext));
    }

  if (GTK_PROGRESS (widget)->x_align != 0.5
      || GTK_PROGRESS (widget)->y_align != 0.5)
    {
      source_add( data,
                  "  gtk_progress_set_text_alignment (GTK_PROGRESS (%s), %g, %g);\n",
                  data->wname,
		  GTK_PROGRESS (widget)->x_align,
		  GTK_PROGRESS (widget)->y_align);
    }
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_progress_bar_init ()
{
  /* Initialise the GTK type */
  gtk_progress_bar_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = progressbar_xpm;
  gbwidget.tooltip = _("Progress Bar");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_progress_bar_new;

  gbwidget.gb_widget_create_properties = gb_progress_bar_create_properties;
  gbwidget.gb_widget_get_properties    = gb_progress_bar_get_properties;
  gbwidget.gb_widget_set_properties    = gb_progress_bar_set_properties;
  /*gbwidget.gb_widget_create_popup_menu = gb_progress_bar_create_popup_menu;*/
  gbwidget.gb_widget_write_source      = gb_progress_bar_write_source;

  return &gbwidget;
}
