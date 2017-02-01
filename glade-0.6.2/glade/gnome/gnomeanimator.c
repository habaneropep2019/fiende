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
#include "../graphics/gnome-animator.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *LoopType = "GnomeAnimator::loop_type";
static gchar *Direction = "GnomeAnimator::playback_direction";
static gchar *Speed = "GnomeAnimator::playback_speed";


static const gchar *GbLoopTypeChoices[] =
{
  "None",
  "Restart",
  "Ping-Pong",
  NULL
};
static const gint GbLoopTypeValues[] =
{
  GNOME_ANIMATOR_LOOP_NONE,
  GNOME_ANIMATOR_LOOP_RESTART,
  GNOME_ANIMATOR_LOOP_PING_PONG
};
static const gchar *GbLoopTypeSymbols[] =
{
  "GNOME_ANIMATOR_LOOP_NONE",
  "GNOME_ANIMATOR_LOOP_RESTART",
  "GNOME_ANIMATOR_LOOP_PING_PONG"
};

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeAnimator, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
static GtkWidget*
gb_gnome_animator_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  new_widget = gnome_animator_new_with_size (100, 100);

  /* GnomeAnimator widgets must have a width & height so we set the flags. */
  data->widget_data->flags |= GLADE_WIDTH_SET | GLADE_HEIGHT_SET;
  data->widget_data->width = 100;
  data->widget_data->height = 100;

  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_animator_create_properties (GtkWidget * widget,
				     GbWidgetCreateArgData * data)
{
  property_add_choice (LoopType, _("Loop Type:"),
		       _("If the animation loops, and in what way"),
		       GbLoopTypeChoices);
  property_add_bool (Direction, _("Reverse:"),
		     _("If the initial direction is reversed"));
  property_add_float (Speed, _("Speed:"),
		      _("The playback speed"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_animator_get_properties (GtkWidget *widget,
				  GbWidgetGetArgData * data)
{
  gint i;

  for (i = 0; i < sizeof (GbLoopTypeValues) / sizeof (GbLoopTypeValues[0]);
       i++)
    {
      if (GbLoopTypeValues[i] == GNOME_ANIMATOR (widget)->loop_type)
	gb_widget_output_choice (data, LoopType, i, GbLoopTypeSymbols[i]);
    }
  gb_widget_output_bool (data, Direction,
			 GNOME_ANIMATOR (widget)->playback_direction >= 0
			 ? FALSE : TRUE);
  gb_widget_output_float (data, Speed,
			  GNOME_ANIMATOR (widget)->playback_speed);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_gnome_animator_set_properties (GtkWidget * widget,
				  GbWidgetSetArgData * data)
{
  gchar *loop_type;
  gboolean direction;
  gfloat speed;
  gint i;

  loop_type = gb_widget_input_choice (data, LoopType);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbLoopTypeValues) / sizeof (GbLoopTypeValues[0]);
	   i++)
	{
	  if (!strcmp (loop_type, GbLoopTypeChoices[i])
	      || !strcmp (loop_type, GbLoopTypeSymbols[i]))
	    {
	      gnome_animator_set_loop_type (GNOME_ANIMATOR (widget),
					    GbLoopTypeValues[i]);
	      break;
	    }
	}
    }

  direction = gb_widget_input_bool (data, Direction);
  if (data->apply)
    {
      gnome_animator_set_loop_type (GNOME_ANIMATOR (widget),
				    direction ? -1 : 1);
    }

  speed = gb_widget_input_float (data, Speed);
  if (data->apply)
    {
      if (speed <= 0)
	speed = 1;
      gnome_animator_set_playback_speed (GNOME_ANIMATOR (widget), speed);
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeAnimator, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_animator_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_animator_write_source (GtkWidget * widget,
				GbWidgetWriteSourceData * data)
{
  gint i;

  if (data->create_widget)
    {
      source_add (data, "  %s = gnome_animator_new_with_size (%i, %i);\n",
		  data->wname, data->widget_data->width,
		  data->widget_data->height);
    }

  gb_widget_write_standard_source (widget, data);

  if (GNOME_ANIMATOR (widget)->loop_type != GNOME_ANIMATOR_LOOP_NONE)
    {
      for (i = 0; i < sizeof (GbLoopTypeValues) / sizeof (GbLoopTypeValues[0]);
	   i++)
	{
	  if (GbLoopTypeValues[i] == GNOME_ANIMATOR (widget)->loop_type)
	    source_add (data,
			"  gnome_animator_set_loop_type (GNOME_ANIMATOR (%s), %s);\n",
			data->wname, GbLoopTypeSymbols[i]);
	}
    }

  if (GNOME_ANIMATOR (widget)->playback_direction < 0)
    {
      source_add (data,
		  "  gnome_animator_set_playback_direction (GNOME_ANIMATOR (%s), -1);\n",
		  data->wname);
    }

  if (GNOME_ANIMATOR (widget)->playback_speed != 1)
    {
      source_add (data,
		  "  gnome_animator_set_playback_speed (GNOME_ANIMATOR (%s), %g);\n",
		  data->wname, GNOME_ANIMATOR (widget)->playback_speed);
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_animator_init ()
{
  /* Initialise the GTK type */
  gnome_animator_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_animator_xpm;
  gbwidget.tooltip = _("Gnome Animator");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_animator_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_animator_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_animator_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_animator_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_animator_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_animator_create_popup_menu;
*/

  return &gbwidget;
}

