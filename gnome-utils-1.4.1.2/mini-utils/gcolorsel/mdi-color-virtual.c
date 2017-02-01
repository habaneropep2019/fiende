#include "mdi-color-virtual.h"
#include "mdi-color-generic.h"
#include "menus.h"
#include "utils.h"

#include <gnome.h>

static void         mdi_color_virtual_class_init (MDIColorVirtualClass *class);
static void         mdi_color_virtual_init       (MDIColorVirtual *mcv);
static void         mdi_color_virtual_changed    (MDIColorGeneric *mcg,
						  gpointer data);

MDIColor           *mdi_color_virtual_get_owner  (MDIColor *col);

static MDIColor    *mdi_color_virtual_append_col (MDIColorVirtual *mcv,
						  MDIColor *col);
static MDIColor    *get_col     (MDIColorVirtual *mcv, MDIColor *col_parent);
static col_changed_t mdi_color_virtual_col_changed (MDIColorVirtual *mcv,
						    MDIColor *col);

static MDIColorGenericClass *parent_class = NULL;

guint 
mdi_color_virtual_get_type()
{
  static guint mdi_gen_child_type = 0;
  
  if (!mdi_gen_child_type) {
    GtkTypeInfo mdi_gen_child_info = {
      "MDIColorVirtual",
      sizeof (MDIColorVirtual),
      sizeof (MDIColorVirtualClass),
      (GtkClassInitFunc) mdi_color_virtual_class_init,
      (GtkObjectInitFunc) mdi_color_virtual_init,
      (GtkArgSetFunc) NULL,
      (GtkArgGetFunc) NULL,
    };
    
    mdi_gen_child_type = gtk_type_unique (mdi_color_generic_get_type (),
					  &mdi_gen_child_info);
  }
  
  return mdi_gen_child_type;
}

static void 
mdi_color_virtual_class_init (MDIColorVirtualClass *class)
{
  MDIColorGenericClass *mcg_class;
  GnomeMDIChildClass   *mdi_child_class;
  GtkObjectClass       *object_class;
  
  object_class     = GTK_OBJECT_CLASS (class);
  mdi_child_class  = GNOME_MDI_CHILD_CLASS (class);
  mcg_class        = (MDIColorGenericClass *)class;
  parent_class     = gtk_type_class (mdi_color_generic_get_type());

  mcg_class->document_changed = mdi_color_virtual_changed;

  class->append_col  = mdi_color_virtual_append_col;
  class->col_changed = mdi_color_virtual_col_changed; 
}

static void
mdi_color_virtual_init (MDIColorVirtual *mcv)
{
  MDIColorGeneric *mcg = MDI_COLOR_GENERIC (mcv);

  mcg->get_owner        = mdi_color_virtual_get_owner;

  mcg->flags = CHANGE_REMOVE | CHANGE_NAME | CHANGE_RGB | CHANGE_CLEAR;
}

MDIColorVirtual *
mdi_color_virtual_new ()
{
  MDIColorVirtual *mcv; 

  mcv = gtk_type_new (mdi_color_virtual_get_type ()); 

  return mcv;
}

MDIColor *
mdi_color_virtual_get_owner (MDIColor *col)
{
  MDIColor *parent_col;

  parent_col = gtk_object_get_data (GTK_OBJECT (col), "parent");

  return mdi_color_generic_get_owner (parent_col);
}

static MDIColor *         
mdi_color_virtual_append_col (MDIColorVirtual *mcv, MDIColor *col)
{
  return mdi_color_generic_append_new_set_data (MDI_COLOR_GENERIC (mcv), 
						col->r, col->g, col->b, 
						col->name, "parent", col);
}

static col_changed_t
mdi_color_virtual_col_changed (MDIColorVirtual *mcv, MDIColor *col)
{
  return COL_CHANGED_NOTHING;
}

static MDIColor *
get_col (MDIColorVirtual *mcv, MDIColor *col_parent)
{
  GList *list = MDI_COLOR_GENERIC (mcv)->col;

  while (list) {
    if (gtk_object_get_data (GTK_OBJECT (list->data), "parent") == col_parent)
      return list->data;

    list = g_list_next (list);
  }

  return NULL;
}

static MDIColor *
append_col (MDIColorVirtual *mcv, MDIColor *col, gboolean ref)
{
  MDIColor *new;
  
  new = MDI_COLOR_VIRTUAL_GET_CLASS (mcv)->append_col (mcv, col);

  if (ref) {
    gtk_object_ref (GTK_OBJECT (col));
    if (!new)
      mcv->trash = g_list_prepend (mcv->trash, col);
  }
  
  return new;
}

static void
clear_col_mcg (MDIColorVirtual *mcv, MDIColorGeneric *from, gboolean unref)
{
  MDIColorGeneric *mcg = MDI_COLOR_GENERIC (mcv);
  GList *list, *copy;
  MDIColor *col, *col_parent;

  copy = list = g_list_copy (MDI_COLOR_GENERIC (mcv)->col);

  if (g_list_length (mcg->parents) == 1) {
    if (unref) 
      while (list) {
	col_parent = gtk_object_get_data (GTK_OBJECT (list->data), "parent");
	gtk_object_unref (GTK_OBJECT (col_parent));
	list = g_list_next (list);
      }
    mdi_color_generic_clear (MDI_COLOR_GENERIC (mcv));  
  } else {

    while (list) {
      col = list->data;
      col_parent = gtk_object_get_data (GTK_OBJECT (col), "parent");
      
      if ((! from) || (col_parent->owner == from)) {
	mdi_color_generic_remove (MDI_COLOR_GENERIC (mcv), col);
	
	if (unref)
	  gtk_object_unref (GTK_OBJECT (col_parent));
      }
      
      list = g_list_next (list);
    }
  }
  g_list_free (copy);
}

static void
clear_col_trash (MDIColorVirtual *mcv, MDIColorGeneric *from, gboolean unref)
{
  GList *list = mcv->trash, *next;
  MDIColor *col;

  if ((g_list_length (MDI_COLOR_GENERIC (mcv)->parents) == 1)&&(!unref)) {
    g_list_free (mcv->trash);
    mcv->trash = NULL;    
  } 
  
  else
    
    while (list) {
      next = g_list_next (list);
      
      col = list->data;
      
      if ((!from) || (col->owner == from)) {
	if (list->prev) 
	  g_list_remove (list, col);
	else 
	  mcv->trash = g_list_remove (list, col);      
	
	if (unref)
	  gtk_object_unref (GTK_OBJECT (col));
      }
      
      list = next;
    }
}

static void       
mdi_color_virtual_changed (MDIColorGeneric *mcg, gpointer data)
{
  MDIColorVirtual *mcv;
  MDIColor *col_parent;
  MDIColor *col;
  GList *list = data;
  GList *new_pos;

  mcv = MDI_COLOR_VIRTUAL (mcg);

  mdi_color_generic_freeze (mcg);

  while (list) {
    col_parent = list->data;

    if (col_parent->change & CHANGE_CLEAR) {
      MDIColorGeneric *tmp = col_parent->owner;

      clear_col_mcg (mcv, tmp, TRUE);
      clear_col_trash (mcv, tmp, TRUE);
    }

    else

      if (col_parent->change & CHANGE_POS) { }

    else
      
      if (col_parent->change & CHANGE_APPEND)

	append_col (mcv, col_parent, TRUE);

      else {

	col = get_col (mcv, col_parent);
	
	if (col) { 

	  if (col_parent->change & CHANGE_REMOVE) { 
	    mdi_color_generic_remove (mcg, col);
	    gtk_object_unref (GTK_OBJECT (col_parent));
	  }

	  else

	    if ((col_parent->change & CHANGE_NAME) ||
		(col_parent->change & CHANGE_RGB)) {
	      	      
	      if (MDI_COLOR_VIRTUAL_GET_CLASS (mcv)->col_changed (mcv, col_parent) == COL_CHANGED_TRASH) {
		mdi_color_generic_remove (mcg, col);
		mcv->trash = g_list_prepend (mcv->trash, col_parent);
	      } else {
		/* No, sync views */
		if (col_parent->change & CHANGE_NAME)
		  mdi_color_generic_change_name (mcg, col, col_parent->name);
		
		if (col_parent->change & CHANGE_RGB) 
		  mdi_color_generic_change_rgb (mcg, col,
						col_parent->r,
						col_parent->g,
						col_parent->b);
		
		new_pos = mdi_color_generic_get_append_pos (mcg, col);
		if (new_pos)
		  mdi_color_generic_change_pos (mcg, col, 
					((MDIColor *)new_pos->data)->pos);
		else
		  mdi_color_generic_change_pos (mcg, col, -1);
					      
	      }	      
	    }

	} else { 
	  if (col_parent->change & CHANGE_REMOVE) {
	    mcv->trash = g_list_remove (mcv->trash, col_parent);
	    gtk_object_unref (GTK_OBJECT (col_parent));
	  }
	  
	  else
	    
	    if ((col_parent->change & CHANGE_NAME) ||
		(col_parent->change & CHANGE_RGB)) {
	      /* May be we have to append it now ? */
	      
	      if (MDI_COLOR_VIRTUAL_GET_CLASS (mcv)->col_changed (mcv, col_parent) == COL_CHANGED_DOC) {
		/* Yes, remove it from trash, append it to doc */
		mcv->trash = g_list_remove (mcv->trash, col_parent);
		mdi_color_virtual_append_col (mcv, col_parent);
	      }
	    }						 
	}
      }
    
    list = g_list_next (list);
  }

  mdi_color_generic_thaw (mcg);
}

void mdi_color_virtual_trash_to_doc (MDIColorVirtual *mcv)
{
  GList *list = mcv->trash, *next;

  while (list) {
    next = g_list_next (list);

    if (append_col (mcv, list->data, FALSE)) {
      if (list->prev) 
	g_list_remove (list, list->data);
      else 
	mcv->trash = g_list_remove (list, list->data);
    }

    list = next;
  }
}

void mdi_color_virtual_doc_to_trash (MDIColorVirtual *mcv)
{
  GList *list = MDI_COLOR_GENERIC (mcv)->col, *next;
  MDIColor *col;

  while (list) {
    next = g_list_next (list);

    col = list->data;

    if (MDI_COLOR_VIRTUAL_GET_CLASS (mcv)->col_changed (mcv, col) == COL_CHANGED_TRASH) {
      /* Yes, remove it from trash, append it to doc */
      mcv->trash = g_list_prepend (mcv->trash,
			 gtk_object_get_data (GTK_OBJECT (col), "parent"));
      mdi_color_generic_remove (MDI_COLOR_GENERIC (mcv), col);
    }

    list = next;
  }
}

void mdi_color_virtual_repost_all (MDIColorVirtual *mcv)
{
  GList *list, *list2;

  clear_col_mcg (mcv, NULL, FALSE);
  clear_col_trash (mcv, NULL, FALSE);

  list = MDI_COLOR_GENERIC (mcv)->parents;
  while (list) {
    list2 = MDI_COLOR_GENERIC (list->data)->col;
    while (list2) {
      if (! append_col (mcv, list2->data, FALSE)) 
	mcv->trash = g_list_prepend (mcv->trash, list2->data);

      list2 = g_list_next (list2);
    }

    list = g_list_next (list);
  }
}
