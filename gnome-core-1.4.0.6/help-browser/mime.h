#ifndef _GNOME_HELP_MIME_H_
#define _GNOME_HELP_MIME_H_

#include "docobj.h"

void resolveMIME( docObj obj );
void convertMIME( docObj obj );
gchar *execfilter(gchar *execpath, gchar *docpath, gchar *inbuf);

#endif

