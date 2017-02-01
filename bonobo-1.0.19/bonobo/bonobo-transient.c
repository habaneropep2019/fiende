/**
 * Bonobo transient object implementation.
 *
 * This simplifies the creation of POA managers for transient objects.
 * Objects living in this POA are created on demand and destroyed after use.
 *
 * Authors:
 *   Nat Friedman    (nat@helixcode.com)
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * I just refactored the code from the original PropertyBag, all the smart hacks
 * are from Nat -mig.
 *
 * (C) 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-transient.h>

static GtkObjectClass *parent_class = NULL;

/*
 * BonoboTransient POA and Servant Manager.
 */
typedef struct {
	POA_PortableServer_ServantLocator servant_locator;
	BonoboTransient *bonobo_transient;
} BonoboTransientServantManager;

struct _BonoboTransientPriv {
	BonoboTransientServantNew     new_servant;
	BonoboTransientServantDestroy destroy_servant;
	gpointer                      callback_data;
	PortableServer_POA            poa;	
};

/*
 * This ServantManager method is invoked before a method
 * on a BonoboTransient is called.  It creates the servant
 * for the object and returns it.
 */
static PortableServer_Servant
bonobo_transient_servant_locator_preinvoke (PortableServer_Servant servant_manager,
					    PortableServer_ObjectId *oid,
					    PortableServer_POA adapter,
					    CORBA_Identifier op_name,
					    PortableServer_ServantLocator_Cookie *cookie,
					    CORBA_Environment *ev)
{
	BonoboTransientServantManager *sm;
	PortableServer_Servant servant = NULL;
	BonoboTransient *transient, **cookie_val;
	char *object_name;

	/*
	 * Get the TransientManager out of the servant manager.
	 */
	sm = (BonoboTransientServantManager *) servant_manager;
	transient = sm->bonobo_transient;

	/*
	 * Grab the Property name and the Property Bag.
	 */
	object_name = PortableServer_ObjectId_to_string (oid, ev);
	if (BONOBO_EX (ev)) {
		CORBA_free (object_name);
		g_warning ("BonoboPropertyBag: Could not get property name from Object ID");
		return NULL;
	}

	/*
	 * Create a temporary servant 
	 */
	servant = transient->priv->new_servant (
		adapter, transient, object_name, transient->priv->callback_data);
	CORBA_free (object_name);
	if (servant == NULL) {
		g_warning ("BonoboPropertyBag: Could not create transient Property servant");
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return NULL;
	}

	/*
	 * The cookie is arbitrary data which will get passed to
	 * postinvoke for this Property method invocation.  We have no
	 * use for it.
	 */
	cookie_val = g_new (BonoboTransient *, 1);
	*cookie_val = transient;
	*cookie = cookie_val;

	return servant;
}

/*
 * This method is invoked after a BonoboProperty method invocation.
 * It destroys the transient Property servant.
 */
static void
bonobo_transient_servant_locator_postinvoke (PortableServer_Servant servant_manager,
					     PortableServer_ObjectId *oid,
					     PortableServer_POA adapter,
					     CORBA_Identifier op_name,
					     PortableServer_ServantLocator_Cookie cookie,
					     PortableServer_Servant servant,
					     CORBA_Environment *ev)
{
	BonoboTransient *transient = 
	        BONOBO_TRANSIENT (*((BonoboTransient **)cookie));
	
	transient->priv->destroy_servant (servant, transient->priv->callback_data);

	g_free (cookie);
}

static PortableServer_ServantBase__epv *
bonobo_transient_get_servant_base_epv (void)
{
	PortableServer_ServantBase__epv *epv;

	epv = g_new0 (PortableServer_ServantBase__epv, 1);

	epv->default_POA = PortableServer_ServantBase__default_POA;
	epv->finalize    = PortableServer_ServantBase__fini;

	return epv;
}


static POA_PortableServer_ServantManager__epv *
bonobo_transient_get_servant_manager_epv (void)
{
	POA_PortableServer_ServantManager__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantManager__epv, 1);

	return epv;
}

static POA_PortableServer_ServantLocator__epv *
bonobo_transient_get_servant_locator_epv (void)
{
	POA_PortableServer_ServantLocator__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantLocator__epv, 1);

	epv->preinvoke  = bonobo_transient_servant_locator_preinvoke;
	epv->postinvoke = bonobo_transient_servant_locator_postinvoke;

	return epv;
}

static POA_PortableServer_ServantLocator__vepv *
bonobo_transient_get_servant_locator_vepv (void)
{
	static POA_PortableServer_ServantLocator__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_PortableServer_ServantLocator__vepv, 1);

	vepv->_base_epv				= bonobo_transient_get_servant_base_epv ();
	vepv->PortableServer_ServantManager_epv = bonobo_transient_get_servant_manager_epv ();
	vepv->PortableServer_ServantLocator_epv = bonobo_transient_get_servant_locator_epv ();

	return vepv;
}

/**
 * bonobo_transient_construct:
 * @transient: the BonoboTransient to construct
 * @parent_poa: the POA where the object is created, CORBA_OBJECT_NIL for the default Bonobo POA.
 * @new_servant: A function pointer used to incarnate servants on demand.
 * @destroy_servant: A function pointer used to destroy the on-demand server.
 * @data: data passed to the @new_servant and @destroy_servant functions.
 *
 * This function is only for wrappers and object derivation.  For normal
 * use, please see #bonobo_transient_new.
 *
 * This function will return %NULL on failure; however it is your
 * responsibility to destroy the failed object in that case.
 *
 * Returns: a #BonoboTransient object (the @transient)
 */
BonoboTransient *
bonobo_transient_construct (BonoboTransient          *transient,
			    PortableServer_POA        poa,
			    BonoboTransientServantNew new_servant,
			    BonoboTransientServantDestroy destroy_servant,
			    gpointer data)
{
	CORBA_PolicyList		*policies;
	BonoboTransientServantManager   *sm;
	CORBA_Environment		 ev;
	char				*poa_name;
	gboolean			 success;

	success = FALSE;

	transient->priv->new_servant = new_servant;
	transient->priv->destroy_servant = destroy_servant;
	transient->priv->callback_data = data;

	if (poa == CORBA_OBJECT_NIL)
		poa = bonobo_poa ();
	
	transient->priv->poa = poa;
	
	CORBA_exception_init (&ev);
	
	/*
	 * Create a new custom POA which will manage the
	 * BonoboTransient objects.  We need a custom POA because there
	 * may be many, many properties and we want to avoid
	 * instantiating a servant for each one of them from the
	 * outset (which is what the default POA will require).
	 *
	 * Our new POA will have a special Policy set --
	 * USE_SERVANT_MANAGER -- which means that, when a request
	 * comes in for one of the objects (properties) managed by
	 * this POA, the ORB will dispatch to our special
	 * ServantManager.  The ServantManager will then incarnate the
	 * Servant for the Property which is being operated on.  So we
	 * are creating a ServantManager which will incarnate Property
	 * servants as-needed, and a POA which knows how to dispatch
	 * to our special ServantManager.
	 *
	 * Repetition is probably the only way to get this across, so
	 * allow me to rephrase: When a request comes in for a particular
	 * object, the POA uses the servant manager to get the servant
	 * for the specified object reference.
	 *
	 * This is just on-demand object creation, mired in a bunch of
	 * CORBA jargon.
	 *
	 * The take home message: Each Bonobo Property CORBA object is
	 * not created until someone actually invokes one of its
	 * methods.
	 */
	
	/*
	 * Create a list of CORBA policies which we will apply to our
	 * new POA.
	 */
	policies = g_new0 (CORBA_PolicyList, 1);
	policies->_maximum = 4;
	policies->_length  = 4;
	policies->_buffer  = g_new0 (CORBA_Policy,
				     policies->_length);
	policies->_release = CORBA_FALSE;
	
	/*
	 * Create a new CORBA Policy object which specifies that we
	 * will be using a ServantManager, thank you very much.
	 */
	policies->_buffer [0] = (CORBA_Policy)
		PortableServer_POA_create_request_processing_policy (
			bonobo_poa (),			     /* This argument is ignored. */
			PortableServer_USE_SERVANT_MANAGER,
			&ev);
	
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not create request processing policy for BonoboTransient POA");
		CORBA_exception_free (&ev);
		goto out;
	}
	
	/*
	 * Now, to add a touch more complexity to the whole
	 * system, we go further than just creating Property
	 * servants on-demand; we make them completely transient.
	 * What this means is that, when a Property servant has
	 * finished processing a request on the property object,
	 * it disappears.  So we only use resources on property
	 * servants while a property method invocation is being
	 * processed.  (Now I'm just showing off)
	 *
	 * This is actually important because, with Controls,
	 * properties are used to manipulate many highly-variant
	 * run-time attributes (not just crap like font size).  The
	 * Microsoft ActiveX web controls, for example, use properties
	 * to allow the user (the parent application) to get/set the
	 * current URL being displayed.
	 *
	 * Accordingly, the following CORBA Policy specifies that
	 * servants should not be retained.
	 */
	policies->_buffer [1] = (CORBA_Policy)
		PortableServer_POA_create_servant_retention_policy (
			bonobo_poa (),
			PortableServer_NON_RETAIN,
			&ev);
	
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not create servant retention policy for BonoboTransient POA '%s'",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		goto out;
	}
	
	/*
	 * Set the threading model to SINGLE_THREAD_MODEL, otherwise
	 * an ORB could use the default ORB_CTRL_MODEL, which is to
	 * let the ORB make threads for requests as it likes.
	 */
	policies->_buffer [2] = (CORBA_Policy)
		PortableServer_POA_create_thread_policy (
			bonobo_poa (),
			PortableServer_SINGLE_THREAD_MODEL,
			&ev);
	
	if (BONOBO_EX (&ev)){
		g_warning ("Could not create threading policy for BonoboTransient POA '%s'",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		goto out;
	}

	policies->_buffer [3] = (CORBA_Policy)
		PortableServer_POA_create_implicit_activation_policy (
			bonobo_poa (),
			PortableServer_NO_IMPLICIT_ACTIVATION,
			&ev);

	if (BONOBO_EX (&ev)){
		g_warning ("Could not create activation policy for BonoboTransient POA '%s'",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		goto out;
	}

	/*
	 * Create the BonoboProperty POA as a child of the root
	 * Bonobo POA.
	 */
	poa_name = g_strdup_printf ("BonoboTransient %p", transient);
	transient->priv->poa = PortableServer_POA_create_POA (
		bonobo_poa (), poa_name, bonobo_poa_manager (),
		policies, &ev);
	g_free (poa_name);

	if (BONOBO_EX (&ev)) {
		g_warning ("BonoboTransient: Could not create BonoboTransient POA '%s'",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		goto out;
	}
	
	/*
	 * Create our ServantManager.
	 */
	sm = g_new0 (BonoboTransientServantManager, 1);
	sm->bonobo_transient = transient;

	((POA_PortableServer_ServantLocator *) sm)->vepv = bonobo_transient_get_servant_locator_vepv ();
		
	POA_PortableServer_ServantLocator__init (((PortableServer_ServantLocator *) sm), &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("BonoboTransient: Could not initialize ServantLocator");
		CORBA_exception_free (&ev);
		g_free (sm);
		goto out;
	}

	PortableServer_POA_set_servant_manager (
		transient->priv->poa,
		(PortableServer_ServantManager) sm, &ev);

	if (BONOBO_EX (&ev)) {
		g_warning ("BonoboTransient: Could not set POA servant manager");
		CORBA_exception_free (&ev);
		g_free (sm);
		goto out;
	}

	success = TRUE;

 out:
	if (policies->_buffer [0] != NULL) {
		CORBA_Policy_destroy (policies->_buffer [0], &ev);

		if (BONOBO_EX (&ev)) {
			g_warning ("bonobo_transient_construct(): could not destroy the "
				   "request processing policy");
			CORBA_exception_free (&ev);
			success = FALSE;
		}
	}

	if (policies->_buffer [1] != NULL) {
		CORBA_Policy_destroy (policies->_buffer [1], &ev);

		if (BONOBO_EX (&ev)) {
			g_warning ("bonobo_transient_construct(): could not destroy the "
				   "servant retention policy");
			CORBA_exception_free (&ev);
			success = FALSE;
		}
	}

	if (policies->_buffer [2] != NULL) {
		CORBA_Policy_destroy (policies->_buffer [2], &ev);

		if (BONOBO_EX (&ev)) {
			g_warning ("bonobo_transient_construct(): could not destroy the "
				   "threading policy");
			CORBA_exception_free (&ev);
			success = FALSE;
		}
	}

	if (policies->_buffer [3] != NULL) {
		CORBA_Policy_destroy (policies->_buffer [3], &ev);

		if (BONOBO_EX (&ev)) {
			g_warning ("bonobo_transient_construct(): could not destroy the "
				   "activation policy");
			CORBA_exception_free (&ev);
			success = FALSE;
		}
	}

	g_free (policies->_buffer);
	g_free (policies);

	if (success)
		return transient;
	else
		return NULL;
}

static void
bonobo_transient_destroy (GtkObject *object)
{
	BonoboTransient *transient = BONOBO_TRANSIENT (object);
	
	if (transient->priv->poa) {
		CORBA_Environment ev;

		/* Destroy the POA. */
		CORBA_exception_init (&ev);
		PortableServer_POA_destroy (transient->priv->poa, FALSE, TRUE, &ev);

		if (BONOBO_EX (&ev))
			g_warning ("bonobo_transient_destroy: Could not destroy POA.");

		CORBA_exception_free (&ev);
	} else
		g_warning ("No poa to destroy");

	g_free (transient->priv);
	
	parent_class->destroy (object);
}

static void
bonobo_transient_class_init (BonoboTransientClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->destroy = bonobo_transient_destroy;
}

static void
bonobo_transient_init (BonoboTransient *transient)
{
	transient->priv = g_new0 (BonoboTransientPriv, 1);
}

/**
 * bonobo_transient_get_type:
 *
 * Returns: The GtkType corresponding to the BonoboTransient class.
 */
GtkType
bonobo_transient_get_type (void)
{
	static GtkType type = 0;

	if (! type) {
		GtkTypeInfo info = {
			"BonoboTransient",
			sizeof (BonoboTransient),
			sizeof (BonoboTransientClass),
			(GtkClassInitFunc) bonobo_transient_class_init,
			(GtkObjectInitFunc) bonobo_transient_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_transient_new:
 * @parent_poa: the POA where the object is created, CORBA_OBJECT_NIL for the default Bonobo POA.
 * @new_servant: A function pointer used to incarnate servants on demand.
 * @destroy_servant: A function pointer used to destroy the on-demand server.
 * @data: data passed to the @new_servant and @destroy_servant functions.
 *
 * bonobo_transient_new() creates a new CORBA server that creates a
 * new POA entry (within the @parent_poa POA).  You can construct
 * arbitrary object names inside this space using
 * bonobo_transient_create_objref() and return those to
 * client code.
 *
 * The @new_servant function will be invoked by the POA to resolve
 * object reference to names you have created using
 * bonobo_transient_create_objref().  The @new_servant function
 * should return a PortableServer_Servant that will handle the request.
 *
 * Once the processing is completed, the @destroy_servant will be invoked
 * to release any resources allocated during the invocation of @new_servant
 * or during the execution of the servant that need to be released. 
 *
 * Returns: a new BonoboTransient object.
 */
BonoboTransient *
bonobo_transient_new (PortableServer_POA poa,
                      BonoboTransientServantNew new_servant,
		      BonoboTransientServantDestroy destroy_servant,
		      gpointer data)
{
	BonoboTransient *transient;

	transient = gtk_type_new (BONOBO_TRANSIENT_TYPE);
	if (bonobo_transient_construct (transient, poa, new_servant, destroy_servant, data) == NULL) {
		gtk_object_destroy (GTK_OBJECT (transient));
		return NULL;
	}

	return transient;
}

/**
 * bonobo_transient_create_objref:
 * @transient: The BonoboTransient manager where the object reference is rooted.
 * @iface_name: The CORBA interface name that the returned object is supposed to implement.
 * @name: The name of the object inside the @transient POA's name space.
 * @ev: returns possible errors from the PortableServer_POA_create_reference_with_id() call.
 *
 * Returns: a CORBA_Object reference for an object named @name inside
 * the @transient's POA naming space that implements the @iface_name interface
 *
 * The return value can be CORBA_OBJECT_NIL to indicate an error in the
 * incoming arguments.
 */
CORBA_Object
bonobo_transient_create_objref (BonoboTransient   *transient,
				const char        *iface_name,
				const char        *name,
				CORBA_Environment *ev)
{
	PortableServer_ObjectId *oid;

	g_return_val_if_fail (transient != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (BONOBO_IS_TRANSIENT (transient), CORBA_OBJECT_NIL);
	g_return_val_if_fail (name != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (ev != NULL, CORBA_OBJECT_NIL);

	oid = PortableServer_string_to_ObjectId ((char *) name, ev);
	if (oid == NULL)
		return CORBA_OBJECT_NIL;

	return (CORBA_Object) PortableServer_POA_create_reference_with_id (
		transient->priv->poa, oid, (char *) iface_name, ev);
}

