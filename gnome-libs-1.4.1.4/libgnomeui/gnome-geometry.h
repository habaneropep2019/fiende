#ifndef __GNOME_GEOMETRY_H_
#define __GNOME_GEOMETRY_H_ 

#include <libgnome/gnome-defs.h>
#include <gdk/gdk.h>

BEGIN_GNOME_DECLS

/* Return TRUE on success */
gboolean gnome_parse_geometry (const gchar *geometry, gint *xpos, gint *ypos, 
			       gint *width, gint *height);

/* Return a g_malloc'd string representing the window's geometry, suitable
   for parsing by gnome_parse_geometry. */
gchar * gnome_geometry_string (GdkWindow * window);

END_GNOME_DECLS

#endif
