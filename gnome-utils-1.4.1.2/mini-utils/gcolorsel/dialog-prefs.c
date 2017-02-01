#include <config.h>
#include "gcolorsel.h"
#include "dialogs.h"
#include "utils.h"
#include "mdi-color-generic.h"
#include "mdi-color-virtual-rgb.h"

#include <gnome.h>
#include <glade/glade.h>

static GtkWidget *property = NULL;

static GtkWidget *check_save_session;

static GtkWidget *menu_mdi_mode;
static GtkWidget *menu_tab_pos;
static GtkWidget *check_display_doc;

static GtkWidget *menu_on_drop;
static GtkWidget *menu_on_grab;
static GtkWidget *menu_on_views;
static GtkWidget *menu_on_previews;

static GtkWidget *combo_on_drop;
static GtkWidget *combo_on_grab;
static GtkWidget *combo_on_views;
static GtkWidget *combo_on_previews;

static void actions_changed_cb (GtkWidget *widget, gpointer data);

static void
destroy_cb (GtkWidget *widget, gpointer data)
{
  property = NULL;
}

static actions_t
actions_active (GtkOptionMenu *omenu)
{
  GtkMenu *menu = GTK_MENU (gtk_option_menu_get_menu (omenu));
  GtkObject *active;

  active = GTK_OBJECT (gtk_menu_get_active (menu));
  
  return GPOINTER_TO_INT (gtk_object_get_data (active, "act"));
}

static int
actions2_active (GtkCombo *combo)
{
  GtkList *list = GTK_LIST (GTK_COMBO (combo)->list);
  MDIColorGeneric *mcg;
  GtkObject *object; 

  if (list->selection) {
    object = GTK_OBJECT (list->selection->data);
    
    mcg = MDI_COLOR_GENERIC (gtk_object_get_data (object, "mcg"));
    
    return mcg->key;
  }

  return -1;
}

static void
apply_cb (GtkWidget *widget, int page, gpointer data)
{
  switch (page) {
  case 0:
    
    prefs.save_session = 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_save_session));

    break;

  case 1:
    prefs.mdi_mode = option_menu_get_active (GTK_OPTION_MENU (menu_mdi_mode));
    if (prefs.mdi_mode == 3)
	    prefs.mdi_mode = GNOME_MDI_DEFAULT_MODE;
    prefs.tab_pos = option_menu_get_active (GTK_OPTION_MENU (menu_tab_pos)); 
    gnome_mdi_set_mode (mdi, prefs.mdi_mode);
    mdi_set_tab_pos (mdi, prefs.tab_pos);

    prefs.display_doc = 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_display_doc));

    break;

  case 2:
    prefs.on_drop = actions_active (GTK_OPTION_MENU (menu_on_drop));
    prefs.on_grab = actions_active (GTK_OPTION_MENU (menu_on_grab));
    prefs.on_views = actions_active (GTK_OPTION_MENU (menu_on_views));
    printf ("On views : %d\n", prefs.on_views);
    prefs.on_previews = actions_active (GTK_OPTION_MENU (menu_on_previews));
    
    prefs.on_drop2 = actions2_active (GTK_COMBO (combo_on_drop));
    prefs.on_grab2 = actions2_active (GTK_COMBO (combo_on_grab));
    prefs.on_views2 = actions2_active (GTK_COMBO (combo_on_views));
    prefs.on_previews2 = actions2_active (GTK_COMBO (combo_on_previews));

    break;
  }
}

static void
changed_cb (GtkWidget *widget, gpointer data)
{ 
  if (property)
    gnome_property_box_changed (GNOME_PROPERTY_BOX (property));
}

static void
fill_actions (GtkOptionMenu *omenu, actions_t active, gboolean edit)
{
  GtkMenu *menu = GTK_MENU (gtk_menu_new ());
  GtkWidget *widget;

  widget = gtk_menu_item_new_with_label (_("Do nothing"));
  gtk_object_set_data (GTK_OBJECT (widget), "act",
		       GINT_TO_POINTER (ACTIONS_NOTHING));
  gtk_menu_append (menu, widget);

  widget = gtk_menu_item_new_with_label (_("Append it to ->"));
  gtk_object_set_data (GTK_OBJECT (widget), "act",
		       GINT_TO_POINTER (ACTIONS_APPEND));
  gtk_menu_append (menu, widget);

  widget = gtk_menu_item_new_with_label (_("Search it in ->"));
  gtk_object_set_data (GTK_OBJECT (widget), "act", 
		       GINT_TO_POINTER (ACTIONS_SEARCH));
  gtk_menu_append (menu, widget);

  if (edit) {
    widget = gtk_menu_item_new_with_label (_("Edit it"));
    gtk_object_set_data (GTK_OBJECT (widget), "act", 
			 GINT_TO_POINTER (ACTIONS_EDIT));
    gtk_menu_append (menu, widget);
  }

  gtk_option_menu_set_menu (omenu, GTK_WIDGET (menu));

  gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), active);
}

static GtkWidget *
cfg_actions (GladeXML *gui, char *name, int prefs, GtkCombo *combo, 
	     gboolean edit)
{
  GtkWidget *menu;

  menu = glade_xml_get_widget (gui, name);

  fill_actions (GTK_OPTION_MENU (menu), prefs, edit);

  option_menu_connect_changed (GTK_OPTION_MENU (menu),
			       GTK_SIGNAL_FUNC (actions_changed_cb), combo);

  gtk_widget_set_sensitive (GTK_WIDGET (combo), 
			    (prefs != ACTIONS_NOTHING)&&(prefs != ACTIONS_EDIT));

  return menu;
}

static void
fill_actions2 (GtkCombo *combo, int active, actions_t actions)
{  
  GtkList *gtk_list = GTK_LIST (combo->list);
  GList *list;
  MDIColorGeneric *mcg;
  GtkWidget *item;

  gtk_list_clear_items (gtk_list, 0, -1);

  if ((actions == ACTIONS_NOTHING) || (actions == ACTIONS_EDIT)) {
    gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
    return;
  }

  list = mdi->children;
  while (list) {
    mcg = list->data;
    list = g_list_next (list);

    if ((actions == ACTIONS_APPEND) && 
	(! mdi_color_generic_can_do (mcg, CHANGE_APPEND))) continue;

    if ((actions == ACTIONS_SEARCH) && 
	(! IS_MDI_COLOR_VIRTUAL_RGB (mcg))) continue;

    item = gtk_list_item_new_with_label (mcg->name);
    gtk_object_set_data (GTK_OBJECT (item), "mcg", mcg);
    gtk_widget_show (item);
    gtk_container_add (GTK_CONTAINER (gtk_list), item);

    if (mcg->key == active)
      gtk_list_select_child (gtk_list, item);
  }

  gtk_widget_set_sensitive (GTK_WIDGET (combo), TRUE);  
}

static void
actions_changed_cb (GtkWidget *widget, gpointer data) 
{
  GtkObject *active;

  changed_cb (widget, data);

  active = GTK_OBJECT (gtk_menu_get_active (GTK_MENU (widget)));

  fill_actions2 (GTK_COMBO (data),
		 0, 
		 GPOINTER_TO_INT (gtk_object_get_data (active, "act")));
}

static GtkWidget *
cfg_actions2 (GladeXML *gui, char *name, actions_t action, int prefs)
{
  GtkWidget *combo;

  combo = glade_xml_get_widget (gui, name);

  fill_actions2 (GTK_COMBO (combo), prefs, action);

  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (combo)->list), 
		      "selection_changed", 
		      GTK_SIGNAL_FUNC (changed_cb), NULL);

  return combo;
}

void
dialog_prefs (void)
{  
  GladeXML *gui;
  int menupos;

  static GnomeHelpMenuEntry help_entry = { "gcolorsel", "index.html" };
  
  if (! property) {
    
    gui = glade_xml_new (GCOLORSEL_GLADEDIR "dialog-prefs.glade", NULL);
    g_return_if_fail (gui != NULL);
    
    property = glade_xml_get_widget (gui, "propertybox");
    g_assert (property != NULL);
    
    gtk_signal_connect (GTK_OBJECT (property), "apply",
			GTK_SIGNAL_FUNC (apply_cb), NULL); 
    gtk_signal_connect (GTK_OBJECT (property), "destroy",
			GTK_SIGNAL_FUNC (destroy_cb), NULL);
    gtk_signal_connect (GTK_OBJECT (property), "help",
			GTK_SIGNAL_FUNC (gnome_help_display), &help_entry);
			
    /* Session */

    check_save_session = glade_xml_get_widget (gui, "check-save-session");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_save_session),
				  prefs.save_session);
    gtk_signal_connect (GTK_OBJECT (check_save_session), "toggled",
			GTK_SIGNAL_FUNC (changed_cb), NULL);

    /* MDI */

    menu_mdi_mode = glade_xml_get_widget (gui, "menu-mdi-mode");
    menu_tab_pos  = glade_xml_get_widget (gui, "menu-tab-pos");

    menupos = prefs.mdi_mode;
    if (menupos == GNOME_MDI_DEFAULT_MODE)
	    menupos = 3;

    gtk_option_menu_set_history (GTK_OPTION_MENU (menu_mdi_mode),
				 prefs.mdi_mode);
    gtk_option_menu_set_history (GTK_OPTION_MENU (menu_tab_pos),
				 prefs.tab_pos);

    option_menu_connect_changed (GTK_OPTION_MENU (menu_mdi_mode),
				 changed_cb, NULL);
    option_menu_connect_changed (GTK_OPTION_MENU (menu_tab_pos),
				 changed_cb, NULL);

    check_display_doc = glade_xml_get_widget (gui, "check-display-doc");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_display_doc),
				  prefs.display_doc);
    gtk_signal_connect (GTK_OBJECT (check_display_doc), "toggled",
			GTK_SIGNAL_FUNC (changed_cb), NULL);

    /* Action 2 */

    combo_on_drop     = cfg_actions2 (gui, "combo-on-drop", 
				      prefs.on_drop, prefs.on_drop2);
    combo_on_grab     = cfg_actions2 (gui, "combo-on-grab", 
				      prefs.on_grab, prefs.on_grab2);
    combo_on_views    = cfg_actions2 (gui, "combo-on-views", 
				      prefs.on_views, prefs.on_views2);
    combo_on_previews = cfg_actions2 (gui, "combo-on-previews", 
				      prefs.on_previews, prefs.on_previews2);

    /* Actions */

    menu_on_drop     = cfg_actions (gui, "menu-on-drop", prefs.on_drop, 
				    GTK_COMBO (combo_on_drop), FALSE);
    menu_on_grab     = cfg_actions (gui, "menu-on-grab", prefs.on_grab, 
				    GTK_COMBO (combo_on_grab), FALSE);
    menu_on_views    = cfg_actions (gui, "menu-on-views", prefs.on_views, 
				    GTK_COMBO (combo_on_views), TRUE);
    menu_on_previews = cfg_actions (gui, "menu-on-previews", 
				    prefs.on_previews, 
				    GTK_COMBO (combo_on_previews), FALSE);
    
    gtk_object_unref (GTK_OBJECT (gui));
    
    gtk_widget_show_all (property);
    while (property) {
      gnome_dialog_run (GNOME_DIALOG (property));
    }
  } 
}
