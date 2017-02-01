#ifndef MENUS_H
#define MENUS_H

#include "gnome.h"
#include "mdi-color-file.h"
#include "view-color-generic.h"

extern GnomeUIInfo main_menu [];
extern GnomeUIInfo main_menu_with_doc[];
extern GnomeUIInfo toolbar [];

void menu_view_do_popup (GdkEventButton *event, ViewColorGeneric *view);
void menu_edit (MDIColor *col);

gint save_file (MDIColorFile *mcf);

#endif
