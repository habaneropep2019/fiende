#ifndef _GP_PS_UNICODE_H_
#define _GP_PS_UNICODE_H_

/*
 * Unicode to PostScript glyph mapping
 *
 * Authors:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 1999-2000 Helix Code, Inc.
 *
 */

#include <glib.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

/* 0 if not defined */

gint gp_unicode_from_ps (const gchar * name);
gint gp_unicode_from_dingbats (const gchar * name);

/* NULL if not defined */

const gchar * gp_ps_from_unicode (gint unicode);

/* NULL if not defined */

const gchar * gp_const_ps_from_ps (const gchar * name);

/* Returns GSList of code values, if there are > 1, NULL otherwise */

const GSList * gp_multi_from_ps (const gchar * name);

/* Get ps name semantics according to Adobe guidelines */
/* Reference: http://partners.adobe.com/asn/developer/typeforum/unicodegn.html */

GSList * gp_ps_get_semantics (const gchar * name, gboolean * isDecomp, gboolean * isVar);

END_GNOME_DECLS

#endif

