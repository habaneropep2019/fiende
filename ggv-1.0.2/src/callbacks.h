#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include "ggvwindow.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* callback prototypes */
void about_callback(GtkWidget *widget, gpointer data);
void new_callback(GtkWidget *widget, gpointer data);
void button_press_callback(GtkWidget *widget, GdkEventButton *event, gpointer data);
void button_release_callback(GtkWidget *widget, GdkEventButton *event, gpointer data);
void close_callback (GtkWidget *widget, ggv_window *ggv);
gint delete_callback (GtkWidget *widget, GdkEventAny *event, gpointer data);
gint exit_callback(GtkWidget *widget, gpointer data);
void file_open_cancel_callback(GtkWidget *widget, gpointer data);
void file_open_delete_callback(GtkWidget *widget, GdkEventAny *e, gpointer data);
void file_open_ok_callback(GtkWidget *widget, gpointer data);
void file_save_cancel_callback(GtkWidget *widget, gpointer data);
void file_save_delete_callback(GtkWidget *widget, GdkEventAny *e, gpointer data);
void file_save_ok_callback(GtkWidget *widget, gpointer data);
void show_menubar_callback(GtkWidget *widget, gpointer data);
void show_toolbar_callback(GtkWidget *widget, gpointer data);

void hide_panel_callback(GtkWidget *widget, gpointer data);
void motion_callback(GtkWidget *widget, GdkEventMotion *event, gpointer data);
void key_pressed_event_callback(GtkWidget *widget, GdkEventKey *event, gpointer data);
void first_page_callback(GtkWidget *widget, gpointer data);
void last_page_callback(GtkWidget *widget, gpointer data);
void next_page_callback(GtkWidget *widget, gpointer data);
void open_callback(GtkWidget *widget, gpointer data);
void override_orientation_callback(GtkWidget *widget, gpointer data);
void orientation_callback(GtkWidget *widget, gpointer data);
void preferences_callback(GtkWidget *widget, gpointer data);
void previous_page_callback(GtkWidget *widget, gpointer data);
void print_callback(GtkWidget *widget, gpointer data);
void print_marked_pages_callback(GtkWidget *widget, gpointer data);
void recenter_page_callback(GtkWidget *widget, gpointer data);
void reload_callback(GtkWidget *widget, gpointer data);
void scrollpane_middle_click_callback(GtkWidget *widget, gpointer data);
void scrollpane_right_click_callback(GtkWidget *widget, gpointer data);
void save_callback(GtkWidget *widget, gpointer data);
void save_as_callback(GtkWidget *widget, gpointer data);
void select_page_callback(GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer data);
void zoom_callback(GtkWidget *widget, gpointer data);
void drop_callback(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time, gpointer data);
void prefs_destroy_callback(GtkWidget *w, ggv_prefs *p);
void prefs_apply_callback(GtkWidget *w, gint page, ggv_prefs *p);
void prefs_help_callback(GtkWidget *w, gint page, ggv_prefs *p);
void prefs_changed_callback(GtkWidget *w, ggv_prefs *p);
void zoom_to(ggv_window *ggv, gint magstep);
void zoomin_callback(GtkWidget *widget, gpointer *data);
void zoomout_callback(GtkWidget *widget, gpointer *data);
void recent_add(char *filename);
void recent_update_menus (ggv_window *ggv, GList *recent_files);
void recent_update(void);
void recent_callback(GtkWidget *w, gpointer *data);
void interpreter_message_callback(GtkGS *gs, gchar *msg, gpointer data);
void unmark_all_pages_callback(GtkWidget *widget, gpointer data);
void toggle_current_page_callback(GtkWidget *widget, gpointer data);
void toggle_even_pages_callback(GtkWidget *widget, gpointer data);
void toggle_odd_pages_callback(GtkWidget *widget, gpointer data);
void toggle_all_pages_callback(GtkWidget *widget, gpointer data);
void select_page_button_press_callback(GtkWidget *widget,
				       GdkEventButton *event,
				       gpointer data);
void page_toggled_callback (GtkWidget *widget, gint row, gboolean toggled, gpointer data);
gint timer_callback (gpointer data);
void antialiasing_callback(GtkWidget *widget, gpointer data);
void watch_file_callback(GtkWidget *widget, gpointer data);
void paper_callback(GtkWidget *widget, gpointer *data);
void override_paper_callback(GtkWidget *widget, gpointer data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
