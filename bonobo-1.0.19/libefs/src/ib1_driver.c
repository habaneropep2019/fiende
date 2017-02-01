/* ib1_driver.c: INode Dased Driver 1 SuperOps

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

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <zlib.h>

#include "ib1_driver.h"

static EFSResult  ib1_open     (EFSDir **dir, EFSDriver *driver, 
				const char *path, gint flags, gchar *passwd);
static EFSResult  ib1_create   (EFSDir **dir, EFSDriver *driver, 
				const char *path, gint flags, gint mode, 
				gchar *passwd);
static EFSResult  ib1_close    (EFSDir *root);
static EFSResult  ib1_commit   (EFSDir *root);
static EFSResult  ib1_revert   (EFSDir *root);
static EFSResult  ib1_fsstat   (EFSDir *root, EFSFSStat *fsstat);

EFSSuperOps super_ops_ib1 = {
	ib1_open,
	ib1_create,
	ib1_close,
	ib1_commit,
	ib1_revert,
	ib1_fsstat,
};

EFSDriver efs_driver_ib1 = {
	"IB1", FALSE,
	"INode based driver (IB1)",
	&super_ops_ib1,
	&file_ops_ib1,
};

EFSDriver efs_driver_ib1_enc = {
	"IB1ENC", TRUE,
	"INode based driver with encryption (IB1ENC)",
	&super_ops_ib1,
	&file_ops_ib1,
};

static gint
flag_conv (gint flags)
{
	gint pflags;

	if (flags&EFS_WRITE) pflags = O_RDWR;
	else pflags = O_RDONLY;
	if (flags&EFS_CREATE) pflags |= O_CREAT;
	if (flags&EFS_EXCL) pflags |= O_EXCL;

	return pflags;
}

void           
ib1_encrypt (IB1EFS *efs, guint32 *buf, gint count)
{
	gint i;

	g_return_if_fail ((count&1) == 0); /* only 64 bit blocks */

	for (i = 0; i < count; i+=2) {
		blowfish_encrypt (&efs->ctx, &buf[i], &buf[i+1]);
	}
}

void           
ib1_decrypt (IB1EFS *efs, guint32 *buf, gint count)
{
	gint i;

	g_return_if_fail ((count&1) == 0); /* only 64 bit blocks */

	for (i = 0; i < count; i+=2) {
		blowfish_decrypt (&efs->ctx, &buf[i], &buf[i+1]);
	}

}

static guint32
ib1_calc_crc (IB1Header *head)
{
	guint32 crc = adler32 (0L, NULL, 0);

	crc = adler32 (crc, (guint8 *)head, 
			(guint8 *)&head->crc-(guint8 *)head);

	return crc;
}

static gint
ib1_read_head (IB1EFS *efs, IB1Header *head)
{
	if (lseek (efs->fd, 0, SEEK_SET) == -1) return FALSE;
	if (read (efs->fd, head, 512) != 512) return FALSE;


	((EFS *)efs)->type = GINT32_FROM_LE(head->head.type);

	if (GINT32_FROM_LE(head->head.protected) && 
	    (((EFS*)efs)->driver->encrypted)) {
		ib1_decrypt (efs, &head->cb, (512 - sizeof(EFSHeader))/4);
	}

	if (ib1_calc_crc (head) != GUINT32_FROM_LE(head->crc)) return FALSE;

	head->head.protected = GINT32_FROM_LE (head->head.protected);
	head->head.type = GINT32_FROM_LE (head->head.protected);

	head->crc  = 0;
	head->cb  = GUINT32_FROM_LE(head->cb);
	head->version  = GUINT32_FROM_LE(head->version);
	head->imap_start  = GUINT32_FROM_LE(head->imap_start);
	head->inode_count  = GUINT32_FROM_LE(head->inode_count);
	head->free_blocks  = GUINT32_FROM_LE(head->free_blocks);
	
	return TRUE;
}

static gint
ib1_write_head (IB1EFS *efs, IB1Header *head)
{
	IB1Header tmp;
	guint32 crc;
	
	memcpy (&tmp, head, 512);

	strncpy(tmp.head.efs_id, EFS_FILE_ID, 4);
	strncpy(tmp.head.drivername, ((EFS *)efs)->driver->drivername, 12);

	tmp.cb  = GUINT32_TO_LE(head->cb);
	tmp.version  = GUINT32_TO_LE(head->version);
	tmp.imap_start  = GUINT32_TO_LE(head->imap_start);
	tmp.inode_count  = GUINT32_TO_LE(head->inode_count);
	tmp.free_blocks  = GUINT32_TO_LE(head->free_blocks);
	tmp.head.type = GINT32_TO_LE(((EFS *)efs)->type);
	tmp.head.protected = GINT32_TO_LE(head->head.protected);

	crc=ib1_calc_crc (&tmp);
	tmp.crc  = GUINT32_TO_LE(crc);
	
	if (head->head.protected && (((EFS*)efs)->driver->encrypted)) {
		ib1_encrypt (efs, &tmp.cb, (512 - sizeof(EFSHeader))/4);
	}

	if (lseek (efs->fd, 0, SEEK_SET) == -1) return FALSE;
	if (write(efs->fd, &tmp, 512) == 512) return TRUE;
	
	return FALSE;
}

static EFSResult
create_default_inodes (IB1EFS *efs)
{
	guint32 inode;

	/* create all inodes in the first block 
	 * the first block containst is special
	 * since it contains the IMAP
	 */

	if (!(inode = ib1_inode_create(efs)) || (inode != IB1_IMAP_INODE))
		return EFS_ERR_INT;
	
	if (!(inode = ib1_inode_create(efs)) || (inode != IB1_ROOT_INODE))
		return EFS_ERR_INT;

	if (!(inode = ib1_inode_create(efs)) || (inode != IB1_TYPE_INODE))
		return EFS_ERR_INT;

	return EFS_ERR_OK;
} 

static void
ib1_create_typefd (IB1EFS *efs)
{
	((EFS *)efs)->typefd = (EFSFile *)g_new0 (IB1File, 1);
	((EFS *)efs)->typefd->efs = (EFS *)efs;
	((EFS *)efs)->typefd->mode = (((EFS *)efs)->mode&EFS_RDWR) | EFS_FILE;
	((IB1File *)((EFS *)efs)->typefd)->inode = IB1_TYPE_INODE;
	ib1_inode_ref (efs, IB1_TYPE_INODE);
}

static EFSResult
ib1_open (EFSDir **dir, EFSDriver *driver, const char *path, gint flags, 
	  gchar *passwd)
{
	IB1EFS *efs;
	IB1CacheEntry *ce;

	g_return_val_if_fail (sizeof(IB1Header) == 512, EFS_ERR_INT);
	g_return_val_if_fail (sizeof(IB1INodeBlock) == 512, EFS_ERR_INT);

	efs = g_new0 (IB1EFS, 1);
	((EFS *)efs)->driver = driver;
	if (flags&EFS_WRITE) ((EFS *)efs)->mode = EFS_RDWR;
	else ((EFS *)efs)->mode = EFS_READ;

	if ((efs->fd = open(path, flag_conv(flags))) == -1) {
		g_free (efs);
		return EFS_ERR_ERRNO;
	}

	if (passwd && (((EFS*)efs)->driver->encrypted))
		blowfish_init (&efs->ctx, passwd, strlen(passwd));

	if (!ib1_read_head (efs, &efs->head)) {
		close (efs->fd);
		g_free (efs);
		return EFS_ERR_ERRNO;
	} 

	/* Fixme: make more consistency checks ?*/

	if (!efs->head.cb) return EFS_ERR_FORMAT;

	ib1_bitmap_init (efs, efs->head.cb);

	if (efs->head.cb == 1) { 
		if (efs->head.imap_start != 1) return EFS_ERR_FORMAT; 
		if (efs->head.inode_count != 3) return EFS_ERR_FORMAT;
		efs->head.inode_count = 0;
		if (ib1_block_alloc (efs) != 1) return EFS_ERR_INT;
		if (create_default_inodes (efs) != EFS_ERR_OK) {
			g_free (efs); 
			return EFS_ERR_INT; 
		}
	}
	
	if (flags&EFS_WRITE) {
		if (!(ce = ib1_cache_map_clone (efs, efs->head.imap_start))) { 
			g_free (efs);
			return EFS_ERR_INT; 
		}
		((IB1INodeBlock *)(ce->data))->pblock = efs->head.imap_start;
		((IB1INodeBlock *)(ce->data))->version = 
			GUINT32_TO_LE (efs->head.version);

		efs->head.imap_start = ce->block;
	}

	ib1_create_typefd (efs);

	(*dir) = (EFSDir *)g_new0 (IB1Dir, 1);
	(*dir)->efs = (EFS *)efs;
	(*dir)->pos = 0;
	((IB1Dir *)(*dir))->inode = IB1_ROOT_INODE;

	return EFS_ERR_OK;
}

static EFSResult
ib1_create (EFSDir **dir, EFSDriver *driver, const char *path, gint flags, 
	    gint mode, gchar *passwd)
{
	IB1EFS *efs;

	g_return_val_if_fail (sizeof(IB1Header) == 512, EFS_ERR_INT);
	g_return_val_if_fail (sizeof(IB1INodeBlock) == 512, EFS_ERR_INT);

	efs = g_new0 (IB1EFS, 1);
	((EFS *)efs)->driver = driver;
	if (flags&EFS_WRITE) ((EFS *)efs)->mode = EFS_RDWR;
	else ((EFS *)efs)->mode = EFS_READ;


	if ((efs->fd = open(path, flag_conv(flags), mode)) == -1) { 
		g_free (efs);
		return EFS_ERR_ERRNO;
	}

	efs->head.cb = 1;
	ib1_bitmap_init (efs, 1);

	if (passwd) {
		efs->head.head.protected = 1;
		if (!driver->encrypted) efs_passwd_set(&efs->head.head,passwd);
		else blowfish_init (&efs->ctx, passwd, strlen(passwd));
	}


	if (!(efs->head.imap_start = ib1_block_alloc (efs))) 
		return EFS_ERR_INT;
	
	if (create_default_inodes (efs) != EFS_ERR_OK) {
		g_free (efs);
		return EFS_ERR_INT;
	} 

		       
        if (!ib1_write_head (efs, &efs->head)) {
		g_free (efs);
		return EFS_ERR_ERRNO;
	}

	ib1_create_typefd (efs);

	(*dir) = (EFSDir *)g_new0 (IB1Dir, 1);
	(*dir)->efs = (EFS *)efs;
	(*dir)->pos = 0;
	((IB1Dir *)(*dir))->inode = IB1_ROOT_INODE;

	return EFS_ERR_OK;
}

static EFSResult
ib1_close (EFSDir *root)
{
	IB1EFS *efs = (IB1EFS *)root->efs;

	ib1_inode_list_free (efs);

	ib1_cache_flush (efs);

	ib1_bitmap_free (efs);

	if (root->efs->mode&EFS_WRITE) ftruncate(efs->fd, efs->head.cb*512);
       
	if (close (efs->fd)) return EFS_ERR_ERRNO;

	return EFS_ERR_OK;
}

static EFSResult       
ib1_commit (EFSDir *root)
{
	IB1EFS *efs = (IB1EFS *)root->efs;
	IB1CacheEntry *ce;
	gint i, free_blocks;

	ib1_inode_list_free (efs);

	free_blocks = ib1_block_get_fbc (efs);

	ib1_cache_flush (efs);

	efs->head.free_blocks += free_blocks;
	efs->head.cb = efs->bmap.bc + 1;
	efs->head.version++;

	if (!ib1_write_head (efs, &efs->head)) return EFS_ERR_ERRNO;

	ftruncate(efs->fd, efs->head.cb*512);
	sync ();

	for (i=0; i< IB1_ICACHE_SIZE; i++) efs->icache[i].inode = 0;
	for (i=0; i< IB1_IBCACHE_SIZE; i++) efs->ibcache[i].inode = 0;

	ib1_bitmap_free (efs);
	ib1_bitmap_init (efs, efs->head.cb);

	if (!(ce = ib1_cache_map_clone (efs, efs->head.imap_start))) 
		return EFS_ERR_INT; 
	((IB1INodeBlock *)(ce->data))->pblock = efs->head.imap_start;
	((IB1INodeBlock *)(ce->data))->version = 
		GUINT32_TO_LE (efs->head.version);

	efs->head.imap_start = ce->block;

	return EFS_ERR_OK;
}

static EFSResult 
ib1_revert (EFSDir *root)
{
	IB1EFS *efs = (IB1EFS *)root->efs;
	IB1CacheEntry *ce;
	gint i;

	ib1_inode_list_free (efs);
	ib1_cache_flush (efs);

	ftruncate(efs->fd, efs->head.cb*512);
     	
	if (!ib1_read_head (efs, &efs->head)) return EFS_ERR_ERRNO;
	
	for (i=0; i< IB1_ICACHE_SIZE; i++) efs->icache[i].inode = 0;
	for (i=0; i< IB1_IBCACHE_SIZE; i++) efs->ibcache[i].inode = 0;

	ib1_bitmap_free (efs);
	ib1_bitmap_init (efs, efs->head.cb);
	
	if (!(ce=ib1_cache_map_clone (efs, efs->head.imap_start))) 
		return EFS_ERR_INT; 
	((IB1INodeBlock *)(ce->data))->pblock = efs->head.imap_start;
	((IB1INodeBlock *)(ce->data))->version = 
		GUINT32_TO_LE (efs->head.version);

	efs->head.imap_start = ce->block;
	
	return EFS_ERR_OK;
}

static EFSResult
ib1_fsstat (EFSDir *root, EFSFSStat *fsstat)
{
	IB1EFS *efs;
	gint free = 0;

	efs = (IB1EFS *)root->efs;
	
	if (root->efs->mode&EFS_WRITE) free = ib1_block_get_fbc (efs);
	
	fsstat->drivername = ((EFS *)efs)->driver->drivername;
	fsstat->size = efs->head.cb+efs->bmap.bc*512;
	fsstat->free = (efs->head.free_blocks + free)*512;
	fsstat->namelen = IB1_NAME_LEN;
	fsstat->version = efs->head.version;	
	fsstat->protected = efs->head.head.protected;
	fsstat->encrypted = root->efs->driver->encrypted;

	return EFS_ERR_OK; 
}
