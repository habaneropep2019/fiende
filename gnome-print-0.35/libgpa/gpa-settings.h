/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GPA_SETTINGS_H__
#define __GPA_SETTINGS_H__

BEGIN_GNOME_DECLS

#include <glib.h>
#include "gpa-structs.h"

typedef enum {
	GNOME_PRINT_ORIENT_PORTRAIT,
	GNOME_PRINT_ORIENT_LANDSCAPE,
	GNOME_PRINT_ORIENT_REVERSE_PORTRAIT,
	GNOME_PRINT_ORIENT_REVERSE_LANDSCAPE
} GnomePrintOrient;


#define GPA_TYPE_SETTINGS            (gpa_settings_get_type ())
#define GPA_SETTINGS(obj)            (GTK_CHECK_CAST ((obj), GPA_TYPE_SETTINGS, GpaSettings))
#define GPA_SETTINGS_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GPA_TYPE_SETTINGS, GpaSettingsClass))
#define GPA_IS_SETTINGS(obj)	     (GTK_CHECK_TYPE ((obj), GPA_TYPE_SETTINGS))
#define GPA_IS_SETTINGS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GPA_TYPE_SETTINGS))

GtkType gpa_settings_get_type (void);

#define gpa_settings_ref(p)   gtk_object_ref (GTK_OBJECT (p))
#define gpa_settings_unref(p) gtk_object_unref (GTK_OBJECT (p))


extern gboolean   debug_turned_on;

/* Basic "GpaSettings" Object operations either by an individual
 * object, or by a list of them */
GpaSettings * gpa_settings_new_from_model (GpaModel *model,
					   GpaPrinter *printer);

GpaSettings * gpa_settings_copy (GpaSettings *settings);
      GList * gpa_settings_list_copy (GList *settings_list);

     gboolean gpa_settings_verify (GpaSettings *settings, gboolean fail);
     gboolean gpa_settings_list_verify (GList *settings_list, gboolean fail);


/* Selected Settings. Get, select and query  */
GpaSettings * gpa_settings_get_selected (GList *settings_list);
     gboolean gpa_settings_select (GpaSettings *settings_in,
				   GList *settings_list);
     gboolean gpa_settings_is_selected (const GpaSettings *settings);

/* Settings name convenience functions. Used for Copy & Rename Settings */
gboolean gpa_settings_name_taken (GList *settings_list, const gchar *name);
    void gpa_settings_name_replace (GpaSettings *settings, const gchar *name);
gchar *  gpa_settings_get_free_name (GpaSettings *settings,
				     GList *settings_list);


/* Values stuff. This are the actual settings that we modify */
gchar*   gpa_settings_value_dup (GpaSettings *settings,
				 const gchar *key);
gchar*   gpa_settings_value_dup_required (GpaSettings *settings,
					  const gchar *key);
gboolean gpa_settings_value_insert (GpaSettings *settings,
				    const gchar *key,
				    const gchar *value);
gboolean gpa_settings_value_replace (GpaSettings *settings,
				     const gchar *key,
				     const gchar *new_value);
gboolean gpa_settings_value_get_from_option_int (GpaSettings *settings,
						 GpaOption *option,
						 gint *value);
gboolean gpa_settings_value_get_from_option_double (GpaSettings *settings,
						    GpaOption *option,
						    gdouble *value);

/* Access to the struct */
      gchar * gpa_settings_dup_name (GpaSettings *settings);
const gchar * gpa_settings_get_name (GpaSettings *settings);

      gchar * gpa_settings_dup_command (GpaSettings *settings);
const gchar * gpa_settings_get_command (GpaSettings *settings);

const GpaModel* gpa_settings_get_model (GpaSettings *settings);


/* Selected options list */
GList *  gpa_settings_get_selected_options (GpaSettings *settings);
void     gpa_settings_set_selected_options (GpaSettings *settings,
					    GList *list);
gboolean gpa_settings_select_option (GpaSettings *settings,
				     GpaOption *option);

/* This is used to set the command to print to (i.e. "lpr")
 * should go away in the future when we add better spooler
 * manupitaion support */
void gpa_settings_replace_command (GpaSettings *settings,
				   const gchar *command);

/* Query code for the printer drivers */
const gchar * gpa_settings_query_options (GpaSettings *settings,
					  const gchar *options_id);
gboolean gpa_settings_query_options_boolean (GpaSettings *settings,
					     const gchar *options_id);

END_GNOME_DECLS

#endif /* __GPA_SETTINGS_H__ */
