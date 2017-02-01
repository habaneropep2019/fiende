/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *  Bonobo::ZoomableFrame - container side part of Bonobo::Zoomable.
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
#include <bonobo/bonobo-zoomable-frame.h>
#include <gtk/gtksignal.h>

#undef ZOOMABLE_DEBUG

static BonoboObjectClass   *bonobo_zoomable_frame_parent_class;
static BonoboZoomableFrameClass *bonobo_zoomable_frame_class;

struct _BonoboZoomableFramePrivate {
	Bonobo_Zoomable			 zoomable;
};

enum {
	ZOOM_LEVEL_CHANGED,
	ZOOM_PARAMETERS_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

typedef struct {
	POA_Bonobo_ZoomableFrame	 servant;

	BonoboZoomableFrame		*gtk_object;
} impl_POA_Bonobo_ZoomableFrame;

POA_Bonobo_ZoomableFrame__vepv bonobo_zoomable_frame_vepv;

static inline BonoboZoomableFrame *
bonobo_zoomable_frame_from_servant (PortableServer_Servant servant)
{
	if (!BONOBO_IS_ZOOMABLE_FRAME (bonobo_object_from_servant (servant)))
		return NULL;
	else
		return BONOBO_ZOOMABLE_FRAME (bonobo_object_from_servant (servant));
}

static void 
impl_Bonobo_ZoomableFrame_onLevelChanged (PortableServer_Servant  servant,
						     const CORBA_float      zoom_level,
						     CORBA_Environment      *ev)
{
	BonoboZoomableFrame *zoomable_frame;

	zoomable_frame = bonobo_zoomable_frame_from_servant (servant);
	gtk_signal_emit (GTK_OBJECT (zoomable_frame), signals[ZOOM_LEVEL_CHANGED],
			 zoom_level);
}

static void 
impl_Bonobo_ZoomableFrame_onParametersChanged (PortableServer_Servant  servant,
							  CORBA_Environment      *ev)
{
	BonoboZoomableFrame *zoomable_frame;

	zoomable_frame = bonobo_zoomable_frame_from_servant (servant);
	gtk_signal_emit (GTK_OBJECT (zoomable_frame), signals[ZOOM_PARAMETERS_CHANGED]);
}


/**
 * bonobo_zoomable_frame_get_epv:
 *
 * Returns: The EPV for the default BonoboZoomableFrame implementation.  
 */
POA_Bonobo_ZoomableFrame__epv *
bonobo_zoomable_frame_get_epv (void)
{
	POA_Bonobo_ZoomableFrame__epv *epv;

	epv = g_new0 (POA_Bonobo_ZoomableFrame__epv, 1);

	epv->onLevelChanged = impl_Bonobo_ZoomableFrame_onLevelChanged;
	epv->onParametersChanged = impl_Bonobo_ZoomableFrame_onParametersChanged;

	return epv;
}

static void
init_zoomable_corba_class (void)
{
	/* The VEPV */
	bonobo_zoomable_frame_vepv.Bonobo_Unknown_epv  = bonobo_object_get_epv ();
	bonobo_zoomable_frame_vepv.Bonobo_ZoomableFrame_epv = bonobo_zoomable_frame_get_epv ();
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
bonobo_zoomable_frame_destroy (GtkObject *object)
{
	BonoboZoomableFrame *zoomable_frame;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (object));

	zoomable_frame = BONOBO_ZOOMABLE_FRAME (object);

	if (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (zoomable_frame->priv->zoomable, NULL);
	zoomable_frame->priv->zoomable = CORBA_OBJECT_NIL;

	GTK_OBJECT_CLASS (bonobo_zoomable_frame_parent_class)->destroy (object);
}

static void
bonobo_zoomable_frame_finalize (GtkObject *object)
{
	BonoboZoomableFrame *zoomable_frame;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (object));

	zoomable_frame = BONOBO_ZOOMABLE_FRAME (object);

	g_free (zoomable_frame->priv);
	zoomable_frame->priv = NULL;

	GTK_OBJECT_CLASS (bonobo_zoomable_frame_parent_class)->finalize (object);
}

static void
bonobo_zoomable_frame_class_init (BonoboZoomableFrameClass *klass)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass*) klass;
	
	bonobo_zoomable_frame_parent_class = gtk_type_class (bonobo_object_get_type ());
	bonobo_zoomable_frame_class = klass;

	signals[ZOOM_LEVEL_CHANGED] =
		gtk_signal_new ("zoom_level_changed",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableFrameClass, zoom_level_changed),
				marshal_NONE__FLOAT,
				GTK_TYPE_NONE, 1, GTK_TYPE_FLOAT);
	signals[ZOOM_PARAMETERS_CHANGED] =
		gtk_signal_new ("zoom_parameters_changed",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboZoomableFrameClass, zoom_parameters_changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = bonobo_zoomable_frame_destroy;
	object_class->finalize = bonobo_zoomable_frame_finalize;

	init_zoomable_corba_class ();
}

static void
bonobo_zoomable_frame_init (BonoboZoomableFrame *zoomable)
{
	zoomable->priv = g_new0 (BonoboZoomableFramePrivate, 1);

}

/**
 * bonobo_zoomable_frame_get_type:
 *
 * Returns: the GtkType for a BonoboZoomableFrame object.
 */
GtkType
bonobo_zoomable_frame_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboZoomableFrame",
			sizeof (BonoboZoomableFrame),
			sizeof (BonoboZoomableFrameClass),
			(GtkClassInitFunc) bonobo_zoomable_frame_class_init,
			(GtkObjectInitFunc) bonobo_zoomable_frame_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

Bonobo_ZoomableFrame
bonobo_zoomable_frame_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_ZoomableFrame *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_ZoomableFrame *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_zoomable_frame_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_ZoomableFrame__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);
	return (Bonobo_ZoomableFrame) bonobo_object_activate_servant (object, servant);
}

BonoboZoomableFrame *
bonobo_zoomable_frame_construct (BonoboZoomableFrame	*p,
				 Bonobo_ZoomableFrame	 corba_p)
{
	g_return_val_if_fail (p != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (p), NULL);
	g_return_val_if_fail (corba_p != NULL, NULL);

	bonobo_object_construct (BONOBO_OBJECT (p), corba_p);

	return p;
}

/**
 * bonobo_zoomable_frame_new:
 * 
 * Create a new bonobo-zoomable implementing BonoboObject
 * interface.
 * 
 * Return value: 
 **/
BonoboZoomableFrame *
bonobo_zoomable_frame_new (void)
{
	BonoboZoomableFrame	*p;
	Bonobo_ZoomableFrame	 corba_p;

	p = gtk_type_new (bonobo_zoomable_frame_get_type ());
	g_return_val_if_fail (p != NULL, NULL);

	corba_p = bonobo_zoomable_frame_corba_object_create (BONOBO_OBJECT (p));
	if (corba_p == CORBA_OBJECT_NIL){
		bonobo_object_unref (BONOBO_OBJECT (p));
		return NULL;
	}

	return bonobo_zoomable_frame_construct (p, corba_p);
}

/**
 * bonobo_zoomable_frame_bind_to_zoomable:
 * @zoomable_frame: A BonoboZoomableFrame object.
 * @zoomable: The CORBA object for the BonoboZoomable embedded
 * in this BonoboZoomableFrame.
 *
 * Associates @zoomable with this @zoomable_frame.
 */
void
bonobo_zoomable_frame_bind_to_zoomable (BonoboZoomableFrame *zoomable_frame, Bonobo_Zoomable zoomable)
{
	CORBA_Environment ev;

	g_return_if_fail (zoomable != CORBA_OBJECT_NIL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));

	/*
	 * Keep a local handle to the Zoomable.
	 */
	if (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL)
		g_warning ("FIXME: leaking zoomable reference");

	zoomable_frame->priv->zoomable = bonobo_object_dup_ref (zoomable, NULL);

	/*
	 * Introduce ourselves to the Zoomable.
	 */
	CORBA_exception_init (&ev);
	Bonobo_Zoomable_setFrame (zoomable, BONOBO_OBJREF (zoomable_frame),
				  &ev);
	if (BONOBO_EX (&ev))
		bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame), zoomable, &ev);
	CORBA_exception_free (&ev);
}

/**
 * bonobo_zoomable_frame_get_zoomable:
 * @zoomable_frame: A BonoboZoomableFrame which is bound to a remote
 * BonoboZoomable.
 *
 * Returns: The Bonobo_Zoomable CORBA interface for the remote Zoomable
 * which is bound to @frame.  See also
 * bonobo_zoomable_frame_bind_to_zoomable().
 */
Bonobo_Zoomable
bonobo_zoomable_frame_get_zoomable (BonoboZoomableFrame *zoomable_frame)
{
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), CORBA_OBJECT_NIL);

	return zoomable_frame->priv->zoomable;
}

void
bonobo_zoomable_frame_zoom_in (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (zoomable_frame != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomIn  (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

void
bonobo_zoomable_frame_zoom_out (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (zoomable_frame != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomOut (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

void
bonobo_zoomable_frame_zoom_to_fit (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (zoomable_frame != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomFit (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

void
bonobo_zoomable_frame_zoom_to_default (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (zoomable_frame != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomDefault (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

float
bonobo_zoomable_frame_get_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	float retval;

	g_return_val_if_fail (zoomable_frame != NULL, 0.0);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), 0.0);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, 0.0);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_level (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = 0.0;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

float
bonobo_zoomable_frame_get_min_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	float retval;

	g_return_val_if_fail (zoomable_frame != NULL, 0.0);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), 0.0);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, 0.0);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_minLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = 0.0;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

float
bonobo_zoomable_frame_get_max_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	float retval;

	g_return_val_if_fail (zoomable_frame != NULL, 0.0);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), 0.0);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, 0.0);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_maxLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = 0.0;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

gboolean
bonobo_zoomable_frame_has_min_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	gboolean retval;

	g_return_val_if_fail (zoomable_frame != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), FALSE);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, FALSE);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_hasMinLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = FALSE;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

gboolean
bonobo_zoomable_frame_has_max_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	gboolean retval;

	g_return_val_if_fail (zoomable_frame != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), FALSE);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, FALSE);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_hasMaxLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = FALSE;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

gboolean
bonobo_zoomable_frame_is_continuous (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	gboolean retval;

	g_return_val_if_fail (zoomable_frame != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), FALSE);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, FALSE);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_isContinuous (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = FALSE;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

GList *
bonobo_zoomable_frame_get_preferred_zoom_levels (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	Bonobo_ZoomLevelList *zoom_levels;
	GList *list = NULL;
	int i;

	g_return_val_if_fail (zoomable_frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), NULL);

	CORBA_exception_init (&ev);

	zoom_levels = Bonobo_Zoomable__get_preferredLevels (
		zoomable_frame->priv->zoomable, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
					 zoomable_frame->priv->zoomable, &ev);
		CORBA_exception_free (&ev);
		return NULL;
	}

	CORBA_exception_free (&ev);

	if (zoom_levels == CORBA_OBJECT_NIL)
		return NULL;

	for (i = 0; i < zoom_levels->_length; i++) {
		float *this;

		this = g_new0 (float, 1);
		*this = zoom_levels->_buffer [i];

		list = g_list_prepend (list, this);
	}

	CORBA_free (zoom_levels);

	return g_list_reverse (list);
}

GList *
bonobo_zoomable_frame_get_preferred_zoom_level_names (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	Bonobo_ZoomLevelNameList *zoom_level_names;
	GList *list = NULL;
	int i;

	g_return_val_if_fail (zoomable_frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), NULL);

	CORBA_exception_init (&ev);

	zoom_level_names = Bonobo_Zoomable__get_preferredLevelNames (
		zoomable_frame->priv->zoomable, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
					 zoomable_frame->priv->zoomable, &ev);
		CORBA_exception_free (&ev);
		return NULL;
	}
	CORBA_exception_free (&ev);

	if (zoom_level_names == CORBA_OBJECT_NIL)
		return NULL;

	for (i = 0; i < zoom_level_names->_length; i++)
		list = g_list_prepend (list, g_strdup (zoom_level_names->_buffer [i]));

	CORBA_free (zoom_level_names);

	return g_list_reverse (list);
}

void
bonobo_zoomable_frame_set_zoom_level (BonoboZoomableFrame *zoomable_frame, float zoom_level)
{
	CORBA_Environment ev;

	g_return_if_fail (zoomable_frame != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_setLevel (zoomable_frame->priv->zoomable, zoom_level, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}
