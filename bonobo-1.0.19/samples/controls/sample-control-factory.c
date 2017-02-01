/**
 * sample-control-factory.c
 *
 * Author:
 *   Nat Friedman  (nat@nat.org)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>

#include "bonobo-clock-control.h"
#include "bonobo-calculator-control.h"

static BonoboObject *
control_factory (BonoboGenericFactory *this,
		 const char           *object_id,
		 void                 *data)
{
	BonoboObject *object  = NULL;
	
	g_return_val_if_fail (object_id != NULL, NULL);

	if (!strcmp (
		object_id,
		"OAFIID:Bonobo_Sample_Clock"))
		object = bonobo_clock_control_new ();
	
	else if (!strcmp (
		object_id,
		"OAFIID:Bonobo_Sample_Calculator"))
		object = bonobo_calculator_control_new ();
	
	else if (!strcmp (
		object_id,
		"OAFIID:Bonobo_Sample_Entry"))
		object = bonobo_entry_control_new ();

	return object;
}

BONOBO_OAF_FACTORY_MULTI ("OAFIID:Bonobo_Sample_ControlFactory",
			  "bonobo-sample-controls", VERSION,
			  control_factory,
			  NULL)
