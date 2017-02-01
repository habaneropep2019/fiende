#ifndef __MDI_COLOR_H__
#define __MDI_COLOR_H__

#include <gtk/gtkobject.h>

BEGIN_GNOME_DECLS

#define TYPE_MDI_COLOR          (mdi_color_get_type ())
#define MDI_COLOR(obj)          GTK_CHECK_CAST (obj, mdi_color_get_type (), MDIColor)
#define MDI_COLOR_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, mdi_color_get_type (), MDIColorClass)
#define IS_MDI_COLOR(obj)       GTK_CHECK_TYPE (obj, mdi_color_get_type ())
#define MDI_COLOR_GET_CLASS(obj)  (MDI_COLOR_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _MDIColor       MDIColor;
typedef struct _MDIColorClass  MDIColorClass;

struct _MDIColor {
  GtkObject object;

  MDIColorGeneric *owner; /* Never a MDIColorVirtual, ... */

  guint pos;
  guint r : 8;
  guint g : 8;
  guint b : 8;
  char *name;

  guint change : 8;
  int change_phase;

  GList *list; /* list->data == self */
};

struct _MDIColorClass {
  GtkObjectClass parent_class;
};

typedef enum {
  /* Insert new */
  CHANGE_APPEND = 1 << 0,

  /* Edit existing */
  CHANGE_NAME   = 1 << 1,
  CHANGE_RGB    = 1 << 2,
  CHANGE_POS    = 1 << 4,

  /* Remove existing */
  CHANGE_REMOVE = 1 << 5,

  /* Remove all */
  CHANGE_CLEAR  = 1 << 6
} MDIColorChangeType;

guint mdi_color_get_type (void);

GtkObject *mdi_color_new (void);

END_GNOME_DECLS

#endif
