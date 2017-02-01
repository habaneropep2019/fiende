/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 * 
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Rebecca Schulman <rebecka@eazel.com>
 *  
 */

/* medusa-queue.h -- A simple FIFO structure */

#ifndef MEDUSA_QUEUE_H
#define MEDUSA_QUEUE_H

#include <glib.h>

typedef struct MedusaQueue MedusaQueue;

MedusaQueue *    medusa_queue_new           (void);
/* Enqueue */
void             medusa_queue_add           (MedusaQueue *queue,
					     gpointer element);
/* Dequeue */
gpointer         medusa_queue_remove        (MedusaQueue *queue);

gboolean         medusa_queue_is_empty      (MedusaQueue *queue);

void             medusa_queue_free          (MedusaQueue *queue);
/* Free each element of the queue still left with a g_free */
void             medusa_queue_free_deep     (MedusaQueue *queue);
					     
#endif /* MEDUSA_QUEUE_H */
