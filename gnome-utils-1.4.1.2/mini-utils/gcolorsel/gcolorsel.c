#include <config.h>

#include "gcolorsel.h"
#include "menus.h"
#include "mdi-color-generic.h"
#include "mdi-color-file.h"
#include "mdi-color-virtual-rgb.h"
#include "mdi-color-virtual-monitor.h"
#include "view-color-grid.h"
#include "view-color-list.h"
#include "view-color-edit.h"
#include "session.h"
#include "utils.h"

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <glade/glade.h>

GnomeMDI *mdi = NULL;
GtkWidget *event_widget = NULL;
prefs_t prefs;

/* First view is considered as the default view ... */
views_t views_tab[] = { {N_("List"), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS, 
			 (views_new)view_color_list_new, 
			 view_color_list_get_type,
			 "List view description TODO" },
			{N_("Grid"), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC,
			 (views_new)view_color_grid_new, 
			 view_color_grid_get_type,
			 "Grid view description TODO" },
			{N_("Edit"), GTK_POLICY_NEVER, GTK_POLICY_NEVER,
			 (views_new)view_color_edit_new, 
			 view_color_edit_get_type,
			 "Edit view description TODO" },
			
			{ NULL, 0, 0, NULL, 0 } };

docs_t docs_tab[] = { 
  { N_("File"),       mdi_color_file_get_type,            TRUE,  FALSE,
  "File document description TODO" },
  { N_("Search RGB"), mdi_color_virtual_rgb_get_type,     TRUE,  TRUE,  
  "Search RGB document description TODO" },
  { N_("Monitor"),    mdi_color_virtual_monitor_get_type, FALSE, TRUE,  
  "Monitor document description TODO" },
  { N_("Concat"),    mdi_color_virtual_get_type,         TRUE,  TRUE,  
  "Concat document description TODO" },
  { NULL, NULL, FALSE },
};

static GtkTargetEntry app_drop_targets[] = {
  { "application/x-color", 0 }
};

views_t *
get_views_from_type (GtkType type)
{
  int i = 0;

  while (views_tab[i].name) {
    if (views_tab[i].type () == type) return &views_tab[i];
    i++;
  }

  return NULL;
}

static void
call_get_type (void)
{
  int i = 0;

  while (views_tab[i].name) {
    views_tab[i].type ();
    i++;
  }

  i = 0;
  while (docs_tab[i].name) {
    docs_tab[i].type ();
    i++;
  }
}

static gint 
mdi_remove_child (GnomeMDI *mdi, MDIColorGeneric *mcg)
{
  GtkWidget *dia;
  int ret;
  char *str;

  if (mcg->other_views) return TRUE;

  if ((IS_MDI_COLOR_FILE (mcg)) && (mcg->modified)) {

    str = g_strdup_printf (_("'%s' document has been modified; do you wish to save it ?"), mcg->name);
    
    dia = gnome_message_box_new (str, GNOME_MESSAGE_BOX_QUESTION, 
				 GNOME_STOCK_BUTTON_YES, 
				 GNOME_STOCK_BUTTON_NO,
				 GNOME_STOCK_BUTTON_CANCEL, NULL);    
    g_free (str);
      
    ret = gnome_dialog_run_and_close (GNOME_DIALOG (dia));
    if (ret == 2) return FALSE;

    if (!ret)     
      while (1) {
	ret = save_file (MDI_COLOR_FILE (mcg));

	/* We need to save the filename in the config ...
	   because session is saved before this function */
	if (ret == 0) {
	  char *prefix = prefix = g_strdup_printf ("/gcolorsel/%d/", mcg->key);
	  gnome_config_push_prefix (prefix);
	  MDI_COLOR_GENERIC_GET_CLASS (mcg)->save (mcg); 
	  gnome_config_pop_prefix ();
	  gnome_config_sync ();
	}
	if (ret == -1) return FALSE;
	if (ret != 1) break;
      }
  }

  return TRUE;
}

static void
drag_data_received (GtkWidget *widget, GdkDragContext *context,
		    gint x, gint y,
		    GtkSelectionData *selection_data, guint info,
		    guint time)
{
  guint16 *data = (guint16 *)selection_data->data;
  int r, g, b;
  
  if (data) {
    r = (data[0] * 255.0) / 0xffff;
    g = (data[1] * 255.0) / 0xffff;
    b = (data[2] * 255.0) / 0xffff;

    actions_drop (r, g, b);  
  }
}

static void
app_created (GnomeMDI *mdi, GnomeApp *app)
{
  GtkWidget *statusbar;

  gtk_widget_set_usize (GTK_WIDGET (app), 320, 410);

  statusbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_USER);
  gnome_app_set_statusbar (GNOME_APP (app), statusbar);
  gnome_app_install_menu_hints (app, gnome_mdi_get_menubar_info (app));

  gtk_widget_set_usize (GTK_WIDGET (gnome_appbar_get_progress (GNOME_APPBAR (statusbar))), 60, -1);

  gtk_signal_connect (GTK_OBJECT (app), "drag_data_received",
		      GTK_SIGNAL_FUNC (drag_data_received), NULL);

  gtk_drag_dest_set (GTK_WIDGET (app), 
		     GTK_DEST_DEFAULT_MOTION |
		     GTK_DEST_DEFAULT_HIGHLIGHT |
		     GTK_DEST_DEFAULT_DROP,
		     app_drop_targets, 1,
		     GDK_ACTION_COPY);
}

static gint 
selection_clear (GtkWidget         *widget,
		 GdkEventSelection *event,
		 gpointer data)
{
  char *str = gtk_object_get_data (GTK_OBJECT (event_widget), "col");

  if (str) {
    g_free (str);
    gtk_object_set_data (GTK_OBJECT (event_widget), "col", NULL);
  }

  return TRUE;
}

static void 
selection_get (GtkWidget        *widget, 
	       GtkSelectionData *selection_data,
	       guint             info,
	       guint             time_stamp,
	       gpointer          data)
{
  char *str;

  str = gtk_object_get_data (GTK_OBJECT (event_widget), "col");

  gtk_selection_data_set (selection_data, GDK_SELECTION_TYPE_STRING,
			  8, str, strlen (str) + 1);
}

static void
paste_fail ()
{
  GtkWidget *dialog;
  char *str;

  str = _("GColorsel was unable to paste colors from the clipboard.");

  dialog = gnome_message_box_new (str, GNOME_MESSAGE_BOX_ERROR,
				  GNOME_STOCK_BUTTON_OK, NULL);

  gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
}

static void
paste_bad_doc ()
{
  GtkWidget *dialog;
  char *str;

  str = _("Please select a file document before.");

  dialog = gnome_message_box_new (str, GNOME_MESSAGE_BOX_ERROR,
				  GNOME_STOCK_BUTTON_OK, NULL);

  gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
}

static void 
selection_received (GtkWidget        *widget,
		    GtkSelectionData *selection_data, 
		    gpointer          data)
{
  char *str, name[256];
  int r, g, b;
  GList *list = NULL, *l;
  GtkObject *col;
  GnomeMDIChild *child;
  GtkWidget *w;
  ViewColorGeneric *view;
  int pos;

  if (selection_data->length < 0) return;
  if (selection_data->type != GDK_SELECTION_TYPE_STRING) return;

  child = gnome_mdi_get_active_child (mdi);
  if ((! child) || 
      (! mdi_color_generic_can_do (MDI_COLOR_GENERIC (child), CHANGE_APPEND))) {
    paste_bad_doc ();
    return;
  }

  w = gnome_mdi_get_active_view (mdi);
  if (! w) return;

  view = gtk_object_get_data (GTK_OBJECT (w), "view_object");

  str = (char *)selection_data->data;

  while (str[0]) {
   
    if (sscanf(str, "%d %d %d\t\t%255[^\n]\n", &r, &g, &b, name) < 3) {
      paste_fail ();

      l = list;
      while (l) {
	gtk_object_destroy (GTK_OBJECT (l->data));
	l = g_list_next (l);
      }

      return; 
    }

    col = mdi_color_new ();
    MDI_COLOR (col)->r = r; MDI_COLOR (col)->g = g; MDI_COLOR (col)->b = b;
    MDI_COLOR (col)->name = g_strdup (name);

    list = g_list_prepend (list, col);
    
    while ((str[0]) && (str[0] != '\n')) str++;
    while (str[0] == '\n') str++;
  }

  pos = VIEW_COLOR_GENERIC_GET_CLASS (view)->get_insert_pos (view);
  mdi_color_generic_freeze (MDI_COLOR_GENERIC (child));

  l = list;
  if (l) 
    while (l) {
      mdi_color_generic_append (MDI_COLOR_GENERIC (child), l->data);
      mdi_color_generic_change_pos (MDI_COLOR_GENERIC (child), l->data, pos);
      l = g_list_next (l);
    }
    
  mdi_color_generic_thaw (MDI_COLOR_GENERIC (child));
    
  g_list_free (list);
    
  return;
}

/**************************** Preferences *******************************/

void prefs_load (void)
{
  gnome_config_push_prefix ("/gcolorsel/gcolosel/");

  prefs.save_session = gnome_config_get_bool ("SaveSession=TRUE");
  prefs.display_doc  = gnome_config_get_bool ("DisplayDoc=FALSE");

  prefs.on_drop      = gnome_config_get_int ("OnDrop=-0");
  prefs.on_grab      = gnome_config_get_int ("OnGrab=-0");
  prefs.on_views     = gnome_config_get_int ("OnViews=-0");
  prefs.on_previews  = gnome_config_get_int ("OnPreviews=-0");

  prefs.on_drop2     = gnome_config_get_int ("OnDrop2=-0");
  prefs.on_grab2     = gnome_config_get_int ("OnGrab2=-0");
  prefs.on_views2    = gnome_config_get_int ("OnViews2=-0");
  prefs.on_previews2 = gnome_config_get_int ("OnPreviews2=-0");

  prefs.tab_pos = gnome_config_get_int ("TabPos=-1");
  if (prefs.tab_pos < GTK_POS_LEFT ||
      prefs.tab_pos > GTK_POS_BOTTOM)
	  prefs.tab_pos = GTK_POS_TOP;
  mdi->tab_pos = prefs.tab_pos;

  prefs.mdi_mode = gnome_config_get_int ("MDIMode=-1");
  if (prefs.mdi_mode != GNOME_MDI_NOTEBOOK &&
      prefs.mdi_mode != GNOME_MDI_TOPLEVEL &&
      prefs.mdi_mode != GNOME_MDI_MODAL &&
      prefs.mdi_mode != GNOME_MDI_DEFAULT_MODE)
	  prefs.mdi_mode = GNOME_MDI_NOTEBOOK;
  gnome_mdi_set_mode (mdi, prefs.mdi_mode);  

  gnome_config_pop_prefix ();
}

void prefs_save (void)
{
  gnome_config_push_prefix ("/gcolorsel/gcolosel/");

  gnome_config_set_bool ("SaveSession", prefs.save_session);
  gnome_config_set_bool ("DisplayDoc", prefs.display_doc);

  gnome_config_set_int ("OnDrop", prefs.on_drop);
  gnome_config_set_int ("OnGrab", prefs.on_grab);
  gnome_config_set_int ("OnViews", prefs.on_views);
  gnome_config_set_int ("OnPreviews", prefs.on_previews);

  gnome_config_set_int ("OnDrop2", prefs.on_drop2);
  gnome_config_set_int ("OnGrab2", prefs.on_grab2);
  gnome_config_set_int ("OnViews2", prefs.on_views2);
  gnome_config_set_int ("OnPreviews2", prefs.on_previews2);

  gnome_config_set_int ("TabPos", prefs.tab_pos);
  gnome_config_set_int ("MDIMode", prefs.mdi_mode);

  gnome_config_pop_prefix ();
  gnome_config_sync ();
}

static void 
set_menu (void)
{
  if (prefs.display_doc) {
    gnome_mdi_set_menubar_template (mdi, main_menu_with_doc);
    gnome_mdi_set_child_list_path (mdi, GNOME_MENU_FILES_PATH);
  } else {
    gnome_mdi_set_menubar_template (mdi, main_menu);
    gnome_mdi_set_child_list_path (mdi, NULL);
  }
}

/***************************** Action ****************************************/

static MDIColorGeneric *
search_mcg_from_key (int key)
{
  GList *list = mdi->children;
  MDIColorGeneric *mcg;

  while (list) {
    mcg = list->data;

    if (mcg->key == key) return mcg;

    list = g_list_next (list);
  }

  return NULL;
}

static void
actions_fail ()
{
  GtkWidget *dia;
  char *str;

  str = _("I can't execute this action. Check that :\n\n1. You have associated an action with this event.\n2. The associated document still exists.\n3. The type of the associated document.\n\nGo to preferences, in the edit menu.");

  dia = gnome_message_box_new (str, GNOME_MESSAGE_BOX_WARNING,
			       GNOME_STOCK_BUTTON_OK, NULL);

  gtk_widget_show (GTK_WIDGET (dia));
  /*  gnome_dialog_run_and_close (GNOME_DIALOG (dia)); */
  /* don't work ... */
}

static void
actions (int r, int g, int b, char *name, 
	 MDIColor *col,
	 actions_t type, int key)
{
  MDIColorGeneric *mcg;

  switch (type) {
  case ACTIONS_NOTHING:
    actions_fail ();
    break;

  case ACTIONS_APPEND:
    mcg = search_mcg_from_key (key);

    if ((mcg) && (mdi_color_generic_can_do (mcg, CHANGE_APPEND))) {
      if (col)
	mdi_color_generic_append_new (mcg, col->r, col->g, col->b, col->name);
      else
	mdi_color_generic_append_new (mcg, r, g, b, name);
    } else
      actions_fail ();

    break;

  case ACTIONS_EDIT:
    menu_edit (col);
    break;

  case ACTIONS_SEARCH:
    mcg = search_mcg_from_key (key);

    if ((mcg) && (IS_MDI_COLOR_VIRTUAL (mcg))) {
      GList *list;

      if (! col)
	mdi_color_virtual_rgb_set (MDI_COLOR_VIRTUAL_RGB (mcg), 
				   r, g, b, 
				   MDI_COLOR_VIRTUAL_RGB (mcg)->t);
      else
	mdi_color_virtual_rgb_set (MDI_COLOR_VIRTUAL_RGB (mcg), 
				   col->r, col->g, col->b, 
				   MDI_COLOR_VIRTUAL_RGB (mcg)->t);
      
      list = GNOME_MDI_CHILD (mcg)->views;
      g_assert (list != NULL);

      gnome_mdi_set_active_view (mdi, GTK_WIDGET (list->data));
    } else 
      actions_fail ();

    break;

  default:
    g_warning ("Bad action type %d", type);
  }
}

void 
actions_drop (int r, int g, int b)
{
  actions (r, g, b, "Dropped", NULL, prefs.on_drop, prefs.on_drop2);
}

void 
actions_grab (int r, int g, int b)
{
  actions (r, g, b, "Grabbed", NULL, prefs.on_grab, prefs.on_grab2);
}

void 
actions_previews (int r, int g, int b)
{
  actions (r, g, b, "New", NULL, prefs.on_previews, prefs.on_previews2);
}

void 
actions_views (MDIColor *col)
{
  actions (0, 0, 0, NULL, col, prefs.on_views, prefs.on_views2);
}

/**************************** Main *******************************/

int main (int argc, char *argv[])
{

  /* Initialize i18n */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);
  
  gnome_init ("gcolorsel", VERSION, argc, argv);    

  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-color-browser.png");
  glade_gnome_init ();
  
  /* Init GnomeMDI */
  mdi = GNOME_MDI (gnome_mdi_new ("gcolorsel", _("GColorsel")));

  gtk_signal_connect (GTK_OBJECT (mdi), "destroy",
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (mdi), "remove_child", 
		      GTK_SIGNAL_FUNC (mdi_remove_child), NULL);
  gtk_signal_connect (GTK_OBJECT (mdi), "app_created",
		      GTK_SIGNAL_FUNC (app_created), NULL);
  
  /* Load prefs */
  prefs_load ();
		      
  /* Init menu/toolbar */
  set_menu ();
  gnome_mdi_set_toolbar_template (mdi, toolbar);		      
		  
  /* For clipboard */
  event_widget = gtk_window_new (GTK_WINDOW_POPUP);

  gtk_selection_add_target (event_widget,
			    GDK_SELECTION_PRIMARY,
			    GDK_SELECTION_TYPE_STRING, 1);
  gtk_signal_connect (GTK_OBJECT (event_widget), "selection_clear_event",
		      GTK_SIGNAL_FUNC (selection_clear), NULL);
  gtk_signal_connect (GTK_OBJECT (event_widget), "selection_get",
		      GTK_SIGNAL_FUNC (selection_get), NULL);
  gtk_signal_connect (GTK_OBJECT (event_widget), "selection_received",
		      GTK_SIGNAL_FUNC (selection_received), NULL);
		      
  /* For gtk_type_from_name in session.c */
  call_get_type ();		      

  /* Load old session if user want that */
  if (prefs.save_session) {
    if (! session_load (mdi)) session_create (mdi, TRUE);
  } else
    /* Else, construct a default session */
    session_create (mdi, FALSE);

  /* Load all file */
  session_load_data (mdi);

  gtk_main ();

  return 0;
}

