#ifndef __GNOME_PRINT_ADMIN_DEFS_H__
#define __GNOME_PRINT_ADMIN_DEFS_H__

#include "gpa-i18n.h"

#ifndef __GNUC__
  #define __FUNCTION__   ""
#endif
#define debug(section,str) if (debug_turned_on) g_print ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str); 
#define gpa_error g_warning
#define gp_print g_print
#define skip_text(node) if (strcmp ( ((xmlNodePtr)node)->name, "text") == 0) { \
			node = ((xmlNodePtr)node)->next; continue ; };

#define g_fixme(str) g_print ("\n** FIXME ** (%s:%d): %s\n", __FILE__, __LINE__, str);
#define gpa_warning(str) { static gboolean warned = FALSE; \
                          if (!warned) \
		             g_warning ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str); \
			    warned = TRUE; }

		
#define g_implementme() { static gint warned = FALSE; \
					if (!warned) \
						g_warning ("Implement me please. File: %s, Line: %d", __FILE__, __LINE__); \
					warned = TRUE; }

#define GPA_GLADE_FILE "gpa.glade"

#ifdef __cplusplus
#ifndef BEGIN_GNOME_DECLS
#define BEGIN_GNOME_DECLS extern "C" {
#endif
#ifndef END_GNOME_DECLS
#define END_GNOME_DECLS }
#endif
#endif

#ifndef __cplusplus
#define BEGIN_GNOME_DECLS
#define END_GNOME_DECLS
#endif

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#endif /* __GNOME_PRINT_ADMIN_DEFS_H__ */
