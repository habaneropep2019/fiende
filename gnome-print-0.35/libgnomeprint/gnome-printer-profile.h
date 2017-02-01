#ifndef __GNOME_PRINTER_PROFILE_H__
#define __GNOME_PRINTER_PROFILE_H__

BEGIN_GNOME_DECLS

struct _GnomePrinterProfile;
typedef struct _GnomePrinterProfile GnomePrinterProfile;

typedef GList GnomePrinterProfileList;

/* Loading of printing profiles */
GnomePrinterProfileList *gnome_printer_get_profiles (void);

void gnome_printer_profile_free_profiles (GnomePrinterProfileList *pl);

/* Query routines */
const char             *gnome_printer_profile_get_printer_name(GnomePrinterProfile *pp);
const char             *gnome_printer_profile_get_comment     (GnomePrinterProfile *pp);
const char             *gnome_printer_profile_get_location    (GnomePrinterProfile *pp);
const char             *gnome_printer_profile_get_mime_type   (GnomePrinterProfile *pp);
const char             *gnome_printer_profile_get_driver_name (GnomePrinterProfile *pp);
const char             *gnome_printer_profile_get_output      (GnomePrinterProfile *pp);

GnomePrinter           *gnome_printer_profile_get_printer     (GnomePrinterProfile *pp,
							       const char *optional_file,
							       const char *optional_command);

END_GNOME_DECLS

#endif /* __GNOME_PRINTER_PROFILE_H__ */
