#ifndef __MDI_COLOR_FILE_H__
#define __MDI_COLOR_FILE_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "mdi-color-generic.h"

BEGIN_GNOME_DECLS

#define MDI_COLOR_FILE(obj)          GTK_CHECK_CAST (obj, mdi_color_file_get_type (), MDIColorFile)
#define MDI_COLOR_FILE_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, mdi_color_file_get_type (), MDIColorFileClass)
#define IS_MDI_COLOR_FILE(obj)       GTK_CHECK_TYPE (obj, mdi_color_file_get_type ())

typedef struct _MDIColorFile       MDIColorFile;
typedef struct _MDIColorFileClass  MDIColorFileClass;

struct _MDIColorFile {
  MDIColorGeneric mdi_child;

  char *filename;
  gboolean create;

  char *header;
  char *comments_begin;
  char *comments_end;
};

struct _MDIColorFileClass {
  MDIColorGenericClass parent_class;
};

guint         mdi_color_file_get_type     (void);

MDIColorFile *mdi_color_file_new          (void);

gboolean      mdi_color_file_load         (MDIColorFile *mcf, GnomeMDI *mdi);
gboolean      mdi_color_file_save         (MDIColorFile *mcf);
void          mdi_color_file_set_filename (MDIColorFile *mcf, 
					   const char *filename, gboolean create);
					   
END_GNOME_DECLS

#endif
