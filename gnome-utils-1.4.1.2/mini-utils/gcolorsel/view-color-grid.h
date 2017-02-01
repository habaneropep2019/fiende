#ifndef VIEW_COLOR_GRID_H
#define VIEW_COLOR_GRID_H

#include "view-color-generic.h"
#include "mdi-color-generic.h"

#include <gnome.h>
  
BEGIN_GNOME_DECLS

#define TYPE_VIEW_COLOR_GRID            (view_color_grid_get_type ())
#define VIEW_COLOR_GRID(obj)            (GTK_CHECK_CAST ((obj), TYPE_VIEW_COLOR_GRID, ViewColorGrid))
#define VIEW_COLOR_GRID_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TYPE_VIEW_COLOR_GRID, ViewColorGridClass))
#define IS_VIEW_COLOR_GRID(obj)         (GTK_CHECK_TYPE ((obj), TYPE_VIEW_COLOR_GRID))
#define IS_VIEW_COLOR_GRID_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), TYPE_VIEW_COLOR_GRID))
#define VIEW_COLOR_GRID_GET_CLASS(obj)  (VIEW_COLOR_GRID_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _ViewColorGrid ViewColorGrid;
typedef struct _ViewColorGridClass ViewColorGridClass;
typedef struct _ViewColorGridCol ViewColorGridCol;

struct _ViewColorGrid {
  ViewColorGeneric vcl;
};

struct _ViewColorGridClass {
  ViewColorGenericClass parent_class;
};

GtkType view_color_grid_get_type (void);

GtkObject *view_color_grid_new (MDIColorGeneric *mcg);

END_GNOME_DECLS

#endif /* _VIEW_COLOR_GRID_H_ */
