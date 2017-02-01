#ifndef __GNOME_PRINTER_H__
#define __GNOME_PRINTER_H__

#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h> /* for GNOME_DECLS */

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINTER	   (gnome_printer_get_type ())
#define GNOME_PRINTER(obj)	   (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINTER, GnomePrinter))
#define GNOME_PRINTER_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINTER, GnomePrinterClass))
#define GNOME_IS_PRINTER(obj)	   (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINTER))
#define GNOME_IS_PRINTER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINTER))

typedef struct _GnomePrinter       GnomePrinter;
typedef struct _GnomePrinterClass  GnomePrinterClass;

/*
 * INTERNALS ARE PRIVATE UNTIL STABILIZED
 */

GtkType gnome_printer_get_type (void);

#define gnome_printer_ref(p) gtk_object_ref (GTK_OBJECT (p))
#define gnome_printer_unref(p) gtk_object_unref (GTK_OBJECT (p))

GnomePrinter *gnome_printer_new_generic_ps (const char *filename);

typedef enum {
	GNOME_PRINTER_ACTIVE,
	GNOME_PRINTER_INACTIVE,
	GNOME_PRINTER_OFFLINE,
	GNOME_PRINTER_NET_FAILURE,
} GnomePrinterStatus;

GnomePrinterStatus  gnome_printer_get_status (GnomePrinter *printer);
const char         *gnome_printer_str_status (GnomePrinterStatus status);

void gnome_printer_set_print_to_file (GnomePrinter *printer, gboolean print_to_file);
gchar * gnome_printer_dup_command (GnomePrinter *printer);

END_GNOME_DECLS

#endif /* __GNOME_PRINTER_H__ */
