#ifndef VIEW_COLOR_LIST_H
#define VIEW_COLOR_LIST_H

#include "view-color-generic.h"
#include "mdi-color-generic.h"

#include <gnome.h>
  
BEGIN_GNOME_DECLS

#define TYPE_VIEW_COLOR_LIST            (view_color_list_get_type ())
#define VIEW_COLOR_LIST(obj)            (GTK_CHECK_CAST ((obj), TYPE_VIEW_COLOR_LIST, ViewColorList))
#define VIEW_COLOR_LIST_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TYPE_VIEW_COLOR_LIST, ViewColorListClass))
#define IS_VIEW_COLOR_LIST(obj)         (GTK_CHECK_TYPE ((obj), TYPE_VIEW_COLOR_LIST))
#define IS_VIEW_COLOR_LIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), TYPE_VIEW_COLOR_LIST))
#define VIEW_COLOR_LIST_GET_CLASS(obj)  (VIEW_COLOR_LIST_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _ViewColorList ViewColorList;
typedef struct _ViewColorListClass ViewColorListClass;
typedef struct _ViewColorListCol ViewColorListCol;

struct _ViewColorList {
  ViewColorGeneric vcl;

  int col_width;
  int col_height;
  gboolean draw_numbers;

  GdkGC *gc;
  GdkColor color_black;
  GdkColor color_white;
  GdkFont *pixmap_font;
  
  guint idle;
  GList *idle_todo;
};

struct _ViewColorListClass {
  ViewColorGenericClass parent_class;
};

GtkType view_color_list_get_type (void);

GtkObject *view_color_list_new (MDIColorGeneric *mcg);

END_GNOME_DECLS

#endif /* _VIEW_COLOR_LIST_H_ */
