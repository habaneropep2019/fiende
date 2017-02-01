/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo Unknown interface base implementation
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999,2000 Helix Code, Inc.
 */
#ifndef _BONOBO_OBJECT_H_
#define _BONOBO_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>

BEGIN_GNOME_DECLS

#undef BONOBO_OBJECT_DEBUG
 
#define BONOBO_OBJECT_TYPE        (bonobo_object_get_type ())
#define BONOBO_OBJECT(o)          (GTK_CHECK_CAST ((o), BONOBO_OBJECT_TYPE, BonoboObject))
#define BONOBO_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_OBJECT_TYPE, BonoboObjectClass))
#define BONOBO_IS_OBJECT(o)       (GTK_CHECK_TYPE ((o), BONOBO_OBJECT_TYPE))
#define BONOBO_IS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_OBJECT_TYPE))

#define BONOBO_OBJREF(o)          (bonobo_object_corba_objref(BONOBO_OBJECT(o)))
#define BONOBO_OBJECT_IS_LOCAL(o) ((o)->servant && (o)->vepv)

/*
 * If you're using a custom servant for your CORBA objects, just make
 * sure that the second element is a 'gpointer' to hold the BonoboObject
 * pointer for servant->BonoboObject translation
 */
typedef struct {
	POA_Bonobo_Unknown servant_placeholder;
	gpointer           bonobo_object;
} BonoboObjectServant;

typedef struct _BonoboObjectPrivate BonoboObjectPrivate;

typedef struct {
	GtkObject            base;

	Bonobo_Unknown       corba_objref;
	gpointer             servant;
	BonoboObjectPrivate *priv;
} BonoboObject;

typedef struct {
	GtkObjectClass parent_class;

	/*
	 * signals.  
	 */
	void  (*query_interface) (BonoboObject *object, const char *repo_id,  CORBA_Object      *retval);
	void  (*system_exception)(BonoboObject *object, CORBA_Object cobject, CORBA_Environment *ev);

	gpointer expansion; /* Used by XObject */
} BonoboObjectClass;

GtkType                  bonobo_object_get_type               (void);
BonoboObject            *bonobo_object_construct              (BonoboObject           *object,
							       CORBA_Object            corba_object);
BonoboObject            *bonobo_object_new_from_servant       (void                   *servant);
BonoboObject            *bonobo_object_from_servant           (PortableServer_Servant  servant);
void                     bonobo_object_bind_to_servant        (BonoboObject           *object,
							       void                   *servant);
PortableServer_Servant   bonobo_object_get_servant            (BonoboObject           *object);
Bonobo_Unknown           bonobo_object_activate_servant_full  (BonoboObject           *object,
							       void                   *servant,
							       gpointer shlib_id);
Bonobo_Unknown           bonobo_object_activate_servant       (BonoboObject           *object,
							       void                   *servant);
void                     bonobo_object_add_interface          (BonoboObject           *object,
							       BonoboObject           *newobj);
BonoboObject            *bonobo_object_query_local_interface  (BonoboObject           *object,
							       const char             *repo_id);
Bonobo_Unknown           bonobo_object_query_interface        (BonoboObject           *object,
							       const char             *repo_id);
Bonobo_Unknown           bonobo_object_corba_objref           (BonoboObject           *object);

/*
 * Gnome Object Life Cycle
 */
Bonobo_Unknown           bonobo_object_dup_ref                (Bonobo_Unknown          object,
							       CORBA_Environment      *ev);
void                     bonobo_object_release_unref          (Bonobo_Unknown          object,
							       CORBA_Environment      *ev);
void                     bonobo_object_ref                    (BonoboObject           *object);
void                     bonobo_object_idle_unref             (BonoboObject           *object);
void                     bonobo_object_unref                  (BonoboObject           *object);
POA_Bonobo_Unknown__epv *bonobo_object_get_epv                (void);
void                     bonobo_object_init                   (void);
void                     bonobo_object_trace_refs             (BonoboObject *object,
							       const char   *fn,
							       int           line,
							       gboolean      ref);
#ifdef BONOBO_OBJECT_DEBUG
#	define           bonobo_object_ref(o)   G_STMT_START{bonobo_object_trace_refs((o),G_GNUC_PRETTY_FUNCTION,__LINE__,TRUE);}G_STMT_END
#	define           bonobo_object_unref(o) G_STMT_START{bonobo_object_trace_refs((o),G_GNUC_PRETTY_FUNCTION,__LINE__,FALSE);}G_STMT_END
#endif	/* BONOBO_OBJECT_DEBUG */
void                     bonobo_object_dump_interfaces        (BonoboObject *object);

/*
 * Error checking
 */
void                     bonobo_object_check_env              (BonoboObject           *object,
							       CORBA_Object            corba_object,
							       CORBA_Environment      *ev);

#define BONOBO_OBJECT_CHECK(o,c,e)				\
			G_STMT_START {				\
			if ((e)->_major != CORBA_NO_EXCEPTION)	\
				bonobo_object_check_env(o,c,e);	\
			} G_STMT_END

/*
 * Others
 */
gboolean  bonobo_unknown_ping           (Bonobo_Unknown object);
void      bonobo_object_list_unref_all  (GList        **list);
void      bonobo_object_slist_unref_all (GSList       **list);

END_GNOME_DECLS

#endif
