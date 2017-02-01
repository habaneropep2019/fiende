#ifndef __GNOME_PRINT_PCLR_H__
#define __GNOME_PRINT_PCLR_H__

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-rgbp.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintPixbuf printer context.
 */
#define GNOME_TYPE_PRINT_PCLR	      (gnome_print_pclr_get_type ())
#define GNOME_PRINT_PCLR(obj)	      (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_PCLR, GnomePrintPCLR))
#define GNOME_PRINT_PCLR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_PCLR, GnomePrintPCLRClass))
#define GNOME_IS_PRINT_PCLR(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_PCLR))
#define GNOME_IS_PRINT_PCLR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_PCLR))

typedef struct _GnomePrintPCLR        GnomePrintPCLR;
typedef struct _GnomePrintPCLRPrivate GnomePrintPCLRPrivate;
typedef struct _GnomePrintPCLRClass   GnomePrintPCLRClass;

struct _GnomePrintPCLR {
	GnomePrintRGBP rgbp;

	GnomePrintPCLRPrivate *priv;
};

struct _GnomePrintPCLRClass {
	GnomePrintRGBPClass parent_class;
};

GtkType            gnome_print_pclr_get_type  (void);
GnomePrintContext *gnome_print_pclr_new       (GnomePrinter *printer,
					       const char *paper_size,
					       int dpi);
GnomePrintPCLR    *gnome_print_pclr_construct (GnomePrintPCLR *pclr,
					       GnomePrinter *printer,
					       const GnomePaper *paper_info,
					       int dpi);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_PCLR_H__ */

