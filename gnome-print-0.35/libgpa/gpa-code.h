#ifndef __GNOME_PRINT_ADMIN_CODE_H__
#define __GNOME_PRINT_ADMIN_CODE_H__

BEGIN_GNOME_DECLS

#define GPA_IS_CODE_FRAGMENT(obj) (obj) /* For now check for NULL only */

#include "gpa-structs.h"

extern gboolean   debug_turned_on;

typedef struct _GpaCodeFragment GpaCodeFragment;

GpaCodeFragment * gpa_code_fragment_new (void);

/* For testing purposes */
void gpa_code_emit_all (GpaSettings *settings);

gboolean gpa_code_fragments_verify_with_options (GpaOptions *options);
gboolean gpa_code_fragments_verify_with_printer (GList *list_,
									    GpaPrinter *printer);

GList * gpa_code_fragments_copy (GList *list_);
gboolean gpa_code_fragments_free (GList *list_);


END_GNOME_DECLS

#if 0
  /* these functions are from gnome-print-encode.h, and libgnomeprint has gtk dependencies -NickM */
  #define GPC_MAX_CHARS_PER_LINE 80 /* Needs to be a multiple of 2 ! */
  #define gnome_print_decode_hex_wcs(size) ((int)((size/2)+((size/2)/GPC_MAX_CHARS_PER_LINE)+4))
#endif


#endif /* __GNOME_PRINT_ADMIN_CODE_H__ */



