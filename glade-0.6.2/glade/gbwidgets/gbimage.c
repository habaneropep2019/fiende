
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

#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkspinbutton.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/image.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *XAlign = "Image|GtkMisc::xalign";
static gchar *YAlign = "Image|GtkMisc::yalign";
static gchar *XPad = "Image|GtkMisc::xpad";
static gchar *YPad = "Image|GtkMisc::ypad";

/* Note: these are not shown, and the symbols used for Visual do not exist. */
static gchar *Width = "GtkImage::image_width";
static gchar *Height = "GtkImage::image_height";
static gchar *Type = "GtkImage::image_type";
static gchar *Visual = "GtkImage::image_visual";

static gchar *ImageNormal = "GDK_IMAGE_NORMAL";
static gchar *ImageShared = "GDK_IMAGE_SHARED";
static gchar *ImageFastest = "GDK_IMAGE_FASTEST";

static gchar *VisualSystem = "GDK_VISUAL_SYSTEM";
static gchar *VisualBest = "GDK_VISUAL_BEST";

static void show_image_dialog (GbWidgetNewData * data);
static void on_image_dialog_ok (GtkWidget * widget,
				GbWidgetNewData * data);
static void on_image_dialog_destroy (GtkWidget * widget,
				     GbWidgetNewData * data);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkImage, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_image_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;
  GdkImage *gdkimage;
  gint width = 0, height = 0, type;
  gchar *type_symbol, *visual_symbol;
  GdkVisual *visual;

  if (data->action == GB_LOADING)
    {
      width = load_int (data->loading_data, Width);
      height = load_int (data->loading_data, Height);
      if (width == 0)
	width = 100;
      if (height == 0)
	height = 100;

      type_symbol = load_string (data->loading_data, Type);
      if (type_symbol)
	{
	  if (!strcmp (type_symbol, ImageShared))
	    {
	      type = GDK_IMAGE_SHARED;
	      type_symbol = ImageShared;
	    }
	  else if (!strcmp (type_symbol, ImageFastest))
	    {
	      type = GDK_IMAGE_FASTEST;
	      type_symbol = ImageFastest;
	    }
	  else
	    {
	      type = GDK_IMAGE_NORMAL;
	      type_symbol = ImageNormal;
	    }
	}
      else
	{
	  type = GDK_IMAGE_NORMAL;
	  type_symbol = ImageNormal;
	}

      visual_symbol = load_string (data->loading_data, Visual);
      if (visual_symbol)
	{
	  if (!strcmp (visual_symbol, VisualBest))
	    {
	      visual = gdk_visual_get_best ();
	      visual_symbol = VisualBest;
	    }
	  else
	    {
	      visual = gdk_visual_get_system ();
	      visual_symbol = VisualSystem;
	    }
	}
      else
	{
	  visual = gdk_visual_get_system ();
	  visual_symbol = VisualSystem;
	}

      gdkimage = gdk_image_new (type, visual, width, height);
      new_widget = gtk_image_new (gdkimage, NULL);

      /* Save the width, height, type & visual. */
      gtk_object_set_data (GTK_OBJECT (new_widget), Width, GINT_TO_POINTER (width));
      gtk_object_set_data (GTK_OBJECT (new_widget), Height, GINT_TO_POINTER (height));
      gtk_object_set_data (GTK_OBJECT (new_widget), Type, type_symbol);
      gtk_object_set_data (GTK_OBJECT (new_widget), Visual, visual_symbol);

      return new_widget;
    }
  else
    {
      show_image_dialog (data);
      return NULL;
    }
}


static void
show_image_dialog (GbWidgetNewData * data)
{
  GtkWidget *dialog, *vbox, *table, *label, *spinbutton, *button;
  GtkObject *adjustment;
  GSList *group;

  dialog = glade_util_create_dialog (_("New image"), data->parent,
				     on_image_dialog_ok, data, &vbox);
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      GTK_SIGNAL_FUNC (on_image_dialog_destroy), data);

  /* Image width & height spinbuttons */
  table = gtk_table_new (4, 7, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 5);
  gtk_widget_show (table);

  label = gtk_label_new (_("Image Width:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    GTK_FILL, GTK_FILL, 5, 5);
  gtk_widget_show (label);

  adjustment = gtk_adjustment_new (100, 1, 10000, 1, 5, 10);
  spinbutton = glade_util_spin_button_new (GTK_OBJECT (dialog), "width",
					   GTK_ADJUSTMENT (adjustment), 1, 0);
  gtk_table_attach (GTK_TABLE (table), spinbutton, 1, 7, 0, 1,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_widget_grab_focus (spinbutton);
  gtk_widget_show (spinbutton);

  /* Image height */
  label = gtk_label_new (_("Image Height:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    GTK_FILL, GTK_FILL, 5, 5);
  gtk_widget_show (label);

  adjustment = gtk_adjustment_new (100, 1, 10000, 1, 5, 10);
  spinbutton = glade_util_spin_button_new (GTK_OBJECT (dialog), "height",
					   GTK_ADJUSTMENT (adjustment), 1, 0);
  gtk_table_attach (GTK_TABLE (table), spinbutton, 1, 7, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_widget_show (spinbutton);

  /* Type - Normal/Shared/Fastest */
  label = gtk_label_new (_("Image Type:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    GTK_FILL, GTK_FILL, 5, 5);
  gtk_widget_show (label);

  button = gtk_radio_button_new_with_label (NULL, _("Normal"));
  gtk_table_attach (GTK_TABLE (table), button, 1, 3, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_widget_show (button);
  gtk_object_set_data (GTK_OBJECT (dialog), "normal", button);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label (group, _("Shared"));
  gtk_table_attach (GTK_TABLE (table), button, 3, 5, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_widget_show (button);
  gtk_object_set_data (GTK_OBJECT (dialog), "shared", button);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label (group, _("Fastest"));
  gtk_table_attach (GTK_TABLE (table), button, 5, 7, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_widget_show (button);
  gtk_object_set_data (GTK_OBJECT (dialog), "fastest", button);

  /* Visual - System/Best */
  label = gtk_label_new (_("Visual:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    GTK_FILL, GTK_FILL, 5, 5);
  gtk_widget_show (label);

  button = gtk_radio_button_new_with_label (NULL, _("System"));
  gtk_table_attach (GTK_TABLE (table), button, 1, 4, 3, 4,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_widget_show (button);
  gtk_object_set_data (GTK_OBJECT (dialog), "system", button);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label (group, _("Best"));
  gtk_table_attach (GTK_TABLE (table), button, 4, 7, 3, 4,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
  gtk_widget_show (button);
  gtk_object_set_data (GTK_OBJECT (dialog), "best", button);

  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
}


static void
on_image_dialog_ok (GtkWidget * widget, GbWidgetNewData * data)
{
  GtkWidget *new_widget, *spinbutton, *window, *button;
  GdkImage *gdkimage;
  gint width, height;
  gint type = GDK_IMAGE_NORMAL;
  GdkVisual *visual = NULL;
  gchar *type_symbol = ImageNormal, *visual_symbol = NULL;

  window = gtk_widget_get_toplevel (widget);

  /* Only call callback if placeholder/fixed widget is still there */
  if (gb_widget_can_finish_new (data))
    {
      /* Get image width & height */
      spinbutton = gtk_object_get_data (GTK_OBJECT (window), "width");
      g_return_if_fail (spinbutton != NULL);
      width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

      spinbutton = gtk_object_get_data (GTK_OBJECT (window), "height");
      g_return_if_fail (spinbutton != NULL);
      height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

      /* Get image type */
      button = gtk_object_get_data (GTK_OBJECT (window), "normal");
      g_return_if_fail (button != NULL);
      if (GTK_TOGGLE_BUTTON (button)->active)
	{
	  type = GDK_IMAGE_NORMAL;
	  type_symbol = ImageNormal;
	}

      button = gtk_object_get_data (GTK_OBJECT (window), "shared");
      g_return_if_fail (button != NULL);
      if (GTK_TOGGLE_BUTTON (button)->active)
	{
	  type = GDK_IMAGE_SHARED;
	  type_symbol = ImageShared;
	}

      button = gtk_object_get_data (GTK_OBJECT (window), "fastest");
      g_return_if_fail (button != NULL);
      if (GTK_TOGGLE_BUTTON (button)->active)
	{
	  type = GDK_IMAGE_FASTEST;
	  type_symbol = ImageFastest;
	}

      /* Get image visual */
      button = gtk_object_get_data (GTK_OBJECT (window), "system");
      g_return_if_fail (button != NULL);
      if (GTK_TOGGLE_BUTTON (button)->active)
	{
	  visual = gdk_visual_get_system ();
	  /* Note: these don't actually exist in GDK. */
	  visual_symbol = VisualSystem;
	}

      button = gtk_object_get_data (GTK_OBJECT (window), "best");
      g_return_if_fail (button != NULL);
      if (GTK_TOGGLE_BUTTON (button)->active)
	{
	  visual = gdk_visual_get_best ();
	  visual_symbol = VisualBest;
	}

      g_return_if_fail (visual != NULL);

      /* Create image */
      gdkimage = gdk_image_new (type, visual, width, height);
      new_widget = gtk_image_new (gdkimage, NULL);

      /* Save the width, height, type & visual. */
      gtk_object_set_data (GTK_OBJECT (new_widget), Width, GINT_TO_POINTER (width));
      gtk_object_set_data (GTK_OBJECT (new_widget), Height, GINT_TO_POINTER (height));
      gtk_object_set_data (GTK_OBJECT (new_widget), Type, type_symbol);
      gtk_object_set_data (GTK_OBJECT (new_widget), Visual, visual_symbol);

      gb_widget_initialize (new_widget, data);
      (*data->callback) (new_widget, data);
    }
  gtk_widget_destroy (window);
}


static void
on_image_dialog_destroy (GtkWidget * widget,
			 GbWidgetNewData * data)
{
  gb_widget_free_new_data (data);
  gtk_grab_remove (widget);
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_image_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_float_range (XAlign, _("X Align:"),
			    _("The horizontal alignment"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_float_range (YAlign, _("Y Align:"),
			    _("The vertical alignment"),
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
gb_image_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gb_widget_output_float (data, XAlign, GTK_MISC (widget)->xalign);
  gb_widget_output_float (data, YAlign, GTK_MISC (widget)->yalign);
  gb_widget_output_int (data, XPad, GTK_MISC (widget)->xpad);
  gb_widget_output_int (data, YPad, GTK_MISC (widget)->ypad);

  if (data->action == GB_SAVING)
    {
      gb_widget_output_int (data, Width, GPOINTER_TO_INT (gtk_object_get_data
					     (GTK_OBJECT (widget), Width)));
      gb_widget_output_int (data, Height, GPOINTER_TO_INT (gtk_object_get_data
					    (GTK_OBJECT (widget), Height)));
      gb_widget_output_string (data, Type,
			       gtk_object_get_data (GTK_OBJECT (widget),
						    Type));
      gb_widget_output_string (data, Visual,
			       gtk_object_get_data (GTK_OBJECT (widget),
						    Visual));
    }
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_image_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gfloat xalign, yalign;
  gint xpad, ypad;
  gboolean set_alignment = FALSE, set_padding = FALSE;

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
 * Add commands to aid in editing a GtkImage, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_image_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_image_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{

  if (data->create_widget)
    {
      gint width, height;
      gchar *type, *visual, *visual_code;
      type = gtk_object_get_data (GTK_OBJECT (widget), Type);
      visual = gtk_object_get_data (GTK_OBJECT (widget), Visual);
      if (!strcmp (visual, VisualBest))
	visual_code = "gdk_visual_get_best ()";
      else
	visual_code = "gdk_visual_get_system ()";
      width = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), Width));
      height = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), Height));

      /* FIXME: I'm not sure how useful this is, and it sets the mask to NULL.
       */
      source_add (data,
	   "  %s = gtk_image_new (gdk_image_new (%s, %s, %i, %i), NULL);\n",
		  data->wname, type, visual_code, width, height);
    }

  gb_widget_write_standard_source (widget, data);

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
gb_image_init ()
{
  /* Initialise the GTK type */
  gtk_image_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = image_xpm;
  gbwidget.tooltip = _("Image");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_image_new;
  gbwidget.gb_widget_create_properties = gb_image_create_properties;
  gbwidget.gb_widget_get_properties = gb_image_get_properties;
  gbwidget.gb_widget_set_properties = gb_image_set_properties;
  gbwidget.gb_widget_write_source = gb_image_write_source;
/*
   gbwidget.gb_widget_create_popup_menu = gb_image_create_popup_menu;
 */

  return &gbwidget;
}
