#ifndef __GNOME_PRINT_ADMIN_PPD_PAPER_H__
#define __GNOME_PRINT_ADMIN_PPD_PAPER_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

#define gpa_ppd_error g_warning

#include "gpa-ppd.h"

gboolean gpa_ppd_add_paper_options (GpaPpdInfo *ppd_info);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_PAPER_H__ */

