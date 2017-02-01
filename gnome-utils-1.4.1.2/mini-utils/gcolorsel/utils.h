#ifndef UTILS_H
#define UTILS_H

#include <gnome.h>

void pixel_to_rgb (GdkColormap *cmap, guint32 pixel, 
		   gint *red, gint *green, gint *blue);

GtkWidget *drag_window_create (gint red, gint green, gint blue);

/* Set properties without emiting a signal */
void spin_set_value (GtkSpinButton *spin, float val, gpointer data);
void entry_set_text (GtkEntry *entry, char *str, gpointer data);

void spin_connect_value_changed (GtkSpinButton *spin, GtkSignalFunc cb, 
				 gpointer data);

void preview_fill (GtkWidget *preview, int r, int g, int b);


void set_config_key_pos (int pos);
int  get_config_key_pos (void);
int  get_config_key (void);

void display_todo (void);

int my_strcmp (char *str1, char *str2);

void file_selection_ok_cb (GtkWidget *widget, gboolean *cancel);
gint file_selection_delete_event_cb (GtkWidget *widget);

void gtk_flush (void);

void msg_put   (GnomeMDI *mdi, char *msg);
void msg_push  (GnomeMDI *mdi, char *msg);
void msg_pop   (GnomeMDI *mdi);
void msg_flash (GnomeMDI *mdi, char *msg);

void mdi_set_sensitive (GnomeMDI *mdi, gboolean val);

void progress_set (GnomeMDI *mdi, float val);

gpointer clist_get_data_selected (GtkCList *clist);

gint option_menu_get_active (GtkOptionMenu *omenu);
void option_menu_connect_changed (GtkOptionMenu *omenu, GtkSignalFunc cb,
				  gpointer data);

void mdi_set_tab_pos (GnomeMDI *mdi, int tab_pos);
void mdi_realize (GnomeMDI *mdi);
     
#endif

