/* ib1_file.c: INode Dased Driver 1 INode functions

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

#include <time.h>

#include "ib1_driver.h"

#define INODE_CREATE 1
#define INODE_CLONE  2
#define INODE_ERASE  4

#define USE_ICACHE
#define USE_IBCACHE

static guint32
ib1_imap_lookup (IB1EFS *efs, guint32 inode, gint flags, guint32 *oblock)
{
	IB1CacheEntry *ce, *ce1;
	guint32 block, imap_block, ind, *p;

	if (oblock) *oblock = 0;

	if (flags&INODE_CREATE) flags |= INODE_CLONE;

	if (inode < 4) return efs->head.imap_start;
	if ((inode > efs->head.inode_count) && !(flags&INODE_CREATE)) return 0;
	if (inode > (efs->head.inode_count+1)) return 0;

	/* we can store the first IB1_IDATA_LEN*2 imap entries 
	   in the imap_start block! */
	if (inode < (IB1_IDATA_LEN*2)) {
		IB1INode *node;

		if (!(ce = ib1_cache_map(efs, efs->head.imap_start, FALSE))) 
			return 0;
		node = NODEP(ce, (inode/IB1_IDATA_LEN));
		p = (guint32 *)(node->data+((inode%IB1_IDATA_LEN)&0xfffffffc));
		ind = GUINT32_FROM_LE(*p);
	} else {
		imap_block = (inode-(IB1_IDATA_LEN*2))/508;
		if (!(ce = ib1_inode_bmap(efs, 1, imap_block, (flags!=0)))) 
			return 0;
		p = ((guint32 *)ce->data+((inode-(IB1_IDATA_LEN*2))/4)%127);
		ind = GUINT32_FROM_LE(*p);
	}

	if ((block = (ind>>4))) {
		if (!(flags&INODE_CREATE) && !(ind&(1<<(inode%4)))) return 0;
		if (!(ind&(1<<(inode%4)))) {
			if (flags&INODE_ERASE) return 0;
			ind |= (1<<(inode%4));
			*p = GUINT32_TO_LE(ind);
			ib1_cache_touch (ce, TRUE);		
		}
		if (flags&INODE_ERASE) {
			g_assert (block >= efs->head.cb);
			ind &= ~(1<<(inode%4));
			*p = GUINT32_TO_LE(ind);
			ib1_cache_touch (ce, TRUE);		
			if (!(ind&15)) {
				*p = 0;
				ib1_block_free (efs, block);
			} 
		} else {
			if ((flags&INODE_CLONE) && (block < efs->head.cb)) {
				ce1 = ib1_cache_map_clone (efs, block);
				if (oblock) *oblock = block;
				block = ce1->block;
				ind = (block<<4)|(ind&0xf);
				*p = GUINT32_TO_LE(ind);
				ib1_cache_touch (ce, TRUE);
			}
		}
	} else {
		if (flags&INODE_ERASE) return 0;
		if (!(flags&INODE_CREATE)) return 0;
		block = ib1_block_alloc (efs);
		ind = (block<<4)|(1<<(inode%4));
		*p = GUINT32_TO_LE(ind);
		ib1_cache_touch (ce, TRUE);
	}

	return block;
}

guint32        
ib1_inode_create (IB1EFS *efs)
{
	guint32 inode, block;
	IB1CacheEntry *ce;
	IB1INode *node;
	gint i;

	inode = ++efs->head.inode_count;

	block = ib1_imap_lookup (efs, inode, INODE_CREATE, NULL);

	if (!block) return 0;

	if (!(ce = ib1_cache_map(efs, block, FALSE))) return 0;
	node = NODEP(ce, inode);

	((IB1INodeBlock *)ce)->inum = GUINT32_TO_LE((inode/4));
	node->size = 0;
	node->blocks = 0;
	node->type = 0;

	for (i = 0; i < (IB1_N_BLOCKS-3); i++) node->block[i] = 0;

	ib1_cache_touch(ce, TRUE);

	return inode;
}

static gint
ib1_inode_erase_real (IB1EFS *efs, guint32 inode)
{
	if (inode < 4) return 0; /* don´t erase system inodes */

	ib1_inode_trunc(efs, inode, 0);

	return ib1_imap_lookup (efs, inode, INODE_ERASE, NULL);
}

gint           
ib1_inode_erase (IB1EFS *efs, guint32 inode)
{
	GList *l;
	IB1INodeLEntry *e;

	l = efs->inode_list;
	while (l) {
		e = (IB1INodeLEntry *)l->data;
		if (e->inode == inode) {
			e->erase = TRUE;
			return 0;
		}
		l = l->next;
	}

	return ib1_inode_erase_real (efs, inode);
}

static void
trunc_ind (IB1EFS *efs, IB1CacheEntry *ce, guint32 *bref, guint32 block, 
	   guint32 level)
{
	IB1CacheEntry *ce1;
	guint32 *buf, rb, hb;
	gint i, c;

	if (!(*bref)) return;

	if (!(ce1 = ib1_cache_map_clone (efs, GUINT32_FROM_LE(*bref)))) return;
	
	if (ce1->block != GUINT32_FROM_LE(*bref)) {
		*bref = GUINT32_TO_LE (ce1->block);
		ib1_cache_touch (ce, TRUE);
	}
	buf = (guint32 *)ce1->data;
	CLOCK(ce1);

	for (i = block/level, rb = block%level; i<127; i++) {
		hb = GUINT32_FROM_LE(buf[i]);
		if ((level > 1) && (((guint32*)ce1->data)[i]))
			trunc_ind (efs,ce1,&(((guint32*)ce1->data)[i]),
				   rb, level/127);
		else if (hb) {
			if (hb >= efs->head.cb) ib1_block_free (efs, hb);
			buf[i] = 0;
			ib1_cache_touch(ce1, TRUE);  
		}
		rb = 0;
	}

	CUNLOCK(ce1);

	c = 0; for (i=0; i<127;i++) if (buf[i]) c++;

	if (!c) {
		ib1_block_free (efs, GUINT32_FROM_LE(*bref));
		*bref = 0;
		ib1_cache_touch(ce, TRUE);
	} else {
		ib1_cache_touch(ce1, TRUE);
	}
}

gint           
ib1_inode_trunc (IB1EFS *efs, guint32 inode, gint32 block)
{
	IB1CacheEntry *ce;
	IB1INode *node;
	gint32 hb;
	gint i;

	if (!(ce = ib1_inode_map (efs, inode, TRUE))) return -1;
	node = NODEP(ce, inode);

	if (block == GUINT32_FROM_LE(node->blocks)) return 0;
	if (block > GUINT32_FROM_LE(node->blocks)) return -1;

	ib1_cache_touch (ce, TRUE);

	CLOCK (ce);

	for (i=block; i < (IB1_N_BLOCKS-3); i++) {
		hb = GUINT32_FROM_LE(node->block[i]);
		if (hb && (hb >= efs->head.cb)) ib1_block_free(efs, hb);
		node->block[i] = 0;
	}

	if ((block -= (IB1_N_BLOCKS-3)) < 0) block = 0;
	if ((block < 127) && (node->block[IB1_N_BLOCKS-3]))
		trunc_ind (efs, ce, &node->block[IB1_N_BLOCKS-3], block, 1);

	if ((block -= 127) < 0) block = 0;
	if ((block < (127*127)) && (node->block[IB1_N_BLOCKS-2]))
		trunc_ind (efs, ce, &node->block[IB1_N_BLOCKS-2], block, 127);
      
	if ((block -= (127*127)) < 0) block = 0;
	if ((block < (127*127*127)) && (node->block[IB1_N_BLOCKS-1]))
		trunc_ind (efs,ce,&node->block[IB1_N_BLOCKS-1],block,127*127);

	CUNLOCK(ce);	    
	return 0;
}

IB1CacheEntry*
ib1_inode_map (IB1EFS *efs, guint32 inode, gboolean clone)
{
	IB1CacheEntry *ce;
	IB1INode *node;
	gint ind = inode%IB1_ICACHE_SIZE;
	guint32 ib = 0, ob = 0;

	g_return_val_if_fail (inode != 0, NULL);

#ifdef USE_ICACHE
	if (efs->icache[ind].inode == inode) {
		ib = efs->icache[ind].block;
		if (clone && (ib < efs->head.cb)) {
			ib = 0;
		}
	}
#endif
		
	if (!ib) {
		if (clone) ib = ib1_imap_lookup (efs, inode, INODE_CLONE, &ob);
		else ib = ib1_imap_lookup (efs, inode, 0, NULL);
	}

	efs->icache[ind].inode = inode;
	efs->icache[ind].block = ib;


	if (ib && (ce = ib1_cache_map(efs, ib, FALSE))) {
		if (clone) {
			node = NODEP(ce, inode);
			if (ob) {
				((IB1INodeBlock *)(ce->data))->pblock = 
					GUINT32_TO_LE (ob);
				((IB1INodeBlock *)(ce->data))->version = 
					GUINT32_TO_LE (efs->head.version);
			}
		}
		return ce;
	}

	return NULL;
}

static IB1CacheEntry*
map_ind (IB1EFS *efs, IB1CacheEntry *ce, guint32 *bref, gboolean clone)
{
	IB1CacheEntry *ce1;
	guint32 nb;
	gint i;
	
	if (!(*bref)) {
		if (!clone) return NULL;
                nb = ib1_block_alloc(efs);
		*bref = GUINT32_TO_LE(nb);
		ib1_cache_touch(ce, TRUE);
		ce1 = ib1_cache_map (efs, nb, TRUE);
		for (i=0;i<128;i++) ((guint32 *)ce1->data)[i]=0;
	} else {
		nb = GUINT32_FROM_LE(*bref);
		if (clone) ce1 = ib1_cache_map_clone (efs, nb);
		else ce1 = ib1_cache_map (efs, nb, FALSE);

		if (nb != ce1->block) {
			*bref = GUINT32_TO_LE(ce1->block);
			ib1_cache_touch(ce, TRUE);
		}
	}

	return ce1;
}

static IB1CacheEntry*
ib1_ibcache_lookup (IB1EFS *efs, guint32 inode, guint32 block, gboolean clone)
{
	guint32 ibb, ind;

#ifndef USE_IBCACHE 
	return NULL;
#endif

	ibb = 0;
	ind = (inode+block)%IB1_IBCACHE_SIZE;

	if ((efs->ibcache[ind].inode == inode) && 
	    (efs->ibcache[ind].iblk == block)) {
		ibb = efs->ibcache[ind].block;
		if (clone && (ibb < efs->head.cb)) {
			efs->ibcache[ind].inode = 0;
			return NULL;
		}
	}

	if (ibb) return ib1_cache_map (efs, ibb, FALSE);
	return NULL;
}

static void
ib1_ibcache_add (IB1EFS *efs, guint32 inode, guint32 block, guint32 blk)
{
	guint32 ind;
	
	ind = (inode+block)%IB1_IBCACHE_SIZE;

	efs->ibcache[ind].inode = inode;
	efs->ibcache[ind].iblk = block;
	efs->ibcache[ind].block = blk;
}

IB1CacheEntry*
ib1_inode_bmap (IB1EFS *efs, guint32 inode, guint32 block, gboolean clone)
{
	IB1CacheEntry *ce, *ce1, *ce2, *ce3, *ce4;
	IB1INode *node;
	guint32 cb;

	if (block >= IB1_N_BLOCKS-3+127+127*127+127*127*127) return NULL;

	if ((ce = ib1_ibcache_lookup (efs, inode, block, clone))) return ce;

	if (!(ce = ib1_inode_map (efs, inode, clone))) return NULL;
	node = NODEP(ce, inode);
	if (block > GUINT32_FROM_LE(node->blocks)) return NULL; /* no holes */ 

	if (block == GUINT32_FROM_LE(node->blocks)) {
		if (!clone) return NULL;
		if (!((((EFS *)efs)->mode)&EFS_WRITE)) return NULL;
		node->blocks = GUINT32_TO_LE(block+1);
		ib1_cache_touch (ce, TRUE);
	} 

	if (block<(IB1_N_BLOCKS-3)) { 
		ce1 = map_ind (efs, ce, &node->block[block], clone);
		return ce1;
	}
	
	cb = block - (IB1_N_BLOCKS-3);
	if (cb < 127) {
		ce1 = map_ind(efs, ce, &node->block[IB1_N_BLOCKS-3], clone);
		if (!ce1) return NULL;
		ce2 = map_ind(efs,ce1,&(((guint32*)ce1->data)[cb]),clone);
		if (ce2) ib1_ibcache_add (efs, inode, block, ce2->block); 
		return ce2;
	}

	cb -= 127;
	if (cb<127*127) {
		ce1=map_ind(efs,ce,&node->block[IB1_N_BLOCKS-2],clone);
		if (!ce1) return NULL;
		ce2=map_ind(efs,ce1,&(((guint32*)ce1->data)[cb/127]),clone);
		if (!ce2) return NULL;
		ce3=map_ind(efs,ce2,&(((guint32*)ce2->data)[cb%127]),clone);
		if (ce3) ib1_ibcache_add (efs, inode, block, ce3->block); 
		return ce3;
	}

	cb -= 127*127;
	ce1 = map_ind (efs, ce, &node->block[IB1_N_BLOCKS-1], clone);
	if (!ce1) return NULL;
	ce2 = map_ind (efs, ce1, &(((guint32*)ce1->data)[cb/(127*127)]), clone);
	if (!ce2) return NULL;
	ce3 = map_ind (efs,ce2,&(((guint32*)ce2->data)[(cb/127)%127]),clone);
	if (!ce3) return NULL;
	ce4 = map_ind (efs, ce3, &(((guint32*)ce3->data)[cb%127]), clone);
	if (ce4) ib1_ibcache_add (efs, inode, block, ce4->block); 
	return ce4;
}

gint
ib1_inode_ref (IB1EFS *efs, guint32 inode)
{
	GList *l;
	IB1INodeLEntry *e;

	l = efs->inode_list;
	while (l) {
		e = (IB1INodeLEntry *)l->data;
		if (e->inode == inode) {
			e->ref_count++;
			return e->ref_count;
		}
		l = l->next;
	}
	
	e = g_new0 (IB1INodeLEntry, 1);
	e->inode = inode;
	e->ref_count = 1;
	efs->inode_list = g_list_prepend (efs->inode_list, e);

	return 1;
}

gint
ib1_inode_unref (IB1EFS *efs, guint32 inode)
{
	GList *l;
	IB1INodeLEntry *e;

	l = efs->inode_list;
	while (l) {
		e = (IB1INodeLEntry *)l->data;
		if (e->inode == inode) {
			e->ref_count--;
			if (!e->ref_count) {
				if (e->erase) 
					ib1_inode_erase_real (efs, inode);
				
				efs->inode_list =
					g_list_remove (efs->inode_list, e);
				g_free (e);
				return 0;
			} else return e->ref_count;
		}
		l = l->next;
	}
	
	return -1;
}

gint
ib1_inode_refcount (IB1EFS *efs, guint32 inode)
{
	GList *l;
	IB1INodeLEntry *e;

	l = efs->inode_list;
	while (l) {
		e = (IB1INodeLEntry *)l->data;
		if (e->inode == inode) return e->ref_count;
		l = l->next;
	}
	
	return -1;
}

void
ib1_inode_list_free (IB1EFS *efs)
{
	GList *l;

	l = efs->inode_list;
	while (l) {
		IB1INodeLEntry *le = (IB1INodeLEntry *)l->data;
		if (le->erase) ib1_inode_erase_real (efs, le->inode);
		g_free (l->data);
		l = l->next;
	}
	g_list_free (efs->inode_list);
	efs->inode_list = NULL;
}
