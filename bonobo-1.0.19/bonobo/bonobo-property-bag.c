/*
 * bonobo-property-bag.c: property bag object implementation.
 *
 * Authors:
 *   Nat Friedman  (nat@helixcode.com)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-property.h>
#include <bonobo/bonobo-persist-stream.h>
#include <bonobo/bonobo-transient.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *parent_class = NULL;
         

/*
 * Internal data structures.
 */
struct _BonoboPropertyBagPrivate {
	GHashTable         *props;

	BonoboPropertySetFn set_prop;
	BonoboPropertyGetFn get_prop;
	gpointer            user_data;

	BonoboTransient    *transient;
};


/* BonoboPropertyBag CORBA methods.*/
static void
bonobo_property_bag_foreach_create_list (gpointer key, 
					 gpointer value,
					 gpointer data)
{
	GList **l = (GList **) data;

	*l = g_list_prepend (*l, value);
}


/**
 * bonobo_property_bag_get_prop_list:
 * @pb: A #BonoboPropertyBag.
 *
 * Returns a #GList of #BonoboProperty structures.  This function is
 * private and should only be used internally, or in a PropertyBag
 * persistence implementation.  You should not touch the
 * #BonoboProperty structure unless you know what you're doing.
 */
GList *
bonobo_property_bag_get_prop_list (BonoboPropertyBag *pb)
{
	GList *l;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL);

	l = NULL;

	g_hash_table_foreach (pb->priv->props,
			      bonobo_property_bag_foreach_create_list,
			      &l);

	return l;
}

static Bonobo_EventSource
impl_Bonobo_PropertyBag_getEventSource (PortableServer_Servant servant,
					CORBA_Environment      *ev)
{
	BonoboPropertyBag   *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));

	return bonobo_object_dup_ref (BONOBO_OBJREF (pb->es), ev);
}

static Bonobo_PropertyList *
impl_Bonobo_PropertyBag_getProperties (PortableServer_Servant  servant,
				       CORBA_Environment      *ev)
{
	BonoboPropertyBag   *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	Bonobo_PropertyList *prop_list;
	GList		   *props;
	GList		   *curr;
	int		    len;
	int		    i;

	/*
	 * Create the PropertyList and allocate space for the
	 * properties.
	 */
	len = g_hash_table_size (pb->priv->props);

	prop_list = Bonobo_PropertyList__alloc ();
	prop_list->_length = len;

	if (len == 0)
		return prop_list;

	prop_list->_buffer = CORBA_sequence_Bonobo_Property_allocbuf (len);

	/*
	 * Create a list of Object references for the properties.
	 */
	props = bonobo_property_bag_get_prop_list (pb);

	i = 0;
	for (curr = props; curr != NULL; curr = curr->next) {
		BonoboProperty *prop = curr->data;

		prop_list->_buffer [i] =  bonobo_transient_create_objref (
			pb->priv->transient, "IDL:Bonobo/Property:1.0",
			prop->name, ev);

		if (BONOBO_EX (ev)) {
			g_warning ("BonoboPropertyBag: Could not create property objref!");
			g_list_free (props);
			CORBA_free (prop_list);
			return CORBA_OBJECT_NIL;
		}

		i++;
		
	}

	g_list_free (props);

	return prop_list;
}

static Bonobo_Property
impl_Bonobo_PropertyBag_getPropertyByName (PortableServer_Servant servant,
					   const CORBA_char      *name,
					   CORBA_Environment     *ev)
{
	BonoboPropertyBag *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	Bonobo_Property    prop;

	if (g_hash_table_lookup (pb->priv->props, name) == NULL) {

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PropertyBag_NotFound,
				     NULL);

		return CORBA_OBJECT_NIL;
	}

	prop = bonobo_transient_create_objref (pb->priv->transient, 
					       "IDL:Bonobo/Property:1.0", 
					        name, ev);

	return prop;
}

static Bonobo_PropertyNames *
impl_Bonobo_PropertyBag_getPropertyNames (PortableServer_Servant servant,
					  CORBA_Environment     *ev)
{
	BonoboPropertyBag        *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	Bonobo_PropertyNames	*name_list;
	GList			*props;
	GList			*curr;
	int                      len;
	int			 i;

	/*
	 * Create the PropertyNames list and allocate space for the
	 * names.
	 */
	len = g_hash_table_size (pb->priv->props);

	name_list = Bonobo_PropertyNames__alloc ();
	name_list->_length = len;

	if (len == 0)
		return name_list;

	name_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);

	/*
	 * Create the list of property names.
	 */
	props = bonobo_property_bag_get_prop_list (pb);

	i = 0;
	for (curr = props; curr != NULL; curr = curr->next) {
		BonoboProperty *prop = curr->data;

		name_list->_buffer [i] = CORBA_string_dup (prop->name);
		i ++;
	}

	g_list_free (props);

	return name_list;
}

static Bonobo_PropertySet *
impl_Bonobo_PropertyBag_getValues (PortableServer_Servant  servant,
				   CORBA_Environment      *ev)
{
	BonoboPropertyBag   *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	Bonobo_PropertySet *set;
	GList		   *props;
	GList		   *curr;
	int		    len;
	int		    i;

	/*
	 * Create the PropertyList and allocate space for the
	 * properties.
	 */
	len = g_hash_table_size (pb->priv->props);

	set = Bonobo_PropertySet__alloc ();
	set->_length = len;

	if (len == 0)
		return set;

	set->_buffer = CORBA_sequence_Bonobo_Pair_allocbuf (len);
	CORBA_sequence_set_release (set, TRUE);

	/*
	 * Create a list of Object references for the properties.
	 */
	props = bonobo_property_bag_get_prop_list (pb);

	i = 0;
	for (curr = props; curr != NULL; curr = curr->next) {
		BonoboProperty *prop = curr->data;
		BonoboArg *arg;

		set->_buffer [i].name =  CORBA_string_dup (prop->name);

		arg = bonobo_arg_new (prop->type);
	
		prop->get_prop (pb, arg, prop->idx, ev, 
				prop->user_data);

		set->_buffer [i].value = *arg;

		i++;		
	}

	g_list_free (props);

	return set;
}

static void
impl_Bonobo_PropertyBag_setValues (PortableServer_Servant    servant,
				   const Bonobo_PropertySet *set,
				   CORBA_Environment        *ev)
{
	BonoboPropertyBag   *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	int i;

	for (i = 0; i < set->_length; i++) {

		bonobo_property_bag_set_value (pb, set->_buffer [i].name,
					       &set->_buffer [i].value, ev);
	}
}


/*
 * BonoboPropertyBag construction/deconstruction functions. 
 */

/**
 * bonobo_property_bag_construct:
 * @pb: #BonoboPropertyBag to construct
 * @get_prop: the property get callback
 * @set_prop: the property set callback
 * @es: an event source to aggregate
 * @user_data: user data for the callbacks
 * 
 * Constructor, only for use in wrappers and object derivation, please
 * refer to the #bonobo_property_bag_new for normal use.
 *
 * This function returns @pb, or %NULL in case of error.  If it returns %NULL,
 * the passed in @pb is unrefed.
 *
 * Returns:  #BonoboPropertyBag pointer or %NULL.
 */
BonoboPropertyBag *
bonobo_property_bag_construct (BonoboPropertyBag   *pb,
			       BonoboPropertyGetFn  get_prop,
			       BonoboPropertySetFn  set_prop,
			       BonoboEventSource   *es,
			       gpointer             user_data)
{
	pb->es              = es;
	pb->priv->set_prop  = set_prop;
	pb->priv->get_prop  = get_prop;
	pb->priv->user_data = user_data;

	bonobo_object_add_interface (BONOBO_OBJECT (pb), BONOBO_OBJECT (es));

	if (!(pb->priv->transient = bonobo_transient_new (NULL, bonobo_property_servant_new, bonobo_property_servant_destroy, pb))) {
		bonobo_object_unref (BONOBO_OBJECT (pb));
		return NULL;
	}
	
	return pb;
}

/**
 * bonobo_property_bag_new_full:
 * @get_prop: the property get callback
 * @set_prop: the property set callback
 * @es: an event source to aggregate
 * @user_data: user data for the callbacks
 *
 * Creates a new property bag with the specified callbacks.
 *
 * Returns: A new #BonoboPropertyBag object.
 */
BonoboPropertyBag *
bonobo_property_bag_new_full (BonoboPropertyGetFn  get_prop,
			      BonoboPropertySetFn  set_prop,
			      BonoboEventSource   *es,
			      gpointer             user_data)
{
	BonoboPropertyBag *pb;

	g_return_val_if_fail (es != NULL, NULL);

	pb = gtk_type_new (BONOBO_PROPERTY_BAG_TYPE);

	return bonobo_property_bag_construct (pb, get_prop, set_prop, es, 
					      user_data);
}

/**
 * bonobo_property_bag_new:
 * @get_prop: the property get callback
 * @set_prop: the property set callback
 * @user_data: user data for the callbacks
 *
 * Creates a new property bag with the specified callbacks.
 *
 * Returns: A new #BonoboPropertyBag object.
 */
BonoboPropertyBag *
bonobo_property_bag_new (BonoboPropertyGetFn get_prop,
			 BonoboPropertySetFn set_prop,
			 gpointer            user_data)
{
	BonoboEventSource *es;

	es = bonobo_event_source_new ();

	return bonobo_property_bag_new_full (get_prop, set_prop, es, 
					     user_data);
}

static void
bonobo_property_destroy (BonoboProperty *prop)
{
	g_free (prop->name);
	prop->idx = -1;

	bonobo_arg_release (prop->default_value);

	g_free (prop->docstring);

	g_free (prop);
}

static gboolean
bonobo_property_bag_foreach_remove_prop (gpointer key, 
					 gpointer value,
					 gpointer user_data)
{
	bonobo_property_destroy (value);

	return TRUE;
}

static void
bonobo_property_bag_destroy (GtkObject *object)
{
	BonoboPropertyBag *pb = BONOBO_PROPERTY_BAG (object);
	
	/* Destroy the transient POA */
	gtk_object_unref (GTK_OBJECT (pb->priv->transient));

	/* Destroy all properties. */
	g_hash_table_foreach_remove (pb->priv->props,
				     bonobo_property_bag_foreach_remove_prop,
				     NULL);
	g_hash_table_destroy (pb->priv->props);

	g_free (pb->priv);

	parent_class->destroy (object);
}


/*
 * BonoboPropertyBag property manipulation API.
 */

/**
 * bonobo_property_bag_add_full:
 * @pb: property bag to add to
 * @name: name of new property
 * @idx: integer index for fast callback switch statement - NB.
 * this value is opaque to the implementation, and is not used
 * for keying properties.
 * @type: the CORBA type eg. TC_long
 * @default_value: the default value or NULL
 * @docstring: the translated documentation string
 * @flags: various flags
 * @get_prop: a per property get callback
 * @set_prop: a per property set callback
 * @user_data: user data for the callbacks
 * 
 * This adds a property to @pb at the full tilt of complexity.
 **/
void
bonobo_property_bag_add_full (BonoboPropertyBag  *pb,
			      const char         *name,
			      int                 idx,
			      BonoboArgType       type,
			      BonoboArg          *default_value,
			      const char         *docstring,
			      BonoboPropertyFlags flags,
			      BonoboPropertyGetFn get_prop,
			      BonoboPropertySetFn set_prop,
			      gpointer            user_data)
{
	BonoboProperty *prop;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG (pb));
	g_return_if_fail (name != NULL);
	g_return_if_fail (type != NULL);
	g_return_if_fail (g_hash_table_lookup (pb->priv->props, name) == NULL);

	if (flags == 0) { /* Compatibility hack */
		flags = BONOBO_PROPERTY_READABLE |
			BONOBO_PROPERTY_WRITEABLE;
	}
			    
	if (((flags & BONOBO_PROPERTY_READABLE)  && !get_prop) ||
	    ((flags & BONOBO_PROPERTY_WRITEABLE) && !set_prop)) {
		g_warning ("Serious property error, missing get/set fn. "
			   "on %s", name);
		return;
	}

	if (!(flags & BONOBO_PROPERTY_READABLE) && default_value)
		g_warning ("Assigning a default value to a non readable "
			   "property '%s'", name);

	prop = g_new0 (BonoboProperty, 1);

	prop->name          = g_strdup (name);
	prop->idx           = idx;
	prop->type          = type;
	prop->docstring     = g_strdup (docstring);
	prop->flags         = flags;
	prop->get_prop      = get_prop;
	prop->set_prop      = set_prop;
	prop->user_data     = user_data;

	if (default_value)
		prop->default_value = bonobo_arg_copy (default_value);

	g_hash_table_insert (pb->priv->props, prop->name, prop);
}

static BonoboPropertyFlags
flags_gtk_to_bonobo (guint flags)
{
	BonoboPropertyFlags f = 0;

	if (!flags & GTK_ARG_READABLE)
		f |= BONOBO_PROPERTY_READABLE;

	if (!flags & GTK_ARG_WRITABLE)
		f |= BONOBO_PROPERTY_WRITEABLE;

	return f;
}

#define BONOBO_GTK_MAP_KEY "BonoboGtkMapKey"

static void
get_prop (BonoboPropertyBag *bag,
	  BonoboArg         *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	GtkArg *gtk_arg = user_data;
	GtkArg  new;
	GtkObject *obj;

	if (!(obj = gtk_object_get_data (GTK_OBJECT (bag), 
					 BONOBO_GTK_MAP_KEY))) {
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		return;
	}
	
/*	g_warning ("Get prop ... %d: %s", arg_id, gtk_arg->name);*/

	new.type = gtk_arg->type;
	new.name = gtk_arg->name;
	gtk_object_getv (obj, 1, &new);

	bonobo_arg_from_gtk (arg, &new);

	if (new.type == GTK_TYPE_STRING &&
	    GTK_VALUE_STRING (new))
		g_free (GTK_VALUE_STRING (new));
}

static void
set_prop (BonoboPropertyBag *bag,
	  const BonoboArg   *arg,
	  guint              arg_id,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	GtkArg *gtk_arg = user_data;
	GtkArg  new;
	GtkObject *obj;

	if (!(obj = gtk_object_get_data (GTK_OBJECT (bag),
					 BONOBO_GTK_MAP_KEY))) {
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		return;
	}
		
/*	g_warning ("Set prop ... %d: %s", arg_id, gtk_arg->name);*/

	new.type = gtk_arg->type;
	new.name = gtk_arg->name;
	bonobo_arg_to_gtk (&new, arg);

	gtk_object_setv (obj, 1, &new);
}

/**
 * bonobo_property_bag_add_gtk_args:
 * @pb: destination property bag
 * @object: a generic Gtk Object
 * 
 * Transfers GtkArgs from the object to the property bag,
 * and maps between the two objects property systems.
 **/
void
bonobo_property_bag_add_gtk_args (BonoboPropertyBag  *pb,
				  GtkObject          *object)
{
	GtkArg  *args, *arg;
	guint32 *arg_flags;
	guint    nargs = 0;
	int      i;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG (pb));
	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_OBJECT (object));

	if (gtk_object_get_data (GTK_OBJECT (pb),
				 BONOBO_GTK_MAP_KEY)) {
		g_warning ("Cannot proxy two gtk objects in the same bag yet");
		return;
	}

	gtk_object_set_data (GTK_OBJECT (pb),
			     BONOBO_GTK_MAP_KEY, object);
	/*
	 * FIXME: we should do this on a per class basis perhaps.
	 */
	args = gtk_object_query_args (GTK_OBJECT_TYPE (object),
				      &arg_flags, &nargs);
	
	if (!nargs) {
		g_warning ("Strange, no Gtk arguments to map to Bonobo");
		return;
	}

	arg = args;
	/* Setup types, and names */
	for (i = 0; i < nargs; arg++, i++) {
		BonoboPropertyFlags flags;
		BonoboArgType       type;
		char               *desc;

		type = bonobo_arg_type_from_gtk (arg->type);
		if (!type) {
			g_warning ("Can't handle type '%s' on arg '%s'",
				   gtk_type_name (arg->type),
				   arg->name);
			continue;
		}

		flags = flags_gtk_to_bonobo (arg_flags [i]);

		desc = g_strconcat (arg->name, " is a ",
				    gtk_type_name (arg->type), NULL);

		g_warning ("Mapping '%s'", desc);
		bonobo_property_bag_add_full (pb, arg->name, i, type,
					      NULL, desc, flags,
					      get_prop, set_prop, arg);
		g_free (desc);
	}

/* FIXME: leaks like a privatised water company */
/*	g_free (args);*/
	g_free (arg_flags);
}

/**
 * bonobo_property_bag_add:
 * @pb: property bag to add to
 * @name: name of new property
 * @idx: integer index for fast callback switch statement
 * this value is opaque to the implementation, and is not used
 * for keying properties.
 * @type: the CORBA type eg. TC_long
 * @default_value: the default value or NULL
 * @docstring: the translated documentation string
 * @flags: various flags
 * 
 *  Adds a property to the property bag.
 **/
void
bonobo_property_bag_add (BonoboPropertyBag  *pb,
			 const char         *name,
			 int                 idx,
			 BonoboArgType       type,
			 BonoboArg          *default_value,
			 const char         *docstring,
			 BonoboPropertyFlags flags)
{
	g_return_if_fail (pb != NULL);

	return bonobo_property_bag_add_full (pb, name, idx, type,
					     default_value, docstring, flags,
					     pb->priv->get_prop,
					     pb->priv->set_prop,
					     pb->priv->user_data);
}

static void
notify_listeners (BonoboPropertyBag *pb,
		  BonoboProperty    *prop,
		  const BonoboArg   *new_value,
		  CORBA_Environment *opt_ev)
{
	if (prop->flags & BONOBO_PROPERTY_NO_LISTENING)
		return;
	
	bonobo_event_source_notify_listeners_full (pb->es,
						   "Bonobo/Property",
						   "change", prop->name,
						   new_value, opt_ev);
}

/**
 * bonobo_property_bag_notify_listeners:
 * @pb: the property bag
 * @name: the name of the property that changed value
 * @new_value: the new value
 * @opt_ev: optional CORBA exception environment or NULL
 * 
 * This function is used by the implementation of the property
 * proper, to signal to the property bag that the value of the
 * property has changed.
 * NB. There is no need to call this when you do a set_value.
 **/
void
bonobo_property_bag_notify_listeners (BonoboPropertyBag *pb,
				      const char        *name,
				      const BonoboArg   *new_value,
				      CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;

	bonobo_return_if_fail (pb != NULL, opt_ev);
	bonobo_return_if_fail (BONOBO_IS_PROPERTY_BAG (pb), opt_ev);
	bonobo_return_if_fail (name != NULL, opt_ev);
	bonobo_return_if_fail (pb->priv != NULL, opt_ev);
 	bonobo_return_if_fail (new_value != NULL, opt_ev);

	if (!(prop = g_hash_table_lookup (pb->priv->props, name))) {
		bonobo_exception_set (opt_ev, ex_Bonobo_PropertyBag_NotFound);
		return;
	}

	if (!bonobo_arg_type_is_equal (prop->type, new_value->_type, opt_ev)) {
		bonobo_exception_set (opt_ev, ex_Bonobo_Property_InvalidValue);
		return;
	}

	notify_listeners (pb, prop, new_value, opt_ev);
}

/**
 * bonobo_property_bag_set_value:
 * @pb: the property bag
 * @name: the name of the property
 * @value: the new value to set to 
 * @opt_ev: optional CORBA exception environment or NULL
 * 
 * This method sets the value of the property with @name
 * to @value.
 **/
void
bonobo_property_bag_set_value (BonoboPropertyBag *pb,
			       const char        *name,
			       const BonoboArg   *value,
			       CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;
	CORBA_Environment ev, *my_ev;

	bonobo_return_if_fail (pb != NULL, opt_ev);
	bonobo_return_if_fail (BONOBO_IS_PROPERTY_BAG (pb), opt_ev);
	bonobo_return_if_fail (name != NULL, opt_ev);
	bonobo_return_if_fail (pb->priv != NULL, opt_ev);
	bonobo_return_if_fail (value != NULL, opt_ev);
	
	prop = g_hash_table_lookup (pb->priv->props, name);

	if (!prop || !prop->set_prop) {
		bonobo_exception_set (opt_ev, ex_Bonobo_PropertyBag_NotFound);
		return;
	}

	if (!(prop->flags & BONOBO_PROPERTY_WRITEABLE)) {
		bonobo_exception_set (opt_ev, ex_Bonobo_Property_ReadOnlyProperty);
		return;
	}

	if (!bonobo_arg_type_is_equal (prop->type, value->_type, opt_ev)) {
		bonobo_exception_set (opt_ev, ex_Bonobo_Property_InvalidValue);
		return;
	}

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	prop->set_prop (pb, value, prop->idx, my_ev, prop->user_data);

	if (!BONOBO_EX (my_ev))
		notify_listeners (pb, prop, value, my_ev);

	if (!opt_ev)
		CORBA_exception_free (&ev);
}

/**
 * bonobo_property_bag_get_value:
 * @pb: the property bag
 * @name: the name of the property
 * @opt_ev: optional CORBA exception environment or NULL
 * 
 * Return value: the value of the property with name @name or NULL
 * on exception.
 **/
BonoboArg *
bonobo_property_bag_get_value (BonoboPropertyBag *pb, 
			       const char        *name,
			       CORBA_Environment *opt_ev)
{
	BonoboProperty    *prop;
	BonoboArg         *arg;
	CORBA_Environment  ev, *my_ev;

	bonobo_return_val_if_fail (pb != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL, opt_ev);
	bonobo_return_val_if_fail (name != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (pb->priv != NULL, NULL, opt_ev);

	prop = g_hash_table_lookup (pb->priv->props, name);

	if (!prop || !prop->get_prop) {
		bonobo_exception_set (opt_ev, ex_Bonobo_PropertyBag_NotFound);
		return NULL;
	}

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	arg = bonobo_arg_new (prop->type);

	prop->get_prop (pb, arg, prop->idx, my_ev, prop->user_data);

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return arg;
}

/**
 * bonobo_property_bag_get_property_type:
 * @pb: the property bag
 * @name: the name of the property
 * @opt_ev: optional CORBA exception environment or NULL
 * 
 * Return value: the type of the property with name @name
 **/
BonoboArgType
bonobo_property_bag_get_property_type (BonoboPropertyBag *pb, 
				       const char        *name,
				       CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;

	bonobo_return_val_if_fail (pb != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL, opt_ev);
	bonobo_return_val_if_fail (name != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (pb->priv != NULL, NULL, opt_ev);

	if (!(prop = g_hash_table_lookup (pb->priv->props, name))) {
		bonobo_exception_set (opt_ev, ex_Bonobo_PropertyBag_NotFound);
		return NULL;
	}

	return prop->type;
}

/**
 * bonobo_property_bag_get_default:
 * @pb: the property bag
 * @name: the name of the property
 * @opt_ev: optional CORBA exception environment or NULL
 * 
 * Return value: the default value of the property with name @name
 **/
BonoboArg *
bonobo_property_bag_get_default (BonoboPropertyBag *pb, 
				 const char        *name,
				 CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;

	bonobo_return_val_if_fail (pb != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL, opt_ev);
	bonobo_return_val_if_fail (name != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (pb->priv != NULL, NULL, opt_ev);

	if (!(prop = g_hash_table_lookup (pb->priv->props, name))) {
		bonobo_exception_set (opt_ev, ex_Bonobo_PropertyBag_NotFound);
		return NULL;
	}

	if (prop->default_value)
		return bonobo_arg_copy (prop->default_value);
	else {
		BonoboArg *a = bonobo_arg_new (prop->type);
		return a;
	}
}

/**
 * bonobo_property_bag_get_default:
 * @pb: the property bag
 * @name: the name of the property
 * 
 * Return value: TRUE if the bag has a property of this name
 **/
gboolean
bonobo_property_bag_has_property (BonoboPropertyBag *pb, 
				  const char        *name)
{
	g_return_val_if_fail (pb != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (pb->priv != NULL, FALSE);

	if (g_hash_table_lookup (pb->priv->props, name) == NULL)
		return FALSE;

	return TRUE;
}

/**
 * bonobo_property_bag_get_docstring:
 * @pb: the property bag
 * @name: the name of the property
 * @opt_ev: optional CORBA exception environment or NULL
 * 
 * Return value: the documentation string for this property.
 **/
const char *
bonobo_property_bag_get_docstring (BonoboPropertyBag *pb, 
				   const char        *name,
				   CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;

	bonobo_return_val_if_fail (pb != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL, opt_ev);
	bonobo_return_val_if_fail (name != NULL, NULL, opt_ev);
	bonobo_return_val_if_fail (pb->priv != NULL, NULL, opt_ev);

	if (!(prop = g_hash_table_lookup (pb->priv->props, name))) {
		bonobo_exception_set (opt_ev, ex_Bonobo_PropertyBag_NotFound);
		return NULL;
	}

	return prop->docstring;
}

/**
 * bonobo_property_bag_get_flags:
 */
const BonoboPropertyFlags
bonobo_property_bag_get_flags (BonoboPropertyBag *pb, 
			       const char        *name,
			       CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;

	bonobo_return_val_if_fail (pb != NULL, 0, opt_ev);
	bonobo_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), 0, opt_ev);
	bonobo_return_val_if_fail (name != NULL, 0, opt_ev);
	bonobo_return_val_if_fail (pb->priv != NULL, 0, opt_ev);

	if (!(prop = g_hash_table_lookup (pb->priv->props, name))) {
		bonobo_exception_set (opt_ev, ex_Bonobo_PropertyBag_NotFound);
		return 0;
	}

	return prop->flags;
}


/* Class/object initialization functions. */

static void
bonobo_property_bag_class_init (BonoboPropertyBagClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_PropertyBag__epv *epv = &klass->epv;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_property_bag_destroy;

	epv->getProperties        = impl_Bonobo_PropertyBag_getProperties;
	epv->getPropertyByName    = impl_Bonobo_PropertyBag_getPropertyByName;
	epv->getPropertyNames     = impl_Bonobo_PropertyBag_getPropertyNames;
	epv->getEventSource       = impl_Bonobo_PropertyBag_getEventSource;
	epv->getValues            = impl_Bonobo_PropertyBag_getValues;
	epv->setValues            = impl_Bonobo_PropertyBag_setValues;
}

static void
bonobo_property_bag_init (BonoboPropertyBag *pb)
{
	pb->priv = g_new0 (BonoboPropertyBagPrivate, 1);

	pb->priv->props = g_hash_table_new (g_str_hash, g_str_equal);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboPropertyBag, 
			 Bonobo_PropertyBag,
			 PARENT_TYPE,
			 bonobo_property_bag);
