/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-context.c - context VFS modules can use to communicate with gnome-vfs proper

   Copyright (C) 1999 Free Software Foundation

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

   Author: Havoc Pennington <hp@redhat.com> */

#include <config.h>
#include "gnome-vfs-context.h"

#include "gnome-vfs-backend-private.h"
#include "gnome-vfs-cancellation.h"
#include "gnome-vfs-private-utils.h"
#include "gnome-vfs-utils.h"
#include <stdio.h>

#if 1
#define DEBUG_MSG (x) printf x
#else
#define DEBUG_MSG (x)
#endif


struct GnomeVFSContext {
        GnomeVFSCancellation *cancellation;

        guint refcount;
};

/* This is a token Context to return in situations
 * where we don't normally have a context: eg, during sync calls
 */
const GnomeVFSContext sync_context = {NULL, 1};

GnomeVFSContext*
gnome_vfs_context_new (void)
{
        GnomeVFSContext *ctx;

        GNOME_VFS_ASSERT_PRIMARY_THREAD;

        ctx = g_new0(GnomeVFSContext, 1);

        ctx->cancellation = gnome_vfs_cancellation_new();

        ctx->refcount = 1;
 
        return ctx;
}

void
gnome_vfs_context_ref (GnomeVFSContext *ctx)
{
        g_return_if_fail(ctx != NULL);

	/* FIXME: this function should be removed in Gnome 2.0 GnomeVFS */
  	g_warning ("Warning call to deprecated function '%s'\n", G_GNUC_FUNCTION);
  	
        ctx->refcount += 1;
}

/* Note: _unref should be replaced with a _free function in the gnome 2.0 platform */ 
void
gnome_vfs_context_unref (GnomeVFSContext *ctx)
{
        g_return_if_fail(ctx != NULL);
        g_return_if_fail(ctx->refcount > 0);
  
        if (ctx->refcount == 1) {
                gnome_vfs_cancellation_destroy(ctx->cancellation);

                g_free(ctx);
        } else {
                ctx->refcount -= 1;
        }
}

GnomeVFSCancellation*
gnome_vfs_context_get_cancellation (const GnomeVFSContext *ctx)
{
        g_return_val_if_fail(ctx != NULL, NULL);
        return ctx->cancellation;
}

const GnomeVFSContext *
gnome_vfs_context_peek_current		  (void)
{
	const GnomeVFSContext *ret;
	
	gnome_vfs_backend_get_current_context ((GnomeVFSContext **)&ret);

	/* If the context is NULL, then this must be a synchronous call */
	if (ret == NULL) {
		ret = &sync_context;
	}

	return ret;
}

gboolean
gnome_vfs_context_check_cancellation_current (void)
{
	const GnomeVFSContext *current_ctx;

	current_ctx = gnome_vfs_context_peek_current ();

	if (current_ctx == &sync_context) {
		return FALSE;
	} else if (current_ctx != NULL) {
		return gnome_vfs_cancellation_check (gnome_vfs_context_get_cancellation (current_ctx));
	} else {
		return FALSE;
	}	
}
