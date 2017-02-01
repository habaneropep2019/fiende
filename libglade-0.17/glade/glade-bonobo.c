/* -*- Mode: C; c-basic-offset: 8 -*-
 * libglade - a library for building interfaces from XML files at runtime
 * Copyright (C) 1998-2001  James Henstridge <james@daa.com.au>
 *
 * glade-bonobo.c: support for bonobo widgets in libglade.
 * Copyright (C) 2000 Helix Code, Inc.
 *
 * Author:
 *      Michael Meeks (michael@helixcode.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

/* this file is only built if GNOME support is enabled */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <glade/glade.h>
#include <glade/glade-build.h>
#include <glade/glade-private.h>

#include <gnome.h>
#include <bonobo.h>

#ifndef ENABLE_NLS
/* a slight optimisation when gettext is off */
#define glade_xml_gettext(xml, msgid) (msgid)
#endif
#undef _
#define _(msgid) (glade_xml_gettext(xml, msgid))

static GtkWidget *
gnome_control_new (GladeXML *xml, GladeWidgetInfo *info, char **err)
{
	GtkWidget               *widget;
	BonoboControlFrame      *cf;
	Bonobo_PropertyBag       pb;
	GList                   *tmp;

	g_return_val_if_fail (info->class != NULL, NULL);

	widget = bonobo_widget_new_control (info->class, CORBA_OBJECT_NIL);

	if (!widget) {
		*err = g_strdup_printf ("unknown bonobo control '%s'", info->class);
		return NULL;
	}

	cf = bonobo_widget_get_control_frame (BONOBO_WIDGET (widget));

	if (!cf) {
		*err = g_strdup_printf ("control '%s' has no frame", info->class);
		gtk_widget_unref (widget);
		return NULL;
	}

	pb = bonobo_control_frame_get_control_property_bag (cf, NULL);
	if (pb == CORBA_OBJECT_NIL)
		return widget;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;
		CORBA_TypeCode tc;

		tc  = bonobo_property_bag_client_get_property_type (pb, attr->name, NULL);

		switch (tc->kind) {

		case CORBA_tk_boolean:
			bonobo_property_bag_client_set_value_gboolean (pb, attr->name,
								       attr->value[0] == 'T', NULL);
			break;

		case CORBA_tk_string:
			bonobo_property_bag_client_set_value_string (pb, attr->name, attr->value, NULL);
			break;

		case CORBA_tk_long:
			bonobo_property_bag_client_set_value_glong (pb, attr->name, strtol (attr->value, NULL, 0), NULL);
			break;

		case CORBA_tk_float:
			bonobo_property_bag_client_set_value_gfloat (pb, attr->name, strtod (attr->value, NULL), NULL);
			break;

		case CORBA_tk_double:
			bonobo_property_bag_client_set_value_gdouble (pb, attr->name, strtod (attr->value, NULL), NULL);
			break;

		default:
			g_warning ("Unhandled type %d", tc->kind);
			break;
		}
	}

	gtk_widget_show (widget);
	return widget;
}

/**
 * glade_bonobo_init
 *
 * This function performs initialisation of glade, similar to what glade_init
 * does (in fact it calls glade_init for you).  The difference is that it
 * also initialises the Bonobo widget building routines.
 *
 * As well as calling this initialisation function, Bonoboized programs should
 * also link with the libglade-gnome and libglade-bonobo libraries, which
 * contains all the GNOME, Bonobo libglade stuff.
 */
void
glade_bonobo_init(void)
{
	static gboolean initialised = FALSE;

	if (initialised) return;
	initialised = TRUE;
	glade_init();
	glade_gnome_init();

	glade_xml_build_extended_widget = gnome_control_new;
}
