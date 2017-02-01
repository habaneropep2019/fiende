/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * bonobo-ui-sync-menu.h: The Bonobo UI/XML sync engine for menus
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include <config.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libgnomeui/gtkpixmapmenuitem.h>
#include <libgnomeui/gnome-preferences.h>

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-sync.h>
#include <bonobo/bonobo-ui-sync-menu.h>

#undef WIDGET_SYNC_DEBUG

static GtkObjectClass *parent_class = NULL;

#define PARENT_TYPE bonobo_ui_sync_get_type ()

/* Doesn't the GtkRadioMenuItem API suck badly ! */
#define MAGIC_RADIO_GROUP_KEY "Bonobo::RadioGroupName"
#define UI_SYNC_MENU_KEY "Bonobo::UISyncMenu"

typedef struct {
	GtkMenu          *menu;
	char             *path;
} Popup;

static void
popup_remove (BonoboUISyncMenu *smenu,
	      Popup            *popup)
{
	BonoboUINode *node;

	g_return_if_fail (smenu != NULL);
	g_return_if_fail (popup != NULL);

	gtk_signal_disconnect_by_data (GTK_OBJECT (popup->menu), popup);

	node = bonobo_ui_engine_get_path (smenu->parent.engine,
					  popup->path);

	if (node)
		bonobo_ui_engine_prune_widget_info (
			smenu->parent.engine, node, TRUE);

	smenu->popups = g_slist_remove (
		smenu->popups, popup);
	
	g_free (popup->path);
	g_free (popup);
}

void
bonobo_ui_sync_menu_remove_popup (BonoboUISyncMenu *sync,
				  const char       *path)
{
	GSList *l, *next;

	g_return_if_fail (path != NULL);
	g_return_if_fail (BONOBO_IS_UI_SYNC_MENU (sync));

	for (l = sync->popups; l; l = next) {
		Popup *popup = l->data;

		next = l->next;
		if (!strcmp (popup->path, path))
			popup_remove (sync, popup);
	}
}

static void
popup_destroy (GtkObject *menu, Popup *popup)
{
	BonoboUISyncMenu *smenu = gtk_object_get_data (
		GTK_OBJECT (menu), UI_SYNC_MENU_KEY);

	g_return_if_fail (smenu != NULL);
	popup_remove (smenu, popup);
}

static gboolean
node_is_popup (BonoboUINode *node)
{
	if (!node)
		return FALSE;

	if (bonobo_ui_node_has_name (node, "popup"))
		return TRUE;
	
	else if (bonobo_ui_node_has_name (node, "menu"))
		return FALSE;

	else
		return node_is_popup (bonobo_ui_node_parent (node));
}

static void
add_tearoff (BonoboUINode *node, GtkMenu *menu, gboolean popup_init)
{
	GtkWidget    *tearoff;
	char         *txt;
	gboolean      has_tearoff;

	has_tearoff = gnome_preferences_get_menus_have_tearoff ();

	if (node) {
		txt = bonobo_ui_node_get_attr (node, "tearoff");

		if (txt)
			has_tearoff = atoi (txt);
		else if (node_is_popup (node))
			has_tearoff = FALSE;

		bonobo_ui_node_free_string (txt);

	} else if (popup_init)
		has_tearoff = FALSE;

	/*
	 * Create the tearoff item at the beginning of the menu shell,
	 * if appropriate.
	 */
	if (has_tearoff) {
		tearoff = gtk_tearoff_menu_item_new ();
		gtk_widget_show (tearoff);
		gtk_menu_prepend (GTK_MENU (menu), tearoff);
	}
}

void
bonobo_ui_sync_menu_add_popup (BonoboUISyncMenu *smenu,
			       GtkMenu          *menu,
			       const char       *path)
{
	Popup        *popup;
	BonoboUINode *node;
	GList        *children;

	g_return_if_fail (path != NULL);
	g_return_if_fail (GTK_IS_MENU (menu));
	g_return_if_fail (BONOBO_IS_UI_SYNC_MENU (smenu));

	bonobo_ui_sync_menu_remove_popup (smenu, path);

	popup       = g_new (Popup, 1);
	popup->menu = menu;
	popup->path = g_strdup (path);

	if ((children = gtk_container_children (GTK_CONTAINER (menu)))) {
		g_warning ("Extraneous items in blank popup");
		g_list_free (children);
	}

	add_tearoff (bonobo_ui_engine_get_path (
		smenu->parent.engine, path), menu, TRUE);

	smenu->popups = g_slist_prepend (smenu->popups, popup);

	gtk_object_set_data (GTK_OBJECT (menu), UI_SYNC_MENU_KEY, smenu);

	gtk_signal_connect (GTK_OBJECT (menu), "destroy",
			    (GtkSignalFunc) popup_destroy, popup);

	node = bonobo_ui_engine_get_path (smenu->parent.engine, path);

	bonobo_ui_engine_dirty_tree (smenu->parent.engine, node);
}

static void
radio_group_remove (GtkRadioMenuItem *menuitem,
		    char             *group_name)
{
	GtkRadioMenuItem *master;
	char             *orig_key;
	GSList           *l;
	BonoboUISyncMenu *menu_sync =
		gtk_object_get_data (GTK_OBJECT (menuitem),
				     MAGIC_RADIO_GROUP_KEY);

	if (!g_hash_table_lookup_extended
	    (menu_sync->radio_groups, group_name, (gpointer *)&orig_key,
	     (gpointer *)&master)) {
		g_warning ("Radio group hash inconsistancy");
		return;
	}
	
	l = master->group;
	while (l && l->data == menuitem)
		l = l->next;
	
	g_hash_table_remove (menu_sync->radio_groups, group_name);
	g_free (orig_key);

	if (l) { /* Entries left in group */
		g_hash_table_insert (menu_sync->radio_groups,
				     group_name, l->data);
	} else /* alloced in signal_connect; grim hey */
		g_free (group_name);
}

static void
radio_group_add (BonoboUISyncMenu *menu_sync,
		 GtkRadioMenuItem *menuitem,
		 const char       *group_name)
{
	GtkRadioMenuItem *master;

	g_return_if_fail (menuitem != NULL);
	g_return_if_fail (menu_sync != NULL);
	g_return_if_fail (group_name != NULL);

	if (!(master = g_hash_table_lookup (menu_sync->radio_groups, group_name))) {
		g_hash_table_insert (menu_sync->radio_groups, g_strdup (group_name),
				     menuitem);
	} else {
		gtk_radio_menu_item_set_group (
			menuitem, gtk_radio_menu_item_group (master));
		
		/* 
		 * Since we created this item without a group, it's
		 * active, but now we are adding it to a group so it
		 * should not be active.
		 */
		GTK_CHECK_MENU_ITEM (menuitem)->active = FALSE;
	}

	gtk_object_set_data (GTK_OBJECT (menuitem),
			     MAGIC_RADIO_GROUP_KEY, menu_sync);

	gtk_signal_connect (GTK_OBJECT (menuitem), "destroy",
			    (GtkSignalFunc) radio_group_remove,
			    g_strdup (group_name));
}

static GtkWidget *
get_item_widget (GtkWidget *widget)
{
	if (!widget)
		return NULL;

	if (GTK_IS_MENU (widget))
		return gtk_menu_get_attach_widget (
			GTK_MENU (widget));

	return NULL;
}

static GtkWidget *
cmd_get_menu_pixmap (BonoboUINode     *node,
		     BonoboUINode     *cmd_node)
{
	GtkWidget *pixmap;
	char      *type;

	if ((type = bonobo_ui_node_get_attr (node, "pixtype"))) {
		pixmap = bonobo_ui_util_xml_get_icon_pixmap_widget (node, TRUE);
		bonobo_ui_node_free_string (type);
		return pixmap;
	}

	if ((type = bonobo_ui_node_get_attr (cmd_node, "pixtype"))) {
		pixmap = bonobo_ui_util_xml_get_icon_pixmap_widget (cmd_node, TRUE);
		bonobo_ui_node_free_string (type);
		return pixmap;
	}

	return NULL;
}

/*
 * The second parameter uses underscores to escape keyboard
 * shortcuts. We ignore the first underscore, but if there
 * are two in a row, they are treated as if there was one.
 */
static gboolean
str_uscore_equal (const char *plain, const char *scored)
{
	while (*plain || *scored) {
		if (*scored == '_') {
			scored++;

			/*
			 * Must check for badly formed string
			 * (ending with single underscore)
			 * or risk reading off end.
			 */
			if (!*scored)
				return FALSE;
		}

		if (*scored != *plain)
			return FALSE;
		else {
			scored++;
			plain++;
		}
	}

	return TRUE;
}

static gboolean
label_same (GtkBin *menu_widget, const char *txt)
{
	GtkWidget *label;

	return menu_widget &&
		(label = menu_widget->child) &&
		GTK_IS_ACCEL_LABEL (label) &&
		((GtkLabel *)label)->label &&
		str_uscore_equal (((GtkLabel *)label)->label, txt);
}

static void
impl_bonobo_ui_sync_menu_state (BonoboUISync *sync,
				BonoboUINode *node,
				BonoboUINode *cmd_node,
				GtkWidget    *widget,
				GtkWidget    *parent)
{
	GtkWidget        *menu_widget;
	BonoboUISyncMenu *sync_menu = BONOBO_UI_SYNC_MENU (sync);
	BonoboUIEngine   *engine = sync->engine;
	char             *type, *txt, *label_attr;

	g_return_if_fail (parent != NULL);

	if (bonobo_ui_node_has_name (node, "placeholder") ||
	    bonobo_ui_node_has_name (node, "separator")) {

		bonobo_ui_engine_queue_update (
			engine, widget, node, cmd_node);
		return;
	}

	if (bonobo_ui_node_has_name (node, "submenu")) {
		menu_widget = get_item_widget (widget);
		if (!menu_widget)
			menu_widget = widget;

		/* Recurse here just once, don't duplicate in the build. */
		bonobo_ui_engine_update_node (engine, node);

	} else if (bonobo_ui_node_has_name (node, "menuitem"))
		menu_widget = widget;
	else
		return;

	label_attr = bonobo_ui_engine_get_attr (node, cmd_node, "label");

	if ((type = bonobo_ui_engine_get_attr (node, cmd_node, "type")))
		bonobo_ui_node_free_string (type);
	else {
		if (GTK_IS_PIXMAP_MENU_ITEM (menu_widget)) {
			GtkWidget *pixmap;
			GtkPixmapMenuItem *gack = GTK_PIXMAP_MENU_ITEM (menu_widget);

			if (gnome_preferences_get_menus_have_icons () ||
			    label_attr == NULL)
				pixmap = cmd_get_menu_pixmap (node, cmd_node);
			else
				pixmap = NULL;

			if (pixmap) {
				/* Since this widget sucks we must claw inside its guts */
				if (gack->pixmap) {
					gtk_widget_destroy (gack->pixmap);
					gack->pixmap = NULL;
				}
				gtk_widget_show (GTK_WIDGET (pixmap));
				gtk_pixmap_menu_item_set_pixmap (
					GTK_PIXMAP_MENU_ITEM (menu_widget),
					GTK_WIDGET (pixmap));
			}
		}
	}

	if (label_attr) {
		GtkWidget *label;
		guint      keyval;
		gboolean   err;

		txt = bonobo_ui_util_decode_str (label_attr, &err);
		if (err) {
			g_warning ("Encoding error in label on '%s', you probably forgot to "
				   "put an '_' before label in your xml file",
				   bonobo_ui_xml_make_path (node));
			return;
		}

		if (!label_same (GTK_BIN (menu_widget), txt)) {
			label = gtk_accel_label_new (txt);

			/* Setup the widget. */
			gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
			gtk_widget_show (label);
			
			/*
			 * Insert it into the menu item widget and setup the
			 * accelerator. FIXME: rather inefficient.
			 */
			if (GTK_BIN (menu_widget)->child)
				gtk_widget_destroy (GTK_BIN (menu_widget)->child);
			
			gtk_container_add (GTK_CONTAINER (menu_widget), label);
			gtk_accel_label_set_accel_widget (
				GTK_ACCEL_LABEL (label), menu_widget);
			
			keyval = gtk_label_parse_uline (GTK_LABEL (label), txt);
			
			if (keyval != GDK_VoidSymbol) {
				if (GTK_IS_MENU (parent))
					gtk_widget_add_accelerator (
						menu_widget, "activate_item",
						gtk_menu_ensure_uline_accel_group (
							GTK_MENU (parent)),
						keyval, 0, 0);
				
				else if (GTK_IS_MENU_BAR (parent) &&
					 sync_menu->accel_group != NULL)
					gtk_widget_add_accelerator (
						menu_widget, "activate_item",
						sync_menu->accel_group,
						keyval, GDK_MOD1_MASK, 0);
				else
					g_warning ("Adding accelerator went bananas");
			}
		} /* else
			g_warning ("No change in label '%s'", txt); */

		g_free (txt);
	}

	bonobo_ui_node_free_string (label_attr);
	
	if ((txt = bonobo_ui_engine_get_attr (node, cmd_node, "accel"))) {
		guint           key;
		GdkModifierType mods;

/*		fprintf (stderr, "Accel name is afterwards '%s'\n", text); */
		bonobo_ui_util_accel_parse (txt, &key, &mods);
		bonobo_ui_node_free_string (txt);

		if (!key) /* FIXME: this looks strange */
			return;

		gtk_widget_add_accelerator (menu_widget,
					    "activate",
					    sync_menu->accel_group,
					    key, mods,
					    GTK_ACCEL_VISIBLE);
	}

	bonobo_ui_engine_queue_update (
		engine, menu_widget, node, cmd_node);
}

static void
put_hint_in_statusbar (GtkWidget      *menuitem,
		       BonoboUIEngine *engine)
{
	BonoboUINode *node;
	BonoboUINode *cmd_node;
	char *hint, *txt;
	gboolean err;

	g_return_if_fail (engine != NULL);

	node = bonobo_ui_engine_widget_get_node (menuitem);
	g_return_if_fail (node != NULL);

	cmd_node = bonobo_ui_engine_get_cmd_node (engine, node);

	hint = bonobo_ui_engine_get_attr (node, cmd_node, "tip");

/*	g_warning ("Getting tooltip on '%s', '%s' : '%s'",
		   bonobo_ui_xml_make_path (node),
		   cmd_node ? bonobo_ui_xml_make_path (cmd_node) : "no cmd",
		   hint);*/
	if (!hint)
		return;

	txt = bonobo_ui_util_decode_str (hint, &err);
	if (err) {
		g_warning ("Encoding error in tip on '%s', you probably forgot to "
			   "put an '_' before tip in your xml file",
			   bonobo_ui_xml_make_path (node));

	} else
		bonobo_ui_engine_add_hint (engine, txt);

	g_free (txt);

	bonobo_ui_node_free_string (hint);
}

static void
remove_hint_from_statusbar (GtkWidget      *menuitem,
			    BonoboUIEngine *engine)
{
	bonobo_ui_engine_remove_hint (engine);
}

static gint
exec_verb_cb (GtkWidget *item, BonoboUIEngine *engine)
{
	bonobo_ui_engine_emit_verb_on_w (engine, GTK_WIDGET (item));

	return FALSE;
}

static gint
menu_toggle_emit_ui_event (GtkCheckMenuItem *item,
			   BonoboUIEngine   *engine)
{
	const char *state;

	if (item->active)
		state = "1";
	else
		state = "0";

	bonobo_ui_engine_emit_event_on_w (
		engine, GTK_WIDGET (item), state);

	return FALSE;
}

static gint
sucking_gtk_keybindings_cb (GtkWidget   *widget,
			    GdkEventKey *event,
			    gpointer     dummy)
{
	static GtkWidgetClass *klass = NULL;
	static guint           id = 0;
	gboolean               ret;

	if (!klass)
		klass = gtk_type_class (GTK_TYPE_MENU_SHELL);
	if (!id)
		id = gtk_signal_lookup ("key_press_event",
					GTK_TYPE_WIDGET);

	if (klass->key_press_event (widget, event))
		ret = TRUE;
	else
		ret = FALSE;

	gtk_signal_emit_stop (GTK_OBJECT (widget), id);
	
	return ret;
}

static GtkWidget *
impl_bonobo_ui_sync_menu_build (BonoboUISync     *sync,
				BonoboUINode     *node,
				BonoboUINode     *cmd_node,
				int              *pos,
				GtkWidget        *parent)
{
	BonoboUIEngine *engine = sync->engine;
	BonoboUISyncMenu *menu_sync = BONOBO_UI_SYNC_MENU (sync);
	GtkWidget      *menu_widget = NULL;
	GtkWidget      *ret_widget;
	char           *type;

	if (!parent) /* A popup without a GtkMenu inserted as yet. */
		return NULL;

	if (bonobo_ui_node_has_name (node, "separator")) {

		menu_widget = gtk_menu_item_new ();
		gtk_widget_set_sensitive (menu_widget, FALSE);

	} else if (bonobo_ui_node_has_name (node, "control")) {

		GtkWidget *control = bonobo_ui_engine_build_control (
			engine, node);

		if (!control)
			return NULL;

		else if (!GTK_IS_MENU_ITEM (control)) {
			menu_widget = gtk_menu_item_new ();
			gtk_container_add (GTK_CONTAINER (menu_widget),
					   control);
		} else
			menu_widget = control;

	} else if (bonobo_ui_node_has_name (node, "menuitem") ||
		   bonobo_ui_node_has_name (node, "submenu")) {

		/* Create menu item */
		if ((type = bonobo_ui_engine_get_attr (node, cmd_node, "type"))) {
			if (!strcmp (type, "radio")) {
				char *group = bonobo_ui_engine_get_attr (node, cmd_node, "group");

				menu_widget = gtk_radio_menu_item_new (NULL);

				if (group)
					radio_group_add (
						BONOBO_UI_SYNC_MENU (sync),
						GTK_RADIO_MENU_ITEM (menu_widget), group);

				bonobo_ui_node_free_string (group);
			} else if (!strcmp (type, "toggle"))
				menu_widget = gtk_check_menu_item_new ();
			
			else
				menu_widget = NULL;
			
			gtk_check_menu_item_set_show_toggle (
				GTK_CHECK_MENU_ITEM (menu_widget), TRUE);

			gtk_signal_connect (GTK_OBJECT (menu_widget), "toggled",
					    (GtkSignalFunc) menu_toggle_emit_ui_event,
					    engine);

			bonobo_ui_node_free_string (type);
		} else {
			char *txt;
			
			/* FIXME: why not always create pixmap menu items ? */
			if ((txt = bonobo_ui_engine_get_attr (node, cmd_node, "pixtype")) &&
			    gnome_preferences_get_menus_have_icons ()) {
				gtk_widget_push_visual (gdk_rgb_get_visual ());
				gtk_widget_push_colormap (gdk_rgb_get_cmap ());

				menu_widget = gtk_pixmap_menu_item_new ();

				gtk_widget_pop_visual ();
				gtk_widget_pop_colormap ();
			} else
				menu_widget = gtk_menu_item_new ();

			bonobo_ui_node_free_string (txt);
		}

		if (!menu_widget)
			return NULL;
			
		gtk_signal_connect (GTK_OBJECT (menu_widget),
				    "select",
				    GTK_SIGNAL_FUNC (put_hint_in_statusbar),
				    engine);

		gtk_signal_connect (GTK_OBJECT (menu_widget),
				    "deselect",
				    GTK_SIGNAL_FUNC (remove_hint_from_statusbar),
				    engine);
	}

	if (!menu_widget)
		return NULL;	

	if (bonobo_ui_node_has_name (node, "submenu")) {
		GtkMenuShell *shell;
		GtkMenu      *menu;
		
		shell = GTK_MENU_SHELL (parent);
		gtk_signal_connect (GTK_OBJECT (shell), "key_press_event",
				    (GtkSignalFunc) sucking_gtk_keybindings_cb, NULL);


		/* Create the menu shell. */
		menu = GTK_MENU (gtk_menu_new ());
		gtk_signal_connect (GTK_OBJECT (menu), "key_press_event",
				    (GtkSignalFunc) sucking_gtk_keybindings_cb, NULL);

		gtk_menu_set_accel_group (menu, menu_sync->accel_group);

		add_tearoff (node, GTK_MENU (menu), FALSE);

		/*
		 * Associate this menu shell with the menu item for
		 * this submenu.
		 */
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_widget),
					   GTK_WIDGET (menu));

		/* We don't recurse here, it is done once in the state set */

		gtk_widget_show (GTK_WIDGET (menu));
		gtk_widget_show (GTK_WIDGET (shell));

		ret_widget = GTK_WIDGET (menu);
	} else
		ret_widget = menu_widget;

	if (!GTK_IS_CHECK_MENU_ITEM (menu_widget))
		gtk_signal_connect (GTK_OBJECT (menu_widget), "activate",
				    (GtkSignalFunc) exec_verb_cb, engine);

	gtk_signal_connect (GTK_OBJECT (menu_widget), "key_press_event",
			    (GtkSignalFunc) sucking_gtk_keybindings_cb, NULL);

	gtk_widget_show (menu_widget);
			    
	gtk_menu_shell_insert (GTK_MENU_SHELL (parent),
			       menu_widget, (*pos)++);

	return ret_widget;
}

static GtkWidget *
impl_bonobo_ui_sync_menu_build_placeholder (BonoboUISync     *sync,
					    BonoboUINode     *node,
					    BonoboUINode     *cmd_node,
					    int              *pos,
					    GtkWidget        *parent)
{
	GtkWidget *widget;

	if (!parent) /* A popup without a GtkMenu inserted as yet. */
		return NULL;

	widget = gtk_menu_item_new ();
	gtk_widget_set_sensitive (widget, FALSE);

	gtk_menu_shell_insert (GTK_MENU_SHELL (parent),
			       GTK_WIDGET (widget), (*pos)++);

	return widget;
}

static GList *
impl_bonobo_ui_sync_menu_get_widgets (BonoboUISync *sync,
				      BonoboUINode *node)
{
	GtkWidget *widget;

	widget = bonobo_ui_engine_node_get_widget (sync->engine, node);

	if (widget)
		return gtk_container_children (GTK_CONTAINER (widget));
	else
		return NULL; /* A popup child with no GtkMenu yet */
}

static void
impl_bonobo_ui_sync_menu_state_update (BonoboUISync *sync,
				       GtkWidget    *widget,
				       const char   *new_state)
{
	if (GTK_IS_CHECK_MENU_ITEM (widget)) {
#ifdef STATE_SYNC_DEBUG
		g_warning ("Setting check menu item '%p' to '%s'",
			   widget, state);
#endif
		gtk_check_menu_item_set_active (
			GTK_CHECK_MENU_ITEM (widget), 
			atoi (new_state));
	} else
		g_warning ("TESTME: strange, setting "
			   "state '%s' on weird object '%s'",
			   new_state, gtk_type_name (GTK_OBJECT (
				   widget)->klass->type));
}

static gboolean
radio_group_destroy (gpointer	key,
		     gpointer	value,
		     gpointer	user_data)
{
	g_free (key);

	return TRUE;
}

static void
impl_destroy (GtkObject *object)
{
	BonoboUISyncMenu *sync;

	sync = BONOBO_UI_SYNC_MENU (object);

	g_hash_table_foreach_remove (sync->radio_groups,
				     radio_group_destroy, NULL);
	g_hash_table_destroy (sync->radio_groups);
	sync->radio_groups = NULL;

	parent_class->destroy (object);
}

static gboolean
impl_bonobo_ui_sync_menu_ignore_widget (BonoboUISync *sync,
					GtkWidget    *widget)
{
	return GTK_IS_TEAROFF_MENU_ITEM (widget);
}

static void
impl_bonobo_ui_sync_menu_update_root (BonoboUISync *sync,
				      BonoboUINode *root)
{
	BonoboUISyncMenu *smenu = BONOBO_UI_SYNC_MENU (sync);

	if (bonobo_ui_node_has_name (root, "menu") &&
	    smenu->menu_dock_item)
		bonobo_ui_sync_do_show_hide (
			sync, root, NULL,
			smenu->menu_dock_item);
}

static void
impl_bonobo_ui_sync_menu_stamp_root (BonoboUISync *sync)
{
	BonoboUISyncMenu *smenu = BONOBO_UI_SYNC_MENU (sync);
	BonoboUINode     *node;
	GSList           *l;

#ifdef WIDGET_SYNC_DEBUG
	fprintf (stderr, "Stamping menu sync's roots\n");
#endif

	node = bonobo_ui_engine_get_path (sync->engine, "/menu");
	if (smenu->menu) {
		GtkWidget *widget = GTK_WIDGET (smenu->menu);

		bonobo_ui_engine_stamp_root (sync->engine, node, widget);
		bonobo_ui_sync_do_show_hide (sync, node, NULL, widget);
	}

	for (l = smenu->popups; l; l = l->next) {
		Popup *popup = l->data;

		if ((node = bonobo_ui_engine_get_path (sync->engine,
						       popup->path))) {
#ifdef WIDGET_SYNC_DEBUG
			fprintf (stderr, "Stamping popup root '%s(%p)' with '%p'\n",
				 popup->path, node, popup->menu);
#endif
			bonobo_ui_engine_stamp_root (sync->engine, node,
						     GTK_WIDGET (popup->menu));
		} else
			g_warning ("Can't find path '%s' for popup widget",
				   popup->path);
	}

	if ((node = bonobo_ui_engine_get_path (sync->engine, "/popups")))
		bonobo_ui_engine_node_set_dirty (sync->engine, node, FALSE);
}

static gboolean
impl_bonobo_ui_sync_menu_can_handle (BonoboUISync *sync,
				     BonoboUINode *node)
{
	return bonobo_ui_node_has_name (node, "menu") ||
	       bonobo_ui_node_has_name (node, "popups");
}

/* We need to map the shell to the item */

static GtkWidget *
impl_bonobo_ui_sync_menu_get_attached (BonoboUISync *sync,
				       GtkWidget    *widget,
				       BonoboUINode *node)
{
	return get_item_widget (widget);
}

static void
class_init (BonoboUISyncClass *sync_class)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class (BONOBO_TYPE_UI_SYNC);

	object_class = GTK_OBJECT_CLASS (sync_class);
	object_class->destroy  = impl_destroy;

	sync_class->sync_state = impl_bonobo_ui_sync_menu_state;
	sync_class->build      = impl_bonobo_ui_sync_menu_build;
	sync_class->build_placeholder = impl_bonobo_ui_sync_menu_build_placeholder;

	sync_class->get_widgets   = impl_bonobo_ui_sync_menu_get_widgets;
	sync_class->ignore_widget = impl_bonobo_ui_sync_menu_ignore_widget;
	sync_class->update_root   = impl_bonobo_ui_sync_menu_update_root;

	sync_class->state_update  = impl_bonobo_ui_sync_menu_state_update;
	sync_class->stamp_root    = impl_bonobo_ui_sync_menu_stamp_root;
	sync_class->can_handle    = impl_bonobo_ui_sync_menu_can_handle;

	sync_class->get_attached  = impl_bonobo_ui_sync_menu_get_attached;
}

static void
init (BonoboUISyncMenu *sync)
{
	sync->radio_groups = g_hash_table_new (
		g_str_hash, g_str_equal);
}

GtkType
bonobo_ui_sync_menu_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUISyncMenu",
			sizeof (BonoboUISyncMenu),
			sizeof (BonoboUISyncMenuClass),
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
bonobo_ui_sync_menu_new (BonoboUIEngine *engine,
			 GtkMenuBar     *menu,
			 GtkWidget      *menu_dock_item,
			 GtkAccelGroup  *group)
{
	BonoboUISyncMenu *sync;

	g_return_val_if_fail (BONOBO_IS_UI_ENGINE (engine), NULL);

	sync = gtk_type_new (BONOBO_TYPE_UI_SYNC_MENU);

	sync->menu           = menu;
	sync->menu_dock_item = menu_dock_item;
	sync->accel_group    = group;

	return bonobo_ui_sync_construct (
		BONOBO_UI_SYNC (sync), engine, TRUE, TRUE);
}
