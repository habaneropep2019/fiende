/* Must include these two first */
#include "config.h"
#include <libgnomeprint/gnome-print-i18n.h>

#include <gtk/gtk.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-private.h>
#ifdef ENABLE_LIBGPA
#include <libgpa/gpa-printer.h>
#include <libgpa/gpa-settings.h>
#endif

static void gnome_printer_class_init (GnomePrinterClass *klass);
static void gnome_printer_init (GnomePrinter *printer);
static void gnome_printer_finalize (GtkObject *object);

static GtkObjectClass *parent_class = NULL;

GtkType
gnome_printer_get_type (void)
{
	static GtkType printer_type = 0;
	
	if (!printer_type)
	{
		GtkTypeInfo printer_info =
		{
			"GnomePrinter",
			sizeof (GnomePrinter),
			sizeof (GnomePrinterClass),
			(GtkClassInitFunc) gnome_printer_class_init,
			(GtkObjectInitFunc) gnome_printer_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		printer_type = gtk_type_unique (gtk_object_get_type (), &printer_info);
	}
	
	return printer_type;
}

static void
gnome_printer_class_init (GnomePrinterClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass*) class;
	
	parent_class = gtk_type_class (gtk_object_get_type ());
	
	object_class->finalize = gnome_printer_finalize;
	

}

static void
gnome_printer_init (GnomePrinter *printer)
{
#ifdef ENABLE_LIBGPA
	printer->gpa_printer = NULL;
	printer->gpa_settings = NULL;
	printer->print_to_file = FALSE;
#else
	printer->driver = NULL;
#endif
	printer->filename = NULL;
}

GnomePrinter *
gnome_printer_new_generic_ps (const char *filename)
{
	GnomePrinter *printer;
	
	printer = gtk_type_new (gnome_printer_get_type ());
#ifndef ENABLE_LIBGPA	
	printer->driver = g_strdup ("gnome-print-ps2");
#endif
	printer->filename = g_strdup (filename);
	
	return printer;
}


static void
gnome_printer_finalize (GtkObject *object)
{
	GnomePrinter *printer;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_PRINTER (object));
	
	printer = GNOME_PRINTER (object);
	
	if (printer->filename)
		g_free (printer->filename);
#ifdef ENABLE_LIBGPA
	if (printer->gpa_printer)
		gpa_printer_unref (printer->gpa_printer);
	printer->gpa_printer = NULL;
#else
	if (printer->driver)
		g_free (printer->driver);
#endif	
	
	(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

/**
 * gnome_printer_get_status:
 * @printer: The printer
 *
 * Returns the current status of @printer
 */
GnomePrinterStatus
gnome_printer_get_status (GnomePrinter *printer)
{
	g_return_val_if_fail (printer != NULL, GNOME_PRINTER_INACTIVE);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), GNOME_PRINTER_INACTIVE);

	return GNOME_PRINTER_INACTIVE;
}

/**
 * gnome_printer_str_status
 * @status: A status type
 *
 * Returns a string representation of the printer status code @status
 */
const char *
gnome_printer_str_status (GnomePrinterStatus status)
{
	switch (status){
	case GNOME_PRINTER_ACTIVE:
		return _("Printer is active");

	case GNOME_PRINTER_INACTIVE:
		return _("Printer is ready to print");

	case GNOME_PRINTER_OFFLINE:
		return _("Printer is off-line");

	case GNOME_PRINTER_NET_FAILURE:
		return _("Can not communicate with printer");

	}
	return _("Unknown status");
}






#ifdef ENABLE_LIBGPA
/**
 * gnome_printer_create:
 * @gpa_printer: A pointer to the GpaPrinter structure 
 * 
 * Creates a GnomePrinter printer from a GpaPrinter pointer
 * 
 * Return Value: 
 **/
GnomePrinter *
gnome_printer_new (GpaPrinter *gpa_printer)
{
	GnomePrinter *printer;

	g_return_val_if_fail (GPA_IS_PRINTER (gpa_printer), NULL);
		
	printer = gtk_type_new (gnome_printer_get_type ());
	printer->gpa_printer = gpa_printer;
	printer->gpa_settings = gpa_printer_settings_get_first (gpa_printer);
	printer->filename = gpa_settings_dup_command (printer->gpa_settings);

	g_print ("Using the *%s* settings\n", gpa_settings_get_name (printer->gpa_settings));
		
	return printer;
}


const gchar *
gnome_printer_const_get (GnomePrinter *printer, const gchar *info_id)
{
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer->gpa_printer), NULL);

	return gpa_printer_backend_info_get (printer->gpa_printer, "GNOME", info_id);
}

void
gnome_printer_set_print_to_file (GnomePrinter *printer, gboolean print_to_file)
{
	g_return_if_fail (GNOME_IS_PRINTER (printer));

	printer->print_to_file = print_to_file ? TRUE : FALSE;
}


gchar *
gnome_printer_dup_command (GnomePrinter *printer)
{
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);

	return gpa_settings_dup_command (printer->gpa_settings);
}
#endif	
