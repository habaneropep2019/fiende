#ifndef _GNOME_HELP_MISC_H_
#define _GNOME_HELP_MISC_H_

#include <glib.h>

gint getOutputFrom(gchar *argv[], gchar *writePtr, gint writeBytesLeft,
		   guchar **outbuf, gint *outbuflen);
void map_spaces_to_underscores( gchar *str );
gint loadFileToBuf( gchar *file, guchar **buf, gint *lenout );

#endif
