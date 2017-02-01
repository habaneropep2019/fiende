#include <config.h>
#include "widget-control-virtual-rgb.h"
#include "mdi-color-virtual-rgb.h"
#include "utils.h"
#include "gcolorsel.h"

#include <gnome.h>

static void control_virtual_rgb_class_init (ControlVirtualRGBClass *class);
static void control_virtual_rgb_init       (ControlVirtualRGB *cl);

static void preview_size_allocate_cb   (GtkWidget *widget, 
					GtkAllocation *allocation,
					GtkWidget *cs);
static void preview_button_press_cb    (GtkWidget *widget, GdkEventButton *event,
					GtkWidget *cs);
static void preview_drag_begin_cb      (GtkWidget *widget, 
					GdkDragContext *context, 
					GtkWidget *cs);
static void preview_drag_data_get_cb   (GtkWidget *widget, 
					GdkDragContext *context, 
					GtkSelectionData *selection_data, 
					guint info,
					guint time, GtkWidget *cs);

static GtkWidget *range_create             (gchar *title, 
					    gfloat max, gfloat val,
					    GtkWidget **prange, 
					    GtkSignalFunc cb, gpointer data);

static void range_value_changed_cb     (gpointer data);

static void control_virtual_rgb_update_preview (ControlVirtualRGB *cs);

void control_virtual_rgb_sync                  (ControlGeneric *cg);

void control_virtual_rgb_set_rgbt              (ControlVirtualRGB *cv, 
					    float r, float g, 
					    float b, float t);

void control_virtual_rgb_update_range          (GtkRange *range, gfloat val);

static ControlGenericClass *parent_class = NULL;

static const GtkTargetEntry preview_drag_targets[] = {
  { "application/x-color", 0 }
};

GtkType 
control_virtual_rgb_get_type (void)
{
  static guint cv_type = 0;

  if (!cv_type) {
    GtkTypeInfo cv_info = {
      "ControlVirtualRGB",
      sizeof (ControlVirtualRGB),
      sizeof (ControlVirtualRGBClass),
      (GtkClassInitFunc) control_virtual_rgb_class_init,
      (GtkObjectInitFunc) control_virtual_rgb_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    cv_type = gtk_type_unique (control_generic_get_type (), &cv_info);
  }

  return cv_type;
}

static void
control_virtual_rgb_class_init (ControlVirtualRGBClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (TYPE_CONTROL_GENERIC);
  object_class = (GtkObjectClass *) class; 
}

static void
control_virtual_rgb_init (ControlVirtualRGB *cs)
{
  GtkWidget *vbox;
  GtkWidget *frame;

  cs->r = 0;
  cs->g = 0;
  cs->b = 0;
  cs->t = 0;
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (cs), vbox, FALSE, FALSE, 2);

  /* Create entries */
  
  gtk_box_pack_start_defaults (GTK_BOX(vbox),
	      range_create (_("Red :"), 256.0, 0.0, 
			    &cs->range_red, range_value_changed_cb, cs));

  gtk_box_pack_start_defaults (GTK_BOX(vbox),
	      range_create (_("Green :"), 256.0, 0.0, 
			    &cs->range_green, range_value_changed_cb, cs));

  gtk_box_pack_start_defaults (GTK_BOX(vbox),
              range_create (_("Blue :"), 256.0, 0.0, 
			    &cs->range_blue, range_value_changed_cb, cs));

  gtk_box_pack_start_defaults (GTK_BOX(vbox),
              range_create (_("Tolerance :"), 255 * 3 + 2.0, 0.0, 
			    &cs->range_tolerance, range_value_changed_cb, cs));
 
  /* Create preview */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX(cs), frame, TRUE, TRUE, 2);

  cs->preview = gtk_preview_new (GTK_PREVIEW_COLOR);    
  gtk_preview_set_expand (GTK_PREVIEW (cs->preview), TRUE);  
  gtk_container_add (GTK_CONTAINER (frame), cs->preview);

  gtk_widget_set_usize (frame, 100, -1);

  gtk_drag_source_set (cs->preview, GDK_BUTTON1_MASK,
		       preview_drag_targets, 1, 
		       GDK_ACTION_COPY);

  gtk_signal_connect (GTK_OBJECT (cs->preview), "size_allocate",
		      GTK_SIGNAL_FUNC (preview_size_allocate_cb), cs);

  gtk_signal_connect (GTK_OBJECT (cs->preview), "button_press_event",
		      GTK_SIGNAL_FUNC (preview_button_press_cb), cs);

  gtk_signal_connect (GTK_OBJECT (cs->preview), "drag_data_get",
		      GTK_SIGNAL_FUNC (preview_drag_data_get_cb), cs);
  
  gtk_signal_connect (GTK_OBJECT (cs->preview), "drag_begin",
		      GTK_SIGNAL_FUNC (preview_drag_begin_cb), cs);

  CONTROL_GENERIC (cs)->sync = control_virtual_rgb_sync;
}

static GtkWidget *
range_create (gchar *title, gfloat max, gfloat val,
	      GtkWidget **prange, GtkSignalFunc cb, gpointer data)
{
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *range;
  GtkObject *adj;

  adj = gtk_adjustment_new (val, 0.0, max, 1.0, 1.0, 1.0);

  if (cb)
    gtk_signal_connect_object (GTK_OBJECT (adj), "value_changed", cb, data);

  hbox = gtk_hbox_new (TRUE, 2);

  label = gtk_label_new (title);
  gtk_box_pack_start_defaults(GTK_BOX(hbox), label);

  range = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_scale_set_value_pos (GTK_SCALE (range), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (range), 0);

  gtk_box_pack_start_defaults(GTK_BOX(hbox), range);

  if (prange) *prange = range;

  return hbox;
}

GtkWidget *
control_virtual_rgb_new (void)
{
  GtkWidget *widget;

  widget = gtk_type_new (TYPE_CONTROL_VIRTUAL_RGB);

  return widget;
}

static void 
range_value_changed_cb (gpointer data)
{
  ControlVirtualRGB *cv = CONTROL_VIRTUAL_RGB (data);
  GtkAdjustment *adj_red = gtk_range_get_adjustment (GTK_RANGE (cv->range_red));
  GtkAdjustment *adj_green = gtk_range_get_adjustment (GTK_RANGE (cv->range_green));
  GtkAdjustment *adj_blue = gtk_range_get_adjustment (GTK_RANGE (cv->range_blue));
  GtkAdjustment *adj_tolerance = gtk_range_get_adjustment (GTK_RANGE (cv->range_tolerance));

  mdi_color_virtual_rgb_set (MDI_COLOR_VIRTUAL_RGB (CONTROL_GENERIC (data)->mcg), 
			     adj_red->value, adj_green->value, adj_blue->value,
			     adj_tolerance->value);
}

static void
preview_size_allocate_cb (GtkWidget *widget, 
			  GtkAllocation *allocation, GtkWidget *cs)
{
  control_virtual_rgb_update_preview (CONTROL_VIRTUAL_RGB (cs));
}

static void 
preview_drag_begin_cb (GtkWidget *widget, 
		       GdkDragContext *context, GtkWidget *cs)
{
  ControlVirtualRGB *cv = CONTROL_VIRTUAL_RGB (cs);
    
  gtk_drag_set_icon_widget (context, 
			    drag_window_create (cv->r, cv->g, cv->b), -2, -2);
}

static void
preview_button_press_cb (GtkWidget *widget, GdkEventButton *event,
			 GtkWidget *cs)
{
  ControlVirtualRGB *cv = CONTROL_VIRTUAL_RGB (cs);
  
  if (event->type == GDK_2BUTTON_PRESS) 
    actions_previews (cv->r, cv->g, cv->b);
}

static void 
preview_drag_data_get_cb (GtkWidget *widget, GdkDragContext *context, 
			  GtkSelectionData *selection_data, guint info,
			  guint time, GtkWidget *cs)
{
  ControlVirtualRGB *cv = CONTROL_VIRTUAL_RGB (cs);
  guint16 vals[4];

  vals[0] = (cv->r / 255.0) * 0xffff;
  vals[1] = (cv->g / 255.0) * 0xffff;
  vals[2] = (cv->b / 255.0) * 0xffff;
  vals[3] = 0xffff;
    
  gtk_selection_data_set (selection_data, 
			  gdk_atom_intern ("application/x-color", FALSE),
			  16, (guchar *)vals, 8);
}	         

void
control_virtual_rgb_update_preview (ControlVirtualRGB *cv)
{
  preview_fill (cv->preview, cv->r, cv->g, cv->b);
}

void
control_virtual_rgb_update_range (GtkRange *range, gfloat val)
{
  GtkAdjustment *adj;

  adj = gtk_range_get_adjustment (range);

  adj->value = val;
  gtk_range_slider_update (range);
  gtk_widget_queue_clear (GTK_WIDGET (range));
}

void
control_virtual_rgb_set_rgbt (ControlVirtualRGB *cv, 
			  float r, float g, float b, float t)
{
  if ((int)r != (int)cv->r) {
    control_virtual_rgb_update_range (GTK_RANGE (cv->range_red), r);
    cv->r = r;
  }
  
  if ((int)g != (int)cv->g) {
    control_virtual_rgb_update_range (GTK_RANGE (cv->range_green), g);
    cv->g = g;
  }

  if ((int)b != (int)cv->b) {
    control_virtual_rgb_update_range (GTK_RANGE (cv->range_blue), b);
    cv->b = b;
  }
  
  if ((int)t != (int)cv->t) {
    control_virtual_rgb_update_range (GTK_RANGE (cv->range_tolerance), t);
    cv->t = t;
  }

  control_virtual_rgb_update_preview (cv);
}

void 
control_virtual_rgb_sync (ControlGeneric *cg)
{
  float r, g, b, t;

  mdi_color_virtual_rgb_get (MDI_COLOR_VIRTUAL_RGB (cg->mcg), &r, &g, &b, &t);  
  control_virtual_rgb_set_rgbt (CONTROL_VIRTUAL_RGB (cg), r, g, b, t);  
}

