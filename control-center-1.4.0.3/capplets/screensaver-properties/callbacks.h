/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#ifndef __GNOMECC_CALLBACKS_H__
#define __GNOMECC_CALLBACKS_H__ 1
#include <gtk/gtk.h>
#include "parser.h"
void screensaver_load (void);
GtkWidget *get_and_set_min_entry (void);
GtkWidget *get_and_set_pword (void);
GtkWidget *get_and_set_dpmsmin (void);
GtkWidget *get_and_set_dpmscheck (GtkWidget *box);
GtkAdjustment *get_and_set_nice (void);
GtkWidget *get_and_set_mode(void);
void launch_miniview (screensaver_data *sd);
void dialog_callback (GtkWidget *dialog, gint button, screensaver_data *sd);
void dialog_destroy_callback (GtkWidget *widget, screensaver_data *sd);
void monitor_expose_callback (GtkWidget *list, GdkEventButton *event, void *data);
void monitor_died_callback (GtkWidget *check, void *data);
void list_click_callback (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data);
void insert_text_callback (GtkEditable *editable, const gchar *text, gint length, gint *position, void *data);
void insert_text_callback2 (GtkEditable *editable, const gchar *text, gint length, gint *position, void *data);
void delete_text_callback (GtkEditable *editable, gint length, gint *position, void *data);
void dpms_callback (GtkWidget *check, void *data);
void password_callback (GtkWidget *password, void *data);
void password_callback (GtkWidget *password, void *data);
void nice_callback (GtkObject *adj, void *data);
void preview_callback (GtkWidget *widget, gpointer data);
void setup_callback (GtkWidget *widget, gpointer data);
void destroy_callback (GtkWidget *widget, void *data);
void ok_callback (void);
void page_hide_callback (void);
void page_show_callback (void);
void try_callback (void);
void help_callback (void);
void revert_callback (void);
void sig_child(int sig);

#endif
