#ifndef __GNOME_PRINT_PREVIEW_H__
#define __GNOME_PRINT_PREVIEW_H__

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintPreview printer context.
 */
#define GNOME_TYPE_PRINT_PREVIEW	 (gnome_print_preview_get_type ())
#define GNOME_PRINT_PREVIEW(obj)	 (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_PREVIEW, GnomePrintPreview))
#define GNOME_PRINT_PREVIEW_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_PREVIEW, GnomePrintPreviewClass))
#define GNOME_IS_PRINT_PREVIEW(obj)	    (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_PREVIEW))
#define GNOME_IS_PRINT_PREVIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_PREVIEW))

typedef struct _GnomePrintPreview        GnomePrintPreview;
typedef struct _GnomePrintPreviewClass   GnomePrintPreviewClass;

GtkType            gnome_print_preview_get_type  (void);
GnomePrintContext *gnome_print_preview_new       (GnomeCanvas *canvas,
						  const char *paper_size);
void               gnome_print_preview_construct (GnomePrintPreview *preview,
						  GnomeCanvas *canvas,
						  const GnomePaper *paper_info);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_PREVIEW_H__ */

