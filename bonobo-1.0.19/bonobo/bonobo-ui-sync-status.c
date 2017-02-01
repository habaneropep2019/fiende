/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * bonobo-ui-sync-status.h: The Bonobo UI/XML sync engine for status bits.
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

#include <libgnomeui/gnome-uidefs.h>
#include <libgnomeui/gnome-preferences.h>

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-sync.h>
#include <bonobo/bonobo-ui-sync-status.h>

#include <bonobo/bonobo-ui-toolbar-separator-item.h>

static GtkObjectClass *parent_class = NULL;

#define PARENT_TYPE bonobo_ui_sync_get_type ()

#define HINT_KEY "BonoboWindow:hint"

static void
set_hint_cb (BonoboUIEngine   *engine,
	     const char       *str,
	     BonoboUISyncStatus *msync)
{
	guint id;

	if (msync->main_status) {
		id = gtk_statusbar_get_context_id (
			msync->main_status, HINT_KEY);

		gtk_statusbar_push (msync->main_status, id, str);
	}
}

static void
remove_hint_cb (BonoboUIEngine   *engine,
		BonoboUISyncStatus *msync)
{
	if (msync->main_status) {
		guint id;

		id = gtk_statusbar_get_context_id (
			msync->main_status, HINT_KEY);

		gtk_statusbar_pop (msync->main_status, id);
	}
}

static void
impl_bonobo_ui_sync_status_state (BonoboUISync     *sync,
				  BonoboUINode     *node,
				  BonoboUINode     *cmd_node,
				  GtkWidget        *widget,
				  GtkWidget        *parent)
{
	char             *name;
	BonoboUISyncStatus *msync = BONOBO_UI_SYNC_STATUS (sync);
		
	name = bonobo_ui_node_get_attr (node, "name");
	if (!name)
		return;

	if (!strcmp (name, "main")) {
		const char *id_str;

		id_str = bonobo_ui_engine_node_get_id (sync->engine, node);

		msync->main_status = GTK_STATUSBAR (widget);
			
		if (id_str) {
			guint id;
			char *txt;

			id = gtk_statusbar_get_context_id (
				msync->main_status, id_str);

			if ((txt = bonobo_ui_node_get_content (node))) {
				gboolean err;
				char    *status;

				status = bonobo_ui_util_decode_str (txt, &err);

				if (err)
					g_warning ("It looks like the status '%s' is not correctly "
						   "encoded, use bonobo_ui_component_set_status", txt);
				else
					gtk_statusbar_push (msync->main_status,
							    id, status);

				g_free (status);
			} else
				gtk_statusbar_pop (msync->main_status, id);

			bonobo_ui_node_free_string (txt);
		}
	}

	bonobo_ui_node_free_string (name);
}

static void
main_status_null (GtkObject        *dummy,
		  BonoboUISyncStatus *msync)
{
	msync->main_status = NULL;
}

/*
 * This function is to ensure that the status bar
 * does not ask for any space, but fills the free
 * horizontal space in the hbox.
 */
static void
clobber_request_cb (GtkWidget      *widget,
		    GtkRequisition *requisition,
		    gpointer        user_data)
{
	requisition->width = 1;
}

static GtkWidget *
impl_bonobo_ui_sync_status_build (BonoboUISync     *sync,
				  BonoboUINode     *node,
				  BonoboUINode     *cmd_node,
				  int              *pos,
				  GtkWidget        *parent)
{
	char *name;
	GtkWidget *widget = NULL;
	BonoboUISyncStatus *msync = BONOBO_UI_SYNC_STATUS (sync);
		
	name = bonobo_ui_node_get_attr (node, "name");
	if (!name)
		return NULL;

	if (!strcmp (name, "main")) {

		widget = gtk_statusbar_new ();

		gtk_signal_connect (GTK_OBJECT (widget),
				    "size_request",
				    clobber_request_cb, NULL);

		msync->main_status = GTK_STATUSBAR (widget);

		gtk_signal_connect (GTK_OBJECT (widget), "destroy",
				    (GtkSignalFunc) main_status_null, msync);

		/* insert a little padding so text isn't jammed against frame */
		gtk_misc_set_padding (
			GTK_MISC (GTK_STATUSBAR (widget)->label),
			GNOME_PAD, 0);
		gtk_widget_show (GTK_WIDGET (widget));

		gtk_box_pack_start (GTK_BOX (parent), widget, TRUE, TRUE, 0);
			
	} else if (bonobo_ui_node_has_name (node, "control")) {

		widget = bonobo_ui_engine_build_control (sync->engine, node);

		if (widget)
			gtk_box_pack_end (GTK_BOX (parent), widget,
					  FALSE, FALSE, 0);
	}
	bonobo_ui_node_free_string (name);

	if (widget)
		gtk_box_reorder_child (msync->status, widget, (*pos)++);

	return widget;
}

static GtkWidget *
impl_bonobo_ui_sync_status_build_placeholder (BonoboUISync     *sync,
					    BonoboUINode     *node,
					    BonoboUINode     *cmd_node,
					    int              *pos,
					    GtkWidget        *parent)
{
	GtkWidget *widget;
	BonoboUISyncStatus *msync = BONOBO_UI_SYNC_STATUS (sync);

	g_warning ("TESTME: status bar placeholders");

	widget = bonobo_ui_toolbar_separator_item_new ();
	gtk_widget_set_sensitive (widget, FALSE);

	gtk_box_pack_end (GTK_BOX (parent), widget,
			  FALSE, FALSE, 0);

	if (widget)
		gtk_box_reorder_child (msync->status, widget, (*pos)++);

	return widget;
}

static GList *
box_get_children_in_order (GtkBox *box)
{
	GList       *ret = NULL;
	GList       *l;

	g_return_val_if_fail (GTK_IS_BOX (box), NULL);

	for (l = box->children; l; l = l->next) {
		GtkBoxChild *child = l->data;

		ret = g_list_prepend (ret, child->widget);
	}

	return g_list_reverse (ret);
}

static void
impl_bonobo_ui_sync_status_stamp_root (BonoboUISync *sync)
{
	BonoboUISyncStatus *sstatus = BONOBO_UI_SYNC_STATUS (sync);
	BonoboUINode       *node;
	GtkWidget          *widget;

	node = bonobo_ui_engine_get_path (sync->engine, "/status");

	if (node) {
		widget = GTK_WIDGET (sstatus->status);

		bonobo_ui_engine_stamp_root (sync->engine, node, widget);

		bonobo_ui_sync_do_show_hide (sync, node, NULL, widget);
	}
}

static GList *
impl_bonobo_ui_sync_status_get_widgets (BonoboUISync *sync,
					BonoboUINode *node)
{
	if (bonobo_ui_node_has_name (node, "status"))
		return box_get_children_in_order (
			GTK_BOX (BONOBO_UI_SYNC_STATUS (sync)->status));
	else
		return NULL;
}

static void
impl_destroy (GtkObject *object)
{
	parent_class->destroy (object);
}

static gboolean
impl_bonobo_ui_sync_status_can_handle (BonoboUISync *sync,
				       BonoboUINode *node)
{
	return bonobo_ui_node_has_name (node, "status");
}

/* We need to map the shell to the item */

static void
class_init (BonoboUISyncClass *sync_class)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class (BONOBO_TYPE_UI_SYNC);

	object_class = GTK_OBJECT_CLASS (sync_class);
	object_class->destroy  = impl_destroy;

	sync_class->sync_state = impl_bonobo_ui_sync_status_state;
	sync_class->build      = impl_bonobo_ui_sync_status_build;
	sync_class->build_placeholder = impl_bonobo_ui_sync_status_build_placeholder;

	sync_class->get_widgets   = impl_bonobo_ui_sync_status_get_widgets;
	sync_class->stamp_root    = impl_bonobo_ui_sync_status_stamp_root;

	sync_class->can_handle    = impl_bonobo_ui_sync_status_can_handle;
}

static void
init (BonoboUISyncStatus *msync)
{
}

GtkType
bonobo_ui_sync_status_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUISyncStatus",
			sizeof (BonoboUISyncStatus),
			sizeof (BonoboUISyncStatusClass),
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
bonobo_ui_sync_status_new (BonoboUIEngine *engine,
			   GtkBox         *status)
{
	BonoboUISyncStatus *sync;

	g_return_val_if_fail (BONOBO_IS_UI_ENGINE (engine), NULL);

	sync = gtk_type_new (BONOBO_TYPE_UI_SYNC_STATUS);

	sync->status = status;

	gtk_signal_connect (GTK_OBJECT (engine), "add_hint",
			    (GtkSignalFunc) set_hint_cb, sync);

	gtk_signal_connect (GTK_OBJECT (engine), "remove_hint",
			    (GtkSignalFunc) remove_hint_cb, sync);

	return bonobo_ui_sync_construct (
		BONOBO_UI_SYNC (sync), engine, FALSE, TRUE);
}
