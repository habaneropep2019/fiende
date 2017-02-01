#ifndef _GNOME_HELP_TOC_INFO_H_
#define _GNOME_HELP_TOC_INFO_H_

#include <glib.h>

#include "toc2.h"

GList *newInfoTable(struct _toc_config *conf);
gint expandInfoTable(GList *table, gchar *name);

#endif
