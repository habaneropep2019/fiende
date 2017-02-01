#ifndef __MDI_COLOR_GENERIC_H__
#define __MDI_COLOR_GENERIC_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <libgnomeui/gnome-mdi-child.h>

BEGIN_GNOME_DECLS

#define MDI_COLOR_GENERIC(obj)          GTK_CHECK_CAST (obj, mdi_color_generic_get_type (), MDIColorGeneric)
#define MDI_COLOR_GENERIC_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, mdi_color_generic_get_type (), MDIColorGenericClass)
#define IS_MDI_COLOR_GENERIC(obj)       GTK_CHECK_TYPE (obj, mdi_color_generic_get_type ())
#define MDI_COLOR_GENERIC_GET_CLASS(obj)  (MDI_COLOR_GENERIC_CLASS (GTK_OBJECT (obj)->klass))

typedef struct _MDIColorGeneric       MDIColorGeneric;
typedef struct _MDIColorGenericClass  MDIColorGenericClass;

#include "mdi-color.h"
#include "widget-control-generic.h"

struct _MDIColorGeneric {
  GnomeMDIChild mdi_child;

  char *name; /* Same that GnomeMDIChild->name */

  int key;

  int modified;
  int monitor_modified;

  int flags; /* What the user can do. For example, it make no sense to
                move or append a color in a MDIColorVirtual.
		But he can Remove a color in a MDIColorVirtual */

  gboolean temp; /* TRUE means that it's a temporary doc, that no
		    confirmation is needed when closing the document, ... */

  int freeze_count;
  int last;
 
  GList *views_type;

  GList *other_views;
  GList *docs;
  GList *parents;

  GList *col;
  GList *last_col;

  GList *changes;
  GList *last_changes;
  int changes_phase;

  MDIColor * (*get_owner) (MDIColor *col);
  GList * (*get_append_pos)     (MDIColorGeneric *mcg, MDIColor *col);
  GtkType (*get_control_type) (MDIColorGeneric *mcg);
};

struct _MDIColorGenericClass {
  GnomeMDIChildClass parent_class;

  void     (*document_changed) (MDIColorGeneric *mcg, gpointer data);
  gpointer (*get_control)      (MDIColorGeneric *cg, GtkVBox *box,
				void (*changed_cb)(gpointer data), 
				gpointer change_data);
  void     (*apply)            (MDIColorGeneric *mcg, gpointer data);
  void     (*close)            (MDIColorGeneric *mcg, gpointer data);
  void     (*sync)             (MDIColorGeneric *mcg, gpointer data);

  void     (*save)             (MDIColorGeneric *mcg);
  void     (*load)             (MDIColorGeneric *mcg);
};

guint mdi_color_generic_get_type (void);

GtkWidget *mdi_color_generic_create_other_view (MDIColorGeneric *mcg);

void mdi_color_generic_append           (MDIColorGeneric *mcg, 
					 MDIColor *col);
MDIColor *
mdi_color_generic_append_new            (MDIColorGeneric *mcg,
					 int r, int g, int b, char *name);
MDIColor *
mdi_color_generic_append_new_set_data   (MDIColorGeneric *mcg, 
					 int r, int g, int b, char *name,
					 char *str, gpointer data);

void mdi_color_generic_remove           (MDIColorGeneric *mcg, 
					 MDIColor *col);
void mdi_color_generic_remove_list      (MDIColorGeneric *mcg, 
					 GList *list_to_del);
void mdi_color_generic_change_rgb       (MDIColorGeneric *mcg, 
					 MDIColor *col,
					 int r, int g, int b);
void mdi_color_generic_change_name      (MDIColorGeneric *mcg, 
					 MDIColor *col,
					 char *name);
void mdi_color_generic_change_pos       (MDIColorGeneric *mcg,
					 MDIColor *col, int new_pos);
void mdi_color_generic_clear            (MDIColorGeneric *mcg);

void mdi_color_generic_freeze           (MDIColorGeneric *mcg);
void mdi_color_generic_thaw             (MDIColorGeneric *mcg);

void mdi_color_generic_post_change      (MDIColorGeneric *mcg, 
					 MDIColor *col,
					 MDIColorChangeType type);
void mdi_color_generic_dispatch_changes (MDIColorGeneric *mcg);

gboolean mdi_color_generic_can_do       (MDIColorGeneric *mcg, 
					 MDIColorChangeType what);

MDIColor *
mdi_color_generic_search_by_data        (MDIColorGeneric *mcg, char *str,
					 gpointer data);

void mdi_color_generic_connect          (MDIColorGeneric *mcg,
					 MDIColorGeneric *to);

void mdi_color_generic_disconnect       (MDIColorGeneric *mcg,
					 MDIColorGeneric *to);

MDIColor *
mdi_color_generic_get_owner             (MDIColor *col);

GtkType mdi_color_generic_get_control_type (MDIColorGeneric *mcg);
GList  *mdi_color_generic_get_append_pos (MDIColorGeneric *mcg,
					  MDIColor *col);


void mdi_color_generic_sync_control     (MDIColorGeneric *mcg);

void mdi_color_generic_append_view_type   (MDIColorGeneric *mcg, GtkType type);

void mdi_color_generic_set_modified (MDIColorGeneric *mcg, gboolean modified);
void mdi_color_generic_set_name     (MDIColorGeneric *mcg, char *name);

void mdi_color_generic_set_temp     (MDIColorGeneric *mcg, gboolean val);
					
END_GNOME_DECLS

#endif
