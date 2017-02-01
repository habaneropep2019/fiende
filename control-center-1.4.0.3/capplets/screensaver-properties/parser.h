/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#ifndef __PARSER_H__
#define __PARSER_H__
#include "glib.h"
#include "gtk/gtkwidget.h"
typedef struct _screensaver_data screensaver_data;
struct _screensaver_data
{
        gchar *desktop_filename; /* the file name of the screensaver .desktop file */
        gchar *tryexec; /* the name of the screensaver executable */
        gchar *name; /* the name of the screensaver */
        gchar *windowid; /* the flag that lets you put the screensaver in a partic. window */
        gchar *root; /* the flag that lets you right on the root window */
        gchar *args; /* the current, up to date args that the screensaver has. */
        gchar *icon; /* the complete pathname of the screensaver icon */
        gchar *comment; /* as it sounds.  Used in the dialog box */
        gchar *author; /* the authors name. */
        gchar *demo; /* the args used to make the demo in the window */
        GtkWidget *dialog; /* the current dialog */
        GList *setup_data; /* internally used */
};

void init_screensaver_data (screensaver_data *sd);
GtkWidget *get_screensaver_widget (screensaver_data *sd);
void store_screensaver_data (screensaver_data *sd);
void free_screensaver_data (screensaver_data *sd);
#endif
