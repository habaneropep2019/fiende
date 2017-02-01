/* efs_internal.h - internal EFS data structures

   Copyright (C) 2000 Maurer IT Systemlösungen KEG

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Dietmar Maurer <dietmar@maurer-it.com>

*/

#ifndef _EFS_INTERNAL_H_
#define _EFS_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>
#include <glib.h>
#include <zlib.h>

#include "efs.h"

#define EFS_ROOT        256
#define EFS_ERASE       512

#define Z_BUFSIZE     16384

typedef struct _EFSDriver EFSDriver;

typedef struct {
	z_stream  stream;
	gint      err;       /* error code for last stream operation */
	gint      eof;       /* set if end of input file */
	gpointer  inbuf;     /* input buffer */
	gpointer  outbuf;    /* output buffer */
	guint32   crc;       /* crc32 of uncompressed data */
} GZStream;

typedef struct {
	EFSDriver  *driver;
	EFSDir     *root;
	guint32     mode;
	guint32     type;
	gchar      *lockname;
	GHashTable *s2i_hash; /* used by the type system */
	GHashTable *i2s_hash; /* used by the type system */
	EFSFile    *typefd;   /* used by the type system */
	guint32     tc;       /* number of user defined types */
} EFS;

struct _EFSNode {
	EFS      *efs;
	guint32   mode;
	guint32   pos;
	GZStream *gzstream;
};

typedef struct {
	EFSResult (*open)          (EFSDir **dir, EFSDriver *driver, 
				    const char *path, gint flags, 
				    gchar *passwd);
	EFSResult (*create)        (EFSDir **dir, EFSDriver *driver, 
				    const char *path, gint flags, gint mode, 
				    gchar *passwd);
	EFSResult (*close)         (EFSDir *root);
	EFSResult (*commit)        (EFSDir *root);
	EFSResult (*revert)        (EFSDir *root);
	EFSResult (*fsstat)        (EFSDir *root, EFSFSStat *fsstat);
} EFSSuperOps;

typedef struct {
	EFSResult (*node_open)     (EFSNode **node, EFSDir *dir, 
				    const char *path, gint flags, gint type);
	EFSResult (*node_close)    (EFSNode *node);
	EFSResult (*file_seek)     (EFSFile *file, gint32 offset, gint whence,
				    guint32 *pos);
	EFSResult (*dir_seek)      (EFSDir *dir, guint32 offset);
	EFSResult (*file_read)     (EFSFile *file, gpointer buf, gint32 count,
				    gint32 *bytes_read);
	EFSResult (*dir_read)      (EFSDir *dir, EFSDirEntry *de);
	EFSResult (*file_write)    (EFSFile *file, gpointer buf, gint32 count);
	EFSResult (*file_trunc)    (EFSFile *file, guint32 size);
	EFSResult (*type_set)      (EFSNode *node, guint32 type);
	EFSResult (*type_get)      (EFSNode *node, guint32 *type);
	EFSResult (*node_stat)     (EFSNode *node, EFSStat *stat);
	EFSResult (*erase)         (EFSDir *dir, const char *path);
	EFSResult (*rename)        (EFSDir *dir, const char *old_path, 
				    const char *new_path);
	gboolean  (*node_equal)    (EFSNode *node1, EFSNode *node2);
} EFSFileOps;

struct _EFSDriver {
	gchar       *drivername;
	gboolean     encrypted;
	gchar       *description;
	EFSSuperOps *sops;
	EFSFileOps  *fops;
};

EFSDriver*   efs_find_driver    (gchar *drivername);
gboolean     efs_passwd_compare (EFSHeader *head, gchar *passwd);
void         efs_passwd_set     (EFSHeader *head, gchar *passwd);

#ifdef __cplusplus
}
#endif

#endif /* _EFS_INTERNAL_H_ */


