#ifndef __GNOME_PRINT_PIXBUF_H__
#define __GNOME_PRINT_PIXBUF_H__

#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintPixbuf printer context.
 */

#define GNOME_TYPE_PRINT_PIXBUF	      (gnome_print_pixbuf_get_type ())
#define GNOME_PRINT_PIXBUF(obj)	      (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_PIXBUF, GnomePrintPixbuf))
#define GNOME_PRINT_PIXBUF_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_PIXBUF, GnomePrintPixbufClass))
#define GNOME_IS_PRINT_PIXBUF(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_PIXBUF))
#define GNOME_IS_PRINT_PIXBUF_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_PIXBUF))

typedef struct _GnomePrintPixbuf        GnomePrintPixbuf;
typedef struct _GnomePrintPixbufClass   GnomePrintPixbufClass;

GtkType            gnome_print_pixbuf_get_type  (void);

GnomePrintContext *gnome_print_pixbuf_new       (gdouble x0, gdouble y0,
						 gdouble x1, gdouble y1,
						 gdouble xdpi, gdouble ydpi,
						 gboolean alpha);
GnomePrintContext *gnome_print_pixbuf_construct (GnomePrintPixbuf * gpb,
						 gdouble x0, gdouble y0,
						 gdouble x1, gdouble y1,
						 gdouble xdpi, gdouble ydpi,
						 gboolean alpha);


GdkPixbuf * gnome_print_pixbuf_get_pixbuf (GnomePrintPixbuf * gpb);
gint gnome_print_pixbuf_get_pagenum (GnomePrintPixbuf * gpb);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_PIXBUF_H__ */

