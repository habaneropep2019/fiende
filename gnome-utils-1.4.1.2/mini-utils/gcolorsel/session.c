#include <config.h>
#include "session.h"
#include "mdi-color-generic.h"
#include "mdi-color-file.h"
#include "mdi-color-virtual-rgb.h"
#include "view-color-generic.h"
#include "view-color-list.h"
#include "view-color-grid.h"
#include "utils.h"
#include "gcolorsel.h"

#include "gnome.h"

static void session_delete_temp (GnomeMDI *mdi);

static gboolean corrupted;

/******************************* Save **********************************/

static char *
create_string_from_list (GList *list, char *data, int offset)
{
  char *buf;
  GString *string = g_string_new (NULL);
  char *d;

  while (list) {
    if (list->prev)
      g_string_append_c (string, ':');

    if (data) 
      d = gtk_object_get_data (GTK_OBJECT (list->data), data);
    else
      d = list->data;

    buf = g_strdup_printf ("%d", *((int *)(d + offset)));
    g_string_append (string, buf);
    g_free (buf);

    list = g_list_next (list);
  }

  buf = string->str;
  g_string_free (string, FALSE);

  return buf;
}

static void
save_views_config (GnomeMDIChild *child)
{
  GList *list = child->views;
  ViewColorGeneric *vcg;
  char *prefix;

  while (list) {    
    vcg = gtk_object_get_data (GTK_OBJECT (list->data), "view_object");

    prefix = g_strdup_printf ("/gcolorsel/%d/", vcg->key);
    gnome_config_clean_section (prefix);
    gnome_config_push_prefix (prefix);

    gnome_config_set_string ("Type", gtk_type_name (GTK_OBJECT_TYPE (vcg)));

    VIEW_COLOR_GENERIC_GET_CLASS (vcg)->save (vcg); 

    gnome_config_pop_prefix ();

    list = g_list_next (list);
  }
}

static void
save_config (GnomeMDI *mdi)
{
  GList *list = mdi->children;
  MDIColorGeneric *mcg;
  char *prefix;
  char *str;

  while (list) {
    mcg = list->data;
    
    prefix = g_strdup_printf ("/gcolorsel/%d/", mcg->key);
    gnome_config_clean_section (prefix);
    gnome_config_push_prefix (prefix);
    
    gnome_config_set_string ("Type", gtk_type_name (GTK_OBJECT_TYPE (mcg)));
    
    str = create_string_from_list (GNOME_MDI_CHILD (mcg)->views, "view_object",
				   G_STRUCT_OFFSET (ViewColorGeneric, key));
    gnome_config_set_string ("Views", str);
    g_free (str);

    str = create_string_from_list (mcg->parents, NULL,
				   G_STRUCT_OFFSET (MDIColorGeneric, key));
    gnome_config_set_string ("Parents", str);
    g_free (str);
    
    MDI_COLOR_GENERIC_GET_CLASS (mcg)->save (mcg); 

    gnome_config_pop_prefix ();
    g_free (prefix);

    save_views_config (GNOME_MDI_CHILD (mcg));
    
    list = g_list_next (list);
  }
}

void
session_save (GnomeMDI *mdi)
{
  save_config (mdi);
  gnome_mdi_save_state (mdi, "/gcolorsel/mdi");  

  gnome_config_push_prefix ("/gcolorsel/session/");
  gnome_config_set_bool ("FirstTime", FALSE);
  gnome_config_set_int  ("Key", get_config_key_pos ());
  gnome_config_pop_prefix ();

  gnome_config_sync ();
}

/******************************** Load ************************************/

static void
next_views (MDIColorGeneric *mcg, char *str)
{
  gchar **tab;
  char *prefix;
  char *buf;
  int i = 0;
  GtkType type;

  tab = g_strsplit (str, ":", 0);
  
  while (tab[i]) {
    prefix = g_strdup_printf ("/gcolorsel/%s/", tab[i]);
    gnome_config_push_prefix (prefix);
    g_free (prefix);

    buf = gnome_config_get_string ("Type");
    type = gtk_type_from_name (buf);
    g_free (buf);

    if (!type) {
      type = views_tab[0].type ();
      corrupted = TRUE;
    }

    mdi_color_generic_append_view_type (mcg, type);

    gnome_config_pop_prefix ();

    i++;
  }
  
  g_strfreev (tab);
}

static
GnomeMDIChild *child_create (const gchar *config)
{
  char *prefix;
  char *buf;
  GtkType type;
  GnomeMDIChild *child;

  prefix = g_strdup_printf ("/gcolorsel/%s/", config);
  gnome_config_push_prefix (prefix);
  g_free (prefix);

  buf = gnome_config_get_string ("Type");
  type = gtk_type_from_name (buf);
  g_free (buf);

  if (!type) {
    type = mdi_color_generic_get_type ();
    corrupted = TRUE;
  }

  buf = gnome_config_get_string ("Views");

  /* Create Child */

  child = GNOME_MDI_CHILD (gtk_type_new (type));
  MDI_COLOR_GENERIC (child)->key = atoi (config);

  /* Load child properties */
  MDI_COLOR_GENERIC_GET_CLASS (child)->load (MDI_COLOR_GENERIC (child)); 

  gnome_config_pop_prefix ();

  next_views (MDI_COLOR_GENERIC (child), buf);
  g_free (buf);

  return child;
}

static void
load_views_config (GnomeMDIChild *child, char *str)
{
  GList *list = child->views;
  ViewColorGeneric *vcg;
  char *prefix;
  char **tab;
  int i = 0;

  tab = g_strsplit (str, ":", 0);

  while (list) {    
    vcg = gtk_object_get_data (GTK_OBJECT (list->data), "view_object");

    vcg->key = atoi (tab[i]);

    /* Load properties */
    prefix = g_strdup_printf ("/gcolorsel/%d/", vcg->key);
    gnome_config_push_prefix (prefix);
    VIEW_COLOR_GENERIC_GET_CLASS (vcg)->load (vcg); 
    gnome_config_pop_prefix ();

    list = g_list_next (list); i++;
  }

  g_strfreev (tab);
}

static MDIColorGeneric *
search_from_key (GnomeMDI *mdi, int key)
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
connect_to_parents (GnomeMDI *mdi, MDIColorGeneric *mcg, char *str)
{
  char **tab;
  int i = 0;
  MDIColorGeneric *parent;

  tab = g_strsplit (str, ":", 0);

  while (tab[i]) {
    parent = search_from_key (mdi, atoi (tab[i]));

    if (parent)
      mdi_color_generic_connect (parent, mcg);

    i++;
  }

  g_strfreev (tab);
}

static void
load_config (GnomeMDI *mdi)
{
  GList *list = mdi->children;
  MDIColorGeneric *mcg;
  char *prefix;
  char *str;

  while (list) {
    mcg = list->data;
    
    prefix = g_strdup_printf ("/gcolorsel/%d/", mcg->key);
    gnome_config_push_prefix (prefix);
    g_free (prefix);

    /* Configure views */
    str = gnome_config_get_string ("Views");    
    load_views_config (GNOME_MDI_CHILD (mcg), str);
    g_free (str);

    /* Connect to parent */
    str = gnome_config_get_string ("Parents");
    if (str) {
      connect_to_parents (mdi, mcg, str);
      g_free (str);
    }

    gnome_config_pop_prefix ();
    
    list = g_list_next (list);
  }
}

static int
session_fail ()
{
  GtkWidget *dia;

  dia = gnome_message_box_new (_("Your config file is corrupted.\nDo you want to create a new session ?"), GNOME_MESSAGE_BOX_ERROR, GNOME_STOCK_BUTTON_YES, GNOME_STOCK_BUTTON_NO, NULL);

  return gnome_dialog_run_and_close (GNOME_DIALOG (dia));
}

gboolean 
session_load (GnomeMDI *mdi)
{
  corrupted = FALSE;

  if (! gnome_config_get_bool ("/gcolorsel/session/FirstTime=TRUE")) {

    gnome_mdi_restore_state (mdi, "/gcolorsel/mdi", child_create);

    load_config (mdi);

    if (corrupted) 
      if (! session_fail ()) {
	gnome_mdi_remove_all (mdi, TRUE);
	return FALSE;
      }

    session_delete_temp (mdi);

    set_config_key_pos (gnome_config_get_int ("/gcolorsel/session/Key"));

    return TRUE;
  } 

  return FALSE;
}

static
void session_delete_temp (GnomeMDI *mdi)
{
  GList *list = mdi->children;
  GnomeMDIChild *child;
  
  while (list) {
    child = list->data;
    list = g_list_next (list);

    if (MDI_COLOR_GENERIC (child)->temp) 
      gnome_mdi_remove_child (mdi, child, FALSE);    
  }
}

void
session_load_data (GnomeMDI *mdi)
{
  GList *list = mdi->children;
  GString *str = NULL;

  msg_push (mdi, _("Loading files, please wait ..."));
  mdi_set_sensitive (mdi, FALSE);
  gtk_flush ();

  while (list) {
    if (IS_MDI_COLOR_FILE (list->data) && (MDI_COLOR_FILE (list->data)->filename)) {
      if (! mdi_color_file_load (MDI_COLOR_FILE (list->data), mdi)) {
	if (! str) 
	  str = g_string_new (MDI_COLOR_FILE (list->data)->filename);
	else {
	  g_string_append (str, "\n");
	  g_string_append (str, MDI_COLOR_FILE (list->data)->filename);
	}
      }
    }

    list = g_list_next (list);
  }

  mdi_set_sensitive (mdi, TRUE);
  msg_pop (mdi);

  if (str) {
    GtkWidget *dia;

    g_string_prepend (str,_("One or more files can not be restored :\n\n"));

    dia = gnome_message_box_new (str->str, GNOME_MESSAGE_BOX_WARNING,
				 GNOME_STOCK_BUTTON_OK, NULL);

    g_string_free (str, TRUE);

    gnome_dialog_run_and_close (GNOME_DIALOG (dia));
  }
}
       
/******************************* Create ***********************************/

void
session_create (GnomeMDI *mdi, gboolean init_actions)
{
  GtkWidget *first;
  MDIColorFile *file_sys, *file_usr;
  MDIColorVirtualRGB *virtual;
  char *buf;

  /* Create a file document for the system colors */
  file_sys = mdi_color_file_new ();
  gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (file_sys));
  
  mdi_color_file_set_filename (file_sys, XLIBDIR "X11/rgb.txt", FALSE);
  mdi_color_generic_set_name (MDI_COLOR_GENERIC (file_sys), _("System Colors"));
  
  /* Add a ColorList view for the system colors document */
  gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (file_sys));
  first = gnome_mdi_get_active_view (mdi);

  /* Add a ColorGrid view for the system colors document */
  mdi_color_generic_append_view_type (MDI_COLOR_GENERIC (file_sys), 
				      TYPE_VIEW_COLOR_GRID);
  gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (file_sys));
  
  /* Create a file document for the user colors */
  file_usr = mdi_color_file_new ();
  gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (file_usr));
  
  /* Add a ColorList view for the user colors document */
  gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (file_usr));
  
  buf = gnome_util_prepend_user_home ("/.gcolorsel_colors");
  mdi_color_file_set_filename (file_usr, buf, TRUE);
  g_free (buf);
  mdi_color_generic_set_name (MDI_COLOR_GENERIC (file_usr), _("User Colors"));  

  /* Create a search document */
  virtual = mdi_color_virtual_rgb_new ();
  gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (virtual));

  /* Configure search */
  mdi_color_virtual_rgb_set (virtual, 255, 255, 255, 100);
  mdi_color_generic_set_name (MDI_COLOR_GENERIC (virtual), _("Search"));
  
  /* Add a ColorList view for the search document  */  
  gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (virtual)); 

  /* Connect Search document to users and system documents */
  mdi_color_generic_connect (MDI_COLOR_GENERIC (file_sys),
			     MDI_COLOR_GENERIC (virtual));
			     
  mdi_color_generic_connect (MDI_COLOR_GENERIC (file_usr),
                             MDI_COLOR_GENERIC (virtual));			      
 
  /* Tell the MDI to display ColorList for the file document */
  gnome_mdi_set_active_view (mdi, first);
  
  if (init_actions) {
    int key_search = MDI_COLOR_GENERIC (virtual)->key;
    int key_user   = MDI_COLOR_GENERIC (file_usr)->key;

    prefs.on_drop  = ACTIONS_APPEND;
    prefs.on_drop2 = key_user;

    prefs.on_grab  = ACTIONS_SEARCH;
    prefs.on_grab2 = key_search;

    prefs.on_views = ACTIONS_EDIT;

    prefs.on_previews  = ACTIONS_SEARCH;
    prefs.on_previews2 = key_search;
  }
}
