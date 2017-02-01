#include "mdi-color-virtual-monitor.h"
#include "mdi-color-virtual.h"
#include "mdi-color-generic.h"
#include "menus.h"
#include "utils.h"

#include <gnome.h>

static void   mdi_color_virtual_monitor_class_init (MDIColorVirtualMonitorClass *class);
static void   mdi_color_virtual_monitor_init       (MDIColorVirtualMonitor *mcv);

static void   mdi_color_virtual_monitor_save       (MDIColorGeneric *mcg);
static void   mdi_color_virtual_monitor_load       (MDIColorGeneric *mcg);

static MDIColor *mdi_color_virtual_monitor_append_col (MDIColorVirtual *mcv,
						       MDIColor *col);

static MDIColorVirtualClass *parent_class = NULL;
static MDIColorGenericClass *parent_2_class = NULL;

guint 
mdi_color_virtual_monitor_get_type()
{
  static guint mdi_gen_child_type = 0;
  
  if (!mdi_gen_child_type) {
    GtkTypeInfo mdi_gen_child_info = {
      "MDIColorVirtualMonitor",
      sizeof (MDIColorVirtualMonitor),
      sizeof (MDIColorVirtualMonitorClass),
      (GtkClassInitFunc) mdi_color_virtual_monitor_class_init,
      (GtkObjectInitFunc) mdi_color_virtual_monitor_init,
      (GtkArgSetFunc) NULL,
      (GtkArgGetFunc) NULL,
    };
    
    mdi_gen_child_type = gtk_type_unique (mdi_color_virtual_get_type (),
					  &mdi_gen_child_info);
  }
  
  return mdi_gen_child_type;
}

static void 
mdi_color_virtual_monitor_class_init (MDIColorVirtualMonitorClass *class)
{
  MDIColorGenericClass *mcg_class;
  MDIColorVirtualClass *mcv_class;
  
  mcg_class        = (MDIColorGenericClass *)class;
  mcv_class        = (MDIColorVirtualClass *)class;
  parent_class     = gtk_type_class (mdi_color_virtual_get_type());
  parent_2_class   = gtk_type_class (mdi_color_generic_get_type());

  mcg_class->save         = mdi_color_virtual_monitor_save;
  mcg_class->load         = mdi_color_virtual_monitor_load;

  mcv_class->append_col  = mdi_color_virtual_monitor_append_col;
}

static void
mdi_color_virtual_monitor_init (MDIColorVirtualMonitor *mcv)
{
  mcv->monit = FALSE;
}

MDIColorVirtualMonitor *
mdi_color_virtual_monitor_new ()
{
  MDIColorVirtualMonitor *mcv; 

  mcv = gtk_type_new (mdi_color_virtual_monitor_get_type ()); 

  return mcv;
}

static MDIColor *
mdi_color_virtual_monitor_append_col (MDIColorVirtual *mcv,
				      MDIColor *col)
{
  MDIColorVirtualMonitor *m = MDI_COLOR_VIRTUAL_MONITOR (mcv);
  GList *list = m->monit;

  while (list) {
    if (list->data == col) {
      if (list->prev) 
	g_list_remove (list, list->data);
      else
	m->monit = g_list_remove (list, list->data);

      return parent_class->append_col (mcv, col);
    }

    list = g_list_next (list);
  }

  return NULL;
}

/* may be search color in parent ... but works well if parent are attached
 after ... */
void
mdi_color_virtual_monitor_add (MDIColorVirtualMonitor *m, MDIColor *col)
{
  m->monit = g_list_append (m->monit, col);
}

/******************************** Config *********************************/

static void         
mdi_color_virtual_monitor_save (MDIColorGeneric *mcg)
{
  /*  MDIColorVirtualMonitor *mcv = MDI_COLOR_VIRTUAL_Monitor (mcg); */

  parent_2_class->save (mcg);
}

static void         
mdi_color_virtual_monitor_load (MDIColorGeneric *mcg)
{
  /*  MDIColorVirtualMonitor *mcv = MDI_COLOR_VIRTUAL_Monitor (mcg); */
  
  mdi_color_generic_sync_control (mcg); 

  parent_2_class->load (mcg);
}
