#ifndef __GNOME_FONT_FACE_PRIVATE_H__
#define __GNOME_FONT_FACE_PRIVATE_H__

/*
 * Here are private experimental API methods
 *
 * Any of these can be changed without notice, they
 * are strictly meant for developers, creating tools
 * and support libraries for gnome-print, NOT
 * application developers
 *
 * Authors:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <libgnomeprint/gnome-font-face.h>

/*
 * WARNING
 *
 * THIS IS UNSTABLE API
 *
 * IT CAN BE REMOVED WITHOUT NOTICE
 *
 */

#ifdef USE_GNOME_FONT_FACE_EXPERIMENTAL_INTERFACE
GnomeFontFace *gnome_font_face_private_from_files (const guchar *filename, gint facenum, const GSList *additional);
#endif

/*
 * END OF WARNING
 *
 */

END_GNOME_DECLS

#endif /* __GNOME_FONT_FACE_H__ */



