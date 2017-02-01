/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-selector-widget.c: Bonobo Component Selector widget
 *
 * Authors:
 *   Michael Meeks    (michael@helixcode.com)
 *   Richard Hestilow (hestgray@ionet.net)
 *   Miguel de Icaza  (miguel@kernel.org)
 *   Martin Baulig    (martin@
 *   Anders Carlsson  (andersca@gnu.org)
 *   Havoc Pennington (hp@redhat.com)
 *   Dietmar Maurer   (dietmar@maurer-it.com)
 *
 * Copyright 1999, 2000 Richard Hestilow, Helix Code, Inc,
 *                      Martin Baulig, Anders Carlsson,
 *                      Havoc Pennigton, Dietmar Maurer
 */
#include <config.h>
#include <string.h> /* strcmp */
#include <glib.h>
#include <libgnome/gnome-defs.h>
#define GNOME_EXPLICIT_TRANSLATION_DOMAIN PACKAGE
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-pixmap.h>
#include <libgnomeui/gnome-uidefs.h>
#include <bonobo/bonobo-selector-widget.h>
#include <bonobo/bonobo-object-directory.h>

#include "bonobo-insert-component.xpm"

#define GET_CLASS(o) BONOBO_SELECTOR_WIDGET_CLASS (GTK_OBJECT (o)->klass)

static GtkHBoxClass *parent_class;

enum {
	FINAL_SELECT,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };

struct _BonoboSelectorWidgetPrivate {
	GtkWidget    *clist;
	GtkWidget    *desc_label;
	GList        *servers;
};

static gint
server_list_compare (gconstpointer a, gconstpointer b)
{
	return strcmp (bonobo_directory_get_server_info_name ((ODServerInfo *)a),
		       bonobo_directory_get_server_info_name ((ODServerInfo *)b));

}

static GList *
get_filtered_objects (const gchar **interfaces_required)
{
	return g_list_sort (
		bonobo_directory_get_server_list (interfaces_required),
		server_list_compare);
}

static void
bonobo_selector_widget_finalize (GtkObject *object)
{
	g_return_if_fail (BONOBO_IS_SELECTOR_WIDGET (object));

	g_free (BONOBO_SELECTOR_WIDGET (object)->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static gchar *
impl_get_id (BonoboSelectorWidget *sel)
{
	GList *selection;
	gchar *text;
	BonoboSelectorWidgetPrivate *priv; 

	g_return_val_if_fail (sel != NULL, NULL);
	priv = sel->priv;	
	selection = GTK_CLIST (priv->clist)->selection;
	
	if (!selection)
		return NULL;

	gtk_clist_get_text (GTK_CLIST (priv->clist),
			    GPOINTER_TO_INT (selection->data),
			    1, &text);

	return g_strdup (text);	
}

/**
 * bonobo_selector_widget_get_id:
 * @sel: A BonoboSelectorWidget widget.
 *
 * Returns: A newly-allocated string containing the ID of the
 * currently-selected CORBA server (i.e., the corba server whose name
 * is highlighted in the list).  The user of this function is
 * responsible for freeing this. It will give an oaf iid back.
 */
gchar *
bonobo_selector_widget_get_id (BonoboSelectorWidget *sel)
{
	return GET_CLASS (sel)->get_id (sel);
}

static gchar *
impl_get_name (BonoboSelectorWidget *sel)
{
	GList *selection;
	gchar *text;
	BonoboSelectorWidgetPrivate *priv; 

	g_return_val_if_fail (sel != NULL, NULL);
	priv = sel->priv;	
	selection = GTK_CLIST (priv->clist)->selection;
	
	if (!selection)
		return NULL;

	gtk_clist_get_text (GTK_CLIST (priv->clist),
			    GPOINTER_TO_INT (selection->data),
			    0, &text);

	return g_strdup (text);
}

/**
 * bonobo_selector_widget_get_name:
 * @sel: A BonoboSelectorWidget widget.
 *
 * Returns: A newly-allocated string containing the name of the
 * currently-selected CORBA server (i.e., the corba server whose name
 * is highlighted in the list).  The user of this function is
 * responsible for freeing this.
 */
gchar *
bonobo_selector_widget_get_name (BonoboSelectorWidget *sel)
{
	return GET_CLASS (sel)->get_name (sel);
}

static gchar *
impl_get_description (BonoboSelectorWidget *sel)
{
	GList *selection;
	gchar *text;
	BonoboSelectorWidgetPrivate *priv; 

	g_return_val_if_fail (sel != NULL, NULL);
	priv = sel->priv;	
	selection = GTK_CLIST (priv->clist)->selection;
	
	if (!selection)
		return NULL;

	gtk_clist_get_text (GTK_CLIST (priv->clist),
			    GPOINTER_TO_INT (selection->data),
			    2, &text);

	return g_strdup (text);
}

/**
 * bonobo_selector_widget_get_description:
 * @sel: A BonoboSelectorWidget widget.
 *
 * Returns: A newly-allocated string containing the description of the
 * currently-selected CORBA server (i.e., the corba server whose name
 * is highlighted in the list).  The user of this function is
 * responsible for freeing this.
 */
gchar *
bonobo_selector_widget_get_description (BonoboSelectorWidget *sel)
{
	return GET_CLASS (sel)->get_description (sel);
}

static void
select_row (GtkCList *clist, gint row, gint col, 
	    GdkEvent *event, BonoboSelectorWidget *sel)
{
	if (event && event->type == GDK_2BUTTON_PRESS) {
		gtk_signal_emit (GTK_OBJECT (sel), signals [FINAL_SELECT],
				 NULL);

	} else {
		GtkCListClass *cl;
		gchar *text;
		
		gtk_clist_get_text (GTK_CLIST (clist), row,
				    2, &text);
		gtk_label_set_text (GTK_LABEL (sel->priv->desc_label), text);
		
		cl = gtk_type_class (GTK_TYPE_CLIST);

		if (cl->select_row)
			cl->select_row (clist, row, col, event);
	}
}

static void
bonobo_selector_widget_init (GtkWidget *widget)
{
	BonoboSelectorWidget *sel = BONOBO_SELECTOR_WIDGET (widget);
	GtkWidget *scrolled, *pixmap;
	GtkWidget *hbox;
	GtkWidget *frame;
	BonoboSelectorWidgetPrivate *priv;
	gchar *titles [] = { N_("Name"), "Description", "ID", NULL };
	
	g_return_if_fail (sel != NULL);

	titles [0] = gettext (titles [0]);
	priv = sel->priv = g_new0 (BonoboSelectorWidgetPrivate, 1);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	priv->clist = gtk_clist_new_with_titles (3, titles);
	gtk_clist_set_selection_mode (GTK_CLIST (priv->clist),
		GTK_SELECTION_BROWSE);
	gtk_signal_connect (GTK_OBJECT (priv->clist), "select-row",
			    GTK_SIGNAL_FUNC (select_row), sel);
	gtk_clist_set_column_visibility (GTK_CLIST (priv->clist), 1, FALSE);
	gtk_clist_set_column_visibility (GTK_CLIST (priv->clist), 2, FALSE);
	gtk_clist_column_titles_passive (GTK_CLIST (priv->clist));

	gtk_container_add (GTK_CONTAINER (scrolled), priv->clist);
	gtk_box_pack_start (GTK_BOX (sel), scrolled, TRUE, TRUE, 0);

	frame = gtk_frame_new (_("Description"));
	gtk_box_pack_start (GTK_BOX (sel), frame, FALSE, TRUE, 0);

	
	priv->desc_label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (priv->desc_label), 0.0, 0.5);
	gtk_label_set_line_wrap (GTK_LABEL (priv->desc_label), TRUE);
	gtk_label_set_justify (GTK_LABEL (priv->desc_label), GTK_JUSTIFY_LEFT);

	hbox = gtk_hbox_new (FALSE, 0);

	pixmap = gnome_pixmap_new_from_xpm_d (bonobo_insert_component_xpm);
	gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, TRUE, GNOME_PAD_SMALL);
	
	gtk_box_pack_start (GTK_BOX (hbox), priv->desc_label, TRUE, TRUE, GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), hbox);
	
	gtk_widget_set_usize (widget, 400, 300); 
	gtk_widget_show_all (widget);
}

static void
impl_set_interfaces (BonoboSelectorWidget *widget,
		     const char           **required_interfaces)
{
	GList *servers;
	BonoboSelectorWidgetPrivate *priv;
	
	g_return_if_fail (widget != NULL);
	
	priv = widget->priv;
	
	g_return_if_fail (priv->clist != NULL);
	
	gtk_clist_freeze (GTK_CLIST (priv->clist));

	gtk_clist_clear (GTK_CLIST (priv->clist));

	servers = get_filtered_objects (required_interfaces);
	
	if (servers) {
		GList *l;

		for (l = servers; l; l = l->next) {
			const gchar *text [4];

			text [0] = bonobo_directory_get_server_info_name (l->data);
			text [1] = bonobo_directory_get_server_info_id   (l->data);
			text [2] = bonobo_directory_get_server_info_description (l->data);
			text [3] = NULL;
			
			gtk_clist_append (GTK_CLIST (priv->clist), (gchar **) text);
		}
		bonobo_directory_free_server_list (servers);
	}

	gtk_clist_thaw (GTK_CLIST (priv->clist));
}

void
bonobo_selector_widget_set_interfaces (BonoboSelectorWidget *widget,
				       const char           **required_interfaces)
{
	GET_CLASS (widget)->set_interfaces (widget, required_interfaces);
}

/**
 * bonobo_selector_widget_new:
 *
 * Creates a new BonoboSelectorWidget widget, this contains
 * a CList and a description pane for each component.
 *
 * Returns: A pointer to the newly-created BonoboSelectorWidget widget.
 */
GtkWidget *
bonobo_selector_widget_new (void)
{
	return gtk_type_new (bonobo_selector_widget_get_type ());
}

static void
bonobo_selector_widget_class_init (BonoboSelectorWidgetClass *klass)
{
	GtkObjectClass *object_class;
	
	g_return_if_fail (klass != NULL);
	
	object_class = (GtkObjectClass *) klass;
	parent_class = gtk_type_class (gtk_vbox_get_type ());

	klass->get_id          = impl_get_id;
	klass->get_name        = impl_get_name;
	klass->get_description = impl_get_description;
	klass->set_interfaces  = impl_set_interfaces;

	signals [FINAL_SELECT] = gtk_signal_new (
		"final_select", GTK_RUN_FIRST, object_class->type,
		GTK_SIGNAL_OFFSET (BonoboSelectorWidgetClass, final_select),
		gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	object_class->finalize = bonobo_selector_widget_finalize;
}

/**
 * bonobo_selector_widget_get_type:
 *
 * Returns: The GtkType for the BonoboSelectorWidget object class.
 */
GtkType
bonobo_selector_widget_get_type (void)
{
	static guint bonobo_selector_widget_type = 0;

	if (!bonobo_selector_widget_type) {
		GtkTypeInfo bonobo_selector_widget_info = {
			"BonoboSelectorWidget",
			sizeof (BonoboSelectorWidget),
			sizeof (BonoboSelectorWidgetClass),
			(GtkClassInitFunc)  bonobo_selector_widget_class_init,
			(GtkObjectInitFunc) bonobo_selector_widget_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		bonobo_selector_widget_type = gtk_type_unique (
			gtk_vbox_get_type (),
			&bonobo_selector_widget_info);
	}

	return bonobo_selector_widget_type;
}
