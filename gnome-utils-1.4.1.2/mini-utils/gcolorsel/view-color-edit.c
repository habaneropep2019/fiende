#include <stdio.h>
#include <gnome.h>
#include <glade/glade.h>

#include "view-color-generic.h"
#include "view-color-edit.h"
#include "mdi-color-generic.h"
#include "menus.h"
#include "utils.h"
#include "gcolorsel.h"

static ViewColorGenericClass *parent_class = NULL;

static void view_color_edit_class_init (ViewColorEditClass *class);
static void view_color_edit_init       (ViewColorEdit *vcl);

static void view_color_edit_data_changed (ViewColorGeneric *vcg, 
					  gpointer data);
static GList* view_color_edit_get_selected (ViewColorGeneric *vcg);

static gpointer 
view_color_edit_get_control       (ViewColorGeneric *vcg, GtkVBox *box,
				   void (*changed_cb)(gpointer data), 
				   gpointer change_data);
static void view_color_edit_apply (ViewColorGeneric *vcg,
				   gpointer data);
static void view_color_edit_close (ViewColorGeneric *vcg,
				   gpointer data);
static void view_color_edit_sync  (ViewColorGeneric *vcg,
				   gpointer data);

static const GtkTargetEntry preview_drag_targets[] = {
  { "application/x-color", 0 }
};

GtkType 
view_color_edit_get_type (void)
{
  static guint cg_type = 0;

  if (!cg_type) {
    GtkTypeInfo cg_info = {
      "ViewColorEdit",
      sizeof (ViewColorEdit),
      sizeof (ViewColorEditClass),
      (GtkClassInitFunc) view_color_edit_class_init,
      (GtkObjectInitFunc) view_color_edit_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    cg_type = gtk_type_unique (view_color_generic_get_type (), &cg_info);
  }

  return cg_type;
}

static void
view_color_edit_class_init (ViewColorEditClass *class)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;
  ViewColorGenericClass *vcg_class; 

  object_class = GTK_OBJECT_CLASS (class);
  parent_class = gtk_type_class (TYPE_VIEW_COLOR_GENERIC);
  widget_class = (GtkWidgetClass *)class;
  vcg_class    = (ViewColorGenericClass *)class;
  
  vcg_class->data_changed = view_color_edit_data_changed;
  vcg_class->get_selected = view_color_edit_get_selected;
/*  vcg_class->get_control = view_color_edit_get_control;
    vcg_class->apply       = view_color_edit_apply;
    vcg_class->close       = view_color_edit_close;
    vcg_class->sync        = view_color_edit_sync;*/
}

static void
view_color_edit_init (ViewColorEdit *vcg)
{
  vcg->editing = NULL;
}

static GList *
list_search_list (GtkList *list, MDIColor *col)
{
  GList *l = list->children;

  while (l) {
    if (gtk_object_get_data (l->data, "col") == col) break;
    l = g_list_next (l);
  }

  return l;
}

static GtkWidget *
list_search (GtkList *list, MDIColor *col)
{
  return list_search_list (list, col)->data;
}

static void
spin_set_rgb (ViewColorEdit *vce, MDIColor *col) 
{
  spin_set_value (GTK_SPIN_BUTTON (vce->spin_red), col->r, vce);
  spin_set_value (GTK_SPIN_BUTTON (vce->spin_green), col->g, vce);
  spin_set_value (GTK_SPIN_BUTTON (vce->spin_blue), col->b, vce);
}

static void
list_selection_changed_cb (GtkList *list, ViewColorEdit *vce)
{
  MDIColor *col;
  GtkWidget *widget;

  if (list->selection) {
    widget = list->selection->data;
    col = gtk_object_get_data (GTK_OBJECT (widget), "col");

    if (col != vce->editing) {
      if (! vce->editing) 
	gtk_widget_set_sensitive (VIEW_COLOR_GENERIC (vce)->widget, TRUE);

      vce->editing = col;

      entry_set_text (GTK_ENTRY (vce->entry_name), col->name, vce);

      spin_set_rgb (vce, col);
      preview_fill (vce->preview, col->r, col->g, col->b);

      /* Update button */

      gtk_widget_set_sensitive (vce->button_next, 
		      g_list_last (list->children)->data != list->selection->data);

      gtk_widget_set_sensitive (vce->button_prev, 
				list->selection->data != list->children->data);
    }
    
  } else 
    if (vce->editing) {
      vce->editing = FALSE;
      gtk_widget_set_sensitive (VIEW_COLOR_GENERIC (vce)->widget, FALSE);
      entry_set_text (GTK_ENTRY (vce->entry_name), "", vce);
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (vce->combo)->entry), "");
    }
}

static void
spin_rgb_value_changed_cb (GtkWidget *widget, ViewColorEdit *vce)
{
  MDIColor *col;

  col = mdi_color_generic_get_owner (vce->editing);

  mdi_color_generic_change_rgb (col->owner, col, 
	     gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (vce->spin_red)),
	     gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (vce->spin_green)),
	     gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (vce->spin_blue)));
}

static void
entry_changed_cb (GtkWidget *widget, ViewColorEdit *vce)
{
  MDIColor *col;

  col = mdi_color_generic_get_owner (vce->editing);

  mdi_color_generic_change_name (col->owner, col, 
				 gtk_entry_get_text (GTK_ENTRY (vce->entry_name)));
}

static void
preview_size_allocate_cb (GtkWidget *widget, 
			  GtkAllocation *allocation, ViewColorEdit *vce)
{
  if (vce->editing)
    preview_fill (vce->preview, vce->editing->r, vce->editing->g, vce->editing->b);
  else
    preview_fill (vce->preview, 0, 0, 0);
}

static void
preview_button_press_cb (GtkWidget *widget, GdkEventButton *event,
			 ViewColorEdit *vce)
{
  if (event->type == GDK_2BUTTON_PRESS) 
    actions_previews (vce->editing->r, vce->editing->g,
		      vce->editing->b);
}

static void 
preview_drag_begin_cb (GtkWidget *widget, GdkDragContext *context, 
		       ViewColorEdit *vce)
{
  gtk_drag_set_icon_widget (context, 
			    drag_window_create (vce->editing->r, 
						vce->editing->g, 
						vce->editing->b), -2, -2);
}

static void 
preview_drag_data_get_cb (GtkWidget *widget, 
			  GdkDragContext *context, 
			  GtkSelectionData *selection_data, 
			  guint info,
			  guint time, ViewColorEdit *vce)
{
  guint16 vals[4];

  vals[0] = (vce->editing->r / 255.0) * 0xffff;
  vals[1] = (vce->editing->g / 255.0) * 0xffff;
  vals[2] = (vce->editing->b / 255.0) * 0xffff;
  vals[3] = 0xffff;
    
  gtk_selection_data_set (selection_data, 
			  gdk_atom_intern ("application/x-color", FALSE),
			  16, (guchar *)vals, 8);
}

static void
button_prev_clicked_cb (GtkWidget *widget, ViewColorEdit *vce)
{
  GList *l;
  GtkWidget *item = NULL;
  GtkList *list = GTK_LIST (GTK_COMBO (vce->combo)->list);

  l = list_search_list (list, vce->editing);

  if ((l)&&(l->prev)) item = l->prev->data;
  
  if (item) 
    gtk_list_select_child (list, item);
}

static void
button_next_clicked_cb (GtkWidget *widget, ViewColorEdit *vce)
{
  GList *l;
  GtkWidget *item = NULL;
  GtkList *list = GTK_LIST (GTK_COMBO (vce->combo)->list);

  l = list_search_list (list, vce->editing);

  if ((l)&&(l->next)) item = l->next->data;
  
  if (item) 
    gtk_list_select_child (list, item);
}

GtkObject *
view_color_edit_new (MDIColorGeneric *mcg)
{
  GtkObject *object;
  ViewColorEdit *view;
  GladeXML *gui;
  GtkWidget *eventbox;

  object = gtk_type_new (TYPE_VIEW_COLOR_EDIT);
  view = VIEW_COLOR_EDIT (object);

  VIEW_COLOR_GENERIC (object)->mcg = mcg;

  gui = glade_xml_new (GCOLORSEL_GLADEDIR "view-color-edit.glade", "vbox");
  g_return_val_if_fail (gui != NULL, NULL);

  VIEW_COLOR_GENERIC (view)->widget = glade_xml_get_widget (gui, "vbox");

  /* Button */

  view->button_prev = glade_xml_get_widget (gui, "button-prev");
  view->button_next = glade_xml_get_widget (gui, "button-next");

  gtk_signal_connect (GTK_OBJECT (view->button_prev), "clicked",
		      GTK_SIGNAL_FUNC (button_prev_clicked_cb), object);
  gtk_signal_connect (GTK_OBJECT (view->button_next), "clicked",
		      GTK_SIGNAL_FUNC (button_next_clicked_cb), object);

  /* Preview */

  view->preview = glade_xml_get_widget (gui, "preview");
  eventbox = glade_xml_get_widget (gui, "eventbox");

  gtk_drag_source_set (view->preview, GDK_BUTTON1_MASK,
		       preview_drag_targets, 1, 
		       GDK_ACTION_COPY);

  gtk_signal_connect (GTK_OBJECT (view->preview), "size_allocate",
		      GTK_SIGNAL_FUNC (preview_size_allocate_cb), object);
  gtk_signal_connect (GTK_OBJECT (eventbox), "button_press_event",
		      GTK_SIGNAL_FUNC (preview_button_press_cb), object);
  gtk_signal_connect (GTK_OBJECT (view->preview), "drag_data_get",
		      GTK_SIGNAL_FUNC (preview_drag_data_get_cb), object);
    gtk_signal_connect (GTK_OBJECT (view->preview), "drag_begin",
		      GTK_SIGNAL_FUNC (preview_drag_begin_cb), object);

  /* entry Name */

  view->entry_name = glade_xml_get_widget (gui, "entry-name");
  gtk_signal_connect (GTK_OBJECT (view->entry_name), "changed",
		      GTK_SIGNAL_FUNC (entry_changed_cb), object);

  /* spin RGB */

  view->spin_red   = glade_xml_get_widget (gui, "spin-red");
  view->spin_green = glade_xml_get_widget (gui, "spin-green");
  view->spin_blue  = glade_xml_get_widget (gui, "spin-blue");

  spin_connect_value_changed (GTK_SPIN_BUTTON (view->spin_red),
			      GTK_SIGNAL_FUNC (spin_rgb_value_changed_cb), object);
  spin_connect_value_changed (GTK_SPIN_BUTTON (view->spin_green),
			      GTK_SIGNAL_FUNC (spin_rgb_value_changed_cb), object);
  spin_connect_value_changed (GTK_SPIN_BUTTON (view->spin_blue),
			      GTK_SIGNAL_FUNC (spin_rgb_value_changed_cb), object);

  /* spin Position */

  view->spin_position = glade_xml_get_widget (gui, "spin-position");

  /* combo */
  view->combo = glade_xml_get_widget (gui, "combo");
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (view->combo)->list), 
		      "selection_changed", 
		      GTK_SIGNAL_FUNC (list_selection_changed_cb), object);


  gtk_object_unref (GTK_OBJECT (gui));

  gtk_widget_set_sensitive (VIEW_COLOR_GENERIC (view)->widget, FALSE);

  return object;
}

static void
item_destroy_notify (GtkWidget *widget, gpointer data)
{
  gtk_object_unref (GTK_OBJECT (data));
}

static void
view_color_edit_data_changed (ViewColorGeneric *vcg, gpointer data)
{
  GList *list = data;
  MDIColor *col;
  GtkWidget *item;
  GtkCombo *combo = GTK_COMBO (VIEW_COLOR_EDIT (vcg)->combo);
  GtkList *gtk_list = GTK_LIST (combo->list);
  ViewColorEdit *vce = VIEW_COLOR_EDIT (vcg);
  int next_select = -1;

  while (list) {
    col = list->data;

    if (col->change & CHANGE_APPEND) {
      gtk_object_ref (GTK_OBJECT (col));

      item = gtk_list_item_new_with_label (col->name);
      gtk_object_set_data (GTK_OBJECT (item), "col", col);
      gtk_widget_show (item);
      gtk_container_add (GTK_CONTAINER (gtk_list), item);

      if (g_list_length (gtk_list->children) > 1)
	gtk_widget_set_sensitive (vce->button_next, TRUE);

      gtk_signal_connect (GTK_OBJECT (item), "destroy", 
			  GTK_SIGNAL_FUNC (item_destroy_notify), col);
    } 

    else
      
      if (col->change & CHANGE_REMOVE) {
	GList l; l.data = list_search (gtk_list, col); l.next = l.prev = NULL;
	
	next_select = gtk_list_child_position (gtk_list, l.data);
	if (next_select) next_select--;

	gtk_list_remove_items (gtk_list, &l);	
      }

      else

	if (col->change & CHANGE_CLEAR) {
	  gtk_list_clear_items (gtk_list, 0, -1);
	}

	else

	  if (col->change & CHANGE_POS) {

	  }

	  else {

	    if (col->change & CHANGE_NAME) {
	      if (col == vce->editing) {
		entry_set_text (GTK_ENTRY (vce->entry_name), col->name, vce);
		entry_set_text (GTK_ENTRY (combo->entry), col->name,
				combo);
	      }

	      item = list_search (gtk_list, col);
	      gtk_label_set_text (GTK_LABEL (GTK_BIN (item)->child), col->name);
	    }

	    if (col->change & CHANGE_RGB) {
	      if (col == vce->editing) {
		spin_set_rgb (vce, col);
		preview_fill (vce->preview, col->r, col->g, col->b);
	      }
	    }
	  }

    list = g_list_next (list);
  }

  if (next_select != -1)
    gtk_list_select_item (gtk_list, next_select);
}

static GList *
view_color_edit_get_selected (ViewColorGeneric *vcg)
{
  if (VIEW_COLOR_EDIT (vcg)->editing)
    return g_list_append (NULL, VIEW_COLOR_EDIT (vcg)->editing);
  else
    return NULL;
}

/*********************** PROPERTIES ***************************/

typedef struct prop_t {
  GladeXML *gui;

  gpointer parent_data;

  void (*changed_cb)(gpointer data);
  gpointer change_data;

  GtkWidget *spin_width;
  GtkWidget *spin_height;
} prop_t;

static void
spin_changed_cb (GtkWidget *widget, prop_t *prop)
{
  prop->changed_cb (prop->change_data);
}

static gpointer 
view_color_edit_get_control (ViewColorGeneric *vcg, GtkVBox *box,
			     void (*changed_cb)(gpointer data), 
			     gpointer change_data)
{
  prop_t *prop = g_new0 (prop_t, 1);
  GtkWidget *frame;

  prop->changed_cb  = changed_cb;
  prop->change_data = change_data;

  prop->parent_data = parent_class->get_control (vcg, box, changed_cb, change_data);

  prop->gui = glade_xml_new (GCOLORSEL_GLADEDIR "view-color-edit-properties.glade", "frame");
  g_assert (prop->gui != NULL);

  frame = glade_xml_get_widget (prop->gui, "frame");
  gtk_box_pack_start_defaults (GTK_BOX (box), frame);

  
  /*  prop->spin_width = glade_xml_get_widget (prop->gui, "spin-width");  */
      


  return prop;
}

static void     
view_color_edit_apply (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;

  /* gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (prop->spin_width)),
     gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (prop->spin_height))); */

  parent_class->apply (vcg, prop->parent_data);
}

static void 
view_color_edit_close (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;

  parent_class->close (vcg, prop->parent_data);

  gtk_object_unref (GTK_OBJECT (prop->gui));
  g_free (prop);
}

static void 
view_color_edit_sync (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;
/*  GtkAdjustment *adj;*/

  /* spin-width */
/*  adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (prop->spin_width));
  gtk_signal_handler_block_by_data (GTK_OBJECT (adj), prop);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON (prop->spin_width), cg->col_width);
  gtk_signal_handler_unblock_by_data (GTK_OBJECT (adj), prop);*/

  parent_class->sync (vcg, prop->parent_data);
}
