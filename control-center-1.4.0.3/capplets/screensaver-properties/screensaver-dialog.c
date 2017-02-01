/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#include <config.h>
#include "screensaver-dialog.h"
#include "callbacks.h"
#include "gnome.h"

extern GtkWidget *capplet;

GtkWidget *
make_dialog (screensaver_data *sd)
{
        GtkWidget *dialog;
        GtkWidget *hbox, *vbox, *alignment, *frame;
        GtkWidget *iconframe;
        GtkWidget *icon;
        GtkWidget *authorframe;
        GtkWidget *avbox;
        GtkWidget *temphbox;
        GtkWidget *label;
        GtkWidget *align2;

        dialog = gnome_dialog_new (sd->name, _("Preview"), GNOME_STOCK_BUTTON_OK, NULL);
        gnome_dialog_set_default (GNOME_DIALOG (dialog), 1);
        gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (capplet));
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        hbox = gtk_hbox_new (FALSE, GNOME_PAD);
        vbox = gtk_vbox_new (FALSE, GNOME_PAD);
    

        if (sd->icon) 
                icon = gnome_pixmap_new_from_file (sd->icon);

        if (icon) {
                iconframe = gtk_frame_new (NULL);

                gtk_frame_set_shadow_type (GTK_FRAME (iconframe), GTK_SHADOW_IN);
                alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
                gtk_container_add (GTK_CONTAINER (alignment), iconframe);
                gtk_container_add (GTK_CONTAINER (iconframe), icon);
                gtk_box_pack_start (GTK_BOX (vbox), alignment, TRUE, TRUE, 0);
        }
        avbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
        authorframe = gtk_frame_new(_("About:"));
        temphbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
        gtk_container_set_border_width (GTK_CONTAINER (avbox), GNOME_PAD_SMALL);
        if (sd->author) {
                align2 = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
                label = gtk_label_new (_("Author:"));
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
                gtk_container_add (GTK_CONTAINER (align2), label);
                gtk_box_pack_start (GTK_BOX (temphbox), align2, FALSE, FALSE, 0);

                label = gtk_label_new (sd->author);
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
                gtk_box_pack_start (GTK_BOX (temphbox), label, FALSE, FALSE, 0);

        } else {
                label = gtk_label_new (_("Author: UNKNOWN"));
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
                gtk_box_pack_start (GTK_BOX (temphbox), label, FALSE, FALSE, 0);
        }
        gtk_box_pack_start (GTK_BOX (avbox), temphbox, FALSE, FALSE, 0);
        if (sd->comment) {
                temphbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
                label = gtk_label_new (sd->comment);
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
                gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
                gtk_box_pack_start (GTK_BOX (temphbox), label, FALSE, FALSE, 0);
                gtk_box_pack_start (GTK_BOX (avbox), temphbox, FALSE, FALSE, 0);
        }

        gtk_container_add (GTK_CONTAINER (authorframe), avbox);
        gtk_box_pack_end (GTK_BOX (vbox), authorframe, FALSE, FALSE, 0);

        frame = gtk_frame_new (_("Settings"));
        gtk_container_add (GTK_CONTAINER (frame), get_screensaver_widget (sd));
        gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, TRUE, TRUE, GNOME_PAD_SMALL);

        gtk_signal_connect (GTK_OBJECT (dialog), "clicked", (GtkSignalFunc) dialog_callback, sd);
        gtk_signal_connect (GTK_OBJECT (dialog), "destroy", (GtkSignalFunc) dialog_destroy_callback, sd);
        gtk_widget_show_all (dialog);
        return dialog;
}
