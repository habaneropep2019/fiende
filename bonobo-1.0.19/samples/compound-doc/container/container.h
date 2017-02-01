#ifndef SAMPLE_CONTAINER_H
#define SAMPLE_CONTAINER_H

#include <bonobo.h>
#include <gnome.h>

typedef struct _SampleApp        SampleApp;
typedef struct _SampleClientSite SampleClientSite;

struct _SampleApp {
	BonoboItemContainer *container;
	BonoboUIContainer   *ui_container;

	BonoboViewFrame     *curr_view;
	GList               *components;

	GtkWidget           *app;
	GtkWidget           *box;
	GtkWidget           *fileselection;
};

SampleClientSite *sample_app_add_component    (SampleApp        *app,
					       gchar            *goad_id);
void              sample_app_remove_component (SampleApp        *app,
					       SampleClientSite *site);
void              sample_app_exit             (SampleApp        *app);

#endif /* SAMPLE_CONTAINER_H */

