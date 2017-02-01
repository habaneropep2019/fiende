#include "mdi-color-generic.h"

#include <gtk/gtkobject.h>

static void mdi_color_class_init       (MDIColorClass *class);
static void mdi_color_init             (MDIColor *mcg);

static void mdi_color_destroy          (GtkObject *object);

static GtkObjectClass *parent_class = NULL;

guint 
mdi_color_get_type()
{
  static guint mdi_color_type = 0;
  
  if (!mdi_color_type) {
    GtkTypeInfo mdi_color_type_info = {
      "MDIColor",
      sizeof (MDIColor),
      sizeof (MDIColorClass),
      (GtkClassInitFunc) mdi_color_class_init,
      (GtkObjectInitFunc) mdi_color_init,
      (GtkArgSetFunc) NULL,
      (GtkArgGetFunc) NULL,
    };
    
    mdi_color_type = gtk_type_unique (gtk_object_get_type (), 
				      &mdi_color_type_info);
  }
  
  return mdi_color_type;
}

static void 
mdi_color_class_init (MDIColorClass *class)
{
  GtkObjectClass      *object_class;

  object_class          = GTK_OBJECT_CLASS (class);
  parent_class          = gtk_type_class (gtk_object_get_type ());

  object_class->destroy = mdi_color_destroy;
}

static void
mdi_color_init (MDIColor *col)
{
  col->owner = NULL;
  col->list  = NULL;
}

static void
mdi_color_destroy (GtkObject *object)
{
  MDIColor *col = MDI_COLOR (object);

  /*  printf ("destroy notify : %s\n", col->name); */

  g_free (col->name);
  col->name = NULL;

  parent_class->destroy (object);
}

GtkObject *
mdi_color_new (void)
{
  GtkObject *object;

  object = gtk_type_new (TYPE_MDI_COLOR);

  return object;
}
