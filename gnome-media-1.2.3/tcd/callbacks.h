/* Various callbacks for gtcd. */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gnome.h>

void play_cb(GtkWidget *widget, gpointer data);
void pause_cb(GtkWidget *widget, gpointer data);
void stop_cb(GtkWidget *widget, gpointer data);
void eject_cb(GtkWidget *widget, gpointer data);
void about_cb(GtkWidget *widget, gpointer data);
gint goto_track_cb(GtkWidget *widget, gpointer data);
void quit_cb(GtkWidget *widget, gpointer data);
void mixer_cb(GtkWidget *widget, gpointer data);
void changer_cb(GtkWidget *widget, gpointer data);

#endif
