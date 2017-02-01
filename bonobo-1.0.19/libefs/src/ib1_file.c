/* ib1_file.c: INode Dased Driver 1 FileOps

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

#include <string.h>
#include "ib1_driver.h"

static EFSResult  ib1_node_open  (EFSNode **node, EFSDir *dir, 
				  const char *path, gint flags, gint type);
static EFSResult  ib1_node_close (EFSNode *node);
static EFSResult  ib1_file_seek  (EFSFile *file, gint32 offset, gint whence,
				  guint32 *pos);
static EFSResult  ib1_dir_seek   (EFSDir *dir, guint32 offset);
static EFSResult  ib1_file_read  (EFSFile *file, gpointer buf, gint32 count,
				  gint32 *bytes_read);
static EFSResult  ib1_dir_read   (EFSDir *dir, EFSDirEntry *de);
static EFSResult  ib1_file_write (EFSFile *file, gpointer buf, gint32 count);
static EFSResult  ib1_file_trunc (EFSFile *file, guint32 size);
static EFSResult  ib1_type_set   (EFSNode *node, guint32 type);
static EFSResult  ib1_type_get   (EFSNode *node, guint32 *type);
static EFSResult  ib1_node_stat  (EFSNode *node, EFSStat *stat);
static EFSResult  ib1_erase      (EFSDir *dir, const char *path); 
static EFSResult  ib1_rename     (EFSDir *dir, const char *old_path, 
				  const char *new_path);
static gboolean   ib1_node_equal (EFSNode *node1, EFSNode *node2);

EFSFileOps file_ops_ib1 = {
	ib1_node_open,
	ib1_node_close,
	ib1_file_seek,
	ib1_dir_seek,
	ib1_file_read,
	ib1_dir_read,
	ib1_file_write,
	ib1_file_trunc,
	ib1_type_set,
	ib1_type_get,
	ib1_node_stat,
	ib1_erase,
	ib1_rename,
	ib1_node_equal
};

static IB1DirEntry   root_de = { GUINT32_TO_LE(IB1_ROOT_INODE), 
				 0, 0, EFS_DIR, {0}};
static IB1CacheEntry ce_root;

/**
 * ib1_check_entry:
 * @de: pointer to the directory entry
 * @maxlen: maximum allowed length
 *
 * Description: check if the specified directory entry is OK.
 *
 * Returns: TRUE if the entry is OK, FALSE if not.
 */


static gboolean
ib1_check_entry (IB1DirEntry *de, gint maxlen)
{
	gint i;

	if (!de->rec_len) return FALSE;
	if (GUINT16_FROM_LE (de->rec_len) > maxlen) return FALSE;
	if (!de->inode) return TRUE;
	if (de->name_len > (maxlen-8)) return FALSE;
	if (!((de->type&EFS_DIR) || (de->type&EFS_FILE))) return FALSE;
	for (i = 0; i< de->name_len; i++) if (!de->name[i]) return FALSE;

	return TRUE;
}

/**
 * ib1_inode_info:
 * @efs: pointer to the #EFS structure
 * @dir_inode: the inode containing the directory
 *
 * Description: get information about the size of the directory
 *
 * Returns: the number of blocks
 */

static EFSResult
ib1_inode_info (IB1EFS *efs, guint32 dir_inode, guint32 *blocks, guint32 *size)
{
	IB1CacheEntry *ce;
	IB1INode *node;

	if (!(ce = ib1_inode_map (efs, dir_inode, FALSE))) return EFS_ERR_INT;
	node = NODEP(ce, dir_inode);
	if ((node->type&EFS_DIR)&&(node->size)) return EFS_ERR_INT;

	if (blocks) *blocks = GUINT32_FROM_LE(node->blocks);
	if (size) *size = GUINT32_FROM_LE(node->size);

	return EFS_ERR_OK;
}

/**
 * ib1_dir_empty:
 * @efs: pointer to the #EFS structure
 * @dir_inode: the inode containing the directory
 *
 * Description: check if the specified directory @inode is empty
 *
 * Returns: TRUE if the directory is empty, FALSE if not
 */

static gboolean
ib1_dir_empty (IB1EFS *efs, guint32 dir_inode)
{
	IB1CacheEntry *ce;
	IB1DirEntry *de;
	gint blocks, cb, ind;

	if (ib1_inode_info (efs, dir_inode, &blocks, NULL)) return FALSE;

	cb = 0;
	while (cb < blocks) {
		ce = ib1_inode_bmap (efs, dir_inode, cb, FALSE);
		if (!ce) return FALSE;

		ind = 0;
		while (ind < 508) {
			de = (IB1DirEntry *)(ce->data+ind);
			if (!ib1_check_entry (de, 508-ind)) return FALSE;
			if (de->inode) return FALSE;
			ind += GUINT16_FROM_LE(de->rec_len);
		}
		cb++;
	}		
	return TRUE;
}

/**
 * ib1_delete_entry:
 * @efs: pointer to the #EFS structure
 * @dir_inode: the inode containing the directory
 * @block: block offset
 * @ind: offset within block
 *
 * Description: deletes an directory entry and joins the remaining entries.
 *
 * Returns: 
 */

static IB1CacheEntry*
ib1_delete_entry (IB1EFS *efs, guint32 dir_inode, gint block, gint ind)
{
	IB1CacheEntry *ce;
	IB1DirEntry *de, *de1;
	gint next_ind;

	if (!(ce = ib1_inode_bmap (efs, dir_inode, block, TRUE))) return NULL;
	de = (IB1DirEntry *)(ce->data+ind);

	ib1_inode_erase (efs, GUINT32_FROM_LE(de->inode));

	de->inode = 0;
	de->type = 0;
	de->name_len = 0;
	ib1_cache_touch (ce, TRUE);

	ind = 0;
	while (ind < 508) {
		de = (IB1DirEntry *)(ce->data+ind);

		if (!de->rec_len) return NULL;
		if ((next_ind=ind+GUINT16_FROM_LE(de->rec_len)) == 508) break;
		if (next_ind>508) return NULL;

		de1 = (IB1DirEntry *)(ce->data+
				      ind+GUINT16_FROM_LE(de->rec_len));
		if (!de->inode && !de1->inode) {
			de->rec_len = GUINT16_TO_LE (
				GUINT16_FROM_LE(de->rec_len) + 
				GUINT16_FROM_LE(de1->rec_len));
		} else ind = next_ind;
	}

	return ce;
}

static IB1CacheEntry*
ib1_add_entry (IB1EFS *efs, guint32 dir_inode, gint block, gint ind, 
	       const gchar *name, IB1DirEntry **res_de)
{
	IB1CacheEntry *ce;
	IB1DirEntry *de, *de1;
	gint len = strlen (name);

	*res_de = NULL;
	if (!(ce = ib1_inode_bmap (efs, dir_inode, block, TRUE))) return NULL;
	de = (IB1DirEntry *)(ce->data+ind);
	if (de->inode) return NULL;

	de->inode = 0;
	de->type = 0;
	de->name_len = len;
	strncpy (de->name, name, len);

	if ((GUINT16_FROM_LE(de->rec_len)-8-len)>16) {
		gint rl = 8+((len+3)/4)*4;
		gint rest = GUINT16_FROM_LE(de->rec_len)-rl;
		de->rec_len = GUINT16_TO_LE(rl); 
		de1 = (IB1DirEntry *)(ce->data+ind+rl);
		de1->inode = 0;
		de1->rec_len = GUINT16_TO_LE(rest);
		de1->name_len = 0;
		de1->type = 0;
	}
		
	ib1_cache_touch (ce, TRUE);

	*res_de = de;
	return ce;
}


/**
 * ib1_find_entry:
 *
 */
	
static IB1CacheEntry*
ib1_find_entry (IB1EFS *efs, guint32 dir_inode, const gchar *name, 
		IB1DirEntry **res_de, gint flags, gint *not_empty)
{
	IB1CacheEntry *ce;
	IB1DirEntry *de;
	gint blocks, cb, ind;
	gint len = strlen (name);
	guint32 inode;
	gboolean del = (flags&EFS_ERASE);
	gboolean create = (flags&EFS_CREATE);

	*res_de = NULL;
	if (not_empty) *not_empty = 0;

	if (len > IB1_NAME_LEN) return NULL;
	if (ib1_inode_info (efs, dir_inode, &blocks, NULL)) return NULL;

	cb = 0;
	while ((cb < blocks) || (create && (cb <= blocks))) {
		ce = ib1_inode_bmap (efs, dir_inode, cb, (cb == blocks));
		if (!ce) return NULL;

		if (cb == blocks) { /* append new block */
			de = (IB1DirEntry *)ce->data;
			de->inode = 0;
			de->rec_len = GUINT16_TO_LE(508);
			de->name_len = 0;
			de->type = 0;
			ib1_cache_touch (ce, TRUE);
		}

		ind = 0;
		while (ind < 508) {
			de = (IB1DirEntry *)(ce->data+ind);
			if (!ib1_check_entry (de, 508-ind)) return NULL;
			inode = GINT32_FROM_LE(de->inode);
			if (inode && (len == de->name_len) &&
			    !strncmp(de->name, name, de->name_len)) {
				/* fixme: return EFS_ERR_EXISTS */
				if (create && (flags&EFS_EXCL)) return NULL;
				if (del && not_empty && 
				    (de->type == EFS_DIR) &&
				    !ib1_dir_empty(efs, inode)) {
					*not_empty = TRUE;
					return NULL;
				}
				if (del) return ib1_delete_entry 
						   (efs, dir_inode, cb, ind);
				*res_de = de;
				return ce;
			}

			if (!inode && create && 
			    ((GUINT16_FROM_LE(de->rec_len)-8) >= len)) {
				return ib1_add_entry (efs, dir_inode, cb, ind, 
						      name, res_de);
			}
			if (!GUINT16_FROM_LE(de->rec_len)) break;

			ind += GUINT16_FROM_LE(de->rec_len);
		}
		cb++;
	}		
	return NULL;
}

/*
 * ib1_namei:
 *
 * This is the basic name resolution function, turning a pathname
 * into a direntry. If flag EFS_CREATE is set then a new entry is created if
 * it does not already exist (with inode number 0). 
 */

static IB1CacheEntry *
ib1_namei (IB1EFS *efs, guint32 dir_inode, const char *path, 
	   IB1DirEntry **res_de, gint flags, gint *not_empty)
{
	IB1CacheEntry *ce;
	gchar name[EFS_MAXPATHLEN];
	gint i, j, last;

	*res_de = NULL;
       
	if (!dir_inode) return NULL;
	if (strlen(path) >= EFS_MAXPATHLEN) return NULL;
	while ((*path) == EFS_FDEL) path++;

	if (!path[0]&&(dir_inode==IB1_ROOT_INODE)) {
		if (flags&EFS_ERASE) return NULL;
		*((IB1DirEntry *)ce_root.data) = root_de;
		*res_de = &root_de; 
		return &ce_root; 
	}
	
	while (*path) {
		i=0; while (path[i]&&(path[i]!=EFS_FDEL)) i++;
		j=i; while (path[j]&&(path[j]==EFS_FDEL)) j++;
		last = !path[j];
		strncpy (name, path, i); name[i]=0;
		ce = ib1_find_entry(efs, dir_inode, name, res_de,
				    last ? flags : 0, not_empty);
		if (!ce || last) return ce;
		dir_inode = GUINT32_FROM_LE((*res_de)->inode);
		path = &path[j];
	}

	return NULL;
}

/*
 * ib1_node_open:
 *
 *
 */

static EFSResult
ib1_node_open (EFSNode **node, EFSDir *efs_dir, const char *path, 
	       gint flags, gint type)
{
	IB1CacheEntry *ce;
	IB1Dir *parent = (IB1Dir *)efs_dir;
	IB1EFS *efs = (IB1EFS *)efs_dir->efs;
	IB1DirEntry *de;
	guint32 inode;
	const gchar *p;

	if ((flags&EFS_CREATE) && !type) return EFS_ERR_INVAL;

	p = path; while ((*p) == '/') p++;

	type = type & (EFS_FILE|EFS_DIR);

	if (!(*p)) { /* dup directory*/
		if (type == EFS_FILE) return EFS_ERR_NOTFILE; 
		(*node) = (EFSNode *)g_new0 (IB1Dir, 1);
		*((IB1Dir *)(*node)) = *((IB1Dir *)parent);
		ib1_inode_ref (efs, ((IB1Dir *)(*node))->inode);
		(*node)->mode &= ~(EFS_ROOT);
		return EFS_ERR_OK;
	}

	if (!(ce = ib1_namei (efs, parent->inode, p, &de, flags, NULL)) || !de)
		return EFS_ERR_NOENT;
	
	if (!de->inode) { /* create a new file or directory*/
		if (!(flags&EFS_CREATE)) return EFS_ERR_NOENT;
		CLOCK(ce);
		inode = ib1_inode_create (efs);
		CUNLOCK(ce);
		if (!inode) return EFS_ERR_INT;
		de->inode = GUINT32_TO_LE(inode);
		de->type = type | (flags&EFS_COMP);
	} else inode = GUINT32_FROM_LE(de->inode); /* file/dir exists */
	
	if (de->type & EFS_FILE) {
		if (!(de->type & EFS_FILE)) return EFS_ERR_NOTFILE;
		(*node) = (EFSNode *)g_new0 (IB1File, 1);
		(*node)->efs = (EFS *)efs;
		(*node)->mode = (flags&(EFS_RDWR|EFS_APPEND)) | EFS_FILE | 
			(de->type&EFS_COMP);
		((IB1File *)(*node))->inode = inode;
		ib1_inode_ref (efs, inode);
	}

	if (de->type & EFS_DIR) {
		if (!(de->type & EFS_DIR)) return EFS_ERR_NOTDIR;
		(*node) = (EFSNode *)g_new0 (IB1Dir, 1);
		(*node)->efs = (EFS *)efs; 
		(*node)->mode = (flags&EFS_RDWR) | EFS_DIR;
		((IB1Dir *)(*node))->inode = inode;
		ib1_inode_ref (efs, inode);
	}
	
	return EFS_ERR_OK;
}

/*
 * ib1_node_close:
 *
 * 
 */

static EFSResult
ib1_node_close (EFSNode *node)
{
	IB1File *file = (IB1File *)node;
	IB1EFS *efs = (IB1EFS *)node->efs;

	ib1_inode_unref (efs, file->inode);

	return EFS_ERR_OK;
}

/*
 * ib1_file_seek:
 *
 * see efs_file_seek().
 */

static EFSResult
ib1_file_seek (EFSFile *efs_file, gint32 offset, gint whence, guint32 *pos)
{
	IB1File *file = (IB1File *)efs_file;
	IB1EFS *efs = (IB1EFS *)efs_file->efs;
	gint32 npos;
	guint32 size;

	*pos = efs_file->pos;

	if ((whence == EFS_SEEK_SET)&&(offset==0)) {
		efs_file->pos = 0;
		*pos = efs_file->pos;
		return EFS_ERR_OK;
	}
	if ((whence == EFS_SEEK_CUR)&&(offset==0)) return EFS_ERR_OK;
       
	if (ib1_inode_info (efs, file->inode, NULL, &size)) return EFS_ERR_INT;

	switch (whence) {
	
	case EFS_SEEK_SET: npos = offset; break;
		
	case EFS_SEEK_CUR: npos = efs_file->pos + offset; break;
	
	case EFS_SEEK_END: npos = size + offset; break;
	
	default: return EFS_ERR_INVAL;
	
	}

	if (npos > size) return EFS_ERR_INVAL;

	efs_file->pos = npos;
	*pos = efs_file->pos;

	return EFS_ERR_OK;
}

/*
 * ib1_dir_seek:
 *
 * see efs_dir_seek().
 */

static EFSResult
ib1_dir_seek (EFSDir *efs_dir, guint32 offset)
{
	IB1Dir *dir = (IB1Dir *)efs_dir;
	IB1EFS *efs = (IB1EFS *)efs_dir->efs;
	gint32 blocks;

	if (offset == 0) {
		efs_dir->pos = 0;
		return EFS_ERR_OK;
	}

	if (ib1_inode_info (efs, dir->inode, &blocks, NULL)) 
		return EFS_ERR_INT;

	if (offset > (blocks*508)) return EFS_ERR_INVAL;

	efs_dir->pos = offset;

	return EFS_ERR_OK;
}

/*
 * ib1_file_read:
 *
 * see efs_file_read().
 */

static EFSResult
ib1_file_read (EFSFile *efs_file, gpointer buf, gint32 count, 
	       gint32 *bytes_read)
{
	IB1File *file = (IB1File *)efs_file;
	IB1EFS *efs = (IB1EFS *)efs_file->efs;
	IB1CacheEntry *ce, *ce1;
	IB1INode *node;
	guint32 size, block, ind, mb;
	
	*bytes_read = 0;

	if (!(ce = ib1_inode_map (efs, file->inode, FALSE))) 
		return EFS_ERR_INT;
	node = NODEP(ce, file->inode);
	size = GUINT32_FROM_LE(node->size);
	if ((efs_file->pos+count) > size) count = size - efs_file->pos;
	if (count <= 0) return EFS_ERR_OK; /* eof */

	if (efs_file->pos < IB1_IDATA_LEN) { /* read from inode */
		mb = MIN ((IB1_IDATA_LEN-efs_file->pos), count);
		memcpy (buf, &node->data[efs_file->pos], mb);
		*bytes_read = mb;
		buf += mb;
		efs_file->pos += mb;
	}

        while ((*bytes_read) < count) {

		block = (efs_file->pos - IB1_IDATA_LEN)/508;
		ind = (efs_file->pos - IB1_IDATA_LEN)%508;
		mb = MIN ((508-ind), count -  (*bytes_read));

		ce1 = ib1_inode_bmap (efs, file->inode, block, FALSE);
		if (!ce1) return EFS_ERR_INT;
		memcpy (buf, ce1->data + ind, mb);

		*bytes_read += mb;
		buf += mb;
		efs_file->pos += mb;
	}

	return EFS_ERR_OK;
}

/*
 * ib1_file_write:
 *
 * see efs_file_write().
 */

static EFSResult
ib1_file_write (EFSFile *efs_file, gpointer buf, gint32 count)
{
	IB1File *file = (IB1File *)efs_file;
	IB1EFS *efs = (IB1EFS *)efs_file->efs;
	IB1CacheEntry *ce, *ce1;
	IB1INode *node;
	guint32 size, block, ind, mb;
	gint bytes_written;

	if (!(ce = ib1_inode_map (efs, file->inode, TRUE))) return EFS_ERR_INT;
	CLOCK(ce);
	node = NODEP(ce, file->inode);
	size = GUINT32_FROM_LE(node->size);
	if ((efs_file->pos > size) || (efs_file->mode&EFS_APPEND)) 
		efs_file->pos = size;

	bytes_written = 0;

	if (efs_file->pos < IB1_IDATA_LEN) { /* write to inode */
		mb = MIN ((IB1_IDATA_LEN-efs_file->pos), count);
		memcpy (&node->data[efs_file->pos], buf, mb);
		bytes_written = mb;
		buf += mb;
		efs_file->pos += mb;
		ib1_cache_touch (ce, TRUE);
	}

	while (bytes_written < count) {
		block = (efs_file->pos-IB1_IDATA_LEN)/508;
		ind = (efs_file->pos-IB1_IDATA_LEN)%508;
		mb = MIN ((508-ind), count-bytes_written);

		ce1 = ib1_inode_bmap (efs, file->inode, block, TRUE);
		if (!ce1) { CUNLOCK(ce); return EFS_ERR_INT; }
		ib1_cache_touch (ce1, TRUE);
		memcpy (ce1->data+ind, buf, mb);

		bytes_written += mb;
		buf += mb;
		efs_file->pos += mb;
	}

	if (efs_file->pos > GUINT32_FROM_LE(node->size)) {
		node->size = GUINT32_TO_LE(efs_file->pos);
		ib1_cache_touch (ce, TRUE);
	}

	CUNLOCK(ce);

	return EFS_ERR_OK;	
}

/*
 * ib1_dir_read:
 *
 * see efs_dir_read().
 */

static EFSResult     
ib1_dir_read  (EFSDir *efs_dir, EFSDirEntry *res_de)
{
	IB1Dir *dir = (IB1Dir *)efs_dir;
	IB1EFS *efs = (IB1EFS *)efs_dir->efs;
	IB1CacheEntry *ce;
	IB1DirEntry *de;
	IB1INode *node;
	gint32 block, ind;
	guint32 size;

	if (!(ce = ib1_inode_map (efs, dir->inode, FALSE))) return EFS_ERR_INT;
	node = NODEP(ce, dir->inode);
	if (node->size) return EFS_ERR_INT;
	size = GUINT32_FROM_LE(node->blocks)*508;

	while (efs_dir->pos < size) {
		block = efs_dir->pos/508;
		ind = efs_dir->pos%508;

		ce = ib1_inode_bmap (efs, dir->inode, block, FALSE);
		if (!ce) return EFS_ERR_INT;
		de = (IB1DirEntry *)(ce->data+ind);
		if (!ib1_check_entry (de, 508-ind)) return EFS_ERR_INT;
		
		efs_dir->pos += GUINT16_FROM_LE(de->rec_len);

		if (de->inode) {
			res_de->inode = GUINT32_FROM_LE(de->inode);
			res_de->type = de->type;
			res_de->offset = efs_dir->pos-
				GUINT16_FROM_LE(de->rec_len);
			res_de->length = de->name_len;
			strncpy (res_de->name, de->name, de->name_len);
			res_de->name[de->name_len] = 0;
			return EFS_ERR_OK;
		}
	}

	return EFS_ERR_NOENT; /* eof */
}

/*
 * ib1_file_trunc:
 *
 * see efs_file_trunc().
 */

static EFSResult
ib1_file_trunc (EFSFile *efs_file, guint32 size)
{
	IB1File *file = (IB1File *)efs_file;
	IB1EFS *efs = (IB1EFS *)efs_file->efs;
	IB1CacheEntry *ce;
	IB1INode *node;
	guint32 fs, block;

	if (!(ce = ib1_inode_map (efs, file->inode, TRUE))) return EFS_ERR_INT;
	node = NODEP(ce, file->inode);
	CLOCK(ce);
	fs = GUINT32_FROM_LE(node->size);
	if (fs <= size) return EFS_ERR_OK; /* eof */

	if (size <= IB1_IDATA_LEN) block = 0;
	else block = ((size-IB1_IDATA_LEN)+507)/508;
	
	if (ib1_inode_trunc (efs, file->inode, block)) {
		CUNLOCK(ce); 
		return EFS_ERR_INT;
	}

	node->size = GUINT32_TO_LE(size);
	efs_file->pos = size;
	ib1_cache_touch (ce, TRUE);

	CUNLOCK(ce);
	return EFS_ERR_OK;
}

/*
 * ib1_type_set:
 *
 */

static EFSResult
ib1_type_set (EFSNode *efs_node, guint32 type)
{
	IB1File *file = (IB1File *)efs_node;
	IB1EFS *efs = (IB1EFS *)efs_node->efs;
	IB1CacheEntry *ce;
	IB1INode *node;

	if (!(ce = ib1_inode_map (efs, file->inode, TRUE))) return EFS_ERR_INT;
	node = NODEP(ce, file->inode);
	node->type = GINT32_TO_LE(type);
	ib1_cache_touch (ce, TRUE);

	return EFS_ERR_OK;
}

/*
 * ib1_type_get:
 *
 */

static EFSResult
ib1_type_get (EFSFile *efs_node, guint32 *type)
{
	IB1File *file = (IB1File *)efs_node;
	IB1EFS *efs = (IB1EFS *)efs_node->efs;
	IB1CacheEntry *ce;
	IB1INode *node;

	if (!(ce=ib1_inode_map (efs, file->inode, FALSE))) return EFS_ERR_INT;
	node = NODEP(ce, file->inode);
       
	*type = GUINT32_FROM_LE(node->type);

	return EFS_ERR_OK;
}

/*
 * ib1_node_stat:
 *
 */

static EFSResult
ib1_node_stat (EFSNode *efs_node, EFSStat *stat)
{
	IB1EFS *efs = (IB1EFS *)efs_node->efs;
	IB1CacheEntry *ce;
	IB1INode *node;
	guint32 inode = ((IB1File *)efs_node)->inode;

	stat->type = efs_node->mode & (EFS_DIR|EFS_FILE);

	if (!(ce = ib1_inode_map (efs, inode, FALSE))) return EFS_ERR_INT;
	node = NODEP(ce, inode);
	
	stat->size = GUINT32_FROM_LE (node->size);

	return EFS_ERR_OK;
}

/*
 * ib1_erase:
 *
 * see efs_erase().
 */

static EFSResult
ib1_erase (EFSDir *efs_dir, const char *path) 
{
	IB1Dir *dir = (IB1Dir *)efs_dir;
	IB1EFS *efs = (IB1EFS *)efs_dir->efs;
	IB1DirEntry *de;
	gint not_empty;

	if (!(ib1_namei (efs, dir->inode, path, &de, EFS_ERASE, &not_empty))) {
		if (not_empty) return EFS_ERR_NOTEMPTY;
		else return EFS_ERR_NOENT;
	}

	return EFS_ERR_OK;
}

/*
 * ib1_rename:
 *
 * see efs_rename().
 */

static EFSResult
ib1_rename (EFSDir *efs_dir, const char *old_path, const char *new_path)
{
	IB1Dir *dir = (IB1Dir *)efs_dir;
	IB1EFS *efs = (IB1EFS *)efs_dir->efs;
	IB1CacheEntry *ce, *ce1;
	IB1DirEntry *de, *de1;

	/* fixme: this is not an atomic action */

	if (!(ce1=ib1_namei(efs,dir->inode,new_path,&de1,EFS_CREATE|EFS_EXCL,
			    NULL)) || !de1) 
		return EFS_ERR_NOENT;

	if (!(ce = ib1_namei (efs, dir->inode, old_path, &de, 0, NULL)) || !de)
		return EFS_ERR_NOENT;

	de1->inode = de->inode;
	de1->type = de->type;

	/* trick: set inode to zero and then call erase to delete the
	   directory entry, but not the content */
	de->inode = 0;
	ib1_namei(efs, dir->inode, old_path, &de, EFS_ERASE,NULL);

	return EFS_ERR_OK;
}

/*
 * ib1_node_equal:
 *
 * see efs_node_equal().
 */

static gboolean
ib1_node_equal (EFSNode *node1, EFSNode *node2)
{
	IB1File *n1 = (IB1File *)node1;
	IB1File *n2 = (IB1File *)node2;

	if (n1->inode == n2->inode) return TRUE;

	return FALSE;
}


