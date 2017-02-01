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

#ifdef ENABLE_BONOBO

#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkspinbutton.h>

#include "../gb.h"
#include "../palette.h"

#include <bonobo.h>
#include <bonobo/bonobo-object-directory.h>

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/entry.xpm"
#include "../graphics/gnome-calculator.xpm"
#include "../graphics/gtk-clock.xpm"
#include "../graphics/gnome-animator.xpm"
#include "../graphics/custom.xpm"
#include "../graphics/calendar.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */

#define PBC_KEY   "PROPERTY_BAG_CLIENT"

typedef struct {
	GbWidget gbwidget;
	char    *obj_id;
	char    *descr;
} BonoboGbWidget;

static GtkWidget *
control_create (char *obj_id)
{
  GtkWidget               *widget;
  BonoboControlFrame      *cf;
  Bonobo_PropertyBag       pb;

  g_return_val_if_fail (obj_id != NULL, NULL);

  widget = bonobo_widget_new_control (obj_id, CORBA_OBJECT_NIL);
  
  g_return_val_if_fail (widget != NULL, NULL);
  cf = bonobo_widget_get_control_frame (BONOBO_WIDGET (widget));

  if (!cf) {
    g_warning ("Control has no frame!");
    gtk_widget_destroy (widget);

    return NULL;
  }

  pb = bonobo_control_frame_get_control_property_bag (cf, NULL);

  gtk_object_set_data (GTK_OBJECT (widget), PBC_KEY, pb);

  return widget;
}

static Bonobo_PropertyBag
control_get_pb (GtkWidget *widget)
{
  return gtk_object_get_data (GTK_OBJECT (widget),
			      PBC_KEY);
}

GtkWidget *
gb_bonobo_control_new (GbWidgetNewData * data)
{
  GbWidget     *gbwidget;
  GladeWidgetData *w;

  w = data->widget_data;
  g_return_val_if_fail (w != NULL, NULL);

  gbwidget = w->gbwidget;
  g_return_val_if_fail (gbwidget != NULL, NULL);
  
  return control_create (gbwidget->class_id);
}

inline static char *
create_prop_name (const char *txt)
{
  return g_strconcat ("BonoboWidget::", txt, NULL);
}

/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_bonobo_control_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  Bonobo_PropertyBag pb = control_get_pb (widget);
  GList *names;

  if (!pb)
	  return;

  for (names = bonobo_property_bag_client_get_property_names (pb, NULL);
       names; names = names->next) {
    CORBA_TypeCode tc;
    char          *doc;
    char          *prop = create_prop_name (names->data);

    tc  = bonobo_property_bag_client_get_property_type (pb, names->data, NULL);
    doc = bonobo_property_bag_client_get_docstring (pb, names->data, NULL);
    switch (tc->kind) {
    case CORBA_tk_boolean:
      property_add_bool (prop, names->data, doc);
      break;
    case CORBA_tk_string:
      property_add_string (prop, names->data, doc);
      break;
    case CORBA_tk_short:
      property_add_int (prop, names->data, doc);
      break;
    case CORBA_tk_double:
    case CORBA_tk_float:
      property_add_float (prop, names->data, doc);
      break;
    case CORBA_tk_long:
      property_add_int (prop, names->data, doc);
      break;
    default:
      g_warning ("Unhandled type %d", tc->kind);
      break;
    }
    g_free (prop);
  }
}

/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_bonobo_control_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  Bonobo_PropertyBag pb = control_get_pb (widget);
  GList      *names;
  GbWidget   *gbwidget;

  g_return_if_fail (pb != NULL);

  gbwidget = gb_widget_lookup (widget);

  for (names = bonobo_property_bag_client_get_property_names (pb, NULL);
       names; names = names->next) {
    CORBA_TypeCode tc;
    char          *prop = create_prop_name (names->data);

    tc  = bonobo_property_bag_client_get_property_type (pb, names->data, NULL);
    switch (tc->kind) {
    case CORBA_tk_boolean:
      gb_widget_output_bool (data, prop,
			     bonobo_property_bag_client_get_value_gboolean (pb, names->data, NULL));
      break;
    case CORBA_tk_string:
    {
      char *str = bonobo_property_bag_client_get_value_string (pb, names->data, NULL);

      gb_widget_output_string (data, prop, str);

      g_free (str);
      break;
    }
    case CORBA_tk_long:
      gb_widget_output_int (data, prop,
			    bonobo_property_bag_client_get_value_glong (pb, names->data, NULL));
      break;
    case CORBA_tk_float:
      gb_widget_output_float (data, prop,
			      bonobo_property_bag_client_get_value_gfloat (pb, names->data, NULL));
      break;
    case CORBA_tk_double:
      gb_widget_output_float (data, prop,
			      bonobo_property_bag_client_get_value_gdouble (pb, names->data, NULL));
      break;
    default:
      g_warning ("Unhandled type %d", tc->kind);
      break;
    }
    g_free (prop);
  }
}


/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_bonobo_control_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  Bonobo_PropertyBag pb = control_get_pb (widget);

  GList *names;

  g_return_if_fail (pb != NULL);

  for (names = bonobo_property_bag_client_get_property_names (pb, NULL);
       names; names = names->next) {
    CORBA_TypeCode tc;
    char          *prop = create_prop_name (names->data);

    tc  = bonobo_property_bag_client_get_property_type (pb, names->data, NULL);
    switch (tc->kind) {
    case CORBA_tk_boolean:
    {
      gboolean val;

      val = gb_widget_input_bool (data, prop);
      if (data->apply)
        bonobo_property_bag_client_set_value_gboolean (pb, names->data, val, NULL);
      break;
    }
    case CORBA_tk_string:
    {
      const char *str;

      str = gb_widget_input_string (data, prop);
      if (data->apply)
        bonobo_property_bag_client_set_value_string (pb, names->data, str, NULL);

      break;
    }
    case CORBA_tk_float:
    {
      gfloat val;

      val = gb_widget_input_float (data, prop);
      if (data->apply)
        bonobo_property_bag_client_set_value_gfloat (pb, names->data, val, NULL);
      break;
    }
    case CORBA_tk_double:
    {
      gdouble val;

      val = gb_widget_input_float (data, prop);
      if (data->apply)
        bonobo_property_bag_client_set_value_gdouble (pb, names->data, val, NULL);
      break;
    }
    case CORBA_tk_long:
    {
      glong val;

      val = gb_widget_input_int (data, prop);
      if (data->apply)
        bonobo_property_bag_client_set_value_glong (pb, names->data, val, NULL);
      break;
    }
    default:
      g_warning ("Unhandled type %d", tc->kind);
      break;
    }
    g_free (prop);
  }
}

GbWidget *
bonobo_gb_widget_new (const char *obj_id, const char *descr)
{
	BonoboGbWidget *w = g_new (BonoboGbWidget, 1);

	w->obj_id = g_strdup (obj_id);
	w->descr  = g_strdup (descr);

	gb_widget_init_struct (&w->gbwidget);

	/* This is an ugly hack for my nice demo */
	if (!strcmp (obj_id, "OAFIID:bonobo_clock:d42cc651-44ae-4f69-a10d-a0b6b2cc6ecc"))
		w->gbwidget.pixmap_struct = gtk_clock_xpm;
	else if (!strcmp (obj_id, "OAFIID:bonobo_calculator:fab8c2a7-9576-437c-aa3a-a8617408970f"))
		w->gbwidget.pixmap_struct = gnome_calculator_xpm;
	else if (!strcmp (obj_id, "OAFIID:bonobo_entry:04e49c0b-95e2-4305-88a7-9f6721ddfa51"))
		w->gbwidget.pixmap_struct = entry_xpm;
	else if (!strcmp (obj_id, "OAFIID:eog_animator:0548c96d-7b3e-4bdb-9842-258584a36cae"))
		w->gbwidget.pixmap_struct = gnome_animator_xpm;
	else if (!strcmp (obj_id, "OAFIID:control:calendar:dd34ddae-25c6-486b-a8a8-3e8f0286b54c"))
		w->gbwidget.pixmap_struct = calendar_xpm;
	else
		w->gbwidget.pixmap_struct = custom_xpm;

	/* Fill in the pixmap struct & tooltip */
	w->gbwidget.tooltip       = w->descr;
	
	/* Fill in any functions that this Gbwidget has */
	w->gbwidget.gb_widget_new = gb_bonobo_control_new;
	w->gbwidget.gb_widget_create_properties = gb_bonobo_control_create_properties;
	w->gbwidget.gb_widget_get_properties = gb_bonobo_control_get_properties;
	w->gbwidget.gb_widget_set_properties = gb_bonobo_control_set_properties;

	MSG2 ("goad id '%s' '%s'\n", w->obj_id, w->descr);
	gb_widget_register_gbwidget (w->obj_id, (GbWidget *)w);
	palette_add_gbwidget ((GbWidget *)w, "Controls", w->obj_id);
	
	return (GbWidget *)w;
}

void
bonobo_gb_widget_destroy (GbWidget *gbwidget)
{
	BonoboGbWidget *w = (BonoboGbWidget *)gbwidget;

	if (w->obj_id)
		g_free (w->obj_id);
	w->obj_id = NULL;

	if (w->descr)
		g_free (w->descr);
	w->descr = NULL;
}

/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
void
gb_bonobo_control_init ()
{
	const char *ids [] = { "IDL:Bonobo/Control:1.0", NULL };
	GList *controls, *l;

	/* Initialise the GTK type */
	bonobo_widget_get_type ();

	controls = bonobo_directory_get_server_list (ids);
	for (l = controls; l; l = l->next) {
		static GbWidget *gbwidget;

		const char *obj_id = bonobo_directory_get_server_info_id (l->data);
		const char *desc   = bonobo_directory_get_server_info_description (l->data);

		gbwidget = bonobo_gb_widget_new (obj_id, desc);
		bonobo_directory_server_info_unref (l->data);
	}
	g_list_free (controls);
}

#endif
