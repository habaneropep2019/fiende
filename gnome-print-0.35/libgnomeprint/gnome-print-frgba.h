#ifndef __GNOME_PRINT_FRGBA_H__
#define __GNOME_PRINT_FRGBA_H__

#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

/*
 * GnomePrintFRGBA printer context.
 *
 * (C) 2000 Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This is Fake RGBA (TM) printing context - i.e. it spies existing print
 * context and renders all semitransparent graphics into RGB buffer printing
 * that buffer instead of original.
 *
 * This is needed for PS alpha printing
 *
 */

#define GNOME_TYPE_PRINT_FRGBA	 (gnome_print_frgba_get_type ())
#define GNOME_PRINT_FRGBA(obj)	 (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_FRGBA, GnomePrintFRGBA))
#define GNOME_PRINT_FRGBA_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_FRGBA, GnomePrintFRGBAClass))
#define GNOME_IS_PRINT_FRGBA(obj)	    (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_FRGBA))
#define GNOME_IS_PRINT_FRGBA_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_FRGBA))

typedef struct _GnomePrintFRGBA GnomePrintFRGBA;
typedef struct _GnomePrintFRGBAPrivate GnomePrintFRGBAPrivate;
typedef struct _GnomePrintFRGBAClass GnomePrintFRGBAClass;

GtkType gnome_print_frgba_get_type (void);

GnomePrintContext * gnome_print_frgba_new (GnomePrintContext * context);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_FRGBA_H__ */

