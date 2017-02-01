#ifndef __MDI_COLOR_VIRTUAL_MONITOR_H__
#define __MDI_COLOR_VIRTUAL_MONITOR_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "mdi-color-virtual.h"
#include "mdi-color-generic.h"

BEGIN_GNOME_DECLS

#define MDI_COLOR_VIRTUAL_MONITOR(obj)          GTK_CHECK_CAST (obj, mdi_color_virtual_monitor_get_type (), MDIColorVirtualMonitor)
#define MDI_COLOR_VIRTUAL_MONITOR_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, mdi_color_virtual_monitor_get_type (), MDIColorVirtualMonitorClass)
#define IS_MDI_COLOR_VIRTUAL_MONITOR(obj)       GTK_CHECK_TYPE (obj, mdi_color_virtual_monitor_get_type ())

typedef struct _MDIColorVirtualMonitor       MDIColorVirtualMonitor;
typedef struct _MDIColorVirtualMonitorClass  MDIColorVirtualMonitorClass;

struct _MDIColorVirtualMonitor {
  MDIColorVirtual virtual;
  
  GList *monit;
};

struct _MDIColorVirtualMonitorClass {
  MDIColorVirtualClass parent_class;
};

guint            mdi_color_virtual_monitor_get_type (void);
MDIColorVirtualMonitor *mdi_color_virtual_monitor_new      (void);

void             mdi_color_virtual_monitor_add (MDIColorVirtualMonitor *m,
						MDIColor *col);
END_GNOME_DECLS

#endif
