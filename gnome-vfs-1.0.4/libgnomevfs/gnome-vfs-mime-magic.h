#ifndef __GNOME_VFS_MIME_MAGIC_H__
#define __GNOME_VFS_MIME_MAGIC_H__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/* Used internally by the magic code, exposes the parsing to users */
typedef enum {
	T_END, /* end of array */
	T_BYTE,
	T_SHORT,
	T_LONG,
	T_STR,
	T_DATE, 
	T_BESHORT,
	T_BELONG,
	T_BEDATE,
	T_LESHORT,
	T_LELONG,
	T_LEDATE
} GnomeMagicType;

typedef struct _GnomeMagicEntry {
	GnomeMagicType type;
	guint16 range_start, range_end;
	
	guint16 pattern_length;
	gboolean use_mask;
	char pattern [48];
	char mask [48];
	
	char mimetype[48];
} GnomeMagicEntry;

GnomeMagicEntry *gnome_vfs_mime_magic_parse (const gchar *filename, gint *nents);

/* testing calls */
GnomeMagicEntry *gnome_vfs_mime_test_get_magic_table (const char *table_path);
void gnome_vfs_mime_dump_magic_table (void);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif
