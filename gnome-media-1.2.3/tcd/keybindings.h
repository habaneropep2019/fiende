#ifndef GTCD_KEYBINDINGS_H
#define GTCD_KEYBINDINGS_H

#include "prefs.h"

typedef struct
{
    Shortcut *key;
    char *desc;
    char *signal;
    GtkWidget *widget;
    GtkWidget *data;
    gint data2;
} KeyBinding;

void add_key_binding(GtkWidget *widget, char *signal, char *desc, Shortcut *key);

#endif
