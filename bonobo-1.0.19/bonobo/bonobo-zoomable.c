/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *  Bonobo::Zoomable - zoomable interface for Controls.
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *                2000 SuSE GmbH.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Maciej Stachowiak <mjs@eazel.com>
 *           Martin Baulig <baulig@suse.de>
 *
 */

#include <config.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-zoomable.h>
#include <bonobo/bonobo-property-bag.h>
#include <gtk/gtksignal.h>

#undef ZOOMABLE_DEBUG

static BonoboObjectClass   *bonobo_zoomable_parent_class;
static BonoboZoomableClass *bonobo_zoomable_class;

struct _BonoboZoomablePrivate {
	CORBA_float		 zoom_level;

	CORBA_float		 min_zoom_level;
	CORBA_float		 max_zoom_level;
	CORBA_boolean		 has_min_zoom_level;
	CORBA_boolean		 has_max_zoom_level;
	CORBA_boolean		 is_continuous;

	GArray                  *preferred_zoom_levels;
	GArray                  *preferred_zoom_level_names;

	Bonobo_ZoomableFrame	 zoomable_frame;
};

enum {
	SET_FRAME,
	SET_ZOOM_LEVEL,
	ZOOM_IN,
	ZOOM_OUT,
	ZOOM_TO_FIT,
	ZOOM_TO_DEFAULT,
	LAST_SIGNAL
};

enum {
	ARG_0,
	ARG_ZOOM_LEVEL,
	ARG_MIN_ZOOM_LEVEL,
	ARG_MAX_ZOOM_LEVEL,
	ARG_HAS_MIN_ZOOM_LEVEL,
	ARG_HAS_MAX_ZOOM_LEVEL,
	ARG_IS_CONTINUOUS
};

static guint signals[LAST_SIGNAL];

typedef struct {
	POA_Bonobo_Zoomable	servant;
	
	BonoboZoomable		*gtk_object;
} impl_POA_Bonobo_Zoomable;

POA_Bonobo_Zoomable__vepv bonobo_zoomable_vepv;

#define CLASS(o) BONOBO_ZOOMABLE_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboZoomable *
bonobo_zoomable_from_servant (PortableServer_Servant servant)
{
	if (!BONOBO_IS_ZOOMABLE (bonobo_object_from_servant (servant)))
		return NULL;
	else
		return BONOBO_ZOOMABLE (bonobo_object_from_servant (servant));
}

static CORBA_float
impl_Bonobo_Zoomable__get_level (PortableServer_Servant  servant,
				 CORBA_Environment      *ev)
{
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	return zoomable->priv->zoom_level;
}

static CORBA_float
impl_Bonobo_Zoomable__get_minLevel (PortableServer_Servant  servant,
				    CORBA_Environment      *ev)
{
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	return zoomable->priv->min_zoom_level;
}

static CORBA_float
impl_Bonobo_Zoomable__get_maxLevel (PortableServer_Servant  servant,
				    CORBA_Environment      *ev)
{
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	return zoomable->priv->max_zoom_level;
}

static CORBA_boolean
impl_Bonobo_Zoomable__get_hasMinLevel (PortableServer_Servant  servant,
				       CORBA_Environment      *ev)
{
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	return zoomable->priv->has_min_zoom_level;
}

static CORBA_boolean
impl_Bonobo_Zoomable__get_hasMaxLevel (PortableServer_Servant  servant,
				       CORBA_Environment      *ev)
{
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	return zoomable->priv->has_max_zoom_level;
}

static CORBA_boolean
impl_Bonobo_Zoomable__get_isContinuous (PortableServer_Servant  servant,
					CORBA_Environment      *ev)
{
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	return zoomable->priv->is_continuous;
}

static Bonobo_ZoomLevelList *
impl_Bonobo_Zoomable__get_preferredLevels (PortableServer_Servant  servant,
					   CORBA_Environment      *ev)
{
	Bonobo_ZoomLevelList *list;
	BonoboZoomable *zoomable;
	float *zoom_levels;
	int num_zoom_levels;
	int i;

	zoomable = bonobo_zoomable_from_servant (servant);

	num_zoom_levels = zoomable->priv->preferred_zoom_levels->len;
	zoom_levels = (float *) zoomable->priv->preferred_zoom_levels->data;

	list = Bonobo_ZoomLevelList__alloc ();
	list->_maximum = zoomable->priv->preferred_zoom_levels->len;
	list->_length  = zoomable->priv->preferred_zoom_levels->len;
	list->_buffer  = CORBA_sequence_Bonobo_ZoomLevel_allocbuf (num_zoom_levels);

	for (i = 0; i < num_zoom_levels; ++i) {
		/* assigned one at a time to convert float to CORBA_float */
		list->_buffer [i] = zoom_levels [i];
	}
	
	CORBA_sequence_set_release (list, CORBA_TRUE);

	return list;
}

static Bonobo_ZoomLevelNameList *
impl_Bonobo_Zoomable__get_preferredLevelNames (PortableServer_Servant  servant,
					       CORBA_Environment      *ev)
{
	Bonobo_ZoomLevelNameList *list;
	BonoboZoomable *zoomable;
	gchar **zoom_level_names;
	int num_zoom_level_names;
	int i;

	zoomable = bonobo_zoomable_from_servant (servant);

	num_zoom_level_names = zoomable->priv->preferred_zoom_level_names->len;
	zoom_level_names = (gchar **) zoomable->priv->preferred_zoom_level_names->data;

	list = Bonobo_ZoomLevelNameList__alloc ();
	list->_maximum = zoomable->priv->preferred_zoom_level_names->len;
	list->_length  = zoomable->priv->preferred_zoom_level_names->len;
	list->_buffer  = CORBA_sequence_Bonobo_ZoomLevelName_allocbuf (num_zoom_level_names);

	for (i = 0; i < num_zoom_level_names; ++i) {
		list->_buffer [i] = CORBA_string_dup (zoom_level_names [i]);
	}

	CORBA_sequence_set_release (list, CORBA_TRUE);

	return list;
}

static void 
impl_Bonobo_Zoomable_setLevel (PortableServer_Servant  servant,
			       const CORBA_float       zoom_level,
			       CORBA_Environment      *ev)
{
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	gtk_signal_emit (GTK_OBJECT (zoomable), signals[SET_ZOOM_LEVEL], zoom_level);
}

static void
impl_Bonobo_Zoomable_zoomIn (PortableServer_Servant  servant,
			     CORBA_Environment      *ev)
{	
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	gtk_signal_emit (GTK_OBJECT (zoomable), signals[ZOOM_IN]);
}

static void
impl_Bonobo_Zoomable_zoomOut (PortableServer_Servant  servant,
			      CORBA_Environment      *ev)
{	
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	gtk_signal_emit (GTK_OBJECT (zoomable), signals[ZOOM_OUT]);
}

static void
impl_Bonobo_Zoomable_zoomFit (PortableServer_Servant  servant,
			      CORBA_Environment      *ev)
{	
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	gtk_signal_emit (GTK_OBJECT (zoomable), signals[ZOOM_TO_FIT]);
}

static void
impl_Bonobo_Zoomable_zoomDefault (PortableServer_Servant  servant,
				  CORBA_Environment      *ev)
{	
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);
	gtk_signal_emit (GTK_OBJECT (zoomable), signals[ZOOM_TO_DEFAULT]);
}

static void
impl_Bonobo_Zoomable_setFrame (PortableServer_Servant  servant,
			       Bonobo_ZoomableFrame    zoomable_frame,
			       CORBA_Environment      *ev)
{	
	BonoboZoomable *zoomable;

	zoomable = bonobo_zoomable_from_servant (servant);

	g_assert (zoomable->priv->zoomable_frame == CORBA_OBJECT_NIL);
	zoomable->priv->zoomable_frame = CORBA_Object_duplicate (zoomable_frame, ev);
	gtk_signal_emit (GTK_OBJECT (zoomable), signals[SET_FRAME]);
}


/**
 * bonobo_zoomable_get_epv:
 *
 * Returns: The EPV for the default BonoboZoomable implementation.  
 */
POA_Bonobo_Zoomable__epv *
bonobo_zoomable_get_epv (void)
{
	POA_Bonobo_Zoomable__epv *epv;

	epv = g_new0 (POA_Bonobo_Zoomable__epv, 1);

	epv->_get_level = impl_Bonobo_Zoomable__get_level;
	epv->_get_minLevel = impl_Bonobo_Zoomable__get_minLevel;
	epv->_get_maxLevel = impl_Bonobo_Zoomable__get_maxLevel;
	epv->_get_hasMinLevel = impl_Bonobo_Zoomable__get_hasMinLevel;
	epv->_get_hasMaxLevel = impl_Bonobo_Zoomable__get_hasMaxLevel;
	epv->_get_isContinuous = impl_Bonobo_Zoomable__get_isContinuous;
	epv->_get_preferredLevels = impl_Bonobo_Zoomable__get_preferredLevels;
	epv->_get_preferredLevelNames = impl_Bonobo_Zoomable__get_preferredLevelNames;

	epv->zoomIn      = impl_Bonobo_Zoomable_zoomIn;
	epv->zoomOut     = impl_Bonobo_Zoomable_zoomOut;
	epv->zoomFit     = impl_Bonobo_Zoomable_zoomFit;
	epv->zoomDefault = impl_Bonobo_Zoomable_zoomDefault;

	epv->setLevel = impl_Bonobo_Zoomable_setLevel;
	epv->setFrame = impl_Bonobo_Zoomable_setFrame;
	
	return epv;
}

static void
init_zoomable_corba_class (void)
{
	/* The VEPV */
	bonobo_zoomable_vepv.Bonobo_Unknown_epv  = bonobo_object_get_epv ();
	bonobo_zoomable_vepv.Bonobo_Zoomable_epv = bonobo_zoomable_get_epv ();
}

static void
marshal_NONE__FLOAT (GtkObject *object,
		      GtkSignalFunc func,
		      gpointer func_data,
		      GtkArg *args)
{
	(* (void (*)(GtkObject *, float, gpointer)) func)
		(object,
		 GTK_VALUE_FLOAT (args[0]),
		 func_data);
}

static void
bonobo_zoomable_get_arg (GtkObject* obj, GtkArg* arg, guint arg_id)
{
	BonoboZoomable *zoomable = BONOBO_ZOOMABLE (obj);
	BonoboZoomablePrivate *priv = zoomable->priv;

	switch (arg_id) {
	case ARG_ZOOM_LEVEL:
		GTK_VALUE_FLOAT(*arg) = priv->zoom_level;
		break;
	case ARG_MIN_ZOOM_LEVEL:
		GTK_VALUE_FLOAT(*arg) = priv->min_zoom_level;
		break;
	case ARG_MAX_ZOOM_LEVEL:
		GTK_VALUE_FLOAT(*arg) = priv->max_zoom_level;
		break;
	case ARG_HAS_MIN_ZOOM_LEVEL:
		GTK_VALUE_BOOL(*arg) = priv->has_min_zoom_level;
		break;
	case ARG_HAS_MAX_ZOOM_LEVEL:
		GTK_VALUE_BOOL(*arg) = priv->has_max_zoom_level;
		break;
	case ARG_IS_CONTINUOUS:
		GTK_VALUE_BOOL(*arg) = priv->is_continuous;
		break;
	default:
		g_message ("Unknown arg_id `%d'", arg_id);
		break;
	};
}

static void
bonobo_zoomable_free_preferred_zoom_level_arrays (BonoboZoomable *zoomable)
{
	int i;
	gchar **zoom_level_names;

	zoom_level_names = (gchar **) zoomable->priv->preferred_zoom_level_names->data;

	for (i = 0; i < zoomable->priv->preferred_zoom_level_names->len; ++i) {
		g_free (zoom_level_names [i]);
	}
	
	g_array_free (zoomable->priv->preferred_zoom_level_names, TRUE);
	zoomable->priv->preferred_zoom_level_names = NULL;

	g_array_free (zoomable->priv->preferred_zoom_levels, TRUE);
	zoomable->priv->preferred_zoom_levels = NULL;
}

static void
bonobo_zoomable_destroy (GtkObject *object)
{
	BonoboZoomable *zoomable;

	g_return_if_fail (BONOBO_IS_ZOOMABLE (object));

	zoomable = BONOBO_ZOOMABLE (object);

	bonobo_zoomable_free_preferred_zoom_level_arrays (zoomable);

	GTK_OBJECT_CLASS (bonobo_zoomable_parent_class)->destroy (object);
}

static void
bonobo_zoomable_finalize (GtkObject *object)
{
	BonoboZoomable *zoomable;

	g_return_if_fail (BONOBO_IS_ZOOMABLE (object));

	zoomable = BONOBO_ZOOMABLE (object);

	g_free (zoomable->priv);
	zoomable->priv = NULL;

	GTK_OBJECT_CLASS (bonobo_zoomable_parent_class)->finalize (object);
}

static void
bonobo_zoomable_class_init (BonoboZoomableClass *klass)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass*) klass;
	
	bonobo_zoomable_parent_class = gtk_type_class (bonobo_object_get_type ());
	bonobo_zoomable_class = klass;

	gtk_object_add_arg_type("BonoboZoomable::zoom_level",
				GTK_TYPE_FLOAT, GTK_ARG_READABLE, ARG_ZOOM_LEVEL);
	gtk_object_add_arg_type("BonoboZoomable::min_zoom_level",
				GTK_TYPE_FLOAT, GTK_ARG_READABLE, ARG_MIN_ZOOM_LEVEL);
	gtk_object_add_arg_type("BonoboZoomable::max_zoom_level",
				GTK_TYPE_FLOAT, GTK_ARG_READABLE, ARG_MAX_ZOOM_LEVEL);
	gtk_object_add_arg_type("BonoboZoomable::has_min_zoom_level",
				GTK_TYPE_FLOAT, GTK_ARG_READABLE, ARG_HAS_MIN_ZOOM_LEVEL);
	gtk_object_add_arg_type("BonoboZoomable::has_max_zoom_level",
				GTK_TYPE_FLOAT, GTK_ARG_READABLE, ARG_HAS_MAX_ZOOM_LEVEL);
	gtk_object_add_arg_type("BonoboZoomable::is_continuous",
				GTK_TYPE_FLOAT, GTK_ARG_READABLE, ARG_IS_CONTINUOUS);

	signals[SET_FRAME] =
		gtk_signal_new ("set_frame",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableClass, set_frame),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	signals[SET_ZOOM_LEVEL] =
		gtk_signal_new ("set_zoom_level",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableClass, set_zoom_level),
				marshal_NONE__FLOAT,
				GTK_TYPE_NONE, 1, GTK_TYPE_FLOAT);
	signals[ZOOM_IN] = 
		gtk_signal_new ("zoom_in",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableClass, zoom_in),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	signals[ZOOM_OUT] = 
		gtk_signal_new ("zoom_out",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableClass, zoom_out),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	signals[ZOOM_TO_FIT] = 
		gtk_signal_new ("zoom_to_fit",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableClass, zoom_to_fit),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	signals[ZOOM_TO_DEFAULT] = 
		gtk_signal_new ("zoom_to_default",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableClass, zoom_to_default),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->get_arg = bonobo_zoomable_get_arg;

	object_class->destroy = bonobo_zoomable_destroy;
	object_class->finalize = bonobo_zoomable_finalize;

	init_zoomable_corba_class ();
}

static void
bonobo_zoomable_init (BonoboZoomable *zoomable)
{
	zoomable->priv = g_new0 (BonoboZoomablePrivate, 1);

	zoomable->priv->zoom_level = 0.0;
	zoomable->priv->min_zoom_level = 0.0;
	zoomable->priv->max_zoom_level = 0.0;
	zoomable->priv->has_min_zoom_level = FALSE;
	zoomable->priv->has_max_zoom_level = FALSE;
	zoomable->priv->is_continuous = TRUE;
	zoomable->priv->preferred_zoom_levels = g_array_new (FALSE, TRUE, sizeof (float));
	zoomable->priv->preferred_zoom_level_names = g_array_new (FALSE, TRUE, sizeof (gchar *));
}

/**
 * bonobo_zoomable_get_type:
 *
 * Returns: the GtkType for a BonoboZoomable object.
 */
GtkType
bonobo_zoomable_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboZoomable",
			sizeof (BonoboZoomable),
			sizeof (BonoboZoomableClass),
			(GtkClassInitFunc) bonobo_zoomable_class_init,
			(GtkObjectInitFunc) bonobo_zoomable_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

Bonobo_Zoomable
bonobo_zoomable_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Zoomable *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Zoomable *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_zoomable_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Zoomable__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);
	return (Bonobo_Zoomable) bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_zoomable_set_parameters_full:
 * 
 * This is used by the component to set new zooming parameters (and to set the
 * initial zooming parameters including the initial zoom level after creating
 * the BonoboZoomable) - for instance after loading a new file.
 *
 * If any of the zoom parameters such as the minimum or maximum zoom level has
 * changed, it is likely that the zoom level has become invalid as well - at
 * least, the container must query it in any case, so we set it here.
 * 
 * Return value: 
 **/
void
bonobo_zoomable_set_parameters_full (BonoboZoomable  *zoomable,
				     float            zoom_level,
				     float            min_zoom_level,
				     float            max_zoom_level,
				     gboolean         has_min_zoom_level,
				     gboolean         has_max_zoom_level,
				     gboolean         is_continuous,
				     float           *preferred_zoom_levels,
				     const gchar    **preferred_zoom_level_names,
				     gint             num_preferred_zoom_levels)
{
	int i;
	gchar **zoom_level_names;
	BonoboZoomable *p = zoomable; 

	g_return_if_fail (BONOBO_IS_ZOOMABLE (p));

	p->priv->zoom_level = zoom_level;
	p->priv->min_zoom_level = min_zoom_level;
	p->priv->max_zoom_level = max_zoom_level;
	p->priv->has_min_zoom_level = has_min_zoom_level;
	p->priv->has_max_zoom_level = has_max_zoom_level;
	p->priv->is_continuous = is_continuous;

	bonobo_zoomable_free_preferred_zoom_level_arrays (p);
	
	p->priv->preferred_zoom_levels = g_array_new (FALSE, TRUE, sizeof (float));
	
	if (preferred_zoom_levels) {
		g_array_append_vals (p->priv->preferred_zoom_levels,
				     preferred_zoom_levels,
				     num_preferred_zoom_levels);
	}
	
	p->priv->preferred_zoom_level_names = g_array_new (FALSE, TRUE, sizeof (gchar *));
	
	if (preferred_zoom_level_names) {
		g_array_set_size (p->priv->preferred_zoom_levels, num_preferred_zoom_levels);
		zoom_level_names = (gchar **) p->priv->preferred_zoom_level_names->data;
		for (i = 0; i < p->priv->preferred_zoom_level_names->len; ++i) {
			zoom_level_names [i] = g_strdup (preferred_zoom_level_names [i]);
		}
	}
}

/**
 * bonobo_zoomable_set_parameters:
 * 
 * This is a simple version of @bonobo_zoomable_set_parameters_full() for components
 * which support continuous zooming. It does not override any of the parameters
 * which can only be set by the _full version.
 * 
 * Return value: 
 **/
void
bonobo_zoomable_set_parameters (BonoboZoomable  *zoomable,
                                float            zoom_level,
                                float            min_zoom_level,
                                float            max_zoom_level,
                                gboolean         has_min_zoom_level,
                                gboolean         has_max_zoom_level)
{
	BonoboZoomable *p = zoomable;

	g_return_if_fail (BONOBO_IS_ZOOMABLE (p));

	p->priv->zoom_level = zoom_level;
	p->priv->min_zoom_level = min_zoom_level;
	p->priv->max_zoom_level = max_zoom_level;
	p->priv->has_min_zoom_level = has_min_zoom_level;
	p->priv->has_max_zoom_level = has_max_zoom_level;
}

/**
 * bonobo_zoomable_add_preferred_zoom_level:
 * @zoomable: the zoomable
 * @zoom_level: the new level
 * @zoom_level_name: the new level's name
 * 
 * This appends a new zoom level's name and value to the
 * internal list of these.
 **/
void
bonobo_zoomable_add_preferred_zoom_level (BonoboZoomable *zoomable,
                                          float           zoom_level,
                                          const gchar    *zoom_level_name)
{
	gchar *name;
	CORBA_float level;
	
	/* convert zoom_level to a CORBA_float */
	level = zoom_level;
	g_array_append_val (zoomable->priv->preferred_zoom_levels, level);

	name = g_strdup (zoom_level_name);
	g_array_append_val (zoomable->priv->preferred_zoom_level_names, name);
}

/**
 * bonobo_zoomable_construct:
 * @zoomable: the zoomable
 * @corba_p: the corba object reference.
 * 
 * Construct a newly created BonoboZoomable pointed to
 * by @zoomable, and associate it with @corba_p
 * 
 * Return value: a newly constructed zoomable or NULL on error
 **/
BonoboZoomable *
bonobo_zoomable_construct (BonoboZoomable	*zoomable,
			   Bonobo_Zoomable	 corba_p)
{
	BonoboZoomable *p = zoomable;

	g_return_val_if_fail (p != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE (p), NULL);
	g_return_val_if_fail (corba_p != NULL, NULL);

	bonobo_object_construct (BONOBO_OBJECT (p), corba_p);

	return p;
}

/**
 * bonobo_zoomable_new:
 * 
 * Create a new bonobo-zoomable implementing BonoboObject
 * interface.
 * 
 * Return value: 
 **/
BonoboZoomable *
bonobo_zoomable_new (void)
{
	BonoboZoomable	*p;
	Bonobo_Zoomable	 corba_p;

	p = gtk_type_new (bonobo_zoomable_get_type ());
	g_return_val_if_fail (p != NULL, NULL);

	corba_p = bonobo_zoomable_corba_object_create (BONOBO_OBJECT (p));
	if (corba_p == CORBA_OBJECT_NIL){
		bonobo_object_unref (BONOBO_OBJECT (p));
		return NULL;
	}

	return bonobo_zoomable_construct (p, corba_p);
}

/**
 * bonobo_zoomable_report_zoom_level_changed:
 *
 * @new_zoom_level: The new zoom level.
 * 
 * Reports the BonoboZoomableFrame that the zoom level has changed (but the
 * other zoom parameters are still the same).
 *
 * This is called after the component has successfully completed a zooming
 * operation - the @new_zoom_level may have been modified from what the
 * container requested to match what the component actually displays at the
 * moment.
 * 
 * Return value: 
 **/
void
bonobo_zoomable_report_zoom_level_changed (BonoboZoomable *zoomable,
					   float           new_zoom_level)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_ZOOMABLE (zoomable));

	zoomable->priv->zoom_level = new_zoom_level;

	if (zoomable->priv->zoomable_frame == CORBA_OBJECT_NIL)
		return;

	CORBA_exception_init (&ev);
	Bonobo_ZoomableFrame_onLevelChanged (zoomable->priv->zoomable_frame,
					     zoomable->priv->zoom_level,
					     &ev);
	CORBA_exception_free (&ev);
}

/**
 * bonobo_zoomable_report_zoom_parameters_changed:
 *
 * Reports the BonoboZoomableFrame that the zoom parameters have changed;
 * this also includes the zoom level.
 *
 * On the container side (the BonoboZoomableFrame) this implies that the
 * zoom level has changed as well, so you need to query the BonoboZoomable
 * for the new zoom level as well.
 * 
 * Return value: 
 **/
void
bonobo_zoomable_report_zoom_parameters_changed (BonoboZoomable *zoomable)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_ZOOMABLE (zoomable));

	if (zoomable->priv->zoomable_frame == CORBA_OBJECT_NIL)
		return;

	CORBA_exception_init (&ev);
	Bonobo_ZoomableFrame_onParametersChanged (zoomable->priv->zoomable_frame,
							     &ev);
	CORBA_exception_free (&ev);
}
