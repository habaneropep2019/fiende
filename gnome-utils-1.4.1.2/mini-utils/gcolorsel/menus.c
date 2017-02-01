#include <config.h>

#include "menus.h"
#include "gcolorsel.h"
#include "mdi-color-generic.h"
#include "mdi-color-file.h"
#include "mdi-color-virtual-monitor.h"
#include "view-color-generic.h"
#include "view-color-edit.h"
#include "gcolorsel.h"
#include "session.h"
#include "utils.h"
#include "idle.h"
#include "dialogs.h"

#include "gnome.h"
#include <gdk/gdkx.h>

static GList *prop_list = NULL;  /* List of GnomePropertyBox */

/* New */
static void new_doc_cb  (GtkWidget *widget);
static void new_view_cb (GtkWidget *widget);

/* Close */
static void close_doc_cb (GtkWidget *widget);
static void close_view_cb (GtkWidget *widget);

/* File */
static void open_cb    (GtkWidget *widget);
static void save_cb    (GtkWidget *widget);
static void save_as_cb (GtkWidget *widget);
static void revert_cb  (GtkWidget *widget);
static void exit_cb    (GtkWidget *widget);

/* edit */
static void copy_cb    (GtkWidget *widget, gpointer data);
static void paste_cb    (GtkWidget *widget);
static void insert_color_cb  (GtkWidget *widget, gpointer data);
static void remove_cb     (GtkWidget *widget, gpointer data);
static void edit_cb       (GtkWidget *widget, gpointer data);
static void preferences_cb (GtkWidget *widget);
static void properties_cb (GtkWidget *widget, gpointer data);

/* Help */
static void about_cb   (GtkWidget *widget, gpointer data);

/* Other */
static void grab_cb    (GtkWidget *widget);

GnomeUIInfo new_menu[] = {
  GNOMEUIINFO_MENU_NEW_ITEM     (N_("New Document"), 
				 N_("Create a new document"),
				 new_doc_cb, NULL),
  GNOMEUIINFO_MENU_NEW_ITEM     (N_("New View"),
				 N_("Create a new view for a document"),
				 new_view_cb, NULL),
  GNOMEUIINFO_END
};

GnomeUIInfo close_menu[] = {
  GNOMEUIINFO_ITEM_STOCK       (N_("Close document"),
				N_("Close the current document"),
				close_doc_cb, GNOME_STOCK_MENU_CLOSE),
  GNOMEUIINFO_ITEM_STOCK        (N_("Close view"),
				N_("Close the current view"),
				close_view_cb, GNOME_STOCK_MENU_CLOSE),
  GNOMEUIINFO_END
};

GnomeUIInfo file_menu[] = {
  GNOMEUIINFO_MENU_NEW_SUBTREE  (new_menu),

  GNOMEUIINFO_SEPARATOR,
  
  GNOMEUIINFO_ITEM_STOCK        (N_("_Open palette..."),
                                 N_("Load a palette"),
                                 open_cb,    GNOME_STOCK_MENU_OPEN),
  GNOMEUIINFO_ITEM_STOCK        (N_("_Save palette"),
                                 N_("Save the current palette in a file"),
                                 save_cb,    GNOME_STOCK_MENU_SAVE),
  GNOMEUIINFO_ITEM_STOCK        (N_("Save palette _As..."),
                                 N_("Save the current palette with a different name"),
                                 save_as_cb, GNOME_STOCK_MENU_SAVE_AS),
/*  GNOMEUIINFO_MENU_REVERT_ITEM  (revert_cb,  NULL), */
  
  GNOMEUIINFO_SEPARATOR,

  GNOMEUIINFO_SUBTREE_STOCK     (N_("Close"), close_menu,
				 GNOME_STOCK_MENU_CLOSE),

  GNOMEUIINFO_SEPARATOR,
  
  GNOMEUIINFO_MENU_EXIT_ITEM    (exit_cb,    NULL),
  
  GNOMEUIINFO_END
};

GnomeUIInfo edit_menu[] = {
  GNOMEUIINFO_MENU_COPY_ITEM (copy_cb, NULL),
  GNOMEUIINFO_MENU_PASTE_ITEM (paste_cb, NULL),

  GNOMEUIINFO_SEPARATOR,

  GNOMEUIINFO_ITEM_STOCK (N_("Insert color"), NULL,
			  insert_color_cb, GNOME_STOCK_PIXMAP_ADD),
  GNOMEUIINFO_ITEM_STOCK (N_("Remove selected colors"), NULL, 
			  remove_cb, GNOME_STOCK_PIXMAP_REMOVE),
  GNOMEUIINFO_ITEM_NONE (N_("Edit selected colors ..."), NULL, edit_cb),
  
  GNOMEUIINFO_SEPARATOR,

  GNOMEUIINFO_ITEM_STOCK (N_("Document/View properties"), NULL,
                          properties_cb, GNOME_STOCK_MENU_PROP),
  GNOMEUIINFO_MENU_PREFERENCES_ITEM (preferences_cb, NULL),
			     
  GNOMEUIINFO_END
};

GnomeUIInfo help_menu[] = {
  GNOMEUIINFO_HELP ("gcolorsel"),
  GNOMEUIINFO_MENU_ABOUT_ITEM   (about_cb,   NULL),
  GNOMEUIINFO_END
};

GnomeUIInfo main_menu[] = {
  GNOMEUIINFO_MENU_FILE_TREE    (file_menu),
  GNOMEUIINFO_MENU_EDIT_TREE    (edit_menu),
  GNOMEUIINFO_MENU_HELP_TREE    (help_menu),
  GNOMEUIINFO_END
};

GnomeUIInfo docs_menu[] = {
  GNOMEUIINFO_END
};

GnomeUIInfo main_menu_with_doc[] = {
  GNOMEUIINFO_MENU_FILE_TREE    (file_menu),
  GNOMEUIINFO_MENU_EDIT_TREE    (edit_menu),
  GNOMEUIINFO_MENU_FILES_TREE   (docs_menu),
  GNOMEUIINFO_MENU_HELP_TREE    (help_menu),
  GNOMEUIINFO_END
};

/********************************* New ******************************/

static void
new_doc_cb (GtkWidget *widget)
{
  /*  MDIColorFile *file;
      
      file = MDI_COLOR_FILE (mdi_color_file_new ());
      mdi_color_generic_set_name (MDI_COLOR_GENERIC (file), _("New"));
      
      gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (file));
      gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (file));
      
      msg_flash (mdi, _("Document created ..."));*/
  dialog_new_doc ();
}

static void
new_view_cb (GtkWidget *widget)
{
  MDIColorGeneric *mcg;

  mcg = MDI_COLOR_GENERIC (gnome_mdi_get_active_child (mdi));
  
  if (mcg)
    dialog_new_view (mcg);
}

static gint
save_fail (char *filename)
{
  GtkWidget *dia;
  char *str;

  str = g_strdup_printf (_("GColorsel was unable to save the file :\n\n%s\n\nMake sure that the path you provided exists,\nand that you have the approprate write permissions.\n\nDo you want to retry ?"), filename);

  dia = gnome_message_box_new (str, GNOME_MESSAGE_BOX_ERROR,

			       GNOME_STOCK_BUTTON_YES, 
			       GNOME_STOCK_BUTTON_NO,
			       GNOME_STOCK_BUTTON_CANCEL,
			       NULL);
  g_free (str);

  return gnome_dialog_run_and_close (GNOME_DIALOG (dia));
}

static void
load_fail (char *filename)
{
  GtkWidget *dia;
  char *str;

  str = g_strdup_printf (_("GColorsel was unable to load the file:\n\n%s\n\nMake sure that the path you provided exists,\nand that you have permissions for opening the file."), filename);

  dia = gnome_message_box_new (str, GNOME_MESSAGE_BOX_ERROR,
			       GNOME_STOCK_BUTTON_OK, NULL);
  g_free (str);

  gnome_dialog_run_and_close (GNOME_DIALOG (dia));
}

/******************************* Open *******************************/

static void 
open_cb (GtkWidget *widget)
{
  GtkFileSelection *fs;
  gboolean cancel = TRUE, ret;
  MDIColorFile *file;
  char *filename;

  fs = GTK_FILE_SELECTION (gtk_file_selection_new (_("Open Palette")));
  gtk_file_selection_hide_fileop_buttons (fs);
  gtk_window_set_modal (GTK_WINDOW (fs), TRUE);

  gtk_signal_connect (GTK_OBJECT (fs->ok_button), "clicked", 
		      GTK_SIGNAL_FUNC (file_selection_ok_cb), &cancel);
  gtk_signal_connect (GTK_OBJECT (fs->cancel_button), "clicked", 
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

  gtk_signal_connect (GTK_OBJECT (fs), "delete_event",
		      GTK_SIGNAL_FUNC (file_selection_delete_event_cb), NULL);
  
  gtk_widget_show (GTK_WIDGET (fs));
  gtk_main ();

  if (!cancel) {
    filename = gtk_file_selection_get_filename (fs);
    
    file = MDI_COLOR_FILE (mdi_color_file_new ());
    mdi_color_file_set_filename (file, filename, FALSE);
    mdi_color_generic_set_name (MDI_COLOR_GENERIC (file), 
				g_basename (filename));

    gtk_widget_destroy (GTK_WIDGET (fs));
    
    msg_push (mdi, _("Loading file, please wait ..."));
    mdi_set_sensitive (mdi, FALSE);
    idle_block (); /* without that gtk_flush can take a long long time ... */
    gtk_flush ();

    ret = mdi_color_file_load (file, mdi);

    msg_pop (mdi);
    mdi_set_sensitive (mdi, TRUE);    
    idle_unblock ();
    
    if (ret) {
      gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (file));
      gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (file));
    } else {
      load_fail (file->filename);
      gtk_object_unref (GTK_OBJECT (file));
    }
  } else
    gtk_widget_destroy (GTK_WIDGET (fs));
}

/**************************** Save ************************************/

static char *
save_as_get_filename (void)
{
  GtkFileSelection *fs;
  gboolean cancel = TRUE;
  char *tmp;

  fs = GTK_FILE_SELECTION (gtk_file_selection_new (_("Save Palette as")));
  gtk_file_selection_hide_fileop_buttons (fs);
  gtk_window_set_modal (GTK_WINDOW (fs), TRUE);

  gtk_signal_connect (GTK_OBJECT (fs->ok_button), "clicked", 
		      GTK_SIGNAL_FUNC (file_selection_ok_cb), &cancel);
  gtk_signal_connect (GTK_OBJECT (fs->cancel_button), "clicked", 
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

  gtk_signal_connect (GTK_OBJECT (fs), "delete_event",
		      GTK_SIGNAL_FUNC (file_selection_delete_event_cb), NULL);

  gtk_widget_show (GTK_WIDGET (fs));
  gtk_main ();

  if (cancel) 
    tmp = NULL;
  else 
    tmp = g_strdup (gtk_file_selection_get_filename (fs));

  gtk_widget_destroy (GTK_WIDGET (fs));

  return tmp;
}

/* Ok : 0 ; Cancel : -1 ; Retry : 1 ; No Retry : 2 */
gint
save_file (MDIColorFile *mcf)
{
  gboolean change_name = FALSE;
  gint ret;
  
  if (! mcf->filename) {
    if (!(mcf->filename = save_as_get_filename ())) return -1;    
    change_name = TRUE;    
  }
  
  if (! mdi_color_file_save (mcf)) {
    ret = save_fail (mcf->filename);
    
    if (change_name) {
      g_free (mcf->filename);
      mcf->filename = NULL;
    }

    if ((ret >= 0) && (ret <= 2)) {

      msg_flash (mdi, _("File not saved ..."));

      switch (ret) {
      case 0: /* Yes (retry) */ return 1;
      case 1: /* No (retry)  */ return 2;
      case 2: /* Cancel      */ return -1;
      }
    }
  }
  
  msg_flash (mdi, _("File saved ..."));
  
  return 0;
}

static void 
save_cb (GtkWidget *widget)
{
  MDIColorGeneric *mcg;

  mcg = MDI_COLOR_GENERIC (gnome_mdi_get_active_child (mdi));

  if (IS_MDI_COLOR_FILE (mcg)) 
    while (save_file (MDI_COLOR_FILE (mcg)) == 1);
  else 
    display_todo ();
}

static void 
save_as_cb (GtkWidget *widget)
{
  char *old;
  MDIColorGeneric *mcg;
  MDIColorFile *mcf;

  mcg = MDI_COLOR_GENERIC (gnome_mdi_get_active_child (mdi));

  if (IS_MDI_COLOR_FILE (mcg)) {
    mcf = MDI_COLOR_FILE (mcg);

    while (1) {

      old = mcf->filename;
      mcf->filename = NULL;
      
      switch (save_file (mcf)) {
      case -1:
      case 2:
	mcf->filename = old;
	return;
      case 0 : /* Ok */
	g_free (old);
	return;
      }
    }
  } else
    display_todo ();
}

/******************************* Revert *******************************/

static void 
revert_cb (GtkWidget *widget)
{
  display_todo ();
}

/***************************** Close **********************************/

static void 
close_view_cb (GtkWidget *widget)
{
  GnomeMDIChild *child = gnome_mdi_get_active_child (mdi);
  MDIColorGeneric *mcg;
  GtkWidget *view;

  if (!child) return;

  mcg = MDI_COLOR_GENERIC (child);

  if (g_list_length (child->views) == 1) {

    if (! mcg->temp) {
      GtkWidget *dia;
      char *str;
      int ret;
            
      if (MDI_COLOR_GENERIC (child)->docs) 
	str = g_strdup_printf (_("I'm going to close the last view of '%s'.\n\nThis document will be closed too, continue ?\n\nPlease note that %d virtual document use data from this document."), MDI_COLOR_GENERIC (child)->name, g_list_length (MDI_COLOR_GENERIC (child)->docs));
      else 
      	str = g_strdup_printf (_("I'm going to close the last view of '%s'.\n\nThis document will be closed too, continue ?"), MDI_COLOR_GENERIC (child)->name);
            
      dia = gnome_message_box_new (str, GNOME_MESSAGE_BOX_QUESTION,
				   GNOME_STOCK_BUTTON_YES, 
				   GNOME_STOCK_BUTTON_NO,
				   NULL);
      g_free (str);
      
      ret = gnome_dialog_run_and_close (GNOME_DIALOG (dia));
      
      if (ret == 1) return;
    }

    gnome_mdi_remove_child (mdi, child, FALSE);
    return;
  }

  if ((view = gnome_mdi_get_active_view  (mdi))) 
    gnome_mdi_remove_view (mdi, view, FALSE);
}

static void
close_doc_cb (GtkWidget *widget)
{
  GnomeMDIChild *child = gnome_mdi_get_active_child (mdi);
  MDIColorGeneric *mcg;
  char *str;
  GtkWidget *dia;
  int ret;
  
  if (!child) return;
  mcg = MDI_COLOR_GENERIC (child);

  if (mcg->docs)
    str = g_strdup_printf (_("I'm going to close '%s'.\n\nPlease note that %d views will be destroyed,\nand that %d virtual document use data from this document.\n\nDo you want to continue ?"), mcg->name, g_list_length (child->views), g_list_length (mcg->docs));
  else
    str = g_strdup_printf (_("I'm going to close '%s'.\n\nPlease note that %d views will be destroyed.\n\nDo you want to continue ?"), mcg->name, g_list_length (child->views)); 

  dia = gnome_message_box_new (str, GNOME_MESSAGE_BOX_QUESTION,
			       GNOME_STOCK_BUTTON_YES, GNOME_STOCK_BUTTON_NO,
			       GNOME_STOCK_BUTTON_CANCEL, NULL);
  g_free (str);
  
  ret = gnome_dialog_run_and_close (GNOME_DIALOG (dia));
  
  if (ret == 2) return;
  
  if (ret == 0) 
    gnome_mdi_remove_child (mdi, child, FALSE);  
}

static void 
exit_cb (GtkWidget *widget)
{  
  /* Save prefs */
  prefs_save ();

  /* Save session */ 
  if (prefs.save_session) 
    session_save (mdi);

  if (gnome_mdi_remove_all (mdi, FALSE))
    gtk_object_destroy (GTK_OBJECT (mdi));
  else  /* Cancel ... */
    return;
}

static void
about_cb (GtkWidget *widget, gpointer data)
{
  static GtkWidget *about = NULL;
  
  gchar *authors[] = {
    "Eric Brayeur (eb2@ibelgique.com)",
    NULL
  };

  if (about) {
    gtk_widget_show_now (about);
    gdk_window_raise (about->window);
    return;
  }
  
  about = gnome_about_new (_("Gnome Color Browser"), VERSION
			   , "(C) 1997-98 Tim P. Gerla", 
			   (const gchar**)authors,
			   _("Small utility to browse available X11 colors."),
			   NULL);                                
  
  gtk_signal_connect (GTK_OBJECT (about), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);
  
  gtk_widget_show(about);
}

/****************** Edit Menu ***************************************/

static void 
copy_cb (GtkWidget *widget, gpointer data)
{
  GtkWidget *w;
  GList *l, *list;
  MDIColor *col;
  gint have_selection;
  ViewColorGeneric *view;
  GString *str;
  char *tmp;
  gboolean first = TRUE;

  w = gnome_mdi_get_active_view (mdi);
  if (! w) return;

  view = gtk_object_get_data (GTK_OBJECT (w), "view_object");
  
  l = list = VIEW_COLOR_GENERIC_GET_CLASS (view)->get_selected (view);
  if (!list) return;

  str = g_string_new (NULL);

  while (l) {
    col = l->data;
    col = mdi_color_generic_get_owner (col);

    if (!first) 
      g_string_append_c (str, '\n');
    else 
      first = FALSE;

    g_string_sprintfa (str, "%d %d %d\t\t%s", 
		       col->r, col->g, col->b, col->name);

    l = g_list_next (l);
  }

  g_list_free (list);

  tmp = gtk_object_get_data (GTK_OBJECT (event_widget), "col");
  if (tmp) g_free (tmp);

  have_selection = gtk_selection_owner_set (event_widget,
					    GDK_SELECTION_PRIMARY,
					    GDK_CURRENT_TIME);


  gtk_object_set_data (GTK_OBJECT (event_widget), "col", str->str);
  g_string_free (str, FALSE);
}

static void 
paste_cb (GtkWidget *widget)
{
  gtk_selection_convert (event_widget, GDK_SELECTION_PRIMARY, 
			 GDK_SELECTION_TYPE_STRING, GDK_CURRENT_TIME);
}

void
menu_edit (MDIColor *col)
{
  MDIColorVirtualMonitor *monitor;

  col = mdi_color_generic_get_owner (col);

  monitor = mdi_color_virtual_monitor_new ();
  mdi_color_generic_set_name (MDI_COLOR_GENERIC (monitor), _("Edit"));
  mdi_color_generic_set_temp (MDI_COLOR_GENERIC (monitor), TRUE);
  gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (monitor));

  mdi_color_virtual_monitor_add (monitor, col);
  
  mdi_color_generic_append_view_type (MDI_COLOR_GENERIC (monitor),
				      TYPE_VIEW_COLOR_EDIT);

  gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (monitor));

  mdi_color_generic_connect (MDI_COLOR_GENERIC (col->owner),
			     MDI_COLOR_GENERIC (monitor)); 
}

static void
insert_color_cb (GtkWidget *widget, gpointer data)
{
  MDIColor *col;
  ViewColorGeneric *view;
  GtkWidget *w;
  int pos;
  
  w = gnome_mdi_get_active_view (mdi);
  if (! w) return;
  
  view = gtk_object_get_data (GTK_OBJECT (w), "view_object");

  if (! mdi_color_generic_can_do (view->mcg, CHANGE_APPEND)) return;

  pos = VIEW_COLOR_GENERIC_GET_CLASS (view)->get_insert_pos (view);

  mdi_color_generic_freeze (view->mcg);
  col = mdi_color_generic_append_new (view->mcg, 255, 255, 255, _("New Color"));
  mdi_color_generic_change_pos (view->mcg, col, pos);
  mdi_color_generic_thaw (view->mcg);

  menu_edit (col);
}

static void 
remove_cb (GtkWidget *widget, gpointer data)
{
  ViewColorGeneric *view;
  GtkWidget *w;
  GList *l;
  MDIColorGeneric *mcg;
  
  w = gnome_mdi_get_active_view (mdi);
  if (! w) return;

  view = gtk_object_get_data (GTK_OBJECT (w), "view_object");

  VIEW_COLOR_GENERIC_GET_CLASS (view)->remove_selected (view);

  /* Remove doc if 1) is temporary doc AND 2) no more colors */

  l = mdi->children;
  while (l) {
    mcg = MDI_COLOR_GENERIC (l->data);
    l = g_list_next (l);

    if ((mcg->temp) && (mcg->last == -1)) 
      gnome_mdi_remove_child (mdi, GNOME_MDI_CHILD (mcg), FALSE);
  }
}

static void
edit_cb (GtkWidget *widget, gpointer data)
{
  GtkWidget *w;
  ViewColorGeneric *view;
  MDIColorVirtualMonitor *monitor;
  GList *list, *l;
  MDIColor *col;
  GList *connect = NULL;
  
  w = gnome_mdi_get_active_view (mdi);
  if (! w) return;

  view = gtk_object_get_data (GTK_OBJECT (w), "view_object");

g_assert (IS_VIEW_COLOR_GENERIC (view));
  l = list = VIEW_COLOR_GENERIC_GET_CLASS (view)->get_selected (view);
  if (!list) return;

  monitor = mdi_color_virtual_monitor_new ();
  mdi_color_generic_set_name (MDI_COLOR_GENERIC (monitor), _("Edit"));
  mdi_color_generic_set_temp (MDI_COLOR_GENERIC (monitor), TRUE);
  gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (monitor));

  while (l) {
    col = l->data;

    col = mdi_color_generic_get_owner (col);

    mdi_color_virtual_monitor_add (monitor, col);

    if (! g_list_find (connect, col->owner))
      connect = g_list_prepend (connect, col->owner);

    l = g_list_next (l);
  }

  mdi_color_generic_append_view_type (MDI_COLOR_GENERIC (monitor),
				      TYPE_VIEW_COLOR_EDIT);
  gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (monitor));

  l = connect;
  while (l) {
    mdi_color_generic_connect (MDI_COLOR_GENERIC (l->data),
			       MDI_COLOR_GENERIC (monitor)); 

    l = g_list_next (l);
  }

  g_list_free (connect);
  g_list_free (list);
}

/******* gcolorsel properties *********/

static void
preferences_cb (GtkWidget *widget)
{
  dialog_prefs ();
}

/******* Doc view / Properties ********/

static void
properties_apply_cb (GtkWidget *widget, int page, gpointer data)
{
  ViewColorGeneric *view = NULL, *view2;
  MDIColorGeneric *mcg = NULL, *mcg2;
  gpointer view_data, mcg_data, view2_data, mcg2_data;
  GList *list;
  GtkWidget *widget2;

  switch (page) {
  case 0 : 
    view      = gtk_object_get_data (GTK_OBJECT (widget), "prop-view");
    view_data = gtk_object_get_data (GTK_OBJECT (widget), "prop-view-data");
    VIEW_COLOR_GENERIC_GET_CLASS (view)->apply (view, view_data);
    break;

  case 1:        
    mcg       = gtk_object_get_data (GTK_OBJECT (widget), "prop-document");
    mcg_data  = gtk_object_get_data (GTK_OBJECT (widget), "prop-document-data");
    MDI_COLOR_GENERIC_GET_CLASS  (mcg)->apply  (mcg, mcg_data);
    break;

  default:
    return;
  }

  /* sync other properties */
  
  list = prop_list;
  while (list) {
    widget2 = list->data;

    if (widget2 != widget) {

      view2 = gtk_object_get_data (GTK_OBJECT (widget2), "prop-view");
      if (view2 == view) {
	view2_data = gtk_object_get_data (GTK_OBJECT (widget2), "prop-view-data");
	VIEW_COLOR_GENERIC_GET_CLASS (view2)->sync (view2, view2_data);
      }
      
      mcg2 = gtk_object_get_data (GTK_OBJECT (widget2), "prop-document");
      if (mcg2 == mcg) {
	mcg2_data = gtk_object_get_data (GTK_OBJECT (widget2), "prop-document-data");
	MDI_COLOR_GENERIC_GET_CLASS (mcg2)->sync (mcg2, mcg2_data);
      }

      /* Bugs ... when when view modified and doc sync ... */
      gnome_property_box_set_modified (GNOME_PROPERTY_BOX (widget2), FALSE);
    }      

    list = g_list_next (list);
  }
}

static void
properties_destroy_cb (GtkWidget *widget, gpointer data)
{
  ViewColorGeneric *view;
  MDIColorGeneric *mcg;
  gpointer view_data, mcg_data;

  view      = gtk_object_get_data (GTK_OBJECT (widget), "prop-view");
  view_data = gtk_object_get_data (GTK_OBJECT (widget), "prop-view-data");
  mcg       = gtk_object_get_data (GTK_OBJECT (widget), "prop-document");
  mcg_data  = gtk_object_get_data (GTK_OBJECT (widget), "prop-document-data");

  if (view) {
    VIEW_COLOR_GENERIC_GET_CLASS (view)->close (view, view_data);  
    gtk_signal_disconnect_by_data (GTK_OBJECT (view), widget);
  }

  if (mcg) {
    MDI_COLOR_GENERIC_GET_CLASS  (mcg)->close  (mcg, mcg_data);
    gtk_signal_disconnect_by_data (GTK_OBJECT (mcg), widget);
  }

  prop_list = g_list_remove (prop_list, widget);
}

static void
properties_changed_cb (gpointer data)
{ 
  gnome_property_box_changed (GNOME_PROPERTY_BOX (data));
}

static void
properties_mcg_destroy_cb (GtkWidget *widget, gpointer data)
{
  MDIColorGeneric *mcg;
  gpointer mcg_data;
  GtkWidget *vbox;

  mcg       = gtk_object_get_data (GTK_OBJECT (data), "prop-document");
  mcg_data  = gtk_object_get_data (GTK_OBJECT (data), "prop-document-data");
  vbox      = gtk_object_get_data (GTK_OBJECT (data), "prop-document-vbox");

  MDI_COLOR_GENERIC_GET_CLASS (mcg)->close (mcg, mcg_data);

  gtk_object_set_data (GTK_OBJECT (data), "prop-document", NULL);

  if (! gtk_object_get_data (GTK_OBJECT (data), "prop-view")) 
    gtk_object_destroy (GTK_OBJECT (data));
  else
    gtk_object_destroy (GTK_OBJECT (vbox));  
}

static void
properties_view_destroy_cb (GtkWidget *widget, gpointer data)
{
  ViewColorGeneric *view;
  gpointer view_data;
  GtkWidget *vbox;

  view      = gtk_object_get_data (GTK_OBJECT (data), "prop-view");
  view_data = gtk_object_get_data (GTK_OBJECT (data), "prop-view-data");
  vbox      = gtk_object_get_data (GTK_OBJECT (data), "prop-view-vbox");

  VIEW_COLOR_GENERIC_GET_CLASS (view)->close (view, view_data);

  gtk_object_set_data (GTK_OBJECT (data), "prop-view", NULL);

  if (! gtk_object_get_data (GTK_OBJECT (data), "prop-document")) 
    gtk_object_destroy (GTK_OBJECT (data));
  else
    gtk_object_destroy (GTK_OBJECT (vbox));  
}

static void
properties_cb (GtkWidget *widget, gpointer data)
{
  GtkWidget *property;
  ViewColorGeneric *view;
  GtkWidget *vbox_mcg, *vbox_view;
  MDIColorGeneric *mcg;
  gpointer view_data, mcg_data;
  
  view = gtk_object_get_data (GTK_OBJECT (mdi->active_view), "view_object");
  if (!view) return;
  
  mcg = VIEW_COLOR_GENERIC (view)->mcg;

  property = gnome_property_box_new ();

  gtk_signal_connect (GTK_OBJECT (property), "apply",
		      GTK_SIGNAL_FUNC (properties_apply_cb), NULL); 
  gtk_signal_connect (GTK_OBJECT (property), "destroy",
		      GTK_SIGNAL_FUNC (properties_destroy_cb), NULL);

  /* view properties */

  vbox_view = gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_view), GNOME_PAD);

  view_data = VIEW_COLOR_GENERIC_GET_CLASS (view)->get_control (view, 
		    GTK_VBOX (vbox_view), properties_changed_cb, property);

  gtk_signal_connect (GTK_OBJECT (view), "destroy", 
		      properties_view_destroy_cb, property);

  gnome_property_box_append_page (GNOME_PROPERTY_BOX (property), 
				  vbox_view, gtk_label_new (_("View properties")));

  /* document properties */

  vbox_mcg = gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_mcg), GNOME_PAD);

  mcg_data = MDI_COLOR_GENERIC_GET_CLASS (mcg)->get_control (mcg,
		    GTK_VBOX (vbox_mcg), properties_changed_cb, property);

  gtk_signal_connect (GTK_OBJECT (mcg), "destroy", 
		      properties_mcg_destroy_cb, property);

  gnome_property_box_append_page (GNOME_PROPERTY_BOX (property), 
				  vbox_mcg, 
				  gtk_label_new (_("Document properties")));

  gtk_object_set_data (GTK_OBJECT (property), "prop-view", view);
  gtk_object_set_data (GTK_OBJECT (property), "prop-view-data", view_data);
  gtk_object_set_data (GTK_OBJECT (property), "prop-view-vbox", vbox_view);
  gtk_object_set_data (GTK_OBJECT (property), "prop-document", mcg);
  gtk_object_set_data (GTK_OBJECT (property), "prop-document-data", mcg_data);
  gtk_object_set_data (GTK_OBJECT (property), "prop-document-vbox", vbox_mcg);

  VIEW_COLOR_GENERIC_GET_CLASS (view)->sync (view, view_data);
  MDI_COLOR_GENERIC_GET_CLASS  (mcg)->sync  (mcg, mcg_data);

  prop_list = g_list_append (prop_list, property);

  gtk_widget_show_all (property);
}

/*********************** View Popup Menu *******************************/

void
menu_view_do_popup (GdkEventButton *event, ViewColorGeneric *view)
{
  static GtkWidget *popup = NULL;

  if (!popup) 
    popup = gnome_popup_menu_new (edit_menu);

  gnome_popup_menu_do_popup (popup, NULL, NULL, event, view);
}

/************************* ToolBar *************************************/

GnomeUIInfo toolbar [] = {
  GNOMEUIINFO_ITEM_STOCK (N_("Open"), N_("Open a palette"),
			  open_cb, GNOME_STOCK_PIXMAP_OPEN),
  GNOMEUIINFO_ITEM_STOCK (N_("Close"), N_("Close the current view"),
			  close_view_cb, GNOME_STOCK_PIXMAP_CLOSE),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_ITEM_STOCK (N_("Grab"), N_("Grab a color on the screen"),
			  grab_cb, GNOME_STOCK_PIXMAP_JUMP_TO),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_ITEM_STOCK (N_("About"), N_("About this application"),
			  about_cb, GNOME_STOCK_PIXMAP_ABOUT),
  GNOMEUIINFO_ITEM_STOCK (N_("Exit"), N_("Exit the program"),
			  exit_cb, GNOME_STOCK_PIXMAP_EXIT),
  GNOMEUIINFO_END
};

/************************* Grab *******************************/

/* Mostly recopied from gnome-colorsel module */

static char dropper_bits[] = {
  0xff, 0x8f, 0x01, 0xff, 0x77, 0x01, 0xff, 0xfb, 0x00, 0xff, 0xf8, 0x00,
  0x7f, 0xff, 0x00, 0xff, 0x7e, 0x01, 0xff, 0x9d, 0x01, 0xff, 0xd8, 0x01,
  0x7f, 0xd4, 0x01, 0x3f, 0xee, 0x01, 0x1f, 0xff, 0x01, 0x8f, 0xff, 0x01,
  0xc7, 0xff, 0x01, 0xe3, 0xff, 0x01, 0xf3, 0xff, 0x01, 0xfd, 0xff, 0x01,
  0xff, 0xff, 0x01, };

static char dropper_mask[] = {
  0x00, 0x70, 0x00, 0x00, 0xf8, 0x00, 0x00, 0xfc, 0x01, 0x00, 0xff, 0x01,
  0x80, 0xff, 0x01, 0x00, 0xff, 0x00, 0x00, 0x7f, 0x00, 0x80, 0x3f, 0x00,
  0xc0, 0x3f, 0x00, 0xe0, 0x13, 0x00, 0xf0, 0x01, 0x00, 0xf8, 0x00, 0x00,
  0x7c, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x0d, 0x00, 0x00,
  0x02, 0x00, 0x00, };

#define DROPPER_WIDTH 17
#define DROPPER_HEIGHT 17
#define DROPPER_X_HOT 2
#define DROPPER_Y_HOT 16

static GdkCursor *picker_cursor = NULL;

static void grab_mouse_clicked_release (GtkWidget *widget, 
					GdkEventButton *event, gpointer data);

static void
initialize_cursor ()
{
  GdkColor fg, bg;

  GdkPixmap *pixmap =
    gdk_bitmap_create_from_data (NULL,
				 dropper_bits,
				 DROPPER_WIDTH, DROPPER_HEIGHT);
  GdkPixmap *mask =
    gdk_bitmap_create_from_data (NULL,
				 dropper_mask,
				 DROPPER_WIDTH, DROPPER_HEIGHT);

  gdk_color_white (gdk_colormap_get_system (), &bg);
  gdk_color_black (gdk_colormap_get_system (), &fg);

  picker_cursor = gdk_cursor_new_from_pixmap (pixmap, mask, &fg, &bg, 
					      DROPPER_X_HOT ,DROPPER_Y_HOT);

  gdk_pixmap_unref (pixmap);
  gdk_pixmap_unref (mask);
}

static void
grab_mouse_clicked_release (GtkWidget *widget, GdkEventButton *event, 
			    gpointer data)
{
  GdkImage *image;
  guint32 pixel;
  GdkVisual *visual;
  GdkColormap *colormap = gtk_widget_get_colormap (widget);
  XColor xcolor;
  double r = 0, g = 0, b = 0;

  image = gdk_image_get (GDK_ROOT_PARENT (),event->x_root, event->y_root, 1, 1);
  pixel = gdk_image_get_pixel (image, 0, 0);
  visual = gdk_colormap_get_visual (colormap);

  switch (visual->type) {
  case GDK_VISUAL_DIRECT_COLOR:
  case GDK_VISUAL_TRUE_COLOR:
    r = (double)((pixel & visual->red_mask)>>visual->red_shift)/
		  ((1<<visual->red_prec) - 1);
    g = (double)((pixel & visual->green_mask)>>visual->green_shift)/
                  ((1<<visual->green_prec) - 1);
    b = (double)((pixel & visual->blue_mask)>>visual->blue_shift)/
                  ((1<<visual->blue_prec) - 1);
    break;
  case GDK_VISUAL_STATIC_GRAY:
  case GDK_VISUAL_GRAYSCALE:
    r = (double)pixel/((1<<visual->depth) - 1);
    g = (double)pixel/((1<<visual->depth) - 1);
    b = (double)pixel/((1<<visual->depth) - 1);
    break;
  case GDK_VISUAL_STATIC_COLOR:
    xcolor.pixel = pixel;
    XQueryColor (GDK_DISPLAY (), GDK_COLORMAP_XCOLORMAP (colormap), &xcolor);
    r = xcolor.red/65535.0;
    g = xcolor.green/65535.0;
    b = xcolor.blue/65535.0;
    break;
  case GDK_VISUAL_PSEUDO_COLOR:
    r = colormap->colors[pixel].red/(double)0xffffff;
    g = colormap->colors[pixel].green/(double)0xffffff;
    b = colormap->colors[pixel].blue/(double)0xffffff;
    break;
  default:
    g_assert_not_reached ();
    break;
  }

  gtk_signal_disconnect_by_func (GTK_OBJECT (widget), 
				 grab_mouse_clicked_release, data);

  gdk_pointer_ungrab (0);

  actions_grab (r * 255, g * 255, b * 255);
}

static void 
grab_cb (GtkWidget *widget)
{
  gtk_signal_connect (GTK_OBJECT (widget), "button_release_event", 
		      grab_mouse_clicked_release, NULL);

  if (picker_cursor == NULL)
    initialize_cursor ();

  gdk_pointer_grab (widget->window,
		    FALSE,
		    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
		    NULL,
		    picker_cursor,
		    0);
}
