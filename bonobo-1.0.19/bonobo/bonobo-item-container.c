/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-item-container.h: a generic container for monikers.
 *
 * The BonoboItemContainer object represents a document which may have one
 * or more embedded document objects.  To embed an object in the
 * container, create a BonoboClientSite, add it to the container, and
 * then create an object supporting Bonobo::Embeddable and bind it to
 * the client site.  The BonoboItemContainer maintains a list of the client
 * sites which correspond to the objects embedded in the container.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman    (nat@nat.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkwidget.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-item-container.h>
#include <bonobo/bonobo-client-site.h>

enum {
	GET_OBJECT,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *bonobo_item_container_parent_class;

struct _BonoboItemContainerPrivate {
	GHashTable *objects;
};

static gboolean
remove_object (gpointer key,
	       gpointer value,
	       gpointer user_data)
{
	g_free (key);
	bonobo_object_unref (value);

	return TRUE;
}

static void
bonobo_item_container_destroy (GtkObject *object)
{
	BonoboItemContainer *container = BONOBO_ITEM_CONTAINER (object);

	/* Destroy all the ClientSites. */
	g_hash_table_foreach_remove (container->priv->objects,
				     remove_object, NULL);

	bonobo_item_container_parent_class->destroy (object);
}

static void
bonobo_item_container_finalize (GtkObject *object)
{
	BonoboItemContainer *container = BONOBO_ITEM_CONTAINER (object);

	g_hash_table_destroy (container->priv->objects);
	g_free (container->priv);
	
	bonobo_item_container_parent_class->finalize (object);
}

static void
get_object_names (gpointer key, gpointer value, gpointer user_data)
{
	GSList **l = user_data;

	*l = g_slist_prepend (*l, CORBA_string_dup (key));
}

/*
 * Returns a list of the objects in this container
 */
static Bonobo_ItemContainer_ObjectNames *
impl_Bonobo_ItemContainer_enumObjects (PortableServer_Servant servant,
				       CORBA_Environment     *ev)
{
	Bonobo_ItemContainer_ObjectNames *list;
	BonoboItemContainer              *container;
	GSList                           *objects, *l;
	int                               i;

	container = BONOBO_ITEM_CONTAINER (
		bonobo_object_from_servant (servant));
	g_return_val_if_fail (container != NULL, NULL);

	list = Bonobo_ItemContainer_ObjectNames__alloc ();
	if (!list)
		return NULL;

	objects = NULL;
	g_hash_table_foreach (container->priv->objects,
			      get_object_names, &objects);

	list->_length = list->_maximum = g_slist_length (objects);
	
	list->_buffer = CORBA_sequence_CORBA_string_allocbuf (list->_length);
	if (!list->_buffer) {
		GSList *l;
		CORBA_free (list);
		for (l = objects; l; l = l->next)
			CORBA_free (l->data);
		g_slist_free (objects);
		return NULL;
	}
	
	/* Assemble the list of objects */
	for (i = 0, l = objects; l; l = l->next)
		list->_buffer [i++] = l->data;

	g_slist_free (objects);

	return list;
}

static Bonobo_Unknown
impl_Bonobo_ItemContainer_getObjectByName (PortableServer_Servant servant,
					   const CORBA_char      *item_name,
					   CORBA_boolean          only_if_exists,
					   CORBA_Environment     *ev)
{
	Bonobo_Unknown ret;
	
	gtk_signal_emit (
		GTK_OBJECT (bonobo_object_from_servant (servant)),
		signals [GET_OBJECT], item_name, only_if_exists, ev, &ret);

	return ret;
}

typedef Bonobo_Unknown (*GnomeSignal_POINTER__POINTER_BOOL_POINTER) (
	BonoboItemContainer *item_container,
	CORBA_char *item_name,
	CORBA_boolean only_if_exists,
	CORBA_Environment *ev,
	gpointer func_data);

static void 
gnome_marshal_POINTER__POINTER_BOOL_POINTER (GtkObject * object,
					     GtkSignalFunc func,
					     gpointer func_data,
					     GtkArg * args)
{
	GnomeSignal_POINTER__POINTER_BOOL_POINTER rfunc;
	void **return_val;
	return_val = GTK_RETLOC_POINTER (args[3]);
	rfunc = (GnomeSignal_POINTER__POINTER_BOOL_POINTER) func;
	*return_val = (*rfunc) (BONOBO_ITEM_CONTAINER (object),
				GTK_VALUE_POINTER (args[0]),
				GTK_VALUE_BOOL (args[1]),
				GTK_VALUE_POINTER (args[2]),
				func_data);
}

/* BonoboItemContainer class initialization routine  */
static void
bonobo_item_container_class_init (BonoboItemContainerClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_ItemContainer__epv *epv = &klass->epv;

	bonobo_item_container_parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_item_container_destroy;
	object_class->finalize = bonobo_item_container_finalize;

	signals [GET_OBJECT] =
		gtk_signal_new  (
			"get_object",
			GTK_RUN_LAST,
			object_class->type,
			0,
			gnome_marshal_POINTER__POINTER_BOOL_POINTER,
			GTK_TYPE_POINTER,
			3,
			GTK_TYPE_POINTER, GTK_TYPE_BOOL, GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	epv->enumObjects     = impl_Bonobo_ItemContainer_enumObjects;
	epv->getObjectByName = impl_Bonobo_ItemContainer_getObjectByName;
}

/*
 * BonoboItemContainer instance initialization routine
 */
static void
bonobo_item_container_init (BonoboItemContainer *container)
{
	container->priv = g_new0 (BonoboItemContainerPrivate, 1);
	container->priv->objects = g_hash_table_new (
		g_str_hash, g_str_equal);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboItemContainer, 
			   Bonobo_ItemContainer,
			   PARENT_TYPE,
			   bonobo_item_container);

/**
 * bonobo_item_container_new:
 *
 * Creates a new BonoboItemContainer object.  These are used to hold
 * client sites.
 *
 * Returns: The newly created BonoboItemContainer object
 */
BonoboItemContainer *
bonobo_item_container_new (void)
{
	return gtk_type_new (bonobo_item_container_get_type ());
}

/**
 * bonobo_item_container_add:
 * @container: The object to operate on.
 * @name: The name of the object
 * @object: The object to add to the container
 *
 * Adds the @object to the list of objects managed by this
 * container
 */
void
bonobo_item_container_add (BonoboItemContainer *container,
			   const char          *name,
			   BonoboObject        *object)
{
	g_return_if_fail (name != NULL);
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (BONOBO_IS_ITEM_CONTAINER (container));

	if (g_hash_table_lookup (container->priv->objects, name)) {
		g_warning ("Object of name '%s' already exists", name);
	} else {
		bonobo_object_ref (object);
		g_hash_table_insert (container->priv->objects,
				     g_strdup (name),
				     object);
	}
}

/**
 * bonobo_item_container_remove_by_name:
 * @container: The object to operate on.
 * @name: The name of the object to remove from the container
 *
 * Removes the named object from the @container
 */
void
bonobo_item_container_remove_by_name (BonoboItemContainer *container,
				      const char          *name)
{
	gpointer key, value;

	g_return_if_fail (name != NULL);
	g_return_if_fail (BONOBO_IS_ITEM_CONTAINER (container));

	if (!g_hash_table_lookup_extended (container->priv->objects, name,
					   &key, &value))
		g_warning ("Removing '%s' but not in container", name);
	else {
		g_hash_table_remove (container->priv->objects, name);
		g_free (key);
		bonobo_object_unref (value);
	}
}

