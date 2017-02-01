#ifndef WIDGET_CONTROL_VIRTUAL_RGB_H
#define WIDGET_CONTROL_VIRTUAL_RGB_H

#include "widget-control-generic.h"

#include <gnome.h>
  
BEGIN_GNOME_DECLS

#define TYPE_CONTROL_VIRTUAL_RGB            (control_virtual_rgb_get_type ())
#define CONTROL_VIRTUAL_RGB(obj)            (GTK_CHECK_CAST ((obj), TYPE_CONTROL_VIRTUAL_RGB, ControlVirtualRGB))
#define CONTROL_VIRTUAL_RGB_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TYPE_CONTROL_VIRTUAL_RGB, ControlVirtualRGBClass))
#define IS_CONTROL_VIRTUAL_RGB(obj)         (GTK_CHECK_TYPE ((obj), TYPE_CONTROL_VIRTUAL_RGB))
#define IS_CONTROL_VIRTUAL_RGB_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), TYPE_CONTROL_VIRTUAL_RGB))
#define CONTROL_VIRTUAL_RGB_GET_CLASS(obj)  (CONTROL_VIRTUAL_RGB_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _ControlVirtualRGB ControlVirtualRGB;
typedef struct _ControlVirtualRGBClass ControlVirtualRGBClass;

struct _ControlVirtualRGB {
  ControlGeneric cg;

  GtkWidget *preview;

  GtkWidget *range_red;  
  GtkWidget *range_green;
  GtkWidget *range_blue;
  GtkWidget *range_tolerance;

  float r;
  float g;
  float b;
  float t;
};

struct _ControlVirtualRGBClass {
  ControlGenericClass parent_class; 
};

GtkType    control_virtual_rgb_get_type (void);
GtkWidget *control_virtual_rgb_new      (void);

END_GNOME_DECLS

#endif /* _CONTROL_VIRTUAL_RGB_H_ */
