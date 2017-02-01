/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-win.c: The Bonobo Window implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_WINDOW_H_
#define _BONOBO_WINDOW_H_

#include <gtk/gtkmenu.h>
#include <gtk/gtkwindow.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-ui-engine.h>

#define BONOBO_TYPE_WINDOW        (bonobo_window_get_type ())
#define BONOBO_WINDOW(o)          (GTK_CHECK_CAST ((o), BONOBO_TYPE_WINDOW, BonoboWindow))
#define BONOBO_WINDOW_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_TYPE_WINDOW, BonoboWindowClass))
#define BONOBO_IS_WINDOW(o)       (GTK_CHECK_TYPE ((o), BONOBO_TYPE_WINDOW))
#define BONOBO_IS_WINDOW_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_TYPE_WINDOW))

typedef struct _BonoboWindowPrivate BonoboWindowPrivate;

typedef struct {
	GtkWindow          parent;
	
	BonoboWindowPrivate  *priv;
} BonoboWindow;

typedef struct {
	GtkWindowClass    parent_class;
} BonoboWindowClass;

GtkType              bonobo_window_get_type                       (void);

GtkWidget           *bonobo_window_construct                      (BonoboWindow      *win,
								   const char        *win_name,
								   const char        *title);

GtkWidget           *bonobo_window_new                            (const char        *win_name,
								   const char        *title);

void                 bonobo_window_set_contents                   (BonoboWindow      *win,
								   GtkWidget         *contents);
GtkWidget           *bonobo_window_get_contents                   (BonoboWindow      *win);


BonoboUIEngine      *bonobo_window_get_ui_engine                  (BonoboWindow      *win);

void                 bonobo_window_set_name                       (BonoboWindow      *win,
								   const char        *win_name);

char                *bonobo_window_get_name                       (BonoboWindow      *win);

GtkAccelGroup       *bonobo_window_get_accel_group                (BonoboWindow      *win);

/* Mostly deprecated API; see bonobo-ui-engine.h */
void                 bonobo_window_freeze                         (BonoboWindow      *win);

void                 bonobo_window_thaw                           (BonoboWindow      *win);

BonoboUIError        bonobo_window_xml_merge                      (BonoboWindow      *win,
								   const char        *path,
								   const char        *xml,
								   const char        *component);

BonoboUIError        bonobo_window_xml_merge_tree                 (BonoboWindow      *win,
								   const char        *path,
								   BonoboUINode      *tree,
								   const char        *component);

char                *bonobo_window_xml_get                        (BonoboWindow      *win,
								   const char        *path,
								   gboolean           node_only);

gboolean             bonobo_window_xml_node_exists                (BonoboWindow      *win,
								   const char        *path);

BonoboUIError        bonobo_window_xml_rm                         (BonoboWindow      *win,
								   const char        *path,
								   const char        *by_component);

BonoboUIError        bonobo_window_object_set                     (BonoboWindow      *win,
								   const char        *path,
								   Bonobo_Unknown     object,
								   CORBA_Environment *ev);

BonoboUIError        bonobo_window_object_get                     (BonoboWindow      *win,
								   const char        *path,
								   Bonobo_Unknown    *object,
								   CORBA_Environment *ev);

void                 bonobo_window_dump                           (BonoboWindow      *win,
								   const char        *msg);

void                 bonobo_window_register_component             (BonoboWindow      *win,
								   const char        *name,
								   Bonobo_Unknown     component);

void                 bonobo_window_deregister_component           (BonoboWindow      *win,
								   const char        *name);

void                 bonobo_window_deregister_component_by_ref    (BonoboWindow      *win,
								   Bonobo_Unknown     component);

void                 bonobo_window_deregister_dead_components     (BonoboWindow      *win);

GList               *bonobo_window_deregister_get_component_names (BonoboWindow      *win);

Bonobo_Unknown       bonobo_window_component_get                  (BonoboWindow      *win,
								   const char        *name);

void                 bonobo_window_add_popup                      (BonoboWindow      *win,
								   GtkMenu           *popup,
								   const char        *path);

void                 bonobo_window_set_ui_container               (BonoboWindow      *win,
								   BonoboObject      *container);

/*
 * NB. popups are automaticaly removed on destroy, you probably don't
 * want to use this.
 */
void                 bonobo_window_remove_popup                   (BonoboWindow      *win,
								   const char        *path);

#endif /* _BONOBO_WINDOW_H_ */
