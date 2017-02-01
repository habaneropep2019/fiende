/* Keep trace of idle in a list; block remove all idle, and insert 
   re-add all idle */

#include "idle.h"

#include "gtk/gtk.h"

static gint   id   = 0;
static GList *list = NULL;
gboolean remove = TRUE;

typedef struct idle_t {
  GtkFunction function;
  gpointer data;
  
  int id;
  int gtk_id;
} idle_t;

static idle_t *
idle_search_by_id (int id)
{
  GList *l = list;
  idle_t *idle;

  while (l) {
    idle = l->data;
    
    if (idle->id == id) return idle;
    
    l = g_list_next (l);
  }

  return NULL;
}

static idle_t *
idle_search_by_data (gpointer data)
{
  GList *l = list;
  idle_t *idle;

  while (l) {
    idle = l->data;
    
    if (idle->data == data) return idle;
    
    l = g_list_next (l);
  }

  return NULL;
}

static void
destroy (gpointer data)
{
  idle_t *idle;

  if (remove) {
    idle = idle_search_by_data (data);
    
    if (idle) 
      list = g_list_remove (list, idle);
  }
}

int
idle_add (GtkFunction function, gpointer data)
{
  idle_t *idle = g_new0 (idle_t, 1);

  idle->id = ++id;
  idle->function = function;
  idle->data = data;

  idle->gtk_id = gtk_idle_add_full (GTK_PRIORITY_DEFAULT, function, 
				    NULL, data, destroy);

  list = g_list_append (list, idle);

  return idle->id;
}

void 
idle_remove (int id)
{
  idle_t *idle = idle_search_by_id (id);

  if (idle) 
    gtk_idle_remove (idle->gtk_id); /* call destroy */
}

void
idle_block (void)
{
  GList *l = list;
  idle_t *idle;

  remove = FALSE; /* destroy not called */

  while (l) {
    idle = l->data;

    l = g_list_next (l);
    
    if (idle->gtk_id) {
      gtk_idle_remove (idle->gtk_id);
      idle->gtk_id = 0;
    }
  }  

  remove = TRUE;
}

void
idle_unblock (void)
{
  GList *l = list;
  idle_t *idle;

  while (l) {
    idle = l->data;

    if (!idle->gtk_id) {
      idle->gtk_id = gtk_idle_add_full (GTK_PRIORITY_DEFAULT, idle->function, 
					NULL, idle->data, destroy);
    }

    l = g_list_next (l);
  }
}
