
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

#include <config.h>

#ifdef USE_GNOME
#include <gnome.h>
#include "../glade_gnome.h"
#else
#include <gtk/gtkhbbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkspinbutton.h>
#endif

#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/hbuttonbox.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Size = "HBBox|GtkBox::size";
static gchar *Layout = "HBBox|GtkButtonBox::layout_style";
static gchar *Spacing = "HBBox|GtkButtonBox::spacing";
static gchar *Width = "HBBox|GtkButtonBox::child_min_width";
static gchar *Height = "HBBox|GtkButtonBox::child_min_height";
static gchar *XPad = "HBBox|GtkButtonBox::child_ipad_x";
static gchar *YPad = "HBBox|GtkButtonBox::child_ipad_y";

static const gchar *GbLayoutChoices[] =
{"Default", "Spread", "Edge",
 "Start", "End", NULL};
static const gint GbLayoutValues[] =
{
  GTK_BUTTONBOX_DEFAULT_STYLE,
  GTK_BUTTONBOX_SPREAD,
  GTK_BUTTONBOX_EDGE,
  GTK_BUTTONBOX_START,
  GTK_BUTTONBOX_END
};
static const gchar *GbLayoutSymbols[] =
{
  "GTK_BUTTONBOX_DEFAULT_STYLE",
  "GTK_BUTTONBOX_SPREAD",
  "GTK_BUTTONBOX_EDGE",
  "GTK_BUTTONBOX_START",
  "GTK_BUTTONBOX_END"
};

static void show_hbbox_dialog (GbWidgetNewData * data);
static void on_hbbox_dialog_ok (GtkWidget * widget,
				GbWidgetNewData * data);
static void on_hbbox_dialog_destroy (GtkWidget * widget,
				     GbWidgetNewData * data);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkHButtonBox, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_hbutton_box_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;

  if (data->action == GB_LOADING)
    {
      new_widget = gtk_hbutton_box_new ();
      return new_widget;
    }
  else
    {
      show_hbbox_dialog (data);
      return NULL;
    }
}


static void
show_hbbox_dialog (GbWidgetNewData * data)
{
  GtkWidget *dialog, *vbox, *hbox, *label, *spinbutton;
  GtkObject *adjustment;

  dialog = glade_util_create_dialog (_("New horizontal button box"),
				     data->parent, on_hbbox_dialog_ok, data,
				     &vbox);
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      GTK_SIGNAL_FUNC (on_hbbox_dialog_destroy), data);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Number of columns:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 5);
  gtk_widget_show (label);

  adjustment = gtk_adjustment_new (3, 1, 100, 1, 10, 10);
  spinbutton = glade_util_spin_button_new (GTK_OBJECT (dialog), "cols",
					   GTK_ADJUSTMENT (adjustment), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, TRUE, TRUE, 5);
  gtk_widget_set_usize (spinbutton, 50, -1);
  gtk_widget_grab_focus (spinbutton);
  gtk_widget_show (spinbutton);

  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
}


static void
on_hbbox_dialog_ok (GtkWidget * widget, GbWidgetNewData * data)
{
  GtkWidget *new_widget, *spinbutton, *window, *new_child;
  gint cols, i;

  window = gtk_widget_get_toplevel (widget);

  /* Only call callback if placeholder/fixed widget is still there */
  if (gb_widget_can_finish_new (data))
    {
      spinbutton = gtk_object_get_data (GTK_OBJECT (window), "cols");
      g_return_if_fail (spinbutton != NULL);
      cols = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

      new_widget = gtk_hbutton_box_new ();
      for (i = 0; i < cols; i++)
	{
	  new_child = gb_widget_new ("GtkButton", new_widget);
	  GTK_WIDGET_SET_FLAGS (new_child, GTK_CAN_DEFAULT);
	  gtk_container_add (GTK_CONTAINER (new_widget), new_child);
	}
      gb_widget_initialize (new_widget, data);
      (*data->callback) (new_widget, data);
    }
  gtk_widget_destroy (window);
}


static void
on_hbbox_dialog_destroy (GtkWidget * widget,
			 GbWidgetNewData * data)
{
  gb_widget_free_new_data (data);
  gtk_grab_remove (widget);
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_hbutton_box_create_properties (GtkWidget * widget, GbWidgetCreateArgData *
				  data)
{
  property_add_int_range (Size, _("Size:"), _("The number of buttons"),
			  0, 1000, 1, 10, 1);
  property_add_choice (Layout, _("Layout:"),
		       _("The layout style of the buttons"),
		       GbLayoutChoices);
  property_add_int_range (Spacing, _("Spacing:"), _("The space between the buttons"),
			  0, 1000, 1, 10, 1);
  property_add_int_range (Width, _("Min Width:"),
			  _("The minimum width of the buttons"),
			  0, 1000, 1, 10, 1);
  property_add_int_range (Height, _("Min Height:"),
			  _("The minimum height of the buttons"),
			  0, 1000, 1, 10, 1);
  property_add_int_range (XPad, _("X Pad:"),
			  _("The horizontal padding of the buttons"),
			  0, 1000, 1, 10, 1);
  property_add_int_range (YPad, _("Y Pad:"),
			  _("The vertical padding of the buttons"),
			  0, 1000, 1, 10, 1);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_hbutton_box_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  GtkButtonBoxStyle layout;
  gint i, spacing, min_width, min_height, ipad_x, ipad_y;
  gint default_spacing;
  gint default_min_width, default_min_height, default_ipad_x, default_ipad_y;

  if (data->action != GB_SAVING)
    gb_widget_output_int (data, Size, g_list_length (GTK_BOX (widget)->children));

  layout = gtk_button_box_get_layout (GTK_BUTTON_BOX (widget));
  for (i = 0; i < sizeof (GbLayoutValues) / sizeof (GbLayoutValues[0]); i++)
    {
      if (GbLayoutValues[i] == layout)
	gb_widget_output_choice (data, Layout, i, GbLayoutSymbols[i]);
    }

  spacing = gtk_button_box_get_spacing (GTK_BUTTON_BOX (widget));
  default_spacing = gtk_hbutton_box_get_spacing_default ();
  if (spacing == GTK_BUTTONBOX_DEFAULT)
    spacing = default_spacing;
  gb_widget_output_int (data, Spacing, spacing);

  gtk_button_box_get_child_size_default (&default_min_width,
					 &default_min_height);
  gtk_button_box_get_child_size (GTK_BUTTON_BOX (widget), &min_width,
				 &min_height);
  if (min_width == GTK_BUTTONBOX_DEFAULT)
    min_width = default_min_width;
  if (min_height == GTK_BUTTONBOX_DEFAULT)
    min_height = default_min_height;
  gb_widget_output_int (data, Width, min_width);
  gb_widget_output_int (data, Height, min_height);

  gtk_button_box_get_child_ipadding_default (&default_ipad_x, &default_ipad_y);
  gtk_button_box_get_child_ipadding (GTK_BUTTON_BOX (widget), &ipad_x, &ipad_y);
  if (ipad_x == GTK_BUTTONBOX_DEFAULT)
    ipad_x = default_ipad_x;
  if (ipad_y == GTK_BUTTONBOX_DEFAULT)
    ipad_y = default_ipad_y;
  gb_widget_output_int (data, XPad, ipad_x);
  gb_widget_output_int (data, YPad, ipad_y);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_hbutton_box_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gint size, i, spacing, min_width, min_height, ipad_x, ipad_y;
  gchar *layout;
  gboolean set_child_size = FALSE, set_child_padding = FALSE;
  gboolean queue_resize = FALSE;

  size = gb_widget_input_int (data, Size);
  if (data->apply)
    gb_box_set_size (widget, size);

  layout = gb_widget_input_choice (data, Layout);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbLayoutValues) / sizeof (GbLayoutValues[0]);
	   i++)
	{
	  if (!strcmp (layout, GbLayoutChoices[i])
	      || !strcmp (layout, GbLayoutSymbols[i]))
	    {
	      gtk_button_box_set_layout (GTK_BUTTON_BOX (widget), GbLayoutValues
					 [i]);
	      queue_resize = TRUE;
	      break;
	    }
	}
    }

  spacing = gb_widget_input_int (data, Spacing);
  if (data->apply)
    {
      gtk_button_box_set_spacing (GTK_BUTTON_BOX (widget), spacing);
      queue_resize = TRUE;
    }

  min_width = gb_widget_input_int (data, Width);
  if (data->apply)
    set_child_size = TRUE;
  else
    min_width = GTK_BUTTON_BOX (widget)->child_min_width;

  min_height = gb_widget_input_int (data, Height);
  if (data->apply)
    set_child_size = TRUE;
  else
    min_height = GTK_BUTTON_BOX (widget)->child_min_height;

  if (set_child_size)
    {
      gtk_button_box_set_child_size (GTK_BUTTON_BOX (widget),
				     min_width, min_height);
      queue_resize = TRUE;
    }

  ipad_x = gb_widget_input_int (data, XPad);
  if (data->apply)
    set_child_padding = TRUE;
  else
    ipad_x = GTK_BUTTON_BOX (widget)->child_ipad_x;

  ipad_y = gb_widget_input_int (data, YPad);
  if (data->apply)
    set_child_padding = TRUE;
  else
    ipad_y = GTK_BUTTON_BOX (widget)->child_ipad_y;

  if (set_child_padding)
    {
      gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (widget),
					 ipad_x, ipad_y);
      queue_resize = TRUE;
    }

  if (queue_resize)
    gtk_widget_queue_resize (widget);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkHButtonBox, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_hbutton_box_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_hbutton_box_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GtkButtonBoxStyle layout_style;
  gint spacing, default_spacing, i;
  gint min_width, min_height, ipad_x, ipad_y;
  gint default_min_width, default_min_height, default_ipad_x, default_ipad_y;

#ifdef USE_GNOME
  /* GnomeLibs workaround: We don't want to output any code for a
     GnomeMessageBox action_area, since it isn't created until a button is
     added. */
  gchar *child_name;

  child_name = gb_widget_get_child_name (widget);
  if (child_name && !strcmp (child_name, "GnomeDialog:action_area")
      && GNOME_IS_MESSAGE_BOX (gtk_widget_get_toplevel (widget)))
    {
      return;
    }
#endif

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_hbutton_box_new ();\n", data->wname);
    }

  gb_widget_write_standard_source (widget, data);

  layout_style = GTK_BUTTON_BOX (widget)->layout_style;
  if (layout_style != GTK_BUTTONBOX_DEFAULT_STYLE)
    {
      for (i = 0; i < sizeof (GbLayoutValues) / sizeof (GbLayoutValues[0]); i
	   ++)
	{
	  if (GbLayoutValues[i] == layout_style)
	    source_add (data,
		 "  gtk_button_box_set_layout (GTK_BUTTON_BOX (%s), %s);\n",
			data->wname, GbLayoutSymbols[i]);
	}
    }

  spacing = gtk_button_box_get_spacing (GTK_BUTTON_BOX (widget));
  default_spacing = gtk_hbutton_box_get_spacing_default ();
  if (spacing != GTK_BUTTONBOX_DEFAULT && spacing != default_spacing)
    {
      source_add (data,
		"  gtk_button_box_set_spacing (GTK_BUTTON_BOX (%s), %i);\n",
		  data->wname, spacing);
    }

  gtk_button_box_get_child_size_default (&default_min_width,
					 &default_min_height);
  gtk_button_box_get_child_size (GTK_BUTTON_BOX (widget), &min_width,
				 &min_height);
  if ((min_width != GTK_BUTTONBOX_DEFAULT && min_width != default_min_width)
      || (min_height != GTK_BUTTONBOX_DEFAULT
	  && min_height != default_min_height))
    {
      source_add (data,
	 "  gtk_button_box_set_child_size (GTK_BUTTON_BOX (%s), %i, %i);\n",
		  data->wname, min_width, min_height);
    }

  gtk_button_box_get_child_ipadding_default (&default_ipad_x, &default_ipad_y);
  gtk_button_box_get_child_ipadding (GTK_BUTTON_BOX (widget), &ipad_x, &ipad_y);
  if ((ipad_x != GTK_BUTTONBOX_DEFAULT && ipad_x != default_ipad_x)
      || (ipad_y != GTK_BUTTONBOX_DEFAULT && ipad_y != default_ipad_y))
    {
      source_add (data,
      "  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (%s), %i, %i);\n",
		  data->wname, ipad_x, ipad_y);
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_hbutton_box_init ()
{
  /* Initialise the GTK type */
  gtk_hbutton_box_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = hbuttonbox_xpm;
  gbwidget.tooltip = _("Horizontal Button Box");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_hbutton_box_new;
  gbwidget.gb_widget_create_properties = gb_hbutton_box_create_properties;
  gbwidget.gb_widget_get_properties = gb_hbutton_box_get_properties;
  gbwidget.gb_widget_set_properties = gb_hbutton_box_set_properties;
  gbwidget.gb_widget_write_source = gb_hbutton_box_write_source;
/*
   gbwidget.gb_widget_create_popup_menu = gb_hbutton_box_create_popup_menu;
 */

  return &gbwidget;
}
