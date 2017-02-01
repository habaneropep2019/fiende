/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <glib.h>
#include <gnome.h>
#include "tree.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* menubar callbacks */
void exit_row_callback(GtkWidget *widget, gint row, gint, GdkEventButton *, gpointer);
void exit_dialog_ok_callback(GtkWidget *widget, gpointer data);
void exit_dialog_cancel_callback(GtkWidget *widget, gpointer data);
void exit_dialog_close_callback(GtkWidget *widget, gpointer data);
int exit_callback(GtkWidget *widget, gpointer data);
void help_callback(GtkWidget *widget, gpointer data);
void item_help_callback(GtkWidget *widget, gpointer data);
void about_callback(GtkWidget *widget, gpointer data);
void create_templist(node_data *data, GList **list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

