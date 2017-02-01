#ifndef _GNOME_HELP_TOC_MAN_H_
#define _GNOME_HELP_TOC_MAN_H_

#include <glib.h>

#include "toc2.h"

GList *newManTable(struct _toc_config *conf);

gchar *getManSection(gchar ch);

#endif
