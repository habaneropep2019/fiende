/*
 * bonobo-calculator-control.c
 *
 * Author:
 *    Michael Meeks (mmeeks@gnu.org)
 *    Nat Friedman  (nat@nat.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-calculator.h>
#include <bonobo.h>

#include "bonobo-calculator-control.h"

static void
set_prop (BonoboPropertyBag *bag,
	  const BonoboArg   *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	gnome_calculator_set (user_data, BONOBO_ARG_GET_DOUBLE (arg));
}

static void
get_prop (BonoboPropertyBag *bag,
	  BonoboArg         *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	GnomeCalculator *calc = user_data;

	BONOBO_ARG_SET_DOUBLE (arg, calc->result);
}

BonoboObject *
bonobo_calculator_control_new (void)
{
	BonoboControl     *control;
	GtkWidget	  *calc;
	BonoboPropertyBag *pb;

	calc = gnome_calculator_new ();
	gtk_widget_show (calc);

	control = bonobo_control_new (calc);

	pb = bonobo_property_bag_new (get_prop, set_prop, calc);
	bonobo_control_set_properties (control, pb);
	bonobo_object_unref (BONOBO_OBJECT (pb));

	bonobo_property_bag_add (pb, "value", 1, BONOBO_ARG_DOUBLE, NULL,
				 "Caluculation result", 0);

	bonobo_control_set_automerge (control, TRUE);

	return BONOBO_OBJECT (control);
}
