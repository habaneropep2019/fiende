#ifndef __GF_TTF_H__
#define __GF_TTF_H__

#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnomeprint/parseAFM.h>

BEGIN_GNOME_DECLS

typedef struct _GFTTF GFTTF;

struct _GFTTF {
	gchar *filename;
	guint num_faces;
	GlobalFontInfo gfi;
};

GFTTF *gf_ttf_open (const gchar *name, gint face, const guchar *familyname, const guchar *stylename);

void gf_ttf_close (GFTTF *pfb);

END_GNOME_DECLS

#endif
