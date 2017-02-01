#ifndef __GNOME_PRINT_ADMIN_PPD_H__
#define __GNOME_PRINT_ADMIN_PPD_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

#include "gpa-structs.h"

#define gpa_ppd_error g_warning
#define gpa_ppd_warning g_warning

#define GPA_PPD_TAG_PSVERSION "PSVersion"
#define GPA_PPD_TAG_PAGE_SIZE "PageSize"
#define GPA_PPD_TAG_MODEL_NAME "ModelName"
#define GPA_PPD_TAG_DEFAULT "*Default"
#define GPA_PPD_TAG_ORDER_DEPENDENCY "*OrderDependency"
#define GPA_PPD_TAG_END "*End"
#define GPA_PPD_TAG_SET_CODE "SetCode"
#define GPA_PPD_TAG_PRE_VAR  "-Var-"
#define GPA_PPD_TAG_POST_VAR "-"

GpaModel * gpa_ppd_add_model (const gchar *full_path);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_H__ */

