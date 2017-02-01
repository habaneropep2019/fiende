#include <stdio.h>
#include <gnome.h>
#include <glade/glade.h>

#include "view-color-generic.h"
#include "view-color-grid.h"
#include "mdi-color-generic.h"
#include "widget-color-grid.h"
#include "menus.h"
#include "utils.h"
#include "gcolorsel.h"

static ViewColorGenericClass *parent_class = NULL;

static void view_color_grid_class_init (ViewColorGridClass *class);
static void view_color_grid_init       (ViewColorGrid *vcl);
static void view_color_grid_move_item  (ColorGrid *cg, int old_pos,
					int new_pos, ViewColorGrid *vcg,
					ViewColorGeneric *mcg);

static void view_color_grid_data_changed    (ViewColorGeneric *vcg, 
					     gpointer data);
static void view_color_grid_remove_selected (ViewColorGeneric *vcg);
static gint view_color_grid_get_insert_pos (ViewColorGeneric *vcg);
static GList *view_color_grid_get_selected (ViewColorGeneric *vcg);
static gpointer view_color_grid_get_control (ViewColorGeneric *vcg,
					     GtkVBox *box,
					     void (*changed_cb)(gpointer data),
					     gpointer change_data);
static void view_color_grid_apply (ViewColorGeneric *vcg, gpointer data);
static void view_color_grid_close (ViewColorGeneric *vcg, gpointer data);
static void view_color_grid_sync  (ViewColorGeneric *vcg, gpointer data);
static void view_color_grid_save  (ViewColorGeneric *vcg);
static void view_color_grid_load  (ViewColorGeneric *vcg);

static gint view_color_grid_button_press (GtkWidget *widget,
					  GdkEventButton *event, 
					  gpointer data);

GtkType 
view_color_grid_get_type (void)
{
  static guint cg_type = 0;

  if (!cg_type) {
    GtkTypeInfo cg_info = {
      "ViewColorGrid",
      sizeof (ViewColorGrid),
      sizeof (ViewColorGridClass),
      (GtkClassInitFunc) view_color_grid_class_init,
      (GtkObjectInitFunc) view_color_grid_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    cg_type = gtk_type_unique (view_color_generic_get_type (), &cg_info);
  }

  return cg_type;
}

static void
view_color_grid_class_init (ViewColorGridClass *class)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;
  ViewColorGenericClass *vcg_class;

  object_class = GTK_OBJECT_CLASS (class);
  parent_class = gtk_type_class (TYPE_VIEW_COLOR_GENERIC);
  widget_class = (GtkWidgetClass *)class;
  vcg_class    = (ViewColorGenericClass *)class;
  
  vcg_class->data_changed = view_color_grid_data_changed;
  vcg_class->remove_selected = view_color_grid_remove_selected;
  vcg_class->get_insert_pos  = view_color_grid_get_insert_pos;
  vcg_class->get_selected = view_color_grid_get_selected;
  vcg_class->get_control = view_color_grid_get_control;
  vcg_class->apply       = view_color_grid_apply;
  vcg_class->close       = view_color_grid_close;
  vcg_class->sync        = view_color_grid_sync;
  vcg_class->save        = view_color_grid_save;
  vcg_class->load        = view_color_grid_load;
}

static void
view_color_grid_init (ViewColorGrid *vcg)
{

}

static gint
view_color_grid_compare_func (gconstpointer ptr1, gconstpointer ptr2)
{
  ColorGridCol *col1 = (ColorGridCol *)ptr1, *col2 = (ColorGridCol *)ptr2;
  MDIColor *c1 = col1->data, *c2 = col2->data;

  if (c1->pos < c2->pos) return -1;
  if (c1->pos > c2->pos) return 1;

  return 0;
}


GtkObject *
view_color_grid_new (MDIColorGeneric *mcg)
{
  GtkObject *object;
  ColorGrid *cg;

  object = gtk_type_new (TYPE_VIEW_COLOR_GRID);

  VIEW_COLOR_GENERIC (object)->mcg = mcg;

  cg = COLOR_GRID (color_grid_new (view_color_grid_compare_func));
  VIEW_COLOR_GENERIC (object)->widget = GTK_WIDGET (cg);

  color_grid_can_move (cg, mdi_color_generic_can_do (mcg, CHANGE_POS));

  gtk_signal_connect (GTK_OBJECT (cg), "move_item", 
		      GTK_SIGNAL_FUNC (view_color_grid_move_item), object);

  gtk_signal_connect_after (GTK_OBJECT (cg), "button_press_event",
		      GTK_SIGNAL_FUNC (view_color_grid_button_press), object);
  
  return object;
}

static gint
view_color_grid_button_press (GtkWidget *widget, 
			      GdkEventButton *event, gpointer data)
{
  ColorGrid *cg = COLOR_GRID (widget);

  if (event->type == GDK_BUTTON_PRESS) {
    if (event->button == 3) {
      menu_view_do_popup (event, VIEW_COLOR_GENERIC (data));
      return FALSE;
    }
  } else

    if (event->type == GDK_2BUTTON_PRESS) {
      if (cg->last_clicked)
	actions_views (MDI_COLOR (cg->last_clicked->data));
    }
  
  return TRUE;
}

static void
item_destroy_notify (gpointer data)
{
  gtk_object_unref (GTK_OBJECT (data));
}

static void
view_color_grid_data_changed (ViewColorGeneric *vcg, gpointer data)
{
  GList *list = data;
  MDIColor *col;
  ColorGrid *cg = COLOR_GRID (vcg->widget);

  color_grid_freeze (cg);

  while (list) {
    col = list->data;

    if (col->change & CHANGE_APPEND) {
      gtk_object_ref (GTK_OBJECT (col));
      color_grid_append (cg, col->r, col->g, col->b, col, item_destroy_notify);    
    } else

      if (col->change & CHANGE_CLEAR) color_grid_clear (cg);
    
      else {

	if (col->change & CHANGE_REMOVE) 
	  color_grid_remove (cg, col);
	else {

	  if (col->change & CHANGE_RGB) 
	    color_grid_change_rgb (cg, col, col->r, col->g, col->b);
	
	/* CHANGE_NAME, don't care */
	
	  if (col->change & CHANGE_POS) {
	    color_grid_change_pos (cg, col);
	  }
	}
      }
    
    list = g_list_next (list);
  }

  color_grid_thaw (cg);
}

static gint
view_color_grid_get_insert_pos (ViewColorGeneric *vcg)
{
  ColorGrid *cg = COLOR_GRID (vcg->widget);
  MDIColor *col;

  if (!cg->last_clicked) return -1;

  col = (MDIColor *)cg->last_clicked->data;    

  return col->pos;
}

/*************** Remove Selected **************************************/

typedef struct remove_t {
  MDIColorGeneric *mcg;
  GList *col;
  GList *last;
} remove_t;

static remove_t *
remove_search (GList *list, MDIColorGeneric *mcg)
{
  while (list) {
    if (((remove_t *)list->data)->mcg == mcg) return list->data;
    list = g_list_next (list);
  }

  return NULL;
}

static gint
compare_func (gconstpointer ptr1, gconstpointer ptr2)
{
  MDIColor *col1 = (MDIColor *)ptr1, *col2 = (MDIColor *)ptr2;

  if (col1->pos < col2->pos) return -1;
  if (col1->pos > col2->pos) return 1;

  return 0;
}

static GList *
view_color_grid_get_selected (ViewColorGeneric *vcg)
{
  ColorGrid *cg = COLOR_GRID (vcg->widget);
  GList *list = cg->selected;
  GList *rem_list = NULL;

  while (list) {
    rem_list = g_list_append (rem_list, ((ColorGridCol *)list->data)->data);    

    list = g_list_next (list);
  }

  return rem_list;
}

static void
view_color_grid_remove_selected (ViewColorGeneric *vcg)
{
  ColorGrid *cg = COLOR_GRID (vcg->widget);
  GList *list = cg->selected;
  MDIColor *col;
  remove_t *remove;
  GList *remove_list = NULL;

  while (list) {  
    col = ((ColorGridCol *)list->data)->data;    
    col = mdi_color_generic_get_owner (col);

    remove = remove_search (remove_list, col->owner);
    if (remove) {
      if (((MDIColor *)remove->last->data)->pos > col->pos)
	remove->col = g_list_insert_sorted (remove->col, col,
					    compare_func);
      else {
	g_list_append (remove->last, col);
	remove->last = remove->last->next;
      }
    } else {
      remove = g_new0 (remove_t, 1);
      remove->mcg = col->owner;
      remove->col = g_list_append (NULL, col);
      remove->last = remove->col;

      remove_list = g_list_prepend (remove_list, remove);
    }

    list = g_list_next (list);
  }   

  list = remove_list;

  while (list) {
    remove = list->data;

    mdi_color_generic_freeze (remove->mcg);
    mdi_color_generic_remove_list (remove->mcg, remove->col);
    mdi_color_generic_thaw (remove->mcg);

    g_free (remove);

    list = g_list_next (list);
  }

  g_list_free (remove_list);
}

static void 
view_color_grid_move_item (ColorGrid *cg, int old_pos,
			   int new_pos, ViewColorGrid *vcg,
			   ViewColorGeneric *mvg)
{
  ColorGridCol *col = g_list_nth (cg->col, old_pos)->data;
  MDIColor *c = col->data;

  mdi_color_generic_freeze (mvg->mcg);
  mdi_color_generic_change_pos (mvg->mcg, c, new_pos);
  mdi_color_generic_thaw (mvg->mcg);
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
view_color_grid_get_control (ViewColorGeneric *vcg, GtkVBox *box,
			     void (*changed_cb)(gpointer data), 
			     gpointer change_data)
{
  prop_t *prop = g_new0 (prop_t, 1);
  GtkWidget *frame;
  GtkAdjustment *adj;

  prop->changed_cb  = changed_cb;
  prop->change_data = change_data;

  prop->parent_data = parent_class->get_control (vcg, box, changed_cb, change_data);

  prop->gui = glade_xml_new (GCOLORSEL_GLADEDIR "view-color-grid-properties.glade", "frame");
  g_return_val_if_fail (prop->gui != NULL, NULL);

  frame = glade_xml_get_widget (prop->gui, "frame");
  g_return_val_if_fail (frame != NULL, NULL);
  gtk_box_pack_start_defaults (GTK_BOX (box), frame);

  prop->spin_width = glade_xml_get_widget (prop->gui, "spin-width");  
  adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (prop->spin_width));
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (spin_changed_cb), prop);

  prop->spin_height = glade_xml_get_widget (prop->gui, "spin-height");  
  adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (prop->spin_height));
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed", 
		      GTK_SIGNAL_FUNC (spin_changed_cb), prop);

  return prop;
}

static void     
view_color_grid_apply (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;
  ColorGrid *cg = COLOR_GRID (vcg->widget);

  color_grid_set_col_width_height (cg,
        gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (prop->spin_width)),
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (prop->spin_height)));

  parent_class->apply (vcg, prop->parent_data);
}

static void 
view_color_grid_close (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;

  parent_class->close (vcg, prop->parent_data);

  gtk_object_unref (GTK_OBJECT (prop->gui));
  g_free (prop);
}

static void 
view_color_grid_sync (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;
  ColorGrid *cg = COLOR_GRID (vcg->widget);

  spin_set_value (GTK_SPIN_BUTTON (prop->spin_width), cg->col_width, prop);
  spin_set_value (GTK_SPIN_BUTTON (prop->spin_height), cg->col_height, prop);

  parent_class->sync (vcg, prop->parent_data);
}

/********************************* Config **********************************/

static void 
view_color_grid_save (ViewColorGeneric *vcg)
{
  ColorGrid *cg = COLOR_GRID (vcg->widget);

  gnome_config_set_int ("ColWidth", cg->col_width);
  gnome_config_set_int ("ColHeight", cg->col_height);

  parent_class->save (vcg);
}

static void 
view_color_grid_load (ViewColorGeneric *vcg)
{
  ColorGrid *cg = COLOR_GRID (vcg->widget);

  cg->col_width  = gnome_config_get_int ("ColWidth");
  cg->col_height = gnome_config_get_int ("ColHeight");

  parent_class->load (vcg);
}
