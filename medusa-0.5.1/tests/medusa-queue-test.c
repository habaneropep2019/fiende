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
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 *
 *  medusa-queue-test.c: Test files in the text indexer directory 
 *
 */

#include <stdio.h>

#include <medusa-queue.h>

int main ()
{
        char *some_elements[] = { "one", "two", "three" };
        char *removed_element;
        MedusaQueue *queue;

        queue = medusa_queue_new ();
        g_assert (medusa_queue_is_empty (queue));
        medusa_queue_add (queue, some_elements[0]);
        g_assert (!medusa_queue_is_empty (queue));
        removed_element = medusa_queue_remove (queue);
        g_assert (removed_element == some_elements[0]);
        g_assert (medusa_queue_is_empty (queue));

        medusa_queue_add (queue, some_elements[0]);
        medusa_queue_add (queue, some_elements[1]);
        medusa_queue_add (queue, some_elements[2]);
        g_assert (medusa_queue_remove (queue) == some_elements[0]);
        g_assert (medusa_queue_remove (queue) == some_elements[1]);
        g_assert (medusa_queue_remove (queue) == some_elements[2]);
        g_assert (medusa_queue_is_empty (queue));
        medusa_queue_free (queue);

        printf ("One, two, three, this queue's good enough for me!\n");
        printf ("all tests passed\n");
  
        return 0;
}
