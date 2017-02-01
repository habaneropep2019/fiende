/* queue functions (for forward/backward movement */

#include <glib.h>

#include "queue.h"

struct _Queue {
	GList  *queue;
	GList  *current;
};

struct _queue_element {
    gchar *ref;
    gint pos;
};

Queue
queue_new(void)
{
	Queue  h;

	h = g_malloc(sizeof(*h));

	h->queue = NULL;
	h->current = NULL;
	return h;
}

static void
queue_free_element( struct _queue_element *el, gpointer foo )
{
        g_free(el->ref);
	g_free(el);
}

void
queue_free(Queue h)
{
	g_return_if_fail( h != NULL );

	/* needs to free data in list as well! */
	if (h->queue) {
		g_list_foreach(h->queue, (GFunc)queue_free_element, NULL);
		g_list_free(h->queue);
	}

	g_free(h);
}

void
queue_move_prev(Queue h)
{
	if (!h || !h->queue || (h->current == g_list_first(h->queue)))
		return;

	h->current = g_list_previous(h->current);
}

void
queue_move_next(Queue h)
{
	if (!h || !h->queue || (h->current == g_list_last(h->queue)))
		return;

	h->current = g_list_next(h->current);
}

gchar
*queue_prev(Queue h, gint *pos)
{
	GList *p;

	if (!h || !h->queue || (h->current == g_list_first(h->queue)))
		return NULL;

	p = g_list_previous(h->current);

	if (pos) {
	        *pos = ((struct _queue_element *)p->data)->pos;
	}
	return ((struct _queue_element *)p->data)->ref;
}

gchar
*queue_next(Queue h, gint *pos)
{
	GList *p;


	if (!h || !h->queue || (h->current == g_list_last(h->queue)))
		return NULL;

	p = g_list_next(h->current);

	if (pos) {
	        *pos = ((struct _queue_element *)p->data)->pos;
	}
	return ((struct _queue_element *)p->data)->ref;
}

void 
queue_mark_current(Queue h, gint pos)
{
    if (h->current) {
	((struct _queue_element *)(h->current->data))->pos = pos;
    }
}

void 
queue_add(Queue h, gchar *ref, gint pos)
{
	GList *trash=NULL;
	struct _queue_element *el;

	g_return_if_fail( h != NULL );
	g_return_if_fail( ref != NULL );

	if (h->current) {
		trash = h->current->next;
		h->current->next = NULL;
	}

	el = g_malloc(sizeof(*el));
	el->pos = pos;
	el->ref = g_strdup(ref);
	h->queue = g_list_append(h->queue, el);
	h->current = g_list_last(h->queue);

	if (trash) {
		g_list_foreach(trash, (GFunc)queue_free_element, NULL);
		g_list_free(trash);
	}
	
}

gboolean
queue_isnext(Queue h)
{
	if (!h || !h->queue || (h->current == g_list_last(h->queue)))
		return FALSE;

	return (g_list_next(h->current) != NULL);
}

gboolean
queue_isprev(Queue h)
{
	if (!h || !h->queue || (h->current == g_list_first(h->queue)))
		return FALSE;

	return (g_list_previous(h->current) != NULL);
}


