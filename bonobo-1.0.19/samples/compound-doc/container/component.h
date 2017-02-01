#ifndef SAMPLE_COMPONENT_H
#define SAMPLE_COMPONENT_H

#include <gnome.h>
#include <bonobo.h>

#include "container.h"

#define SAMPLE_CLIENT_SITE_TYPE        (sample_client_site_get_type ())
#define SAMPLE_CLIENT_SITE(o)          (GTK_CHECK_CAST ((o), SAMPLE_CLIENT_SITE_TYPE, SampleClientSite))
#define SAMPLE_CLIENT_SITE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), SAMPLE_CLIENT_SITE_TYPE, SampleClientSiteClass))
#define SAMPLE_IS_CLIENT_SITE(o)       (GTK_CHECK_TYPE ((o), SAMPLE_CLIENT_SITE_TYPE))
#define SAMPLE_IS_CLIENT_SITE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SAMPLE_CLIENT_SITE_TYPE))

struct _SampleClientSite {
	BonoboClientSite parent;

	SampleApp *app;
	gchar     *obj_id;

	GtkWidget *widget;
	GtkWidget *views_hbox;
	GtkWidget *frame;
};

typedef struct {
	BonoboClientSiteClass parent_class;
} SampleClientSiteClass;

GtkType           sample_client_site_get_type   (void);
SampleClientSite *sample_client_site_new        (BonoboItemContainer *container,
						 SampleApp           *app,
						 BonoboObjectClient  *embeddable,
						 const char          *embeddable_id);

GtkWidget        *sample_client_site_get_widget (SampleClientSite   *site);

void              sample_client_site_add_frame  (SampleClientSite *site);

void              object_print                  (BonoboObjectClient *object,
						 GnomePrintContext  *ctx,
						 gdouble x, gdouble y,
						 gdouble width, gdouble height);

#endif
