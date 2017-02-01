#ifndef __MDI_COLOR_VIRTUAL_H__
#define __MDI_COLOR_VIRTUAL_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "mdi-color-generic.h"
#include "mdi-color.h"

BEGIN_GNOME_DECLS

#define MDI_COLOR_VIRTUAL(obj)          GTK_CHECK_CAST (obj, mdi_color_virtual_get_type (), MDIColorVirtual)
#define MDI_COLOR_VIRTUAL_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, mdi_color_virtual_get_type (), MDIColorVirtualClass)
#define IS_MDI_COLOR_VIRTUAL(obj)       GTK_CHECK_TYPE (obj, mdi_color_virtual_get_type ())
#define MDI_COLOR_VIRTUAL_GET_CLASS(obj)  (MDI_COLOR_VIRTUAL_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _MDIColorVirtual       MDIColorVirtual;
typedef struct _MDIColorVirtualClass  MDIColorVirtualClass;

typedef enum {
  COL_CHANGED_NOTHING,
  COL_CHANGED_TRASH,
  COL_CHANGED_DOC
} col_changed_t;

struct _MDIColorVirtual {
  MDIColorGeneric mdi_child;

  GList *trash;
};

struct _MDIColorVirtualClass {
  MDIColorGenericClass parent_class;

  MDIColor *     (*append_col)  (MDIColorVirtual *mcv, MDIColor *col);
  col_changed_t  (*col_changed) (MDIColorVirtual *mcv, MDIColor *col);
};

guint            mdi_color_virtual_get_type (void);
MDIColorVirtual *mdi_color_virtual_new      (void);

void mdi_color_virtual_trash_to_doc (MDIColorVirtual *mcv);
void mdi_color_virtual_doc_to_trash (MDIColorVirtual *mcv);
void mdi_color_virtual_repost_all (MDIColorVirtual *mcv);

END_GNOME_DECLS

#endif
