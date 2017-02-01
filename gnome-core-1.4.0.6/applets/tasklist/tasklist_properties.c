#include <config.h>
#include "tasklist_applet.h"

/* Callback for apply */
static void
cb_apply (GtkWidget *widget, gint page, gpointer data)
{
	Tasklist *tasklist = data;
	
	/* Copy the Property struct back to the Config struct */
	memcpy (&tasklist->config, &tasklist->PropsConfig, sizeof (TasklistConfig));

	/* Redraw everything */
	tasklist_redo_vtasks (tasklist);
	tasklist_change_size (tasklist, TRUE, -1);

	/* setup tooltips */
	if (tasklist->config.enable_tooltips)
		gtk_tooltips_enable (tasklist->tooltips);
	else
		gtk_tooltips_disable (tasklist->tooltips);

}

/* Callback for radio buttons */
static void
cb_radio_button (GtkWidget *widget, gint *data)
{
	Tasklist *tasklist = gtk_object_get_user_data (GTK_OBJECT (widget));
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		*data = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget),
							      "number"));
		gnome_property_box_changed (GNOME_PROPERTY_BOX (tasklist->prop));
	}
}

/* Callback for spin buttons */
static void
cb_spin_button (GtkWidget *widget, gint *data)
{
	Tasklist *tasklist = gtk_object_get_user_data (GTK_OBJECT (widget));
	
	gnome_property_box_changed (GNOME_PROPERTY_BOX (tasklist->prop));
	
	*data = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
}

/* Callback for check buttons */
static void
cb_check_button (GtkWidget *widget, gboolean *data)
{
	Tasklist *tasklist = gtk_object_get_user_data (GTK_OBJECT (widget));
	
	gnome_property_box_changed (GNOME_PROPERTY_BOX (tasklist->prop));

	*data = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

static void
cb_check_button_disable (GtkWidget *widget, GtkWidget *todisable)
{
	gboolean active;
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	gtk_widget_set_sensitive (todisable, !active);
}

 
/* Create a spin button */
static GtkWidget *
create_spin_button (Tasklist *tasklist,
		    gchar *name,
		    gint *init_value,
		    gfloat min_value,
		    gfloat max_value,
		    gfloat page_value)
{
	GtkObject *adj;
	GtkWidget *spin;
	GtkWidget *hbox;
	GtkWidget *label;

	adj = gtk_adjustment_new (*init_value,
				  min_value,
				  max_value,
				  1,
				  page_value,
				  page_value);
	
	hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);

	spin = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
	gtk_object_set_user_data (GTK_OBJECT (spin), tasklist);
	gtk_signal_connect (GTK_OBJECT (spin), "changed",
			    GTK_SIGNAL_FUNC (cb_spin_button), init_value);
						


	label = gtk_label_new (name);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, TRUE, 0);

	return hbox;
}

/* Create a radio button */
static GtkWidget *
create_radio_button (Tasklist *tasklist, gchar *name, GSList **group, 
		     gint number, gint *change_value)
{
	GtkWidget *radiobutton;
	
	radiobutton = gtk_radio_button_new_with_label (*group, name);
	*group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton));

	if (number == *change_value)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), TRUE);
	
	gtk_signal_connect (GTK_OBJECT (radiobutton), "toggled",
			    GTK_SIGNAL_FUNC (cb_radio_button), change_value);
	gtk_object_set_data (GTK_OBJECT (radiobutton), "number",
			     GINT_TO_POINTER (number));
	gtk_object_set_user_data (GTK_OBJECT (radiobutton), tasklist);

	return radiobutton;
}

/* Create a check button */
static GtkWidget *
create_check_button (Tasklist *tasklist, gchar *name, gboolean *change_value)
{
	GtkWidget *checkbutton;

	checkbutton = gtk_check_button_new_with_label (name);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), *change_value);
	gtk_object_set_user_data (GTK_OBJECT (checkbutton), tasklist);
	gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
			    GTK_SIGNAL_FUNC (cb_check_button), change_value);
	return checkbutton;
}

/* Create the size page */
static void
create_size_page (Tasklist *tasklist)
{
	GtkWidget *hbox,/* *table,*/ *frame, *vbox, *topbox;
	GSList *vertgroup = NULL, *horzgroup = NULL;
	GtkWidget *autobutton, *w;
	
	topbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (topbox), GNOME_PAD_SMALL);
	
	autobutton = create_check_button (
		tasklist, _("Follow panel size"),
		&tasklist->PropsConfig.follow_panel_size);
	gtk_box_pack_start (GTK_BOX (topbox),
			    autobutton,
			    FALSE, TRUE, 0);

	hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);
	gtk_box_pack_start_defaults (GTK_BOX (topbox), hbox);

	frame = gtk_frame_new (_("Horizontal"));

	gtk_box_pack_start_defaults (GTK_BOX (hbox), frame);
	
	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

	gtk_box_pack_start (
		GTK_BOX (vbox),
		create_spin_button (
			tasklist, _("Tasklist width:"),
			&tasklist->PropsConfig.horz_width,
			48,
			8192,
			10),
		FALSE, TRUE, 0);
	w = create_spin_button (
		tasklist, _("Rows of tasks:"),
		&tasklist->PropsConfig.horz_rows,
		1,
		8,
		1);
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (autobutton), "toggled",
			    GTK_SIGNAL_FUNC (cb_check_button_disable),
			    w);
	cb_check_button_disable (autobutton, w);

	gtk_box_pack_start (
		GTK_BOX (vbox),
		create_spin_button (
			tasklist, _("Default task size:"),
			&tasklist->PropsConfig.horz_taskwidth,
			48,
			350,
			10),
		FALSE, TRUE, 0);


	gtk_box_pack_start (GTK_BOX (vbox),
			    create_radio_button (
				    tasklist, _("Tasklist width is fixed"),
				    &horzgroup, TRUE, &tasklist->PropsConfig.horz_fixed),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox),
			    create_radio_button (
				    tasklist, _("Tasklist width is dynamic"), 
				    &horzgroup, FALSE, &tasklist->PropsConfig.horz_fixed),
			    FALSE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (vbox),
			    create_check_button (tasklist, _("Only use empty space"),
						 &tasklist->PropsConfig.horz_never_push),
			    FALSE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER (frame), vbox);
	
	frame = gtk_frame_new (_("Vertical"));
	gtk_box_pack_start_defaults (GTK_BOX (hbox), frame);
	
	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

	gtk_box_pack_start (
		GTK_BOX (vbox),
		create_spin_button (tasklist, _("Tasklist height:"),
				    &tasklist->PropsConfig.vert_height,
				    48,
				    1024*8,
				    10),
		FALSE, TRUE, 0);

	w = create_spin_button (tasklist, _("Tasklist width:"),
				&tasklist->PropsConfig.vert_width,
				48,
				512,
				10);
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (autobutton), "toggled",
			   GTK_SIGNAL_FUNC (cb_check_button_disable),
			   w);
	cb_check_button_disable (autobutton, w);

	gtk_box_pack_start (GTK_BOX (vbox),
			    create_radio_button (
				    tasklist,_("Tasklist height is fixed"),
				    &vertgroup, TRUE, &tasklist->PropsConfig.vert_fixed),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (
		GTK_BOX (vbox),
		create_radio_button (
			tasklist, _("Tasklist height is dynamic"), 
			&vertgroup, FALSE, &tasklist->PropsConfig.vert_fixed),
		FALSE, TRUE, 0);


	gtk_box_pack_start (GTK_BOX (vbox),
			    create_check_button (tasklist, _("Only use empty space"),
						 &tasklist->PropsConfig.vert_never_push),
			    FALSE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (vbox),
			    create_check_button (tasklist, 
						 _("Tasklist width is that of longest title"),
						 &tasklist->PropsConfig.vert_width_full),
			    FALSE, TRUE, 0);
	
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	gnome_property_box_append_page (
		GNOME_PROPERTY_BOX (tasklist->prop), topbox,
		gtk_label_new (_("Size")));
}

static void
create_display_page (Tasklist *tasklist)
{
	GtkWidget *vbox, *frame;
	GtkWidget *miscbox, *taskbox;
	/*GtkWidget *radio;*/
	/*GSList *taskgroup = NULL;*/

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	
	frame = gtk_frame_new (_("Which tasks to show"));
	gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);

	taskbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (taskbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), taskbox);
	
	gtk_box_pack_start (
		GTK_BOX (taskbox),
		create_check_button (
			tasklist, _("Show normal applications"),
			&tasklist->PropsConfig.show_normal),
		FALSE, TRUE, 0);
	gtk_box_pack_start (
		GTK_BOX (taskbox),
		create_check_button (
			tasklist, _("Show iconified (minimized) applications"),
			&tasklist->PropsConfig.show_minimized),
		FALSE, TRUE, 0);
			    
	gtk_box_pack_start (
		GTK_BOX (taskbox),
		create_check_button (
			tasklist, _("Show normal applications on all desktops"),
			&tasklist->PropsConfig.all_desks_normal),
		FALSE, TRUE, 0);
	gtk_box_pack_start (
		GTK_BOX (taskbox),
		create_check_button (tasklist,
				     _("Show iconified (minimized) applications on all desktops"),
				     &tasklist->PropsConfig.all_desks_minimized),
		FALSE, TRUE, 0);

	frame = gtk_frame_new (_("Miscellaneous"));
	gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);
	
	miscbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (miscbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), miscbox);

	gtk_box_pack_start (
		GTK_BOX (miscbox),
		create_check_button (
			tasklist, _("Show mini icons"),
			&tasklist->PropsConfig.show_mini_icons),
		FALSE, TRUE, 0);
	gtk_box_pack_start (
		GTK_BOX (miscbox),
		create_check_button (
			tasklist,
			_("Confirm before killing windows"),
			&tasklist->PropsConfig.confirm_before_kill),
		FALSE, TRUE, 0);
	gtk_box_pack_start (
		GTK_BOX (miscbox),
		create_check_button (
			tasklist,
			_("Move iconified tasks to current workspace when restoring"),
			&tasklist->PropsConfig.move_to_current),
		FALSE, TRUE, 0);
	gtk_box_pack_start (
		GTK_BOX (miscbox),
		create_check_button (
			tasklist,
			_("Display tooltips with full task names"),
			&tasklist->PropsConfig.enable_tooltips),
		FALSE, TRUE, 0);

	gtk_box_pack_start (
		GTK_BOX (miscbox),
		create_check_button (
			tasklist,
			_("Enable task grouping"),
			&tasklist->PropsConfig.enable_grouping),
		FALSE, TRUE, 0);

	gtk_box_pack_start (
		GTK_BOX (miscbox),
		create_spin_button (
			tasklist,
			_("Number of tasks before grouping occurs"),
			&tasklist->PropsConfig.grouping_min,
			1, 10, 3),
		FALSE, TRUE, 0);

	gtk_box_pack_start (
                GTK_BOX (miscbox),
		create_check_button (
                        tasklist,
                        _("Sink tasklist into panel"), 
			&tasklist->PropsConfig.sunken),
		FALSE, TRUE, 0);

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (tasklist->prop), vbox,
					gtk_label_new (_("Display")));
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "tasklist_applet",
					  "index.html#TASKLIST-APPLET-PROPERTIES" };
	gnome_help_display(NULL, &help_entry);
}

/* Display property dialog */
void
tasklist_display_properties (Tasklist *tasklist)
{
	if (tasklist->prop != NULL)
	{
		gdk_window_show (tasklist->prop->window);
		gdk_window_raise (tasklist->prop->window);
		return;

	}
	/*
	 * Copy memory from the tasklist config 
	 * to the tasklist properties config.
	 */
	memcpy (&tasklist->PropsConfig, &tasklist->config, sizeof (TasklistConfig));

	tasklist->prop = gnome_property_box_new ();
	gtk_window_set_title (GTK_WINDOW (tasklist->prop), _("Tasklist properties"));
	gtk_window_set_wmclass (GTK_WINDOW (tasklist->prop), "tasklist", "Tasklist");
	gtk_signal_connect (GTK_OBJECT (tasklist->prop), "apply",
			    GTK_SIGNAL_FUNC (cb_apply), tasklist);
	gtk_signal_connect (GTK_OBJECT (tasklist->prop), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &tasklist->prop);
	gtk_signal_connect (GTK_OBJECT (tasklist->prop), "help",
			    GTK_SIGNAL_FUNC (phelp_cb), NULL);
	create_display_page (tasklist);
	create_size_page (tasklist);

	gtk_widget_show_all (tasklist->prop);
}
