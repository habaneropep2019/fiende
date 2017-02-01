/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-selector.h: Bonobo Component Selector
 *
 * Authors:
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
#ifndef __BONOBO_SELECTOR_H__
#define __BONOBO_SELECTOR_H__

#include <libgnomeui/gnome-dialog.h>
#include <bonobo/bonobo-selector-widget.h>

BEGIN_GNOME_DECLS

#define BONOBO_SELECTOR(obj)		GTK_CHECK_CAST(obj, bonobo_selector_get_type (), BonoboSelector)
#define BONOBO_SELECTOR_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, bonobo_selector_get_type (), BonoboSelectorClass)
#define BONOBO_IS_SELECTOR(obj)		GTK_CHECK_TYPE (obj, bonobo_selector_get_type ())

typedef struct _BonoboSelectorPrivate BonoboSelectorPrivate;

typedef struct {
	GnomeDialog dialog;

	BonoboSelectorPrivate *priv;
} BonoboSelector;

typedef struct {
	GnomeDialogClass parent_class;
	
	void (* ok)	(BonoboSelector *sel);
	void (* cancel)	(BonoboSelector *sel);
} BonoboSelectorClass;

GtkType	   bonobo_selector_get_type        (void);

GtkWidget *bonobo_selector_construct       (BonoboSelector       *sel,
					    const gchar          *title,
					    BonoboSelectorWidget *selector);

GtkWidget *bonobo_selector_new             (const gchar *title,
					    const gchar **interfaces_required);


gchar	  *bonobo_selector_get_selected_id          (BonoboSelector *sel);
gchar     *bonobo_selector_get_selected_name        (BonoboSelector *sel);
gchar     *bonobo_selector_get_selected_description (BonoboSelector *sel);

gchar	  *bonobo_selector_select_id       (const gchar *title,
					    const gchar **interfaces_required);

END_GNOME_DECLS

#endif /* __BONOBO_SELECTOR_H__ */

