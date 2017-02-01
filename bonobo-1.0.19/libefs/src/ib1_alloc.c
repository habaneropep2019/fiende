/* ib1_alloc.c: IB1 driver block allocation routines

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

#include "ib1_driver.h"

/**
 * ib1_bitmap_init:
 * @efs:
 * @first_block: block offset
 *
 * Description: initialize the block allocation bitmap. The
 * block before @first_block can´t be free´d.
 */

void
ib1_bitmap_init (IB1EFS *efs, guint32 first_block)
{
	efs->bmap.fb = first_block;
	efs->bmap.bc = 0;
	efs->bmap.dbc = 0;
	efs->bmap.alen = 512;
	efs->bmap.data = g_malloc0 (efs->bmap.alen);
	efs->bmap.fbc = g_malloc0 (efs->bmap.alen);
}

/**
 * ib1_bitmap_free:
 * @efs:
 *
 * Description: free´s allocatee resources in the bitmap.
 */

void
ib1_bitmap_free (IB1EFS *efs)
{
	gint i;

	for (i=0; i < efs->bmap.dbc; i++) 
		if (efs->bmap.data[i]) g_free (efs->bmap.data[i]);

	g_free (efs->bmap.data);
	g_free (efs->bmap.fbc);
}

static guint32
ib1_block_init (IB1EFS *efs, guint32 block)
{
	IB1CacheEntry *ce;
	guint32 *buf;
	gint i;

	if (block != (efs->bmap.bc+efs->bmap.fb)) return 0;
	if (!block) return 0;
	
	if (!(ce = ib1_cache_map (efs, block, TRUE))) return 0;
	buf = (guint32 *) ce->data;
	for (i = 0; i < 128; i++) buf[i] = 0;
	
	return block;
}

/**
 * ib1_block_alloc:
 * @efs:
 *
 * Description: allocates a free block.
 *
 * Returns: the block number of the allocated block, or zero on failure. 
 */

guint32
ib1_block_alloc(IB1EFS *efs)
{
	guint b,i,j,k,rb;
	guint32 rval;

	g_return_val_if_fail (((EFS*)efs)->mode&EFS_WRITE, 0);

	for (b = 0; b < efs->bmap.dbc; b++) if (efs->bmap.fbc[b] > 0) {
		for (i=0;i<128;i++) if (efs->bmap.data[b][i] != 0xffffffff) {
			k = 1<<31;
			for (j=0;j<32;j++) {
				rb = b*512*8+i*32+j;
				if ((efs->bmap.data[b][i]&k) == 0) {
					efs->bmap.data[b][i] |= k;
					rval = efs->bmap.fb + rb;
					if (rb >= efs->bmap.bc) {
						rval = ib1_block_init 
							(efs, rval);
						if (rval) efs->bmap.bc = rb + 1;
					}
					if (rval) efs->bmap.fbc[b]--;
					return rval;
				}
				k=k>>1;
			}
		}
	}
	
	i = efs->bmap.bc;
	
	if ((rval = ib1_block_init (efs, efs->bmap.fb + i))) {
		efs->bmap.bc++;
		if (efs->bmap.bc >= (efs->bmap.dbc*512*8)) {
			if (efs->bmap.dbc >= efs->bmap.alen) {
				efs->bmap.alen += 512;
				efs->bmap.data = g_realloc (efs->bmap.data, 
							    efs->bmap.alen);
				efs->bmap.fbc = g_realloc (efs->bmap.fbc, 
							   efs->bmap.alen);
			}

			efs->bmap.data[efs->bmap.dbc] = g_malloc0(512);
			efs->bmap.fbc[efs->bmap.dbc] = 512*8;
			efs->bmap.dbc++;
		}

		b = i/(512*8);
		j = (i%(512*8))/32;
		k = 1 << (31-(i%(512*8))%32);
	
		efs->bmap.data[b][j] |= k;
		efs->bmap.fbc[b]--;
	}

	return rval;	
}

gint
ib1_block_get_fbc (IB1EFS *efs)
{
	gint i, db, ind, bi;
	gint free = 0;
	gint lb = 0;

	for (i = 0; i < efs->bmap.bc; i++) {
		db = i/(512*8);
		ind = (i%(512*8))/32;
		bi = 1 << (31-((i%(512*8))%32));
		if (efs->bmap.data[db][ind]&bi) lb = i;
		else free++;
	}
	
	free -= (efs->bmap.bc -1 - lb);
	efs->bmap.bc = lb+efs->bmap.fb;

	return free;
}

/**
 * ib1_block_free:
 * @efs:
 * @block: block number to free
 *
 * Description: marks @block as free.
 */

void   
ib1_block_free (IB1EFS *efs, guint32 block)
{
	guint b,j,k;

	g_return_if_fail (block >= efs->bmap.fb);

	for (j=0; j < IB1_CACHE_SIZE; j++) if (efs->cache[j].block == block)
		efs->cache[j].dirty = FALSE;

	block -= efs->bmap.fb;
	b = block/(512*8);
	j = (block%(512*8))/32;
	k = 1 << (31-(block%(512*8))%32);

	if (efs->bmap.data[b]) {
		if (!(efs->bmap.data[b][j]&k)) {
			g_warning ("block is not allocated!");
			return;
		}

		efs->bmap.data[b][j] &= ~k;
		efs->bmap.fbc[b]++;
	}
}
