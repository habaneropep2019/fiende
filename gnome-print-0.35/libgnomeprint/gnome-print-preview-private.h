#ifndef __GNOME_PRINT_PREVIEW_PRIVATE_H__
#define __GNOME_PRINT_PREVIEW_PRIVATE_H__

#include <libgnome/gnome-defs.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeprint/gnome-print-private.h>

BEGIN_GNOME_DECLS

typedef struct _GnomePrintPreviewPrivate GnomePrintPreviewPrivate;

struct _GnomePrintPreview {
	GnomePrintContext pc;

	GnomePrintPreviewPrivate *priv;

	GnomeCanvas *canvas;
};

struct _GnomePrintPreviewClass {
	GnomePrintContextClass parent_class;
};

END_GNOME_DECLS

#endif

