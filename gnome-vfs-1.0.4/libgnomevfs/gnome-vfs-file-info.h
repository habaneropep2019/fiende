/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-file-info.h - Handling of file information for the GNOME
   Virtual File System.

   Copyright (C) 1999,2001 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@comm2000.it> 
           Seth Nickell <snickell@stanford.edu>
*/

#ifndef GNOME_VFS_FILE_INFO_H
#define GNOME_VFS_FILE_INFO_H

#include <libgnomevfs/gnome-vfs-file-size.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <sys/stat.h>
#include <sys/types.h>

/* File flags.  */
typedef enum {
	GNOME_VFS_FILE_FLAGS_NONE = 0,
	/* Whether the file is a symlink.  */
	GNOME_VFS_FILE_FLAGS_SYMLINK = 1 << 0,
	/* Whether the file is on a local file system.  */
	GNOME_VFS_FILE_FLAGS_LOCAL = 1 << 1,
} GnomeVFSFileFlags;

/* Flags indicating what fields in a GnomeVFSFileInfo struct are valid. 
   Name is always assumed valid (how else would you have gotten a
   FileInfo struct otherwise?)
 */

/* The file type.  */
typedef enum {
	GNOME_VFS_FILE_TYPE_UNKNOWN,
	GNOME_VFS_FILE_TYPE_REGULAR,
	GNOME_VFS_FILE_TYPE_DIRECTORY,
	GNOME_VFS_FILE_TYPE_FIFO,
	GNOME_VFS_FILE_TYPE_SOCKET,
	GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE,
	GNOME_VFS_FILE_TYPE_BLOCK_DEVICE,
	GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK
} GnomeVFSFileType;

typedef enum {
	GNOME_VFS_FILE_INFO_FIELDS_NONE = 0,
	GNOME_VFS_FILE_INFO_FIELDS_TYPE = 1 << 0,
	GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS = 1 << 1,
	GNOME_VFS_FILE_INFO_FIELDS_FLAGS = 1 << 2,
	GNOME_VFS_FILE_INFO_FIELDS_DEVICE = 1 << 3,
	GNOME_VFS_FILE_INFO_FIELDS_INODE = 1 << 4,
	GNOME_VFS_FILE_INFO_FIELDS_LINK_COUNT = 1 << 5,
	GNOME_VFS_FILE_INFO_FIELDS_SIZE = 1 << 6,
	GNOME_VFS_FILE_INFO_FIELDS_BLOCK_COUNT = 1 << 7,
	GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE = 1 << 8,
	GNOME_VFS_FILE_INFO_FIELDS_ATIME = 1 << 9,
	GNOME_VFS_FILE_INFO_FIELDS_MTIME = 1 << 10,
	GNOME_VFS_FILE_INFO_FIELDS_CTIME = 1 << 11,
	GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME = 1 << 12,
	GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE = 1 << 13
} GnomeVFSFileInfoFields;

/* File permissions.  These are the same as the Unix ones, but we wrap them
   into a nicer VFS-like enum.  */
/* FIXME: It's silly to use the symbolic constants for POSIX here.
 * This is supposed to be a virtual file system, so it makes no
 * sense to use the values of the POSIX-required constants on the
 * particular machine this code is compiled on. These should all be changed
 * to use numeric constants like GNOME_VFS_PERM_STICKY does now. However,
 * be careful in making such a change, since some existing code might
 * wrongly assume these equivalencies.
 */
typedef enum {
	GNOME_VFS_PERM_SUID = S_ISUID,
	GNOME_VFS_PERM_SGID = S_ISGID,	
	GNOME_VFS_PERM_STICKY = 01000,	/* S_ISVTX not defined on all systems */
	GNOME_VFS_PERM_USER_READ = S_IRUSR,
	GNOME_VFS_PERM_USER_WRITE = S_IWUSR,
	GNOME_VFS_PERM_USER_EXEC = S_IXUSR,
	GNOME_VFS_PERM_USER_ALL = S_IRUSR | S_IWUSR | S_IXUSR,
	GNOME_VFS_PERM_GROUP_READ = S_IRGRP,
	GNOME_VFS_PERM_GROUP_WRITE = S_IWGRP,
	GNOME_VFS_PERM_GROUP_EXEC = S_IXGRP,
	GNOME_VFS_PERM_GROUP_ALL = S_IRGRP | S_IWGRP | S_IXGRP,
	GNOME_VFS_PERM_OTHER_READ = S_IROTH,
	GNOME_VFS_PERM_OTHER_WRITE = S_IWOTH,
	GNOME_VFS_PERM_OTHER_EXEC = S_IXOTH,
	GNOME_VFS_PERM_OTHER_ALL = S_IROTH | S_IWOTH | S_IXOTH
} GnomeVFSFilePermissions;


typedef struct {
	/* Base name of the file (no path).  */
	gchar *name;

	/* Fields which are actually valid in this strcture. */
	GnomeVFSFileInfoFields valid_fields;

	/* File type (i.e. regular, directory, block device...).  */
	GnomeVFSFileType type;

	/* File permissions.  */
	GnomeVFSFilePermissions permissions;

	/* Flags for this file.  */
	GnomeVFSFileFlags flags;

	/* This is only valid if `is_local' is TRUE (see below).  */
	dev_t device;
	ino_t inode;

	/* Link count.  */
	guint link_count;

	/* UID, GID.  */
	guint uid;
	guint gid;

	/* Size in bytes.  */
	GnomeVFSFileSize size;

	/* Size measured in units of 512-byte blocks.  */
	GnomeVFSFileSize block_count;

	/* Optimal buffer size for reading/writing the file.  */
	guint io_block_size;

	/* Access, modification and change times.  */
	time_t atime;
	time_t mtime;
	time_t ctime;

	/* If the file is a symlink (see `flags'), this specifies the file the
           link points to.  */
	gchar *symlink_name;

	/* MIME type.  */
	gchar *mime_type;

	guint refcount;
} GnomeVFSFileInfo;

typedef enum {
	GNOME_VFS_FILE_INFO_DEFAULT = 0, /* FIXME bugzilla.eazel.com 1203: name sucks */
	GNOME_VFS_FILE_INFO_GET_MIME_TYPE = 1 << 0,
	GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE = 1 << 1,
	GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE = 1 << 2,
	GNOME_VFS_FILE_INFO_FOLLOW_LINKS = 1 << 3
} GnomeVFSFileInfoOptions;

typedef enum {
	GNOME_VFS_SET_FILE_INFO_NONE = 0,
	GNOME_VFS_SET_FILE_INFO_NAME = 1 << 0,
	GNOME_VFS_SET_FILE_INFO_PERMISSIONS = 1 << 1,
	GNOME_VFS_SET_FILE_INFO_OWNER = 1 << 2,
	GNOME_VFS_SET_FILE_INFO_TIME = 1 << 3
} GnomeVFSSetFileInfoMask;

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSResult result;
	GnomeVFSFileInfo *file_info;
} GnomeVFSGetFileInfoResult;

#define GNOME_VFS_FILE_INFO_SYMLINK(info)		\
	((info)->flags & GNOME_VFS_FILE_FLAGS_SYMLINK)

#define GNOME_VFS_FILE_INFO_SET_SYMLINK(info, value)			\
	(value ? ((info)->flags |= GNOME_VFS_FILE_FLAGS_SYMLINK)	\
	       : ((info)->flags &= ~GNOME_VFS_FILE_FLAGS_SYMLINK))

#define GNOME_VFS_FILE_INFO_LOCAL(info)			\
	((info)->flags & GNOME_VFS_FILE_FLAGS_LOCAL)

#define GNOME_VFS_FILE_INFO_SET_LOCAL(info, value)			\
	(value ? ((info)->flags |= GNOME_VFS_FILE_FLAGS_LOCAL)		\
	       : ((info)->flags &= ~GNOME_VFS_FILE_FLAGS_LOCAL))



#define GNOME_VFS_FILE_INFO_SUID(info)			\
	((info)->permissions & GNOME_VFS_PERM_SUID)

#define GNOME_VFS_FILE_INFO_SGID(info)			\
	((info)->permissions & GNOME_VFS_PERM_SGID)

#define GNOME_VFS_FILE_INFO_STICKY(info)		\
	((info)->permissions & GNOME_VFS_PERM_STICKY)


#define GNOME_VFS_FILE_INFO_SET_SUID(info, value)		\
	(value ? ((info)->permissions |= GNOME_VFS_PERM_SUID)	\
	       : ((info)->permissions &= ~GNOME_VFS_PERM_SUID))

#define GNOME_VFS_FILE_INFO_SET_SGID(info, value)		\
	(value ? ((info)->permissions |= GNOME_VFS_PERM_SGID)	\
	       : ((info)->permissions &= ~GNOME_VFS_PERM_SGID))

#define GNOME_VFS_FILE_INFO_SET_STICKY(info, value)			\
	(value ? ((info)->permissions |= GNOME_VFS_PERM_STICKY)		\
	       : ((info)->permissions &= ~GNOME_VFS_PERM_STICKY))


GnomeVFSFileInfo *
		 gnome_vfs_file_info_new 	(void);
void 		 gnome_vfs_file_info_unref   	(GnomeVFSFileInfo *info);
void 		 gnome_vfs_file_info_ref     	(GnomeVFSFileInfo *info);
void 		 gnome_vfs_file_info_clear     	(GnomeVFSFileInfo *info);
const gchar	*gnome_vfs_file_info_get_mime_type
						(GnomeVFSFileInfo *info);

void		 gnome_vfs_file_info_copy 	(GnomeVFSFileInfo *dest,
						 const GnomeVFSFileInfo *src);

GnomeVFSFileInfo *
		 gnome_vfs_file_info_dup 	(const GnomeVFSFileInfo *orig);


gboolean	 gnome_vfs_file_info_matches	(const GnomeVFSFileInfo *a,
						 const GnomeVFSFileInfo *b);

GList           *gnome_vfs_file_info_list_ref   (GList *list);
GList           *gnome_vfs_file_info_list_unref (GList *list);
GList           *gnome_vfs_file_info_list_copy  (GList *list);
void             gnome_vfs_file_info_list_free  (GList *list);

#endif /* GNOME_VFS_FILE_INFO_H */
