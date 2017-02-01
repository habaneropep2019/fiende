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

/* medusa-queue.c -- A simple FIFO structure */

#include <glib.h>
#include "medusa-queue.h"

struct MedusaQueue {
  GSList *elements;
  GSList *tail;
}; 


MedusaQueue *    
medusa_queue_new (void)
{
  MedusaQueue *queue;

  queue = g_new0 (MedusaQueue, 1);
  queue->elements = NULL;
  queue->tail = NULL;

  return queue;
}


void             
medusa_queue_add (MedusaQueue *queue,
		  gpointer element)
{
  g_return_if_fail (queue != NULL);
  
  if (queue->tail == NULL) {
    g_assert (queue->elements == NULL);
    queue->tail = g_slist_append (queue->tail,
				  element);
    queue->elements = queue->tail;
  }
  else {
    queue->tail = g_slist_append (queue->tail,
				  element);
    queue->tail = queue->tail->next;
  }
}

gpointer         
medusa_queue_remove (MedusaQueue *queue)
{
  gpointer result;
  GSList *old_cell;

  g_return_val_if_fail (queue != NULL, NULL);
  if (queue->elements == NULL) {
    return NULL;
  }

  result = queue->elements->data;

  old_cell = queue->elements;
  queue->elements = queue->elements->next;
  if (queue->elements == NULL) {
    g_assert (queue->tail == old_cell);
    queue->tail = NULL;
  }
  g_slist_free_1 (old_cell);
  return result;
}

gboolean 
medusa_queue_is_empty (MedusaQueue *queue)
{
  return queue->elements == NULL;
}
					     
void
medusa_queue_free (MedusaQueue *queue)
{
  /* As with glist, we can't be responsible for 
     freeing the queue elements */
  g_slist_free (queue->elements);
  /* No need to free the tail, it is a 
     pointer within the elements list */
  g_free (queue);
}

/* Free each element of the queue still left with a g_free */
void             
medusa_queue_free_deep (MedusaQueue *queue)
{
  gpointer element;
  while (!medusa_queue_is_empty (queue)) {
    element = medusa_queue_remove (queue);
    g_free (element);
  }
  medusa_queue_free (queue);
}
