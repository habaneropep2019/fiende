/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-view.h: a view object of an embeddable
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman    (nat@nat.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_VIEW_H_
#define _BONOBO_VIEW_H_

#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-view-frame.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_VIEW_TYPE        (bonobo_view_get_type ())
#define BONOBO_VIEW(o)          (GTK_CHECK_CAST ((o), BONOBO_VIEW_TYPE, BonoboView))
#define BONOBO_VIEW_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_VIEW_TYPE, BonoboViewClass))
#define BONOBO_IS_VIEW(o)       (GTK_CHECK_TYPE ((o), BONOBO_VIEW_TYPE))
#define BONOBO_IS_VIEW_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_VIEW_TYPE))

typedef struct _BonoboView BonoboView;
typedef struct _BonoboViewPrivate BonoboViewPrivate;

#include <bonobo/bonobo-embeddable.h>

#define BONOBO_VIEW_VERB_FUNC(fn) ((BonoboViewVerbFunc)(fn))
typedef void (*BonoboViewVerbFunc)(BonoboView *view, const char *verb_name, void *user_data);

struct _BonoboView {
	BonoboControl base;

	BonoboEmbeddable  *embeddable;
	Bonobo_ViewFrame   view_frame;
	BonoboViewPrivate *priv;
};

typedef struct {
	BonoboControlClass       parent_class;

	POA_Bonobo_View__epv     epv;

	/* Signals */
	void (*do_verb)         (BonoboView *view,
				 const CORBA_char *verb_name);
	void (*set_zoom_factor) (BonoboView *view, double zoom);

} BonoboViewClass;

GtkType               bonobo_view_get_type               (void);
BonoboView           *bonobo_view_construct              (BonoboView         *view,
							  GtkWidget          *widget);
BonoboView           *bonobo_view_new                    (GtkWidget          *widget);
void                  bonobo_view_set_embeddable         (BonoboView         *view,
							  BonoboEmbeddable   *embeddable);
BonoboEmbeddable     *bonobo_view_get_embeddable         (BonoboView         *view);
void                  bonobo_view_set_view_frame         (BonoboView         *view,
							  Bonobo_ViewFrame    view_frame);
Bonobo_ViewFrame      bonobo_view_get_view_frame         (BonoboView         *view);
Bonobo_UIContainer    bonobo_view_get_remote_ui_container(BonoboView         *view);
BonoboUIComponent    *bonobo_view_get_ui_component       (BonoboView         *view);
void                  bonobo_view_activate_notify        (BonoboView         *view,
							  gboolean            activated);

END_GNOME_DECLS

#endif /* _BONOBO_VIEW_H_ */
