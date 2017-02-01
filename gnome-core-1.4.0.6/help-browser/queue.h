#ifndef _GNOME_HELP_QUEUE_H_
#define _GNOME_HELP_QUEUE_H_

#include <glib.h>

typedef struct _Queue *Queue;

Queue queue_new(void);
void queue_free(Queue h);
gchar *queue_prev(Queue h, gint *pos);
gchar *queue_next(Queue h, gint *pos);
void queue_move_prev(Queue h);
void queue_move_next(Queue h);
void queue_add(Queue h, gchar *ref, gint pos);
void queue_mark_current(Queue h, gint pos);
gboolean queue_isnext(Queue h);
gboolean queue_isprev(Queue h);

#endif
