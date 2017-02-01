/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-object-client.c:
 *   This handles the client-view of a remote Bonobo object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <bonobo/bonobo-object-client.h>
#include <liboaf/liboaf.h>
#include <liboaf/oaf-async.h>
#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-exception.h>

/* Internal data structure for bonobo_object_client_activate_async */
typedef struct {
	BonoboObjectClientAsyncCallback callback;
	gpointer                        user_data;
} BonoboObjectClientAsyncCallbackData;

static BonoboObjectClass *bonobo_object_client_parent_class;

/**
 * bonobo_object_client_construct:
 * @object_client: The BonoboObjectClient object to construct
 * @corba_object: The remote CORBA object this object_client refers to
 *
 * Initializes @object_client with the CORBA object for the
 * Bonobo::Unknown interface provided in @corba_object.
 *
 * Returns: the initialized BonoboObjectClient object.
 */
BonoboObjectClient *
bonobo_object_client_construct (BonoboObjectClient *object_client, CORBA_Object corba_object)
{
	g_return_val_if_fail (BONOBO_IS_OBJECT_CLIENT (object_client), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);
	
	BONOBO_OBJECT (object_client)->corba_objref = corba_object;

	return object_client;
}

/**
 * bonobo_object_activate:
 * @iid: an OAFIID
 * @oaf_flags: activation flags
 *
 * This activates the object from the IID using OAF; you probably
 * don't want to do this, you might want to use bonobo_get_object
 * which does object activation through the moniker system.
 * 
 * You might also consider doing property-based activation using the 
 * the OAF-based capabilities. 
 *
 * Returns: An activated object client or NULL on failure.
 */
BonoboObjectClient *
bonobo_object_activate (const char *iid, gint oaf_flags)
{
	CORBA_Environment ev;
	BonoboObjectClient *object;
	Bonobo_Unknown      corba_object;
	
	g_return_val_if_fail (iid != NULL, NULL);

	/*
	 * We don't want this to do moniker activation too, since we'd
	 * need to break the API to add 'requested_interface' and it's
	 * a different paradigm.
	 */
	CORBA_exception_init (&ev);
	corba_object = oaf_activate_from_id ((gchar *) iid, oaf_flags, NULL, &ev);

	if (BONOBO_EX (&ev) || corba_object == CORBA_OBJECT_NIL) {
		CORBA_exception_free (&ev);
		return NULL;
	}

	CORBA_exception_free (&ev);
	
	object = gtk_type_new (bonobo_object_client_get_type ());

	bonobo_object_client_construct (object, corba_object);

	return object;
}

static void
oaf_activate_async_cb (CORBA_Object activated_object,
		       const char  *error,
		       gpointer     user_data)
{
	BonoboObjectClientAsyncCallbackData *callback_data =
		(BonoboObjectClientAsyncCallbackData *) user_data;
	BonoboObjectClient *object;

	if (activated_object == CORBA_OBJECT_NIL) {
		callback_data->callback (NULL, error, callback_data->user_data);
		g_free(callback_data);
		return;
	}

	object = gtk_type_new (bonobo_object_client_get_type ());
	bonobo_object_client_construct (object, activated_object);

	callback_data->callback (object, NULL, callback_data->user_data);

	g_free (callback_data);
}

/**
 * bonobo_object_activate_async:
 * @iid: an OAFIID
 * @oaf_flags: activation flags
 * @callback: a callback function
 * @user_data: user data
 *
 *   This activates the object from the IID using OAF; you probably
 * don't want to do this; instead do capability based activation
 * using OAF directly.
 */
void
bonobo_object_activate_async (const char                    *iid,
			      gint                           oaf_flags,
			     BonoboObjectClientAsyncCallback callback,
			     gpointer                        user_data)
{
	BonoboObjectClientAsyncCallbackData *callback_data;

	g_return_if_fail (iid != NULL);

	callback_data = g_new0 (BonoboObjectClientAsyncCallbackData, 1);
	callback_data->callback = callback;
	callback_data->user_data = user_data;

	oaf_activate_from_id_async ((gchar*) iid, oaf_flags, 
				    oaf_activate_async_cb, callback_data, NULL);
}

/**
 * bonobo_object_client_from_corba:
 * @corba_object: An existing CORBA object that implements Bonobo_Unknown.
 *
 * Wraps the @corba_object CORBA object reference in a BonoboObjectClient
 * object.  This is typically used if you got a CORBA object yourself and not
 * through one of the activation routines and you want to have a BonoboObjectClient
 * handle to use in any of the Bonobo routines. 
 *
 * Returns: A wrapped BonoboObjectClient.
 */
BonoboObjectClient *
bonobo_object_client_from_corba (Bonobo_Unknown o)
{
	BonoboObjectClient *object;
	
	g_return_val_if_fail (o != CORBA_OBJECT_NIL, NULL);

	object = gtk_type_new (bonobo_object_client_get_type ());
	bonobo_object_client_construct (object, o);

	return object;
}


/**
 * bonobo_object_client_query_interface:
 * @object: object to query interface of
 * @interface_desc: interface description
 * @opt_ev: optional exception environment, or NULL
 * 
 * Queries the object to see if it implements the interface
 * described by @interface_desc. Basically a thin
 * Bonobo_Unknown::query_interface wrapper.
 * 
 * Return value: A valid Bonobo_Unknown reference or
 *               CORBA_OBJECT_NIL if anything untoward happens.
 **/
Bonobo_Unknown
bonobo_object_client_query_interface (BonoboObjectClient *object,
				     const char *interface_desc,
				     CORBA_Environment *opt_ev)
{
	Bonobo_Unknown      interface;
	CORBA_Environment *ev, real_ev;

	g_return_val_if_fail (BONOBO_IS_OBJECT_CLIENT (object), CORBA_OBJECT_NIL);
	g_return_val_if_fail (interface_desc != NULL, CORBA_OBJECT_NIL);

	if (opt_ev)
		ev = opt_ev;
	else {
		ev = &real_ev;
		CORBA_exception_init (ev);
	}

	interface = Bonobo_Unknown_queryInterface (BONOBO_OBJREF (object),
						   interface_desc, ev);
	
        if (BONOBO_EX (ev)) {
		bonobo_object_check_env (BONOBO_OBJECT (object),
					 BONOBO_OBJREF (object), ev);
		if (!opt_ev)
			CORBA_exception_free (ev);

                return CORBA_OBJECT_NIL;
	}

	if (!opt_ev)
		CORBA_exception_free (ev);

	return interface;
}

/**
 * bonobo_object_client_has_interface:
 * @object: object to query interface of
 * @interface_desc: interface description
 * @opt_ev: optional exception environment, or NULL
 * 
 * Queries the object to see if it implements the interface
 * described by @interface_desc. Basically a thin
 * Bonobo_Unknown::query_interface wrapper.
 * 
 * Return value: TRUE if the interface is available else FALSE.
 **/
gboolean
bonobo_object_client_has_interface (BonoboObjectClient *object,
				   const char *interface_desc,
				   CORBA_Environment *opt_ev)
{
	Bonobo_Unknown      interface;

	/* safe type checking in query_interface */
	interface = bonobo_object_client_query_interface (object,
							 interface_desc,
							 opt_ev);

	if (interface != CORBA_OBJECT_NIL) {
		CORBA_Environment *ev, real_ev;

		if (opt_ev)
			ev = opt_ev;
		else {
			ev = &real_ev;
			CORBA_exception_init (ev);
		}

		Bonobo_Unknown_unref  (interface, ev);

		if (BONOBO_EX (ev)) {
			bonobo_object_check_env (BONOBO_OBJECT (object),
						 BONOBO_OBJREF (object), ev);
			if (!opt_ev)
				CORBA_exception_free (ev);
			return FALSE;
		}

		CORBA_Object_release (interface, ev);

		if (BONOBO_EX (ev)) {
			bonobo_object_check_env (BONOBO_OBJECT (object),
						 BONOBO_OBJREF (object), ev);
			if (!opt_ev)
				CORBA_exception_free (ev);
			return FALSE;
		}

		if (!opt_ev)
			CORBA_exception_free (ev);
		return TRUE;
	} else
		return FALSE;
}

/**
 * bonobo_object_client_ref:
 * @object_client: the object client
 * @opt_exception_obj: optional CORBA exception environment
 * 
 * Increments the Bonobo ref count on the remote object.
 **/
void
bonobo_object_client_ref (BonoboObjectClient *object_client,
			  BonoboObject       *opt_exception_obj)
{
	CORBA_Environment ev;
	BonoboObject     *object;

	g_return_if_fail (BONOBO_IS_OBJECT (object_client));

	object = BONOBO_OBJECT (object_client);

	CORBA_exception_init (&ev);

	Bonobo_Unknown_ref (object->corba_objref, &ev);

	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		bonobo_object_check_env (opt_exception_obj?opt_exception_obj:object,
					 object->corba_objref, &ev);
	}

	CORBA_exception_free (&ev);
}

/**
 * bonobo_object_client_unref:
 * @object_client: the object client
 * @opt_exception_obj: optional CORBA exception environment
 * 
 * Decrements the Bonobo ref count on the remote object.
 **/
void
bonobo_object_client_unref (BonoboObjectClient *object_client,
			    BonoboObject       *opt_exception_obj)
{
	CORBA_Environment ev;
	BonoboObject     *object;

	g_return_if_fail (BONOBO_IS_OBJECT (object_client));

	object = BONOBO_OBJECT (object_client);

	CORBA_exception_init (&ev);

	Bonobo_Unknown_unref (object->corba_objref, &ev);

	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		bonobo_object_check_env (opt_exception_obj?opt_exception_obj:object,
					 object->corba_objref, &ev);
	}

	CORBA_exception_free (&ev);
}

static void
bonobo_object_client_destroy (GtkObject *object)
{
	BonoboObject *bonobo_object = BONOBO_OBJECT (object);
	Bonobo_Unknown objref;

	objref = bonobo_object_corba_objref (bonobo_object);
	if (objref != CORBA_OBJECT_NIL) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		Bonobo_Unknown_unref (objref, &ev);
		CORBA_exception_free (&ev);
	}

	GTK_OBJECT_CLASS (bonobo_object_client_parent_class)->destroy (object);
}

static void
bonobo_object_client_class_init (BonoboObjectClientClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_object_client_parent_class = gtk_type_class (bonobo_object_get_type ());

	object_class->destroy = bonobo_object_client_destroy;
}

/**
 * bonobo_object_client_get_type:
 *
 * Returns: the GtkType for the BonoboObjectClient class.
 */
GtkType
bonobo_object_client_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"Handle to remote Bonobo::Unknown",
			sizeof (BonoboObjectClient),
			sizeof (BonoboObjectClientClass),
			(GtkClassInitFunc) bonobo_object_client_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}
