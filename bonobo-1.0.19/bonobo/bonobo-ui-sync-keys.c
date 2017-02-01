/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * bonobo-ui-sync-keys.h: The Bonobo UI/XML sync engine for keys bits.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include <config.h>
#include <stdlib.h>

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-sync.h>
#include <bonobo/bonobo-ui-sync-keys.h>

static GtkObjectClass *parent_class = NULL;

#define PARENT_TYPE bonobo_ui_sync_get_type ()

#define	BINDING_MOD_MASK()				\
	(gtk_accelerator_get_default_mod_mask () | GDK_RELEASE_MASK)

typedef struct {
	guint           key;
	GdkModifierType mods;
	BonoboUINode        *node;
} Binding;

static gboolean
keybindings_free (gpointer key,
		  gpointer value,
		  gpointer user_data)
{
	g_free (key);

	return TRUE;
}

/* Shamelessly stolen from gtkbindings.c */
static guint
keybinding_hash_fn (gconstpointer  key)
{
	register const Binding *e = key;
	register guint h;

	h = e->key;
	h ^= e->mods;

	return h;
}

static gint
keybinding_compare_fn (gconstpointer a,
		       gconstpointer b)
{
	register const Binding *ba = a;
	register const Binding *bb = b;

	return (ba->key == bb->key && ba->mods == bb->mods);
}

gint
bonobo_ui_sync_keys_binding_handle (GtkWidget        *widget,
				    GdkEventKey      *event,
				    BonoboUISyncKeys *msync)
{
	Binding lookup, *binding;

	lookup.key  = gdk_keyval_to_lower (event->keyval);
	lookup.mods = event->state & BINDING_MOD_MASK ();

	if (!(binding = g_hash_table_lookup (
		msync->keybindings, &lookup)))

		return FALSE;
	else {
		bonobo_ui_engine_emit_verb_on (
			msync->parent.engine, binding->node);

		return TRUE;
	}
	
	return FALSE;
}

static void
impl_destroy (GtkObject *object)
{
	BonoboUISyncKeys *sync;

	sync = BONOBO_UI_SYNC_KEYS (object);

	g_hash_table_foreach_remove (sync->keybindings,
				     keybindings_free, NULL);
	g_hash_table_destroy (sync->keybindings);
	sync->keybindings = NULL;

	parent_class->destroy (object);
}

static void
update_keybindings (BonoboUISyncKeys *msync,
		    BonoboUINode     *node)
{
	BonoboUINode    *l;
	BonoboUIXmlData *data;

	if (!bonobo_ui_engine_node_is_dirty (msync->parent.engine, node))
		return;

	data = bonobo_ui_xml_get_data (NULL, node);
	g_return_if_fail (data != NULL);

	if (!data->dirty)
		return;

	g_hash_table_foreach_remove (
		msync->keybindings, keybindings_free, NULL);

	for (l = bonobo_ui_node_children (node); l;
	     l = bonobo_ui_node_next (l)) {
		guint           key;
		GdkModifierType mods;
		char           *name;
		Binding        *binding;
		
		name = bonobo_ui_node_get_attr (l, "name");
		if (!name)
			continue;
		
		bonobo_ui_util_accel_parse (name, &key, &mods);
		bonobo_ui_node_free_string (name);

		binding       = g_new0 (Binding, 1);
		binding->mods = mods & BINDING_MOD_MASK ();
		binding->key  = gdk_keyval_to_lower (key);
		binding->node = l;

		g_hash_table_insert (msync->keybindings, binding, binding);
	}
}

static void
impl_bonobo_ui_sync_keys_update_root (BonoboUISync *sync,
				      BonoboUINode *root)
{
	if (bonobo_ui_node_has_name (root, "keybindings"))
		update_keybindings (BONOBO_UI_SYNC_KEYS (sync), root);
}

static void
impl_bonobo_ui_sync_keys_stamp_root (BonoboUISync *sync)
{
	BonoboUINode *node;

	node = bonobo_ui_engine_get_path (sync->engine, "/keybindings");

	if (node)
		bonobo_ui_engine_node_set_dirty (sync->engine, node, TRUE);
}

static gboolean
impl_bonobo_ui_sync_keys_can_handle (BonoboUISync *sync,
				     BonoboUINode *node)
{
	return bonobo_ui_node_has_name (node, "keybindings");
}

/* We need to map the shell to the item */

static void
class_init (BonoboUISyncClass *sync_class)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class (BONOBO_TYPE_UI_SYNC);

	object_class = GTK_OBJECT_CLASS (sync_class);
	object_class->destroy  = impl_destroy;

	sync_class->update_root   = impl_bonobo_ui_sync_keys_update_root;
	sync_class->can_handle    = impl_bonobo_ui_sync_keys_can_handle;
	sync_class->stamp_root    = impl_bonobo_ui_sync_keys_stamp_root;
}

static void
init (BonoboUISyncKeys *msync)
{
	msync->keybindings = g_hash_table_new (
		keybinding_hash_fn, keybinding_compare_fn);	
}

GtkType
bonobo_ui_sync_keys_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUISyncKeys",
			sizeof (BonoboUISyncKeys),
			sizeof (BonoboUISyncKeysClass),
			(GtkClassInitFunc)  class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

BonoboUISync *
bonobo_ui_sync_keys_new (BonoboUIEngine *engine)
{
	BonoboUISyncKeys *sync;

	g_return_val_if_fail (BONOBO_IS_UI_ENGINE (engine), NULL);

	sync = gtk_type_new (BONOBO_TYPE_UI_SYNC_KEYS);

	return bonobo_ui_sync_construct (
		BONOBO_UI_SYNC (sync), engine, FALSE, FALSE);
}
