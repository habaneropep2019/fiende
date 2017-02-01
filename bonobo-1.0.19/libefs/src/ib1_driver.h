/* ib1_driver.h: IB1 driver header file

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

#ifndef _EFS_IB1_H_
#define _EFS_IB1_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "efs_internal.h"
#include "blowfish.h"

#define IB1_IMAP_INODE 1
#define IB1_ROOT_INODE 2
#define IB1_TYPE_INODE 3

#define IB1_NAME_LEN      244
#define IB1_N_BLOCKS      11
#define IB1_IDATA_LEN     64
#define IB1_CACHE_SIZE    20
#define IB1_ICACHE_SIZE   256
#define IB1_IBCACHE_SIZE  256

#define NODEP(ce,inode) ((IB1INode *)(ce->data+(inode%4)*sizeof(IB1INode)))
#define CLOCK(ce) {ce->lock++;}
#define CUNLOCK(ce) {if (ce->lock) ce->lock--;}


typedef struct {
	EFSFile      file;
        guint32      inode;
} IB1File;

typedef IB1File IB1Dir;

typedef struct {
	guint32 inode;
	guint16 rec_len;
	guint8  name_len;
	guint8  type;
	gchar   name[IB1_NAME_LEN];
} IB1DirEntry;

typedef struct {
	guint32 size;
	guint32 blocks;
	guint32 block[IB1_N_BLOCKS];
	guint32 type;
	guint32 reserved;
	gchar   data[IB1_IDATA_LEN];
} IB1INode;

typedef struct {
	IB1INode node[4];
	guint32  version;
	guint32  inum;
	guint32  pblock;
	guint32  crc;
} IB1INodeBlock;

typedef struct {
	guint32  inode;
	guint32  ref_count;
	gboolean erase;
} IB1INodeLEntry;

typedef struct {
	guint32    fb;        /* first block */
	guint32    bc;        /* block count */
	guint32    dbc;       /* data block count */
	guint32    alen;      /* data array length */  
	guint32  **data;    /* pointer to data blocks (each 512 bytes) */     
	guint32   *fbc;      /* number of free blocks in each data block */
} IB1Bitmap;

typedef struct {
        guint32   at;
        gboolean  dirty;
        guint16   lock;
        guint32   block;
        gchar     data[512];
} IB1CacheEntry;

typedef struct {
	guint32 inode;
	guint32 block;
} IB1ICEntry;

typedef struct {
	guint32 inode;
	guint32 iblk;
	guint32 block;
} IB1IBCEntry;

typedef struct {
	EFSHeader head;
	guint32   cb;             /* commited blocks */
	guint32   version;
	guint32   imap_start;
	guint32   inode_count;
	guint32   free_blocks;
	guint32   reserverd[106];
        guint32   crc;
} IB1Header;

typedef struct {
	EFS           efs;
	gint          fd;
	IB1Header     head;
	IB1Bitmap     bmap;
	IB1CacheEntry cache[IB1_CACHE_SIZE];
	IB1ICEntry    icache[IB1_ICACHE_SIZE];
	IB1IBCEntry   ibcache[IB1_IBCACHE_SIZE];
	GList         *inode_list;
	BlowfishCTX  ctx;
} IB1EFS;

extern EFSDriver   efs_driver_ib1;
extern EFSDriver   efs_driver_ib1_enc;

extern EFSSuperOps super_ops_ib1;
extern EFSFileOps  file_ops_ib1;

void           ib1_cache_flush     (IB1EFS *efs);
void           ib1_cache_touch     (IB1CacheEntry *ce, gboolean dirty);
IB1CacheEntry *ib1_cache_map       (IB1EFS *efs, guint32 block, 
				    gboolean noread);
IB1CacheEntry *ib1_cache_map_clone (IB1EFS *efs, guint32 block);

void           ib1_bitmap_init   (IB1EFS *efs, guint32 first_block);
void           ib1_bitmap_free   (IB1EFS *efs);
guint32        ib1_block_alloc   (IB1EFS *efs);
void           ib1_block_free    (IB1EFS *efs, guint32 block);
gint           ib1_block_get_fbc (IB1EFS *efs);

guint32        ib1_inode_create  (IB1EFS *efs);
gint           ib1_inode_erase   (IB1EFS *efs, guint32 inode);
gint           ib1_inode_trunc   (IB1EFS *efs, guint32 inode, 
				  gint32 block);
IB1CacheEntry *ib1_inode_map     (IB1EFS *efs, guint32 inode, gboolean clone);
IB1CacheEntry *ib1_inode_bmap    (IB1EFS *efs, guint32 inode, 
				  guint32 block, gboolean clone);

gint           ib1_inode_ref       (IB1EFS *efs, guint32 inode);
gint           ib1_inode_unref     (IB1EFS *efs, guint32 inode);
gint           ib_inode_refcount   (IB1EFS *efs, guint32 inode);
void           ib1_inode_list_free (IB1EFS *efs);

void           ib1_encrypt       (IB1EFS *efs, guint32 *buf, gint count);
void           ib1_decrypt       (IB1EFS *efs, guint32 *buf, gint count);

#ifdef __cplusplus
}
#endif

#endif /* _EFS_IB1_H_ */


