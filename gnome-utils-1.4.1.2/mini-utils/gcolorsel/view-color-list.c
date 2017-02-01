#include <config.h>
#include <stdio.h>
#include <gnome.h>
#include <glade/glade.h>

#include <gdk/gdkx.h>

#include "view-color-generic.h"
#include "view-color-list.h"
#include "mdi-color-generic.h"
#include "menus.h"
#include "utils.h"
#include "idle.h"
#include "gcolorsel.h"

static ViewColorGenericClass *parent_class = NULL;

/* From nautilus */
static char * down_xpm[] = {
"6 5 2 1",
" 	c None",
".	c #000000",
"......",
"      ",
" .... ",
"      ",
"  ..  "};

/* From nautilus */
static char * up_xpm[] = {
"6 5 2 1",
" 	c None",
".	c #000000",
"  ..  ",
"      ",
" .... ",
"      ",
"......"};

#define UP_ARROW_KEY    "up_arrow"
#define DOWN_ARROW_KEY  "down_arrow"

#define COLUMN_PIXMAP 0
#define COLUMN_VALUE  1
#define COLUMN_NAME   2

static const GtkTargetEntry drag_targets[] = {
  { "application/x-color", 0 }
};

static void view_color_list_class_init     (ViewColorListClass *class);
static void view_color_list_init           (ViewColorList *vcl);

static GtkWidget *create_column_widget     (GtkCList *clist,
					    int num, char *text);

static char *view_color_list_render_value  (ViewColorList *vcl, 
					    MDIColor *col);
static void  view_color_list_render_pixmap (ViewColorList *cl, 
					    GdkPixmap *pixmap,
					    MDIColor *col);
static gint  view_color_list_append        (ViewColorList *vcl, 
					    MDIColor *col);

static void view_color_list_destroy (GtkObject *object);
void
view_color_list_set_sort_column (ViewColorList *vcl, 
				 gint column, GtkSortType type);
static int
view_color_list_compare_rows    (GtkCList *clist, 
				 gconstpointer ptr1, gconstpointer ptr2);

static void
view_color_list_data_changed    (ViewColorGeneric *vcg, gpointer data);

static GList *view_color_list_get_selected (ViewColorGeneric *vcg);
static int    view_color_list_get_insert_pos  (ViewColorGeneric *vcg);
static gpointer 
view_color_list_get_control     (ViewColorGeneric *vcg, GtkVBox *box,
				 void (*changed_cb)(gpointer data), 
				 gpointer change_data);
static void     view_color_list_apply (ViewColorGeneric *vcg,
				       gpointer data);
static void     view_color_list_close (ViewColorGeneric *vcg,
				       gpointer data);
static void     view_color_list_sync  (ViewColorGeneric *vcg,
				       gpointer data);

static void     view_color_list_save  (ViewColorGeneric *vcg);
static void     view_color_list_load  (ViewColorGeneric *vcg);

static void 
view_color_list_click_column (GtkCList *clist, gint column,ViewColorList *vcl);
static gint 
view_color_list_button_press (GtkCList *clist, GdkEventButton *event,
			      ViewColorList *vcl);
static void 
view_color_list_drag_begin   (GtkCList *clist, GdkDragContext *context);
static void 
view_color_list_drag_data_get(GtkCList *clist, GdkDragContext *context, 
			      GtkSelectionData *selection_data, guint info,
			      guint time);

GtkType 
view_color_list_get_type (void)
{
  static guint cg_type = 0;

  if (!cg_type) {
    GtkTypeInfo cg_info = {
      "ViewColorList",
      sizeof (ViewColorList),
      sizeof (ViewColorListClass),
      (GtkClassInitFunc) view_color_list_class_init,
      (GtkObjectInitFunc) view_color_list_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    cg_type = gtk_type_unique (view_color_generic_get_type (), &cg_info);
  }

  return cg_type;
}

static void
view_color_list_class_init (ViewColorListClass *class)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;
  ViewColorGenericClass *vcg_class;

  object_class = GTK_OBJECT_CLASS (class);
  parent_class = gtk_type_class (TYPE_VIEW_COLOR_GENERIC);
  widget_class = (GtkWidgetClass *)class;
  vcg_class    = (ViewColorGenericClass *)class;

  vcg_class->data_changed    = view_color_list_data_changed;
  vcg_class->get_selected    = view_color_list_get_selected;
  vcg_class->get_insert_pos  = view_color_list_get_insert_pos;
  vcg_class->get_control     = view_color_list_get_control;
  vcg_class->close           = view_color_list_close;
  vcg_class->apply           = view_color_list_apply;
  vcg_class->sync            = view_color_list_sync;
  vcg_class->save            = view_color_list_save;
  vcg_class->load            = view_color_list_load;
  
  object_class->destroy = view_color_list_destroy;
}

static void
view_color_list_init (ViewColorList *vcg)
{
  vcg->col_width = 48;
  vcg->col_height = 15;
  vcg->draw_numbers = TRUE;

  gdk_color_parse ("black", &vcg->color_black);
  gdk_color_alloc (gtk_widget_get_default_colormap (), &vcg->color_black);

  gdk_color_parse ("white", &vcg->color_white);
  gdk_color_alloc (gtk_widget_get_default_colormap (), &vcg->color_white);

  vcg->pixmap_font = gdk_font_load ("-bitstream-courier-medium-r-normal-*-12-*-*-*-*-*-*-*");

  vcg->idle = 0;
  vcg->idle_todo = NULL;
}

GtkObject *
view_color_list_new (MDIColorGeneric *mcg)
{
  GtkObject *object;
  GtkCList *cl;
  GtkWidget *col_pix;

  object = gtk_type_new (TYPE_VIEW_COLOR_LIST);
  
  VIEW_COLOR_GENERIC (object)->mcg = mcg;

  cl = GTK_CLIST (gtk_clist_new (3));
  VIEW_COLOR_GENERIC (object)->widget = GTK_WIDGET (cl);

  gtk_signal_connect (GTK_OBJECT (cl), "drag_begin",
		      GTK_SIGNAL_FUNC (view_color_list_drag_begin), object);
  gtk_signal_connect (GTK_OBJECT (cl), "drag_data_get",
		      GTK_SIGNAL_FUNC (view_color_list_drag_data_get), object);
  
  gtk_drag_source_set (GTK_WIDGET (cl), GDK_BUTTON1_MASK, 
		       drag_targets, 1, GDK_ACTION_COPY);
  
  gtk_clist_set_selection_mode (cl, GTK_SELECTION_EXTENDED);
  gtk_clist_column_titles_show (cl);

  col_pix = create_column_widget (cl, COLUMN_PIXMAP, _("Color"));
  create_column_widget (cl, COLUMN_VALUE, _("Value"));
  create_column_widget (cl, COLUMN_NAME, _("Name"));

  gtk_clist_set_column_width (cl, COLUMN_PIXMAP, 
			      VIEW_COLOR_LIST (object)->col_width);
  gtk_clist_set_row_height (cl, VIEW_COLOR_LIST (object)->col_height);

  gtk_clist_set_column_width (cl, COLUMN_VALUE, 70);
  gtk_clist_set_column_resizeable (cl, COLUMN_PIXMAP, FALSE);

  gtk_clist_set_compare_func (cl, view_color_list_compare_rows);
  view_color_list_set_sort_column (VIEW_COLOR_LIST (object), COLUMN_PIXMAP,
				   GTK_SORT_ASCENDING);

  gtk_clist_set_auto_sort (cl, TRUE);

  gtk_clist_column_titles_active (cl);
  gtk_clist_set_use_drag_icons (cl, 0);
  gtk_signal_connect (GTK_OBJECT (cl), "click_column", 
		      GTK_SIGNAL_FUNC (view_color_list_click_column), object);

  gtk_signal_connect (GTK_OBJECT (cl), "button_press_event", 
		      GTK_SIGNAL_FUNC (view_color_list_button_press), object);

  /*  gtk_clist_set_column_min_width(cl, COLUMN_PIXMAP,
      col_pix->requisition.width + 10);*/
		      
  return object;
}

static void
view_color_list_destroy (GtkObject *object)
{
  ViewColorList *vcl = VIEW_COLOR_LIST (object);

  if (vcl->idle_todo)
    g_list_free (vcl->idle_todo);

  if (vcl->idle)
    idle_remove (vcl->idle);

  if (vcl->gc)
    gdk_gc_unref (vcl->gc);
  if (vcl->pixmap_font)
    gdk_font_unref (vcl->pixmap_font);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static GtkWidget *
create_column_widget (GtkCList *cl, int num, char *text)
{
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *up;
  GtkWidget *down;

  hbox = gtk_hbox_new (FALSE, 0);
  
  label = gtk_label_new (text);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, GNOME_PAD_SMALL);

  gtk_widget_show_all (hbox);

  up = gnome_pixmap_new_from_xpm_d (up_xpm);
  down = gnome_pixmap_new_from_xpm_d (down_xpm);

  gtk_box_pack_end (GTK_BOX (hbox), up, FALSE, FALSE, GNOME_PAD_SMALL); 
  gtk_box_pack_end (GTK_BOX (hbox), down, FALSE, FALSE, GNOME_PAD_SMALL); 

  gtk_object_set_data (GTK_OBJECT (hbox), UP_ARROW_KEY, up);
  gtk_object_set_data (GTK_OBJECT (hbox), DOWN_ARROW_KEY, down);

  gtk_clist_set_column_widget (cl, num, hbox);

  return hbox;
}

static int
view_color_list_compare_rows (GtkCList *clist, 
			 gconstpointer ptr1, gconstpointer ptr2)
{
  GtkCListRow *row1;
  GtkCListRow *row2;
  MDIColor *c1;
  MDIColor *c2;
  int t1;
  int t2;
  
  row1 = (GtkCListRow *) ptr1;
  row2 = (GtkCListRow *) ptr2;

  if (! (c1 = (MDIColor *) row1->data)) 
    c1 = gtk_object_get_data (GTK_OBJECT (clist), "insert_data");

  if (! (c2 = (MDIColor *) row2->data))
    c2 = gtk_object_get_data (GTK_OBJECT (clist), "insert_data");
    
  g_assert (row1 != NULL);
  g_assert (row2 != NULL);
  
/*  if (!c2) return (c1 != NULL);
  if (!c1) return 0;*/

  switch (clist->sort_column) {
  case COLUMN_PIXMAP:
    if (c1->pos < c2->pos) return -1;
    if (c1->pos > c2->pos) return 1;
    return 0;

  case COLUMN_NAME:
    return g_strcasecmp (c1->name, c2->name);

  case COLUMN_VALUE:
    t1 = c1->r + c1->g + c1->b;
    t2 = c2->r + c2->g + c2->b;

    if (t1 < t2) return -1;
    if (t1 > t2) return 1;

    break;
  default:
    g_assert_not_reached ();
  }

  return 0;
}

static void
color_list_show_arrow (ViewColorList *vcl)
{
  GtkWidget *hbox;
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);
  
  hbox = gtk_clist_get_column_widget (clist, 
				      GTK_CLIST (clist)->sort_column);

  gtk_widget_show (gtk_object_get_data (GTK_OBJECT (hbox), 
                   GTK_CLIST (clist)->sort_type == GTK_SORT_ASCENDING ? 
   		   UP_ARROW_KEY : DOWN_ARROW_KEY));
}

static void
color_list_hide_arrow (ViewColorList *vcl)
{
  GtkWidget *hbox;
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);
    
  hbox = gtk_clist_get_column_widget (GTK_CLIST (clist), 
				      GTK_CLIST (clist)->sort_column);

  gtk_widget_hide (gtk_object_get_data (GTK_OBJECT (hbox), 
                   GTK_CLIST (clist)->sort_type == GTK_SORT_ASCENDING ? 
   		   UP_ARROW_KEY : DOWN_ARROW_KEY));
}

void
view_color_list_set_sort_column (ViewColorList *vcl, 
				 gint column, GtkSortType type)
{
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);

  color_list_hide_arrow (vcl);

  gtk_clist_set_sort_column (clist, column);
  gtk_clist_set_sort_type (clist, type);

  color_list_show_arrow (vcl);

  gtk_clist_sort (clist);
}

static void
view_color_list_click_column (GtkCList *clist, gint column, ViewColorList *vcl)
{
  GtkSortType type;

  if (column == clist->sort_column) 
    type = clist->sort_type == GTK_SORT_ASCENDING ? 
                                   GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
  else
    type = GTK_SORT_ASCENDING;

  view_color_list_set_sort_column (vcl, column, type);
}

static gint
view_color_list_button_press (GtkCList *clist, GdkEventButton *event,
			      ViewColorList *vcl)
{
  int col, row;
  GtkCListRow *r;
  
  if (event->type == GDK_BUTTON_PRESS) {

    if (event->button == 3) {
      if (gtk_clist_get_selection_info (clist, event->x, event->y, &row, &col)){
	
	/* 1. Si la ligne est deja selectionnee, on continue 
	   2. Sinon, on deselectionne tout et on selectionne la ligne */
	
	r = g_list_nth (clist->row_list, row)->data;
	if (r->state != GTK_STATE_SELECTED) {
	  gtk_clist_freeze (clist);
	  gtk_clist_unselect_all (clist);
	  clist->focus_row = row;
	  gtk_clist_select_row (clist, row, col);
	  gtk_clist_thaw (clist);
	}      
	
	menu_view_do_popup (event, VIEW_COLOR_GENERIC (vcl));
	return FALSE;
      }
    }
    
  } else
      
      if (event->type == GDK_2BUTTON_PRESS) {
	if(gtk_clist_get_selection_info(clist, event->x, event->y, &row, &col))
	  actions_views (gtk_clist_get_row_data (clist, row));
      }

  return TRUE;
}

static void
view_color_list_drag_begin (GtkCList *clist, GdkDragContext *context)
{
  MDIColor *col;
  int row;

  if (clist->focus_row < 0) 
    row = 0;
  else 
    row = clist->focus_row;

  gtk_clist_select_row (clist, row, 0);
  col = gtk_clist_get_row_data (clist, row);  
  if (!col) return;
  gtk_drag_set_icon_widget (context, 
	       drag_window_create (col->r, col->g, col->b), -2, -2);
}

static void 
view_color_list_drag_data_get (GtkCList *clist, GdkDragContext *context, 
			  GtkSelectionData *selection_data, guint info,
			  guint time)
{
  MDIColor *col;
  guint16 vals[4];

  col = gtk_clist_get_row_data (clist, clist->focus_row);
  if (!col) return;
  
  vals[0] = ((gdouble)col->r / 255.0) * 0xffff;
  vals[1] = ((gdouble)col->g / 255.0) * 0xffff;
  vals[2] = ((gdouble)col->b / 255.0) * 0xffff;
  vals[3] = 0xffff;
  
  gtk_selection_data_set (selection_data, 
			  gdk_atom_intern ("application/x-color", FALSE),
			  16, (guchar *)vals, 8);
}	         

static char *
view_color_list_render_value (ViewColorList *vcl, MDIColor *col)
{
  switch (VIEW_COLOR_GENERIC (vcl)->format) {
  case FORMAT_DEC_8:
    return g_strdup_printf ("%d %d %d", col->r, col->g, col->b);
  case FORMAT_DEC_16:
    return g_strdup_printf ("%d %d %d", 
			    col->r * 256, col->g * 256, col->b * 256);
  case FORMAT_HEX_8:
    return g_strdup_printf ("#%02x%02x%02x", col->r, col->g, col->b);
  case FORMAT_HEX_16:
    return g_strdup_printf ("#%04x%04x%04x", 
			    col->r * 256, col->g * 256, col->b * 256);
  case FORMAT_FLOAT:
    return g_strdup_printf ("%1.4g %1.4g %1.4g", (float) col->r / 255.0,
                    			         (float) col->g / 255.0,
                        			 (float) col->b / 255.0);
  default:
    g_assert_not_reached ();
  }

  return NULL;   
}

static void
row_destroy_notify (gpointer data)
{
  gtk_object_unref (GTK_OBJECT (data));
}

static gint
view_color_list_idle (gpointer data)
{
  ViewColorList *vcl = VIEW_COLOR_LIST (data);
  MDIColor *col;
  GdkPixmap *pixmap;  
  int row;
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);
  int i;

  for (i = 0; i < 10; i++) {

    if (! vcl->idle_todo) {
      vcl->idle = 0;
      return FALSE;
    }
    
    col = vcl->idle_todo->data;
    vcl->idle_todo = g_list_remove (vcl->idle_todo, col);
    
    row = gtk_clist_find_row_from_data (clist, col);
    
    if (VIEW_COLOR_GENERIC (vcl)->widget->window)
      pixmap = gdk_pixmap_new (VIEW_COLOR_GENERIC (vcl)->widget->window, 
			       vcl->col_width,
			       vcl->col_height, 
			       -1);
    else
      pixmap = gdk_pixmap_new (GDK_ROOT_PARENT (),
			       vcl->col_width,
			       vcl->col_height,
			       -1);
    
    view_color_list_render_pixmap (vcl, pixmap, col);
    
    gtk_clist_set_pixmap (clist, row, 
			  COLUMN_PIXMAP, pixmap, NULL);
    
    gdk_pixmap_unref (pixmap);
  }
  
  return TRUE;
}

static void
view_color_list_idle_append (ViewColorList *vcl, MDIColor *col)
{
  vcl->idle_todo = g_list_prepend (vcl->idle_todo, col);
  
  if (! vcl->idle) 
    vcl->idle = idle_add (view_color_list_idle, vcl); 
}

static gint
view_color_list_append (ViewColorList *vcl, MDIColor *col)
{
  gchar *string[3];
  gint row;
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);

  string[COLUMN_PIXMAP] = NULL;
  string[COLUMN_VALUE]  = view_color_list_render_value (vcl, col);
  string[COLUMN_NAME]   = col->name;

  gtk_object_set_data (GTK_OBJECT (clist), "insert_data", col);  

  row = gtk_clist_append (clist, string);

  if (string[COLUMN_VALUE]) 
    g_free (string[COLUMN_VALUE]);

  gtk_clist_set_row_data_full (clist, row, col, row_destroy_notify);

  view_color_list_idle_append (vcl, col);
    
  return row;
}

static void
view_color_list_render_pixmap (ViewColorList *vcl, GdkPixmap *pixmap,
			       MDIColor *col)
{
  GdkColor color;
  int h, w;
  char *str;

  color.red   = col->r * 255; 
  color.green = col->g * 255;
  color.blue  = col->b * 255;
  
  gdk_color_alloc (gtk_widget_get_default_colormap (), &color);

  if (vcl->gc == NULL) {
    if (VIEW_COLOR_GENERIC (vcl)->widget->window)
      vcl->gc = gdk_gc_new (VIEW_COLOR_GENERIC (vcl)->widget->window);
    else
      vcl->gc = gdk_gc_new (GDK_ROOT_PARENT ());
  }

  /* Border */
  gdk_gc_set_foreground (vcl->gc, &vcl->color_black);
  gdk_draw_rectangle (pixmap, vcl->gc, FALSE, 0, 0, 
		      vcl->col_width - 1,
		      vcl->col_height - 1);
  /* Color */
  gdk_gc_set_foreground (vcl->gc, &color);
  gdk_draw_rectangle (pixmap, vcl->gc, TRUE, 1, 1, 
		      vcl->col_width - 2,
		      vcl->col_height - 2);

  if (vcl->draw_numbers) {
    
    str = g_strdup_printf ("%d", col->pos);
    
    if ((col->r + col->g + col->b) < ((255 * 3) / 2)) 
      gdk_gc_set_foreground (vcl->gc, &vcl->color_white);
    else
      gdk_gc_set_foreground (vcl->gc, &vcl->color_black);
    
    h = gdk_string_height (vcl->pixmap_font, str);  
    w = gdk_string_width  (vcl->pixmap_font, str);
    
    gdk_draw_string (pixmap, vcl->pixmap_font, vcl->gc, 
		     (vcl->col_width - w) / 2,
		     ((vcl->col_height - h) / 2) + h, str);
    
    g_free (str);
  }
}

static void
view_color_list_redraw_all (ViewColorList *vcl)
{  
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);
  MDIColor *col;
  int i;
  char *str;

  for (i=0; i<clist->rows; i++) {
    col = gtk_clist_get_row_data (clist, i);

    view_color_list_idle_append (vcl, col);

    str = view_color_list_render_value (vcl, col);
    gtk_clist_set_text (clist, i, COLUMN_VALUE, str);
    g_free (str);
  }

  gtk_clist_set_row_height (clist, vcl->col_height);
  gtk_clist_set_column_width (clist, COLUMN_PIXMAP, vcl->col_width);
}

/********************************* MDI **************************************/

static void
view_color_list_data_changed (ViewColorGeneric *vcg, gpointer data)
{
  ViewColorList *vcl = VIEW_COLOR_LIST (vcg);
  GList *list = data;
  MDIColor *col;
  gint row;
  GtkCList *clist = GTK_CLIST (vcg->widget);

  gtk_clist_freeze (clist);

  while (list) {
    col = list->data;

    if (col->change & CHANGE_CLEAR) {
      if (vcl->idle_todo) {
	g_list_free (vcl->idle_todo);
	vcl->idle_todo = NULL;	
      }      
    }

    else

      if (col->change & CHANGE_APPEND) {
	gtk_object_ref (GTK_OBJECT (col));
	view_color_list_append (vcl, col);    
      } else {
	row = gtk_clist_find_row_from_data (clist, col);

	if (col->change & CHANGE_REMOVE) {
	  gtk_clist_remove (clist, row); 
	  vcl->idle_todo = g_list_remove (vcl->idle_todo, col);
	}

	else {
	  
	  /* Redraw pixmap if CHANGE_RGB   OR  (CHANGE_POS and DRAW_NUMBERS) */
	  if ((col->change & CHANGE_POS)||(col->change & CHANGE_RGB)) {
	    if ((vcl->draw_numbers)||(col->change & CHANGE_RGB)) {
	      vcl->idle_todo = g_list_remove (vcl->idle_todo, col);
	      view_color_list_idle_append (vcl, col);

	      if (col->change & CHANGE_RGB) {
		char *str = view_color_list_render_value (vcl, col);
		gtk_clist_set_text (clist, row, COLUMN_VALUE, str);
		g_free (str);	    		
	      }
	    }
	  }
	  
	  if (col->change & CHANGE_NAME) 
	    gtk_clist_set_text (clist, row, COLUMN_NAME, col->name);
	}
      }
    
    list = g_list_next (list);
  }

  gtk_clist_thaw (clist);
}

static gint
view_color_list_get_insert_pos (ViewColorGeneric *vcg)
{
  GtkCList *clist = GTK_CLIST (vcg->widget);

  return clist->focus_row;
}

static GList *
view_color_list_get_selected (ViewColorGeneric *vcg)
{
  GtkCList *clist = GTK_CLIST (vcg->widget);
  GList *list = clist->selection;
  MDIColor *col;
  GList *get_list = NULL;

  while (list) {
    col = gtk_clist_get_row_data (clist, GPOINTER_TO_INT (list->data));   
    
    get_list = g_list_prepend (get_list, col);

    list = g_list_next (list);
  }   

  return get_list;
}

/*************************** PROPERTIES **********************************/

typedef struct prop_t {
  GladeXML *gui;

  gpointer parent_data;

  void (*changed_cb)(gpointer data);
  gpointer change_data;

  GtkWidget *spin_width;
  GtkWidget *spin_height;
  GtkWidget *check_draw_numbers;
} prop_t;

static void
spin_changed_cb (GtkWidget *widget, prop_t *prop)
{
  prop->changed_cb (prop->change_data);
}

static void
check_toggled_cb (GtkWidget *widget, prop_t *prop)
{
  prop->changed_cb (prop->change_data);
}

static gpointer 
view_color_list_get_control (ViewColorGeneric *vcg, GtkVBox *box,
			     void (*changed_cb)(gpointer data), 
			     gpointer change_data)
{
  GtkWidget *frame;
  prop_t *prop = g_new0 (prop_t, 1);
  GtkAdjustment *adj;

  prop->changed_cb  = changed_cb;
  prop->change_data = change_data;

  prop->parent_data = parent_class->get_control (vcg, box, 
						 changed_cb, change_data);

  prop->gui = glade_xml_new (GCOLORSEL_GLADEDIR "view-color-list-properties.glade", "frame");
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

  prop->check_draw_numbers = glade_xml_get_widget (prop->gui, 
						   "check-draw-numbers");
  gtk_signal_connect (GTK_OBJECT (prop->check_draw_numbers), "toggled",
		      GTK_SIGNAL_FUNC (check_toggled_cb), prop);

  return prop;
}

static void     
view_color_list_apply (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;
  GtkCList *clist = GTK_CLIST (vcg->widget);

  VIEW_COLOR_LIST (vcg)->col_width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (prop->spin_width));
  
  VIEW_COLOR_LIST (vcg)->col_height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (prop->spin_height));

  VIEW_COLOR_LIST (vcg)->draw_numbers = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prop->check_draw_numbers));

  parent_class->apply (vcg, prop->parent_data);

  gtk_clist_freeze (clist);
  view_color_list_redraw_all (VIEW_COLOR_LIST (vcg));
  gtk_clist_thaw (clist);
}

static void 
view_color_list_close (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;

  parent_class->close (vcg, prop->parent_data);

  gtk_object_unref (GTK_OBJECT (prop->gui));
  g_free (prop);
}

static void 
view_color_list_sync (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;
  ViewColorList *vcl = VIEW_COLOR_LIST (vcg);

  spin_set_value (GTK_SPIN_BUTTON (prop->spin_width), vcl->col_width, prop);
  spin_set_value (GTK_SPIN_BUTTON (prop->spin_height), vcl->col_height, prop);

  /* check-draw-numbers */
  gtk_signal_handler_block_by_data (GTK_OBJECT (prop->check_draw_numbers), prop);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prop->check_draw_numbers),
				vcl->draw_numbers);
  gtk_signal_handler_unblock_by_data (GTK_OBJECT (prop->check_draw_numbers), prop);

  parent_class->sync (vcg, prop->parent_data);
}

/**************************** Config **********************************/

static void
view_color_list_save (ViewColorGeneric *vcg)
{
  ViewColorList *vcl = VIEW_COLOR_LIST (vcg);
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);

  gnome_config_set_int ("ColWidth", vcl->col_width);
  gnome_config_set_int ("ColHeight", vcl->col_height);
  gnome_config_set_bool ("DrawNumbers", vcl->draw_numbers);

  gnome_config_set_int ("SortColumn", clist->sort_column);
  gnome_config_set_bool ("SortAscending", 
			 clist->sort_type == GTK_SORT_ASCENDING);

  gnome_config_set_int ("ColumnValueWidth", 
			clist->column[COLUMN_VALUE].width);

  parent_class->save (vcg);
}

static void
view_color_list_load (ViewColorGeneric *vcg)
{
  ViewColorList *vcl = VIEW_COLOR_LIST (vcg);
  GtkCList *clist = GTK_CLIST (VIEW_COLOR_GENERIC (vcl)->widget);

  vcl->col_width    = gnome_config_get_int ("ColWidth=48");
  vcl->col_height   = gnome_config_get_int ("ColHeight=15");
  vcl->draw_numbers = gnome_config_get_bool ("DrawNumbers");

  view_color_list_set_sort_column (vcl, 
				   gnome_config_get_int ("SortColumn"),
				   gnome_config_get_bool ("SortAscending") ? 
				   GTK_SORT_ASCENDING : GTK_SORT_DESCENDING);
  

  gtk_clist_set_row_height (clist, vcl->col_height);
  gtk_clist_set_column_width (clist, COLUMN_PIXMAP, vcl->col_width);

  gtk_clist_set_column_width (clist, COLUMN_VALUE, 
			      gnome_config_get_int ("ColumnValueWidth=50"));

  parent_class->load (vcg);
}
