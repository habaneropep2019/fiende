#ifndef __GNOME_PRINT_PS_H__
#define __GNOME_PRINT_PS_H__

#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINT_PS              (gnome_print_ps_get_type ())
#define GNOME_PRINT_PS(obj)		 (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_PS, GnomePrintPs))
#define GNOME_PRINT_PS_CLASS(klass)	 (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_PS, GnomePrintPsClass))
#define GNOME_IS_PRINT_PS(obj)           (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_PS))
#define GNOME_IS_PRINT_PS_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_PS))

typedef struct _GnomePrintPs       GnomePrintPs;
typedef struct _GnomePrintPsClass  GnomePrintPsClass;

GtkType gnome_print_ps_get_type (void);

GnomePrintPs *gnome_print_ps_new (GnomePrinter *printer);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_PS_H__ */
