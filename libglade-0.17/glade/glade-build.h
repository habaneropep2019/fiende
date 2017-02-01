/* -*- Mode: C; c-basic-offset: 8 -*-
 * libglade - a library for building interfaces from XML files at runtime
 * Copyright (C) 1998-2001  James Henstridge <james@daa.com.au>
 *
 * glade-build.h: functions useful for adding support for new widgets.
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
#ifndef GLADE_BUILD_H
#define GLADE_BUILD_H

#include <glade/glade-xml.h>
#include <gtk/gtktypeutils.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkaccelgroup.h>
#include <gtk/gtkadjustment.h>

#include <glade/glade-widget-tree.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	
/* create a new widget of some type.  Don't parse `standard' widget options */
typedef GtkWidget *(* GladeNewFunc) (GladeXML *xml,
				     GladeWidgetInfo *info);
/* call glade_xml_build_widget on each child node, and pack in self */
typedef void (* GladeBuildChildrenFunc) (GladeXML *xml,
					 GtkWidget *w,
					 GladeWidgetInfo *info,
					 const char *longname);

typedef struct _GladeWidgetBuildData GladeWidgetBuildData;
struct _GladeWidgetBuildData {
  char *name;
  GladeNewFunc new;
  GladeBuildChildrenFunc build_children;
};

/* widgets is a static, NULL terminated array of GladeWidgetBuildData's.
 * They will be added to a hash table, with the name as the key, to be
 * used by glade_xml_build_widget */
void glade_register_widgets(const GladeWidgetBuildData *widgets);

/* set the current toplevel widget while building (use NULL to unset) */
void glade_xml_set_toplevel(GladeXML *xml, GtkWindow *window);

/* push and pop accelerator groups on a stack */
GtkAccelGroup *glade_xml_push_accel(GladeXML *xml);
GtkAccelGroup *glade_xml_pop_accel(GladeXML *xml);

/* make sure that xml->priv->accel_group is a valid AccelGroup */
GtkAccelGroup *glade_xml_ensure_accel(GladeXML *xml);

/* functions for manipulating the uline accel group stack */
void           glade_xml_push_uline_accel(GladeXML *xml, GtkAccelGroup *uline);
void           glade_xml_pop_uline_accel (GladeXML *xml);
GtkAccelGroup *glade_xml_get_uline_accel (GladeXML *xml);

/* this is the handler for GtkLabel accelerators */
void           glade_xml_handle_label_accel(GladeXML *xml, const gchar *target,
					    guint key);
guint          glade_xml_get_parent_accel  (GladeXML *xml);

/* set standard properties on a GtkWindow widget */
void           glade_xml_set_window_props (GtkWindow *window,
					   GladeWidgetInfo *info);

/* this function is called to build the interface by GladeXML */
GtkWidget *glade_xml_build_widget(GladeXML *self, GladeWidgetInfo *info,
				  const char *parent_long);

/* This function performs half of what glade_xml_build_widget does.  It is
 * useful when the widget has already been created.  Usually it would not
 * have any use at all. */
void       glade_xml_set_common_params(GladeXML *self,
				       GtkWidget *widget,
				       GladeWidgetInfo *info,
				       const char *parent_long);

/* A standard child building routine that can be used in widget builders */
void glade_standard_build_children(GladeXML *self, GtkWidget *w,
				   GladeWidgetInfo *info,const char *longname);

/* create an adjustment object for a widget */
GtkAdjustment *glade_get_adjustment(GladeWidgetInfo *info);

/* this is a wrapper for gtk_type_enum_find_value, that just returns the
 * integer value for the enum */
gint glade_enum_from_string(GtkType type, const char *string);

/* a wrapper for gettext */
char *glade_xml_gettext(GladeXML *xml, const char *msgid);

/* invoke the custom widget creation function */
GtkWidget *glade_create_custom(GladeXML *xml, gchar *func_name, gchar *name,
			       gchar *string1, gchar *string2,
			       gint int1, gint int2);


#ifdef __cplusplus
}
#endif /* __cplusplus */
	
#endif
