/* -*- mode: C; c-file-style: "linux" -*- */

/* Miscellaneous Imlib-based functions for desktop-properties */

/* This is a very bad hack to allow having a second instance of imlib
 * in addition to the "normal" one initialized by the Gnome libraries.
 * We need a second instance when setting the root window's background
 * because the root window may be running in a different visual than
 * the one imlib normally uses.
 *
 * In this modified version of Imlib_init, we set the visual and
 * colormap to those which are being used by the root window.
 *
 * I know this is ugly, so sue me.  I don't want to copy the user's
 * .imrc to a temporary file and write a new one with the changed
 * visual and colormap
 *
 * - Federico
 */

#include <gdk/gdkx.h>
#include "imlib-misc.h"
#include <gnome.h>

ImlibData *imlib_data;

void
background_imlib_init(void)
{
	ImlibInitParams params;

	params.visualid = XVisualIDFromVisual (DefaultVisual (GDK_DISPLAY (), DefaultScreen (GDK_DISPLAY ())));
	params.flags = PARAMS_VISUALID;

	imlib_data = Imlib_init_with_params (GDK_DISPLAY (), &params);
}
