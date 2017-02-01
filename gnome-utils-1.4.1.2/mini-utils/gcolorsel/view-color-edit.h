#ifndef VIEW_COLOR_EDIT_H
#define VIEW_COLOR_EDIT_H

#include "view-color-generic.h"
#include "mdi-color-generic.h"

#include <gnome.h>
  
BEGIN_GNOME_DECLS

#define TYPE_VIEW_COLOR_EDIT            (view_color_edit_get_type ())
#define VIEW_COLOR_EDIT(obj)            (GTK_CHECK_CAST ((obj), TYPE_VIEW_COLOR_EDIT, ViewColorEdit))
#define VIEW_COLOR_EDIT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TYPE_VIEW_COLOR_EDIT, ViewColorEditClass))
#define IS_VIEW_COLOR_EDIT(obj)         (GTK_CHECK_TYPE ((obj), TYPE_VIEW_COLOR_EDIT))
#define IS_VIEW_COLOR_EDIT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), TYPE_VIEW_COLOR_EDIT))
#define VIEW_COLOR_EDIT_GET_CLASS(obj)  (VIEW_COLOR_EDIT_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _ViewColorEdit ViewColorEdit;
typedef struct _ViewColorEditClass ViewColorEditClass;
typedef struct _ViewColorEditCol ViewColorEditCol;

struct _ViewColorEdit {
  ViewColorGeneric vcg;

  MDIColor *editing;

  GtkWidget *spin_red;
  GtkWidget *spin_green;
  GtkWidget *spin_blue;
  GtkWidget *spin_position;

  GtkWidget *button_prev;
  GtkWidget *button_next;

  GtkWidget *preview;
  GtkWidget *combo;
  GtkWidget *entry_name;
};

struct _ViewColorEditClass {
  ViewColorGenericClass parent_class;
};

GtkType view_color_edit_get_type (void);

GtkObject *view_color_edit_new (MDIColorGeneric *mcg);

END_GNOME_DECLS

#endif /* _VIEW_COLOR_EDIT_H_ */
