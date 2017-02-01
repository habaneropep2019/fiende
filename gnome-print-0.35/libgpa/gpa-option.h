/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GNOME_PRINT_ADMIN_OPTION_H__
#define __GNOME_PRINT_ADMIN_OPTION_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"

#define GPA_IS_OPTION(obj) (obj) /* For now check for NULL only */

extern gboolean   debug_turned_on;

/* Basic "Options" Operations */
GpaOption * gpa_option_new (const gchar *name,
			    const gchar * id,
			    GpaOptions *parent);
GpaOption * gpa_option_copy (GpaOption *option,
			     GpaOptions *parent);
void       gpa_option_free   (GpaOption *option);
gboolean   gpa_option_verify (GpaOption *option,
			      const GpaModel *model);
gboolean   gpa_option_verify_with_settings (GpaOption *option,
					    GpaSettings *settings);


/* Convenience functions */
gboolean  gpa_option_is_selected (const GpaSettings *settings,
				  const GpaOption *option);
GpaOption* gpa_option_get_from_id (GList *option_list, const gchar *id);

/* Values */
gchar* gpa_option_value_dup        (const GpaOption *option, const gchar *key);
void   gpa_option_value_get_double (const GpaOption *option, const gchar *key, double *value);
void   gpa_option_value_get_int    (const GpaOption *option, const gchar *key, int *value);

/* Access the struct from the world */
const gchar * gpa_option_get_name (const GpaOption *option);
const gchar * gpa_option_get_id   (const GpaOption *option);
      gchar * gpa_option_dup_name (const GpaOption *option);

const GpaOptions * gpa_option_get_parent (const GpaOption *option);
const GpaOptions * gpa_option_get_children (const GpaOption *option);




END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_OPTION_H__ */




