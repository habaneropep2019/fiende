/*
 * bonobo-clock-control.c
 *
 * Authors:
 *    Nat Friedman  (nat@helixcode.com)
 *    Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000, Helix Code, Inc.
 */

#include <config.h>
#include <gnome.h>
#include <bonobo.h>

#include <libgnomeui/gtk-clock.h>

#include "bonobo-clock-control.h"

enum {
	PROP_RUNNING
} MyArgs;

#define RUNNING_KEY  "Clock::Running"

static void
get_prop (BonoboPropertyBag *bag,
	  BonoboArg         *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	GtkObject *clock = user_data;

	switch (arg_id) {

	case PROP_RUNNING:
	{
		gboolean b = GPOINTER_TO_UINT (gtk_object_get_data (clock, RUNNING_KEY));
		BONOBO_ARG_SET_BOOLEAN (arg, b);
		break;
	}

	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static void
set_prop (BonoboPropertyBag *bag,
	  const BonoboArg   *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	GtkClock *clock = user_data;

	switch (arg_id) {

	case PROP_RUNNING:
	{
		guint i;

		i = BONOBO_ARG_GET_BOOLEAN (arg);

		if (i)
			gtk_clock_start (clock);
		else
			gtk_clock_stop (clock);

		gtk_object_set_data (GTK_OBJECT (clock), RUNNING_KEY,
				     GUINT_TO_POINTER (i));
		break;
	}

	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

BonoboObject *
bonobo_clock_control_new (void)
{
	BonoboPropertyBag  *pb;
	BonoboControl      *control;
	GtkWidget	   *clock;
	BonoboArg          *def;

	/* Create the control. */
	clock = gtk_clock_new (GTK_CLOCK_INCREASING);
	gtk_clock_start (GTK_CLOCK (clock));
	gtk_widget_show (clock);
	gtk_object_set_data (GTK_OBJECT (clock), RUNNING_KEY,
			     GUINT_TO_POINTER (1));

	control = bonobo_control_new (clock);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);

	/* Create the properties. */
	pb = bonobo_property_bag_new (get_prop, set_prop, clock);
	bonobo_control_set_properties (control, pb);

	bonobo_property_bag_add (pb, "running", PROP_RUNNING,
				 BONOBO_ARG_BOOLEAN, def,
				 "Whether or not the clock is running", 0);

	bonobo_object_unref (BONOBO_OBJECT (pb));

	return BONOBO_OBJECT (control);
}

BonoboObject *
bonobo_entry_control_new (void)
{
	BonoboPropertyBag  *pb;
	BonoboControl      *control;
	GtkWidget	   *entry;

	/* Create the control. */
	entry = gtk_entry_new ();
	gtk_widget_show (entry);

	control = bonobo_control_new (entry);
	pb = bonobo_property_bag_new (NULL, NULL, NULL);
	bonobo_control_set_properties (control, pb);
	bonobo_object_unref (BONOBO_OBJECT (pb));

	bonobo_property_bag_add_gtk_args (pb, GTK_OBJECT (entry));

	return BONOBO_OBJECT (control);
}
