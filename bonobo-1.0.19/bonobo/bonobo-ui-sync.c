/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * bonobo-ui-sync.h: An abstract base for Bonobo XML / widget sync sync'ing.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include <config.h>
#include <stdlib.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-sync.h>

#define PARENT_TYPE gtk_object_get_type ()
#define CLASS(o) BONOBO_UI_SYNC_CLASS (GTK_OBJECT (o)->klass)

static void
impl_sync_state_placeholder (BonoboUISync     *sync,
			     BonoboUINode     *node,
			     BonoboUINode     *cmd_node,
			     GtkWidget        *widget,
			     GtkWidget        *parent)
{
	gboolean show = FALSE;
	char    *txt;
		
	if ((txt = bonobo_ui_engine_get_attr (
		node, cmd_node, "delimit"))) {

		show = !strcmp (txt, "top");
		bonobo_ui_node_free_string (txt);
	}
		
	if (show)
		gtk_widget_show (widget);	
	else
		gtk_widget_hide (widget);
}

static void
class_init (BonoboUISyncClass *sync_class)
{
	sync_class->sync_state_placeholder = impl_sync_state_placeholder;
}

/**
 * bonobo_ui_sync_get_type:
 * @void: 
 * 
 * Synchronizer type function for derivation.
 * 
 * Return value: the GtkType index.
 **/
GtkType
bonobo_ui_sync_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUISync",
			sizeof (BonoboUISync),
			sizeof (BonoboUISyncClass),
			(GtkClassInitFunc)  class_init,
			(GtkObjectInitFunc) NULL,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

/**
 * bonobo_ui_sync_construct:
 * @sync: the synchronizer
 * @engine: the associated engine
 * @is_recursive: whether it deals with its children recursively
 * @has_widgets: whether it has associated widgets.
 * 
 * Used to construct a new synchronizer object
 * 
 * Return value: the new object.
 **/
BonoboUISync *
bonobo_ui_sync_construct (BonoboUISync   *sync,
			  BonoboUIEngine *engine,
			  gboolean        is_recursive,
			  gboolean        has_widgets)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	sync->engine = engine;
	sync->is_recursive = is_recursive;
	sync->has_widgets  = has_widgets;

	return sync;
}

/**
 * bonobo_ui_sync_is_recursive:
 * @sync: the synchronizer
 * 
 * Return value: whether this deals with its children recursively
 **/
gboolean
bonobo_ui_sync_is_recursive (BonoboUISync *sync)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), FALSE);

	return sync->is_recursive;
}

/**
 * bonobo_ui_sync_has_widgets:
 * @sync: the synchronizer 
 * 
 * Return value: whether this deals with widgets
 **/
gboolean
bonobo_ui_sync_has_widgets (BonoboUISync *sync)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), FALSE);

	return sync->has_widgets;
}

/**
 * bonobo_ui_sync_state:
 * @sync: the synchronizer 
 * @node: the node 
 * @cmd_node: the associated command node 
 * @widget: the widget 
 * @parent: the parent of @node
 * 
 * This method is used to synchronize the state of a @node
 * with that of a @widget, by ensuring the pertainant
 * attributes are reflected in the widget view.
 **/
void
bonobo_ui_sync_state (BonoboUISync     *sync,
		      BonoboUINode     *node,
		      BonoboUINode     *cmd_node,
		      GtkWidget        *widget,
		      GtkWidget        *parent)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	CLASS (sync)->sync_state (sync, node, cmd_node, widget, parent);
}

/**
 * bonobo_ui_sync_state_placeholder:
 * @sync: the synchronizer 
 * @node: the node 
 * @cmd_node: the associated command node 
 * @widget: the widget 
 * @parent: the parent of @node
 * 
 * This synchronizes the state of a placeholder, there is
 * a default implementation for this method.
 **/
void
bonobo_ui_sync_state_placeholder (BonoboUISync     *sync,
				  BonoboUINode     *node,
				  BonoboUINode     *cmd_node,
				  GtkWidget        *widget,
				  GtkWidget        *parent)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	CLASS (sync)->sync_state_placeholder (sync, node, cmd_node, widget, 
					      parent);
}

/**
 * bonobo_ui_sync_build:
 * @sync: the synchronizer 
 * @node: the node 
 * @cmd_node: the associated command node 
 * @pos: the position in the parent container to insert at
 * @parent: the parent of @node 
 * 
 * This function causes a child widget to be build that matches
 * @node's attributes. This should then be inserted by into
 * @parent's associated widget at position @pos in the container.
 * 
 * Return value: the freshly built widget.
 **/
GtkWidget *
bonobo_ui_sync_build (BonoboUISync     *sync,
		      BonoboUINode     *node,
		      BonoboUINode     *cmd_node,
		      int              *pos,
		      GtkWidget        *parent)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	return CLASS (sync)->build (sync, node, cmd_node, pos, parent);
}

/**
 * bonobo_ui_sync_build_placeholder:
 * @sync: the synchronizer 
 * @node: the node 
 * @cmd_node: the associated command node 
 * @pos: position in the parent to insert the built widget
 * @parent: the parent of @node 
 * 
 * As for #bonobo_ui_sync_build but for placeholders
 * 
 * Return value: the freshly built widget.
 **/
GtkWidget *
bonobo_ui_sync_build_placeholder (BonoboUISync     *sync,
				  BonoboUINode     *node,
				  BonoboUINode     *cmd_node,
				  int              *pos,
				  GtkWidget        *parent)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	return CLASS (sync)->build_placeholder (
		sync, node, cmd_node, pos, parent);
}

/**
 * bonobo_ui_sync_get_widgets:
 * @sync: the synchronizer 
 * @node: the node 
 * 
 * This method is used to obtain a sensibly ordered list
 * of child widgets of the container associated with @node.
 * Essentialy this does something like gtk_container_children
 * but preserving the visible order of the widgets in the list.
 * 
 * Return value: An ordered list of child widgets of @node
 **/
GList *
bonobo_ui_sync_get_widgets (BonoboUISync *sync,
			    BonoboUINode *node)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	if (CLASS (sync)->get_widgets)
		return CLASS (sync)->get_widgets (sync, node);
	else
		return NULL;
}

/**
 * bonobo_ui_sync_state_update:
 * @sync: the synchronizer 
 * @widget: the widget 
 * @new_state: the new state
 * 
 * This is used to synchronize state with a stateful widget,
 * eg. when a "state" attribute is set, this is not reflected
 * in the normal 'state-sync' process, but occurs later with
 * a set of state_updates to avoid re-enterancy problems.
 **/
void
bonobo_ui_sync_state_update (BonoboUISync     *sync,
			     GtkWidget        *widget,
			     const char       *new_state)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	CLASS (sync)->state_update (sync, widget, new_state);
}

/**
 * bonobo_ui_sync_remove_root:
 * @sync: the synchronizer 
 * @root: the toplevel node to be removed.
 * 
 * This is called when a 'root' or toplevel node is
 * removed that this synchronizer deals with. eg. in
 * the toolbar case, this might trigger hiding an
 * associated dock item.
 **/
void
bonobo_ui_sync_remove_root (BonoboUISync *sync,
			    BonoboUINode *root)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	if (CLASS (sync)->remove_root)
		CLASS (sync)->remove_root (sync, root);
}

/**
 * bonobo_ui_sync_update_root:
 * @sync: the synchronizer 
 * @root: the toplevel node
 * 
 * This flags the fact that a toplevel node has changed
 * and is used primarily by non-recursive handlers, such
 * as the keybinding sync method.
 **/
void
bonobo_ui_sync_update_root (BonoboUISync *sync,
			    BonoboUINode *root)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	if (CLASS (sync)->update_root)
		CLASS (sync)->update_root (sync, root);
}

/**
 * bonobo_ui_sync_ignore_widget:
 * @sync: the synchronizer 
 * @widget: the widget 
 * 
 * Return value: TRUE if this widget should be ignored in a container
 * this is the case for eg. menu tearoffs items, and toolbar popout items.
 **/
gboolean
bonobo_ui_sync_ignore_widget (BonoboUISync *sync,
			      GtkWidget    *widget)
{
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), FALSE);

	if (CLASS (sync)->ignore_widget)
		return CLASS (sync)->ignore_widget (sync, widget);
	else
		return FALSE;
}

/**
 * bonobo_ui_sync_stamp_root:
 * @sync: the synchronizer 
 * 
 * This asks the synchronizer to stamp all its associated
 * root widget containers into the XML tree.
 **/
void
bonobo_ui_sync_stamp_root (BonoboUISync *sync)
{
	g_return_if_fail (BONOBO_IS_UI_SYNC (sync));

	if (CLASS (sync)->stamp_root)
		CLASS (sync)->stamp_root (sync);
}

/**
 * bonobo_ui_sync_can_handle:
 * @sync: the synchronizer 
 * @node: the node 
 * 
 * This is used to determine which, of multiple synchronizers
 * can be used to deal with a specific node type. Each synchronizer
 * deals with different types of node.
 * 
 * Return value: TRUE if the synchronizer can deal with this node type
 **/
gboolean
bonobo_ui_sync_can_handle (BonoboUISync *sync,
			   BonoboUINode *node)
{
	if (CLASS (sync)->can_handle)
		return CLASS (sync)->can_handle (sync, node);
	else
		return FALSE;
}

/**
 * bonobo_ui_sync_get_attached:
 * @sync: the synchronizer 
 * @widget: the widget 
 * @node: the node 
 * 
 * This is used to get an 'attached' widget - some
 * widgets have associated widgets that are coupled
 * in strange ways - eg. GtkMenuItem <-> GtkMenuShell
 * It is neccessary to store the GtkContainer item of
 * these couples in the XML tree, since then we can
 * do things more genericaly and cleanly.
 * 
 * Return value: an assoicated widget or NULL if none exists.
 **/
GtkWidget *
bonobo_ui_sync_get_attached (BonoboUISync *sync,
			     GtkWidget    *widget,
			     BonoboUINode *node)
{
	/*
	 *   For some widgets such as menus, the submenu widget
	 * is attached to the actual container in a strange way
	 * this works around only having single inheritance.
	 */
	g_return_val_if_fail (BONOBO_IS_UI_SYNC (sync), NULL);

	if (CLASS (sync)->get_attached)
		return CLASS (sync)->get_attached (sync, widget, node);
	else
		return NULL;
}

/**
 * bonobo_ui_sync_do_show_hide:
 * @sync: the synchronizer 
 * @node: the node 
 * @cmd_node: the associated command node 
 * @widget: the widget 
 * 
 * This is a helper function that applies the hidden attribute
 * from either the @node or fallback to the @cmd_node to the
 * @widget.
 * 
 * Return value: TRUE if the widget's hidden / shown state changed,
 * this is needed to work around some nasty dock sizing bugs.
 **/
gboolean
bonobo_ui_sync_do_show_hide (BonoboUISync *sync,
			     BonoboUINode *node,
			     BonoboUINode *cmd_node,
			     GtkWidget    *widget)
{
	char      *txt;
	gboolean   changed;
	GtkWidget *attached;

	if (sync && 
	    (attached = bonobo_ui_sync_get_attached (
		    sync, widget, node)))

		widget = attached;

	if (!widget)
		return FALSE;

	if ((txt = bonobo_ui_engine_get_attr (node, cmd_node, "hidden"))) {
		if (atoi (txt)) {
			changed = GTK_WIDGET_VISIBLE (widget);
			gtk_widget_hide (widget);
		} else {
			changed = !GTK_WIDGET_VISIBLE (widget);
			gtk_widget_show (widget);
		}
		bonobo_ui_node_free_string (txt);
	} else {
		changed = !GTK_WIDGET_VISIBLE (widget);
		gtk_widget_show (widget);
	}

	return changed;
}
