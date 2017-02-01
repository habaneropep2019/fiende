/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-running-context.c: A global running interface
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#include <config.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-running-context.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

#ifdef BONOBO_OBJECT_DEBUG
#	define BONOBO_RUNNING_HOOKS
#endif

/*
 * NB. for a quicker debugging experience simply
 * #define BONOBO_RUNNING_HOOKS
 */

typedef struct {
	gboolean    emitted_last_unref;
	GHashTable *objects;
	GHashTable *keys;
} BonoboRunningInfo;

BonoboRunningInfo *bonobo_running_info = NULL;
BonoboObject      *bonobo_running_context = NULL;
BonoboEventSource *bonobo_running_event_source = NULL;

enum {
	LAST_UNREF,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };

static void
key_free (gpointer name, gpointer dummy1, gpointer user_data)
{
	g_free (name);
}

#ifdef BONOBO_RUNNING_HOOKS
static void
bonobo_debug_print (char *name, char *fmt, ...)
{
	va_list args;
           
	va_start (args, fmt);
	
	printf ("[%06d]:%-15s ", getpid (), name); 
	vprintf (fmt, args);
	printf ("\n"); 

	va_end (args);
}

static void
bonobo_ri_debug_foreach (gpointer key, gpointer value, gpointer user_data)
{
	CORBA_Object *o = value;
	
	bonobo_debug_print ("", "[%p]:CORBA_Object still running", o);
		
}
#endif

static void
running_info_destroy (void)
{
	if (bonobo_running_info) {

		BonoboRunningInfo *ri = bonobo_running_info;

#ifdef BONOBO_RUNNING_HOOKS
		bonobo_debug_print ("rinfo-start", 
			  "-------------------------------------------------");

		bonobo_debug_print ("running-objects", "%d running objects", 
				    g_hash_table_size (ri->objects));
		g_hash_table_foreach (ri->objects,
				      bonobo_ri_debug_foreach, NULL);
		bonobo_debug_print ("rinfo-end", 
			  "-------------------------------------------------");
#endif
		if (ri->objects)
			g_hash_table_destroy (ri->objects);
		ri->objects = NULL;

		if (ri->keys) {
			g_hash_table_foreach_remove (
				ri->keys, (GHRFunc) key_free, NULL);
			g_hash_table_destroy (ri->keys);
			ri->keys = NULL;
		}
		g_free (ri);
	}
	bonobo_running_info = NULL;

	if (bonobo_running_context)
		bonobo_object_unref (BONOBO_OBJECT (bonobo_running_context));
	bonobo_running_context = NULL;
	bonobo_running_event_source = NULL;
}

static void
check_destroy (BonoboObject *object,
	       gpointer      dummy)
{
	bonobo_running_context = NULL;
	bonobo_running_event_source = NULL;
}

static BonoboRunningInfo *
get_running_info (gboolean create)
{
	if (!bonobo_running_info && create) {
		bonobo_running_info = g_new (BonoboRunningInfo, 1);
		bonobo_running_info->objects = g_hash_table_new (NULL, NULL);
		bonobo_running_info->keys    = g_hash_table_new (g_str_hash, g_str_equal);
		bonobo_running_info->emitted_last_unref = FALSE;

		g_atexit (running_info_destroy);
	}

	return bonobo_running_info;
}

static void
check_empty (void)
{
	BonoboRunningInfo *ri = get_running_info (FALSE);

	if (!ri || !bonobo_running_context)
		return;

	if (!ri->emitted_last_unref &&
	    (g_hash_table_size (ri->objects) == 0) &&
	    (g_hash_table_size (ri->keys) == 0)) {

		ri->emitted_last_unref = TRUE;

		gtk_signal_emit (GTK_OBJECT (bonobo_running_context),
				 signals [LAST_UNREF]);

		g_return_if_fail (bonobo_running_event_source != NULL);

		bonobo_event_source_notify_listeners (
			bonobo_running_event_source,
			"bonobo:last_unref", NULL, NULL);
	}
}

#ifndef bonobo_running_context_add_object
void
bonobo_running_context_add_object (CORBA_Object object)
{
#ifdef BONOBO_RUNNING_HOOKS
	bonobo_running_context_trace_objects (object, "local", 0, 0);
#else 
	BonoboRunningInfo *ri = get_running_info (TRUE);

	g_hash_table_insert (ri->objects, object, object);
#endif
}
#endif


#ifndef bonobo_running_context_remove_object
void
bonobo_running_context_remove_object (CORBA_Object object)
{
#ifdef BONOBO_RUNNING_HOOKS
	bonobo_running_context_trace_objects (object, "local", 0, 1);
#else 
	BonoboRunningInfo *ri = get_running_info (FALSE);

	if (ri) {
		g_hash_table_remove (ri->objects, object);

		check_empty ();
	}
#endif
}
#endif

#ifndef bonobo_running_context_ignore_object
void
bonobo_running_context_ignore_object (CORBA_Object object)
{
#ifdef BONOBO_RUNNING_HOOKS
	bonobo_running_context_trace_objects (object, "local", 0, 2);
#else 
	BonoboRunningInfo *ri = get_running_info (FALSE);

	if (ri) {
		g_hash_table_remove (ri->objects, object);
	}
#endif
}
#endif

void          
bonobo_running_context_trace_objects (CORBA_Object object,
				      const char  *fn,
				      int          line,
				      int          mode)
{
	BonoboRunningInfo *ri = get_running_info (mode == 0);
#ifdef BONOBO_RUNNING_HOOKS
	char *cmode[] = {
		"add_object",
		"remove_object",
		"ignore_object"		
	};
#endif
	if (ri) {
		switch (mode) {
		case 0:
			g_hash_table_insert (ri->objects, object, object);
			break;
		case 1:
			g_hash_table_remove (ri->objects, object);

			check_empty ();
			break;
		case 2:
			g_hash_table_remove (ri->objects, object);
			break;
		}
#ifdef BONOBO_RUNNING_HOOKS
		bonobo_debug_print (cmode [mode], 
		        "[%p]:CORBA_Object %d running objects at %s:%d",
			object, g_hash_table_size (ri->objects), fn, line);
#endif
	}
}

static void
impl_Bonobo_RunningContext_addObject (PortableServer_Servant servant,
				      const CORBA_Object     object,
				      CORBA_Environment     *ev)
{
	bonobo_running_context_add_object (object);
}

static void
impl_Bonobo_RunningContext_removeObject (PortableServer_Servant servant,
					 const CORBA_Object     object,
					 CORBA_Environment     *ev)
{
	bonobo_running_context_remove_object (object);
}

static void
impl_Bonobo_RunningContext_addKey (PortableServer_Servant servant,
				   const CORBA_char      *key,
				   CORBA_Environment     *ev)
{
	char              *key_copy, *old_key;
	BonoboRunningInfo *ri = get_running_info (TRUE);

	old_key = g_hash_table_lookup (ri->keys, key);
	if (old_key) {
		g_free (old_key);
		g_hash_table_remove (ri->keys, key);
	}
	key_copy = g_strdup (key);

	g_hash_table_insert (ri->keys, key_copy, key_copy);
}

static void
impl_Bonobo_RunningContext_removeKey (PortableServer_Servant servant,
				      const CORBA_char      *key,
				      CORBA_Environment     *ev)
{
	BonoboRunningInfo *ri = get_running_info (FALSE);
	char              *old_key;

	if (!ri)
		return;

	old_key = g_hash_table_lookup (ri->keys, key);
	if (old_key)
		g_free (old_key);
	g_hash_table_remove (ri->keys, key);

	check_empty ();
}

static void
impl_Bonobo_RunningContext_atExitUnref (PortableServer_Servant servant,
					const CORBA_Object     object,
					CORBA_Environment     *ev)
{
	bonobo_running_context_at_exit_unref (object);
}

static void
bonobo_running_context_class_init (BonoboRunningContextClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_RunningContext__epv *epv = &klass->epv;

	((BonoboRunningContextClass *)klass)->last_unref = NULL;

	signals [LAST_UNREF] = gtk_signal_new (
		"last_unref", GTK_RUN_FIRST, object_class->type,
		GTK_SIGNAL_OFFSET (BonoboRunningContextClass, last_unref),
		gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	epv->addObject     = impl_Bonobo_RunningContext_addObject;
	epv->removeObject  = impl_Bonobo_RunningContext_removeObject;
	epv->addKey        = impl_Bonobo_RunningContext_addKey;
	epv->removeKey     = impl_Bonobo_RunningContext_removeKey;
	epv->atExitUnref   = impl_Bonobo_RunningContext_atExitUnref;

}

static void 
bonobo_running_context_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboRunningContext, 
			 Bonobo_RunningContext,
			 PARENT_TYPE,
			 bonobo_running_context);

BonoboObject *
bonobo_running_context_new (void)
{
	if (bonobo_running_context) {
		bonobo_object_ref (bonobo_running_context);
		return bonobo_running_context;
	}

	bonobo_running_context = gtk_type_new (bonobo_running_context_get_type ());

	bonobo_running_event_source = bonobo_event_source_new ();
	bonobo_running_context_ignore_object (
	        BONOBO_OBJREF (bonobo_running_event_source));
	bonobo_event_source_ignore_listeners (bonobo_running_event_source);

	bonobo_object_add_interface (BONOBO_OBJECT (bonobo_running_context),
				     BONOBO_OBJECT (bonobo_running_event_source));

	gtk_signal_connect (GTK_OBJECT (bonobo_running_context), "destroy",
			    (GtkSignalFunc) check_destroy, NULL);

	return bonobo_running_context;
}

BonoboObject *
bonobo_context_running_get (void)
{
	return bonobo_running_context_new ();
}

static void
last_unref_cb (gpointer      context,
	       CORBA_Object  object)
{
	bonobo_object_release_unref (object, NULL);
}

void 
bonobo_running_context_at_exit_unref (CORBA_Object object)
{
	CORBA_Environment ev;
	CORBA_Object obj_dup;

	CORBA_exception_init (&ev);

	obj_dup = CORBA_Object_duplicate (object, &ev);

	bonobo_running_context_ignore_object (obj_dup);

	if (bonobo_running_context)
		gtk_signal_connect (GTK_OBJECT (bonobo_running_context),
				    "last_unref", last_unref_cb, obj_dup);
	
	CORBA_exception_free (&ev);
}

static void
last_unref_exit_cb (gpointer      context,
		    BonoboObject *object)
{
        bonobo_object_unref (object);
	gtk_main_quit ();
}

void 
bonobo_running_context_auto_exit_unref (BonoboObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_OBJECT (object));

	bonobo_running_context_ignore_object (BONOBO_OBJREF (object));

	if (bonobo_running_context)
		gtk_signal_connect (GTK_OBJECT (bonobo_running_context),
				    "last_unref", last_unref_exit_cb, object);

}

