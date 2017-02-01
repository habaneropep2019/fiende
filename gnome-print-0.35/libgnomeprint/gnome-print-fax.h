#ifndef __GNOME_PRINT_FAX_H__
#define __GNOME_PRINT_FAX_H__

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-rgbp.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintPixbuf printer context.
 */
#define GNOME_TYPE_PRINT_FAX	      (gnome_print_fax_get_type ())
#define GNOME_PRINT_FAX(obj)	      (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_FAX, GnomePrintFAX))
#define GNOME_PRINT_FAX_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_FAX, GnomePrintFAXClass))
#define GNOME_IS_PRINT_FAX(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_FAX))
#define GNOME_IS_PRINT_FAX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_FAX))

typedef struct _GnomePrintFAX        GnomePrintFAX;
typedef struct _GnomePrintFAXPrivate GnomePrintFAXPrivate;
typedef struct _GnomePrintFAXClass   GnomePrintFAXClass;

struct _GnomePrintFAX {
	GnomePrintRGBP rgbp;

	GnomePrintFAXPrivate *priv;
};

struct _GnomePrintFAXClass {
	GnomePrintRGBPClass parent_class;
};

GtkType            gnome_print_fax_get_type  (void);
GnomePrintContext *gnome_print_fax_new       (GnomePrinter *printer,
					       const char *paper_size,
					       int dpi);
GnomePrintFAX    *gnome_print_fax_construct (GnomePrintFAX *fax,
					       GnomePrinter *printer,
					       const GnomePaper *paper_info,
					       int dpi);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_PCLR_H__ */

