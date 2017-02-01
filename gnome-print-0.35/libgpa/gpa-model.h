#ifndef __GNOME_PRINT_ADMIN_MODEL_H__
#define __GNOME_PRINT_ADMIN_MODEL_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"
#include <gtk/gtkobject.h>
#include <gtk/gtktypeutils.h>

#define GPA_TYPE_MODEL 	    	     (gpa_model_get_type ())
#define GPA_MODEL(obj)             (GTK_CHECK_CAST ((obj), GPA_TYPE_MODEL, GpaModel))
#define GPA_MODEL_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), GPA_TYPE_MODEL, GpaModelClass))
#define GPA_IS_MODEL(obj)	     (GTK_CHECK_TYPE ((obj), GPA_TYPE_MODEL))
#define GPA_IS_MODEL_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), GPA_TYPE_MODEL))

GtkType gpa_model_get_type (void);

#define gpa_model_ref(p) gtk_object_ref (GTK_OBJECT (p))
#define gpa_model_unref(p) gtk_object_unref (GTK_OBJECT (p))

typedef struct _GpaModelClass        GpaModelClass;

struct _GpaModelClass
{
	GtkObjectClass parent_class;
};

extern gboolean   debug_turned_on;

/* Basic "GpaModel" object operations */

gboolean gpa_model_verify (const GpaModel *model);
gboolean gpa_model_verify_with_settings (const GpaModel *model,
								 GpaSettings *settings);
gboolean gpa_model_save (GpaModel *model);

/* Use this function to load models */
GpaModel * gpa_model_get_from_id (const gchar *model_id);

/* Use this function to query the models properties */
const gchar*  gpa_model_get_info (const GpaModel *model, const gchar *id);

/* Access to the struct */
          GList * gpa_model_get_options_list (const GpaModel *model);
     GpaOptions * gpa_model_get_options_from_id (const GpaModel *model, const gchar *id);

const GpaVendor * gpa_model_get_vendor (const GpaModel *model);
    const gchar * gpa_model_get_name (const GpaModel *model);
          gchar * gpa_model_dup_name (const GpaModel *model);
    const gchar * gpa_model_get_id (const GpaModel *model);

/* Utility functions */
gboolean gpa_model_add_missing_default_settings_and_values (GpaModel *model);


END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_MODEL_H__ */
