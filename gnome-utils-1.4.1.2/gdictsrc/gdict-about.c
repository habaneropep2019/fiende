/* $Id: gdict-about.c,v 1.4 2000/02/04 23:34:50 spapadim Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Mike Hughes <mfh@psilord.com>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict About box
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>

#include "gdict-about.h"


static GtkWidget *gdict_about_new (void)
{
    const gchar *authors[] = {
        "Mike Hughes <mfh@psilord.com>",
        "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
        "Bradford Hovinen <hovinen@udel.edu>",
        NULL
    };
    GtkWidget *about;
    
    about = gnome_about_new ("GDict", VERSION,
                             "Copyright 1999 by Mike Hughes",
                             authors,
                             _("Client for MIT dictionary server.\nWeb: http://gdict.dhs.org/, http://www.psilord.com/code/"),
                             NULL);
    gtk_window_set_modal (GTK_WINDOW (about), TRUE);
    return about;
}

void gdict_about (void)
{
    GtkWidget *about = gdict_about_new();

    gtk_widget_show(about);
}

