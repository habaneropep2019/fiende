#ifndef __GNOME_PRINT_META_H__
#define __GNOME_PRINT_META_H__

#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINT_META		 (gnome_print_meta_get_type ())
#define GNOME_PRINT_META(obj)		 (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_META, GnomePrintMeta))
#define GNOME_PRINT_META_CLASS(klass)	 (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_META, GnomePrintMetaClass))
#define GNOME_IS_PRINT_META(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_META))
#define GNOME_IS_PRINT_META_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_META))

typedef struct _GnomePrintMeta GnomePrintMeta;
typedef struct _GnomePrintMetaClass GnomePrintMetaClass;

GtkType         gnome_print_meta_get_type (void);
GnomePrintMeta *gnome_print_meta_new      (void);
GnomePrintMeta *gnome_print_meta_new_from (const void *data);

/* Returns the value in GnomePrinterMeta structure, shared with the object */
int             gnome_print_meta_access_buffer (GnomePrintMeta *meta,
						void **buffer, int *buflen);
/* Duplicates the metadata buffer and returns a copy of it */
int             gnome_print_meta_get_copy      (GnomePrintMeta *meta,
						void **buffer, int *buflen);

/* Returns the number of pages generated */
int             gnome_print_meta_pages         (const GnomePrintMeta *meta);

/*
 * Rendering a metafile into a GnomePrintContext
 */

gboolean gnome_print_meta_render_from_object      (GnomePrintContext *destination,
						   const GnomePrintMeta *source);
gboolean gnome_print_meta_render                  (GnomePrintContext *destination,
						   const void *meta_stream);
gboolean gnome_print_meta_render_page             (GnomePrintContext *destination,
						   const void *meta_stream, int page);
gboolean gnome_print_meta_render_from_object_page (GnomePrintContext *destination,
						   const GnomePrintMeta *source, int page);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_META_H__ */
