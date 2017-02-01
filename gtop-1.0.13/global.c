#include <config.h>
#include <gnome.h>
#include <global.h>

#include <properties.h>

static GtkWidget *global_properties_init(GnomePropertyObject *);
static void	global_properties_apply	(GnomePropertyObject *);
static void	global_properties_update(GnomePropertyObject *);
static void	global_properties_load	(GnomePropertyObject *);
static void	global_properties_save	(GnomePropertyObject *);

static void	save_session_cb		(GtkWidget *, RadioButtonCbData *);
static void	adjustment_changed_cb	(GtkWidget *, GtkWidget *);

GnomePropertyDescriptor GlobalProperty_Descriptor = {
	sizeof (GTopGlobalProperties),
	N_("Global"),
	global_properties_init,
	global_properties_apply,
	global_properties_update,
	global_properties_load,
	global_properties_save,
	NULL, NULL, NULL, NULL, NULL
};

static GtkAdjustment *adjustments [UPDATE_FIELDS];

static void
save_session_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopGlobalProperties *prop_ptr = cb_data->object->temp_data;

	prop_ptr->save_session = GTK_TOGGLE_BUTTON (cb_data->button)->active;

	gtop_properties_changed ();
}

static void
adjustment_changed_cb (GtkWidget *widget,
		       GtkWidget *adjustment)
{
	gtop_properties_changed ();
}

static void
global_properties_update (GnomePropertyObject *object)
{
	if (mdi->mode != gtop_properties.global.mdi_mode)
		gnome_mdi_set_mode (mdi, gtop_properties.global.mdi_mode);
}

static void
global_properties_apply (GnomePropertyObject *object)
{
	GTopGlobalProperties *prop_ptr = object->temp_data;
	gint i;
	
	for (i = 0; i < UPDATE_FIELDS; i++)
		prop_ptr->update_times [i] = adjustments [i]->value;
}

static void
radio_mdi_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopGlobalProperties *prop_ptr = cb_data->object->temp_data;

	if (!GTK_TOGGLE_BUTTON (cb_data->button)->active)
		return;

	prop_ptr->mdi_mode = cb_data->index;

	gtop_properties_changed ();
}

static GtkWidget *
global_properties_init (GnomePropertyObject *object)
{
	GtkWidget *vb, *hb, *frame, *button, *table, *label, *spin;
	GTopGlobalProperties *prop_ptr = &gtop_properties.global;
	RadioButtonCbData *cb_data;
	GtkObject *adjustment;
	GSList *group;


	vb = gtk_vbox_new (0, FALSE);
	gtk_container_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

	frame = gtk_frame_new (_("Session management"));
	table = gtk_table_new (1, 1, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

	button = gtk_check_button_new_with_label (_("Always save session"));
	if (prop_ptr->save_session)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = button;
	cb_data->object = object;

	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", save_session_cb, cb_data);

	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 0, 1);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	frame = gtk_frame_new (_("Update times"));
	table = gtk_table_new (3, 3, FALSE);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD);
	gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

	label = gtk_label_new (_("Process List"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_PROCVIEW]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 0, 1);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_PROCVIEW] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 0, 1);

	label = gtk_label_new (_("Memory Usage"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_MEMUSAGE]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 1, 2);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_MEMUSAGE] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 1, 2);

	label = gtk_label_new (_("Filesystem Usage"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_FSUSAGE]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 2, 3);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_FSUSAGE] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 2, 3);

	gtk_container_add (GTK_CONTAINER (frame), table);
	hb = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (hb), frame, TRUE, TRUE, 0);

	frame = gtk_frame_new (_("Update times (graphical summary)"));
	table = gtk_table_new (3, 3, FALSE);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD);
	gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

	label = gtk_label_new (_("Summary CPU"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_CPU]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 0, 1);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_CPU] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 0, 1);

	label = gtk_label_new (_("Summary Mem and Swap"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_MEM]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 1, 2);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_MEM] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 1, 2);

	label = gtk_label_new (_("Load Average"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_LOAD]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 2, 3);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_LOAD] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 2, 3);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (hb), frame, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, TRUE, 0);

	frame = gtk_frame_new (_("Update times (textual summary)"));
	table = gtk_table_new (5, 5, FALSE);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD);
	gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

	label = gtk_label_new (_("CPU statistics"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_STATUS_CPU]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 0, 1);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_STATUS_CPU] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 0, 1);

	label = gtk_label_new (_("Memory statistics"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_STATUS_MEMORY]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 1, 2);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_STATUS_MEMORY] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 1, 2);

	label = gtk_label_new (_("Uptime"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_STATUS_UPTIME]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 2, 3);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_STATUS_UPTIME] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 2, 3);

	label = gtk_label_new (_("Load average"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 3, 4);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_STATUS_LOADAVG]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 3, 4);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_STATUS_LOADAVG] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 3, 4);

	label = gtk_label_new (_("Details dialog"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 4, 5);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 prop_ptr->update_times [GTOP_UPDATE_DETAILS]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 4, 5);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_UPDATE_DETAILS] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("ms"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 4, 5);

	gtk_container_add (GTK_CONTAINER (frame), table);
	hb = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (hb), frame, TRUE, TRUE, 0);

	frame = gtk_frame_new (_("MDI Mode"));
	table = gtk_table_new (3, 4, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

	button = gtk_radio_button_new_with_label (NULL, _("Notebook"));
	if (prop_ptr->mdi_mode == GNOME_MDI_NOTEBOOK)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = button;
	cb_data->object = object;
	cb_data->index = GNOME_MDI_NOTEBOOK;

	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_mdi_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	button = gtk_radio_button_new_with_label (group, _("Toplevel"));
	if (prop_ptr->mdi_mode == GNOME_MDI_TOPLEVEL)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = button;
	cb_data->object = object;
	cb_data->index = GNOME_MDI_TOPLEVEL;

	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_mdi_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 1, 2,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	button = gtk_radio_button_new_with_label (group, _("Modal"));
	if (prop_ptr->mdi_mode == GNOME_MDI_MODAL)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = button;
	cb_data->object = object;
	cb_data->index = GNOME_MDI_MODAL;

	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_mdi_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 2, 3,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (hb), frame, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, TRUE, 0);

	return vb;
}

static void
global_properties_load (GnomePropertyObject *object)
{
	char key [BUFSIZ];

	gtop_properties.global.save_session =
		gnome_config_get_int ("gtop/global/save_session=0");

	sprintf (key, "gtop/global/mdi_mode=%d", GNOME_MDI_NOTEBOOK);
	gtop_properties.global.mdi_mode = gnome_config_get_int (key);

	gtop_properties.global.update_times [GTOP_UPDATE_CPU] =
		gnome_config_get_int ("gtop/global/update_cpu=400");

	gtop_properties.global.update_times [GTOP_UPDATE_MEM] =
		gnome_config_get_int ("gtop/global/update_mem=800");

	gtop_properties.global.update_times [GTOP_UPDATE_PROCVIEW] =
		gnome_config_get_int ("gtop/global/update_procview=3000");

	gtop_properties.global.update_times [GTOP_UPDATE_MEMUSAGE] =
		gnome_config_get_int ("gtop/global/update_memusage=3000");

	gtop_properties.global.update_times [GTOP_UPDATE_FSUSAGE] =
		gnome_config_get_int ("gtop/global/update_fsusage=3000");

	gtop_properties.global.update_times [GTOP_UPDATE_LOAD] =
		gnome_config_get_int ("gtop/global/update_load=400");

	gtop_properties.global.update_times [GTOP_UPDATE_STATUS_CPU] =
		gnome_config_get_int ("gtop/global/update_status_cpu=2500");

	gtop_properties.global.update_times [GTOP_UPDATE_STATUS_MEMORY] =
		gnome_config_get_int ("gtop/global/update_status_memory=5000");

	gtop_properties.global.update_times [GTOP_UPDATE_STATUS_UPTIME] =
		gnome_config_get_int ("gtop/global/update_status_uptime=60000");

	gtop_properties.global.update_times [GTOP_UPDATE_STATUS_LOADAVG] =
		gnome_config_get_int ("gtop/global/update_status_loadavg=1000");

	gtop_properties.global.update_times [GTOP_UPDATE_DETAILS] =
		gnome_config_get_int ("gtop/global/update_details=3000");

	gtop_properties.global.show_menubar =
		gnome_config_get_int ("gtop/global/show_menubar=1");

	gtop_properties.global.show_toolbar =
		gnome_config_get_int ("gtop/global/show_toolbar=1");
}

static void
global_properties_save (GnomePropertyObject *object)
{
	gnome_config_set_int
		("gtop/global/save_session",
		 gtop_properties.global.save_session);

	gnome_config_set_int
		("gtop/global/mdi_mode",
		 gtop_properties.global.mdi_mode);

	gnome_config_set_int
		("gtop/global/update_cpu",
		 gtop_properties.global.update_times [GTOP_UPDATE_CPU]);

	gnome_config_set_int
		("gtop/global/update_mem",
		 gtop_properties.global.update_times [GTOP_UPDATE_MEM]);

	gnome_config_set_int
		("gtop/global/update_procview",
		 gtop_properties.global.update_times [GTOP_UPDATE_PROCVIEW]);

	gnome_config_set_int
		("gtop/global/update_memusage",
		 gtop_properties.global.update_times [GTOP_UPDATE_MEMUSAGE]);

	gnome_config_set_int
		("gtop/global/update_fsusage",
		 gtop_properties.global.update_times [GTOP_UPDATE_FSUSAGE]);

	gnome_config_set_int
		("gtop/global/update_load",
		 gtop_properties.global.update_times [GTOP_UPDATE_LOAD]);

	gnome_config_set_int
		("gtop/global/update_status_cpu",
		 gtop_properties.global.update_times [GTOP_UPDATE_STATUS_CPU]);

	gnome_config_set_int
		("gtop/global/update_status_memory",
		 gtop_properties.global.update_times [GTOP_UPDATE_STATUS_MEMORY]);

	gnome_config_set_int
		("gtop/global/update_status_uptime",
		 gtop_properties.global.update_times [GTOP_UPDATE_STATUS_UPTIME]);

	gnome_config_set_int
		("gtop/global/update_status_loadavg",
		 gtop_properties.global.update_times [GTOP_UPDATE_STATUS_LOADAVG]);

	gnome_config_set_int
		("gtop/global/update_details",
		 gtop_properties.global.update_times [GTOP_UPDATE_DETAILS]);

	gnome_config_set_int
		("gtop/global/show_menubar",
		 gtop_properties.global.show_menubar);

	gnome_config_set_int
		("gtop/global/show_toolbar",
		 gtop_properties.global.show_toolbar);
}

void
gtop_check_old_config (void)
{
    int config_version;

    config_version = gnome_config_get_int ("gtop/config/config_version");
    if (config_version < 2) {
	/* We have an old config file. */

	gnome_config_clean_file ("gtop");

	gnome_config_set_int ("gtop/config/config_version", 2);

	gnome_config_sync ();
    }
}
