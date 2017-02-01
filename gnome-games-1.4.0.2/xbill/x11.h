#ifndef X11_WIDGETS_H
#define X11_WIDGETS_H

#include <gnome.h>
#include "UI.h"

#define OK	1
#define CANCEL	2

void popup (GtkWidget *mi, GtkWidget **dlgbox);

void new_game_cb ();
void quit_game_cb ();

void leave_window_eh(GtkWidget *w, GdkEventCrossing *event);
void enter_window_eh(GtkWidget *w, GdkEventCrossing *event);
void redraw_window_eh(GtkWidget *w, GdkEventExpose *event);
void button_press_eh(GtkWidget *w, GdkEventButton *event);
void button_release_eh(GtkWidget *w, GdkEventButton *event);
gint timer_eh();

void CreateMenuBar(GtkWidget *app);
GtkWidget *CreatePixmapBox(char *title, GdkPixmap *pixmap, const char *text);
GtkWidget *CreateEnterText (char *title, const char *text,
			    GtkSignalFunc callback);
GtkWidget *CreateDialog (char *title, int buttonmask, GdkPixmap *icon,
			 const char *text, const char *buttonlabel,
			 GtkSignalFunc callback);

void show_scores(int pos);
void warp_apply (GtkWidget *w, GtkEntry *entry);
#endif
