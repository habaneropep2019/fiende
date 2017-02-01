#ifndef IMLIB_MISC_H
#define IMLIB_MISC_H

#include <Imlib.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

extern ImlibData *imlib_data;

/* Inits a custom-configured imlib with the visual explicitly set to
 * the default root window visual.
 */
void background_imlib_init(void);

END_GNOME_DECLS
#endif
