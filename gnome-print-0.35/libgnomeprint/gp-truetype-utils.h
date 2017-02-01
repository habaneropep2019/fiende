#ifndef __GP_TRUETYPE_UTILS_H__
#define __GP_TRUETYPE_UTILS_H__

/*
 * Utility to break TrueType file at table boundaries
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Distributed under GNU Lesser General Public License
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#include <glib.h>

GSList *gp_tt_split_file (const guchar *buf, guint len);

END_GNOME_DECLS

#endif

