#ifndef _GNOME_HELP_VISIT_H_
#define _GNOME_HELP_VISIT_H_

#include <glib.h>

#include "window.h"

gint visitURL( HelpWindow win, const gchar *ref, 
	       gboolean useCache, gboolean addToQueue, gboolean addToHistory );

/* old utility macro */
#define getRefBits(x) ( (!strcmp((x), Template)) ? \
			       g_string_new(Bits) : NULL)
#endif
