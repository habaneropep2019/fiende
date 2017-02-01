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

#ifndef _BONOBO_ZOOMABLE_H_
#define _BONOBO_ZOOMABLE_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_ZOOMABLE_TYPE		(bonobo_zoomable_get_type ())
#define BONOBO_ZOOMABLE(o)		(GTK_CHECK_CAST ((o), BONOBO_ZOOMABLE_TYPE, BonoboZoomable))
#define BONOBO_ZOOMABLE_CLASS(k)	(GTK_CHECK_CLASS_CAST((k), BONOBO_ZOOMABLE_TYPE, BonoboZoomableClass))
#define BONOBO_IS_ZOOMABLE(o)		(GTK_CHECK_TYPE ((o), BONOBO_ZOOMABLE_TYPE))
#define BONOBO_IS_ZOOMABLE_CLASS(k)	(GTK_CHECK_CLASS_TYPE ((k), BONOBO_ZOOMABLE_TYPE))

typedef struct _BonoboZoomablePrivate	BonoboZoomablePrivate;

typedef struct {
        BonoboObject		object;

	BonoboZoomablePrivate	*priv;
} BonoboZoomable;

typedef struct {
	BonoboObjectClass	parent;

	void (*set_frame)	(BonoboZoomable *zoomable);
	void (*set_zoom_level)	(BonoboZoomable *zoomable,
				 float zoom_level);

	void (*zoom_in)		(BonoboZoomable *zoomable);
	void (*zoom_out)	(BonoboZoomable *zoomable);
	void (*zoom_to_fit)	(BonoboZoomable *zoomable);
	void (*zoom_to_default)	(BonoboZoomable *zoomable);
} BonoboZoomableClass;

POA_Bonobo_Zoomable__epv *bonobo_zoomable_get_epv  (void);

GtkType		 bonobo_zoomable_get_type (void);
Bonobo_Zoomable	 bonobo_zoomable_corba_object_create		(BonoboObject   *object);

BonoboZoomable	*bonobo_zoomable_new				(void);

BonoboZoomable	*bonobo_zoomable_construct			(BonoboZoomable	*zoomable,
								 Bonobo_Zoomable corba_zoomable);

void		 bonobo_zoomable_set_parameters			(BonoboZoomable	*zoomable,
								 float           zoom_level,
								 float		 min_zoom_level,
								 float		 max_zoom_level,
								 gboolean	 has_min_zoom_level,
								 gboolean	 has_max_zoom_level);

void		 bonobo_zoomable_set_parameters_full		(BonoboZoomable	*zoomable,
								 float           zoom_level,
								 float		 min_zoom_level,
								 float		 max_zoom_level,
								 gboolean	 has_min_zoom_level,
								 gboolean	 has_max_zoom_level,
								 gboolean	 is_continuous,
								 float          *preferred_zoom_levels,
								 const gchar   **preferred_zoom_level_names,
								 gint		 num_preferred_zoom_levels);
void             bonobo_zoomable_add_preferred_zoom_level       (BonoboZoomable *zoomable,
                                                                 float zoom_level,
                                                                 const gchar *zoom_level_name);

void		 bonobo_zoomable_report_zoom_level_changed	(BonoboZoomable	*zoomable,
								 float		 new_zoom_level);

void		 bonobo_zoomable_report_zoom_parameters_changed	(BonoboZoomable	*zoomable);


END_GNOME_DECLS

#endif /* _BONOBO_ZOOMABLE_H_ */
