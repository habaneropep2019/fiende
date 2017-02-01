#include "widget-control-generic.h"
#include "utils.h"

#include <gnome.h>

static void control_generic_class_init (ControlGenericClass *class);
static void control_generic_init       (ControlGeneric *cl);

static GtkHBoxClass *parent_class = NULL;

GtkType 
control_generic_get_type (void)
{
  static guint cv_type = 0;

  if (!cv_type) {
    GtkTypeInfo cv_info = {
      "ControlGeneric",
      sizeof (ControlGeneric),
      sizeof (ControlGenericClass),
      (GtkClassInitFunc) control_generic_class_init,
      (GtkObjectInitFunc) control_generic_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    cv_type = gtk_type_unique (gtk_hbox_get_type (), &cv_info);
  }

  return cv_type;
}

static void
control_generic_class_init (ControlGenericClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (GTK_TYPE_HBOX);
  object_class = (GtkObjectClass *) class;
}

static void
control_generic_init (ControlGeneric *cg)
{
  gtk_container_set_border_width (GTK_CONTAINER (cg), 2);

  cg->mcg  = NULL;
  cg->sync = NULL;
}

GtkWidget *
control_generic_new (void)
{
  GtkWidget *widget;

  widget = gtk_type_new (TYPE_CONTROL_GENERIC);

  return widget;
}

void       
control_generic_assign (ControlGeneric *cg, MDIColorGeneric *mcg)
{
  cg->mcg  = mcg;
 
  control_generic_sync (cg);
}

void       
control_generic_sync (ControlGeneric *cg)
{
  if (cg->sync)
    cg->sync (cg);
}

