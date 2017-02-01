/*
 * GnomePrinterProfile:
 *   Routines to get information about printers and the
 *   way to print on these printers.
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 */
#include <config.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtkobject.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-config.h>
#include <libgnome/gnome-util.h>
#include <libgnomeprint/gnome-print-i18n.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-printer-profile.h>

struct _GnomePrinterProfile {
	char *name;
	char *comment;
	char *driver;
	char *mime_type;
	char *location;
	char *output;
};

static void
gnome_printer_load_profiles_from (const char *filename, GnomePrinterProfileList **list)
{
	char *base_prefix, *name;
	void *iter;

	gnome_config_push_prefix ("");
	
	base_prefix = g_strdup_printf ("=%s=", filename);
	iter = gnome_config_init_iterator_sections (base_prefix);

	while (gnome_config_iterator_next (iter, &name, NULL)){
		GnomePrinterProfile *pp;
		char *prefix;
		
		prefix = g_strdup_printf ("%s/%s/", base_prefix, name);
		gnome_config_push_prefix (prefix);

		pp = g_new0 (GnomePrinterProfile, 1);
		pp->name      = gnome_config_get_translated_string ("name");
		pp->comment   = gnome_config_get_translated_string ("comment");
		pp->driver    = gnome_config_get_string ("driver");
		pp->mime_type = gnome_config_get_string ("mime-type");
		pp->location  = gnome_config_get_string ("location");
		pp->output    = gnome_config_get_string ("output");

		gnome_config_pop_prefix ();
		g_free(prefix);

		if (pp->name && pp->driver && pp->output) {
			*list = g_list_prepend (*list, pp);
		} else {
			g_free (pp);
		}
		g_free (name);
	}
	
	g_free(base_prefix);

	gnome_config_pop_prefix ();
	
	*list = g_list_reverse (*list);
}

	

static void
gnome_printer_load_profiles_from_dir (const char *dirname, GnomePrinterProfileList **list)
{
	DIR *dir;
	struct dirent *dent;
	const int extlen = sizeof (".profile") - 1;

	dir = opendir (dirname);
	if (!dir)
		return;

	while ((dent = readdir (dir)) != NULL){
		char *filename;
		int len = strlen (dent->d_name);

		if (len <= extlen)
			continue;
		
		if (strcmp (dent->d_name + len - extlen, ".profile"))
			continue;

		filename = g_concat_dir_and_file (dirname, dent->d_name);
		gnome_printer_load_profiles_from (filename, list);
		g_free (filename);
	}
	closedir (dir);

}

static GnomePrinterProfile *
gnome_printer_stock_profile (void)
{
	GnomePrinterProfile *pp;

	pp = g_new0 (GnomePrinterProfile, 1);
	pp->name = g_strdup (_("Generic Postscript"));
	pp->driver = g_strdup ("gnome-print-ps2");
	pp->mime_type = g_strdup ("application/postscript");
	pp->output = g_strdup ("output.ps");

	return pp;
}

static gint
gpp_compare_profiles (GnomePrinterProfile *a, GnomePrinterProfile *b)
{
	/*
	 * Now that is silly ;) - but it works for stock profiles
	 * and that is all we need
	 */

	if (!strcmp (a->driver, "gnome-print-ps2") && !strcmp (b->driver, "gnome-print-ps2")) {
		if (!strcmp (a->output, "command,lpr")) return -1;
		if (!strcmp (b->output, "command,lpr")) return 1;
		if (!strcmp (a->output, "file,output.ps")) return -1;
		if (!strcmp (b->output, "file,output.ps")) return 1;
		return strcmp (a->output, b->output);
	} else if (!strcmp (a->driver, "gnome-print-ps2")) {
		return -1;
	}  else if (!strcmp (b->driver, "gnome-print-ps2")) {
		return 1;
	}

	return strcmp (a->output, b->output);
}

/**
 * gnome_printer_get_profiles:
 *
 * Loads the printer profiles defined on the system and in
 * those defined by this user.
 *
 * Returns a list with GnomePrinterProfile structures inside.
 * The list always contains at least one element.
 */
GnomePrinterProfileList *
gnome_printer_get_profiles (void)
{
	GnomePrinterProfileList *list = NULL;
	char *user_dir;

	gnome_printer_load_profiles_from_dir (PROFILEDIR, &list);
	user_dir = gnome_util_home_file ("printer-profile");
	gnome_printer_load_profiles_from_dir (user_dir, &list);
	g_free (user_dir);

	if (list == NULL) {
		list = g_list_prepend (list, gnome_printer_stock_profile ());
	}

	list = g_list_sort (list, (GCompareFunc) gpp_compare_profiles);

	return list;
}

static void
gnome_printer_profile_free (GnomePrinterProfile *pp)
{
	if (pp->name)
		g_free (pp->name);
	if (pp->comment)
		g_free (pp->comment);
	if (pp->driver)
		g_free (pp->driver);
	if (pp->mime_type)
		g_free (pp->mime_type);
	if (pp->location)
		g_free (pp->location);
	if (pp->output)
		g_free (pp->output);

	g_free (pp);
}

/**
 * gnome_printer_profile_free_profiles
 * @pl: a profile list returned from gnome_printer_get_profiles()
 *
 * Releases the memory and resources used by a list of printer
 * profiles
 */
void 
gnome_printer_profile_free_profiles (GnomePrinterProfileList *pl)
{
	GnomePrinterProfileList *l;

	g_return_if_fail (pl != NULL);

	for (l = pl; l; l = l->next)
		gnome_printer_profile_free (l->data);

	g_list_free (pl);
}

static GnomePrinter *
gnome_printer_create (const char *file, const char *driver)
{
	GnomePrinter *printer;
	
	printer = gtk_type_new (gnome_printer_get_type ());
#ifndef ENABLE_LIBGPA	
	printer->driver = g_strdup (driver);
#endif
	printer->filename = g_strdup (file);
	return printer;
}



/** 
 * gnome_printer_profile_get_printer:
 * @pp: A printer profile
 * @optional_file: an optional output file name.
 * @optional_command: an optioanl command for printing.
 *
 * Returns a GnomePrinter object based on the @pp GnomePrinterProfile
 * and if one of @optional_file or @optional_command are non-NULL
 * they modify the printer profile's default setting for output.
 */
GnomePrinter *
gnome_printer_profile_get_printer (GnomePrinterProfile *pp,
				   const char *optional_file,
				   const char *optional_command)
{
	GnomePrinter *printer = NULL;
	char *output = NULL;

	g_return_val_if_fail (pp != NULL, printer);

	if (optional_file != NULL && optional_command != NULL){
		g_warning ("Only one of optional_file or optional_command must be set\n");
		return NULL;
	}

	if (optional_file){
		/*
		 * FIXME: do something about funny filenames?  Like one
		 * starting with a '|'.
		 */
		output = g_strdup (optional_file);
	} else if (optional_command) {
		if (strstr (optional_command, "%s")) {
			output = g_strdup_printf ("*%s", optional_command);
		} else {
			output = g_strdup_printf ("|%s", optional_command);
		}
	} else if (strncmp (pp->output, "file", 4) == 0) {
		output = g_strdup (pp->output + 5);
	} else if (strncmp (pp->output, "command", 7) == 0) {
		if (strstr (pp->output + 8, "%s")) {
			output = g_strdup_printf ("*%s", pp->output + 8);
		} else {
			output = g_strdup_printf ("|%s", pp->output + 8);
		}
	}

	if (!output) output = g_strdup ("gnome-printer-output");

	printer = gnome_printer_create (output, pp->driver);

	g_free (output);

	return printer;
}



/* -------------------------------------------------  Access to the struct ----------------------------------*/
/**
 * gnome_printer_profile_get_printer_name:
 * @pp: a printer profile
 *
 * Returns the name for the printer in the @pp PrinterProfile
 */
const char *
gnome_printer_profile_get_printer_name (GnomePrinterProfile *pp)
{
	g_return_val_if_fail (pp != NULL, NULL);

	return pp->name;
}

/**
 * gnome_printer_profile_get_comment:
 * @pp: a printer profile
 *
 * Returns the comment for the printer in the @pp PrinterProfile
 */
const char *
gnome_printer_profile_get_comment (GnomePrinterProfile *pp)
{
	g_return_val_if_fail (pp != NULL, NULL);

	return pp->comment;
}

/**
 * gnome_printer_profile_get_location:
 * @pp: a printer profile
 *
 * Returns the location for the printer in the @pp PrinterProfile
 */
const char *
gnome_printer_profile_get_location (GnomePrinterProfile *pp)
{
	g_return_val_if_fail (pp != NULL, NULL);

	return pp->location;
}

/**
 * gnome_printer_profile_get_mime_type:
 * @pp: a printer profile
 *
 * Returns the mime-type accepted by the printer in the @pp PrinterProfile
 */
const char *
gnome_printer_profile_get_mime_type (GnomePrinterProfile *pp)
{
	g_return_val_if_fail (pp != NULL, NULL);

	return pp->mime_type;
}

/**
 * gnome_printer_profile_get_drive_name:
 * @pp: a printer profile
 *
 * Returns the name for the driver in the @pp PrinterProfile
 */
const char *
gnome_printer_profile_get_driver_name (GnomePrinterProfile *pp)
{
	g_return_val_if_fail (pp != NULL, NULL);

	return pp->driver;
}

/**
 * gnome_printer_profile_get_output:
 * @pp: a printer profile
 *
 * Returns the desired output method forr the printer in the @pp PrinterProfile
 */
const char *
gnome_printer_profile_get_output (GnomePrinterProfile *pp)
{
	g_return_val_if_fail (pp != NULL, NULL);

	return pp->output;
}

