/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-client-site.h: a ClientSite object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_CLIENT_SITE_H_
#define _BONOBO_CLIENT_SITE_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <libgnomeui/gnome-canvas.h>
#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-object-client.h>
#include <bonobo/bonobo-item-container.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_CLIENT_SITE_TYPE        (bonobo_client_site_get_type ())
#define BONOBO_CLIENT_SITE(o)          (GTK_CHECK_CAST ((o), BONOBO_CLIENT_SITE_TYPE, BonoboClientSite))
#define BONOBO_CLIENT_SITE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CLIENT_SITE_TYPE, BonoboClientSiteClass))
#define BONOBO_IS_CLIENT_SITE(o)       (GTK_CHECK_TYPE ((o), BONOBO_CLIENT_SITE_TYPE))
#define BONOBO_IS_CLIENT_SITE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CLIENT_SITE_TYPE))

typedef struct _BonoboClientSite        BonoboClientSite;
typedef struct _BonoboClientSitePrivate BonoboClientSitePrivate;

#include <bonobo/bonobo-view-frame.h>

struct _BonoboClientSite {
	BonoboXObject base;

	BonoboItemContainer *container;
	BonoboObjectClient  *bound_embeddable; /* IDL:Bonobo/Embeddable:1.0 */
	GList		    *view_frames;
	GList               *canvas_items;
	unsigned int         child_shown:1;

	BonoboClientSitePrivate *priv;
};

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_ClientSite__epv epv;

	void (*show_window)  (BonoboClientSite *, CORBA_boolean shown);
	void (*queue_resize) (BonoboClientSite *);
	void (*save_object)  (BonoboClientSite *, Bonobo_Persist_Status *status);
} BonoboClientSiteClass;

GtkType                     bonobo_client_site_get_type            (void);
BonoboClientSite           *bonobo_client_site_new                 (BonoboItemContainer *container);
BonoboClientSite           *bonobo_client_site_construct           (BonoboClientSite    *client_site,
								    BonoboItemContainer *container);
gboolean                    bonobo_client_site_bind_embeddable     (BonoboClientSite    *client_site,
								    BonoboObjectClient  *object);
BonoboObjectClient         *bonobo_client_site_get_embeddable      (BonoboClientSite    *client_site);
BonoboItemContainer        *bonobo_client_site_get_container       (BonoboClientSite    *client_site);

/*
 * Proxy/Utility functions.
 */
BonoboViewFrame            *bonobo_client_site_new_view_full    (BonoboClientSite   *client_site,
								 Bonobo_UIContainer  uic,
								 gboolean            visible_cover,
								 gboolean            active_view);
BonoboViewFrame            *bonobo_client_site_new_view         (BonoboClientSite   *client_site,
								 Bonobo_UIContainer  uic);
GnomeCanvasItem            *bonobo_client_site_new_item         (BonoboClientSite   *client_site,
								 Bonobo_UIContainer  uic,
								 GnomeCanvasGroup   *group);
GList                      *bonobo_client_site_get_verbs        (BonoboClientSite   *client_site);
void                        bonobo_client_site_free_verbs       (GList              *verb_list);

END_GNOME_DECLS

#endif
