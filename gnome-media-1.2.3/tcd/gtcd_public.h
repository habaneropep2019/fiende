#ifndef GTCD_PUBLIC_H
#define GTCD_PUBLIC_H

#include <gnome.h>
#include <sys/types.h>

#include "linux-cdrom.h"
#include "prefs.h"

void make_goto_menu(void);
void create_warning(char *message_text, char *type);
void draw_status(void);
void gcddb(GtkWidget *widget, gpointer *data);
void preferences(GtkWidget *widget, void *data);
void setup_colors(void);
void setup_fonts(void);
void setup_popup_menu(GtkWidget *w, cd_struct *data);
void edit_window(GtkWidget *widget, gpointer data);
void cddb_status_dialog(GtkWidget *widget, gpointer data);
void update_editor(void);
void help(GtkWidget *widget, gpointer data);
void adjust_status_size(void);

GnomeClient *new_client(void);
void session_die (gpointer client_data);
int save_state (GnomeClient        *client,
		       gint                phase,
		       GnomeRestartStyle   save_style,
		       gint                shutdown,
		       GnomeInteractStyle  interact_style,
		       gint                fast,
		       gpointer            client_data);

extern int titlelabel_f;
extern cd_struct cd;
extern tcd_prefs *prefs;
extern GtkTooltips *tooltips;
extern GtkAccelGroup *accel;
extern GList *keys;
extern GtkWidget *window;

#endif
