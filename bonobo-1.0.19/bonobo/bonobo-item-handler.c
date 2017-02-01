/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-item-handler.c: a generic Item Container resolver (implements ItemContainer)
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 2000 Miguel de Icaza.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkwidget.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>
#include "bonobo-item-handler.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

/*
 * Returns a list of the objects in this container
 */
static Bonobo_ItemContainer_ObjectNames *
impl_enum_objects (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboItemHandler *handler = BONOBO_ITEM_HANDLER (object);

	if (handler->enum_objects)
		return handler->enum_objects (handler, handler->user_data, ev);
	else
		return Bonobo_ItemContainer_ObjectNames__alloc ();
}

static Bonobo_Unknown
impl_get_object (PortableServer_Servant servant,
		 const CORBA_char      *item_name,
		 CORBA_boolean          only_if_exists,
		 CORBA_Environment     *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboItemHandler *handler = BONOBO_ITEM_HANDLER (object);

	if (handler->get_object)
		return handler->get_object (handler, item_name,
					    only_if_exists,
					    handler->user_data, ev);
	else
		return CORBA_OBJECT_NIL;
}

static void
bonobo_item_handler_class_init (BonoboItemHandlerClass *klass)
{
	POA_Bonobo_ItemContainer__epv *epv = &klass->epv;

	epv->enumObjects     = impl_enum_objects;
	epv->getObjectByName = impl_get_object;
}

static void 
bonobo_item_handler_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboItemHandler, 
			   Bonobo_ItemContainer,
			   PARENT_TYPE,
			   bonobo_item_handler);

/**
 * bonobo_item_handler_construct:
 * @container: The handler object to construct
 * @corba_container: The CORBA object that implements Bonobo::ItemContainer
 *
 * Constructs the @container Gtk object using the provided CORBA
 * object.
 *
 * Returns: The constructed BonoboItemContainer object.
 */
BonoboItemHandler *
bonobo_item_handler_construct (BonoboItemHandler             *handler,
			       BonoboItemHandlerEnumObjectsFn enum_objects,
			       BonoboItemHandlerGetObjectFn   get_object,
			       gpointer                       user_data)
{
	g_return_val_if_fail (handler != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ITEM_HANDLER (handler), NULL);
	
	handler->get_object   = get_object;
	handler->enum_objects = enum_objects;
	handler->user_data    = user_data;
	
	return handler;
}

/**
 * bonobo_item_handler_new:
 *
 * Creates a new BonoboItemHandler object.  These are used to hold
 * client sites.
 *
 * Returns: The newly created BonoboItemHandler object
 */
BonoboItemHandler *
bonobo_item_handler_new (BonoboItemHandlerEnumObjectsFn enum_objects,
			 BonoboItemHandlerGetObjectFn   get_object,
			 gpointer                       user_data)

{
	BonoboItemHandler *handler;

	handler = gtk_type_new (bonobo_item_handler_get_type ());

	return bonobo_item_handler_construct (
		handler, enum_objects, get_object, user_data);
}

/**
 * bonobo_parse_item_options:
 * @option_string: a string with a list of options
 *
 * The bonobo_parse_item_options() routine parses the
 * @option_string which is a semi-colon separated list
 * of arguments.
 *
 * Each argument is of the form value[=key].  The entire
 * option string is defined by:
 *
 * option_string := keydef
 *                | keydef ; option_string
 *
 * keydef := value [=key]
 *
 * The key can be literal values, values with spaces, and the
 * \ character is used as an escape sequence.  To include a
 * literal ";" in a value you can use \;.
 *
 * Returns: A GSList that contains structures of type BonoboItemOption
 * each BonoboItemOption
 */
GSList *
bonobo_item_option_parse (const char *option_string)
{
	GSList *list = NULL;
	GString *key = NULL;
	BonoboItemOption *option = NULL;
	const char *p;
	
	for (p = option_string; *p; p++) {
		if (*p == '=' ) {
			GString *value = NULL;
			if (!key)
				return list;
			
			option = g_new0 (BonoboItemOption, 1);
			option->key = key->str;
			g_string_free (key, FALSE);
			key = NULL;

			for (p++; *p; p++) {
				if (*p == ';')
					goto next;
				if (!value)
					value = g_string_new ("");

				if (*p == '\\') {
					p++;
					if (*p == 0)
						break;
					g_string_append_c (value, *p);
					continue;
				}
				g_string_append_c (value, *p);
			}
		next:
			if (value) {
				option->value = value->str;
				g_string_free (value, FALSE);
			}
			list = g_slist_append (list, option);
			if (*p == 0)
				break;
		} else {
			if (key == NULL)
				key = g_string_new ("");
			g_string_append_c (key, *p);
		}
	}

	if (key) {
		BonoboItemOption *option = g_new (BonoboItemOption, 1);
		
		option->key = key->str;
		g_string_free (key, FALSE);
		
		list = g_slist_append (list, option);
	}
	
	return list;
}

/** 
 * bonobo_item_options_free:
 * @options: a GSList of BonoboItemOption structures that was returned by bonobo_item_option_parse()
 *
 * Use this to release a list returned by bonobo_item_option_parse()
 */
void
bonobo_item_options_free (GSList *options)
{
	GSList *l;

	for (l = options; l; l = l->next) {
		BonoboItemOption *option = l->data;

		g_free (option->key);
		if (option->value)
			g_free (option->value);
	}

	g_slist_free (options);
}
