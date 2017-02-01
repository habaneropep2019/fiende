#ifndef WIDGET_CONTROL_GENERIC_H
#define WIDGET_CONTROL_GENERIC_H

#include <gnome.h>
  
BEGIN_GNOME_DECLS

#define TYPE_CONTROL_GENERIC            (control_generic_get_type ())
#define CONTROL_GENERIC(obj)            (GTK_CHECK_CAST ((obj), TYPE_CONTROL_GENERIC, ControlGeneric))
#define CONTROL_GENERIC_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TYPE_CONTROL_GENERIC, ControlGenericClass))
#define IS_CONTROL_GENERIC(obj)         (GTK_CHECK_TYPE ((obj), TYPE_CONTROL_GENERIC))
#define IS_CONTROL_GENERIC_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), TYPE_CONTROL_GENERIC))
#define CONTROL_GENERIC_GET_CLASS(obj)  (CONTROL_GENERIC_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _ControlGeneric ControlGeneric;
typedef struct _ControlGenericClass ControlGenericClass;

#include "mdi-color-generic.h"

struct _ControlGeneric {
  GtkHBox hbox;

  MDIColorGeneric *mcg;
 
  void (*sync) (ControlGeneric *cg);
};

struct _ControlGenericClass {
  GtkHBoxClass parent_class; 
};

GtkType    control_generic_get_type (void);
GtkWidget *control_generic_new      (void);

void       control_generic_assign   (ControlGeneric *cg, MDIColorGeneric *mcg);
void       control_generic_sync     (ControlGeneric *cg);

END_GNOME_DECLS

#endif
