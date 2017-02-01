#ifndef VIEW_COLOR_GENERIC_H
#define VIEW_COLOR_GENERIC_H

#include <gtk/gtkobject.h>
#include <libgnome/gnome-defs.h>

#include "mdi-color-generic.h"
  
BEGIN_GNOME_DECLS

typedef enum {
  FORMAT_DEC_8  = 0, 
  FORMAT_DEC_16,
  FORMAT_HEX_8, 
  FORMAT_HEX_16,
  FORMAT_FLOAT
} ColFormat;

extern char *ColFormatStr[];

#define TYPE_VIEW_COLOR_GENERIC            (view_color_generic_get_type ())
#define VIEW_COLOR_GENERIC(obj)            (GTK_CHECK_CAST ((obj), TYPE_VIEW_COLOR_GENERIC, ViewColorGeneric))
#define VIEW_COLOR_GENERIC_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), TYPE_VIEW_COLOR_GENERIC, ViewColorGenericClass))
#define IS_VIEW_COLOR_GENERIC(obj)         (GTK_CHECK_TYPE ((obj), TYPE_VIEW_COLOR_GENERIC))
#define IS_VIEW_COLOR_GENERIC_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), TYPE_VIEW_COLOR_GENERIC))
#define VIEW_COLOR_GENERIC_GET_CLASS(obj)  (VIEW_COLOR_GENERIC_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _ViewColorGeneric ViewColorGeneric;
typedef struct _ViewColorGenericClass ViewColorGenericClass;
typedef struct _ViewColorGenericCol ViewColorGenericCol;

struct _ViewColorGeneric {
  GtkObject object;

  int key;

  gboolean show_control;
  ControlGeneric *control;

  GtkWidget *widget;
  ColFormat format;

  MDIColorGeneric *mcg;
};

struct _ViewColorGenericClass {
  GtkObjectClass parent_class;  

  void     (*data_changed)    (ViewColorGeneric *cg, gpointer data);
  void     (*remove_selected) (ViewColorGeneric *cg);
  GList *  (*get_selected)    (ViewColorGeneric *cg);
  int      (*get_insert_pos)  (ViewColorGeneric *cg);
  gpointer (*get_control)     (ViewColorGeneric *cg, GtkVBox *box,
			       void (*changed_cb)(gpointer data), 
			       gpointer change_data);
  void     (*apply)           (ViewColorGeneric *cg, gpointer data);
  void     (*close)           (ViewColorGeneric *cg, gpointer data);
  void     (*sync)            (ViewColorGeneric *cg, gpointer data);

  void     (*save)            (ViewColorGeneric *mcg);
  void     (*load)            (ViewColorGeneric *mcg);
};

GtkType view_color_generic_get_type (void);

void view_color_generic_data_changed    (ViewColorGeneric *vcg, 
					 GList *changes);

END_GNOME_DECLS

#endif /* _VIEW_COLOR_GENERIC_H_ */
