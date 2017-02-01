/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GPA_PRINTER_H__
#define __GPA_PRINTER_H__

BEGIN_GNOME_DECLS

#include <gtk/gtkobject.h>
#include "gpa-structs.h"

#define GPA_TYPE_PRINTER             (gpa_printer_get_type ())
#define GPA_PRINTER(obj)             (GTK_CHECK_CAST ((obj), GPA_TYPE_PRINTER, GpaPrinter))
#define GPA_PRINTER_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), GPA_TYPE_PRINTER, GpaClass))
#define GPA_IS_PRINTER(obj)	         (GTK_CHECK_TYPE ((obj), GPA_TYPE_PRINTER))
#define GPA_IS_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), GPA_TYPE_PRINTER))

GtkType gpa_printer_get_type (void);

#define gpa_printer_ref(p)   gtk_object_ref (GTK_OBJECT (p))
#define gpa_printer_unref(p) gtk_object_unref (GTK_OBJECT (p))


extern gboolean   debug_turned_on;

typedef struct _GpaClass        GpaClass;

struct _GpaClass
{
	GtkObjectClass parent_class;
};

typedef enum {
	GNOME_PRINT_PRINTER_ACTIVE,
	GNOME_PRINT_PRINTER_INACTIVE,
	GNOME_PRINT_PRINTER_OFFLINE,
	GNOME_PRINT_PRINTER_NET_FAILURE,
} GpaStatus;

/* Basic "Printer" object operations */
    gboolean gpa_printer_save   (GpaPrinter *printer);
    gboolean gpa_printer_verify (GpaPrinter *printer, gboolean fail);
GpaPrinter * gpa_printer_copy   (GpaPrinter *copy_from);

/* Create a new printer */
GpaPrinter * gpa_printer_new_from_model_info (GpaModelInfo *model_info,
					      const gchar *printer_name);

/* Load Printers */
gboolean gpa_printers_list_load (GList **printers);

/* Status */
GpaStatus    gpa_printer_get_status (GpaPrinter *printer);
const char * gpa_printer_str_status (GpaStatus status);       

/* Settings List */
 GList * gpa_printer_settings_list_get (const GpaPrinter *printer);
    void gpa_printer_settings_list_swap (GpaPrinter *printer_1, GpaPrinter *printer_2);
    gint gpa_printer_settings_list_get_size (const GpaPrinter *printer);

/* Settings */
GpaSettings * gpa_printer_settings_get_first (GpaPrinter *printer);
GpaSettings * gpa_printer_settings_get_selected (const GpaPrinter *printer);
         void gpa_printer_settings_select (const GpaPrinter *printer, GpaSettings *settings);
         void gpa_printer_settings_append (GpaPrinter *printer, GpaSettings *settings);
         void gpa_printer_settings_remove (GpaPrinter *printer,
					   GpaSettings *settings);

/* Default Printer */
GpaPrinter * gpa_printer_get_default (GList *printer_list);
        void gpa_printer_set_default (GList *printer_list, GpaPrinter *printer);
    gboolean gpa_printer_is_default (const GpaPrinter *printer);

/* Access to the struct */
const gchar * gpa_printer_get_name (const GpaPrinter *printer);
const gchar * gpa_printer_get_id (const GpaPrinter *printer);
      gchar * gpa_printer_dup_name (GpaPrinter *printer);
      gchar * gpa_printer_dup_id (const GpaPrinter *printer);

GpaModel * gpa_printer_get_model (GpaPrinter *printer);
   GList * gpa_printer_get_backend_list (const GpaPrinter *printer);

/* Backend info, the drivers use it to query printers*/
const gchar * gpa_printer_backend_info_get (const GpaPrinter *printer,
					    const gchar *backend,
					    const gchar *id);

END_GNOME_DECLS

#endif /* __GPA_PRINTER_H__ */
