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

#ifndef _BONOBO_ZOOMABLE_FRAME_H_
#define _BONOBO_ZOOMABLE_FRAME_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_ZOOMABLE_FRAME_TYPE		(bonobo_zoomable_frame_get_type ())
#define BONOBO_ZOOMABLE_FRAME(o)		(GTK_CHECK_CAST ((o), BONOBO_ZOOMABLE_FRAME_TYPE, BonoboZoomableFrame))
#define BONOBO_ZOOMABLE_FRAME_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), BONOBO_ZOOMABLE_FRAME_TYPE, BonoboZoomableFrameClass))
#define BONOBO_IS_ZOOMABLE_FRAME(o)		(GTK_CHECK_TYPE ((o), BONOBO_ZOOMABLE_FRAME_TYPE))
#define BONOBO_IS_ZOOMABLE_FRAME_CLASS(k)	(GTK_CHECK_CLASS_TYPE ((k), BONOBO_ZOOMABLE_FRAME_TYPE))

typedef struct _BonoboZoomableFramePrivate	BonoboZoomableFramePrivate;

typedef struct {
        BonoboObject			object;

	BonoboZoomableFramePrivate	*priv;
} BonoboZoomableFrame;

typedef struct {
	BonoboObjectClass		parent;

	void (*zoom_level_changed)	(BonoboZoomableFrame *zframe,
					 float zoom_level);
	void (*zoom_parameters_changed)	(BonoboZoomableFrame *zframe);
} BonoboZoomableFrameClass;

POA_Bonobo_ZoomableFrame__epv *bonobo_zoomable_frame_get_epv  (void);

GtkType			 bonobo_zoomable_frame_get_type			(void);
Bonobo_ZoomableFrame	 bonobo_zoomable_frame_corba_object_create	(BonoboObject		*object);

BonoboZoomableFrame	*bonobo_zoomable_frame_new			(void);

BonoboZoomableFrame	*bonobo_zoomable_frame_construct		(BonoboZoomableFrame	*zframe,
									 Bonobo_ZoomableFrame	 corba_zframe);

float		 bonobo_zoomable_frame_get_zoom_level			(BonoboZoomableFrame	*zframe);

float		 bonobo_zoomable_frame_get_min_zoom_level		(BonoboZoomableFrame	*zframe);
float		 bonobo_zoomable_frame_get_max_zoom_level		(BonoboZoomableFrame	*zframe);
gboolean	 bonobo_zoomable_frame_has_min_zoom_level		(BonoboZoomableFrame	*zframe);
gboolean	 bonobo_zoomable_frame_has_max_zoom_level		(BonoboZoomableFrame	*zframe);
gboolean	 bonobo_zoomable_frame_is_continuous			(BonoboZoomableFrame	*zframe);

GList		*bonobo_zoomable_frame_get_preferred_zoom_levels	(BonoboZoomableFrame	*zframe);
GList		*bonobo_zoomable_frame_get_preferred_zoom_level_names	(BonoboZoomableFrame	*zframe);

void		 bonobo_zoomable_frame_set_zoom_level			(BonoboZoomableFrame	*zframe,
									 float			 zoom_level);

void		 bonobo_zoomable_frame_zoom_in				(BonoboZoomableFrame	*zframe);
void		 bonobo_zoomable_frame_zoom_out				(BonoboZoomableFrame	*zframe);
void		 bonobo_zoomable_frame_zoom_to_fit			(BonoboZoomableFrame	*zframe);
void		 bonobo_zoomable_frame_zoom_to_default			(BonoboZoomableFrame	*zframe);

/* Connecting to the remote object */
void		 bonobo_zoomable_frame_bind_to_zoomable			(BonoboZoomableFrame	*zframe,
									 Bonobo_Zoomable	 zoomable);

Bonobo_Zoomable	 bonobo_zoomable_frame_get_zoomable			(BonoboZoomableFrame	*zframe);


END_GNOME_DECLS

#endif /* _BONOBO_ZOOMABLE_FRAME_H_ */

