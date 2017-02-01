#ifndef __GNOME_PRINTER_DIALOG_H__
#define __GNOME_PRINTER_DIALOG_H__

#include <libgnomeprint/gnome-printer.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINTER_WIDGET	     (gnome_printer_widget_get_type ())
#define GNOME_PRINTER_WIDGET(obj)	     (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINTER_WIDGET, GnomePrinterWidget))
#define GNOME_PRINTER_WIDGET_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINTER_WIDGET, GnomePrinterWidgetClass))
#define GNOME_IS_PRINTER_WIDGET(obj)	     (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINTER_WIDGET))
#define GNOME_IS_PRINTER_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINTER_WIDGET))

typedef struct _GnomePrinterWidget GnomePrinterWidget;
typedef struct _GnomePrinterWidgetClass GnomePrinterWidgetClass;

GtkType       gnome_printer_widget_get_type    (void);
GtkWidget    *gnome_printer_widget_new         (void);

void gnome_printer_widget_bind_editable_enters (GnomePrinterWidget * gpw, GnomeDialog * dialog);
void gnome_printer_widget_bind_accel_group (GnomePrinterWidget * gpw, GtkWindow * window);

GnomePrinter *gnome_printer_widget_get_printer (GnomePrinterWidget *widget);

#define GNOME_TYPE_PRINTER_DIALOG	     (gnome_printer_dialog_get_type ())
#define GNOME_PRINTER_DIALOG(obj)	     (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINTER_DIALOG, GnomePrinterDialog))
#define GNOME_PRINTER_DIALOG_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINTER_DIALOG, GnomePrinterDialogClass))
#define GNOME_IS_PRINTER_DIALOG(obj)	     (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINTER_DIALOG))
#define GNOME_IS_PRINTER_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINTER_DIALOG))

typedef struct _GnomePrinterDialog GnomePrinterDialog;
typedef struct _GnomePrinterDialogClass GnomePrinterDialogClass;

GtkType       gnome_printer_dialog_get_type    (void);
GnomePrinter *gnome_printer_dialog_new_modal   (void);
GtkWidget    *gnome_printer_dialog_new         (void);
GnomePrinter *gnome_printer_dialog_get_printer (GnomePrinterDialog *dialog);

END_GNOME_DECLS

#endif /* __GNOME_PRINTER_DIALOG_H__ */
