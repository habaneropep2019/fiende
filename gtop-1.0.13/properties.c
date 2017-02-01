#include <config.h>
#include <gnome.h>
#include "properties.h"
#include "procview.h"

GList *gtop_property_object_list = NULL;

GTopProperties gtop_temp_properties;
GTopProperties gtop_properties;

static GtkWidget *win = NULL;

void
gtop_properties_close (void)
{
	gnome_property_object_list_walk (gtop_property_object_list,
					 GNOME_PROPERTY_ACTION_DISCARD_TEMP);
	
	win = NULL;
}

void
gtop_properties_apply (void)
{
	gnome_property_object_list_walk (gtop_property_object_list,
					 GNOME_PROPERTY_ACTION_APPLY);

	gnome_property_object_list_walk (gtop_property_object_list,
					 GNOME_PROPERTY_ACTION_SAVE_TEMP);

	gnome_property_object_list_walk (gtop_property_object_list,
					 GNOME_PROPERTY_ACTION_SAVE);

	gnome_property_object_list_walk (gtop_property_object_list,
					 GNOME_PROPERTY_ACTION_UPDATE);

	gnome_property_object_list_walk (gtop_property_object_list,
					 GNOME_PROPERTY_ACTION_LOAD_TEMP);
}

void
gtop_properties_changed (void)
{
        gnome_property_box_changed (GNOME_PROPERTY_BOX (win));
}

void
gtop_show_properties (void)
{
	GList *c;

	if (win) {
		gdk_window_raise (win->window);

		return;
	}
	
	win = gnome_property_box_new ();
	gtk_window_set_title (GTK_WINDOW (win), _("GTop - Preferences"));

	gtk_notebook_set_scrollable
		(GTK_NOTEBOOK (GNOME_PROPERTY_BOX (win)->notebook), TRUE);

	for (c = gtop_property_object_list; c; c = c->next) {
		gnome_property_object_register
			(GNOME_PROPERTY_BOX (win),
			 (GnomePropertyObject *) c->data);
	}

	gnome_property_object_list_walk (gtop_property_object_list,
					 GNOME_PROPERTY_ACTION_LOAD_TEMP);

	gtk_signal_connect (GTK_OBJECT (win), "apply",
			    GTK_SIGNAL_FUNC(gtop_properties_apply), NULL);

	gtk_signal_connect (GTK_OBJECT (win), "destroy",
			    GTK_SIGNAL_FUNC(gtop_properties_close), NULL);

	gtk_widget_show_all (win);
}

void
gtop_init_properties (void)
{
}
