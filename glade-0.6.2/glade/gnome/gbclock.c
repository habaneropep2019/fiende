/*  Gtk+ User Interface Builder
 *  Copyright (C) 1999  Damon Chaplin
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

#include <config.h>

#include <gnome.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/gtk-clock.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *ClockType = "GtkClock::type";
static gchar *ClockFormat = "GtkClock::format";
static gchar *ClockSeconds = "GtkClock::seconds";
static gchar *ClockInterval = "GtkClock::interval";

static const gchar *GbClockTypeChoices[] =
{
  "Increasing",
  "Decreasing",
  "Real Time",
  NULL
};
static const gint GbClockTypeValues[] =
{
  GTK_CLOCK_INCREASING,
  GTK_CLOCK_DECREASING,
  GTK_CLOCK_REALTIME
};
static const gchar *GbClockTypeSymbols[] =
{
  "GTK_CLOCK_INCREASING",
  "GTK_CLOCK_DECREASING",
  "GTK_CLOCK_REALTIME"
};


static GtkWidget *start_button, *stop_button;

static void gb_clock_on_start (GtkWidget *button,
			       gpointer data);
static void gb_clock_on_stop (GtkWidget *button,
			      gpointer data);

static void gtk_clock_gen_str(GtkClock *clock);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkClock, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
static GtkWidget*
gb_clock_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  /* FIXME: Need to load type property before creating clock. */

  new_widget = gtk_clock_new (GTK_CLOCK_REALTIME);
  /* GnomeLibs 1.0.5 bug workaround - it doesn't set the type in new(). */
  GTK_CLOCK (new_widget)->type = GTK_CLOCK_REALTIME;
  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_clock_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  GtkWidget *property_table, *hbox;
  gint property_table_row;

  property_add_choice (ClockType, _("Type:"), _("The type of the clock"),
		       GbClockTypeChoices);
  property_add_string (ClockFormat, _("Format:"),
		       _("The format of the time to display. Use %H, %M and %S to insert the hour, minute and seconds values"));
  property_add_int_range (ClockSeconds, _("Seconds:"), _("The starting time"),
			  0, 10000, 1, 10, 10);
  property_add_int_range (ClockInterval, _("Interval:"),
			  _("The time between updates, in seconds"),
			  0, 10000, 1, 10, 1);

  /* Add buttons to start/stop the clock. */
  property_table = property_get_table_position (&property_table_row);
  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);

  start_button = gtk_button_new_with_label (_("Start"));
  gtk_widget_show (start_button);
  gtk_signal_connect (GTK_OBJECT (start_button), "clicked",
		      GTK_SIGNAL_FUNC (gb_clock_on_start), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), start_button, TRUE, TRUE, 0);

  stop_button = gtk_button_new_with_label (_("Stop"));
  gtk_widget_show (stop_button);
  gtk_signal_connect (GTK_OBJECT (stop_button), "clicked",
		      GTK_SIGNAL_FUNC (gb_clock_on_stop), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), stop_button, TRUE, TRUE, 0);

  gtk_table_attach (GTK_TABLE (property_table), hbox, 1, 3,
		    property_table_row, property_table_row + 1,
		    GTK_FILL, GTK_FILL, 10, 10);
}


static void
gb_clock_on_start (GtkWidget *button,
		   gpointer data)
{
  GtkWidget *clock;
  clock = property_get_widget ();
  g_return_if_fail (GTK_IS_CLOCK (clock));
  gtk_clock_start (GTK_CLOCK (clock));

  gtk_widget_set_sensitive (start_button, FALSE);
  gtk_widget_set_sensitive (stop_button, TRUE);
}


static void
gb_clock_on_stop (GtkWidget *button,
		  gpointer data)
{
  GtkWidget *clock;
  gint seconds;

  clock = property_get_widget ();
  g_return_if_fail (GTK_IS_CLOCK (clock));
  gtk_clock_stop (GTK_CLOCK (clock));

  /* We reset the seconds to the value the user set. */
  seconds = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (clock),
						  ClockSeconds));
  if (GTK_CLOCK (clock)->type == GTK_CLOCK_REALTIME)
    seconds = 0;
  gtk_clock_set_seconds (GTK_CLOCK (clock), (time_t) seconds);

  gtk_widget_set_sensitive (start_button, TRUE);
  gtk_widget_set_sensitive (stop_button, FALSE);
}


/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_clock_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gint i, seconds, interval;
  gboolean seconds_sensitive = TRUE, start_sensitive = TRUE;

  for (i = 0; i < sizeof (GbClockTypeValues) / sizeof (GbClockTypeValues[0]);
       i++)
    {
      if (GbClockTypeValues[i] == GTK_CLOCK (widget)->type)
	gb_widget_output_choice (data, ClockType, i, GbClockTypeSymbols[i]);
    }

  gb_widget_output_translatable_string (data, ClockFormat,
					GTK_CLOCK (widget)->fmt);

  seconds = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget),
						  ClockSeconds));
  interval = GTK_CLOCK (widget)->update_interval;

  if (GTK_CLOCK (widget)->type == GTK_CLOCK_REALTIME)
    {
      seconds = 0;
      interval = 60;
      seconds_sensitive = FALSE;
    }
  gb_widget_output_int (data, ClockSeconds, seconds);
  gb_widget_output_int (data, ClockInterval, interval);

  if (data->action == GB_SHOWING)
    {
      property_set_sensitive (ClockSeconds, seconds_sensitive);
      property_set_sensitive (ClockInterval, seconds_sensitive);

      if (GTK_CLOCK (widget)->timer_id != -1)
	start_sensitive = FALSE;
      gtk_widget_set_sensitive (start_button, start_sensitive);
      gtk_widget_set_sensitive (stop_button, !start_sensitive);
    }
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_clock_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *type, *format;
  gint i, seconds, interval;
  gboolean update_label = FALSE, seconds_sensitive = TRUE;

  type = gb_widget_input_choice (data, ClockType);
  if (data->apply)
    {
      for (i = 0;
	   i < sizeof (GbClockTypeValues) / sizeof (GbClockTypeValues[0]);
	   i++)
	{
	  if (!strcmp (type, GbClockTypeChoices[i])
	      || !strcmp (type, GbClockTypeSymbols[i]))
	    {
	      /* These is no function to change the type, so we have to mess
		 around a bit. */
	      GTK_CLOCK (widget)->type = GbClockTypeValues[i];

	      if (data->action == GB_APPLYING)
		{
		  if (GTK_CLOCK (widget)->type == GTK_CLOCK_REALTIME)
		    seconds_sensitive = FALSE;
		  property_set_sensitive (ClockSeconds, seconds_sensitive);
		  property_set_sensitive (ClockInterval, seconds_sensitive);
		}

	      update_label = TRUE;
	      break;
	    }
	}
    }

  format = gb_widget_input_string (data, ClockFormat);
  if (data->apply)
    {
      gtk_clock_set_format (GTK_CLOCK (widget), format);
      update_label = TRUE;
    }

  seconds = gb_widget_input_int (data, ClockSeconds);
  if (data->apply)
    {
      gtk_clock_set_seconds (GTK_CLOCK (widget), (time_t) seconds);
      gtk_object_set_data (GTK_OBJECT (widget), ClockSeconds,
			   GINT_TO_POINTER (seconds));
      update_label = TRUE;
    }

  interval = gb_widget_input_int (data, ClockInterval);
  if (data->apply)
    gtk_clock_set_update_interval (GTK_CLOCK (widget), interval);

  if (update_label)
    gtk_clock_gen_str (GTK_CLOCK (widget));
}


/* This is copied from gtk-clock.c so we can update the label ourself. */
static void
gtk_clock_gen_str(GtkClock *clock)
{
	gchar timestr[64];
	time_t secs;

	switch (clock->type) {
	case GTK_CLOCK_DECREASING:
                secs = clock->seconds-time(NULL);
		break;
	case GTK_CLOCK_INCREASING:
                secs = time(NULL)-clock->seconds;
		break;
	case GTK_CLOCK_REALTIME:
		secs = time(NULL);
		break;
	}

	if (clock->type == GTK_CLOCK_REALTIME) {
		clock->tm = localtime(&secs);
	} else {
		clock->tm->tm_hour = secs/3600;
		secs -= clock->tm->tm_hour*3600;
		clock->tm->tm_min = secs/60;
		clock->tm->tm_sec = secs - clock->tm->tm_min*60;
	}
	
	strftime(timestr, 64, clock->fmt, clock->tm);
	gtk_label_set_text(GTK_LABEL(clock), timestr);
}


/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkClock, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_clock_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_clock_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GtkClockType type;
  gint type_index = 0;
  gchar *format;
  gint seconds, interval, i;
  gboolean set_format = FALSE, set_interval = FALSE, set_seconds = FALSE;

  type = GTK_CLOCK (widget)->type;
  format = GTK_CLOCK (widget)->fmt;
  /* We don't allow large values to be set for seconds so I think we should be
     OK here. */
  seconds = (gint) (GTK_CLOCK (widget)->seconds - time (NULL));
  interval = GTK_CLOCK (widget)->update_interval;

  if (data->create_widget)
    {
      for (i = 0;
	   i < sizeof (GbClockTypeValues) / sizeof (GbClockTypeValues[0]);
	   i++)
	{
	  if (GbClockTypeValues[i] == type)
	    type_index = i;
	}

      source_add (data, "  %s = gtk_clock_new (%s);\n", data->wname,
		  GbClockTypeSymbols[type_index]);
    }

  gb_widget_write_standard_source (widget, data);


  if (type == GTK_CLOCK_REALTIME)
    {
      if (strcmp (format, "%H:%M"))
	set_format = TRUE;
      if (interval != 60)
	set_interval = TRUE;
    }
  else
    {
      if (strcmp (format, "%H:%M:%S"))
	set_format = TRUE;
      if (interval != 1)
	set_interval = TRUE;
    }

  if (set_format)
    source_add (data, "  gtk_clock_set_format (GTK_CLOCK (%s), %s);\n",
		data->wname,
		source_make_string (format, data->use_gettext));
  if (set_seconds)
    source_add (data,
		"  gtk_clock_set_seconds (GTK_CLOCK (%s), (time_t) %i);\n",
		data->wname, seconds);
  if (set_interval)
    source_add (data,
		"  gtk_clock_set_update_interval (GTK_CLOCK (%s), %i);\n",
		data->wname, interval);
}


/* GnomeLibs workaround - it doesn't stop a realtime clock when destroyed!. */
static void
gb_clock_destroy (GtkWidget * widget, GbWidgetDestroyData * data)
{
  if (GTK_CLOCK (widget)->timer_id != -1)
    {
      gtk_timeout_remove (GTK_CLOCK (widget)->timer_id);
      GTK_CLOCK (widget)->timer_id = -1;
    }
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_clock_init ()
{
  /* Initialise the GTK type */
  gtk_clock_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gtk_clock_xpm;
  gbwidget.tooltip = _("Clock");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_clock_new;
  gbwidget.gb_widget_create_properties	= gb_clock_create_properties;
  gbwidget.gb_widget_get_properties	= gb_clock_get_properties;
  gbwidget.gb_widget_set_properties	= gb_clock_set_properties;
  gbwidget.gb_widget_write_source	= gb_clock_write_source;
  gbwidget.gb_widget_destroy		= gb_clock_destroy;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_clock_create_popup_menu;
*/

  return &gbwidget;
}

