#ifndef __MDI_COLOR_VIRTUAL_RGB_H__
#define __MDI_COLOR_VIRTUAL_RGB_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "mdi-color-virtual.h"
#include "mdi-color-generic.h"

BEGIN_GNOME_DECLS

#define MDI_COLOR_VIRTUAL_RGB(obj)          GTK_CHECK_CAST (obj, mdi_color_virtual_rgb_get_type (), MDIColorVirtualRGB)
#define MDI_COLOR_VIRTUAL_RGB_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, mdi_color_virtual_rgb_get_type (), MDIColorVirtualRGBClass)
#define IS_MDI_COLOR_VIRTUAL_RGB(obj)       GTK_CHECK_TYPE (obj, mdi_color_virtual_rgb_get_type ())

typedef struct _MDIColorVirtualRGB       MDIColorVirtualRGB;
typedef struct _MDIColorVirtualRGBClass  MDIColorVirtualRGBClass;

struct _MDIColorVirtualRGB {
  MDIColorVirtual virtual;

  float r;
  float g;
  float b;
  float t; /* Tolerance */
};

struct _MDIColorVirtualRGBClass {
  MDIColorVirtualClass parent_class;
};

guint            mdi_color_virtual_rgb_get_type (void);
MDIColorVirtualRGB *mdi_color_virtual_rgb_new      (void);

void             mdi_color_virtual_rgb_set      (MDIColorVirtualRGB *mcv, 
						 float r, float g, 
						 float b, float t);

void             mdi_color_virtual_rgb_get      (MDIColorVirtualRGB *mcv,
						 float *r, float *g, 
						 float *b, float *t);

END_GNOME_DECLS

#endif
