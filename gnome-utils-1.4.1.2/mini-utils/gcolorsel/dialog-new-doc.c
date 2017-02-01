#include "gcolorsel.h"
#include "dialogs.h"
#include "mdi-color-generic.h"
#include "utils.h"

#include "gnome.h"
#include "glade/glade.h"

GnomeDruid *druid;
GtkWidget *dia; 

/* D_TYPE */
GtkWidget *clist_doc_type;
GtkWidget *entry_name;
GtkWidget *text_description;

/* D_CONNECT */
GtkWidget *button_doc_add;
GtkWidget *button_doc_add_all;
GtkWidget *button_doc_remove;
GtkWidget *button_doc_remove_all;
GtkWidget *clist_doc_avail;
GtkWidget *clist_doc_connect;

/* D_VIEWS */
GtkWidget *button_views_add;
GtkWidget *button_views_add_all;
GtkWidget *button_views_remove;
GtkWidget *button_views_remove_all;
GtkWidget *clist_views_avail;
GtkWidget *clist_views_connect;

int current;

enum {
  D_TYPE,
  D_CONNECT,
  D_VIEWS
};

static GnomeDruidPage *
get_page (GnomeDruid *druid, int page)
{
  return g_list_nth (druid->children, page)->data;
}

static void
set_sensitive (GnomeDruid *druid, int page)
{
  int val;
  char *str;

  switch (page) {
  case D_TYPE:
    gnome_druid_set_show_finish (druid, FALSE);

    val = clist_get_data_selected (GTK_CLIST (clist_doc_type)) != NULL;
    
    str = g_strstrip (g_strdup (gtk_entry_get_text (GTK_ENTRY(entry_name))));

    val = val && str[0];
    
    gnome_druid_set_buttons_sensitive (druid, FALSE, val, TRUE);

    break;

  case D_CONNECT:
    gnome_druid_set_show_finish (druid, FALSE);
    
    val = GTK_CLIST (clist_doc_connect)->rows > 0;

    gnome_druid_set_buttons_sensitive (druid, TRUE, val, TRUE);

    break;

  case D_VIEWS:
    val = GTK_CLIST (clist_views_connect)->rows > 0;

    gnome_druid_set_show_finish (druid, TRUE);
    gnome_druid_set_buttons_sensitive (druid, TRUE, TRUE, TRUE);
    gtk_widget_set_sensitive (druid->finish, val);

    break;

  default:
    g_assert_not_reached ();
  }
}

static void
next_cb (GtkWidget *widget, gpointer data)
{
  docs_t *docs;
  int new_pos = current;

  switch (current) {
  case D_TYPE:
    docs = clist_get_data_selected (GTK_CLIST (clist_doc_type));
    g_assert (docs != NULL);

    if (docs) {
      if (docs->connect) 
	new_pos = D_CONNECT;
      else
	new_pos = D_VIEWS;
    }
    break;

  case D_CONNECT:
    new_pos = D_VIEWS;
    break;

  default:
    g_assert_not_reached ();
  }

  if (new_pos != current) {
    gnome_druid_set_page (druid, get_page (druid, new_pos));
    current = new_pos;
    set_sensitive (druid, current);
  }
}

static void
prev_cb (GtkWidget *widget, gpointer data)
{
  int new_pos = current;
  docs_t *docs;

  switch (current) {
  case D_CONNECT:
    new_pos = D_TYPE;
    break;
      
  case D_VIEWS:
    docs = clist_get_data_selected (GTK_CLIST (clist_doc_type));
    g_assert (docs != NULL);

    if (docs->connect) 
      new_pos = D_CONNECT;
    else
      new_pos = D_TYPE;
   
    break;
    
  default :
    g_assert_not_reached ();
  }

  if (new_pos != current) {
    gnome_druid_set_page (druid, get_page (druid, new_pos));
    current = new_pos;
    set_sensitive (druid, current);
  }
}

static void
cancel_cb (GtkWidget *widget, gpointer data)
{
  gtk_object_destroy (GTK_OBJECT (dia));
}

static void
finish_cb (GtkWidget *widget, gpointer data)
{
  docs_t *docs;
  MDIColorGeneric *mcg;
  GList *list;
  views_t *views;

  docs = clist_get_data_selected (GTK_CLIST (clist_doc_type));
  mcg = MDI_COLOR_GENERIC (gtk_type_new (docs->type ()));

  gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (mcg));
  
  mdi_color_generic_set_name (mcg, 
			      gtk_entry_get_text (GTK_ENTRY(entry_name)));

  if (docs->connect) {
    list = GTK_CLIST (clist_doc_connect)->row_list;
    while (list) {
      mdi_color_generic_connect (((GtkCListRow *)list->data)->data, mcg);

      list = g_list_next (list);
    }    
  }

  list = GTK_CLIST (clist_views_connect)->row_list;
  while (list) {
    views = ((GtkCListRow *)list->data)->data;

    mdi_color_generic_append_view_type (mcg, views->type ());
    gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (mcg));

    list = g_list_next (list);
  }

  gtk_object_destroy (GTK_OBJECT (dia));
}

/* D_TYPE */

static void
fill_clist_doc_type (GtkCList *clist)
{
  int i = 0;
  char *str[1];
  int pos;

  gtk_clist_freeze (clist);

  while (docs_tab[i].name) {
    str[0] = docs_tab[i].name;

    pos = gtk_clist_append (clist, str);
    gtk_clist_set_row_data (clist, pos, &docs_tab[i]);

    if (!i) gtk_clist_select_row (clist, pos, 0); /* Select default */

    i++;
  }

  gtk_clist_thaw (clist);
}

static void
clist_doc_type_row_selected (GtkWidget *widget, gpointer data)
{
  docs_t *docs;
  int pos = 0;
  char *str;

  set_sensitive (druid, current);

  docs = clist_get_data_selected (GTK_CLIST (clist_doc_type));

  gtk_editable_delete_text (GTK_EDITABLE (text_description), 0, -1);

  if (docs) {
    gtk_editable_insert_text (GTK_EDITABLE (text_description), 
			      docs->description,
			      strlen (docs->description), &pos);

    if (!gtk_object_get_data (GTK_OBJECT (entry_name), "modified")) {
      str = g_strdup (docs->name);
      pos = 0;
      while ((str[pos]) && (str[pos] != ' ')) pos++;
      if (str[pos]) str[pos] = 0;

      entry_set_text (GTK_ENTRY (entry_name), str, entry_name);
      g_free (str);
    }
  }
}

static void
entry_name_changed (GtkWidget *widget, gpointer data)
{
  set_sensitive (druid, current);
  
  gtk_object_set_data (GTK_OBJECT (widget), "modified", widget);
}

/* D_CONNECT */

static void
fill_clist_doc_avail (GtkCList *clist)
{
  GList *list = mdi->children;
  char *str[1];
  int pos;

  while (list) {
    str[0] = MDI_COLOR_GENERIC (list->data)->name;
    pos = gtk_clist_append (clist, str);
    gtk_clist_set_row_data (clist, pos, list->data);

    list = g_list_next (list);
  }
}

static void
connect_set_sensitive ()
{
  set_sensitive (druid, D_CONNECT);

  gtk_widget_set_sensitive (button_doc_remove,
			    GTK_CLIST (clist_doc_connect)->selection != NULL);
  gtk_widget_set_sensitive (button_doc_add,
			    GTK_CLIST (clist_doc_avail)->selection != NULL);

  gtk_widget_set_sensitive (button_doc_add_all,
			    GTK_CLIST (clist_doc_avail)->rows > 0);
  gtk_widget_set_sensitive (button_doc_remove_all,
			    GTK_CLIST ( clist_doc_connect)->rows > 0);
}

static void
connect_append (GtkCList *clist, MDIColorGeneric *mcg)
{
  char *str[1];
  int pos;

  str[0] = mcg->name;
  pos = gtk_clist_append (clist, str);
  gtk_clist_set_row_data (clist, pos, mcg); 
}

static void
connect_add_cb (GtkWidget *widget, gpointer data)
{
  GList *list = GTK_CLIST (clist_doc_avail)->selection;
  int pos;

  while (list) {
    pos = GPOINTER_TO_INT (list->data);

    connect_append (GTK_CLIST (clist_doc_connect), 
		    MDI_COLOR_GENERIC (
		   gtk_clist_get_row_data (GTK_CLIST (clist_doc_avail), pos)));

    list = g_list_next (list);

    gtk_clist_remove (GTK_CLIST (clist_doc_avail), pos);
  }

  connect_set_sensitive ();
}

static void
connect_add_all_cb (GtkWidget *widget, gpointer data)
{
  GList *list = GTK_CLIST (clist_doc_avail)->row_list;

  while (list) {
    connect_append (GTK_CLIST (clist_doc_connect), 
		    MDI_COLOR_GENERIC (((GtkCListRow *)list->data)->data));
    
    list = g_list_next (list);
  }

  gtk_clist_clear (GTK_CLIST (clist_doc_avail));

  connect_set_sensitive ();
}

static void
connect_remove_cb (GtkWidget *widget, gpointer data)
{
  GtkCList *clist = GTK_CLIST (clist_doc_connect);
  GList *list = clist->selection;
  int pos;
  MDIColorGeneric *mcg;

  while (list) {
    pos = GPOINTER_TO_INT (list->data);
    list = g_list_next (list);

    mcg = MDI_COLOR_GENERIC (gtk_clist_get_row_data (GTK_CLIST (clist_doc_connect), pos));

    connect_append (GTK_CLIST (clist_doc_avail),  mcg);

    gtk_clist_remove (clist, pos);
  }  

  connect_set_sensitive ();
}

static void
connect_remove_all_cb (GtkWidget *widget, gpointer data)
{
  gtk_clist_clear (GTK_CLIST (clist_doc_connect));

  gtk_clist_clear (GTK_CLIST (clist_doc_avail));
  fill_clist_doc_avail (GTK_CLIST (clist_doc_avail));

  connect_set_sensitive ();
}

/* D_VIEWS */

static void
views_append (GtkCList *clist, views_t *views)
{
  char *str[1];
  int pos;

  str[0] = views->name;
  pos = gtk_clist_append (clist, str);
  gtk_clist_set_row_data (clist, pos, views);
}

static void
fill_clist_views_avail (GtkCList *clist)
{
  int i = 0;

  while (views_tab[i].name) {
    views_append (clist, &views_tab[i]);

    i++;
  }
}

static void
views_set_sensitive ()
{
  set_sensitive (druid, D_VIEWS);

  gtk_widget_set_sensitive (button_views_remove,
			    GTK_CLIST (clist_views_connect)->selection != NULL);
  gtk_widget_set_sensitive (button_views_add,
			    GTK_CLIST (clist_views_avail)->selection != NULL);

  gtk_widget_set_sensitive (button_views_remove_all,
			    GTK_CLIST ( clist_views_connect)->rows > 0);
}

static void
views_add_cb (GtkWidget *widget, gpointer data)
{
  GList *list = GTK_CLIST (clist_views_avail)->selection;
  views_t *views;

  while (list) {
    views = gtk_clist_get_row_data (GTK_CLIST (clist_views_avail),
				    GPOINTER_TO_INT (list->data));

    views_append (GTK_CLIST (clist_views_connect), views);

    list = g_list_next (list);
  }

  views_set_sensitive ();
}

static void
views_add_all_cb (GtkWidget *widget, gpointer data)
{
  fill_clist_views_avail (GTK_CLIST (clist_views_connect));

  views_set_sensitive ();
}

static void
views_remove_cb (GtkWidget *widget, gpointer data)
{
  GList *list = GTK_CLIST (clist_views_connect)->selection;
  int pos;

  while (list) {
    pos = GPOINTER_TO_INT (list->data);
    list = g_list_next (list);

    gtk_clist_remove (GTK_CLIST (clist_views_connect), pos);
  }

  views_set_sensitive ();
}

static void
views_remove_all_cb (GtkWidget *widget, gpointer data)
{
  gtk_clist_clear (GTK_CLIST (clist_views_connect));

  views_set_sensitive ();
}

void
dialog_new_doc (void)
{
  GladeXML *gui;

  current = D_TYPE;

  gui = glade_xml_new (GCOLORSEL_GLADEDIR "dialog-new-doc.glade", NULL);
  g_return_if_fail (gui != NULL);

  dia = glade_xml_get_widget (gui, "dialog-new-doc");
  g_return_if_fail (dia != NULL);

  druid = GNOME_DRUID (glade_xml_get_widget (gui, "druid"));  

  /* D_TYPE */

  /* Name */
  entry_name = glade_xml_get_widget (gui, "entry-name");
  gtk_signal_connect (GTK_OBJECT (entry_name), "changed",
		      entry_name_changed, entry_name);

  /* Description */
  text_description = glade_xml_get_widget (gui, "text-description");

  /* Type */
  clist_doc_type = glade_xml_get_widget (gui, "clist-doc-type");

  gtk_signal_connect (GTK_OBJECT (clist_doc_type), "select_row", 
		      clist_doc_type_row_selected, NULL);
  gtk_signal_connect (GTK_OBJECT (clist_doc_type), "unselect_row", 
		      clist_doc_type_row_selected, NULL);

  fill_clist_doc_type (GTK_CLIST (clist_doc_type));
  
  /* D_CONNECT */
  clist_doc_avail       = glade_xml_get_widget (gui, "clist-doc-avail");
  clist_doc_connect     = glade_xml_get_widget (gui, "clist-doc-connect");
  button_doc_add        = glade_xml_get_widget (gui, "button-doc-add");
  button_doc_add_all    = glade_xml_get_widget (gui, "button-doc-add-all");
  button_doc_remove     = glade_xml_get_widget (gui, "button-doc-remove");
  button_doc_remove_all = glade_xml_get_widget (gui, "button-doc-remove-all");

  gtk_signal_connect (GTK_OBJECT (clist_doc_avail), "select_row",
		      connect_set_sensitive, NULL);
  gtk_signal_connect (GTK_OBJECT (clist_doc_avail), "unselect_row",
		      connect_set_sensitive, NULL);
  gtk_signal_connect (GTK_OBJECT (clist_doc_connect), "select_row",
		      connect_set_sensitive, NULL);
  gtk_signal_connect (GTK_OBJECT (clist_doc_connect), "unselect_row",
		      connect_set_sensitive, NULL);

  gtk_signal_connect (GTK_OBJECT (button_doc_add), "clicked",
		      connect_add_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (button_doc_remove), "clicked",
		      connect_remove_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (button_doc_add_all), "clicked",
		      connect_add_all_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (button_doc_remove_all), "clicked",
		      connect_remove_all_cb, NULL);

  fill_clist_doc_avail (GTK_CLIST (clist_doc_avail));
  connect_set_sensitive ();
 
  /* D_VIEWS */
  clist_views_avail       = glade_xml_get_widget (gui, "clist-views-avail");
  clist_views_connect     = glade_xml_get_widget (gui, "clist-views-connect");
  button_views_add        = glade_xml_get_widget (gui, "button-views-add");
  button_views_add_all    = glade_xml_get_widget (gui, "button-views-add-all");
  button_views_remove     = glade_xml_get_widget (gui, "button-views-remove");
  button_views_remove_all = glade_xml_get_widget (gui, "button-views-remove-all");

  gtk_signal_connect (GTK_OBJECT (clist_views_avail), "select_row",
		      views_set_sensitive, NULL);
  gtk_signal_connect (GTK_OBJECT (clist_views_avail), "unselect_row",
		      views_set_sensitive, NULL);
  gtk_signal_connect (GTK_OBJECT (clist_views_connect), "select_row",
		      views_set_sensitive, NULL);
  gtk_signal_connect (GTK_OBJECT (clist_views_connect), "unselect_row",
		      views_set_sensitive, NULL);

  gtk_signal_connect (GTK_OBJECT (button_views_add), "clicked",
		      views_add_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (button_views_remove), "clicked",
		      views_remove_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (button_views_add_all), "clicked",
		      views_add_all_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (button_views_remove_all), "clicked",
		      views_remove_all_cb, NULL);

 
  fill_clist_views_avail (GTK_CLIST (clist_views_avail));
  views_set_sensitive ();

  gtk_object_unref (GTK_OBJECT (gui));

  set_sensitive (druid, current);

  gtk_signal_connect (GTK_OBJECT (druid->next), "clicked", 
		      next_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (druid->back), "clicked", 
		      prev_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (druid->cancel), "clicked",
		      cancel_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (druid->finish), "clicked",
		      finish_cb, NULL);

  gtk_signal_connect_object (GTK_OBJECT (dia), "destroy", 
			     gtk_main_quit, NULL);

  gtk_widget_show (dia);

  gtk_main ();
}
