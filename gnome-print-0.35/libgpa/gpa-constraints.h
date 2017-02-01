#ifndef __GNOME_PRINT_ADMIN_CONSTRAINTS_H__
#define __GNOME_PRINT_ADMIN_CONSTRAINTS_H__

BEGIN_GNOME_DECLS

#include <libgpa/gpa-option.h>

extern gboolean   debug_turned_on;

typedef enum {
	GPA_CONSTRAINT_TYPE_SINGLE,
	GPA_CONSTRAINT_TYPE_DOUBLE,
	GPA_CONSTRAINT_TYPE_ATLEAST,
	GPA_CONSTRAINT_TYPE_ERROR
} GpaConstraintType;

typedef struct _GpaConstraint   GpaConstraint;

GpaConstraint * gpa_constraint_new (const gchar *offending_path,
									const gchar *constrained_path,
									GpaConstraintType type);

GList *   gpa_constraints_copy (GList *source_list);

gboolean gpa_constraints_verify (GpaOption *option,
						   const GpaModel *model);
gboolean gpa_constraints_verify_with_settings (GpaOption *option,
									  GpaSettings *settings);


gboolean  gpa_constraints_double_links_model (GpaModel *model);

gboolean gpa_constraints_free (GList *constraints_list);

GpaOption * gpa_constraints_is_in_conflict (GpaSettings *settings,
										   GpaOption *option);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_CONSTRAINTS_H__ */

