/**
 * bonobo-selector.c: A control containing a BonoboSelector.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */

/*
 * FIXME, vital features:
 *
 * ItemContainer - OAFIID:SelectorFactory!IDL:Bonobo/Control:1.0
 * Attach lots of lovely properties, perhaps an event source for double clicks
 */

#include <config.h>

#include <bonobo.h>

enum {
	PROP_INTERFACE,
	PROP_ID,
	PROP_NAME,
	PROP_DESCRIPTION
};

static void
set_prop (BonoboPropertyBag *bag,
	  const BonoboArg   *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	g_return_if_fail (BONOBO_IS_SELECTOR_WIDGET (user_data));

	switch (arg_id) {
	case PROP_INTERFACE: {
		const gchar *query [2];

		query [0] = BONOBO_ARG_GET_STRING (arg);
		query [1] = NULL;

		bonobo_selector_widget_set_interfaces (
			user_data, query);
		break;
	}

	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static void
get_prop (BonoboPropertyBag *bag,
	  BonoboArg         *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	g_return_if_fail (BONOBO_IS_SELECTOR_WIDGET (user_data));

	switch (arg_id) {
	case PROP_INTERFACE:
		g_warning ("Cannot read interface");
		BONOBO_ARG_SET_STRING (arg, "");
		break;
	case PROP_ID:
		BONOBO_ARG_SET_STRING (
			arg, bonobo_selector_widget_get_id (user_data));
		break;
	case PROP_NAME:
		BONOBO_ARG_SET_STRING (
			arg, bonobo_selector_widget_get_name (user_data));
		break;
	case PROP_DESCRIPTION:
		BONOBO_ARG_SET_STRING (
			arg, bonobo_selector_widget_get_description (user_data));
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		break;
	}
}

static BonoboObject *
generic_factory (BonoboGenericFactory *this, void *data)
{
	GtkWidget         *widget;
	const gchar       *query[] = { "IDL:Bonobo/Control:1.0", NULL };
	BonoboControl     *bonobo_object;
	BonoboPropertyBag *pb;

	widget = bonobo_selector_widget_new ();
	bonobo_selector_widget_set_interfaces (
		BONOBO_SELECTOR_WIDGET (widget), query);

	bonobo_object = bonobo_control_new (widget);

	pb = bonobo_property_bag_new (get_prop, set_prop, widget);
	bonobo_control_set_properties (BONOBO_CONTROL (bonobo_object), pb);
	bonobo_object_unref (BONOBO_OBJECT (pb));

	bonobo_property_bag_add (pb, "interface", PROP_INTERFACE,
				 BONOBO_ARG_STRING, NULL,
				 "Interface that must be supported",
				 BONOBO_PROPERTY_WRITEABLE);

	bonobo_property_bag_add (pb, "id", PROP_ID,
				 BONOBO_ARG_STRING, NULL,
				 "Selected OAFIID",
				 BONOBO_PROPERTY_READABLE);

	bonobo_property_bag_add (pb, "name", PROP_NAME,
				 BONOBO_ARG_STRING, NULL,
				 "name of selected component",
				 BONOBO_PROPERTY_READABLE);

	bonobo_property_bag_add (pb, "description", PROP_DESCRIPTION,
				 BONOBO_ARG_STRING, NULL,
				 "descroption of selected component",
				 BONOBO_PROPERTY_READABLE);

	return BONOBO_OBJECT (bonobo_object);
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Selector_ControlFactory",
		    "bonobo-selector", VERSION,
		    generic_factory,
		    NULL)
