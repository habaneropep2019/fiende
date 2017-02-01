#ifndef LIBGLADE_BUDDY_H
#define LIBGLADE_BUDDY_H

/* this file should include all of the definitions for libglade */

/* libglade callbacks */
gboolean on_front_page_next (GtkWidget *, GnomeDruid *);
gboolean delete_me (GtkWidget *, GdkEventAny *, gpointer data);
gboolean on_nature_page_next  (GtkWidget *, GnomeDruid *);
gboolean on_debian_page_next (GtkWidget *page, GnomeDruid *druid);
gboolean on_desc_page_next (GtkWidget *, GnomeDruid *);
gboolean on_less_page_prepare (GtkWidget *, GtkWidget *);
gboolean on_less_page_next    (GtkWidget *, GtkWidget *);
gboolean on_misc_page_back    (GtkWidget *, GtkWidget *);
gboolean on_the_druid_cancel  (GtkWidget *);
gboolean on_complete_page_prepare (GtkWidget *, GtkWidget *);
gboolean on_complete_page_finish  (GtkWidget *, GtkWidget *);
gboolean on_gnome_page_prepare    (GtkWidget *, GtkWidget *);
gboolean on_version_page_back (GtkWidget *page, GnomeDruid *druid);
gboolean on_contact_page_next     (GtkWidget *, GtkWidget *);
void on_version_list_select_row   (GtkCList *list, gint row, gint col,
				   GdkEventButton *event, gpointer udata);

void update_selected_row      (GtkWidget *widget, gpointer data);
void on_version_apply_clicked (GtkButton *button, gpointer data);
void on_file_radio_toggled    (GtkWidget *radio, gpointer data);
gboolean on_action_page_next  (GtkWidget *page, GtkWidget *druid);
gboolean on_action_page_back  (GtkWidget *page, GnomeDruid *druid);
gboolean on_action_page_prepare (GtkWidget *page, GnomeDruid *druid);
GtkWidget *make_anim (gchar *widget_name, gchar *string1, 
		      gchar *string2, gint int1, gint int2);
GtkWidget *make_pixmap_button (gchar *widget_name, gchar *string1, 
			       gchar *string2, gint int1, gint int2);
gboolean on_more_page_next (GnomeDruidPage *page, GnomeDruid *druid);
gboolean on_contact_page_prepare (GnomeDruidPage *page, GnomeDruid *druid);
gboolean on_version_page_prepare (GnomeDruidPage *page, GnomeDruid *druid);

void on_gdb_go_clicked (GtkWidget *w, gpointer data);
void on_gdb_stop_clicked (GtkWidget *w, gpointer data);

void on_druid_help_clicked   (GtkWidget *w, gpointer);
void on_druid_about_clicked  (GtkWidget *w, gpointer);
void on_druid_prev_clicked   (GtkWidget *w, gpointer);
void on_druid_next_clicked   (GtkWidget *w, gpointer);
void on_druid_cancel_clicked (GtkWidget *w, gpointer);

GtkWidget *stock_pixmap_buddy (char *w, char *n, char *a, int b, int c);
void title_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data);
void side_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data);

void on_product_list_select_row (GtkWidget *w, int row, int col, gpointer data);
void on_product_list_unselect_row (GtkWidget *w, int row, int col, gpointer data);

void on_component_list_select_row (GtkWidget *w, int row, int col, gpointer data);
void on_component_list_unselect_row (GtkWidget *w, int row, int col, gpointer data);

void intro_page_changed (GtkWidget *w, gpointer data);

void on_progress_cancel_clicked (GtkWidget *w, gpointer data);

#endif /* LIBGLADE_BUDDY_H */
