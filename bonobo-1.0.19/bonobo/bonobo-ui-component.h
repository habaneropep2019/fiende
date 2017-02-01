/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-component.h: Client UI signal multiplexer and verb repository.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_UI_COMPONENT_H_
#define _BONOBO_UI_COMPONENT_H_

#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-ui-node.h>

BEGIN_GNOME_DECLS

#define BONOBO_UI_COMPONENT_TYPE        (bonobo_ui_component_get_type ())
#define BONOBO_UI_COMPONENT(o)          (GTK_CHECK_CAST ((o), BONOBO_UI_COMPONENT_TYPE, BonoboUIComponent))
#define BONOBO_UI_COMPONENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_UI_COMPONENT_TYPE, BonoboUIComponentClass))
#define BONOBO_IS_UI_COMPONENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_UI_COMPONENT_TYPE))
#define BONOBO_IS_UI_COMPONENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_UI_COMPONENT_TYPE))

typedef struct _BonoboUIComponent BonoboUIComponent;
typedef struct _BonoboUIComponentPrivate BonoboUIComponentPrivate;

typedef void (*BonoboUIListenerFn) (BonoboUIComponent           *component,
				    const char                  *path,
				    Bonobo_UIComponent_EventType type,
				    const char                  *state,
				    gpointer                     user_data);

typedef void (*BonoboUIVerbFn)    (BonoboUIComponent           *component,
				   gpointer                     user_data,
				   const char                  *cname);

struct _BonoboUIComponent {
	BonoboXObject             object;
	BonoboUIComponentPrivate *priv;
};

typedef struct {
	BonoboXObjectClass          parent_class;

	POA_Bonobo_UIComponent__epv epv;

	/* Signals */
	void (*exec_verb) (BonoboUIComponent *comp,
			   const char        *cname);

	void (*ui_event)  (BonoboUIComponent *comp,
			   const char        *path,
			   Bonobo_UIComponent_EventType type,
			   const char        *state);
	/* Virtual XML Methods */
	void (*freeze)    (BonoboUIComponent *component,
			   CORBA_Environment *opt_ev);

	void (*thaw)      (BonoboUIComponent *component,
			   CORBA_Environment *opt_ev);

	void (*xml_set)   (BonoboUIComponent *component,
			   const char        *path,
			   const char        *xml,
			   CORBA_Environment *ev);

	CORBA_char *(*xml_get) (BonoboUIComponent *component,
				const char        *path,
				gboolean           recurse,
				CORBA_Environment *ev);

	void (*xml_rm)    (BonoboUIComponent *component,
			   const char        *path,
			   CORBA_Environment *ev);

	void (*set_prop)  (BonoboUIComponent *component,
			   const char        *path,
			   const char        *prop,
			   const char        *value,
			   CORBA_Environment *opt_ev);
	
	gchar *(*get_prop) (BonoboUIComponent *component,
			    const char        *path,
			    const char        *prop,
			    CORBA_Environment *opt_ev);

	gboolean (*exists) (BonoboUIComponent *component,
			    const char        *path,
			    CORBA_Environment *ev);
	

	gpointer            dummy;
} BonoboUIComponentClass;

GtkType            bonobo_ui_component_get_type        (void);

BonoboUIComponent *bonobo_ui_component_construct       (BonoboUIComponent  *component,
							const char         *name);

BonoboUIComponent *bonobo_ui_component_new             (const char         *name);
BonoboUIComponent *bonobo_ui_component_new_default     (void);

void               bonobo_ui_component_set_name        (BonoboUIComponent  *component,
							const char         *name);
const char        *bonobo_ui_component_get_name        (BonoboUIComponent  *component);

void               bonobo_ui_component_set_container   (BonoboUIComponent  *component,
							Bonobo_UIContainer  container);
void               bonobo_ui_component_unset_container (BonoboUIComponent  *component);
Bonobo_UIContainer bonobo_ui_component_get_container   (BonoboUIComponent  *component);

void               bonobo_ui_component_add_verb     (BonoboUIComponent  *component,
						     const char         *cname,
						     BonoboUIVerbFn      fn,
						     gpointer            user_data);

void               bonobo_ui_component_add_verb_full(BonoboUIComponent  *component,
						     const char         *cname,
						     BonoboUIVerbFn      fn,
						     gpointer            user_data,
						     GDestroyNotify      destroy_fn);

void               bonobo_ui_component_remove_verb         (BonoboUIComponent  *component,
							    const char         *cname);

void               bonobo_ui_component_remove_verb_by_func (BonoboUIComponent  *component,
							    BonoboUIVerbFn      fn);

void               bonobo_ui_component_remove_verb_by_data (BonoboUIComponent  *component,
							    gpointer            user_data);

void               bonobo_ui_component_add_listener (BonoboUIComponent  *component,
						     const char         *id,
						     BonoboUIListenerFn  fn,
						     gpointer            user_data);

void               bonobo_ui_component_add_listener_full (BonoboUIComponent  *component,
							  const char         *id,
							  BonoboUIListenerFn  fn,
							  gpointer            user_data,
							  GDestroyNotify      destroy_fn);

void               bonobo_ui_component_remove_listener         (BonoboUIComponent  *component,
								const char         *cname);

void               bonobo_ui_component_remove_listener_by_func (BonoboUIComponent  *component,
								BonoboUIListenerFn  fn);

void               bonobo_ui_component_remove_listener_by_data (BonoboUIComponent  *component,
								gpointer            user_data);

void               bonobo_ui_component_set          (BonoboUIComponent  *component,
						     const char         *path,
						     const char         *xml,
						     CORBA_Environment  *ev);

void               bonobo_ui_component_set_translate(BonoboUIComponent  *component,
						     const char         *path,
						     const char         *xml,
						     CORBA_Environment  *ev);

void               bonobo_ui_component_set_tree     (BonoboUIComponent  *component,
						     const char         *path,
						     BonoboUINode       *node,
						     CORBA_Environment  *ev);

void               bonobo_ui_component_rm           (BonoboUIComponent  *component,
						     const char         *path,
						     CORBA_Environment  *ev);

gboolean           bonobo_ui_component_path_exists  (BonoboUIComponent  *component,
						     const char         *path,
						     CORBA_Environment  *ev);

CORBA_char        *bonobo_ui_component_get          (BonoboUIComponent  *component,
						     const char         *path,
						     gboolean            recurse,
						     CORBA_Environment  *ev);

BonoboUINode      *bonobo_ui_component_get_tree     (BonoboUIComponent  *component,
						     const char         *path,
						     gboolean            recurse,
						     CORBA_Environment  *ev);

void               bonobo_ui_component_object_set   (BonoboUIComponent  *component,
						     const char         *path,
						     Bonobo_Unknown      control,
						     CORBA_Environment  *ev);

Bonobo_Unknown     bonobo_ui_component_object_get   (BonoboUIComponent  *component,
						     const char         *path,
						     CORBA_Environment  *opt_ev);

void               bonobo_ui_component_freeze       (BonoboUIComponent  *component,
						     CORBA_Environment  *opt_ev);

void               bonobo_ui_component_thaw         (BonoboUIComponent  *component,
						     CORBA_Environment  *opt_ev);

void               bonobo_ui_component_set_prop     (BonoboUIComponent  *component,
						     const char         *path,
						     const char         *prop,
						     const char         *value,
						     CORBA_Environment  *opt_ev);

gchar             *bonobo_ui_component_get_prop     (BonoboUIComponent  *component,
						     const char         *path,
						     const char         *prop,
						     CORBA_Environment  *opt_ev);

void               bonobo_ui_component_set_status   (BonoboUIComponent  *component,
						     const char         *text,
						     CORBA_Environment  *opt_ev);

typedef struct {
	char          *cname;
	BonoboUIVerbFn cb;
	gpointer       user_data;
	gpointer       dummy;
} BonoboUIVerb;

#define BONOBO_UI_VERB(name,cb)                  { (name), (cb), NULL   }
#define BONOBO_UI_VERB_DATA(name,cb,data)        { (name), (cb), (data) }
#define BONOBO_UI_UNSAFE_VERB(name,cb)           { (name), ((BonoboUIVerbFn)(cb)), NULL   }
#define BONOBO_UI_UNSAFE_VERB_DATA(name,cb,data) { (name), ((BonoboUIVerbFn)(cb)), (data) }
#define BONOBO_UI_VERB_END                       { NULL, NULL, NULL }

void    bonobo_ui_component_add_verb_list           (BonoboUIComponent  *component,
						     BonoboUIVerb       *list);
void    bonobo_ui_component_add_verb_list_with_data (BonoboUIComponent  *component,
						     BonoboUIVerb       *list,
						     gpointer            user_data);

END_GNOME_DECLS

#endif /* _BONOBO_UI_COMPONENT_H_ */
