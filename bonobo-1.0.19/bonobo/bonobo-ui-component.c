/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-component-ui.c: Client UI signal multiplexer and verb repository.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <gnome.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-util.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *bonobo_ui_component_parent_class;

enum {
	EXEC_VERB,
	UI_EVENT,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

#define GET_CLASS(c) (BONOBO_UI_COMPONENT_CLASS (GTK_OBJECT (c)->klass))

typedef struct {
	char              *id;
	BonoboUIListenerFn cb;
	gpointer           user_data;
	GDestroyNotify     destroy_fn;
} UIListener;

typedef struct {
	char          *cname;
	BonoboUIVerbFn cb;
	gpointer       user_data;
	GDestroyNotify destroy_fn;
} UIVerb;

struct _BonoboUIComponentPrivate {
	GHashTable        *verbs;
	GHashTable        *listeners;
	char              *name;
	Bonobo_UIContainer container;
};

static inline BonoboUIComponent *
bonobo_ui_from_servant (PortableServer_Servant servant)
{
	return BONOBO_UI_COMPONENT (bonobo_object_from_servant (servant));
}

static gboolean
verb_destroy (gpointer dummy, UIVerb *verb, gpointer dummy2)
{
	if (verb) {
		if (verb->destroy_fn)
			verb->destroy_fn (verb->user_data);
		verb->destroy_fn = NULL;
		g_free (verb->cname);
		g_free (verb);
	}
	return TRUE;
}

static gboolean
listener_destroy (gpointer dummy, UIListener *l, gpointer dummy2)
{
	if (l) {
		if (l->destroy_fn)
			l->destroy_fn (l->user_data);
		l->destroy_fn = NULL;
		g_free (l->id);
		g_free (l);
	}
	return TRUE;
}

static void
ui_event (BonoboUIComponent           *component,
	  const char                  *id,
	  Bonobo_UIComponent_EventType type,
	  const char                  *state)
{
	UIListener *list;

	g_return_if_fail (component != NULL);
	g_return_if_fail (component->priv != NULL);

	list = g_hash_table_lookup (component->priv->listeners, id);
	if (list && list->cb)
		list->cb (component, id, type,
			  state, list->user_data);
}

static CORBA_char *
impl_Bonobo_UIComponent_describeVerbs (PortableServer_Servant servant,
				       CORBA_Environment     *ev)
{
	g_warning ("FIXME: Describe verbs unimplemented");
	return CORBA_string_dup ("<NoUIVerbDescriptionCodeYet/>");
}

static void
impl_Bonobo_UIComponent_execVerb (PortableServer_Servant servant,
				  const CORBA_char      *cname,
				  CORBA_Environment     *ev)
{
	BonoboUIComponent *component;
	UIVerb *verb;

	component = bonobo_ui_from_servant (servant);

	g_return_if_fail (component != NULL);
	g_return_if_fail (component->priv != NULL);

	bonobo_object_ref (BONOBO_OBJECT (component));
	
/*	g_warning ("TESTME: Exec verb '%s'", cname);*/

	verb = g_hash_table_lookup (component->priv->verbs, cname);
	if (verb && verb->cb)
		verb->cb (component, verb->user_data, cname);
	else
		g_warning ("FIXME: verb '%s' not found, emit exception", cname);

	gtk_signal_emit (GTK_OBJECT (component),
			 signals [EXEC_VERB],
			 cname);

	bonobo_object_unref (BONOBO_OBJECT (component));
}

static void
impl_Bonobo_UIComponent_uiEvent (PortableServer_Servant             servant,
				 const CORBA_char                  *id,
				 const Bonobo_UIComponent_EventType type,
				 const CORBA_char                  *state,
				 CORBA_Environment                 *ev)
{
	BonoboUIComponent *component;

	component = bonobo_ui_from_servant (servant);

/*	g_warning ("TESTME: Event '%s' '%d' '%s'\n", path, type, state);*/

	bonobo_object_ref (BONOBO_OBJECT (component));

	gtk_signal_emit (GTK_OBJECT (component),
			 signals [UI_EVENT], id, type, state);

	bonobo_object_unref (BONOBO_OBJECT (component));
}


/**
 * bonobo_ui_component_add_verb_full:
 * @component: the component to add it to
 * @cname: the programmatic name of the verb
 * @fn: the callback function for invoking it
 * @user_data: the associated user data for the callback
 * @destroy_fn: a destroy function for the callback data
 * 
 * Add a verb to the UI component, that can be invoked by
 * the container.
 **/
void
bonobo_ui_component_add_verb_full (BonoboUIComponent  *component,
				   const char         *cname,
				   BonoboUIVerbFn      fn,
				   gpointer            user_data,
				   GDestroyNotify      destroy_fn)
{
	UIVerb *verb;
	BonoboUIComponentPrivate *priv;

	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	priv = component->priv;

	if ((verb = g_hash_table_lookup (priv->verbs, cname))) {
		g_hash_table_remove (priv->verbs, cname);
		verb_destroy (NULL, verb, NULL);
	}

	verb = g_new (UIVerb, 1);
	verb->cname      = g_strdup (cname);
	verb->cb         = fn;
	verb->user_data  = user_data;
	verb->destroy_fn = destroy_fn;

	g_hash_table_insert (priv->verbs, verb->cname, verb);
}

/**
 * bonobo_ui_component_add_verb:
 * @component: the component to add it to
 * @cname: the programmatic name of the verb
 * @fn: the callback function for invoking it
 * @user_data: the associated user data for the callback
 *
 * Add a verb to the UI component, that can be invoked by
 * the container.
 **/
void
bonobo_ui_component_add_verb (BonoboUIComponent  *component,
			      const char         *cname,
			      BonoboUIVerbFn      fn,
			      gpointer            user_data)
{
	bonobo_ui_component_add_verb_full (
		component, cname, fn, user_data, NULL);
}

typedef struct {
	gboolean    by_name;
	const char *name;
	gboolean    by_func;
	gpointer    func;
	gboolean    by_data;
	gpointer    user_data;
} RemoveInfo;

static gboolean
remove_verb (gpointer	key,
	     gpointer	value,
	     gpointer	user_data)
{
	RemoveInfo *info = user_data;
	UIVerb     *verb = value;

	if (info->by_name && info->name &&
	    !strcmp (verb->cname, info->name))
		return verb_destroy (NULL, verb, NULL);

	else if (info->by_func &&
		 (BonoboUIVerbFn) info->func == verb->cb)
		return verb_destroy (NULL, verb, NULL);

	else if (info->by_data &&
		 (BonoboUIVerbFn) info->user_data == verb->user_data)
		return verb_destroy (NULL, verb, NULL);

	return FALSE;
}

/**
 * bonobo_ui_component_remove_verb:
 * @component: the component to add it to
 * @cname: the programmatic name of the verb
 * 
 * Remove a verb by it's unique name
 **/
void
bonobo_ui_component_remove_verb (BonoboUIComponent  *component,
				 const char         *cname)
{
	RemoveInfo info;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	memset (&info, 0, sizeof (info));

	info.by_name = TRUE;
	info.name = cname;

	g_hash_table_foreach_remove (component->priv->verbs, remove_verb, &info);
}

/**
 * bonobo_ui_component_remove_verb_by_func:
 * @component: the component to add it to
 * @fn: the function pointer
 * 
 * remove any verb handled by @fn.
 **/
void
bonobo_ui_component_remove_verb_by_func (BonoboUIComponent  *component,
					 BonoboUIVerbFn      fn)
{
	RemoveInfo info;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	memset (&info, 0, sizeof (info));

	info.by_func = TRUE;
	info.func = (gpointer) fn;

	g_hash_table_foreach_remove (component->priv->verbs, remove_verb, &info);
}

/**
 * bonobo_ui_component_remove_verb_by_func:
 * @component: the component to add it to
 * @user_data: the function pointer
 * 
 * remove any verb with associated @user_data pointer
 **/
void
bonobo_ui_component_remove_verb_by_data (BonoboUIComponent  *component,
					 gpointer            user_data)
{
	RemoveInfo info;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	memset (&info, 0, sizeof (info));

	info.by_data = TRUE;
	info.user_data = user_data;

	g_hash_table_foreach_remove (component->priv->verbs, remove_verb, &info);
}

/**
 * bonobo_ui_component_add_listener_full:
 * @component: the component to add it to
 * @id: the programmatic name of the id
 * @fn: the callback function for invoking it
 * @user_data: the associated user data for the callback
 * @destroy_fn: a destroy function for the callback data
 * 
 * Add a listener for stateful events.
 **/
void
bonobo_ui_component_add_listener_full (BonoboUIComponent  *component,
				       const char         *id,
				       BonoboUIListenerFn  fn,
				       gpointer            user_data,
				       GDestroyNotify      destroy_fn)
{
	UIListener *list;
	BonoboUIComponentPrivate *priv;

	g_return_if_fail (fn != NULL);
	g_return_if_fail (id != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	priv = component->priv;

	if ((list = g_hash_table_lookup (priv->listeners, id))) {
		g_hash_table_remove (priv->listeners, id);
		listener_destroy (NULL, list, NULL);
	}

	list = g_new (UIListener, 1);
	list->cb = fn;
	list->id = g_strdup (id);
	list->user_data = user_data;
	list->destroy_fn = destroy_fn;

	g_hash_table_insert (priv->listeners, list->id, list);	
}

/**
 * bonobo_ui_component_add_listener:
 * @component: the component to add it to
 * @id: the programmatic name of the id
 * @fn: the callback function for invoking it
 * @user_data: the associated user data for the callback
 * 
 * Add a listener for stateful events.
 **/
void
bonobo_ui_component_add_listener (BonoboUIComponent  *component,
				  const char         *id,
				  BonoboUIListenerFn  fn,
				  gpointer            user_data)
{
	bonobo_ui_component_add_listener_full (
		component, id, fn, user_data, NULL);
}

static gboolean
remove_listener (gpointer	key,
		 gpointer	value,
		 gpointer	user_data)
{
	RemoveInfo *info = user_data;
	UIListener     *listener = value;

	if (info->by_name && info->name &&
	    !strcmp (listener->id, info->name))
		return listener_destroy (NULL, listener, NULL);

	else if (info->by_func &&
		 (BonoboUIListenerFn) info->func == listener->cb)
		return listener_destroy (NULL, listener, NULL);

	else if (info->by_data &&
		 (BonoboUIListenerFn) info->user_data == listener->user_data)
		return listener_destroy (NULL, listener, NULL);

	return FALSE;
}

/**
 * bonobo_ui_component_remove_listener:
 * @component: the component to add it to
 * @cname: the programmatic name of the id
 * 
 * Remove any listener by its unique id
 **/
void
bonobo_ui_component_remove_listener (BonoboUIComponent  *component,
				     const char         *cname)
{
	RemoveInfo info;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	memset (&info, 0, sizeof (info));

	info.by_name = TRUE;
	info.name = cname;

	g_hash_table_foreach_remove (component->priv->listeners, remove_listener, &info);
}

/**
 * bonobo_ui_component_remove_by_func:
 * @component: the component to add it to
 * @fn: the function pointer
 * 
 * Remove any listener with associated function @fn
 **/
void
bonobo_ui_component_remove_listener_by_func (BonoboUIComponent  *component,
					     BonoboUIListenerFn      fn)
{
	RemoveInfo info;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	memset (&info, 0, sizeof (info));

	info.by_func = TRUE;
	info.func = (gpointer) fn;

	g_hash_table_foreach_remove (component->priv->listeners, remove_listener, &info);
}

/**
 * bonobo_ui_component_remove_by_data:
 * @component: the component to add it to
 * @user_data: the user_data pointer
 * 
 * Remove any listener with associated user_data @user_data
 **/
void
bonobo_ui_component_remove_listener_by_data (BonoboUIComponent  *component,
					     gpointer            user_data)
{
	RemoveInfo info;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	memset (&info, 0, sizeof (info));

	info.by_data = TRUE;
	info.user_data = user_data;

	g_hash_table_foreach_remove (component->priv->listeners, remove_listener, &info);
}

static void
bonobo_ui_component_destroy (GtkObject *object)
{
	BonoboUIComponent *comp = (BonoboUIComponent *) object;
	BonoboUIComponentPrivate *priv = comp->priv;

	if (priv) {
		g_hash_table_foreach_remove (
			priv->verbs, (GHRFunc) verb_destroy, NULL);
		g_hash_table_destroy (priv->verbs);
		priv->verbs = NULL;

		g_hash_table_foreach_remove (
			priv->listeners,
			(GHRFunc) listener_destroy, NULL);
		g_hash_table_destroy (priv->listeners);
		priv->listeners = NULL;

		g_free (priv->name);

		g_free (priv);
	}
	comp->priv = NULL;

	bonobo_ui_component_parent_class->destroy (object);
}

/**
 * bonobo_ui_component_construct:
 * @ui_component: the UI component itself
 * @name: the name of the UI component
 * 
 * Construct the UI component with name @name
 * 
 * Return value: a constructed UI component or NULL on error
 **/
BonoboUIComponent *
bonobo_ui_component_construct (BonoboUIComponent *ui_component,
			       const char        *name)
{
	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (ui_component), NULL);

	ui_component->priv->name = g_strdup (name);

	return ui_component;
}

/**
 * bonobo_ui_component_new:
 * @name: the name of the UI component
 * 
 * Create a new UI component with the specified name
 * 
 * Return value: a new UI component 
 **/
BonoboUIComponent *
bonobo_ui_component_new (const char *name)
{
	BonoboUIComponent *component;

	component = gtk_type_new (BONOBO_UI_COMPONENT_TYPE);
	if (!component)
		return NULL;

	return BONOBO_UI_COMPONENT (
		bonobo_ui_component_construct (
			component, name));
}

/**
 * bonobo_ui_component_new_default:
 * @void: 
 * 
 * Create a UI component with a unique default name
 * constructed from various available system properties.
 * 
 * Return value: a new UI component
 **/
BonoboUIComponent *
bonobo_ui_component_new_default (void)
{
	char              *name;
	BonoboUIComponent *component;

	static int idx = 0;

	name = g_strdup_printf (
		"%s-%s-%d-%d",
		gnome_app_id ? gnome_app_id : "unknown",
		gnome_app_version ? gnome_app_version : "-.-",
		getpid (), idx++);

	component = bonobo_ui_component_new (name);
	
	g_free (name);

	return component;
}

/**
 * bonobo_ui_component_set_name:
 * @component: the UI component
 * @name: the new name
 * 
 * Set the @name of the UI @component
 **/
void
bonobo_ui_component_set_name (BonoboUIComponent  *component,
			      const char         *name)
{
	g_return_if_fail (name != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	
	g_free (component->priv->name);
	component->priv->name = g_strdup (name);
}

/**
 * bonobo_ui_component_get_name:
 * @component: the UI component
 * 
 * Return value: the name of the UI @component
 **/
const char *
bonobo_ui_component_get_name (BonoboUIComponent  *component)
{
	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), NULL);
	
	return component->priv->name;
}

/**
 * bonobo_ui_component_set:
 * @component: the component
 * @path: the path to set
 * @xml: the xml to set
 * @ev: the (optional) CORBA exception environment
 * 
 * Set the @xml fragment into the remote #BonoboUIContainer's tree
 * attached to @component at the specified @path
 *
 * If you see blank menu items ( or just separators ) it's
 * likely that you should be using #bonobo_ui_component_set_translate
 * which substantialy deprecates this routine.
 **/
void
bonobo_ui_component_set (BonoboUIComponent  *component,
			 const char         *path,
			 const char         *xml,
			 CORBA_Environment  *ev)
{
	GET_CLASS (component)->xml_set (component, path, xml, ev);
}

static void
impl_xml_set (BonoboUIComponent  *component,
	      const char         *path,
	      const char         *xml,
	      CORBA_Environment  *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;
	char              *name;

	g_return_if_fail (xml != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (xml [0] == '\0')
		return;

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	name = component->priv->name ? component->priv->name : "";

	Bonobo_UIContainer_setNode (container, path, xml,
				    name, real_ev);

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Serious exception on node_set '$%s' of '%s' to '%s'",
			   bonobo_exception_get_text (real_ev), xml, path);

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_ui_component_set_tree:
 * @component: the component
 * @path: the path to set
 * @node: the #BonoboUINode representation of an xml tree to set
 * @ev: the (optional) CORBA exception environment
 * 
 * Set the @xml fragment into the remote #BonoboUIContainer's tree
 * attached to @component at the specified @path
 * 
 * It is likely that you don't want this routine, but want
 * bonobo_ui_component_set_translate.
 **/
void
bonobo_ui_component_set_tree (BonoboUIComponent *component,
			      const char        *path,
			      BonoboUINode      *node,
			      CORBA_Environment *ev)
{
	char *str;

	str = bonobo_ui_node_to_string (node, TRUE);	

/*	fprintf (stderr, "Merging '%s'\n", str); */
	
	bonobo_ui_component_set (
		component, path, str, ev);

	bonobo_ui_node_free_string (str);
}

/**
 * bonobo_ui_component_set_translate:
 * @component: the component
 * @path: the path to set
 * @xml: the non translated xml to set
 * @ev: the (optional) CORBA exception environment
 * 
 * This routine parses the XML strings, and converts any:
 * _label="Hello World" type strings into the translated,
 * and encoded format expected by the remote #BonoboUIContainer.
 **/
void
bonobo_ui_component_set_translate (BonoboUIComponent  *component,
				   const char         *path,
				   const char         *xml,
				   CORBA_Environment  *ev)
{
	BonoboUINode *node;

	if (!xml)
		return;

	node = bonobo_ui_node_from_string (xml);

	bonobo_ui_util_translate_ui (node);

	bonobo_ui_component_set_tree (component, path, node, ev);

	bonobo_ui_node_free (node);
}

/**
 * bonobo_ui_component_get:
 * @component: the component
 * @path: the path to get
 * @recurse: whether to get child nodes of @path
 * @ev: the (optional) CORBA exception environment
 * 
 * This routine fetches a chunk of the XML tree in the
 * #BonoboUIContainer associated with @component pointed
 * to by @path. If @recurse then the child nodes of @path
 * are returned too, otherwise they are not.
 *
 * Return value: an XML string
 **/
CORBA_char *
bonobo_ui_component_get (BonoboUIComponent *component,
			 const char        *path,
			 gboolean           recurse,
			 CORBA_Environment *ev)
{
	return GET_CLASS (component)->xml_get (component, path, recurse, ev);
}

static CORBA_char *
impl_xml_get (BonoboUIComponent *component,
	      const char        *path,
	      gboolean           recurse,
	      CORBA_Environment *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	CORBA_char *xml;
	Bonobo_UIContainer container;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), NULL);
	g_return_val_if_fail (component->priv != NULL, NULL);
	container = component->priv->container;
	g_return_val_if_fail (container != CORBA_OBJECT_NIL, NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	xml = Bonobo_UIContainer_getNode (container, path, !recurse, real_ev);

	if (BONOBO_EX (real_ev)) {
		if (!ev)
			g_warning ("Serious exception getting node '%s' '$%s'",
				   path, bonobo_exception_get_text (real_ev));

		if (!ev)
			CORBA_exception_free (&tmp_ev);

		return NULL;
	}

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return xml;
}

/**
 * bonobo_ui_component_get_tree:
 * @component: the component
 * @path: the path to get
 * @recurse: whether to get child nodes of @path
 * @ev: the (optional) CORBA exception environment
 * 
 * This routine fetches a chunk of the XML tree in the
 * #BonoboUIContainer associated with @component pointed
 * to by @path. If @recurse then the child nodes of @path
 * are returned too, otherwise they are not.
 *
 * Return value: an #BonoboUINode XML representation
 **/
BonoboUINode *
bonobo_ui_component_get_tree (BonoboUIComponent  *component,
			      const char         *path,
			      gboolean            recurse,
			      CORBA_Environment  *ev)
{	
	char *xml;
	BonoboUINode *node;

	xml = bonobo_ui_component_get (component, path, recurse, ev);

	if (!xml)
		return NULL;

	node = bonobo_ui_node_from_string (xml);

	CORBA_free (xml);

	if (!node)
		return NULL;

	return node;
}

/**
 * bonobo_ui_component_rm:
 * @component: the component
 * @path: the path to set
 * @ev: the (optional) CORBA exception environment
 *
 * This routine removes a chunk of the XML tree in the
 * #BonoboUIContainer associated with @component pointed
 * to by @path.
 **/
void
bonobo_ui_component_rm (BonoboUIComponent  *component,
			const char         *path,
			CORBA_Environment  *ev)
{
	GET_CLASS (component)->xml_rm (component, path, ev);
}

static void
impl_xml_rm (BonoboUIComponent  *component,
	     const char         *path,
	     CORBA_Environment  *ev)
{
	BonoboUIComponentPrivate *priv;
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	priv = component->priv;

	Bonobo_UIContainer_removeNode (
		container, path, priv->name, real_ev);

	if (!ev && BONOBO_EX (real_ev))
		g_warning ("Serious exception removing path  '%s' '%s'",
			   path, bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}


/**
 * bonobo_ui_component_object_set:
 * @component: the component
 * @path: the path to set
 * @control: a CORBA object reference
 * @ev: the (optional) CORBA exception environment
 * 
 * This registers the @control CORBA object into the
 * #BonoboUIContainer associated with this @component at
 * the specified @path. This is most often used to associate
 * controls with a certain path.
 **/
void
bonobo_ui_component_object_set (BonoboUIComponent  *component,
				const char         *path,
				Bonobo_Unknown      control,
				CORBA_Environment  *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	Bonobo_UIContainer_setObject (container, path, control, real_ev);

	if (!ev && BONOBO_EX (real_ev))
		g_warning ("Serious exception setting object '%s' '%s'",
			   path, bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_ui_component_object_get:
 * @component: the component
 * @path: the path to set
 * @ev: the (optional) CORBA exception environment
 * 
 * This returns the @control CORBA object registered with the
 * #BonoboUIContainer associated with this @component at
 * the specified @path.
 *
 * Returns: the associated remote CORBA object.
 **/
Bonobo_Unknown
bonobo_ui_component_object_get (BonoboUIComponent  *component,
				const char         *path,
				CORBA_Environment  *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_Unknown     ret;
	Bonobo_UIContainer container;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component),
			      CORBA_OBJECT_NIL);
	g_return_val_if_fail (component->priv != NULL,
			      CORBA_OBJECT_NIL);
	container = component->priv->container;
	g_return_val_if_fail (container != CORBA_OBJECT_NIL,
			      CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	ret = Bonobo_UIContainer_getObject (container, path, real_ev);

	if (!ev && BONOBO_EX (real_ev))
		g_warning ("Serious exception getting object '%s' '%s'",
			   path, bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return ret;
}

/**
 * bonobo_ui_component_add_verb_list_with_data:
 * @component: the component
 * @list: the list of verbs
 * @user_data: the user data passed to the verb callbacks
 * 
 * This is a helper function to save registering verbs individualy
 * it allows registration of a great batch of verbs at one time
 * in a list of #BonoboUIVerb terminated by #BONOBO_UI_VERB_END
 **/
void
bonobo_ui_component_add_verb_list_with_data (BonoboUIComponent  *component,
					     BonoboUIVerb       *list,
					     gpointer            user_data)
{
	BonoboUIVerb *l;

	g_return_if_fail (list != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	bonobo_object_ref (BONOBO_OBJECT (component));

	for (l = list; l && l->cname; l++) {
		bonobo_ui_component_add_verb (
			component, l->cname, l->cb,
			user_data?user_data:l->user_data);
	}

	bonobo_object_unref (BONOBO_OBJECT (component));
}

/**
 * bonobo_ui_component_add_verb_list:
 * @component: the component
 * @list: the list of verbs.
 * 
 * Add a list of verbs with no associated user_data, you probably
 * want #bonobo_ui_component_add_verb_list_with_data
 **/
void
bonobo_ui_component_add_verb_list (BonoboUIComponent  *component,
				   BonoboUIVerb       *list)
{
	bonobo_ui_component_add_verb_list_with_data (component, list, NULL);
}

/**
 * bonobo_ui_component_freeze:
 * @component: the component
 * @ev: the (optional) CORBA exception environment
 * 
 * This increments the freeze count on the remote associated
 * #BonoboUIContainer, this means that a batch of update operations
 * can be performed without a re-render penalty per update.
 *
 * NB. if your GUI is frozen / not updating you probably have a
 * freeze / thaw reference leak/
 **/
void
bonobo_ui_component_freeze (BonoboUIComponent *component,
			    CORBA_Environment *ev)
{
	GET_CLASS (component)->freeze (component, ev);
}

static void
impl_freeze (BonoboUIComponent *component,
	     CORBA_Environment *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	Bonobo_UIContainer_freeze (container, real_ev);

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Serious exception on UI freeze '$%s'",
			   bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_ui_component_thaw:
 * @component: the component
 * @ev: the (optional) CORBA exception environment
 * 
 * This decrements the freeze count on the remote associated
 * #BonoboUIContainer, this means that a batch of update operations
 * can be performed without a re-render penalty per update.
 *
 * NB. if your GUI is frozen / not updating you probably have a
 * freeze / thaw reference leak/
 **/
void
bonobo_ui_component_thaw (BonoboUIComponent *component,
			  CORBA_Environment *ev)
{
	GET_CLASS (component)->thaw (component, ev);
}

static void
impl_thaw (BonoboUIComponent *component,
	   CORBA_Environment *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	Bonobo_UIContainer_thaw (container, real_ev);

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Serious exception on UI thaw '$%s'",
			   bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_ui_component_set_prop:
 * @component: the component
 * @path: the path to set the property on
 * @prop: the property name
 * @value: the property value
 * @opt_ev: the (optional) CORBA exception environment
 * 
 * This helper function sets an XML property ( or attribute )
 * on the XML node pointed at by @path. It does this by
 * a read / modify / write process. If you find yourself
 * doing this a lot, you need to consider batching this process.
 **/
void
bonobo_ui_component_set_prop (BonoboUIComponent  *component,
			      const char         *path,
			      const char         *prop,
			      const char         *value,
			      CORBA_Environment  *opt_ev)
{
	g_return_if_fail (component != NULL);
	g_return_if_fail (component->priv != NULL);

	if (prop && (!strcmp (prop, "label") || !strcmp (prop, "tip"))) {
		char *encoded = bonobo_ui_util_encode_str (value);
		GET_CLASS (component)->set_prop (component, path, prop, encoded, opt_ev);
		g_free (encoded);
	} else
		GET_CLASS (component)->set_prop (component, path, prop, value, opt_ev);
}

static void
impl_set_prop (BonoboUIComponent  *component,
	       const char         *path,
	       const char         *prop,
	       const char         *value,
	       CORBA_Environment  *opt_ev)
{
	char *full_path;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	full_path = alloca (strlen (path) + 1 + strlen (prop) + 1);
	strcpy (full_path, path);
	strcat (full_path, "#");
	strcat (full_path, prop);

	bonobo_object_ref (BONOBO_OBJECT (component));

	bonobo_ui_component_set (
		component, full_path, value, opt_ev);

	bonobo_object_unref (BONOBO_OBJECT (component));
}

/**
 * bonobo_ui_component_get_prop:
 * @component: the component
 * @path: the path to set the property on
 * @prop: the property name
 * @value: the property value
 * @opt_ev: the (optional) CORBA exception environment
 * 
 * This helper function fetches an XML property ( or attribute )
 * from the XML node pointed at by @path in the #BonoboUIContainer
 * associated with @component
 * 
 * Return value: the xml property value or NULL - free with g_free.
 **/
gchar *
bonobo_ui_component_get_prop (BonoboUIComponent *component,
			      const char        *path,
			      const char        *prop,
			      CORBA_Environment *opt_ev)
{
	char *txt;

	g_return_val_if_fail (component != NULL, NULL);
	g_return_val_if_fail (component->priv != NULL, NULL);

	txt = GET_CLASS (component)->get_prop (component, path, prop, opt_ev);
	
	if (prop && (!strcmp (prop, "label") || !strcmp (prop, "tip"))) {
		char *decoded;
		gboolean err;

		decoded = bonobo_ui_util_decode_str (txt, &err);
		if (err)
			g_warning ("Encoding error getting prop '%s' at path '%s'",
				   prop, path);
		g_free (txt);

		return decoded;
	} else
		return txt;
}

static gchar *
impl_get_prop (BonoboUIComponent *component,
	       const char        *path,
	       const char        *prop,
	       CORBA_Environment *opt_ev)
{
	char *full_path;
	xmlChar *ans;
	gchar   *ret;
        CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), NULL);
	g_return_val_if_fail (component->priv != NULL, NULL);

	full_path = alloca (strlen (path) + 1 + strlen (prop) + 1);
	strcpy (full_path, path);
	strcat (full_path, "#");
	strcat (full_path, prop);

	bonobo_object_ref (BONOBO_OBJECT (component));

	if (opt_ev)
		real_ev = opt_ev;
	else {
		/* Hack to avoid warnings for reading non-existant properties */
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	ans = bonobo_ui_component_get (component, full_path, FALSE, real_ev);

	if (ans) {
		ret = g_strdup (ans);
		CORBA_free (ans);
	} else
		ret = NULL;

	bonobo_object_unref (BONOBO_OBJECT (component));

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);
	
	return ret;
}

/**
 * bonobo_ui_component_path_exists:
 * @component: the component
 * @path: the path to set the property on
 * @ev: the (optional) CORBA exception environment
 * 
 * Return value: TRUE if the path exists in the container.
 **/
gboolean
bonobo_ui_component_path_exists (BonoboUIComponent *component,
				 const char        *path,
				 CORBA_Environment *ev)
{
	return GET_CLASS (component)->exists (component, path, ev);
}

static gboolean
impl_exists (BonoboUIComponent *component,
	     const char        *path,
	     CORBA_Environment *ev)
{
	gboolean ret;
	Bonobo_UIContainer container;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), FALSE);
	g_return_val_if_fail (component->priv != NULL, FALSE);
	container = component->priv->container;
	g_return_val_if_fail (container != CORBA_OBJECT_NIL, FALSE);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	ret = Bonobo_UIContainer_exists (container, path, real_ev);

	if (BONOBO_EX (real_ev)) {
		ret = FALSE;
		if (!ev)
			g_warning ("Serious exception on path_exists '$%s'",
				   bonobo_exception_get_text (real_ev));
	}

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return ret;
}

/**
 * bonobo_ui_component_set_status:
 * @component: the component
 * @text: the new status text
 * @ev: the (optional) CORBA exception environment
 * 
 * This sets the contents of the status bar to @text in the
 * remote #BonoboUIContainer associated with @component.
 * This is done by setting the contents of the /status/main
 * node.
 **/
void
bonobo_ui_component_set_status (BonoboUIComponent *component,
				const char        *text,
				CORBA_Environment *opt_ev)
{
	if (text == NULL ||
	    text [0] == '\0') { /* Remove what was there to reveal other msgs */
		bonobo_ui_component_rm (component, "/status/main/*", opt_ev);
	} else {
		char *str, *encoded;

		encoded = bonobo_ui_util_encode_str (text);
		str = g_strdup_printf ("<item name=\"main\">%s</item>", encoded);
		g_free (encoded);
		
		bonobo_ui_component_set (component, "/status", str, opt_ev);
		
		g_free (str);
	}
}

/**
 * bonobo_ui_component_unset_container:
 * @component: the component
 * 
 * This dis-associates the @component from its associated
 * #BonoboUIContainer.
 **/
void
bonobo_ui_component_unset_container (BonoboUIComponent *component)
{
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	bonobo_object_ref (BONOBO_OBJECT (component));

	container = component->priv->container;
	component->priv->container = CORBA_OBJECT_NIL;

	if (container != CORBA_OBJECT_NIL) {
		CORBA_Environment  ev;
		char              *name;

		CORBA_exception_init (&ev);

		name = component->priv->name ? component->priv->name : "";

		Bonobo_UIContainer_removeNode (container, "/", name, &ev);
		Bonobo_UIContainer_deregisterComponent (container, name, &ev);
		
		if (BONOBO_EX (&ev))
			g_warning ("Serious exception deregistering component '%s'",
				   bonobo_exception_get_text (&ev));

		CORBA_exception_free (&ev);

		bonobo_object_release_unref (container, NULL);
	}

	bonobo_object_unref (BONOBO_OBJECT (component));
}

/**
 * bonobo_ui_component_set_container:
 * @component: the component
 * @container: a remote container object.
 * 
 * This associates this @component with a remote @container
 * object.
 **/
void
bonobo_ui_component_set_container (BonoboUIComponent *component,
				   Bonobo_UIContainer container)
{
	Bonobo_UIContainer ref_cont;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	g_return_if_fail (component->priv != NULL);

	bonobo_object_ref (BONOBO_OBJECT (component));

	if (container != CORBA_OBJECT_NIL) {
		Bonobo_UIComponent corba_component;
		char              *name;
		CORBA_Environment  ev;

		ref_cont = 		
			bonobo_object_dup_ref (container, NULL);

		CORBA_exception_init (&ev);

		corba_component = BONOBO_OBJREF (component);

		name = component->priv->name ? component->priv->name : "";

		Bonobo_UIContainer_registerComponent (
			ref_cont, name, corba_component, &ev);

		if (BONOBO_EX (&ev))
			g_warning ("Serious exception registering component '$%s'",
				   bonobo_exception_get_text (&ev));
		
		CORBA_exception_free (&ev);
	} else
		ref_cont = CORBA_OBJECT_NIL;

	bonobo_ui_component_unset_container (component);

	component->priv->container = ref_cont;

	bonobo_object_unref (BONOBO_OBJECT (component));
}

/**
 * bonobo_ui_component_get_container:
 * @component: the component.
 * 
 * Return value: the associated remote container
 **/
Bonobo_UIContainer
bonobo_ui_component_get_container (BonoboUIComponent *component)
{
	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component),
			      CORBA_OBJECT_NIL);
	g_return_val_if_fail (component->priv != NULL,
			      CORBA_OBJECT_NIL);
	
	return component->priv->container;
}

static void
bonobo_ui_component_class_init (BonoboUIComponentClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	BonoboUIComponentClass *uclass = BONOBO_UI_COMPONENT_CLASS (klass);
	POA_Bonobo_UIComponent__epv *epv = &klass->epv;
	
	bonobo_ui_component_parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_ui_component_destroy;

	uclass->ui_event = ui_event;

	signals [EXEC_VERB] = gtk_signal_new (
		"exec_verb", GTK_RUN_FIRST,
		object_class->type,
		GTK_SIGNAL_OFFSET (BonoboUIComponentClass, exec_verb),
		gtk_marshal_NONE__STRING,
		GTK_TYPE_NONE, 1, GTK_TYPE_STRING);

	signals [UI_EVENT] = gtk_signal_new (
		"ui_event", GTK_RUN_FIRST,
		object_class->type,
		GTK_SIGNAL_OFFSET (BonoboUIComponentClass, ui_event),
		gtk_marshal_NONE__POINTER_INT_POINTER,
		GTK_TYPE_NONE, 3, GTK_TYPE_STRING, GTK_TYPE_INT,
		GTK_TYPE_STRING);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	uclass->freeze   = impl_freeze;
	uclass->thaw     = impl_thaw;
	uclass->xml_set  = impl_xml_set;
	uclass->xml_get  = impl_xml_get;
	uclass->xml_rm   = impl_xml_rm;
	uclass->set_prop = impl_set_prop;
	uclass->get_prop = impl_get_prop;
	uclass->exists   = impl_exists;

	epv->describeVerbs = impl_Bonobo_UIComponent_describeVerbs;
	epv->execVerb      = impl_Bonobo_UIComponent_execVerb;
	epv->uiEvent       = impl_Bonobo_UIComponent_uiEvent;
}

static void
bonobo_ui_component_init (BonoboUIComponent *component)
{
	BonoboUIComponentPrivate *priv;

	priv = g_new0 (BonoboUIComponentPrivate, 1);
	priv->verbs = g_hash_table_new (g_str_hash, g_str_equal);
	priv->listeners = g_hash_table_new (g_str_hash, g_str_equal);
	priv->container = CORBA_OBJECT_NIL;

	component->priv = priv;
}

BONOBO_X_TYPE_FUNC_FULL (BonoboUIComponent, 
			   Bonobo_UIComponent,
			   PARENT_TYPE,
			   bonobo_ui_component);
