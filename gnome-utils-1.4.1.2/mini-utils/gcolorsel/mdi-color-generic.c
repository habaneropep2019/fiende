#include "mdi-color-generic.h"
#include "view-color-generic.h"
#include "menus.h"
#include "utils.h"
#include "gcolorsel.h"

#include <gnome.h>
#include <glade/glade.h>

static void mdi_color_generic_class_init       (MDIColorGenericClass *class);
static void mdi_color_generic_init             (MDIColorGeneric *mcg);
static void mdi_color_generic_destroy          (GtkObject *);
static void mdi_color_generic_document_changed (MDIColorGeneric *mcg,
						gpointer data);
static GtkWidget *mdi_color_generic_create_view      (MDIColorGeneric *mcg);						

static void mdi_color_generic_free_col         (MDIColor *col);
static GList *mdi_color_generic_find_col       (MDIColorGeneric *mcg, int pos);

static void mdi_color_generic_clear_all_docs (MDIColorGeneric *mcg);
static void mdi_color_generic_set_all_color_change (MDIColorGeneric *mcg, 
						    int val);

static gpointer
mdi_color_generic_get_control       (MDIColorGeneric *vcg, GtkVBox *box,
				     void (*changed_cb)(gpointer data), 
				     gpointer change_data);
static void mdi_color_generic_apply (MDIColorGeneric *mcg, gpointer data);
static void mdi_color_generic_close (MDIColorGeneric *mcg, gpointer data);
static void mdi_color_generic_sync  (MDIColorGeneric *mcg, gpointer data);

static void mdi_color_generic_load  (MDIColorGeneric *mcg);
static void mdi_color_generic_save  (MDIColorGeneric *mcg);

static const gchar * get_config_string (GnomeMDIChild *child);

enum {
  DOCUMENT_CHANGED,
  LAST_SIGNAL
};

static guint mcg_signals [LAST_SIGNAL] = { 0 };

static GnomeMDIChildClass *parent_class = NULL;

guint 
mdi_color_generic_get_type()
{
  static guint mdi_gen_child_type = 0;
  
  if (!mdi_gen_child_type) {
    GtkTypeInfo mdi_gen_child_info = {
     "MDIColorGeneric",
      sizeof (MDIColorGeneric),
      sizeof (MDIColorGenericClass),
      (GtkClassInitFunc) mdi_color_generic_class_init,
      (GtkObjectInitFunc) mdi_color_generic_init,
      (GtkArgSetFunc) NULL,
      (GtkArgGetFunc) NULL,
    };
    
    mdi_gen_child_type = gtk_type_unique (gnome_mdi_child_get_type (), 
					  &mdi_gen_child_info);
  }
  
  return mdi_gen_child_type;
}

static void 
mdi_color_generic_class_init (MDIColorGenericClass *class)
{
  GtkObjectClass      *object_class;
  GnomeMDIChildClass  *mdi_child_class;

  object_class          = GTK_OBJECT_CLASS (class);
  mdi_child_class       = GNOME_MDI_CHILD_CLASS (class);
  parent_class          = gtk_type_class (gnome_mdi_child_get_type());

  mcg_signals [DOCUMENT_CHANGED] = 
    gtk_signal_new ("document_changed",
		    GTK_RUN_LAST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (MDIColorGenericClass, document_changed),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);

  gtk_object_class_add_signals (object_class, mcg_signals, LAST_SIGNAL);

  mdi_child_class->create_view = (GnomeMDIChildViewCreator)mdi_color_generic_create_view;

  class->document_changed = mdi_color_generic_document_changed;
  class->get_control      = mdi_color_generic_get_control;
  class->apply            = mdi_color_generic_apply;
  class->close            = mdi_color_generic_close;
  class->sync             = mdi_color_generic_sync;
  class->load             = mdi_color_generic_load;
  class->save             = mdi_color_generic_save;

  object_class->destroy   = mdi_color_generic_destroy;

  mdi_child_class->get_config_string = (GnomeMDIChildConfigFunc)get_config_string;
}

static void
mdi_color_generic_init (MDIColorGeneric *mcg)
{
  mcg->key  = get_config_key ();
  mcg->name = NULL;

  mcg->modified         = FALSE;
  mcg->monitor_modified = FALSE;

  mcg->freeze_count = 0;
  mcg->last         = -1;
  mcg->col          = NULL;
  mcg->changes      = NULL;
  mcg->docs         = NULL;
  mcg->other_views  = NULL;
  mcg->parents      = NULL;

  mcg->get_owner        = NULL;
  mcg->get_append_pos   = NULL;
  mcg->get_control_type = NULL;
  mcg->changes_phase    = 1;

  mcg->temp = FALSE;

  mcg->flags = CHANGE_APPEND | CHANGE_REMOVE | CHANGE_NAME | CHANGE_POS
                     | CHANGE_RGB | CHANGE_CLEAR;
  
  mcg->views_type = NULL;
}

static void 
mdi_color_generic_destroy (GtkObject *obj)
{
  MDIColorGeneric *mcg = MDI_COLOR_GENERIC (obj);
    
  mdi_color_generic_set_all_color_change (mcg, CHANGE_REMOVE);

  while (mcg->parents) 
    mdi_color_generic_disconnect (mcg->parents->data, mcg);

  mdi_color_generic_clear_all_docs (mcg);    

  while (mcg->docs) 
    mdi_color_generic_disconnect (mcg, mcg->docs->data); 

  mcg->freeze_count++;
  mdi_color_generic_clear (mcg);

  if (mcg->changes) g_list_free (mcg->changes);
  if (mcg->docs) g_list_free (mcg->docs);
  if (mcg->parents) g_list_free (mcg->parents);
  if (mcg->views_type) g_list_free (mcg->views_type);
  if (mcg->other_views) g_list_free (mcg->other_views);
  
  if(GTK_OBJECT_CLASS(parent_class)->destroy)
    (* GTK_OBJECT_CLASS(parent_class)->destroy)(obj);
}

static const gchar * 
get_config_string (GnomeMDIChild *child) 
{
  return g_strdup_printf ("%d", MDI_COLOR_GENERIC (child)->key);
}

static void
mdi_color_generic_set_all_color_change (MDIColorGeneric *mcg, int val)
{
  GList *list = mcg->col;
  MDIColor *col;

  while (list) {
    col = list->data;
    col->change = val;
    list = g_list_next (list);
  }
}

static void
mdi_color_generic_view_realize (GtkWidget *widget, ViewColorGeneric *view)
{
  /* Quick hack. Send all the color to the newly created view.
     Humm ... col->change should be null, so no problem with that */

  mdi_color_generic_set_all_color_change (view->mcg, CHANGE_APPEND);
  view_color_generic_data_changed (view, view->mcg->col);
  mdi_color_generic_set_all_color_change (view->mcg, 0);

  gtk_signal_disconnect_by_func (GTK_OBJECT (widget), 
				 mdi_color_generic_view_realize, view);
}

static GtkWidget * 
mdi_color_generic_create_view (MDIColorGeneric *child)
{
  GtkWidget *vbox;
  GtkWidget *sw = NULL;
  ControlGeneric *control = NULL;
  ViewColorGeneric *view = NULL;
  int type;
  views_t *views;
  
  vbox = gtk_vbox_new (FALSE, 0);

  /* Create control */

  type = mdi_color_generic_get_control_type (child);
  if (type) {
    control = CONTROL_GENERIC (gtk_type_new (mdi_color_generic_get_control_type (child)));
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (control), FALSE, FALSE, 0);
  
    control_generic_assign (control, child);
  }

  if (child->views_type) {    
    type = GPOINTER_TO_INT (child->views_type->data);
    child->views_type = g_list_remove (child->views_type, 
				       child->views_type->data);

    views = get_views_from_type (type);
  } else
    views = &views_tab[0];

  /* Create scrolled window */

  if ((views->scroll_h_policy != GTK_POLICY_NEVER) ||
      (views->scroll_v_policy != GTK_POLICY_NEVER)) {
    
    sw = gtk_scrolled_window_new (NULL, NULL);  
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    views->scroll_h_policy,
				    views->scroll_v_policy);
  }

  /* Create view */

  view = VIEW_COLOR_GENERIC (views->new (child));

  /* Pack view */

  if (sw) {
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (sw), view->widget);
  } else
    gtk_box_pack_start (GTK_BOX (vbox), view->widget, TRUE, TRUE, 0);

  /* Set view */

  gtk_object_set_data (GTK_OBJECT (vbox), "view_object", view);

  view->control = control ? CONTROL_GENERIC (control) : NULL;
  view->show_control = control ? TRUE : FALSE;

  gtk_signal_connect_after (GTK_OBJECT (view->widget), "realize",
		      GTK_SIGNAL_FUNC (mdi_color_generic_view_realize), view);

  /* Sync control */

  if (control) 
    control_generic_sync (CONTROL_GENERIC (control));

  gtk_widget_show_all (vbox);

  gtk_signal_connect_object (GTK_OBJECT (vbox), "destroy",
			     GTK_SIGNAL_FUNC (gtk_object_destroy), 
			     GTK_OBJECT (view));

  return vbox;
}

GtkWidget * 
mdi_color_generic_create_other_view (MDIColorGeneric *child)
{
  GtkWidget *view = mdi_color_generic_create_view (child);
  
  child->other_views = g_list_append (child->other_views, view);
  
  gtk_object_ref (GTK_OBJECT (child));
  
  return view;
}

gboolean 
mdi_color_generic_can_do (MDIColorGeneric *mcg, 
			  MDIColorChangeType what)
{
  return mcg->flags & what;
}

void
mdi_color_generic_freeze (MDIColorGeneric *mcg)
{
  GList *list;

  mcg->freeze_count++;

  list = mcg->docs;
  while (list) {
    mdi_color_generic_freeze (list->data);
    list = g_list_next (list);
  }
}

void
mdi_color_generic_thaw (MDIColorGeneric *mcg)
{
  GList *list;

  if (!mcg->freeze_count) return;

  mcg->freeze_count--;

  if (!mcg->freeze_count) 
    mdi_color_generic_dispatch_changes (mcg);  

  list = mcg->docs;
  while (list) {
    mdi_color_generic_thaw (list->data);
    list = g_list_next (list);
  }
}

static void 
mdi_color_generic_document_changed (MDIColorGeneric *mcg, gpointer data)
{
  /*  g_assert_not_reached (); */
}

static GList*
mdi_color_generic_find_col (MDIColorGeneric *mcg, int pos)
{
  MDIColor *col;
  GList *list = mcg->col;

  while (list) {
    col = list->data;
    if (col->pos == pos) return list;
    list = g_list_next (list);
  }

  return NULL;
}

static void 
append_change (MDIColorGeneric *mcg, MDIColor *col)
{
  if (mcg->changes) { /* 1 or more in list */
    g_list_append (mcg->last_changes, col);
    mcg->last_changes = mcg->last_changes->next;
  } else  /* Empty */
    mcg->changes = mcg->last_changes = g_list_append (mcg->changes, col);  
}

void
mdi_color_generic_post_change (MDIColorGeneric *mcg, MDIColor *col,
			       MDIColorChangeType type)
{
  /* 
     Optimisation : Ca ne sert à rien d'ajouter ET de supprimer une couleur
                    Ca ne sert à rien d'ajouter ET de modifier une couleur 
                    Ca ne sert à rien de modifier ET de supprimer une couleur 
  */

  mdi_color_generic_set_modified (mcg, TRUE);  

  if (type == CHANGE_CLEAR) {
    g_list_free (mcg->changes);
    col = g_new0 (MDIColor, 1);
    col->change = CHANGE_CLEAR;
    col->owner = mcg;
    mcg->changes = mcg->last_changes = NULL;
    append_change (mcg, col);
  } else {
    /*    if (! col->change) { */
    if (col->change_phase != mcg->changes_phase) {
      append_change (mcg, col);
      col->change       = type;
      col->change_phase = mcg->changes_phase;
    } else {
      col->change = col->change | type;  
      
      if (col->change & CHANGE_APPEND) {
	if (col->change & CHANGE_REMOVE) { /* 1 */
	  if (mcg->last_changes->data == col) {
	    GList *prev = mcg->last_changes->prev;
	    g_list_remove (mcg->last_changes, col);
	    mcg->last_changes = prev;

	    if (! mcg->last_changes) mcg->changes = NULL;
	  } else
	    mcg->changes = g_list_remove (mcg->changes, col);
	  return;
	}
	
	col->change = CHANGE_APPEND; /* 2 */
      } else 
	if (col->change & CHANGE_REMOVE) col->change = CHANGE_REMOVE; /* 3 */
    }    
  }

  if (!mcg->freeze_count) 
    mdi_color_generic_dispatch_changes (mcg);
}

void
mdi_color_generic_dispatch_changes (MDIColorGeneric *mcg)
{
  ViewColorGeneric *view;
  GList *list;
  
  if (!mcg->changes) return;
  list = GNOME_MDI_CHILD (mcg)->views;
  while (list) {
    view = gtk_object_get_data (GTK_OBJECT (list->data), "view_object");
    if (view) 
      view_color_generic_data_changed (view, mcg->changes);

    list = g_list_next (list);
  }
  
  list = mcg->other_views;
  while (list) {
    view = gtk_object_get_data (GTK_OBJECT (list->data), "view_object");
    if (view)
      view_color_generic_data_changed (view, mcg->changes);
      
    list = g_list_next (list);
  }

  list = mcg->docs;
  while (list) {
  
    gtk_signal_emit_by_name (GTK_OBJECT (list->data), "document_changed", 
              	             mcg->changes);

    list = g_list_next (list);
  }

  g_list_free (mcg->changes);
  mcg->changes = mcg->last_changes = NULL;
  mcg->changes_phase++;
}

static void
mdi_color_generic_free_col (MDIColor *col)
{

}

MDIColor *
mdi_color_generic_append_new (MDIColorGeneric *mcg,
			      int r, int g, int b, char *name)
{
  MDIColor *col;

  col = MDI_COLOR (mdi_color_new ());

  col->r      = r;
  col->g      = g;
  col->b      = b;
  col->name   = g_strdup (name);
  col->change = 0;
  col->change_phase = 0;

  mdi_color_generic_append (mcg, col);

  return col;
}

MDIColor *
mdi_color_generic_append_new_set_data (MDIColorGeneric *mcg, 
				       int r, int g, int b, char *name,
				       char *str, gpointer data)
{
  MDIColor *col = mdi_color_generic_append_new (mcg, r, g, b, name);

  gtk_object_set_data (GTK_OBJECT (col), str, data);

  return col;
}

void
mdi_color_generic_append (MDIColorGeneric *mcg, MDIColor *col)
{
  MDIColor *col2;
  GList *pos;

  mcg->last++;
  col->owner = mcg;
  
  if (! mcg->col) { /* Empty */
    col->list = mcg->col = mcg->last_col = g_list_append (NULL, col);
    col->pos = 0;
  } else {
    pos = mdi_color_generic_get_append_pos (mcg, col);
    if (! pos) /* append last */ {
      g_list_append (mcg->last_col, col);
      col->list = mcg->last_col = mcg->last_col->next;
      col->pos = mcg->last;
    } else {
      if (pos == mcg->col) /* append first */ 
	col->list = mcg->col = g_list_prepend (pos, col);
      else 
	col->list = g_list_prepend (pos, col);

      col->pos = ((MDIColor *)(pos->data))->pos;
	
      /* We need to update position of other colors ! */

      pos = pos;
      while (pos) {
	col2 = pos->data;
	col2->pos++;
	mdi_color_generic_post_change (mcg, col2, CHANGE_POS);
	
	pos = g_list_next (pos);
      }
      
    }
  }

  mdi_color_generic_post_change (mcg, col, CHANGE_APPEND);
}

void 
mdi_color_generic_clear (MDIColorGeneric *mcg)
{ 
  GList *list;

  if (mcg->col) {
    mcg->last = -1;

    list = mcg->col;
    while (list) {
      gtk_object_unref (GTK_OBJECT (list->data));
      
      list = g_list_next (list);
    }
    
    g_list_free (mcg->col);

    mcg->last_col = mcg->col = NULL;
    mdi_color_generic_post_change (mcg, NULL, CHANGE_CLEAR);  
  }
}

static void
remove_col (MDIColorGeneric *mcg, MDIColor *col)
{
  if (col->list != NULL) {
    if (! col->list->next) { /* Remove last */
      if (! col->list->prev) { /* Remove last & list->length = 1 */
        g_list_free (mcg->col);
        mcg->last_col = mcg->col = NULL;
      } else { /* Remove last & list->length > 1 */
        mcg->last_col = col->list->prev;
        g_list_remove (col->list, col);
      }
    } else { /* list->length > 1 */
      if (! col->list->prev) {/* Remove first & list->length > 1*/
        mcg->col = g_list_remove (mcg->col, col);
      } else /* Not first, not last */ {
        g_list_remove (col->list, col);    
      }
    }
    col->list = NULL;
  }
}

void 
mdi_color_generic_remove (MDIColorGeneric *mcg, MDIColor *col)
{
  GList *next;  
  MDIColor *col_tmp;

  next = col->list->next;
  remove_col (mcg, col);

  mdi_color_generic_post_change (mcg, col, CHANGE_REMOVE);
  
  /* We need to update position of other colors ! */
  
  while (next) {
    col_tmp = next->data;
    col_tmp->pos--;
    mdi_color_generic_post_change (mcg, col_tmp, CHANGE_POS);
    
    next = g_list_next (next);
  }

  gtk_object_unref (GTK_OBJECT (col));
    
  mcg->last--;
}

/* list_to_del **have to* be sorted by position */
void
mdi_color_generic_remove_list (MDIColorGeneric *mcg, GList *list_to_del)
{
  GList *list = ((MDIColor *) list_to_del->data)->list;
  MDIColor *col;
  int offset = 0;

  while (list) {
    col = list->data;
    list = g_list_next (list);

    if ((list_to_del)&&(col == list_to_del->data)) {
      remove_col (mcg, col);
      mdi_color_generic_post_change (mcg, col, CHANGE_REMOVE);
      gtk_object_unref (GTK_OBJECT (col));

      offset++;

      list_to_del = g_list_next (list_to_del);
    } else {
      col->pos -= offset;
      mdi_color_generic_post_change (mcg, col, CHANGE_POS);
    }
  }  
}

void 
mdi_color_generic_change_rgb (MDIColorGeneric *mcg, 
			      MDIColor *col,
			      int r, int g, int b)
{
  col->r = r;
  col->g = g;
  col->b = b;

  mdi_color_generic_post_change (mcg, col, CHANGE_RGB);
}

void 
mdi_color_generic_change_name (MDIColorGeneric *mcg, 
			       MDIColor *col,
			       char *name)
{
  g_free (col->name);
  col->name = g_strdup (name);

  mdi_color_generic_post_change (mcg, col, CHANGE_NAME);
}

void 
mdi_color_generic_change_pos (MDIColorGeneric *mcg, 
			      MDIColor *col, int new_pos)
{
  GList *list;
  MDIColor *col_tmp;
  int i;

  if (new_pos == -1) new_pos = mcg->last;

  if (new_pos < col->pos) {
    list = mdi_color_generic_find_col (mcg, new_pos);
    for (i = new_pos; i < col->pos && list; i++, list = g_list_next (list)) {
      col_tmp = list->data;
      col_tmp->pos++;
      mdi_color_generic_post_change (mcg, col_tmp, CHANGE_POS);
    }

  } else

    if (new_pos > col->pos) {     
      list = mdi_color_generic_find_col (mcg, new_pos);
      for (i = new_pos; i > col->pos && list; i--, list = g_list_previous (list)){
	col_tmp = list->data;
	col_tmp->pos--;
	mdi_color_generic_post_change (mcg, col_tmp, CHANGE_POS);
      }

    } else 
      return;

  /*  mcg->col = g_list_remove (mcg->col, col); */
  remove_col (mcg, col);
  
  col->pos = new_pos;
  mcg->col = g_list_insert (mcg->col, col, new_pos);

  mdi_color_generic_post_change (mcg, col, CHANGE_POS);
}

MDIColor *
mdi_color_generic_search_by_data (MDIColorGeneric *mcg, char *str, 
				  gpointer data)
{
  GList *list;
  GtkObject *col;

  list = mcg->col;
  while (list) {
    col = list->data;
    
    if (gtk_object_get_data (col, str) == data) return MDI_COLOR (col);

    list = list->next;
  }

  return NULL;
}

/* 'To' sera avertit par mcg quand celui est modifié. */
void
mdi_color_generic_connect (MDIColorGeneric *mcg,
			   MDIColorGeneric *to)
{
  mcg->docs = g_list_prepend (mcg->docs, to);
  to->parents = g_list_prepend (to->parents, mcg);
  
  /* send append request for 'mcg color' to connected document 'to' */

  mdi_color_generic_set_all_color_change (mcg, CHANGE_APPEND);
  gtk_signal_emit_by_name (GTK_OBJECT (to), "document_changed", mcg->col);
  /*  mdi_color_generic_set_all_color_change (mcg, 0); */
  mcg->changes_phase++;
}

void mdi_color_generic_disconnect (MDIColorGeneric *mcg,
				   MDIColorGeneric *to)
{
  mcg->docs = g_list_remove (mcg->docs, to);
  to->parents = g_list_remove (to->parents, mcg);
}

static void 
mdi_color_generic_clear_all_docs (MDIColorGeneric *mcg)
{
  GList *list = mcg->docs;
  gboolean done = FALSE;
  MDIColorGeneric *to;

  while (list) {
    to = list->data;

    if (g_list_length (to->parents) == 1) {
      MDIColor tmp;
      GList list;
      
      tmp.change = CHANGE_CLEAR;
      tmp.owner = mcg;
      
      list.data = &tmp;
      list.next = list.prev = NULL;
      
      gtk_signal_emit_by_name (GTK_OBJECT (to), "document_changed", &list);
    } else {
      if (!done) {
	mdi_color_generic_set_all_color_change (mcg, CHANGE_REMOVE);
	done = TRUE;
      }
      gtk_signal_emit_by_name (GTK_OBJECT (to), "document_changed", mcg->col);
    }

    list = g_list_next (list);
  }
  
  if (done)
    mcg->changes_phase++;
}

MDIColor *
mdi_color_generic_get_owner (MDIColor *col)
{
  if (col->owner->get_owner)
    return col->owner->get_owner (col);

  return col;
}

GList *
mdi_color_generic_get_append_pos (MDIColorGeneric *mcg, 
				  MDIColor *col)
{
  if (mcg->get_append_pos)
    return mcg->get_append_pos (mcg, col);

  return NULL;
}

GtkType 
mdi_color_generic_get_control_type (MDIColorGeneric *mcg)
{
  if (mcg->get_control_type)
    return mcg->get_control_type (mcg);

  return 0;
}

void
mdi_color_generic_sync_control (MDIColorGeneric *mcg)
{
  GList *list;
  ViewColorGeneric *view;

  list = GNOME_MDI_CHILD (mcg)->views;
  while (list) {
    view = gtk_object_get_data (GTK_OBJECT (list->data), "view_object");
    if (view->control)					  
      control_generic_sync (view->control);

    list = g_list_next (list);
  }
}

void
mdi_color_generic_append_view_type (MDIColorGeneric *mcg, GtkType type)
{
  mcg->views_type = g_list_append (mcg->views_type, GINT_TO_POINTER (type));
}

void
mdi_color_generic_set_modified (MDIColorGeneric *mcg, gboolean modified)
{  
	if ((mcg->monitor_modified)&&(mcg->modified != modified)) {    
		mcg->modified = modified;
		mdi_color_generic_set_name (mcg, mcg->name);      
	} else {
		mcg->modified = modified;
	}
}

void 
mdi_color_generic_set_name (MDIColorGeneric *mcg, char *name)
{
	char *old = mcg->name;

	if (name)
		mcg->name = g_strdup (name);
	else
		mcg->name = g_strdup ("");

	if ((mcg->monitor_modified) && (mcg->modified)) {
		char *tmp = g_strconcat (mcg->name, "*", NULL);
		gnome_mdi_child_set_name (GNOME_MDI_CHILD (mcg), tmp); 
		g_free (tmp);
	} else  {
		gnome_mdi_child_set_name (GNOME_MDI_CHILD (mcg), mcg->name);
	}

	g_free (old);
}

void 
mdi_color_generic_set_temp (MDIColorGeneric *mcg, gboolean val)
{
  mcg->temp = val;
}

/******************************* PROPERTIES ********************************/

typedef struct prop_t {
  GladeXML *gui;

  GtkWidget *entry_name;

  void (*changed_cb)(gpointer data);
  gpointer change_data;
} prop_t;

static void
entry_changed_cb (GtkWidget *widget, prop_t *prop)
{
  prop->changed_cb (prop->change_data);
}

static gpointer
mdi_color_generic_get_control (MDIColorGeneric *vcg, GtkVBox *box,
				void (*changed_cb)(gpointer data), 
			       gpointer change_data)
{
  GtkWidget *frame;
  prop_t *prop = g_new0 (prop_t, 1);

  prop->changed_cb   = changed_cb;
  prop->change_data = change_data;

  prop->gui = glade_xml_new (GCOLORSEL_GLADEDIR "mdi-color-generic-properties.glade", "frame");
  if (!prop->gui) {
    printf ("Could not find mdi-color-generic-properties.glade\n");
    return NULL;
  }

  frame = glade_xml_get_widget (prop->gui, "frame");
  if (!frame) {
    printf ("Corrupt file mdi-color-generic-properties.glade");
    return NULL;
  }

  gtk_box_pack_start_defaults (GTK_BOX (box), frame);

  prop->entry_name = glade_xml_get_widget (prop->gui, "entry-name");  
  gtk_signal_connect (GTK_OBJECT (prop->entry_name), "changed", 
		      GTK_SIGNAL_FUNC (entry_changed_cb), prop);

  return prop;
}

static void
mdi_color_generic_sync (MDIColorGeneric *mcg, gpointer data)
{
  prop_t *prop = data;

  gtk_signal_handler_block_by_data (GTK_OBJECT (prop->entry_name), prop);
  gtk_entry_set_text (GTK_ENTRY (prop->entry_name), mcg->name);
  gtk_signal_handler_unblock_by_data (GTK_OBJECT (prop->entry_name), prop);
}

static void
mdi_color_generic_apply (MDIColorGeneric *mcg, gpointer data)
{
  prop_t *prop = data;  

  mdi_color_generic_set_name (mcg, 
			    gtk_entry_get_text (GTK_ENTRY (prop->entry_name)));
}

static void
mdi_color_generic_close (MDIColorGeneric *mcg, gpointer data)
{
  prop_t *prop = data;

  gtk_object_unref (GTK_OBJECT (prop->gui));
  g_free (prop);
}

/**************************** Config ************************************/

static void
mdi_color_generic_save (MDIColorGeneric *mcg)
{
  gnome_config_set_string ("Name", mcg->name);
  gnome_config_set_bool ("Temp", mcg->temp);
}

static void
mdi_color_generic_load (MDIColorGeneric *mcg)
{
  char *str = gnome_config_get_string ("Name");

  mdi_color_generic_set_name (mcg, str);
  mcg->temp = gnome_config_get_bool ("Temp");

  g_free (str);
}
