
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

#include <gtk/gtkaspectframe.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/aspectframe.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "AspectFrame|GtkFrame::label";
static gchar *LabelXAlign = "AspectFrame|GtkFrame::label_xalign";
/* Frame's yalign is never used */
/*static gchar *LabelYAlign = "AspectFrame|GtkFrame::label_yalign"; */
static gchar *Shadow = "AspectFrame|GtkFrame::shadow_type";
static gchar *XAlign = "GtkAspectFrame::xalign";
static gchar *YAlign = "GtkAspectFrame::yalign";
static gchar *Ratio = "GtkAspectFrame::ratio";
static gchar *Obey = "GtkAspectFrame::obey_child";

static const gchar *GbShadowChoices[] =
{"None", "In", "Out",
 "Etched In", "Etched Out", NULL};
static const gint GbShadowValues[] =
{
  GTK_SHADOW_NONE,
  GTK_SHADOW_IN,
  GTK_SHADOW_OUT,
  GTK_SHADOW_ETCHED_IN,
  GTK_SHADOW_ETCHED_OUT
};
static const gchar *GbShadowSymbols[] =
{
  "GTK_SHADOW_NONE",
  "GTK_SHADOW_IN",
  "GTK_SHADOW_OUT",
  "GTK_SHADOW_ETCHED_IN",
  "GTK_SHADOW_ETCHED_OUT"
};


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkAspectFrame, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_aspect_frame_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget = gtk_aspect_frame_new (NULL, 0.5, 0.5, 1.0, TRUE);
  if (data->action != GB_LOADING)
    gtk_container_add (GTK_CONTAINER (new_widget), editor_new_placeholder ());
  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_aspect_frame_create_properties (GtkWidget * widget, GbWidgetCreateArgData *
				   data)
{
  property_add_string (Label, _("Label:"), _("The text to display"));
  property_add_float_range (LabelXAlign, _("Label X Align:"),
			    _("The horizontal alignment of the frame's label"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_choice (Shadow, _("Shadow:"), _("The type of shadow of the frame"),
		       GbShadowChoices);
  property_add_float_range (XAlign, _("X Align:"),
			    _("The horizontal alignment of the frame's child"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_float_range (YAlign, _("Y Align:"),
			    _("The horizontal alignment of the frame's child"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_float (Ratio, _("Ratio:"),
		      _("The aspect ratio of the frame's child"));
  property_add_bool (Obey, _("Obey Child:"),
		   _("If the aspect ratio should be determined by the child"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_aspect_frame_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gint i;

  gb_widget_output_translatable_string (data, Label,
					GTK_FRAME (widget)->label);
  gb_widget_output_float (data, LabelXAlign, GTK_FRAME (widget)->label_xalign);

  for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]); i++)
    {
      if (GbShadowValues[i] == GTK_FRAME (widget)->shadow_type)
	gb_widget_output_choice (data, Shadow, i, GbShadowSymbols[i]);
    }

  gb_widget_output_float (data, XAlign, GTK_ASPECT_FRAME (widget)->xalign);
  gb_widget_output_float (data, YAlign, GTK_ASPECT_FRAME (widget)->yalign);
  gb_widget_output_float (data, Ratio, GTK_ASPECT_FRAME (widget)->ratio);
  gb_widget_output_bool (data, Obey, GTK_ASPECT_FRAME (widget)->obey_child);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_aspect_frame_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *label;
  gfloat label_xalign, xalign, yalign, ratio;
  gchar *shadow;
  gboolean obey_child, set_aspect_frame = FALSE;
  gint i;

  label = gb_widget_input_string (data, Label);
  if (data->apply)
    gtk_frame_set_label (GTK_FRAME (widget), label);

  label_xalign = gb_widget_input_float (data, LabelXAlign);
  if (data->apply)
    gtk_frame_set_label_align (GTK_FRAME (widget), label_xalign, 0.5);

  shadow = gb_widget_input_choice (data, Shadow);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]); i
	   ++)
	{
	  if (!strcmp (shadow, GbShadowChoices[i])
	      || !strcmp (shadow, GbShadowSymbols[i]))
	    {
	      gtk_frame_set_shadow_type (GTK_FRAME (widget), GbShadowValues[i]);
	      break;
	    }
	}
    }

  xalign = gb_widget_input_float (data, XAlign);
  if (data->apply)
    set_aspect_frame = TRUE;
  else
    xalign = GTK_ASPECT_FRAME (widget)->xalign;

  yalign = gb_widget_input_float (data, YAlign);
  if (data->apply)
    set_aspect_frame = TRUE;
  else
    yalign = GTK_ASPECT_FRAME (widget)->yalign;

  ratio = gb_widget_input_float (data, Ratio);
  if (data->apply)
    set_aspect_frame = TRUE;
  else
    ratio = GTK_ASPECT_FRAME (widget)->ratio;

  obey_child = gb_widget_input_bool (data, Obey);
  if (data->apply)
    set_aspect_frame = TRUE;
  else
    obey_child = GTK_ASPECT_FRAME (widget)->obey_child;

  if (set_aspect_frame)
    gtk_aspect_frame_set (GTK_ASPECT_FRAME (widget), xalign, yalign,
			  ratio, obey_child);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkAspectFrame, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_aspect_frame_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_aspect_frame_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gfloat xalign;
  gint shadow = 0, i;
  gchar *label = GTK_FRAME (widget)->label;

  if (data->create_widget)
    {
      if (label)
	source_add (data, "  %s = gtk_aspect_frame_new (%s, %g, %g, %g, %s);\n",
		    data->wname,
		    source_make_string (GTK_FRAME (widget)->label,
					data->use_gettext),
		    GTK_ASPECT_FRAME (widget)->xalign,
		    GTK_ASPECT_FRAME (widget)->yalign,
		    GTK_ASPECT_FRAME (widget)->ratio,
		    GTK_ASPECT_FRAME (widget)->obey_child ? "TRUE" : "FALSE");
      else
	source_add (data, "  %s = gtk_aspect_frame_new (NULL, %g, %g, %g, %s);\n",
		    data->wname,
		    GTK_ASPECT_FRAME (widget)->xalign,
		    GTK_ASPECT_FRAME (widget)->yalign,
		    GTK_ASPECT_FRAME (widget)->ratio,
		    GTK_ASPECT_FRAME (widget)->obey_child ? "TRUE" : "FALSE");
    }
  gb_widget_write_standard_source (widget, data);

  xalign = GTK_FRAME (widget)->label_xalign;
  if (xalign)
    {
      source_add (data,
		  "  gtk_frame_set_label_align (GTK_FRAME (%s), %g, 0.5);\n",
		  data->wname, xalign);
    }

  if (GTK_FRAME (widget)->shadow_type != GTK_SHADOW_ETCHED_IN)
    {
      for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]); i
	   ++)
	if (GbShadowValues[i] == GTK_FRAME (widget)->shadow_type)
	  {
	    shadow = i;
	    break;
	  }
      source_add (data, "  gtk_frame_set_shadow_type (GTK_FRAME (%s), %s);\n",
		  data->wname, GbShadowSymbols[shadow]);
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_aspect_frame_init ()
{
  /* Initialise the GTK type */
  gtk_aspect_frame_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = aspectframe_xpm;
  gbwidget.tooltip = _("Aspect Frame");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_aspect_frame_new;
  gbwidget.gb_widget_create_properties = gb_aspect_frame_create_properties;
  gbwidget.gb_widget_get_properties = gb_aspect_frame_get_properties;
  gbwidget.gb_widget_set_properties = gb_aspect_frame_set_properties;
  gbwidget.gb_widget_write_source = gb_aspect_frame_write_source;
/*
   gbwidget.gb_widget_create_popup_menu = gb_aspect_frame_create_popup_menu;
 */

  return &gbwidget;
}
