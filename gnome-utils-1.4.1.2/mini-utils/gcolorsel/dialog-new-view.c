#include "gcolorsel.h"
#include "dialogs.h"
#include "mdi-color-generic.h"

#include "gnome.h"
#include "glade/glade.h"

GtkCList *clist_view;
GtkWidget *dia;
GtkWidget *text_description;
MDIColorGeneric *mcg;

static void 
dialog_fill_clist_view (GtkCList *clist)
{
  int i = 0;
  int pos;
  char *str[1];

  gtk_clist_freeze (clist);

  while (views_tab[i].name) {
    str[0] = views_tab[i].name;

    pos = gtk_clist_append (clist, str);

    gtk_clist_set_row_data (clist, pos, &views_tab[i]);
    
    if (!i) gtk_clist_select_row (clist, pos, 0); /* Select default */

    i++;
  }

  gtk_clist_sort (clist);
  gtk_clist_thaw (clist);
}

static void
clist_select (GtkWidget *widget, gint row, gint column, gpointer data)
{
  views_t *views;
  int pos = 0;

  gnome_dialog_set_sensitive (GNOME_DIALOG (dia), 0, 
			      clist_view->selection != NULL);

  gtk_editable_delete_text (GTK_EDITABLE (text_description), 0, -1);

  if (clist_view->selection) {
    views = gtk_clist_get_row_data (clist_view, 
		       GPOINTER_TO_INT (clist_view->selection->data));

    gtk_editable_insert_text (GTK_EDITABLE (text_description), 
			      views->description,
			      strlen (views->description), &pos);    
  }
}

static void
create_view (void)
{
  views_t *views;

  g_assert (clist_view->selection != NULL);

  views = gtk_clist_get_row_data (clist_view,
				GPOINTER_TO_INT (clist_view->selection->data));
  g_assert (views != NULL);


  mdi_color_generic_append_view_type (mcg, views->type ());
  gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (mcg));
}

void
dialog_new_view (MDIColorGeneric *mcg_connect)
{
  GladeXML *gui;
  int result;

  mcg = mcg_connect;

  gui = glade_xml_new (GCOLORSEL_GLADEDIR "dialog-new-view.glade", NULL);
  g_return_if_fail (gui != NULL);

  dia = glade_xml_get_widget (gui, "dialog-new-view");
  g_return_if_fail (dia != NULL);

  clist_view = GTK_CLIST (glade_xml_get_widget (gui, "clist-view"));
  text_description = glade_xml_get_widget (gui, "text-description");

  gtk_object_unref (GTK_OBJECT (gui));

  gtk_signal_connect (GTK_OBJECT (clist_view), "select_row",
		      GTK_SIGNAL_FUNC (clist_select), NULL);
  gtk_signal_connect (GTK_OBJECT (clist_view), "unselect_row",
		      GTK_SIGNAL_FUNC (clist_select), NULL);

  dialog_fill_clist_view (clist_view);

  while (1) {
    result = gnome_dialog_run (GNOME_DIALOG (dia));

    if (result == 2) break;
    
    if (result == 1) { /* FIXME : Help */
    }

    if (result == 0) { /* Ok */
      create_view ();
      break;
    }
  } 

  gnome_dialog_close (GNOME_DIALOG (dia));
}
