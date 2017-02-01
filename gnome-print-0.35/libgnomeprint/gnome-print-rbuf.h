#ifndef __GNOME_PRINT_RBUF_H__
#define __GNOME_PRINT_RBUF_H__

#include <libgnome/gnome-defs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintRBuf printer context.
 *
 * (C) 2000 Lauris Kaplinski <lauris@ariman.ee>
 *
 * This is general, arbitrary-size rasterizer. Instead of sticking to
 *   device page size, we give it preallocated RGB or RGBA buffer and
 *   page-to-buffer transformation matrix.
 *
 * This is needed for PS alpha printing
 *
 */

#define GNOME_TYPE_PRINT_RBUF	 (gnome_print_rbuf_get_type ())
#define GNOME_PRINT_RBUF(obj)	 (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_RBUF, GnomePrintRBuf))
#define GNOME_PRINT_RBUF_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_RBUF, GnomePrintRBufClass))
#define GNOME_IS_PRINT_RBUF(obj)	    (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_RBUF))
#define GNOME_IS_PRINT_RBUF_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_RBUF))

typedef struct _GnomePrintRBuf GnomePrintRBuf;
typedef struct _GnomePrintRBufClass GnomePrintRBufClass;

GtkType gnome_print_rbuf_get_type (void);

GnomePrintContext * gnome_print_rbuf_new (guchar * pixels,
					gint width,
					gint height,
					gint rowstride,
					gdouble page2buf[6],
					gboolean alpha);

GnomePrintRBuf * gnome_print_rbuf_construct (GnomePrintRBuf * rbuf,
					guchar * pixels,
					gint width,
					gint height,
					gint rowstride,
					gdouble page2buf[6],
					gboolean alpha);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_RBUF_H__ */

