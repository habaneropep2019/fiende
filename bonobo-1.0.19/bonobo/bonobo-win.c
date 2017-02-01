/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-win.c: The Bonobo Window implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#include "config.h"
#include <libgnomeui/gnome-dock-item.h>
#include <libgnomeui/gnome-dock.h>
#include <libgnomeui/gnome-preferences.h>
#include <libgnomeui/gnome-window-icon.h>
#include <liboaf/liboaf.h>

#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-sync-menu.h>
#include <bonobo/bonobo-ui-sync-keys.h>
#include <bonobo/bonobo-ui-sync-status.h>
#include <bonobo/bonobo-ui-sync-toolbar.h>
#include <bonobo/bonobo-win.h>

#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>

GtkWindowClass *bonobo_window_parent_class = NULL;

struct _BonoboWindowPrivate {
	BonoboUIEngine *engine;

	BonoboUISync   *sync_menu;
	BonoboUISync   *sync_keys;
	BonoboUISync   *sync_status;
	BonoboUISync   *sync_toolbar;
	
	GnomeDock      *dock;

	GnomeDockItem  *menu_item;
	GtkMenuBar     *menu;

	GtkAccelGroup  *accel_group;

	char           *name;		/* Win name */
	char           *prefix;		/* Win prefix */

	GtkWidget      *main_vbox;

	GtkBox         *status;

	GtkWidget      *client_area;
};

/**
 * bonobo_window_deregister_dead_components:
 * @win: 
 * 
 * Deprecated
 **/
void
bonobo_window_deregister_dead_components (BonoboWindow *win)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_engine_deregister_dead_components (
		win->priv->engine);
}

/**
 * bonobo_window_register_component:
 * @win: 
 * @name: 
 * @component: 
 * 
 * Deprecated
 **/
void
bonobo_window_register_component (BonoboWindow  *win,
				  const char    *name,
				  Bonobo_Unknown component)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_engine_register_component (
		win->priv->engine, name, component);
}

/**
 * bonobo_window_deregister_component:
 * @win: 
 * @name: 
 * 
 * Deprecated
 **/
void
bonobo_window_deregister_component (BonoboWindow *win,
				    const char   *name)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_engine_deregister_component (
		win->priv->engine, name);
}

/**
 * bonobo_window_deregister_component_by_ref:
 * @win: 
 * @ref: 
 * 
 * Deprecated
 **/
void
bonobo_window_deregister_component_by_ref (BonoboWindow  *win,
					   Bonobo_Unknown ref)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_engine_deregister_component_by_ref (
		win->priv->engine, ref);
}

/**
 * bonobo_window_remove_popup:
 * @win: the window
 * @path: the path
 * 
 * Remove the popup at @path
 **/
void
bonobo_window_remove_popup (BonoboWindow     *win,
			    const char    *path)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_sync_menu_remove_popup (
		BONOBO_UI_SYNC_MENU (win->priv->sync_menu), path);
}

/**
 * bonobo_window_add_popup:
 * @win: the window
 * @menu: the menu widget
 * @path: the path
 * 
 * Add a popup @menu at @path
 **/
void
bonobo_window_add_popup (BonoboWindow *win,
			 GtkMenu      *menu,
			 const char   *path)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_sync_menu_add_popup (
		BONOBO_UI_SYNC_MENU (win->priv->sync_menu), menu, path);
}

/**
 * bonobo_window_deregister_get_component_names:
 * @win: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
GList *
bonobo_window_deregister_get_component_names (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	return bonobo_ui_engine_get_component_names (win->priv->engine);
}


/**
 * bonobo_window_component_get:
 * @win: 
 * @name: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
Bonobo_Unknown
bonobo_window_component_get (BonoboWindow *win,
			     const char  *name)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), CORBA_OBJECT_NIL);

	return bonobo_ui_engine_get_component (win->priv->engine, name);
}

/**
 * bonobo_window_set_contents:
 * @win: the bonobo window
 * @contents: the new widget for it to contain.
 * 
 * Insert a widget into the main window contents.
 **/
void
bonobo_window_set_contents (BonoboWindow *win,
			    GtkWidget    *contents)
{
	g_return_if_fail (win != NULL);
	g_return_if_fail (win->priv != NULL);
	g_return_if_fail (win->priv->client_area != NULL);

	gtk_container_add (GTK_CONTAINER (win->priv->client_area), contents);
}

/**
 * bonobo_window_get_contents:
 * @win: the bonobo window
 * 
 * Return value: the contained widget
 **/
GtkWidget *
bonobo_window_get_contents (BonoboWindow *win)
{
	GList     *children;
	GtkWidget *widget;

	g_return_val_if_fail (win != NULL, NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);
	g_return_val_if_fail (win->priv->dock != NULL, NULL);

	children = gtk_container_children (
		GTK_CONTAINER (win->priv->client_area));

	widget = children ? children->data : NULL;

	g_list_free (children);

	return widget;
}

static void
destroy_priv (BonoboWindowPrivate *priv)
{
	gtk_object_unref (GTK_OBJECT (priv->engine));
	priv->engine = NULL;

	g_free (priv->name);
	priv->name = NULL;

	g_free (priv->prefix);
	priv->prefix = NULL;
	
	g_free (priv);
}

static void
bonobo_window_finalize (GtkObject *object)
{
	BonoboWindow *win = (BonoboWindow *)object;
	
	if (win) {
		if (win->priv)
			destroy_priv (win->priv);
		win->priv = NULL;
	}

	GTK_OBJECT_CLASS (bonobo_window_parent_class)->finalize (object);
}

char *
bonobo_window_xml_get (BonoboWindow *win,
		       const char   *path,
		       gboolean      node_only)
{
  	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	return bonobo_ui_engine_xml_get (win->priv->engine, path, node_only);
}

/**
 * bonobo_window_xml_node_exists:
 * @win: 
 * @path: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
gboolean
bonobo_window_xml_node_exists (BonoboWindow *win,
			       const char   *path)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), FALSE);

	return bonobo_ui_engine_xml_node_exists (
		win->priv->engine, path);
}

/**
 * bonobo_window_object_set:
 * @win: 
 * @path: 
 * @object: 
 * @ev: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
BonoboUIError   
bonobo_window_object_set (BonoboWindow  *win,
			  const char    *path,
			  Bonobo_Unknown object,
			  CORBA_Environment *ev)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win),
			      BONOBO_UI_ERROR_BAD_PARAM);

	return bonobo_ui_engine_object_set (
		win->priv->engine, path, object, ev);
}

/**
 * bonobo_window_object_get:
 * @win: 
 * @path: 
 * @object: 
 * @ev: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
BonoboUIError   
bonobo_window_object_get (BonoboWindow   *win,
			  const char     *path,
			  Bonobo_Unknown *object,
			  CORBA_Environment *ev)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win),
			      BONOBO_UI_ERROR_BAD_PARAM);

	return bonobo_ui_engine_object_get (
		win->priv->engine, path, object, ev);
}

/**
 * bonobo_window_xml_merge_tree:
 * @win: 
 * @path: 
 * @tree: 
 * @component: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
BonoboUIError   
bonobo_window_xml_merge_tree (BonoboWindow *win,
			      const char   *path,
			      BonoboUINode *tree,
			      const char   *component)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win),
			      BONOBO_UI_ERROR_BAD_PARAM);

	return bonobo_ui_engine_xml_merge_tree (
		win->priv->engine, path, tree, component);
}

/**
 * bonobo_window_xml_merge:
 * @win: 
 * @path: 
 * @xml: 
 * @component: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
BonoboUIError   
bonobo_window_xml_merge (BonoboWindow *win,
			 const char   *path,
			 const char   *xml,
			 const char   *component)
{
	BonoboUIError    err;
	BonoboUINode *node;
	
	g_return_val_if_fail (win != NULL, BONOBO_UI_ERROR_BAD_PARAM);
	g_return_val_if_fail (xml != NULL, BONOBO_UI_ERROR_BAD_PARAM);
	g_return_val_if_fail (win->priv != NULL, BONOBO_UI_ERROR_BAD_PARAM);

/*	fprintf (stderr, "Merging :\n%s\n", xml);*/

	node = bonobo_ui_node_from_string (xml);
	
	if (!node)
		return BONOBO_UI_ERROR_INVALID_XML;

	err = bonobo_window_xml_merge_tree (win, path, node, component);

	return err;
}

/**
 * bonobo_window_xml_rm:
 * @win: 
 * @path: 
 * @by_component: 
 * 
 * Deprecated
 * 
 * Return value: 
 **/
BonoboUIError   
bonobo_window_xml_rm (BonoboWindow *win,
		      const char   *path,
		      const char   *by_component)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), BONOBO_UI_ERROR_BAD_PARAM);

	return bonobo_ui_engine_xml_rm (
		win->priv->engine, path, by_component);
}

/**
 * bonobo_window_freeze:
 * @win: 
 * 
 * Deprecated
 **/
void
bonobo_window_freeze (BonoboWindow *win)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	bonobo_ui_engine_freeze (win->priv->engine);
}

/**
 * bonobo_window_thaw:
 * @win: 
 * 
 * Deprecated
 **/
void
bonobo_window_thaw (BonoboWindow *win)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));
	
	bonobo_ui_engine_thaw (win->priv->engine);
}

/**
 * bonobo_window_dump:
 * @win: 
 * @msg: 
 * 
 * Deprecated
 **/
void
bonobo_window_dump (BonoboWindow *win,
		    const char   *msg)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	fprintf (stderr, "Bonobo Win '%s'\n", win->priv->name);

	bonobo_ui_engine_dump (win->priv->engine, stderr, msg);
}

/**
 * bonobo_window_get_accel_group:
 * @win: the bonobo window
 * 
 * Return value: the associated accelerator group for this window
 **/
GtkAccelGroup *
bonobo_window_get_accel_group (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	return win->priv->accel_group;
}

static BonoboWindowPrivate *
construct_priv (BonoboWindow *win)
{
	BonoboWindowPrivate *priv;
	GnomeDockItemBehavior behavior;

	priv = g_new0 (BonoboWindowPrivate, 1);

	priv->engine = bonobo_ui_engine_new ();

	priv->dock = GNOME_DOCK (gnome_dock_new ());
	gtk_container_add (GTK_CONTAINER (win),
			   GTK_WIDGET    (priv->dock));

	behavior = (GNOME_DOCK_ITEM_BEH_EXCLUSIVE
		    | GNOME_DOCK_ITEM_BEH_NEVER_VERTICAL);
	if (!gnome_preferences_get_menubar_detachable ())
		behavior |= GNOME_DOCK_ITEM_BEH_LOCKED;

	priv->menu_item = GNOME_DOCK_ITEM (gnome_dock_item_new (
		"menu", behavior));
	priv->menu      = GTK_MENU_BAR (gtk_menu_bar_new ());
	gtk_container_add (GTK_CONTAINER (priv->menu_item),
			   GTK_WIDGET    (priv->menu));
	gnome_dock_add_item (priv->dock, priv->menu_item,
			     GNOME_DOCK_TOP, 0, 0, 0, TRUE);

	/* 
	 * To have menubar relief agree with the toolbar (and have the relief outside of
	 * smaller handles), substitute the dock item's relief for the menubar's relief,
	 * but don't change the size of the menubar in the process. 
	 */
	gtk_menu_bar_set_shadow_type (GTK_MENU_BAR (priv->menu), GTK_SHADOW_NONE);
	if (gnome_preferences_get_menubar_relief ()) {
		guint border_width;

		gtk_container_set_border_width (GTK_CONTAINER (priv->menu_item), 2);
		border_width = GTK_CONTAINER (priv->menu)->border_width;
		if (border_width >= 2)
			border_width -= 2;
		gtk_container_set_border_width (GTK_CONTAINER (priv->menu), border_width);
	} else
		gnome_dock_item_set_shadow_type (GNOME_DOCK_ITEM (priv->menu_item), GTK_SHADOW_NONE);

	priv->main_vbox = gtk_vbox_new (FALSE, 0);
	gnome_dock_set_client_area (priv->dock, priv->main_vbox);

	priv->client_area = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->main_vbox), priv->client_area, TRUE, TRUE, 0);

	priv->status = GTK_BOX (gtk_hbox_new (FALSE, 0));
	gtk_box_pack_start (GTK_BOX (priv->main_vbox), GTK_WIDGET (priv->status), FALSE, FALSE, 0);

	priv->accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (win),
				    priv->accel_group);

	gtk_widget_show_all (GTK_WIDGET (priv->dock));
	gtk_widget_hide (GTK_WIDGET (priv->status));

	priv->sync_menu = bonobo_ui_sync_menu_new (
		priv->engine, priv->menu,
		GTK_WIDGET (priv->menu_item),
		priv->accel_group);

	bonobo_ui_engine_add_sync (priv->engine, priv->sync_menu);


	priv->sync_toolbar = bonobo_ui_sync_toolbar_new (
		priv->engine, GNOME_DOCK (priv->dock));

	bonobo_ui_engine_add_sync (priv->engine, priv->sync_toolbar);

	/* Keybindings; the gtk_binding stuff is just too evil */
	priv->sync_keys = bonobo_ui_sync_keys_new (priv->engine);
	bonobo_ui_engine_add_sync (priv->engine, priv->sync_keys);

	priv->sync_status = bonobo_ui_sync_status_new (
		priv->engine, priv->status);
	bonobo_ui_engine_add_sync (priv->engine, priv->sync_status);

	return priv;
}

/*
 *   To kill bug reports of hiding not working
 * we want to stop show_all showing hidden menus etc.
 */
static void
bonobo_window_show_all (GtkWidget *widget)
{
	BonoboWindow *win = BONOBO_WINDOW (widget);

	if (win->priv->client_area)
		gtk_widget_show_all (win->priv->client_area);

	gtk_widget_show (widget);
}

static gboolean
bonobo_window_key_press_event (GtkWidget *widget,
                               GdkEventKey *event)
{
	gboolean handled;
	BonoboUISync *sync;

	handled = GTK_WIDGET_CLASS (bonobo_window_parent_class)->key_press_event (widget, event);
	if (handled)
		return TRUE;

	sync = BONOBO_WINDOW (widget)->priv->sync_keys;
	if (sync)
		return bonobo_ui_sync_keys_binding_handle
			(widget, event, BONOBO_UI_SYNC_KEYS (sync));

	return FALSE;
}

static void
bonobo_window_class_init (BonoboWindowClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

	bonobo_window_parent_class =
		gtk_type_class (gtk_window_get_type ());

	object_class->finalize = bonobo_window_finalize;

	widget_class->show_all = bonobo_window_show_all;
	widget_class->key_press_event = bonobo_window_key_press_event;
}

static void
bonobo_window_init (BonoboWindow *win)
{
	win->priv = construct_priv (win);
	gnome_window_icon_set_from_default (GTK_WINDOW (win));
}

/**
 * bonobo_window_set_ui_container:
 * @win: 
 * @container: 
 * 
 * Deprecated
 **/
void
bonobo_window_set_ui_container (BonoboWindow *win,
				BonoboObject *container)
{
	g_return_if_fail (BONOBO_IS_WINDOW (win));

	g_warning ("bonobo_window_set_ui_container is deprecated");

	bonobo_ui_engine_set_ui_container (
		win->priv->engine, container);
}

/**
 * bonobo_window_set_name:
 * @win: the bonobo window
 * @win_name: the window name
 * 
 * Set the name of the window - used for configuration
 * serialization.
 **/
void
bonobo_window_set_name (BonoboWindow  *win,
			const char *win_name)
{
	BonoboWindowPrivate *priv;

	g_return_if_fail (BONOBO_IS_WINDOW (win));

	priv = win->priv;

	g_free (priv->name);
	g_free (priv->prefix);

	if (win_name) {
		priv->name = g_strdup (win_name);
		priv->prefix = g_strconcat ("/", win_name, "/", NULL);
	} else {
		priv->name = NULL;
		priv->prefix = g_strdup ("/");
	}
}

/**
 * bonobo_window_get_name:
 * @win: the bonobo window
 *
 * Gets the name of a window.
 * 
 * Return value: the name of the window
 **/
char *
bonobo_window_get_name (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);

	if (win->priv->name)
		return g_strdup (win->priv->name);
	else
		return NULL;
}

/**
 * bonobo_window_get_ui_engine:
 * @win: the bonobo window
 * 
 * Gets the associated UIEngine.
 *
 * Return value: the #BonoboUIEngine
 **/
BonoboUIEngine *
bonobo_window_get_ui_engine (BonoboWindow *win)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);
	g_return_val_if_fail (win->priv != NULL, NULL);

	return win->priv->engine;
}

/**
 * bonobo_window_construct:
 * @win: the window to construct
 * @win_name: the window name
 * @title: the window's title for the title bar
 * 
 * Construct a new BonoboWindow
 * 
 * Return value: a constructed window
 **/
GtkWidget *
bonobo_window_construct (BonoboWindow *win,
			 const char   *win_name,
			 const char   *title)
{
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	bonobo_window_set_name (win, win_name);

	if (title)
		gtk_window_set_title (GTK_WINDOW (win), title);

	return GTK_WIDGET (win);
}

/**
 * bonobo_window_new:
 * @win_name: the window name
 * @title: the window's title for the title bar
 * 
 * Return value: a new BonoboWindow
 **/
GtkWidget *
bonobo_window_new (const char   *win_name,
		   const char   *title)
{
	BonoboWindow *win;

	win = gtk_type_new (BONOBO_TYPE_WINDOW);

	return bonobo_window_construct (win, win_name, title);
}

/**
 * bonobo_window_get_type:
 *
 * Returns: The GtkType for the BonoboWindow class.
 */
GtkType
bonobo_window_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboWindow",
			sizeof (BonoboWindow),
			sizeof (BonoboWindowClass),
			(GtkClassInitFunc) bonobo_window_class_init,
			(GtkObjectInitFunc) bonobo_window_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_window_get_type (), &info);
	}

	return type;
}
