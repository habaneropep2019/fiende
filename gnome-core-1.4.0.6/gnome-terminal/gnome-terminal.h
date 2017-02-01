
#ifndef GNOME_TERMINAL_H
#define GNOME_TERMINAL_H

#include <orb/orbit.h>
#include <gtk/gtkwidget.h>

/* no longer needed, as we use gnome_font_picker now */
/* GtkWidget *create_font_menu (ZvtTerm *term, void *f); */

GtkWidget *new_terminal_for_client (const char *geom);

/* Do we have a TerminalFactory? */
extern int use_terminal_factory;
extern int has_terminal_factory;
extern int start_terminal_factory;

extern int corba_init_server (CORBA_ORB _orb);
extern void corba_activate_server (void);
extern CORBA_Object create_terminal_via_factory (const char *geometry,
						 CORBA_Environment *ev);

#endif
