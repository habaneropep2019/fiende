#ifndef _GNOME_PRINT_RBUF_PRIVATE_H_
#define _GNOME_PRINT_RBUF_PRIVATE_H_

#include <libgnomeprint/gnome-print-private.h>

typedef struct _GnomePrintRBufPrivate GnomePrintRBufPrivate;

struct _GnomePrintRBuf {
	GnomePrintContext pc;

	GnomePrintRBufPrivate * private;
};

struct _GnomePrintRBufClass {
	GnomePrintContextClass parent_class;
};

#endif
