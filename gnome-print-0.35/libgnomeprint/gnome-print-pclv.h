#ifndef __GNOME_PRINT_PCLV_H__
#define __GNOME_PRINT_PCLV_H__

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-rgbp.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintPixbuf printer context.
 */
#define GNOME_TYPE_PRINT_PCLV	      (gnome_print_pclv_get_type ())
#define GNOME_PRINT_PCLV(obj)	      (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_PCLV, GnomePrintPCLV))
#define GNOME_PRINT_PCLV_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_PCLV, GnomePrintPCLVClass))
#define GNOME_IS_PRINT_PCLV(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_PCLV))
#define GNOME_IS_PRINT_PCLV_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_PCLV))

typedef struct _GnomePrintPCLV        GnomePrintPCLV;
typedef struct _GnomePrintPCLVPrivate GnomePrintPCLVPrivate;
typedef struct _GnomePrintPCLVClass   GnomePrintPCLVClass;

struct _GnomePrintPCLV {
	GnomePrintRGBP rgbp;

	GnomePrintPCLVPrivate *priv;
};

struct _GnomePrintPCLVClass {
	GnomePrintRGBPClass parent_class;
};

GtkType            gnome_print_pclv_get_type  (void);
GnomePrintContext *gnome_print_pclv_new       (GnomePrinter *printer,
					       const char *paper_size,
					       int dpi);
GnomePrintPCLV    *gnome_print_pclv_construct (GnomePrintPCLV *pclv,
					       GnomePrinter *printer,
					       const GnomePaper *paper_info,
					       int dpi);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_PCLV_H__ */
