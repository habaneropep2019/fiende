#include "mdi-color-virtual-rgb.h"
#include "mdi-color-virtual.h"
#include "mdi-color-generic.h"
#include "widget-control-virtual-rgb.h"
#include "menus.h"
#include "utils.h"

#include <gnome.h>

static void   mdi_color_virtual_rgb_class_init (MDIColorVirtualRGBClass *class);
static void   mdi_color_virtual_rgb_init       (MDIColorVirtualRGB *mcv);


static int    color_diff                             (int r1, int g1, int b1, 
						      int r2, int g2, int b2);
static int    mdi_color_virtual_rgb_get_diff         (MDIColorVirtualRGB *mcv, 
						      MDIColor *col);

GtkType       mdi_color_virtual_rgb_get_control_type (MDIColorGeneric *mcg);

static void   mdi_color_virtual_rgb_save             (MDIColorGeneric *mcg);
static void   mdi_color_virtual_rgb_load             (MDIColorGeneric *mcg);

static GList *mdi_color_virtual_rgb_get_append_pos (MDIColorGeneric *mcg,
						    MDIColor *col);

static MDIColor *mdi_color_virtual_rgb_append_col    (MDIColorVirtual *mcv,
						      MDIColor *col);
static col_changed_t mdi_color_virtual_rgb_col_changed (MDIColorVirtual *mcv,
							MDIColor *col);

static MDIColorVirtualClass *parent_class = NULL;
static MDIColorGenericClass *parent_2_class = NULL;

guint 
mdi_color_virtual_rgb_get_type()
{
  static guint mdi_gen_child_type = 0;
  
  if (!mdi_gen_child_type) {
    GtkTypeInfo mdi_gen_child_info = {
      "MDIColorVirtualRGB",
      sizeof (MDIColorVirtualRGB),
      sizeof (MDIColorVirtualRGBClass),
      (GtkClassInitFunc) mdi_color_virtual_rgb_class_init,
      (GtkObjectInitFunc) mdi_color_virtual_rgb_init,
      (GtkArgSetFunc) NULL,
      (GtkArgGetFunc) NULL,
    };
    
    mdi_gen_child_type = gtk_type_unique (mdi_color_virtual_get_type (),
					  &mdi_gen_child_info);
  }
  
  return mdi_gen_child_type;
}

static void 
mdi_color_virtual_rgb_class_init (MDIColorVirtualRGBClass *class)
{
  MDIColorGenericClass *mcg_class;
  MDIColorVirtualClass *mcv_class;
  
  mcg_class        = (MDIColorGenericClass *)class;
  mcv_class        = (MDIColorVirtualClass *)class;
  parent_class     = gtk_type_class (mdi_color_virtual_get_type());
  parent_2_class   = gtk_type_class (mdi_color_generic_get_type());

  mcg_class->save         = mdi_color_virtual_rgb_save;
  mcg_class->load         = mdi_color_virtual_rgb_load;

  mcv_class->append_col  = mdi_color_virtual_rgb_append_col;
  mcv_class->col_changed = mdi_color_virtual_rgb_col_changed; 
}

static void
mdi_color_virtual_rgb_init (MDIColorVirtualRGB *mcv)
{
  MDIColorGeneric *mcg = MDI_COLOR_GENERIC (mcv);

  mcg->get_control_type = mdi_color_virtual_rgb_get_control_type;
  mcg->get_append_pos  = mdi_color_virtual_rgb_get_append_pos;

  mcv->r = 0;
  mcv->g = 0;
  mcv->b = 0;
  mcv->t = 100;  
}

MDIColorVirtualRGB *
mdi_color_virtual_rgb_new ()
{
  MDIColorVirtualRGB *mcv; 

  mcv = gtk_type_new (mdi_color_virtual_rgb_get_type ()); 

  return mcv;
}

GtkType
mdi_color_virtual_rgb_get_control_type (MDIColorGeneric *mcg)
{
  return control_virtual_rgb_get_type ();
}

static MDIColor *
mdi_color_virtual_rgb_append_col    (MDIColorVirtual *mcv,
				     MDIColor *col)
{
  int diff = mdi_color_virtual_rgb_get_diff (MDI_COLOR_VIRTUAL_RGB (mcv), col);

  if (diff > MDI_COLOR_VIRTUAL_RGB (mcv)->t) return NULL;

  return parent_class->append_col (mcv, col);
}

static col_changed_t 
mdi_color_virtual_rgb_col_changed (MDIColorVirtual *mcv, MDIColor *col)
{
  int diff = mdi_color_virtual_rgb_get_diff (MDI_COLOR_VIRTUAL_RGB (mcv), col);

  if (diff > MDI_COLOR_VIRTUAL_RGB (mcv)->t) 
    return COL_CHANGED_TRASH;

  return COL_CHANGED_DOC;
}

static GList *
mdi_color_virtual_rgb_get_append_pos (MDIColorGeneric *mcg,
				      MDIColor *col)
{
  MDIColorVirtualRGB *mcv = MDI_COLOR_VIRTUAL_RGB (mcg);
  GList *list;
  int diff, mid;
  
  diff = mdi_color_virtual_rgb_get_diff (mcv, col);
  
  mid = (mdi_color_virtual_rgb_get_diff (mcv, mcg->col->data) +
             mdi_color_virtual_rgb_get_diff (mcv, mcg->last_col->data)) / 2;  
  
  if (diff > mid) {
    list = mcg->last_col;
    while (list) {
      if (col != list->data)
        if (diff >= mdi_color_virtual_rgb_get_diff (mcv, list->data)) 
          return list->next;
        
      list = g_list_previous (list);
    }
  }
  
  else
  
  {                  
    list = mcg->col;
    while (list) {
      if (col != list->data) 
        if (diff <= mdi_color_virtual_rgb_get_diff (mcv, list->data)) return list;
      
      list = g_list_next (list);
    }
  }

  return NULL;
}

static int 
color_diff (int r1, int g1, int b1, int r2, int g2, int b2)
{
  return abs (r1 - r2) + abs (g1 - g2) + abs (b1 - b2); 
}           

static int
mdi_color_virtual_rgb_get_diff (MDIColorVirtualRGB *mcv, MDIColor *col)
{
  return color_diff (mcv->r, mcv->g, mcv->b, col->r, col->g, col->b);
}      


void 
mdi_color_virtual_rgb_set (MDIColorVirtualRGB *mcv, 
		       float r, float g, float b, float t)
{
  mdi_color_generic_freeze (MDI_COLOR_GENERIC (mcv)); 
  
  if ((mcv->r == r) && (mcv->g == g) && (mcv->b == b)) {
    if (mcv->t < t) {
      mcv->t = t; 
      mdi_color_virtual_trash_to_doc (MDI_COLOR_VIRTUAL (mcv));
    } else {  
      mcv->t = t; 
      mdi_color_virtual_doc_to_trash (MDI_COLOR_VIRTUAL (mcv));
    }
        
  } else { 
    mcv->r = r;
    mcv->g = g;
    mcv->b = b;

    mdi_color_virtual_repost_all (MDI_COLOR_VIRTUAL (mcv));   
  }

   mdi_color_generic_thaw (MDI_COLOR_GENERIC (mcv)); 

   mdi_color_generic_sync_control (MDI_COLOR_GENERIC (mcv));  
}

void     
mdi_color_virtual_rgb_get (MDIColorVirtualRGB *mcv, 
		       float *r, float *g, float *b, float *t)
{
  if (r) *r = mcv->r;
  if (g) *g = mcv->g;
  if (b) *b = mcv->b;
  if (t) *t = mcv->t;
}

/******************************** Config *********************************/

static void         
mdi_color_virtual_rgb_save (MDIColorGeneric *mcg)
{
  MDIColorVirtualRGB *mcv = MDI_COLOR_VIRTUAL_RGB (mcg);

  gnome_config_set_int ("Red", mcv->r);
  gnome_config_set_int ("Green", mcv->g);
  gnome_config_set_int ("Blue", mcv->b);
  gnome_config_set_int ("Tolerance", mcv->t);
    
  parent_2_class->save (mcg);
}

static void         
mdi_color_virtual_rgb_load (MDIColorGeneric *mcg)
{
  MDIColorVirtualRGB *mcv = MDI_COLOR_VIRTUAL_RGB (mcg);
  
  mcv->r = gnome_config_get_int ("Red");
  mcv->g = gnome_config_get_int ("Green");
  mcv->b = gnome_config_get_int ("Blue");
  mcv->t = gnome_config_get_int ("Tolerance");

  mdi_color_generic_sync_control (mcg); 

  parent_2_class->load (mcg);
}
