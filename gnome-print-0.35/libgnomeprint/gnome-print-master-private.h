#ifndef _GNOME_PRINT_MASTER_PRIVATE_H_
#define _GNOME_PRINT_MASTER_PRIVATE_H

#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

/*
  The GnomePrintMaster object
*/

struct _GnomePrintMaster {
	GtkObject object;

	void *priv;		/* private data */

	GnomePrintContext *context; /* metafile context */

	/* # copies info */
	gint copies;
	gboolean iscollate;

	/* paper */
	const GnomePaper *paper;

	/* printer, if required */
	GnomePrinter *printer;
};

struct _GnomePrintMasterClass {
	GtkObjectClass parent_class;
};

END_GNOME_DECLS

#endif

