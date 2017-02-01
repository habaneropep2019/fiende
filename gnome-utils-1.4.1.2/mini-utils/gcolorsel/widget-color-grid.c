#include <stdio.h>

#include <gnome.h>

#include "widget-color-grid.h"
#include "utils.h"
#include "menus.h"
#include "idle.h"

enum {
  MOVE_ITEM,
  LAST_SIGNAL
};

static guint wcg_signals [LAST_SIGNAL] = { 0 };

static const GtkTargetEntry color_grid_drag_targets[] = {
  { "application/x-color", 0 }
};

static void color_grid_class_init    (ColorGridClass *class);
static void color_grid_init          (ColorGrid *cl);

static void color_grid_allocate      (GtkWidget *widget, 
				      GtkAllocation *allocation);
static void color_grid_destroy       (GtkObject *object);

static void color_grid_item_draw     (ColorGrid *cg, ColorGridCol *col);

static void color_grid_set_scroll_region  (ColorGrid *cg, int nb);
static gint color_grid_button_press_event (GtkWidget *widget, 
					   GdkEventButton *event);
static gint color_grid_key_press_event    (GtkWidget *widget,
					   GdkEventKey *event);

static ColorGridCol *color_grid_get_col_at (ColorGrid *cg, 
					    ColorGridCol *col, 
					    GtkPositionType type);

static void color_grig_for_idle (ColorGrid *cg, ColorGridCol *col);

static void color_grid_reorganize (ColorGrid *cg);
static gint color_grid_item_event (GnomeCanvasItem *item, 
				   GdkEvent *event, gpointer data);

static GnomeCanvasClass *parent_class = NULL;

GtkType 
color_grid_get_type (void)
{
  static guint cg_type = 0;

  if (!cg_type) {
    GtkTypeInfo cg_info = {
      "ColorGrid",
      sizeof (ColorGrid),
      sizeof (ColorGridClass),
      (GtkClassInitFunc) color_grid_class_init,
      (GtkObjectInitFunc) color_grid_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    cg_type = gtk_type_unique (gnome_canvas_get_type (), &cg_info);
  }

  return cg_type;
}

static void
color_grid_class_init (ColorGridClass *class)
{
  GtkWidgetClass *widget_class;
  GnomeCanvasClass *canvas_class;
  GtkObjectClass *object_class;

  object_class = GTK_OBJECT_CLASS (class);
  parent_class = gtk_type_class (GNOME_TYPE_CANVAS);
  widget_class = (GtkWidgetClass *)class;
  canvas_class = (GnomeCanvasClass *)class;

  widget_class->size_allocate = color_grid_allocate;
  widget_class->button_press_event = color_grid_button_press_event;
  widget_class->key_press_event = color_grid_key_press_event;

  wcg_signals [MOVE_ITEM] = 
    gtk_signal_new ("move_item",
		    GTK_RUN_LAST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (ColorGridClass, move_item),
		    gtk_marshal_NONE__POINTER_UINT_UINT,
		    GTK_TYPE_NONE, 3, GTK_TYPE_POINTER, GTK_TYPE_UINT,
		    GTK_TYPE_UINT);

  gtk_object_class_add_signals (object_class, wcg_signals, LAST_SIGNAL);

  object_class->destroy = color_grid_destroy;
}

static void
color_grid_init (ColorGrid *cg)
{
  GnomeCanvas *canvas = GNOME_CANVAS (cg);

  cg->nb_col         = 10;
  cg->col_height     = 15;
  cg->col_width      = 15;
  cg->in_drag        = FALSE;
  cg->button_pressed = FALSE;
  cg->selected       = NULL;
  cg->col            = NULL;
  cg->last_clicked   = NULL;
  cg->last_focus     = NULL;
  cg->count          = 0;
  cg->freeze         = 0;

  cg->idle           = 0;
  cg->idle_todo      = NULL;

  gdk_color_parse ("black", &cg->color_black);
  gdk_color_alloc (gtk_widget_get_default_colormap (), &cg->color_black);

  gdk_color_parse ("white", &cg->color_white);
  gdk_color_alloc (gtk_widget_get_default_colormap (), &cg->color_white);

  gdk_color_parse ("red", &cg->color_red);
  gdk_color_alloc (gtk_widget_get_default_colormap (), &cg->color_red);

  GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_FOCUS);
  GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_DEFAULT);
}

GtkWidget *
color_grid_new (GCompareFunc compare_func)

{
  GtkWidget *widget;

  widget = gtk_type_new (TYPE_COLOR_GRID);

  COLOR_GRID (widget)->compare_func = compare_func;
		       
  return widget;
}

static void
color_grid_set_scroll_region (ColorGrid *cg, int nb)
{
  gnome_canvas_set_scroll_region (GNOME_CANVAS (cg), 0, 0, 
				  cg->nb_col * cg->col_width, 
				  ((nb / (cg->nb_col)) + 1) * cg->col_height);
}

static void
color_grid_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  ColorGrid *cg = COLOR_GRID (widget);
  gboolean change = FALSE;

  if ((allocation->width != widget->allocation.width)
      || (allocation->height != widget->allocation.height))
    change = TRUE;

  if (GTK_WIDGET_CLASS (parent_class)->size_allocate)
    GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

  if (!change)
    return;

  cg->nb_col = allocation->width / cg->col_width;
  if (!cg->nb_col) cg->nb_col = 1; /* Avoid a crash ... */

  color_grid_reorganize (cg);
}

static void 
color_grid_destroy (GtkObject *object)
{
  ColorGrid *cg = COLOR_GRID (object);

  if (cg->idle)
    idle_remove (cg->idle);

  if (cg->idle_todo)
    g_list_free (cg->idle_todo);

  color_grid_clear (cg);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static gint
color_grid_idle (gpointer data)
{
  ColorGrid *cg = COLOR_GRID (data);
  ColorGridCol *col;
  int i;

  for (i=0; i<cg->nb_col; i++) {

    if (!cg->idle_todo) {
      cg->idle = 0;
      return FALSE;
    }

    col = cg->idle_todo->data;
    color_grid_item_draw (cg, col);
    
    cg->idle_todo = g_list_remove (cg->idle_todo, col);
  }

  return TRUE;
}

static void 
color_grig_for_idle (ColorGrid *cg, ColorGridCol *col)
{
  cg->idle_todo = g_list_prepend (cg->idle_todo, col);

  if (!cg->idle) 
    cg->idle = idle_add (color_grid_idle, cg);
}

static void
color_grid_reorganize (ColorGrid *cg)
{
  GList *list;
  int pos = 0;
  ColorGridCol *c;
  int line, column;

  if (cg->freeze) return;

  g_list_free (cg->idle_todo);
  cg->idle_todo = NULL;

  list = cg->col;  
  while (list) {
    c = list->data;    

    line = pos / cg->nb_col;
    column = pos - (line * cg->nb_col);

    c->line = line;
    c->column = column;

    color_grig_for_idle (cg, c);

    pos++;
    list = g_list_next (list);
  }

  color_grid_set_scroll_region (cg, pos);
}

static ColorGridCol *
color_grid_get_col_at (ColorGrid *cg, ColorGridCol *col, GtkPositionType type)
{
  int pos, new_pos = 0;
  GList *list;

  if (col) {

    pos = g_list_index (cg->col, col);

    switch (type) {
    case GTK_POS_LEFT:
      new_pos = pos - 1;
      break;
    case GTK_POS_RIGHT:
      new_pos = pos + 1;
      break;
    case GTK_POS_TOP:
      new_pos = pos - cg->nb_col;
      break;
    case GTK_POS_BOTTOM:
      new_pos = pos + cg->nb_col;
      break;
    }
    
    if (pos < 0) 
      return NULL;
  } 

  if ((list = g_list_nth (cg->col, new_pos)))
    return list->data;

  return NULL;
}

static void
color_grid_item_draw (ColorGrid *cg, ColorGridCol *col)
{
  GdkColor color;
  GdkColor *outline_col;
  gboolean raise = FALSE;
  double width = 1;
  
  color.red   = col->r * 255; 
  color.green = col->g * 255;
  color.blue  = col->b * 255; 
  gdk_color_alloc (gtk_widget_get_default_colormap (), &color);

  if ((cg->drop)&&(cg->drop == col->item)) {
    outline_col = &cg->color_red;
    raise = TRUE;
    width = 2;
  } else
    if (col->selected) {
      outline_col = &cg->color_white;
      raise = TRUE;
      width = 2;
    } else
      outline_col = &cg->color_black;

  if (col->item) {

    gnome_canvas_item_set (col->item,
      "x1", (float) col->column * cg->col_width,
      "y1", (float) col->line   * cg->col_height,
      "x2", (float) col->column * cg->col_width + cg->col_width,
      "y2", (float) col->line   * cg->col_height+ cg->col_height,
      "outline_color_gdk", outline_col,
      "width_units",       width,
      "fill_color_gdk",    &color,
      NULL);			 

  } else {
      
    col->item = gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (cg)),
      GNOME_TYPE_CANVAS_RECT,
      "x1", (float) col->column * cg->col_width,
      "y1", (float) col->line   * cg->col_height,
      "x2", (float) col->column * cg->col_width + cg->col_width,
      "y2", (float) col->line   * cg->col_height+ cg->col_height,
      "outline_color_gdk", outline_col,
      "width_units",       width,
      "fill_color_gdk",    &color,
      NULL);

    gtk_object_set_data (GTK_OBJECT (col->item), "col", col);

    gtk_signal_connect (GTK_OBJECT (col->item), "event",
			GTK_SIGNAL_FUNC (color_grid_item_event), NULL);
    
  }

  if (raise)
    gnome_canvas_item_raise_to_top (col->item);
}

static void
color_grid_select (ColorGridCol *col, gboolean selected)
{
  if (col->selected != selected) {
    col->selected = selected;
    color_grig_for_idle (col->cg, col);

    if (selected) 
      col->cg->selected = g_list_prepend (col->cg->selected, col);
    else
      col->cg->selected = g_list_remove (col->cg->selected, col);
  }
}

static void
color_grid_deselect_all (ColorGrid *cg)
{
  GList *list = cg->selected;
  ColorGridCol *col;

  while (list) {
    col = list->data;

    color_grig_for_idle (cg, col);
    col->selected = FALSE;

    list = g_list_next (list);
  }

  g_list_free (cg->selected);
  cg->selected = NULL;
}

static gint
color_grid_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
  ColorGrid *cg = COLOR_GRID (widget);
  ColorGridCol *col;
  GnomeCanvasItem *item;  
  double cx, cy;

  gtk_widget_grab_focus (widget);

  if (event->button == 3) {
    
    gnome_canvas_window_to_world (GNOME_CANVAS (widget),
				  event->x, event->y, &cx, &cy);
    item = gnome_canvas_get_item_at (GNOME_CANVAS (widget), cx, cy);
    
    if (item) {
      /* 1. Si l'item ligne est deja selectionnee, on continue 
	 2. Sinon, on deselectionne tout et on selectionne l'item */
      
      col = gtk_object_get_data (GTK_OBJECT (item), "col");
      if (! col->selected) {
	color_grid_deselect_all (cg);
	color_grid_select (col, TRUE);
	cg->last_clicked = col;
	cg->last_focus = col;
      }      
    }

    return FALSE;
  }

  return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, event);
}

static gint 
color_grid_key_press_event (GtkWidget *widget, GdkEventKey *event)
{
  ColorGrid *cg = COLOR_GRID (widget);
  ColorGridCol *col = NULL;

  switch (event->keyval) {
  case GDK_Up:
    col = color_grid_get_col_at (cg, cg->last_focus, GTK_POS_TOP);
    break;
  case GDK_Down:
    col = color_grid_get_col_at (cg, cg->last_focus, GTK_POS_BOTTOM);
    break;
  case GDK_Left:
    col = color_grid_get_col_at (cg, cg->last_focus, GTK_POS_LEFT);
    break;
  case GDK_Right:
    col = color_grid_get_col_at (cg, cg->last_focus, GTK_POS_RIGHT);
    break;
  default:
    return FALSE;
  }

  if (col) {
    color_grid_deselect_all (cg);
    color_grid_select (col, TRUE);
    cg->last_clicked = col;
    cg->last_focus = col;
  }

  return TRUE;
}

static void
color_grid_select_from_to (ColorGrid *cg, ColorGridCol *from, ColorGridCol *to)
{
  GList *list = cg->col;
  
  while (list) {
    if ((list->data == from) || (list->data == to)) break;
    list = g_list_next (list);
  }

  if (!list) return;

  if (list->data == to) to = from;

  while (list) {
    color_grid_select (list->data, TRUE);

    if (list->data == to) break;

    list = g_list_next (list);
  }    
}

static gint
color_grid_item_event (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  GnomeCanvasItem *drop;
  ColorGridCol *col, *col_drop;
  ColorGrid *cg;  

  col = gtk_object_get_data (GTK_OBJECT (item), "col");
  cg = col->cg;

  if (cg->in_drag) {
    switch (event->type) {
    case GDK_BUTTON_RELEASE:
      cg->button_pressed = FALSE;
      cg->in_drag = FALSE;
    
      gnome_canvas_item_ungrab (item, event->button.time);

      if (cg->drop) {
	col_drop = gtk_object_get_data (GTK_OBJECT (cg->drop), "col");
	gtk_signal_emit_by_name (GTK_OBJECT (cg), "move_item", 
				 g_list_index (cg->col, col), 
				 g_list_index (cg->col, col_drop));

	color_grig_for_idle (cg, col_drop);
	cg->drop = NULL;
      }
      
      break;
    case GDK_MOTION_NOTIFY:
      drop = gnome_canvas_get_item_at (GNOME_CANVAS (cg), 
				       event->motion.x,
				       event->motion.y);

      if (drop != cg->drop) {
	if (cg->drop)
	  color_grig_for_idle (cg, 
		 gtk_object_get_data (GTK_OBJECT (cg->drop), "col"));

	if ((item != drop)&&(drop)) {
	  color_grig_for_idle (cg, 
			gtk_object_get_data (GTK_OBJECT (drop), "col"));
	  cg->drop = drop;
	} else 
	  cg->drop = NULL;
      }

      break;
    default:
      return FALSE;
    }  

    return TRUE;
  }

  switch (event->type) {
  case GDK_2BUTTON_PRESS:
    break;
  case GDK_MOTION_NOTIFY:

    if (cg->can_move) 
      if (cg->button_pressed) {
	if (!cg->in_drag) {	
	  color_grid_deselect_all (cg);
	  color_grid_select (col, TRUE);
	  cg->last_clicked = col;

	  cg->in_drag = TRUE;
	  cg->drop = NULL;
	  gnome_canvas_item_grab (item, 
			  GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
				  NULL,
				  event->button.time); 
	}
      }
    break;
  case GDK_BUTTON_RELEASE:
    cg->button_pressed = FALSE;
    break;
  case GDK_BUTTON_PRESS:
    cg->button_pressed = TRUE;
    cg->last_focus = col;

    if (event->button.state & GDK_SHIFT_MASK) {
      ColorGridCol *from;

      if (! (event->button.state & GDK_CONTROL_MASK))
	  color_grid_deselect_all (cg);

      if (cg->last_clicked) 
	from = cg->last_clicked;
      else 
	from = cg->col->data;      

      color_grid_select_from_to (cg, from, col);
    }
    
    else
      
      if (event->button.state & GDK_CONTROL_MASK) {
	color_grid_select (col, ! col->selected);
	cg->last_clicked = col;
      }
        
      else {
	color_grid_deselect_all (cg);
	color_grid_select (col, TRUE);
	cg->last_clicked = col;
      }

    break;
  default:
    return FALSE;
  }
    
  return TRUE;
}

void
color_grid_change_rgb (ColorGrid *cg, gpointer data, int r, int g, int b)
{
  ColorGridCol *col = color_grid_find_item_from_data (cg, data)->data;

  col->r = r;
  col->g = g;
  col->b = b;

  color_grig_for_idle (cg, col);
}

int
color_grid_append (ColorGrid *cg, int r, int g, int b, 
		   gpointer data, GtkDestroyNotify destroy)
{
  ColorGridCol *c;
  int pos;

  c = g_new0 (ColorGridCol, 1);
  c->selected = FALSE;
  c->cg       = cg;
  c->data     = data;
  c->destroy  = destroy;
  c->item     = NULL;
  c->r        = r;
  c->g        = g;
  c->b        = b;

  pos = cg->count++;

  cg->col = g_list_insert_sorted (cg->col, c, cg->compare_func);

  color_grig_for_idle (cg, c);
  
  color_grid_reorganize (cg);
  
  return pos;
}

void
color_grid_remove (ColorGrid *cg, gpointer data)
{
  GList *list;
  ColorGridCol *col;

  list = color_grid_find_item_from_data (cg, data);
  col = list->data;

  if (col->destroy) 
    col->destroy (col->data);

  if (list->prev)
    g_list_remove (list, col);
  else
    cg->col = g_list_remove (list, col);

  gtk_object_destroy (GTK_OBJECT (col->item));  

  if (col->selected) 
    cg->selected = g_list_remove (cg->selected, col);
  
  if (col == cg->last_clicked) cg->last_clicked = NULL;
  if (col == cg->last_focus) cg->last_focus = NULL;
  
  g_free (col);

  color_grid_reorganize (cg);
}

void
color_grid_clear (ColorGrid *cg)
{
  ColorGridCol *col;
  GList *list;

  list = cg->col; 
  
  while (list) {
    col = list->data;

    if (col->destroy) 
      col->destroy (col->data);

    if (col->item)
      gtk_object_destroy (GTK_OBJECT (col->item));

    g_free (col);   
    
    list = g_list_next (list);
  }  

  g_list_free (cg->selected);
  g_list_free (cg->col);
  g_list_free (cg->idle_todo);

  cg->col = NULL;
  cg->selected = NULL;
  cg->idle_todo = NULL;
  cg->last_clicked = NULL;
  cg->last_focus = NULL;
}

void
color_grid_freeze (ColorGrid *cg)
{
  cg->freeze++;
}

void
color_grid_thaw (ColorGrid *cg)
{
  if (cg->freeze) {
    cg->freeze--;

    if (!cg->freeze)       
      color_grid_reorganize (cg);           
  }
}

void
color_grid_sort (ColorGrid *cg)
{
  cg->col = g_list_sort (cg->col, cg->compare_func);
  color_grid_reorganize (cg);
}

void 
color_grid_can_move (ColorGrid *cg, gboolean value)
{
  cg->can_move = value;
}

void 
color_grid_set_col_width_height (ColorGrid *cg, int width, int height)
{
  cg->col_width  = width;
  cg->col_height = height;

  cg->nb_col = GTK_WIDGET (cg)->allocation.width / cg->col_width;

  color_grid_reorganize (cg);
}

GList *
color_grid_find_item_from_data (ColorGrid *cg, gpointer data)
{
  GList *list = cg->col;
  ColorGridCol *col;

  while (list) {
    col = list->data;

    if (col->data == data) return list;

    list = g_list_next (list);
  }

  return NULL;
}

void 
color_grid_change_pos (ColorGrid *cg, gpointer data)
{
  GList *list = color_grid_find_item_from_data (cg, data);
  ColorGridCol *col = list->data;

  if ((list->prev) && (list->next)) {
    ColorGridCol *prev = list->prev->data;
    ColorGridCol *next = list->next->data;

    if ((cg->compare_func (prev, col)<0)&&(cg->compare_func (col, next)<0)) {
      color_grid_reorganize (cg);
      return;
    }
  }

  if (list->prev) 
    g_list_remove (list, col);
  else 
    cg->col = g_list_remove (list, col); 

  cg->col = g_list_insert_sorted (cg->col, col, cg->compare_func);

  color_grid_reorganize (cg);
}
