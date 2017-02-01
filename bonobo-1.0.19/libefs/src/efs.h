/* efs.h - Embedded File System (EFS) library (header)

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

#ifndef _EFS_H_
#define _EFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <efs_error.h>

#define EFS_READ          1 
#define EFS_WRITE         2
#define EFS_RDWR          3
#define EFS_CREATE        4
#define EFS_EXCL          8
#define EFS_COMP         16
#define EFS_APPEND       32
#define EFS_FILE         64
#define EFS_DIR         128

#define EFS_PROT       1024

#define EFS_SEEK_SET      0
#define EFS_SEEK_CUR      1
#define EFS_SEEK_END      2

#define EFS_MAXPATHLEN 1024
#define EFS_MAXNAMELEN  256

#define EFS_FDEL        '/'
#define EFS_FILE_ID  "%EFS"

#define EFS_MIME_DIR  "x-directory/normal"
#define EFS_MIME_DATA "application/octet-stream"

typedef gchar *(*EFSPassFunc) (const gchar *filename, gboolean new);

typedef struct _EFSNode EFSNode; /* File or Directory */
typedef EFSNode         EFSFile;
typedef EFSNode          EFSDir; 

typedef struct {
	gchar   efs_id[4];       /* contains "%EFS" */
	guint32 type;            /* a type code */
	gchar   drivername[12];  /* the name of the driver */
	gint32  protected;       /* protection type */
	gchar   epw[40];         /* optional encrypted password */
} EFSHeader;

typedef struct {
	guint32  inode;
	guint8   type;
	guint16  offset;
	guint8   length;
	gchar    name[EFS_MAXNAMELEN];
} EFSDirEntry;

typedef struct {
	guint8  type; /* EFS_FILE, EFS_DIR */
	guint32 size;
} EFSStat;

typedef struct {
	gchar   *drivername; /* name of the driver */
	guint32  size;       /* file size */
	guint32  free;       /* unused bytes */
	guint32  version;
	guint32  namelen;    /* maximum name length */
	gboolean protected;  /* password protected */
	gboolean encrypted;  /* encrypted */
} EFSFSStat;

EFSResult    efs_open          (EFSDir **dir, const char *path, gint flags, 
				gint mode, gchar *passwd);
EFSResult    efs_open_cb       (EFSDir **dir, const char *path, gint flags, 
				gint mode, EFSPassFunc pass_func);
EFSResult    efs_close         (EFSDir *root);
EFSResult    efs_commit        (EFSDir *root);
EFSResult    efs_revert        (EFSDir *root);
EFSResult    efs_fsstat        (EFSDir *root, EFSFSStat *fsstat);

gchar       *efs_strerror      (EFSResult errnum);

EFSResult    efs_file_open     (EFSFile **file, EFSDir *dir, 
				const char *path, gint flags);
EFSResult    efs_file_close    (EFSFile *file);
EFSResult    efs_file_seek     (EFSFile *file, gint32 offset, gint whence,
				guint32 *pos);
EFSResult    efs_file_tell     (EFSFile *file, guint32 *pos);
EFSResult    efs_file_read     (EFSFile *file, gpointer buf, gint32 count,
				guint32 *bytes_read);
EFSResult    efs_file_write    (EFSFile *file, gpointer buf, gint32 count);
EFSResult    efs_file_trunc    (EFSFile *file, guint32 size);
EFSResult    efs_stat          (EFSDir *dir, const char *path, EFSStat *stat);
EFSResult    efs_erase         (EFSDir *dir, const char *path);

EFSResult    efs_dir_open      (EFSDir **dir, EFSDir *parent, 
				const char *path, gint flags);
EFSResult    efs_dir_close     (EFSDir *dir);
EFSResult    efs_dir_seek      (EFSDir *dir, guint32 offset);
EFSResult    efs_dir_tell      (EFSDir *dir, guint32 *pos);
EFSResult    efs_dir_read      (EFSDir *dir, EFSDirEntry *de);
EFSResult    efs_rename        (EFSDir *dir, const char *old_path,
				const char *new_path);

EFSResult    efs_node_open      (EFSNode **node, EFSDir *parent, 
				 const char *path, gint flags, gint type);
EFSResult    efs_node_close     (EFSNode *node);
EFSResult    efs_node_stat      (EFSNode *node, EFSStat *stat);
gboolean     efs_node_equal     (EFSNode *node1, EFSNode *node2);
EFSResult    efs_type_set       (EFSNode *node, guint32 type);
EFSResult    efs_type_get       (EFSNode *node, guint32 *type);

EFSResult    efs_strtype_set    (EFSNode *node, gchar *strtype);
EFSResult    efs_strtype_get    (EFSNode *node, gchar **strtype);
EFSResult    efs_type_lookup    (EFSNode *node, gchar *strtype, guint32 *type);
EFSResult    efs_strtype_lookup (EFSNode *node, guint32 type, gchar **strtype);

EFSResult    efs_lock_create   (const char *lockfile);
EFSResult    efs_lock_check    (const char *lockfile);
EFSResult    efs_lock_remove   (const char *lockfile);

#ifdef __cplusplus
}
#endif

#endif /* _EFS_H_ */







