#ifndef _GF_PFB_H_
#define _GF_PFB_H_

#include <glib.h>
#include <libgnome/gnome-defs.h>
#include "parseAFM.h"

BEGIN_GNOME_DECLS

typedef struct _GFPFB GFPFB;

struct _GFPFB {
	gchar *filename;
	gboolean ascii;
	GlobalFontInfo gfi;
};

GFPFB * gf_pfb_open (const gchar *name);

void gf_pfb_close (GFPFB *pfb);

/* Checks StartFontMetrics string */
gboolean gf_afm_check (const guchar *name);

END_GNOME_DECLS

#endif
