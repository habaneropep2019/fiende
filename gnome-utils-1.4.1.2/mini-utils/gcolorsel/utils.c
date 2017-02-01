#include "utils.h"

#include "gnome.h"

/* From gdk-pixbuf, gdk-pixbuf-drawable.c */       
void pixel_to_rgb (GdkColormap *cmap, guint32 pixel, 
		   gint *red, gint *green, gint *blue)
{
  GdkVisual *v;

  g_assert (cmap != NULL);
  g_assert (red != NULL);
  g_assert (green != NULL);
  g_assert (blue != NULL);

  v = gdk_colormap_get_visual (cmap);

  switch (v->type) {
    case GDK_VISUAL_STATIC_GRAY:
    case GDK_VISUAL_GRAYSCALE:
    case GDK_VISUAL_STATIC_COLOR:
    case GDK_VISUAL_PSEUDO_COLOR:
      *red   = cmap->colors[pixel].red;
      *green = cmap->colors[pixel].green;
      *blue  = cmap->colors[pixel].blue;
      break;
    case GDK_VISUAL_TRUE_COLOR:
      *red   = ((pixel & v->red_mask)   << (32 - v->red_shift   - v->red_prec))   >> 24;
      *green = ((pixel & v->green_mask) << (32 - v->green_shift - v->green_prec)) >> 24;
      *blue  = ((pixel & v->blue_mask)  << (32 - v->blue_shift  - v->blue_prec))  >> 24;
                 
      break;
    case GDK_VISUAL_DIRECT_COLOR:
      *red   = cmap->colors[((pixel & v->red_mask)   << (32 - v->red_shift   - v->red_prec))   >> 24].red;
      *green = cmap->colors[((pixel & v->green_mask) << (32 - v->green_shift - v->green_prec)) >> 24].green; 
      *blue  = cmap->colors[((pixel & v->blue_mask)  << (32 - v->blue_shift  - v->blue_prec))  >> 24].blue;
      break;
  default:
    g_assert_not_reached ();
  }  
}

/* From gtk+, gtkcolorsel.c */
GtkWidget *
drag_window_create (gint red, gint green, gint blue)
{
  GtkWidget *window;
  GdkColor bg;

  window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_widget_set_app_paintable (GTK_WIDGET (window), TRUE);
  gtk_widget_set_usize (window, 48, 32);
  gtk_widget_realize (window);

  bg.red = (red / 255.0) * 0xffff;
  bg.green = (green / 255.0) * 0xffff;
  bg.blue = (blue / 255.0) * 0xffff;

  gdk_color_alloc (gtk_widget_get_colormap (window), &bg);
  gdk_window_set_background (window->window, &bg);

  return window;
}

void
spin_set_value (GtkSpinButton *spin, float val, gpointer data)
{
  GtkAdjustment *adj;
  
  adj = gtk_spin_button_get_adjustment (spin);
  gtk_signal_handler_block_by_data (GTK_OBJECT (adj), data);
  gtk_spin_button_set_value (spin, val);
  gtk_signal_handler_unblock_by_data (GTK_OBJECT (adj), data);
}

void 
entry_set_text (GtkEntry *entry, char *str, gpointer data) 
{
  gtk_signal_handler_block_by_data (GTK_OBJECT (entry), data);

  if (str)
    gtk_entry_set_text (GTK_ENTRY (entry), str);
  else
    gtk_entry_set_text (GTK_ENTRY (entry), "");

  gtk_signal_handler_unblock_by_data (GTK_OBJECT (entry), data);
}

void 
spin_connect_value_changed (GtkSpinButton *spin, 
			    GtkSignalFunc cb, gpointer data)
{
  GtkAdjustment *adj;

  adj = gtk_spin_button_get_adjustment (spin);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed", cb, data);
}

void
preview_fill (GtkWidget *preview, int r, int g, int b)
{
  guchar *buf;    
  int x, y;

  buf = g_new (guchar, 3 * preview->allocation.width);
  
  for (x = 0; x < preview->allocation.width; x++) {
    buf [x * 3]     = r;
    buf [x * 3 + 1] = g;
    buf [x * 3 + 2] = b;
  }

  for (y=0; y < preview->allocation.height; y++)
     gtk_preview_draw_row (GTK_PREVIEW (preview), 
			   buf, 0, y, preview->allocation.width);
     
  gtk_widget_draw (preview, NULL);

  g_free (buf);    
}

static int key_pos = 1;

void 
set_config_key_pos (int pos)
{
  key_pos = pos;
}

int  
get_config_key_pos (void)
{
  return key_pos;
}

int  
get_config_key (void)
{
  return key_pos++;
}

void 
display_todo (void)
{
  GtkWidget *dia;
  dia = gnome_message_box_new ("Sorry, this feature is not implemented !",
			       GNOME_MESSAGE_BOX_GENERIC,
			       GNOME_STOCK_BUTTON_OK, NULL);
  gnome_dialog_run_and_close (GNOME_DIALOG (dia));
}

int 
my_strcmp (char *str1, char *str2)
{
  if (str1 == str2) return 0; 

  if ((str1 == NULL) || (str2 == NULL)) return 1;

  return strcmp (str1, str2);
}

void
file_selection_ok_cb (GtkWidget *widget, gboolean *cancel)
{
  *cancel = FALSE;

  gtk_main_quit ();
}

gint
file_selection_delete_event_cb (GtkWidget *widget)
{
  gtk_main_quit ();
  
  return TRUE;
}

void
gtk_flush ()
{
  while (gtk_events_pending ()) gtk_main_iteration ();
}

void
msg_flash (GnomeMDI *mdi, char *msg)
{
  GList *list = mdi->windows;

  while (list) 
    if (GNOME_IS_APP (list->data)) {
      gnome_app_flash (GNOME_APP (list->data), msg);
      list = g_list_next (list);
    }
}

void 
msg_put (GnomeMDI *mdi, char *msg)
{
  GList *list = mdi->windows;

  while (list) 
    if (GNOME_IS_APP (list->data)) {
      gnome_appbar_set_status (GNOME_APPBAR (GNOME_APP (list->data)->statusbar), 
     			       msg);
      list = g_list_next (list);
    }
}

void 
msg_push (GnomeMDI *mdi, char *msg)
{
  GList *list = mdi->windows;

  while (list) 
    if (GNOME_IS_APP (list->data)) {
      gnome_appbar_push (GNOME_APPBAR (GNOME_APP (list->data)->statusbar), msg);
      list = g_list_next (list);
    }
}

void 
msg_pop (GnomeMDI *mdi)
{
  GList *list = mdi->windows;

  while (list) 
    if (GNOME_IS_APP (list->data)) {
      gnome_appbar_pop (GNOME_APPBAR (GNOME_APP (list->data)->statusbar));
      gnome_appbar_refresh (GNOME_APPBAR (GNOME_APP (list->data)->statusbar));
      list = g_list_next (list);
    }
}

void 
progress_set (GnomeMDI *mdi, float val)
{
  GList *list = mdi->windows;
  GnomeAppBar *appbar;

  while (list) 
  
    if (GNOME_IS_APP (list->data)) {
  
      appbar = GNOME_APPBAR (GNOME_APP (list->data)->statusbar);
 
      gnome_appbar_set_progress (appbar, val);

      gtk_widget_draw (GTK_WIDGET (gnome_appbar_get_progress (appbar)), NULL);
 
      list = g_list_next (list);
    }
}

void 
mdi_set_sensitive (GnomeMDI *mdi, gboolean val)
{
  GList *list = mdi->windows;

  while (list) 
    if (GNOME_IS_APP (list->data)) {
      gtk_widget_set_sensitive (GTK_WIDGET (GNOME_APP (list->data)), val);
      list = g_list_next (list);
    }
}

gpointer 
clist_get_data_selected (GtkCList *clist)
{
  if (clist->selection)
    return gtk_clist_get_row_data (clist,
				   GPOINTER_TO_INT (clist->selection->data));
  else
    return NULL;
}

gint
option_menu_get_active (GtkOptionMenu *omenu)
{
  GtkMenu *menu = GTK_MENU (gtk_option_menu_get_menu (omenu));
  int i = 0;
  GtkWidget *active;
  GList *list;

  active = gtk_menu_get_active (menu);

  list = GTK_MENU_SHELL (menu)->children;
  while (list) {
    if (list->data == active)
      return i;
    
    i++;

    list = g_list_next (list);
  }

  return 0;
}

void
option_menu_connect_changed (GtkOptionMenu *omenu, 
			     GtkSignalFunc cb, gpointer data)
{
  GtkMenu *menu = GTK_MENU (gtk_option_menu_get_menu (omenu));

  gtk_signal_connect (GTK_OBJECT (GTK_MENU_SHELL (menu)), "selection-done",
		      cb, data);
}

void 
mdi_set_tab_pos (GnomeMDI *mdi, int tab_pos)
{
  GList *list = mdi->windows;
  GtkWidget *nb;

  while (list) 
    if (GNOME_IS_APP (list->data)) {
      nb = GNOME_APP (list->data)->contents;
 
      if (GTK_IS_NOTEBOOK (nb))
        gtk_notebook_set_tab_pos (GTK_NOTEBOOK (nb), tab_pos);
    
      list = g_list_next (list);
    }
}

