/* -*- Mode: C; c-basic-offset: 4 -*-
 * libglade - a library for building interfaces from XML files at runtime
 * Copyright (C) 1998-2001  James Henstridge <james@daa.com.au>
 *
 * glade-widget-tree.h: internal representation of glade files.
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

#ifndef GLADE_WIDGET_TREE_H
#define GLADE_WIDGET_TREE_H

#include <gtk/gtk.h>

typedef struct _GladeAttribute GladeAttribute;
struct _GladeAttribute {
    gchar *name;
    gchar *value;
};

typedef struct _GladeAcceleratorInfo GladeAcceleratorInfo;
struct _GladeAcceleratorInfo {
    guint key;
    GdkModifierType modifiers;
    gchar *signal;
};

typedef struct _GladeSignalInfo GladeSignalInfo;
struct _GladeSignalInfo {
    gchar *name;
    gchar *handler;
    gchar *data;
    gchar *object; /* NULL if this isn't a connect_object signal */
    gboolean after : 1;
};

typedef struct _GladeStyleInfo GladeStyleInfo;
struct _GladeStyleInfo {
    gchar *name;
    gchar *rc_name;
    gboolean local : 1;
};

typedef struct _GladeWidgetInfo GladeWidgetInfo;
struct _GladeWidgetInfo {
    GladeWidgetInfo *parent;

    gchar *class;
    gchar *name;
    gchar *tooltip;

    gint width, height;
    gint border_width;

    /* bit field */
    gboolean visible : 1;
    gboolean sensitive : 1;
    gboolean can_default : 1;
    gboolean can_focus : 1;
    gboolean has_default : 1;
    gboolean has_focus : 1;

    GladeStyleInfo *style;

    /* lists of GladeAttribute's */
    GList *attributes;
    GList *child_attributes; /* for the <child></child> section */

    GList *signals;
    GList *accelerators;

    GList *children;
};

typedef struct _GladeWidgetTree GladeWidgetTree;
struct _GladeWidgetTree {
    guint ref;
    GTime mtime;
    GList *styles;
    GList *widgets;
    GHashTable *names;
};

/* parse a file and create a GladeWidgetTree structure */
GladeWidgetTree *glade_widget_tree_parse_file(const char *file);
/* parse a buffer and create a GladeWidgetTree structure */
GladeWidgetTree *glade_widget_tree_parse_memory(char *buffer, int size);
/* ref/unref a GladeWidgetTree structure*/
GladeWidgetTree *glade_widget_tree_ref(GladeWidgetTree *tree);
void glade_widget_tree_unref(GladeWidgetTree *tree);
/* print the info stored in a GladeWidgetTree structure */
void glade_widget_tree_print(GladeWidgetTree *tree);

#endif
