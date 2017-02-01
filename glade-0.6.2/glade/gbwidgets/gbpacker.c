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

#include <gtk/gtkpacker.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/packer.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

/* FIXME: spacing doesn't appear to do anything in GTK+ 1.2.3. */
/*static const gchar *Spacing = "GtkPacker::spacing";*/
static const gchar *BorderWidth = "GtkPacker::default_border_width";
static const gchar *PadX = "GtkPacker::default_pad_x";
static const gchar *PadY = "GtkPacker::default_pad_y";
static const gchar *IPadX = "GtkPacker::default_ipad_x";
static const gchar *IPadY = "GtkPacker::default_ipad_y";

/* For children of a packer */
static const gchar *GbPackerSide = "GtkPackerChild::side";
static const gchar *GbPackerAnchor = "GtkPackerChild::anchor";
static const gchar *GbPackerExpand = "GtkPackerChild::expand";
static const gchar *GbPackerFillX = "GtkPackerChild::xfill";
static const gchar *GbPackerFillY = "GtkPackerChild::yfill";
static const gchar *GbPackerUseDefault = "GtkPackerChild::use_default";
static const gchar *GbPackerBorder = "GtkPackerChild::border_width";
static const gchar *GbPackerPadX = "GtkPackerChild::xpad";
static const gchar *GbPackerPadY = "GtkPackerChild::ypad";
static const gchar *GbPackerIPadX = "GtkPackerChild::xipad";
static const gchar *GbPackerIPadY = "GtkPackerChild::yipad";

/* This is only used internally. It isn't saved since the order is implicit. */
static const gchar *GbPackerPos = "GtkPackerChild::position";


/* Packer children choices. */
const gchar *GbPackerSideChoices[] =
{"Top", "Bottom", "Left", "Right", NULL};
const gint GbPackerSideValues[] =
{
  GTK_SIDE_TOP,
  GTK_SIDE_BOTTOM,
  GTK_SIDE_LEFT,
  GTK_SIDE_RIGHT
};
const gchar *GbPackerSideSymbols[] =
{
  "GTK_SIDE_TOP",
  "GTK_SIDE_BOTTOM",
  "GTK_SIDE_LEFT",
  "GTK_SIDE_RIGHT"
};

const gchar *GbPackerAnchorChoices[] =
{"Center", "North", "North West", "North East", "South", "South West",
 "South East", "West", "East", NULL};
const gint GbPackerAnchorValues[] =
{
  GTK_ANCHOR_CENTER,
  GTK_ANCHOR_NORTH,
  GTK_ANCHOR_NORTH_WEST,
  GTK_ANCHOR_NORTH_EAST,
  GTK_ANCHOR_SOUTH,
  GTK_ANCHOR_SOUTH_WEST,
  GTK_ANCHOR_SOUTH_EAST,
  GTK_ANCHOR_WEST,
  GTK_ANCHOR_EAST,
};
const gchar *GbPackerAnchorSymbols[] =
{
  "GTK_ANCHOR_CENTER",
  "GTK_ANCHOR_NORTH",
  "GTK_ANCHOR_NORTH_WEST",
  "GTK_ANCHOR_NORTH_EAST",
  "GTK_ANCHOR_SOUTH",
  "GTK_ANCHOR_SOUTH_WEST",
  "GTK_ANCHOR_SOUTH_EAST",
  "GTK_ANCHOR_WEST",
  "GTK_ANCHOR_EAST"
};


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the funtion in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkPacker, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
/*
GtkWidget*
gb_packer_new (GbWidgetNewData *data)
{

}
*/



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_packer_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  /*
  property_add_int_range (Spacing, _("Spacing:"),
			  _("The spacing between child widgets"),
			  0, 10000, 1, 10, 1);
  */
  property_add_int_range (BorderWidth, _("Default Border:"),
			  _("The default border width of child widgets"),
			  0, 10000, 1, 10, 1);

  property_add_int_range (PadX, _("Default Pad X:"),
			  _("The default horizontal padding of child widgets"),
			  0, 10000, 1, 10, 1);
  property_add_int_range (PadY, _("Default Pad Y:"),
			  _("The default vertical padding of child widgets"),
			  0, 10000, 1, 10, 1);

  property_add_int_range (IPadX, _("Default IPad X:"),
			  _("The default internal horizontal padding of child widgets"),
			  0, 10000, 1, 10, 1);
  property_add_int_range (IPadY, _("Default IPad Y:"),
			  _("The default internal vertical padding of child widgets"),
			  0, 10000, 1, 10, 1);
}


/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_packer_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  /*gb_widget_output_int (data, Spacing, GTK_PACKER (widget)->spacing);*/
  gb_widget_output_int (data, BorderWidth,
			GTK_PACKER (widget)->default_border_width);
  gb_widget_output_int (data, PadX, GTK_PACKER (widget)->default_pad_x);
  gb_widget_output_int (data, PadY, GTK_PACKER (widget)->default_pad_y);

  gb_widget_output_int (data, IPadX, GTK_PACKER (widget)->default_i_pad_x);
  gb_widget_output_int (data, IPadY, GTK_PACKER (widget)->default_i_pad_y);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_packer_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  /*gint spacing;*/
  gint border_width, padx, pady, ipadx, ipady;
  gboolean set_padding = FALSE, set_ipadding = FALSE;

  /*
  spacing = gb_widget_input_int (data, Spacing);
  if (data->apply)
    gtk_packer_set_spacing (GTK_PACKER (widget), spacing);
  */
  border_width = gb_widget_input_int (data, BorderWidth);
  if (data->apply)
    gtk_packer_set_default_border_width (GTK_PACKER (widget), border_width);

  padx = gb_widget_input_int (data, PadX);
  if (data->apply)
    set_padding = TRUE;
  else
    padx = GTK_PACKER (widget)->default_pad_x;

  pady = gb_widget_input_int (data, PadY);
  if (data->apply)
    set_padding = TRUE;
  else
    pady = GTK_PACKER (widget)->default_pad_y;

  if (set_padding)
    gtk_packer_set_default_pad (GTK_PACKER (widget), padx, pady);

  ipadx = gb_widget_input_int (data, IPadX);
  if (data->apply)
    set_ipadding = TRUE;
  else
    ipadx = GTK_PACKER (widget)->default_i_pad_x;

  ipady = gb_widget_input_int (data, IPadY);
  if (data->apply)
    set_ipadding = TRUE;
  else
    ipady = GTK_PACKER (widget)->default_i_pad_y;

  if (set_ipadding)
    gtk_packer_set_default_ipad (GTK_PACKER (widget), ipadx, ipady);
}


/*
 * Creates the child packing properties for children of this widget.
 */
static void
gb_packer_create_child_properties (GtkWidget * widget,
				   GbWidgetCreateChildArgData * data)
{
  property_add_int_range (GbPackerPos, _("Position:"),
			  _("The position of the child widget with respect to its siblings"),
			  0, 10000, 1, 10, 1);

  property_add_choice (GbPackerSide, _("Side:"),
		       _("The side of the remaining space to place the widget"),
		       GbPackerSideChoices);
  property_add_choice (GbPackerAnchor, _("Anchor:"),
		       _("Where to anchor the widget in its allocated area"),
		       GbPackerAnchorChoices);
  property_add_bool (GbPackerExpand, _("Expand:"),
		     _("Set True to let the widget expand"));
  property_add_bool (GbPackerFillX, _("Fill X:"),
		     _("Set True to let the widget fill its allocated width"));
  property_add_bool (GbPackerFillY, _("Fill Y:"),
		     _("Set True to let the widget fill its allocated height"));

  property_add_bool (GbPackerUseDefault, _("Use Defaults:"),
		     _("Set True to use the parent GtkPacker's default settings for the border width and all padding properties"));

  property_add_int_range (GbPackerBorder, _("Border Width:"),
			  _("The size of the widget's border"),
			  0, 10000, 1, 10, 1);
  property_add_int_range (GbPackerPadX, _("H Padding:"),
			  _("The widget's horizontal external padding"),
			  0, 10000, 1, 10, 1);
  property_add_int_range (GbPackerPadY, _("V Padding:"),
			  _("The widget's vertical external padding"),
			  0, 10000, 1, 10, 1);
  property_add_int_range (GbPackerIPadX, _("H Int. Padding:"),
			  _("The widget's horizontal internal padding"),
			  0, 10000, 1, 10, 1);
  property_add_int_range (GbPackerIPadY, _("V Int. Padding:"),
			  _("The widget's vertical internal padding"),
			  0, 10000, 1, 10, 1);
}


static void
gb_packer_get_child_properties (GtkWidget *widget, GtkWidget *child,
				GbWidgetGetArgData *data)
{
  GtkPackerChild *pchild;
  gint i;

  pchild = glade_util_find_packer_child (GTK_PACKER (widget), child);
  g_return_if_fail (pchild != NULL);

  if (data->action == GB_SAVING)
    save_start_tag (data, "child");

  if (data->action != GB_SAVING)
    {
      gb_widget_output_int (data, GbPackerPos,
			    g_list_index (GTK_PACKER (widget)->children,
					  pchild));
    }

  for (i = 0; GbPackerSideChoices[i]; i++)
    {
      if (GbPackerSideValues[i] == pchild->side)
	gb_widget_output_choice (data, GbPackerSide, i,
				 GbPackerSideSymbols[i]);
    }
  for (i = 0; GbPackerAnchorChoices[i]; i++)
    {
      if (GbPackerAnchorValues[i] == pchild->anchor)
	gb_widget_output_choice (data, GbPackerAnchor, i,
				 GbPackerAnchorSymbols[i]);
    }

  gb_widget_output_bool (data, GbPackerExpand,
			 pchild->options & GTK_PACK_EXPAND);
  gb_widget_output_bool (data, GbPackerFillX,
			 pchild->options & GTK_FILL_X);
  gb_widget_output_bool (data, GbPackerFillY,
			 pchild->options & GTK_FILL_Y);

  gb_widget_output_bool (data, GbPackerUseDefault, pchild->use_default);
  gb_widget_output_int (data, GbPackerBorder, pchild->border_width);
  gb_widget_output_int (data, GbPackerPadX, pchild->pad_x);
  gb_widget_output_int (data, GbPackerPadY, pchild->pad_y);
  gb_widget_output_int (data, GbPackerIPadX, pchild->i_pad_x);
  gb_widget_output_int (data, GbPackerIPadY, pchild->i_pad_y);

  if (data->action == GB_SHOWING)
    {
      gboolean sensitive;

      sensitive = pchild->use_default ? FALSE : TRUE;
      property_set_sensitive (GbPackerBorder, sensitive);
      property_set_sensitive (GbPackerPadX, sensitive);
      property_set_sensitive (GbPackerPadY, sensitive);
      property_set_sensitive (GbPackerIPadX, sensitive);
      property_set_sensitive (GbPackerIPadY, sensitive);
    }

  if (data->action == GB_SAVING)
    save_end_tag (data, "child");
}


static void
gb_packer_set_child_properties (GtkWidget *widget, GtkWidget *child,
				GbWidgetSetArgData *data)
{
  gchar *side_str, *anchor_str;
  GtkSideType side = GTK_SIDE_TOP;
  GtkAnchorType anchor = GTK_ANCHOR_CENTER;
  gboolean expand, fillx, filly, use_defaults, set_child_packing = FALSE;
  gint pos, border, padx, pady, ipadx, ipady, i;
  GtkPackerChild *pchild;

  pchild = glade_util_find_packer_child (GTK_PACKER (widget), child);
  g_return_if_fail (pchild != NULL);

  pos = gb_widget_input_int (data, GbPackerPos);
  if (data->apply)
    gtk_packer_reorder_child (GTK_PACKER (widget), child, pos);

  side_str = gb_widget_input_choice (data, GbPackerSide);
  if (data->apply)
    {
      set_child_packing = TRUE;
      for (i = 0; GbPackerSideChoices[i]; i++)
        {
          if (!strcmp (side_str, GbPackerSideChoices[i])
              || !strcmp (side_str, GbPackerSideSymbols[i]))
            {
              side = GbPackerSideValues[i];
              break;
            }
        }
    }
  else
    side = pchild->side;

  anchor_str = gb_widget_input_choice (data, GbPackerAnchor);
  if (data->apply)
    {
      set_child_packing = TRUE;
      for (i = 0; GbPackerAnchorChoices[i]; i++)
        {
          if (!strcmp (anchor_str, GbPackerAnchorChoices[i])
              || !strcmp (anchor_str, GbPackerAnchorSymbols[i]))
            {
              anchor = GbPackerAnchorValues[i];
              break;
            }
        }
    }
  else
    anchor = pchild->anchor;

  expand = (gb_widget_input_bool (data, GbPackerExpand)) ? GTK_PACK_EXPAND : 0;
  if (data->apply)
    set_child_packing = TRUE;
  else
    expand = pchild->options & GTK_PACK_EXPAND;

  fillx = (gb_widget_input_bool (data, GbPackerFillX)) ? GTK_FILL_X : 0;
  if (data->apply)
    set_child_packing = TRUE;
  else
    fillx = pchild->options & GTK_FILL_X;

  filly = (gb_widget_input_bool (data, GbPackerFillY)) ? GTK_FILL_Y : 0;
  if (data->apply)
    set_child_packing = TRUE;
  else
    filly = pchild->options & GTK_FILL_Y;

  use_defaults = gb_widget_input_bool (data, GbPackerUseDefault);
  if (data->apply)
    {
      pchild->use_default = use_defaults ? 1 : 0;

      /* FIXME: GTK+ workaround - We make sure the widget's border width
	 & padding properties are reset to the default values. */
      if (use_defaults)
	{
	  pchild->border_width = GTK_PACKER (widget)->default_border_width;
	  pchild->pad_x = GTK_PACKER (widget)->default_pad_x;
	  pchild->pad_y = GTK_PACKER (widget)->default_pad_y;
	  pchild->i_pad_x = GTK_PACKER (widget)->default_i_pad_x;
	  pchild->i_pad_y = GTK_PACKER (widget)->default_i_pad_y;
	  gtk_widget_queue_resize (GTK_WIDGET (pchild->widget));
	}

      if (data->action == GB_APPLYING)
	{
	  gboolean sensitive;

	  sensitive = pchild->use_default ? FALSE : TRUE;
	  property_set_sensitive (GbPackerBorder, sensitive);
	  property_set_sensitive (GbPackerPadX, sensitive);
	  property_set_sensitive (GbPackerPadY, sensitive);
	  property_set_sensitive (GbPackerIPadX, sensitive);
	  property_set_sensitive (GbPackerIPadY, sensitive);
	}
    }
  else
    use_defaults = pchild->use_default;

  border = gb_widget_input_int (data, GbPackerBorder);
  if (data->apply)
    set_child_packing = TRUE;
  else
    border = pchild->border_width;

  padx = gb_widget_input_int (data, GbPackerPadX);
  if (data->apply)
    set_child_packing = TRUE;
  else
    padx = pchild->pad_x;

  pady = gb_widget_input_int (data, GbPackerPadY);
  if (data->apply)
    set_child_packing = TRUE;
  else
    pady = pchild->pad_y;

  ipadx = gb_widget_input_int (data, GbPackerIPadX);
  if (data->apply)
    set_child_packing = TRUE;
  else
    ipadx = pchild->i_pad_x;

  ipady = gb_widget_input_int (data, GbPackerIPadY);
  if (data->apply)
    set_child_packing = TRUE;
  else
    ipady = pchild->i_pad_y;

  if (set_child_packing)
    {
      GtkPackerOptions options;

      options = expand | fillx | filly;

      if (side != pchild->side
	  || anchor != pchild->anchor
	  || options != pchild->options
	  || border != pchild->border_width
	  || padx != pchild->pad_x
	  || pady != pchild->pad_y
	  || ipadx != pchild->i_pad_x
	  || ipady != pchild->i_pad_y)
	{
	  gtk_packer_set_child_packing (GTK_PACKER (widget), child,
					side, anchor, options,
					border, padx, pady, ipadx, ipady);

	  /* FIXME: GTK+ workaround. It sets use_default to 0, which we may not
	     want. So we reset it here. */
	  pchild->use_default = use_defaults;
	}
    }
}

/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkPacker, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_packer_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_packer_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_packer_new ();\n", data->wname);
    }

  /*
  if (GTK_PACKER (widget)->spacing != 0)
    source_add (data,
		"  gtk_packer_set_spacing (GTK_PACKER (%s), %i);\n",
		data->wname, GTK_PACKER (widget)->spacing);
  */

  if (GTK_PACKER (widget)->default_border_width != 0)
    source_add (data,
		"  gtk_packer_set_default_border_width (GTK_PACKER (%s), %i);\n",
		data->wname, GTK_PACKER (widget)->default_border_width);

  if (GTK_PACKER (widget)->default_pad_x != 0
      || GTK_PACKER (widget)->default_pad_y != 0)
    {
      source_add (data,
		  "  gtk_packer_set_default_pad (GTK_PACKER (%s), %i, %i);\n",
		  data->wname,
		  GTK_PACKER (widget)->default_pad_x,
		  GTK_PACKER (widget)->default_pad_y);
    }

  if (GTK_PACKER (widget)->default_i_pad_x != 0
      || GTK_PACKER (widget)->default_i_pad_y != 0)
    {
      source_add (data,
		  "  gtk_packer_set_default_ipad (GTK_PACKER (%s), %i, %i);\n",
		  data->wname,
		  GTK_PACKER (widget)->default_i_pad_x,
		  GTK_PACKER (widget)->default_i_pad_y);
    }


  gb_widget_write_standard_source (widget, data);
}


/* Outputs source to add a child widget to a packer. */
static void
gb_packer_write_add_child_source (GtkWidget * parent,
				  const gchar *parent_name,
				  GtkWidget *child,
				  GbWidgetWriteSourceData * data)
{
  const gchar *side = NULL, *anchor = NULL;
  gchar options[48];
  gint i;
  GtkPackerChild *pchild = glade_util_find_packer_child (GTK_PACKER (parent),
							 child);
  g_return_if_fail (pchild != NULL);

  for (i = 0; GbPackerSideChoices[i]; i++)
    {
      if (GbPackerSideValues[i] == pchild->side)
	side = GbPackerSideSymbols[i];
    }
  for (i = 0; GbPackerAnchorChoices[i]; i++)
    {
      if (GbPackerAnchorValues[i] == pchild->anchor)
	anchor = GbPackerAnchorSymbols[i];
    }
  options[0] = '\0';
  if (pchild->options & GTK_PACK_EXPAND)
    strcpy (options, "GTK_PACK_EXPAND");
  if (pchild->options & GTK_FILL_X)
    {
      if (options[0] != '\0')
	strcat (options, " | ");
      strcat (options, "GTK_FILL_X");
    }
  if (pchild->options & GTK_FILL_Y)
    {
      if (options[0] != '\0')
	strcat (options, " | ");
      strcat (options, "GTK_FILL_Y");
    }
  if (options[0] == '\0')
    strcpy (options, "0");


  if (pchild->use_default)
    {
      source_add (data,
		  "  gtk_packer_add_defaults (GTK_PACKER (%s), %s, %s,\n"
		  "                           %s, (GtkPackerOptions) (%s));\n",
		  parent_name, data->wname, side, anchor, options);
    }
  else
    {
      source_add (data,
		  "  gtk_packer_add (GTK_PACKER (%s), %s, %s,\n"
		  "                  %s, (GtkPackerOptions) (%s),\n"
		  "                  %i, %i, %i, %i, %i);\n",
		  parent_name, data->wname, side, anchor, options,
		  pchild->border_width, pchild->pad_x, pchild->pad_y,
		  pchild->i_pad_x, pchild->i_pad_y);
    }
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_packer_init ()
{
  /* Initialise the GTK type */
  gtk_packer_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = packer_xpm;
  gbwidget.tooltip = _("Packer");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_create_properties	= gb_packer_create_properties;
  gbwidget.gb_widget_get_properties	= gb_packer_get_properties;
  gbwidget.gb_widget_set_properties	= gb_packer_set_properties;
  gbwidget.gb_widget_create_child_properties = gb_packer_create_child_properties;
  gbwidget.gb_widget_get_child_properties = gb_packer_get_child_properties;
  gbwidget.gb_widget_set_child_properties = gb_packer_set_child_properties;
  gbwidget.gb_widget_write_source	= gb_packer_write_source;
  gbwidget.gb_widget_write_add_child_source = gb_packer_write_add_child_source;
/*
  gbwidget.gb_widget_new		= gb_packer_new;
  gbwidget.gb_widget_create_popup_menu	= gb_packer_create_popup_menu;
*/

  return &gbwidget;
}

