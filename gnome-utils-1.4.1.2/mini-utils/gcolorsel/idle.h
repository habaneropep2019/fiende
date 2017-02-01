#ifndef IDLE_H
#define IDLE_H

#include "gtk/gtk.h"

int  idle_add (GtkFunction function, gpointer data);
void idle_remove (int id);

void idle_block (void);
void idle_unblock (void);

#endif
