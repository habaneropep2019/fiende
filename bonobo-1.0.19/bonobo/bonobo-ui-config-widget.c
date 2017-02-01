/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-config-widget.c: Bonobo Component UIConfig widget
 *
 * Authors:
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2001 Helix Code, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>

#include <libgnome/gnome-defs.h>

#define GNOME_EXPLICIT_TRANSLATION_DOMAIN PACKAGE
#include <libgnome/gnome-i18n.h>

#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-engine-private.h>
#include <bonobo/bonobo-ui-config-widget.h>
#include <bonobo/bonobo-ui-sync-toolbar.h>

#define PARENT_TYPE gtk_vbox_get_type ()

static GtkObjectClass *parent_class;

struct _BonoboUIConfigWidgetPrivate {
	GtkWidget  *list;

	GtkWidget  *left_attrs;
	GtkWidget  *right_attrs;

	GtkWidget  *show;
	GtkWidget  *hide;

	GtkWidget  *tooltips;

	GtkWidget  *icon;
	GtkWidget  *icon_and_text;
	GtkWidget  *priority_text;

	const char *cur_path;
};

#define WIDGET_ATTR_KEY "BonoboUIConfigWidget_Attr"

static const char *
widget_get_attr (GtkWidget *widget)
{
	return gtk_object_get_data (GTK_OBJECT (widget),
				    WIDGET_ATTR_KEY);
}

static void
select_child_cb (GtkList	      *list,
		 GtkWidget            *child,
		 BonoboUIConfigWidget *config)
{
	BonoboUINode *node;

	config->priv->cur_path = widget_get_attr (child);

	node = bonobo_ui_engine_get_path (
		config->engine, config->priv->cur_path);

	gtk_widget_set_sensitive (config->priv->left_attrs, node != NULL);
	gtk_widget_set_sensitive (config->priv->right_attrs, node != NULL);

	if (node) {
		char    *txt;
		gboolean hidden = FALSE;
		gboolean tooltips = TRUE;

		if ((txt = bonobo_ui_node_get_attr (node, "hidden"))) {
			hidden = atoi (txt);
			bonobo_ui_node_free_string (txt);
		}

		if (hidden)
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (config->priv->hide),
				TRUE);
		else
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (config->priv->show),
				TRUE);

		if ((txt = bonobo_ui_node_get_attr (node, "tips"))) {
			tooltips = atoi (txt);
			bonobo_ui_node_free_string (txt);
		}
		
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (config->priv->tooltips),
			tooltips);
	} else
		g_warning ("Toolbar has been removed");
}

static void
populate_list (GtkWidget            *list,
	       BonoboUIConfigWidget *config)
{
	BonoboUINode *l, *start;

	GList *items = NULL;

	start = bonobo_ui_node_children (
		bonobo_ui_engine_get_xml (config->engine)->root);

	if (!start)
		g_warning ("No tree");

	for (l = start; l;
	     l = bonobo_ui_node_next (l)) {

		if (bonobo_ui_node_has_name (l, "dockitem")) {
			char    *txt, *name;

			if ((txt = bonobo_ui_node_get_attr (l, "tip"))) {
				gboolean err;

				name = bonobo_ui_util_decode_str (txt, &err);
				g_return_if_fail (!err);

				bonobo_ui_node_free_string (txt);
			} else if (!(name = bonobo_ui_node_get_attr (l, "name")))
				name = NULL;

			if (name) {
				GtkWidget *w = gtk_list_item_new_with_label (name);
				char      *path = bonobo_ui_xml_make_path (l);

				gtk_object_set_data_full (GTK_OBJECT (w),
							  WIDGET_ATTR_KEY,
							  path,
							  (GtkDestroyNotify) g_free);

				gtk_widget_show (w);
				items = g_list_prepend (items, w);
			}
		}
	}

	gtk_list_append_items (GTK_LIST (list), items);
	gtk_signal_connect (GTK_OBJECT (list), "select_child",
			    (GtkSignalFunc) select_child_cb, config);

	gtk_list_select_item (GTK_LIST (list), 0);
}

static void
show_hide_cb (GtkWidget            *button,
	      BonoboUIConfigWidget *config)
{
	g_return_if_fail (config->priv->cur_path != NULL);

	if (button == config->priv->show)
		bonobo_ui_engine_config_remove (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "hidden");
	else
		bonobo_ui_engine_config_add (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "hidden", "1");
}

static void
tooltips_cb (GtkWidget            *button,
	     BonoboUIConfigWidget *config)
{
	g_return_if_fail (config->priv->cur_path != NULL);

	if (gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (button)))
		bonobo_ui_engine_config_remove (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "tips");
	else
		bonobo_ui_engine_config_add (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "tips", "0");
}

static void
look_cb (GtkWidget            *button,
	 BonoboUIConfigWidget *config)
{
	const char *value = NULL;

	g_return_if_fail (config->priv->cur_path != NULL);

	if (button == config->priv->icon)
		value = "icon";

	else if (button == config->priv->icon_and_text)
		value = "both";

	else if (button == config->priv->priority_text)
		value = "text";

	else
		g_warning ("Unknown look selection");
	
	bonobo_ui_engine_config_add (
		bonobo_ui_engine_get_config (config->engine),
		config->priv->cur_path, "look", value);
}

static void
set_values (BonoboUIConfigWidget *config)
{
	BonoboUIToolbarStyle style;
	BonoboUINode *node;
	char *value;

	g_return_if_fail (config->priv->cur_path != NULL);

	node = bonobo_ui_engine_get_path (config->engine, config->priv->cur_path);

	/* Set the look */
	style = bonobo_ui_sync_toolbar_get_look (config->engine, node);
	switch (style) {
	case BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->icon), TRUE);
		break;

	case BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->icon_and_text), TRUE);
		break;

	case BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->priority_text), TRUE);
		break;
		
	default:
		break;
	}
	
	/* Set the tooltips */
	value = bonobo_ui_node_get_attr (node, "tips");
	if (value != NULL) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->tooltips), atoi (value));
		bonobo_ui_node_free_string (value);
	}
}

static void
widgets_init (BonoboUIConfigWidget *config,
	      GtkAccelGroup        *accel_group)
{
	BonoboUIConfigWidgetPrivate *priv;
	GtkWidget *table2;
	GtkWidget *vbox6;
	GtkWidget *frame6;
	GtkWidget *vbox7;
	GSList *visible_group = NULL;
	guint key;
	GtkWidget *frame7;
	GtkWidget *toolbar_list;
	GtkWidget *frame5;
	GtkWidget *vbox5;
	GSList *look_group = NULL;

	priv = config->priv;

	table2 = gtk_table_new (2, 2, FALSE);
	gtk_box_pack_start (GTK_BOX (config), table2, TRUE, TRUE, 0);

	priv->left_attrs = vbox6 = gtk_vbox_new (FALSE, 0);
	gtk_table_attach (GTK_TABLE (table2), vbox6, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);

	frame6 = gtk_frame_new (_("Visible"));
	gtk_box_pack_start (GTK_BOX (vbox6), frame6, TRUE, TRUE, 0);

	vbox7 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame6), vbox7);

	priv->show = gtk_radio_button_new_with_label (visible_group, "");
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (priv->show)->child),
						  _("_Show"));
	gtk_widget_add_accelerator (priv->show, "clicked", accel_group,
				    key, GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_signal_connect (GTK_OBJECT (priv->show), "clicked",
			    (GtkSignalFunc) show_hide_cb, config);
	visible_group = gtk_radio_button_group (GTK_RADIO_BUTTON (priv->show));
	gtk_box_pack_start (GTK_BOX (vbox7), priv->show, FALSE, FALSE, 0);

	priv->hide = gtk_radio_button_new_with_label (visible_group, "");
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (priv->hide)->child),
						   _("_Hide"));
	gtk_widget_add_accelerator (priv->hide, "clicked", accel_group,
				    key, GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_signal_connect (GTK_OBJECT (priv->hide), "clicked",
			    (GtkSignalFunc) show_hide_cb, config);
	visible_group = gtk_radio_button_group (GTK_RADIO_BUTTON (priv->hide));
	gtk_box_pack_start (GTK_BOX (vbox7), priv->hide, FALSE, FALSE, 0);

	priv->tooltips = gtk_check_button_new_with_label ("");
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (priv->tooltips)->child),
						  _("_View tooltips"));
	gtk_widget_add_accelerator (priv->tooltips, "clicked", accel_group,
				    key, GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_box_pack_start (GTK_BOX (vbox6), priv->tooltips, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (priv->tooltips), "clicked",
			    (GtkSignalFunc) tooltips_cb, config);

	frame7 = gtk_frame_new (_("Toolbars"));
	gtk_table_attach (GTK_TABLE (table2), frame7, 0, 2, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	priv->list = toolbar_list = gtk_list_new ();

	gtk_container_add (GTK_CONTAINER (frame7), toolbar_list);
	GTK_WIDGET_SET_FLAGS (toolbar_list, GTK_CAN_DEFAULT);

	frame5 = gtk_frame_new (_("Look"));
	gtk_table_attach (GTK_TABLE (table2), frame5, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);

	priv->right_attrs = vbox5 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame5), vbox5);

	priv->icon = gtk_radio_button_new_with_label (look_group, "");
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (priv->icon)->child),
						  _("_Icon"));
	gtk_widget_add_accelerator (priv->icon, "clicked", accel_group,
				    key, GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_signal_connect (GTK_OBJECT (priv->icon), "clicked",
			    (GtkSignalFunc) look_cb, config);
	look_group = gtk_radio_button_group (GTK_RADIO_BUTTON (priv->icon));
	gtk_box_pack_start (GTK_BOX (vbox5), priv->icon, FALSE, FALSE, 0);

	priv->icon_and_text = gtk_radio_button_new_with_label (look_group, "");
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (priv->icon_and_text)->child),
						  _("_Text and Icon"));
	gtk_widget_add_accelerator (priv->icon_and_text, "clicked", accel_group,
				    key, GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_signal_connect (GTK_OBJECT (priv->icon_and_text), "clicked",
			    (GtkSignalFunc) look_cb, config);
	look_group = gtk_radio_button_group (GTK_RADIO_BUTTON (priv->icon_and_text));
	gtk_box_pack_start (GTK_BOX (vbox5), priv->icon_and_text, FALSE, FALSE, 0);

	priv->priority_text = gtk_radio_button_new_with_label (look_group, "");
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (priv->priority_text)->child),
						  _("_Priority text only"));
	gtk_widget_add_accelerator (priv->priority_text, "clicked", accel_group,
				    key, GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_signal_connect (GTK_OBJECT (priv->priority_text), "clicked",
			    (GtkSignalFunc) look_cb, config);
	look_group = gtk_radio_button_group (GTK_RADIO_BUTTON (priv->priority_text));
	gtk_box_pack_start (GTK_BOX (vbox5), priv->priority_text, FALSE, FALSE, 0);

	populate_list (toolbar_list, config);
	set_values (config);

	gtk_widget_show_all (GTK_WIDGET (config));
	gtk_widget_hide (GTK_WIDGET (config));
}

static void
bonobo_ui_config_widget_init (GtkWidget *widget)
{
	BonoboUIConfigWidget *config = BONOBO_UI_CONFIG_WIDGET (widget);

	config->priv = g_new0 (BonoboUIConfigWidgetPrivate, 1);
}

static void
bonobo_ui_config_widget_finalize (GtkObject *object)
{
	BonoboUIConfigWidget *config = BONOBO_UI_CONFIG_WIDGET (object);

	g_free (config->priv);

	parent_class->finalize (object);
}

GtkWidget *
bonobo_ui_config_widget_construct (BonoboUIConfigWidget *config,
				   BonoboUIEngine       *engine,
				   GtkAccelGroup        *accel_group)
{
	config->engine = engine;
	widgets_init (config, accel_group);

	return GTK_WIDGET (config);
}

/**
 * bonobo_ui_config_widget_new:
 *
 * Creates a new BonoboUIConfigWidget widget, this contains
 * a List of toolbars and allows configuration of the widgets.
 *
 * Returns: A pointer to the newly-created BonoboUIConfigWidget widget.
 */
GtkWidget *
bonobo_ui_config_widget_new (BonoboUIEngine *engine,
			     GtkAccelGroup  *accel_group)
{
	BonoboUIConfigWidget *config = gtk_type_new (
		bonobo_ui_config_widget_get_type ());

	return bonobo_ui_config_widget_construct (
		config, engine, accel_group);
}

static void
bonobo_ui_config_widget_class_init (BonoboUIConfigWidgetClass *klass)
{
	GtkObjectClass *object_class;
	
	g_return_if_fail (klass != NULL);
	parent_class = gtk_type_class (PARENT_TYPE);
	
	object_class = (GtkObjectClass *) klass;
	
	object_class->finalize = bonobo_ui_config_widget_finalize;
}

/**
 * bonobo_ui_config_widget_get_type:
 *
 * Returns: The GtkType for the BonoboUIConfigWidget object class.
 */
GtkType
bonobo_ui_config_widget_get_type (void)
{
	static guint bonobo_ui_config_widget_type = 0;

	if (!bonobo_ui_config_widget_type) {
		GtkTypeInfo bonobo_ui_config_widget_info = {
			"BonoboUIConfigWidget",
			sizeof (BonoboUIConfigWidget),
			sizeof (BonoboUIConfigWidgetClass),
			(GtkClassInitFunc)  bonobo_ui_config_widget_class_init,
			(GtkObjectInitFunc) bonobo_ui_config_widget_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		bonobo_ui_config_widget_type = gtk_type_unique (
			PARENT_TYPE,
			&bonobo_ui_config_widget_info);
	}

	return bonobo_ui_config_widget_type;
}
