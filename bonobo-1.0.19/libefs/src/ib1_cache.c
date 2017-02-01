/* ib1_cache.c: a simple cache for blocks (fully associative with LRU 
   replacement)

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

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "ib1_driver.h"

#include <sys/types.h>
#include <signal.h>

static guint32 cache_access_time = 0;

static gint
ib1_cache_unmap (IB1EFS *efs, IB1CacheEntry *ce)
{
	guint32 buf[128];
	guint32 crc = adler32 (0L, NULL, 0);

	g_return_val_if_fail (ce->block >= efs->head.cb, -1);
	if (lseek(efs->fd, ce->block*512, SEEK_SET) == ce->block*512) {
		memcpy (buf, ce->data, 512);
		crc = adler32 (crc, (guint8 *)buf, 508);
		buf[127] = GUINT32_TO_LE (crc);
		if (efs->head.head.protected&&((EFS*)efs)->driver->encrypted) 
			ib1_encrypt (efs, buf, 512/4);
		write(efs->fd, buf, 512);
		return 0;
	}
	return -1;
}

void
ib1_cache_flush (IB1EFS *efs)
{
	gint i;

	for (i=0;i<IB1_CACHE_SIZE;i++) {
		if (efs->cache[i].dirty) {
			ib1_cache_unmap(efs, &efs->cache[i]);
			efs->cache[i].dirty = FALSE;
		}
	}
}

void
ib1_cache_touch (IB1CacheEntry *ce, gboolean dirty)
{
	cache_access_time++;
	ce->at = cache_access_time;
	if (dirty) ce->dirty = dirty;
}

IB1CacheEntry*
ib1_cache_map (IB1EFS *efs, guint32 block, gboolean noread)
{
	gint i, b;

	b = -1;
	for (i=0;i<IB1_CACHE_SIZE;i++) {
		if (efs->cache[i].block == block) {
			efs->cache[i].at = ++cache_access_time;
			return &efs->cache[i];
		}
		if (efs->cache[i].lock) continue;
		if (b<0) b=i;
		if (efs->cache[i].at<efs->cache[b].at) b = i;
	}

	if (b<0) g_error ("EFS: internal error (cache look)\n");
	
	if (efs->cache[b].dirty) { 
		ib1_cache_unmap(efs, &efs->cache[b]);
		efs->cache[b].dirty = FALSE;
	}
	
	efs->cache[b].at = ++cache_access_time;
	efs->cache[b].block = block;

	if (!noread) {
		guint32 *buf;
		guint32 crc = adler32 (0L, NULL, 0);
	        if (lseek(efs->fd, block*512, SEEK_SET) != block*512) 
			return NULL; 
		if (read(efs->fd, efs->cache[b].data, 512) != 512) return NULL;
		if (efs->head.head.protected&&((EFS*)efs)->driver->encrypted) 
			ib1_decrypt (efs,(guint32 *)efs->cache[b].data,512/4);
		crc = adler32 (crc, efs->cache[b].data, 508);
		buf = (guint32 *)efs->cache[b].data;
		if (crc != GUINT32_FROM_LE (buf[127])) return NULL;
		efs->cache[b].dirty = FALSE;
	} else {
		efs->cache[b].dirty = TRUE;
	}

	return &efs->cache[b];
}

IB1CacheEntry*
ib1_cache_map_clone (IB1EFS *efs, guint32 block)
{
	IB1CacheEntry *ce, *ce1;
	guint32 nb;

	if (!(ce = ib1_cache_map (efs, block, FALSE))) return NULL;
	if (block >= efs->head.cb) return ce;

	if (!(nb = ib1_block_alloc (efs))) return NULL;
	if (!(ce1 = ib1_cache_map (efs, nb, TRUE))) return NULL;

	memcpy (ce1->data, ce->data, 512);
	return ce1;
}
