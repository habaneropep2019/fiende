/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#ifndef __TREE_H__
#define __TREE_H__

#include <gnome.h>
#include "control-center.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */


GNode *read_directory (gchar *directory);
void merge_nodes (GNode *node1, GNode *node2);
GtkWidget *generate_tree (void);
gint button_press (GtkCTree *ctree, GdkEventButton *event, gpointer data);

typedef enum
{
        CAPPLET_INACTIVE,
        CAPPLET_UNREGISTERED,
        CAPPLET_ACTIVE
} CappletState;

typedef struct _node_data node_data;
struct _node_data
{
        GNOME_capplet capplet;
        GnomeDesktopEntry *gde;
        GtkCTreeNode *node;
        GtkCTree *ctree;
        GtkWidget *socket;
        GtkWidget *label;
        gchar *icon;
        gint id;
        GtkWidget *child;
        gboolean modified;
        GtkWidget *try_button;
        GtkWidget *revert_button;
        GtkWidget *ok_button;
        GtkWidget *cancel_button;
        GtkWidget *help_button;
        CappletState state;
};



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
