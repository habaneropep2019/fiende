#ifndef __GNOME_PRINT_RGBP_H__
#define __GNOME_PRINT_RGBP_H__

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-preview-private.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintRgbp printer context.
 */
#define GNOME_TYPE_PRINT_RGBP	      (gnome_print_rgbp_get_type ())
#define GNOME_PRINT_RGBP(obj)	      (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_RGBP, GnomePrintRGBP))
#define GNOME_PRINT_RGBP_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_RGBP, GnomePrintRGBPClass))
#define GNOME_IS_PRINT_RGBP(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_RGBP))
#define GNOME_IS_PRINT_RGBP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_RGBP))

typedef struct _GnomePrintRGBP        GnomePrintRGBP;
typedef struct _GnomePrintRGBPClass   GnomePrintRGBPClass;

typedef struct _GnomePrintRGBPPrivate GnomePrintRGBPPrivate;

struct _GnomePrintRGBP {
	GnomePrintPreview preview;

	GnomePrintRGBPPrivate *priv;
};

struct _GnomePrintRGBPClass {
	GnomePrintPreviewClass parent_class;

	/*
	 * Virtual methods
	 */
	int (*print_init) (GnomePrintRGBP *rgbp, int dpi);
	int (*page_begin) (GnomePrintRGBP *rgbp);
	int (*page_end)   (GnomePrintRGBP *rgbp);
	int (*print_band) (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *rect);
};

GtkType            gnome_print_rgbp_get_type  (void);
GnomePrintContext *gnome_print_rgbp_new       (const char *paper_size,
					       int dpi);
GnomePrintRGBP    *gnome_print_rgbp_construct (GnomePrintRGBP   *rgbp,
					       const GnomePaper *paper_info,
					       int dpi);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_RGBP_H__ */

