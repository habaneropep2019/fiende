#ifndef __GNOME_PRINTER_PRIVATE_H__
#define __GNOME_PRINTER_PRIVATE_H__

#include <gtk/gtkobject.h>


#ifdef ENABLE_LIBGPA
#include <libgpa/gpa-structs.h>
#endif

struct _GnomePrinter
{
	GtkObject object;

#ifdef ENABLE_LIBGPA
	GpaPrinter *gpa_printer;
	GpaSettings *gpa_settings;
	gboolean print_to_file;
#else
	char *driver;
#endif	

	char *filename;
};

struct _GnomePrinterClass
{
	GtkObjectClass parent_class;
};


#ifdef ENABLE_LIBGPA
/* Private prototypes */
GnomePrinter * gnome_printer_new (GpaPrinter *gpa_printer);
 const gchar * gnome_printer_const_get (GnomePrinter *printer, const gchar *info_id);

#endif

#endif /* __GNOME_PRINTER_PRIVATE_H__ */

