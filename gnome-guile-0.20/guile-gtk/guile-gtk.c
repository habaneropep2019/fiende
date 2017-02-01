/*
 * Time-stamp: <1998-03-11 15:38:53 szi>
 *
 * Copyright (C) 1997, 1998, 1999, 2000 Marius Vollmer
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <config.h>
#include <assert.h>
#ifdef GTK_2_0
#include <gtk-2.0/gtk/gtk.h>
#include <gtk-2.0/gdk/gdkprivate.h>
#include <gtk-2.0/gdk/gdkx.h>
#else
#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>
#endif
#include <libguile.h>
#include <guile/gh.h>
#include <libguile/dynl.h>
#include <guile-gtk.h>
#include <string.h>

/* Define this to enable some output during GC and other interesting
   actions. */
#undef DEBUG_PRINT



/* Guile compatability stuff */

#ifndef HAVE_SCM_DONE_MALLOC
void scm_done_malloc (long size);
#endif
#ifndef HAVE_SCM_INTERNAL_CWDR
SCM scm_internal_cwdr (scm_catch_body_t body,
		       void *body_data,
		       scm_catch_handler_t handler,
		       void *handler_data,
		       SCM_STACKITEM *stack_start);
#endif
#ifndef HAVE_SCM_PUTS
void scm_puts (char *str, SCM port);
#endif

#ifndef SCM_LIST1
#define SCM_LIST1(e0) scm_cons ((e0), SCM_EOL)
#endif

#ifndef SCM_LIST2
#define SCM_LIST2(e0, e1) scm_cons2 ((e0), (e1), SCM_EOL)
#endif

#ifndef HAVE_SCM_REVERSE_X
#define scm_reverse_x scm_list_reverse_x
#endif



/* Associating SCM values with Gtk pointers.

   We keep a hash table that can store a SCM value for an arbitray
   gpointer.  This is used for the proxies of GtkObjects and the boxed
   types.  */

static GHashTable *proxy_tab;

static guint
gpointer_hash (gpointer a)
{
  return (guint)a;
}

static gint
gpointer_compare (gpointer a, gpointer b)
{
  return a == b;
}

static void
enter_proxy (gpointer obj, SCM proxy)
{
  if (proxy_tab == NULL)
    proxy_tab = g_hash_table_new ((GHashFunc)gpointer_hash,
				  (GCompareFunc)gpointer_compare);
  g_hash_table_insert (proxy_tab, obj, (gpointer)proxy);
}

static SCM
get_proxy (gpointer obj)
{
  if (proxy_tab)
    {
      gpointer val = g_hash_table_lookup (proxy_tab, obj);
      return val? (SCM) val : SCM_BOOL_F;
    }
  return SCM_BOOL_F;
}

static void
forget_proxy (gpointer obj)
{
  g_hash_table_remove (proxy_tab, obj);
}



/* Storing additional info about a GtkType.

   Each GtkType has a unique sequence number.  We use that to simply
   index an array of sgtk_type_info pointers.  The array is grown
   dynamically when necessary. */

#define TYPE_INFO_INCR_MASK 0xFF

static sgtk_type_info **type_info_tab;
static guint n_type_info_tab = 0;

static void
enter_type_info (sgtk_type_info *info)
{

#ifdef GTK_2_0
  guint seqno = G_TYPE_BRANCH_SEQNO (info->type);
#else  
  guint seqno = GTK_TYPE_SEQNO (info->type);
#endif
  if (seqno >= n_type_info_tab)
    {
      guint i, new_size = (seqno+TYPE_INFO_INCR_MASK)&(~TYPE_INFO_INCR_MASK);
      type_info_tab = (sgtk_type_info **)
	scm_must_realloc ((char *)type_info_tab,
			  sizeof(sgtk_type_info*) * n_type_info_tab,
			  sizeof(sgtk_type_info*) * new_size,
			  "type info table");
      for (i = n_type_info_tab; i < new_size; i++)
	type_info_tab[i] = NULL;
      n_type_info_tab = new_size;
    }

  type_info_tab[seqno] = info;
}

sgtk_type_info*
sgtk_get_type_info (guint seqno)
{
  if (seqno >= n_type_info_tab)
    return NULL;
  return type_info_tab[seqno];
}

static sgtk_type_info*
must_get_type_info (guint seqno)
{
  sgtk_type_info *info = sgtk_get_type_info (seqno);
  if (info == NULL)
    abort ();
  return info;
}

typedef struct _type_infos {
  struct _type_infos *next;
  sgtk_type_info **infos;
} type_infos;

static type_infos *all_type_infos;

/* Find types that are mentioned in our *.defs files but are not
   provided by the Gtk run-time system.  This is only used
   occasionally to update the table in sgtk_try_missing_type.  */
#ifdef NEED_UNUSED_CODE
static void
sgtk_find_missing_types (type_infos *infos)
{
  sgtk_type_info **ip;
  for (ip = infos->infos; *ip; ip++)
    {
      if (gtk_type_from_name ((*ip)->name) == GTK_TYPE_INVALID
	  && (*ip)->type != GTK_TYPE_OBJECT)
	printf ("missing: %s, %s\n",
		(*ip)->name, gtk_type_name ((*ip)->type));
    }
}
#endif

void
sgtk_register_type_infos (sgtk_type_info **infos)
{
  type_infos *t;

  sgtk_init ();

  t = (type_infos *) scm_must_malloc (sizeof(type_infos), "gtk type infos");
  t->infos = infos;
  t->next = all_type_infos;
  all_type_infos = t;

#if 0
  sgtk_find_missing_types (t);
#endif
}

void
sgtk_register_type_infos_gtk (GtkTypeInfo **infos)
{
  GtkTypeInfo **t;

  for (t = infos; t && *t; t++)
    gtk_type_unique (GTK_TYPE_BOXED, *t);
}

/* When INFO refers to one of the known `missing' types, we initialize
   that type ourselves.  This is used to fix certain discrepancies
   between old Gtk versions and our *.defs files.  It is not OK to do
   this in general because we should not assume that we can safely
   initialize types from other modules.  */

static GtkType
sgtk_try_missing_type (char *name)
{
  static sgtk_type_info missing[] = {
    { "GdkGC", GTK_TYPE_BOXED },
    { "GtkToolbarStyle", GTK_TYPE_ENUM },
    { "GtkToolbarChildType", GTK_TYPE_ENUM },
    { "GtkTreeViewMode", GTK_TYPE_ENUM },
    { "GtkSpinButtonUpdatePolicy", GTK_TYPE_ENUM },
    { "GtkCellType", GTK_TYPE_ENUM },
    { "GdkOverlapType", GTK_TYPE_ENUM },
    { "GdkWMDecoration", GTK_TYPE_FLAGS },
    { "GdkWMFunction", GTK_TYPE_FLAGS },
    { "GdkVisibilityState", GTK_TYPE_ENUM },
    { "GdkInputSource", GTK_TYPE_ENUM },
    {NULL, GTK_TYPE_NONE}
  };

  sgtk_type_info *m;
  for (m = missing; m->name; m++)
    if (!strcmp (m->name, name))
      {
	GtkTypeInfo info = { NULL };
	info.type_name = name;
	return gtk_type_unique (m->type, &info);
      }

  return GTK_TYPE_INVALID;
}

static int
sgtk_fillin_type_info (sgtk_type_info *info)
{
  if (info->type != GTK_TYPE_OBJECT
      && info->type == GTK_FUNDAMENTAL_TYPE (info->type)
      && info->type != GTK_TYPE_INVALID)
    {
      GtkType parent_type = info->type;
      GtkType this_type = gtk_type_from_name (info->name);
      if (this_type == GTK_TYPE_INVALID)
	this_type = sgtk_try_missing_type (info->name);
      if (this_type == GTK_TYPE_INVALID)
	{
	  fprintf (stderr, "unknown type `%s'.\n", info->name);
	  return 0;
	}
      info->type = this_type;
      if (GTK_FUNDAMENTAL_TYPE (info->type) != parent_type)
	{
	  fprintf (stderr, "mismatch for type `%s'.\n", info->name);
	  info->type = GTK_TYPE_INVALID;
	  return 0;
	}
      enter_type_info (info);
    }

  return 1;
}      
     
sgtk_type_info*
sgtk_maybe_find_type_info (GtkType type)
{
  sgtk_type_info *info;
  type_infos *infos;
  char *name;

#ifdef GTK_2_0
  info = sgtk_get_type_info (G_TYPE_BRANCH_SEQNO(type));
#else
    info = sgtk_get_type_info (GTK_TYPE_SEQNO(type));
#endif
    
  if (info)
    return info;

  /* XXX - merge this with the GtkObject code.  I don't have the brain
     right now to do it. */

  name = gtk_type_name (type);
  for (infos = all_type_infos; infos; infos = infos->next)
    {
      sgtk_type_info **ip;
      for (ip = infos->infos; *ip; ip++)
	if (!strcmp ((*ip)->name, name))
	  {
	    if (GTK_FUNDAMENTAL_TYPE (type) != (*ip)->type)
	      {
		fprintf (stderr, "mismatch for type `%s'.\n", name);
		info->type = GTK_TYPE_INVALID;
		abort ();
	      }
	    (*ip)->type = type;
	    enter_type_info (*ip);
	    return *ip;
	  }
    }

  /* XXX - should use the Gtk+ type introspection here instead of
     giving up. */

  return NULL;
}

sgtk_type_info *
sgtk_find_type_info (GtkType type)
{
  sgtk_type_info *info = sgtk_maybe_find_type_info (type);

  if (info)
    return info;

  fprintf (stderr, "unknown type `%s'.\n", gtk_type_name (type));
  abort ();
}

/* GtkObjects.

   GtkObjects are wrapped with a smob.  The smob of a GtkObject is
   called its proxy.  The proxy and its GtkObject are strongly
   connected; that is, the GtkObject will stay around as long as the
   proxy is referenced from Scheme, and the proxy will not be
   collected as long as the GtkObject is used from outside of Scheme.

   The lifetime of GtkObjects is controlled by a reference count,
   while Scheme objects are managed by a tracing garbage collector
   (mark/sweep).  These two techniques are made to cooperate like
   this: the pointer from the proxy to the GtkObject is reflected in
   the reference count of the GtkObject.  All proxies are kept in a
   list and those that point to GtkObjects with a reference count
   greater than the number of `internal' references are marked during
   the marking phase of the tracing collector.  An internal reference
   is one that goes from a GtkObject with a proxy to another GtkObject
   with a proxy.  We can only find a subset of the true internal
   references (because Gtk does not yet cooperate), but this should be
   good enough.

   By using this combination of tracing and reference counting it is
   possible to break the cycle that is formed by the proxy pointing to
   the GtkObject and the GtkObject pointing back.  It is
   straightforward to extend this to other kind of cycles that might
   occur.  For example, when connecting a Scheme procedure as a signal
   handler, the procedure is very likely to have the GtkObject that it
   is connected to in its environment.  This cycle can be broken by
   including the procedure in the set of Scheme objects that get
   marked when we are tracing GtkObjects with a reference count
   greater than 1.

   Therefore, each proxy contains a list of `protects' that are marked
   when the proxy itself is marked.  In addition to this, there is
   also a global list of `protects' that is used for Scheme objects
   that are somewhere in Gtk land but not clearly associated with a
   particular GtkObject (like timeout callbacks).

  */

struct sgtk_protshell {
  SCM object;
  struct sgtk_protshell *next;
  struct sgtk_protshell **prevp;
};

static GMemChunk *sgtk_protshell_chunk;

/* Analogous to the PROTECTS list of a proxy but for SCM values that
   are not associated with a particular GtkObject. */

static struct sgtk_protshell *global_protects;

void
sgtk_unprotect (sgtk_protshell *prot)
{
  if (*prot->prevp = prot->next)
    prot->next->prevp = prot->prevp;
  g_chunk_free (prot, sgtk_protshell_chunk);
}

static void
sgtk_mark_protects (sgtk_protshell *prots)
{
  while (prots)
    {
      scm_gc_mark (prots->object);
      prots = prots->next;
    }
}

/* The CDR of a GtkObject smob points to one of these.  PROTECTS is a
   Scheme list of all SCM values that need to be protected from the GC
   because they are in use by OBJ.  PROTECTS includes the smob cell
   itself.  NEXT and PREVP are used to chain all proxies together for
   the marking mentioned above.  NEXT simply points to the next proxy
   struct and PREVP points to the pointer that points to us.  */

typedef struct _sgtk_object_proxy {
  /*
    FIXME: Maybe what we want is to have a GObject instead of GtkObject
  */

  GtkObject *obj;
  struct sgtk_protshell *protects;
  int traced_refs;
  struct _sgtk_object_proxy *next;
  struct _sgtk_object_proxy **prevp;
} sgtk_object_proxy;

/* The list of all existing proxies. */

static sgtk_object_proxy *all_proxies = NULL;

/* Insert the list of protshells starting at PROTS into the global
   protects list.  This is used when a proxy is freed so that we don't
   forget about its protects. */

static void
sgtk_move_prots_to_global (sgtk_protshell *prots)
{
  if (prots)
    {
      sgtk_protshell *g = global_protects;
      global_protects = prots;
      global_protects->prevp = &global_protects;
      if (g)
	{
	  sgtk_protshell *p;
	  for (p = prots; p->next; p = p->next)
	    ;
	  p->next = g;
	  g->prevp = &p->next;
	}
    }
}

#if 0
static int
sgtk_check_protshell (sgtk_protshell *prot)
{
  sgtk_object_proxy *proxy;
  sgtk_protshell *walk;

  for (proxy = all_proxies; proxy; proxy = proxy->next)
    for (walk = proxy->protects; walk; walk = walk->next)
      if (walk == prot)
	return 1;
  for (walk = global_protects; walk; walk = walk->next)
    if (walk == prot)
      return 1;

  fprintf (stderr, "unknown protshell %p\n", prot);
  return 0;
}
#endif

/* The smob for GtkObjects.  */

static long tc16_gtkobj;

#define GTKOBJP(x)       (SCM_NIMP(x) && SCM_CAR(x) == tc16_gtkobj)
#define GTKOBJ_PROXY(x)  ((sgtk_object_proxy *)SCM_CDR(x))

sgtk_protshell *
sgtk_protect (SCM protector, SCM obj)
{
  sgtk_protshell *prot = g_chunk_new (sgtk_protshell, sgtk_protshell_chunk);
  sgtk_protshell **prevp;

  prot->object = obj;

  if (GTKOBJP (protector))
    prevp = &(GTKOBJ_PROXY(protector)->protects);
  else
    prevp = &global_protects;
  
  if (prot->next = *prevp)
	prot->next->prevp = &prot->next;
  *prevp = prot;
  prot->prevp = prevp;

  return prot;
}

static void
mark_traced_ref (GtkWidget *obj, void *data)
{
  SCM p = (SCM)get_proxy (obj);
  if (p != SCM_BOOL_F)
    {
      sgtk_object_proxy *proxy = GTKOBJ_PROXY (p);
#ifdef DEBUG_PRINT
      fprintf (stderr, "marking trace %p %s\n",
	       proxy->obj, gtk_type_name (GTK_OBJECT_TYPE (proxy->obj)));
#endif
      sgtk_mark_protects (proxy->protects);
    }
}

static SCM
gtkobj_mark (SCM obj)
{
  sgtk_object_proxy *proxy = GTKOBJ_PROXY(obj);

#ifdef DEBUG_PRINT
  fprintf (stderr, "marking %p %s\n",
	   proxy->obj, gtk_type_name (GTK_OBJECT_TYPE (proxy->obj)));
#endif

  SCM_SETGC8MARK (obj);
  if (GTK_IS_CONTAINER (proxy->obj))
    gtk_container_foreach (GTK_CONTAINER(proxy->obj), mark_traced_ref, NULL);
  sgtk_mark_protects (proxy->protects);
  return SCM_EOL;
}

static int
gtkobj_print (SCM obj, SCM port, scm_print_state *pstate)
{
  sgtk_object_proxy *proxy = GTKOBJ_PROXY (obj);
  GtkType tid = GTK_OBJECT_TYPE (proxy->obj);

  scm_puts ("#<", port);
  scm_puts (gtk_type_name (tid), port);
  scm_puts (" ", port);
  scm_intprint ((long)proxy->obj, 16, port);
  scm_puts (">", port);
  return 1;
}

static scm_sizet
gtkobj_free (SCM obj)
{
  sgtk_object_proxy *proxy = GTKOBJ_PROXY (obj);

  // fprintf (stderr, "freeing proxy %p\n", proxy);

#ifdef DEBUG_PRINT
  fprintf (stderr, "freeing %p %s\n",
	   proxy->obj, gtk_type_name (GTK_OBJECT_TYPE (proxy->obj)));
#endif

  forget_proxy (proxy->obj);
  gtk_object_unref (proxy->obj);
  if (*proxy->prevp = proxy->next) proxy->next->prevp = proxy->prevp;
  
  sgtk_move_prots_to_global (proxy->protects);

  scm_must_free ((char *)proxy);
  return sizeof (sgtk_object_proxy);
}
/*
#ifdef OLD_GUILE

struct scm_smobfuns gtkobj_smob = {
  gtkobj_mark,
  gtkobj_free,
  gtkobj_print,
  NULL
};

#endif
*/

/* Treating GtkObject proxies right during GC.  We need to run custom
   code during the mark phase of the Scheme GC.  We do this by
   creating a new smob type and allocating one actual smob of it.
   This smob is made permanent and thus its marking function is
   invoked for every GC.  We hijack this function to do the tracing of
   all existing proxies as well. */

static long tc16_gtkobj_marker_hook;

static void
count_traced_ref (GtkWidget *obj, void *data)
{
  SCM p = (SCM)get_proxy (obj);
  if (p != SCM_BOOL_F)
    {
      sgtk_object_proxy *proxy = GTKOBJ_PROXY (p);
#ifdef DEBUG_PRINT
      fprintf (stderr, "counting %p %s\n",
	       proxy->obj, gtk_type_name (GTK_OBJECT_TYPE (proxy->obj)));
#endif
      proxy->traced_refs++;
    }
}

static SCM
gtkobj_marker_hook (SCM obj)
{
  sgtk_object_proxy *proxy;

  SCM_SETGC8MARK (obj);

  /* We do two passes here.  The first pass counts how many references
     an object has from other objects that have a proxy.  The second
     pass marks all objects that have more than this number of
     references.  For the first pass to work, we need to enumerate all
     references that an object has to other objects.  We can't do that
     precisely without help from Gtk+ itself.  But luckily, *not*
     knowing about an `internal' reference is the conservative thing.
     Missing a reference will make it appear to us that an object has
     more `external' references to it than it really has, thus making
     us keep the proxy alive.  Only when these `external' references
     form a cycle over some Scheme values, we loose.  As a first
     approximation to the true set of references of a GtkObject, we
     just traverse its children with gtk_container_foreach.  */

  /* First pass. */
  for (proxy = all_proxies; proxy; proxy = proxy->next)
    {
      GtkObject *obj = proxy->obj;
#ifdef DEBUG_PRINT
      fprintf (stderr, "on %p %p\n", proxy, obj);
#endif
      if (GTK_IS_CONTAINER (obj))
	gtk_container_foreach (GTK_CONTAINER(obj), count_traced_ref, NULL);
    }
#ifdef DEBUG_PRINT
  fprintf (stderr, "done with pass 1.\n");
#endif

  /* Second pass. */
  for (proxy = all_proxies; proxy; proxy = proxy->next)
    {
#ifdef GTK_2_0
      /* 
	 FIXME: proxy struct may need to be changed!
      */
      if (proxy->obj->parent_instance.ref_count > proxy->traced_refs + 1)
	
#else
      if (proxy->obj->ref_count > proxy->traced_refs + 1)
#endif
	
	{
#ifdef DEBUG_PRINT
	  fprintf (stderr, "hooking %p %s\n",
		   proxy->obj, gtk_type_name (GTK_OBJECT_TYPE (proxy->obj)));
#endif
	  sgtk_mark_protects (proxy->protects);
	}
      proxy->traced_refs = 0;
    }
  sgtk_mark_protects (global_protects);
  return SCM_EOL;
}

static int
gtkobj_marker_hook_print (SCM obj, SCM port, scm_print_state *pstate)
{
  scm_puts ("#<the invisible GtkObject marker hook>", port);
  return 1;
}
/*
#ifdef OLD_GUILE
struct scm_smobfuns gtkobj_marker_hook_smob = {
  gtkobj_marker_hook,
  NULL,
  gtkobj_marker_hook_print,
  NULL
};
#endif
*/
static void
install_marker_hook ()
{
  SCM z;

  SCM_DEFER_INTS;
  SCM_NEWCELL (z);
  SCM_SETCAR (z, tc16_gtkobj_marker_hook);
  SCM_SETCDR (z, 0);
  SCM_ALLOW_INTS;
  
  scm_permanent_object (z);
}

/* Create a proxy for OBJ. */

static SCM
make_gtkobj (GtkObject *obj)
{
  sgtk_object_proxy *proxy;
  SCM z;

  proxy = (sgtk_object_proxy *)scm_must_malloc (sizeof(sgtk_object_proxy),
						"GtkObject proxy");
  gtk_object_ref (obj);
  gtk_object_sink (obj);
#ifdef DEBUG_PRINT
  fprintf (stderr, "New proxy %p for %p %s\n", proxy, obj,
	   gtk_type_name (GTK_OBJECT_TYPE (obj)));
#endif
  proxy->obj = obj;
  proxy->protects = NULL;
  proxy->traced_refs = 0;
  proxy->next = all_proxies;
  all_proxies = proxy;
  proxy->prevp = &all_proxies;
  if (proxy->next)
    proxy->next->prevp = &proxy->next;

  SCM_DEFER_INTS;
  SCM_NEWCELL (z);
  SCM_SETCAR (z, tc16_gtkobj);
  SCM_SETCDR (z, (SCM) proxy);
  enter_proxy (obj, z);
  SCM_ALLOW_INTS;

  sgtk_protect (z, z); /* this one is never removed. */

  return z;
}

/* Return the proxy for OBJ if it already has one, else create a new
   one.  When OBJ is NULL, return `#f'. */

SCM
sgtk_wrap_gtkobj (GtkObject *obj)
{
  SCM handle;

  if (obj == NULL)
    return SCM_BOOL_F;

  handle = get_proxy (obj);
  if (handle == SCM_BOOL_F)
    handle = make_gtkobj (obj);
  return handle;
}

int
sgtk_is_a_gtkobj (guint type, SCM obj)
{

  if (!(SCM_NIMP (obj) && GTKOBJP (obj)))
    return 0;
  return gtk_type_is_a (GTK_OBJECT_TYPE(GTKOBJ_PROXY(obj)->obj), type);
}

GtkObject*
sgtk_get_gtkobj (SCM obj)
{
  if (obj == SCM_BOOL_F)
    return NULL;
  else
    return GTKOBJ_PROXY(obj)->obj;
}

/* Enums.

   Enumerations are described by a `sgtk_enum_info' structure.  That
   structure contains a list of all literals and their respective
   values.  In Scheme, an enum element is represented by a symbol
   whose name is the literal. */

SCM sgtk_flags_symbol_protector = SCM_BOOL_F;

static int
sgtk_flags_comp (const void *first, const void *second)
{
  return ((sgtk_enum_literal *) first)->symbol - ((sgtk_enum_literal *) second)->symbol;
}

void 
sgtk_enum_flags_init (sgtk_enum_info *info)
{
  int	i;
  SCM	s;

  if (sgtk_flags_symbol_protector == SCM_BOOL_F)
    {
      SCM_NEWCELL (sgtk_flags_symbol_protector);
      SCM_SETCAR (sgtk_flags_symbol_protector, SCM_BOOL_F);
      SCM_SETCDR (sgtk_flags_symbol_protector, SCM_EOL);

      scm_protect_object (sgtk_flags_symbol_protector);
    }

  for (i = 0; i < info->n_literals; i++)
    {
      info->literals[i].symbol = SCM_CAR (scm_intern0 (info->literals[i].name));

      SCM_NEWCELL (s);
      SCM_SETCAR (s, info->literals[i].symbol);
      SCM_SETCDR (s, SCM_CDR (sgtk_flags_symbol_protector));
      SCM_SETCDR (sgtk_flags_symbol_protector, s);
    }

  qsort (info->literals, info->n_literals, sizeof (sgtk_enum_literal), sgtk_flags_comp);
}

int
sgtk_enum_flags_bin_search (SCM key, sgtk_enum_info *info, int *rval)
{
  int			upper, lower, half;
  sgtk_enum_literal	*ls;
  
  ls = info->literals;

  upper = info->n_literals - 1;
  lower = 0;

  while (upper >= lower)
    {
      half = (upper + lower) >> 1;
      if (key > ls[half].symbol)
	lower = half + 1;
      else
	if (key == ls[half].symbol)
	  { 
	    *rval = ls[half].value; 
	    return TRUE; 
	  }
	else
	  upper = half - 1;
    } 

  *rval = -1;
  return FALSE;
}

SCM
sgtk_enum2scm (gint val, sgtk_enum_info *info)
{
  int i;
  for (i = 0; i < info->n_literals; i++)
    if (info->literals[i].value == val)
      return info->literals[i].symbol;
  SCM_ASSERT (0, SCM_MAKINUM (val), SCM_ARG1, "enum->symbol");
  return SCM_BOOL_F;
}

gint
sgtk_scm2enum (SCM obj, sgtk_enum_info *info, int pos, char *sname)
{
  int rval;

  if (!SCM_NIMP (obj))
    return 0;

  if (SCM_SYMBOLP (obj) &&
      (sgtk_enum_flags_bin_search (obj, info, &rval) == TRUE))
    return rval;


  if (SCM_INUMP (obj))
    return SCM_INUM (obj);

  /* if obj is not integer (or it is not correct symbol)
   * scm_num2long throws an exception for us 
   */

  return scm_num2long (obj, (char *) pos, sname);
}

gint
sgtk_valid_enum (SCM obj, sgtk_enum_info *info)
{
  int tmp;

  if (!SCM_NIMP (obj))
    return 0;

  if (SCM_SYMBOLP (obj))
    return sgtk_enum_flags_bin_search (obj, info, &tmp);

  return (scm_integer_p (obj)) == SCM_BOOL_T;
}

/* Flags.

   Like enums, flags are described by a `sgtk_enum_info' structure.
   In Scheme, flags are represented by a list of symbols, one for each
   bit that is set in the flags value. */

SCM
sgtk_flags2scm (gint val, sgtk_enum_info *info)
{
  SCM ans = SCM_EOL;
  int i;
  for (i = 0; i < info->n_literals; i++)
    if (val & info->literals[i].value)
      {
	ans = scm_cons (info->literals[i].symbol, ans);
	val &= ~info->literals[i].value;
      }
  return ans;
}

gint
sgtk_scm2flags (SCM obj, sgtk_enum_info *info, int pos, char *sname)
{
  int ans = 0, m;

  if (SCM_INUMP (obj))
    return SCM_INUM (obj);

  if (scm_integer_p (obj) == SCM_BOOL_T)
    return scm_num2long (obj, (char *) pos, sname);

  while (SCM_NIMP (obj) && SCM_CONSP (obj))
    {
      SCM sym = SCM_CAR (obj);

      if (SCM_SYMBOLP (sym))
	{
	  if (sgtk_enum_flags_bin_search (sym, info, &m) == FALSE)
	    break;
	}
      else
	if (SCM_INUMP (sym))
	  m = SCM_INUM (sym);
        else
	  if (scm_integer_p (sym) == SCM_BOOL_T)
	    m = scm_num2long (sym, (char *) pos, sname);
	  else
	    break;
      
      ans |= m;
      obj = SCM_CDR (obj);
    }

  if (obj != SCM_EOL)
    SCM_ASSERT (0, obj, pos, sname);

  return ans;
}

gint
sgtk_valid_flags (SCM obj, sgtk_enum_info *info)
{
  int tmp;

  if (SCM_INUMP (obj))
    return TRUE;

  if (scm_integer_p (obj) == SCM_BOOL_T)
    return TRUE;

  while (!SCM_NULLP (obj))
    {
      SCM sym = SCM_CAR (obj);

      if (SCM_SYMBOLP (sym))
	{
	  if (sgtk_enum_flags_bin_search (sym, info, &tmp) == FALSE)
	    return FALSE;
	}
      else
	if (scm_integer_p (sym) == SCM_BOOL_F)
	  return FALSE;
      
      obj = SCM_CDR (obj);
    }
  
  return TRUE;
}

/* String enums.

   A string enum is like an enum, but the values are strings.  The
   range of values can be extended, so anywhere a "string enum" value
   is accepted, we also accept a string (but not a symbol).  */

int
sgtk_valid_senum (SCM obj, sgtk_senum_info *info)
{
  int i;

  if (! SCM_NIMP (obj))
    return 0;
  if (SCM_STRINGP (obj))
    return 1;
  if (! SCM_SYMBOLP (obj))
    return 0;

  for (i = 0; i < info->n_literals; i++)
    if (! strcmp (info->literals[i].name, SCM_CHARS (obj)))
      return 1;
  return 0;
}

SCM
sgtk_senum2scm (char *val, sgtk_senum_info *info)
{
  int i;
  for (i = 0; i < info->n_literals; i++)
    if (! strcmp (info->literals[i].value, val))
      return SCM_CAR (scm_intern0 (info->literals[i].name));
  return scm_makfrom0str (val);
}

char *
sgtk_scm2senum (SCM obj, sgtk_senum_info *info)
{
  int i;

  if (SCM_STRINGP (obj))
    {
      SCM_COERCE_SUBSTR (obj);
      return SCM_CHARS (obj);
    }

  for (i = 0; i < info->n_literals; i++)
    if (! strcmp (info->literals[i].name, SCM_CHARS (obj)))
      return info->literals[i].value;
  return NULL;
}

/* Boxed Values.

 */

static long tc16_boxed;

#define BOXED_P(x)     (SCM_NIMP(x) && (SCM_TYP16(x) == tc16_boxed))
#define BOXED_SEQNO(x) (((guint)SCM_CAR(x))>>16)
#define BOXED_PTR(x)   ((gpointer)SCM_CDR(x))
#define BOXED_INFO(x)  ((sgtk_boxed_info*)must_get_type_info(BOXED_SEQNO(x)))

static scm_sizet
boxed_free (SCM obj)
{
  sgtk_boxed_info *info = BOXED_INFO (obj);
  info->destroy (BOXED_PTR (obj));
  return info->size;
}

static int
boxed_print (SCM exp, SCM port, scm_print_state *pstate)
{
  sgtk_boxed_info *info = BOXED_INFO (exp);
  scm_puts ("#<", port);
  scm_puts (info->header.name, port);
  scm_puts (" ", port);
  scm_intprint ((long)BOXED_PTR (exp), 16, port);
  scm_puts (">", port);
  return 1;
}
/*
#ifdef OLD_GUILE
struct scm_smobfuns boxed_smob = {
  scm_mark0,
  boxed_free,
  boxed_print,
  NULL
};
#endif
*/
SCM
sgtk_boxed2scm (gpointer ptr, sgtk_boxed_info *info, int copyp)
{
  SCM z;

  if (ptr == NULL)
    return SCM_BOOL_F;

  if (!sgtk_fillin_type_info (&info->header))
    return SCM_BOOL_F;

  SCM_DEFER_INTS;
  if (copyp)
    {
      ptr = info->copy (ptr);
      scm_done_malloc (info->size);
    }
  SCM_NEWCELL (z);
#ifdef GTK_2_0
  if (G_TYPE_BRANCH_SEQNO(info->header.type) > 0xFFFF)
#else
  if (GTK_TYPE_SEQNO(info->header.type) > 0xFFFF)
#endif  
    abort ();
#ifdef GTK_2_0
  SCM_SETCAR (z, tc16_boxed | (G_TYPE_BRANCH_SEQNO(info->header.type))<<16);
#else
  SCM_SETCAR (z, tc16_boxed | (GTK_TYPE_SEQNO(info->header.type))<<16);
#endif
  SCM_SETCDR (z, (SCM) ptr);
  SCM_ALLOW_INTS;

  return z;
}

void *
sgtk_scm2boxed (SCM obj)
{
  if (obj == SCM_BOOL_F)
    return NULL;
  return BOXED_PTR (obj);
}

int
sgtk_valid_boxed (SCM obj, sgtk_boxed_info *info)
{
  return (SCM_NIMP (obj) && BOXED_P (obj) && BOXED_INFO (obj) == info);
}

/* Floats.

   Only here to set things straight. */

int
sgtk_valid_float (SCM obj)
{
  return SCM_NUMBERP (obj);
}

SCM gh_double2scm (double);

gfloat
sgtk_scm2float (SCM obj)
{
  return gh_scm2double (obj);
}

SCM
sgtk_float2scm (gfloat f)
{
  return gh_double2scm ((double)f);
}

int
sgtk_valid_double (SCM obj)
{
  return SCM_NUMBERP (obj);
}

SCM gh_double2scm (double);

double
sgtk_scm2double (SCM obj)
{
  return gh_scm2double (obj);
}

SCM
sgtk_double2scm (double f)
{
  return gh_double2scm (f);
}

int
sgtk_valid_point (SCM obj)
{
  return SCM_NIMP (obj) && SCM_CONSP (obj)
    && SCM_NUMBERP (SCM_CAR (obj))    /* too permissive */
    && SCM_NUMBERP (SCM_CDR (obj));   /* too permissive */
}

GdkPoint
sgtk_scm2point (SCM obj)
{
  GdkPoint res;
  res.x = gh_scm2int (SCM_CAR (obj));
  res.y = gh_scm2int (SCM_CDR (obj));
  return res;
}

SCM
sgtk_point2scm (GdkPoint p)
{
  return gh_cons (gh_int2scm (p.x),
		  gh_int2scm (p.y));
}

int
sgtk_valid_rect (SCM obj)
{
  return SCM_NIMP (obj) && SCM_CONSP (obj)
    && sgtk_valid_point (SCM_CAR (obj))
    && sgtk_valid_point (SCM_CDR (obj));
}

GdkRectangle
sgtk_scm2rect (SCM obj)
{
  GdkRectangle res;
  res.x = gh_scm2int (SCM_CAAR (obj));
  res.y = gh_scm2int (SCM_CDAR (obj));
  res.width = gh_scm2int (SCM_CADR (obj));
  res.height = gh_scm2int (SCM_CDDR (obj));
  return res;
}

SCM
sgtk_rect2scm (GdkRectangle r)
{
  return gh_cons (gh_cons (gh_int2scm (r.x),
			   gh_int2scm (r.y)),
		  gh_cons (gh_int2scm (r.width),
			   gh_int2scm (r.height)));
}

GdkAtom
sgtk_scm2atom (SCM symbol)
{
  return gdk_atom_intern (SCM_CHARS(symbol), FALSE);
}

SCM
sgtk_atom2scm (GdkAtom atom)
{
  char *name = gdk_atom_name (atom);
  SCM sym;

  if (name == NULL)
    return SCM_BOOL_F;
  sym = SCM_CAR (scm_intern (name, strlen (name)));
  g_free (name);
  return sym;
}

SCM_SYMBOL (sym_gnome_file, "gnome-file");

/* HAVE_RESTARTS is coming from scmconfig.h */

#ifdef HAVE_RESTARTS
#define SCM_SYSCALL(line) line
#endif  

#ifndef SCM_SYSCALL
# ifdef EINTR
#  if (EINTR > 0)
#   define SCM_SYSCALL(line) do{errno = 0;line;}while(EINTR==errno)
#  endif /*  (EINTR > 0) */
# endif /* def EINTR */
#endif /* ndef SCM_SYSCALL */
            
#ifndef SCM_SYSCALL
# define SCM_SYSCALL(line) line;
#endif /* ndef SCM_SYSCALL */

int
sgtk_port2fileno (SCM port)
{
#ifdef SCM_FSTREAM
  return SCM_FSTREAM(port)->fdes;
#else
  return fileno ((FILE*) SCM_STREAM (port));
#endif
}

SCM
sgtk_fileno2port (int fd)
{
  SCM res;

#ifdef HAVE_SCM_FDES_TO_PORT
  SCM_DEFER_INTS;
  res = scm_fdes_to_port (fd, "r+0", sym_gnome_file);
  if (SCM_NIMP (res) && SCM_OPFPORTP (res))
    scm_setvbuf (res, SCM_MAKINUM (_IONBF), SCM_MAKINUM (0));
#else
  FILE *f;
  static char *proc = "gnome filedescriptor conversion";
 
  if (fd == -1)
      scm_syserror (proc);
  if (!(f = fdopen (fd, "r+"))) {
      SCM_SYSCALL (close (fd));
      scm_syserror (proc);
  }
  SCM_DEFER_INTS;
  res = scm_stdio_to_port (f, "r+0", sym_gnome_file);
  scm_setbuf0 (res);
#endif
  SCM_ALLOW_INTS;
  return res;
}

static long tc16_gtktype;

#define GTKTYPEP(x)     (SCM_NIMP(x) && SCM_CAR(x) == tc16_gtktype)
#define GTKTYPE(x)      ((GtkType)SCM_CDR(x))

static int
gtktype_print (SCM obj, SCM port, scm_print_state *pstate)
{
  GtkType type = GTKTYPE (obj);
  scm_puts ("#<GtkType ", port);
  scm_puts (gtk_type_name (type), port);
  scm_puts (">", port);
  return 1;
}

static SCM
gtktype_equalp (SCM obj1, SCM obj2)
{
  return GTKTYPE (obj1) == GTKTYPE (obj2)? SCM_BOOL_T : SCM_BOOL_F;
}
/*
#ifdef OLD_GUILE
struct scm_smobfuns gtktype_smob = {
  scm_mark0,
  scm_free0,
  gtktype_print,
  gtktype_equalp
};
#endif
*/
GtkType
sgtk_type_from_name (char *name)
{
  GtkType type = gtk_type_from_name (name);
  if (type == GTK_TYPE_INVALID)
    {
      sgtk_object_info *info = sgtk_find_object_info (name);
      if (info)
	type = info->header.type;
    }
  return type;
}

int
sgtk_valid_type (SCM obj)
{
  return obj == SCM_BOOL_F || GTKTYPEP (obj)
    || (SCM_NIMP (obj) && SCM_SYMBOLP (obj)
	&& sgtk_type_from_name (SCM_CHARS (obj)));
}

GtkType
sgtk_scm2type (SCM obj)
{
  if (obj == SCM_BOOL_F)
    return GTK_TYPE_INVALID;
  else if (GTKTYPEP (obj))
    return GTKTYPE (obj);
  else
    return sgtk_type_from_name (SCM_CHARS (obj));
}

SCM
sgtk_type2scm (GtkType t)
{
  SCM z;

  if (t == GTK_TYPE_INVALID)
    return SCM_BOOL_F;

  SCM_DEFER_INTS;
  SCM_NEWCELL (z);
  SCM_SETCAR (z, tc16_gtktype);
  SCM_SETCDR (z, t);
  SCM_ALLOW_INTS;

  return z;
}

/* Illegal objects.  Guile-gtk constructs one of these when it sees a
   object with illegal type.  The use can't do anything with them, but
   the failure is clearly labelled and doesn't pop up until such an
   object is really used. */

static long tc16_illobj;

#define ILLOBJP(x)     (SCM_NIMP(x) && SCM_CAR(x) == tc16_illobj)
#define ILLOBJ_TYPE(x) ((GtkType)SCM_CDR(x))

static int
illobj_print (SCM obj, SCM port, scm_print_state *pstate)
{
  GtkType type = ILLOBJ_TYPE (obj);
  scm_puts ("#<object of illegal type ", port);
  scm_puts (gtk_type_name (type), port);
  scm_puts (">", port);
  return 1;
}
/*
#ifdef OLG_GUILE
struct scm_smobfuns illobj_smob = {
  scm_mark0,
  scm_free0,
  illobj_print,
  NULL
};
#endif 
*/
static SCM
sgtk_make_illegal_type_object (GtkType type)
{
  SCM z;

  SCM_DEFER_INTS;
  SCM_NEWCELL (z);
  SCM_SETCAR (z, tc16_illobj);
  SCM_SETCDR (z, type);
  SCM_ALLOW_INTS;

  return z;
}

/* Composites. */

int
sgtk_valid_composite (SCM obj, int (*predicate)(SCM))
{
  return sgtk_valid_complen (obj, predicate, -1);
}

int
sgtk_valid_complen (SCM obj, int (*predicate)(SCM), int len)
{
  int actual_len;

  if ((actual_len = scm_ilength (obj)) >= 0)
    {
      if (len >= 0 && len != actual_len)
	return 0;

      if (predicate)
	{
	  while (SCM_NIMP(obj) && SCM_CONSP(obj))
	    {
	      if (!predicate (SCM_CAR(obj)))
		return 0;
	      obj = SCM_CDR(obj);
	    }
	}
      return 1;
    }
  else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
    {
      int i;
      SCM *elts;

      actual_len = SCM_LENGTH (obj);
      if (len >= 0 && len != actual_len)
	return 0;

      if (predicate)
	{
	  elts = SCM_VELTS (obj);
	  for (i = 0; i < actual_len; i++)
	    if (!predicate(elts[i]))
	      return 0;
	}
      return 1;
    }
  else
    return 0;
}

SCM
sgtk_composite_inconversion (SCM obj, SCM (*conversion)(SCM))
{
  if (conversion == NULL)
    return obj;

  if (obj == SCM_EOL || (SCM_NIMP(obj) && SCM_CONSP(obj)))
    {
      int pos = 0;
      SCM list = obj;
      SCM newlist = list;
      while (SCM_NIMP(obj) && SCM_CONSP(obj))
	{
	  SCM newelt = conversion (SCM_CAR(obj));
	  if (newelt != SCM_CAR(obj))
	    {
	      if (newlist == list)
		{
		  newlist = scm_list_copy (list);
		  obj = newlist;
		  while (pos > 0)
		    obj = SCM_CDR(obj);
		}
	      SCM_SETCAR(obj, newelt);
	    }
	  obj = SCM_CDR(obj);
	  pos++;
	}
      return newlist;
    }
  else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
    {
      SCM vec = obj;
      SCM newvec = vec;
      int len = SCM_LENGTH(newvec), i;
      for (i = 0; i < len; i++)
	{
	  SCM newelt = conversion (SCM_VELTS(newvec)[i]);
	  if (newelt != SCM_VELTS(newvec)[i])
	    {
	      if (newvec == vec)
		{
		  int j;
		  newvec = scm_make_vector (SCM_MAKINUM(len), SCM_UNDEFINED);
		  for (j = 0; j < len; j++)
		    SCM_VELTS(newvec)[j] = SCM_VELTS(vec)[j];
		}
	      SCM_VELTS(newvec)[i] = newelt;
	    }
	}
      return newvec;
    }
  else
    return obj;
}

SCM
sgtk_composite_outconversion (SCM obj, SCM (*conversion)(SCM))
{
  if (conversion == NULL)
    return obj;

  if (obj == SCM_EOL || (SCM_NIMP(obj) && SCM_CONSP(obj)))
    {
      SCM list = obj;
      while (SCM_NIMP(obj) && SCM_CONSP(obj))
	{
	  SCM_SETCAR(obj, conversion (SCM_CAR(obj)));
	  obj = SCM_CDR(obj);
	}
      return list;
    }
  else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
    {
      int len = SCM_LENGTH(obj), i;
      for (i = 0; i < len; i++)
	SCM_VELTS(obj)[i] = conversion (SCM_VELTS(obj)[i]);
      return obj;
    }
  else
    return obj;
}
  
SCM
sgtk_slist2scm (GSList *list, SCM (*toscm)(void*))
{
  SCM res, *tail = &res;
  while (list)
    {
      *tail = scm_cons (toscm (&list->data), *tail);
      tail = SCM_CDRLOC (*tail);
      list = list->next;
    }
  *tail = SCM_EOL;
  return res;
}

GSList*
sgtk_scm2slist (SCM obj, void (*fromscm)(SCM, void*))
{
  GSList *res, **tail = &res;

  if (obj == SCM_BOOL_F)
    return NULL;
  else if (obj == SCM_EOL || (SCM_NIMP(obj) && SCM_CONSP(obj)))
    {
      while (SCM_NIMP(obj) && SCM_CONSP(obj))
	{
	  *tail = g_slist_alloc ();
	  if (fromscm)
	    fromscm (SCM_CAR (obj), &(*tail)->data);
	  else
	    (*tail)->data = NULL;
	  obj = SCM_CDR(obj);
	  tail = &(*tail)->next;
	}
    }
  else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
    {
      int len = SCM_LENGTH (obj), i;
      SCM *elts = SCM_VELTS (obj);
      for (i = 0; i < len; i++)
	{
	  *tail = g_slist_alloc ();
	  if (fromscm)
	    fromscm (elts[i], &(*tail)->data);
	  else
	    (*tail)->data = NULL;
	  tail = &(*tail)->next;
	}
    }
  *tail = NULL;
  return res;
}

void
sgtk_slist_finish (GSList *list, SCM obj, SCM (*toscm)(void*))
{
  if (list == NULL)
    return;

  if (toscm)
    {
      if (obj == SCM_EOL || (SCM_NIMP(obj) && SCM_CONSP(obj)))
	{
	  while (SCM_NIMP(obj) && SCM_CONSP(obj) && list)
	    {
	      SCM_SETCAR (obj, toscm (list->data));
	      obj = SCM_CDR(obj);
	      list = list->next;
	    }
	}
      else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
	{
	  int len = SCM_LENGTH (obj), i;
	  SCM *elts = SCM_VELTS (obj);
	  for (i = 0; i < len && list; i++)
	    {
	      elts[i] = toscm (list->data);
	      list = list->next;
	    }
	}
    }

  g_slist_free (list);
}

SCM
sgtk_list2scm (GList *list, SCM (*toscm)(void*))
{
  SCM res, *tail = &res;
  while (list)
    {
      *tail = scm_cons (toscm (&list->data), *tail);
      tail = SCM_CDRLOC (*tail);
      list = list->next;
    }
  *tail = SCM_EOL;
  return res;
}

GList*
sgtk_scm2list (SCM obj, void (*fromscm)(SCM, void*))
{
  GList *res = NULL, *tail;

  if (obj == SCM_BOOL_F)
    return NULL;
  else if (obj == SCM_EOL || (SCM_NIMP(obj) && SCM_CONSP(obj)))
    {
      while (SCM_NIMP(obj) && SCM_CONSP(obj))
      {
        GList *n = g_list_alloc ();
	if (res == NULL)
	  res = tail = n;
	else 
	  {
	    g_list_concat (tail, n);
	    tail = n;
	  }
	if (fromscm)
	  fromscm (SCM_CAR (obj), &(n->data));
	else
	  n->data = NULL;
	obj = SCM_CDR(obj);
      }
    }
  else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
    {
      int len = SCM_LENGTH (obj), i;
      SCM *elts = SCM_VELTS (obj);
      for (i = 0; i < len; i++)
	{
	  GList *n = g_list_alloc ();
	  if (res == NULL)
	    res = tail = n;
	  else 
	    {
	      g_list_concat (tail, n);
	      tail = n;
	    }
	  if (fromscm)
	    fromscm (elts[i], &(n->data));
	  else
	    n->data = NULL;
	}
    }

  return res;
}

void
sgtk_list_finish (GList *list, SCM obj, SCM (*toscm)(void*))
{
  if (list == NULL)
    return;

  if (toscm)
    {
      if (obj == SCM_EOL || (SCM_NIMP(obj) && SCM_CONSP(obj)))
	{
	  while (SCM_NIMP(obj) && SCM_CONSP(obj) && list)
	    {
	      SCM_SETCAR (obj, toscm (list->data));
	      obj = SCM_CDR(obj);
	      list = list->next;
	    }
	}
      else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
	{
	  int len = SCM_LENGTH (obj), i;
	  SCM *elts = SCM_VELTS (obj);
	  for (i = 0; i < len && list; i++)
	    {
	      elts[i] = toscm (list->data);
	      list = list->next;
	    }
	}
    }
  
  g_list_free (list);
}

sgtk_cvec
sgtk_scm2cvec (SCM obj, void (*fromscm)(SCM, void*), size_t sz)
{
  sgtk_cvec res;
  int i;
  char *ptr;

  if (obj == SCM_BOOL_F)
    {
      res.vec = NULL;
      res.count = 0;
    }
  else if ((res.count = scm_ilength (obj)) >= 0)
    {
      res.vec = (void *)scm_must_malloc (res.count * sz, "scm2cvec");
      if (fromscm)
	{
	  for (i = 0, ptr = res.vec; i < res.count; i++, ptr += sz)
	    {
	      fromscm (SCM_CAR (obj), ptr);
	      obj = SCM_CDR(obj);
	    }
	}
      else
	memset (res.vec, 0, res.count * sz);
    }
  else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
    {
      SCM *elts = SCM_VELTS (obj);
      res.count = SCM_LENGTH (obj);
      res.vec = (void *)scm_must_malloc (res.count * sz, "scm2cvec");
      if (fromscm)
	{
	  for (i = 0, ptr = res.vec; i < res.count; i++, ptr += sz)
	    fromscm (elts[i], ptr);
	}
      else
	memset (res.vec, 0, res.count * sz);
    }

  return res;
}

void
sgtk_cvec_finish (sgtk_cvec *cvec, SCM obj, SCM (*toscm)(void *), size_t sz)
{
  if (cvec->vec == NULL)
    return;

  if (toscm)
    {
      if (obj == SCM_EOL || (SCM_NIMP(obj) && SCM_CONSP(obj)))
	{
	  int i, len = cvec->count;
	  char *ptr;

	  for (i = 0, ptr = cvec->vec;
	       i < len && SCM_NIMP(obj) && SCM_CONSP(obj);
	       i++, ptr += sz, obj = SCM_CDR (obj))
	    {
	      SCM_SETCAR (obj, toscm (ptr));
	    }
	}
      else if (SCM_NIMP(obj) && SCM_VECTORP(obj))
	{
	  SCM *elts = SCM_VELTS (obj);
	  int len1 = SCM_LENGTH (obj), len2 = cvec->count, i;
	  char *ptr;

	  for (i = 0, ptr = cvec->vec; i < len1 && i < len2; i++, ptr += sz)
	    elts[i] = toscm (ptr);
	}
    }

  scm_must_free (cvec->vec);
}

SCM
sgtk_cvec2scm (sgtk_cvec *cvec, SCM (*toscm)(void *), size_t sz)
{
    int len, i;
    SCM obj = scm_make_vector (len = cvec->count, SCM_UNSPECIFIED);
    SCM *elts = SCM_VELTS (obj);
    char *ptr;

    for (i = 0, ptr = cvec->vec; i < len; i++, ptr += sz)
	elts[i] = toscm (ptr);

    g_free (cvec->vec);
    return obj;
}

/* converting between SCM and GtkArg */

SCM
sgtk_arg2scm (GtkArg *a, int free_mem)
{
  switch (GTK_FUNDAMENTAL_TYPE (a->type))
    {
    case GTK_TYPE_NONE:
      return SCM_UNSPECIFIED;
    case GTK_TYPE_CHAR:
      return gh_char2scm (GTK_VALUE_CHAR(*a));
    case GTK_TYPE_BOOL:
      return GTK_VALUE_BOOL(*a)? SCM_BOOL_T : SCM_BOOL_F;
    case GTK_TYPE_INT:
      return scm_long2num (GTK_VALUE_INT(*a));
    case GTK_TYPE_UINT:
      return scm_ulong2num (GTK_VALUE_UINT(*a));
    case GTK_TYPE_LONG:
      return scm_long2num (GTK_VALUE_LONG(*a));
    case GTK_TYPE_ULONG:
      return scm_ulong2num (GTK_VALUE_ULONG(*a));
    case GTK_TYPE_FLOAT:
      return sgtk_float2scm (GTK_VALUE_FLOAT(*a));
    case GTK_TYPE_DOUBLE:
      return sgtk_double2scm (GTK_VALUE_DOUBLE(*a));
    case GTK_TYPE_STRING:
      {
	SCM ret = scm_makfrom0str (GTK_VALUE_STRING(*a));
	if (free_mem)
	  g_free GTK_VALUE_STRING(*a);
	return ret;
      }
    case GTK_TYPE_ENUM:
      return sgtk_enum2scm (GTK_VALUE_FLAGS(*a),
			     (sgtk_enum_info *)sgtk_find_type_info (a->type));
    case GTK_TYPE_FLAGS:
      return sgtk_flags2scm (GTK_VALUE_FLAGS(*a),
			     (sgtk_enum_info *)sgtk_find_type_info (a->type));
    case GTK_TYPE_BOXED:
      return sgtk_boxed2scm (GTK_VALUE_BOXED(*a),
			     (sgtk_boxed_info *)sgtk_find_type_info (a->type),
			     TRUE);
#ifndef GTK_2_0
    case GTK_TYPE_OBJECT:
      return sgtk_wrap_gtkobj (GTK_VALUE_OBJECT(*a));
#endif
    default:
      return sgtk_make_illegal_type_object (a->type);
    }
}

int
sgtk_valid_arg (GtkArg *a, SCM obj)
{
  switch (GTK_FUNDAMENTAL_TYPE (a->type))
    {
    case GTK_TYPE_NONE:
      return TRUE;
    case GTK_TYPE_CHAR:
      return gh_char_p (obj);
    case GTK_TYPE_BOOL:
      return TRUE;
    case GTK_TYPE_INT:
    case GTK_TYPE_UINT:
    case GTK_TYPE_LONG:
    case GTK_TYPE_ULONG:
    case GTK_TYPE_FLOAT:
    case GTK_TYPE_DOUBLE:
      return gh_number_p (obj);
    case GTK_TYPE_STRING:
      return gh_string_p (obj);
    case GTK_TYPE_ENUM:
      return sgtk_valid_enum (obj, ((sgtk_enum_info *)
				    sgtk_find_type_info (a->type)));
    case GTK_TYPE_FLAGS:
      return sgtk_valid_flags (obj, ((sgtk_enum_info *)
				     sgtk_find_type_info (a->type)));
    case GTK_TYPE_BOXED:
      return sgtk_valid_boxed (obj, ((sgtk_boxed_info *)
				     sgtk_find_type_info (a->type)));
      break;
#ifndef GTK_2_0
    case GTK_TYPE_CALLBACK:
      return gh_procedure_p (obj);
    case GTK_TYPE_OBJECT:
      return sgtk_is_a_gtkobj (a->type, obj);
#endif
    default:
      fprintf (stderr, "unhandled arg type %s\n", gtk_type_name (a->type));
      return FALSE;
    }
}

void
sgtk_scm2arg (GtkArg *a, SCM obj, SCM protector)
{
  switch (GTK_FUNDAMENTAL_TYPE (a->type))
    {
    case GTK_TYPE_NONE:
      return;
    case GTK_TYPE_CHAR:
      GTK_VALUE_CHAR(*a) = gh_scm2char (obj);
      break;
    case GTK_TYPE_BOOL:
      GTK_VALUE_BOOL(*a) = SCM_NFALSEP (obj);
      break;
    case GTK_TYPE_INT:
      GTK_VALUE_INT(*a) = scm_num2long (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_UINT:
      GTK_VALUE_UINT(*a) = scm_num2ulong (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_LONG:
      GTK_VALUE_LONG(*a) = scm_num2long (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_ULONG:
      GTK_VALUE_ULONG(*a) = scm_num2ulong (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_FLOAT:
      GTK_VALUE_FLOAT(*a) = sgtk_scm2float (obj);
      break;
    case GTK_TYPE_DOUBLE:
      GTK_VALUE_DOUBLE(*a) = sgtk_scm2double (obj);
      break;
    case GTK_TYPE_STRING:
      SCM_COERCE_SUBSTR (obj);
      GTK_VALUE_STRING(*a) = SCM_CHARS(obj);
      break;
    case GTK_TYPE_ENUM:
      GTK_VALUE_ENUM(*a) =
	sgtk_scm2enum (obj, (sgtk_enum_info *)sgtk_find_type_info (a->type),
		       SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_FLAGS:
      GTK_VALUE_ENUM(*a) =
	sgtk_scm2flags (obj, (sgtk_enum_info *)sgtk_find_type_info (a->type),
			SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_BOXED:
      GTK_VALUE_BOXED(*a) = sgtk_scm2boxed (obj);
      break;
#ifndef GTK_2_0
    case GTK_TYPE_CALLBACK:
      sgtk_protect (protector, obj);
      GTK_VALUE_CALLBACK(*a).marshal = sgtk_callback_marshal;
      GTK_VALUE_CALLBACK(*a).data = (gpointer)obj;
      GTK_VALUE_CALLBACK(*a).notify = sgtk_callback_destroy;
      break;
    case GTK_TYPE_OBJECT:
      GTK_VALUE_OBJECT(*a) = sgtk_get_gtkobj (obj);
#endif
      break;
    default:
      fprintf (stderr, "unhandled arg type %s\n", gtk_type_name (a->type));
      break;
    }
}

void
sgtk_scm2ret (GtkArg *a, SCM obj)
{
  switch (GTK_FUNDAMENTAL_TYPE (a->type))
    {
    case GTK_TYPE_NONE:
      return;
    case GTK_TYPE_CHAR:
      *GTK_RETLOC_CHAR(*a) = gh_scm2char (obj);
      break;
    case GTK_TYPE_BOOL:
      *GTK_RETLOC_BOOL(*a) = SCM_NFALSEP (obj);
      break;
    case GTK_TYPE_INT:
      *GTK_RETLOC_INT(*a) = scm_num2long (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_UINT:
      *GTK_RETLOC_UINT(*a) = scm_num2ulong (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_LONG:
      *GTK_RETLOC_LONG(*a) = scm_num2long (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_ULONG:
      *GTK_RETLOC_ULONG(*a) = scm_num2ulong (obj, (char*)SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_FLOAT:
      *GTK_RETLOC_FLOAT(*a) = sgtk_scm2float (obj);
      break;
    case GTK_TYPE_DOUBLE:
      *GTK_RETLOC_DOUBLE(*a) = sgtk_scm2double (obj);
      break;
    case GTK_TYPE_STRING:
      SCM_ASSERT (SCM_NIMP(obj) && SCM_STRINGP(obj), obj, SCM_ARG1,
		  "scm->gtk");
      SCM_COERCE_SUBSTR (obj);
      GTK_VALUE_STRING(*a) = g_strdup (SCM_CHARS(obj));
      break;
    case GTK_TYPE_ENUM:
      *GTK_RETLOC_ENUM(*a) =
	sgtk_scm2enum (obj, (sgtk_enum_info *)sgtk_find_type_info (a->type),
		       SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_FLAGS:
      *GTK_RETLOC_ENUM(*a) =
	sgtk_scm2flags (obj, (sgtk_enum_info *)sgtk_find_type_info (a->type),
		       SCM_ARG1, "scm->gtk");
      break;
    case GTK_TYPE_BOXED:
      *GTK_RETLOC_BOXED(*a) = sgtk_scm2boxed (obj);
      break;
#ifndef GTK_2_0
    case GTK_TYPE_OBJECT:
      SCM_ASSERT (sgtk_is_a_gtkobj (a->type, obj), obj, SCM_ARG1, "scm->gtk");
      *GTK_RETLOC_OBJECT(*a) = sgtk_get_gtkobj (obj);
      break;
#endif
    default:
      fprintf (stderr, "unhandled return type %s\n", gtk_type_name (a->type));
      break;
    }
}

/* Callbacks.

   Callbacks are executed within a new dynamic root.  That means that
   the flow of control can't leave them without Gtk noticing.  Throws
   are catched and briefly reported.  Calls to continuations that have
   been made outside the dynamic root can not be activated.

   Callbacks are invoked with whatever arguments that are specified by
   the Gtk documentation.  They do not, however, receive the GtkObject
   that has initiated the callback.

   When callback_trampoline is non-#f, we treat it as a procedure and
   call it as

      (trampoline proc args)

   PROC is the real callback procedure and ARGS is the list of
   arguments that should be passed to it.  */

static SCM callback_trampoline;

/* The SCM_PROC for gtk-callback-trampoline is in gtk-support.c to
   have it be snarfed for sgtk_init_support */

SCM
sgtk_callback_trampoline (SCM new)
{
  SCM old = SCM_CAR (callback_trampoline);
  if (new != SCM_UNDEFINED)
    SCM_SETCAR (callback_trampoline, new);
  return old;
}

struct callback_info {
  SCM proc;
  gint n_args;
  GtkArg *args;
};

static SCM
inner_callback_marshal (void *data)
{
  struct callback_info *info = (struct callback_info *)data;
  int i;
  SCM args = SCM_EOL, ans;

  for (i = info->n_args-1; i >= 0; i--)
    args = scm_cons (sgtk_arg2scm (info->args+i, 0), args);
  if (SCM_FALSEP (SCM_CAR(callback_trampoline)))
    ans = scm_apply (info->proc, args, SCM_EOL);
  else
    ans = scm_apply (SCM_CAR(callback_trampoline),
		     scm_cons2 (info->proc, args, SCM_EOL), SCM_EOL);
  if (info->args[info->n_args].type != GTK_TYPE_NONE)
    sgtk_scm2ret (info->args+info->n_args, ans);

  return SCM_UNSPECIFIED;
}

/* Be carefull when this macro is true.
   scm_gc_heap_lock is set during gc.  */
#define SCM_GC_P (scm_gc_heap_lock)

void
sgtk_callback_marshal (GtkObject *obj,
		       gpointer data,
		       guint n_args,
		       GtkArg *args)
{
  SCM_STACKITEM stack_item;
  struct callback_info info;

  if (SCM_GC_P)
    {
      /* This should only happen for the "destroy" signal and is then
         harmless. */
      fprintf (stderr, "callback ignored during GC!\n");
      return;
    }
  
  info.proc = ((sgtk_protshell *)data)->object;
  info.n_args = n_args;
  info.args = args;

  scm_internal_cwdr ((scm_catch_body_t)inner_callback_marshal, &info,
		     scm_handle_by_message_noexit, "gtk",
		     &stack_item);
}

void
sgtk_callback_destroy (gpointer data)
{
  sgtk_unprotect ((sgtk_protshell *)data);
}



/* Type conversions */

extern sgtk_boxed_info sgtk_gdk_color_info;

SCM
sgtk_color_conversion (SCM color)
{
  SCM orig_color = color;

  if (SCM_NIMP (color) && SCM_STRINGP (color))
    {
      GdkColor colstruct;
      GdkColormap *colmap;

      SCM_COERCE_SUBSTR (color);
      SCM_DEFER_INTS;
      if (!gdk_color_parse (SCM_CHARS (color), &colstruct))
	{
	  SCM_ALLOW_INTS;
	  scm_misc_error ("string->color",
#ifdef HAVE_SCM_SIMPLE_FORMAT
			  "no such color: ~S",
#else
			  "no such color: %S",
#endif
			  scm_cons (orig_color, SCM_EOL));
	}
      colmap = gtk_widget_peek_colormap ();
      if (!gdk_color_alloc (colmap, &colstruct))
	{
	  SCM_ALLOW_INTS;
	  scm_misc_error ("string->color",
#ifdef HAVE_SCM_SIMPLE_FORMAT
			  "can't allocate color: ~S",
#else
			  "can't allocate color: %S",
#endif
			  scm_cons (orig_color, SCM_EOL));
	}
      SCM_ALLOW_INTS;
      return sgtk_boxed2scm (&colstruct, &sgtk_gdk_color_info, 1);
    }
  return color;
}

extern SCM sgtk_gdk_font_load (SCM font);

SCM
sgtk_font_conversion (SCM font)
{
  SCM orig_font = font;

  if (SCM_NIMP (font) && SCM_STRINGP (font))
    {
      SCM_COERCE_SUBSTR (font);
      font = sgtk_gdk_font_load (font);
      if (font == SCM_BOOL_F)
	scm_misc_error ("string->font",
#ifdef HAVE_SCM_SIMPLE_FORMAT
			"no such font: ~S",
#else
			"no such font: %S",
#endif
			scm_cons (orig_font, SCM_EOL));
    }
  return font;
}

SCM
sgtk_string_conversion (SCM str)
{
  if (SCM_NIMP (str) && SCM_STRINGP (str))
    SCM_COERCE_SUBSTR (str);
  return str;
}



/* Support for gtk_object_new, gtk_object_set, ... */

/* The SCM_PROC for the exported functions is in gtk-support.c to have
   it be snarfed for sgtk_init_gtk_support. */

sgtk_object_info *sgtk_find_object_info (char *name);

sgtk_object_info *
sgtk_find_object_info_from_type (GtkType type)
{
  sgtk_object_info *info;
  if (type == GTK_TYPE_INVALID)
    return NULL;
#ifdef GTK_2_0
  info = (sgtk_object_info *)sgtk_get_type_info (G_TYPE_BRANCH_SEQNO(type));
#else
  info = (sgtk_object_info *)sgtk_get_type_info (GTK_TYPE_SEQNO(type));
#endif

  if (info)
    return info;
  
  return sgtk_find_object_info (gtk_type_name (type));
}

sgtk_object_info *
sgtk_find_object_info (char *name)
{
  GtkType type, parent;
  sgtk_object_info *info;
  type_infos *infos;
  int i;

  type = gtk_type_from_name (name);
  if (type != GTK_TYPE_INVALID)
    {
#ifdef GTK_2_0
      info = (sgtk_object_info *)sgtk_get_type_info (G_TYPE_BRANCH_SEQNO(type));
#else
      info = (sgtk_object_info *)sgtk_get_type_info (GTK_TYPE_SEQNO(type));
#endif
      if (info)
	return info;
    }

  for (infos = all_type_infos; infos; infos = infos->next)
    {
      sgtk_type_info **ip;
      for (ip = infos->infos; *ip; ip++)
	if (!strcmp ((*ip)->name, name))
	  {
	    if (GTK_FUNDAMENTAL_TYPE((*ip)->type) != GTK_TYPE_OBJECT)
	      return NULL;

	    info = (sgtk_object_info *)*ip;
	    info->header.type = info->init_func ();
	    enter_type_info ((sgtk_type_info*)info);
	    goto query_args;
	  }
    }

  /* Not found among our precompiled types.  Construct a fresh
     sgtk_object_info, if it's known to Gtk+. */

  if (type != GTK_TYPE_INVALID)
    {
      fprintf (stderr, "Fresh info for %s, %d\n", name, type);

      info = (sgtk_object_info *)scm_must_malloc (sizeof(sgtk_object_info),
						  "sgtk_object_info");
      info->header.type = type;
      info->header.name = name;
      info->init_func = NULL;
      enter_type_info ((sgtk_type_info*)info);
    }
  else
    return NULL;

 query_args:
  gtk_type_class (info->header.type);
  info->args = gtk_object_query_args (info->header.type,
				      &info->args_flags,
				      &info->n_args);
  info->args_short_names =
    (char **)scm_must_malloc (info->n_args*(sizeof(char*)),
			      "args short names");
  for (i = 0; i < info->n_args; i++)
    {
      char *l = info->args[i].name;
      char *d = strchr (l, ':');
      if (d == NULL || d[1] != ':')
	{
	  fprintf (stderr, "`%s' has no class part.\n", l);
	  info->args_short_names[i] = l;
	}
      else
	info->args_short_names[i] = d+2;
    }
  
  parent = gtk_type_parent (info->header.type);
  if (parent != GTK_TYPE_INVALID)
    info->parent = sgtk_find_object_info_from_type (parent);
  else
    info->parent = NULL;
  
  return info;
}

#ifdef NEED_UNUSED_CODE
static char*
xstrndup (char *str, int n)
{
  char *dup;

  if (str == NULL)
    return NULL;
  dup = scm_must_malloc (n+1, "xstrndup");
  strncpy (dup, str, n);
  dup[n] = '\0';
  return dup;
}
#endif

static void
sgtk_find_arg_info (GtkArg *arg, sgtk_object_info *info, char *name)
{
  /* XXX - handle signal handlers.  Do not use '::', use '.' instead. */

  char *d = strchr (name, ':');
  if (d && d[1] == ':')
    {
      /* A long name.  Find the object_info for the class part. */
      int len = d-name;

      while (info)
	{
	  if (info->header.name[len] == '\0'
	      && !strncmp (info->header.name, name, len))
	    break;
	  info = info->parent;
	}
      name = d+2;
    }
  
#ifdef DEBUG_PRINT
  fprintf (stderr, "searching short `%s'\n", name);
#endif
  while (info)
    {
      int i;
      for (i = 0; i < info->n_args; i++)
	{
#ifdef DEBUG_PRINT
	  fprintf (stderr, " on %s\n", info->args[i].name);
#endif
	  if (!strcmp (info->args_short_names[i], name))
	    {
	      *arg = info->args[i];
	      return;
	    }
	}
      info = info->parent;
    }
  
  arg->type = GTK_TYPE_INVALID;
  return;
}
      
GtkArg*
sgtk_build_args (sgtk_object_info *info, int *n_argsp, SCM scm_args,
		 SCM protector, char *subr)
{
  int i, n_args = *n_argsp;
  GtkArg *args;
  char *name;
  SCM kw, val;
  sgtk_type_info *type_info;

  args = g_new0 (GtkArg, n_args);

  for (i = 0; i < n_args; i++)
    {
      kw = SCM_CAR (scm_args);
      val = SCM_CADR (scm_args);
      scm_args = SCM_CDDR (scm_args);

      if (SCM_NIMP (kw) && SCM_SYMBOLP (kw))
	name = SCM_CHARS(kw);
      else if (SCM_NIMP (kw) && SCM_KEYWORDP (kw))
	name = SCM_CHARS(SCM_KEYWORDSYM(kw))+1;
      else
	{
	  fprintf (stderr, "bad keyword\n");
	  n_args -= 1;
	  i -= 1;
	  continue;
	}

      sgtk_find_arg_info (&args[i], info, name);
      if (args[i].type == GTK_TYPE_INVALID)
	{
	  fprintf (stderr, "no such arg for type `%s': %s\n",
		   info->header.name, name);
	  n_args -= 1;
	  i -= 1;
	  continue;
	}

      /* XXX - rethink type info business.  Avoid double lookups. */

      type_info = sgtk_maybe_find_type_info (args[i].type);
      if (type_info && type_info->conversion)
	val = type_info->conversion (val);

      if (!sgtk_valid_arg (&args[i], val))
	{
	  SCM throw_args = 
	    SCM_LIST2 (scm_makfrom0str (gtk_type_name (args[i].type)),
		       val);
	  g_free (args);
	  SCM_ALLOW_INTS;
	  scm_misc_error (subr,
#ifdef HAVE_SCM_SIMPLE_FORMAT
			  "wrong type for ~A: ~S",
#else
			  "wrong type for %s: %S",
#endif
			  throw_args);
	}
	  
      sgtk_scm2arg (&args[i], val, protector);
    }

  *n_argsp = n_args;
  return args;
}

SCM
sgtk_gtk_object_new (SCM type_obj, SCM scm_args)
{
  int n_args;
  sgtk_object_info *info;
  GtkArg *args;
  GtkObject *obj;
  SCM scm_obj;

  SCM_ASSERT (type_obj != SCM_BOOL_F && sgtk_valid_type (type_obj), type_obj,
	      SCM_ARG1, "gtk-object-new");
  n_args = scm_ilength (scm_args);
  SCM_ASSERT (n_args >= 0 && (n_args%2) == 0, scm_args,
	      SCM_ARG2, "gtk-object-new");
  n_args = n_args/2;

  info = sgtk_find_object_info_from_type (sgtk_scm2type (type_obj));
  SCM_ASSERT (info != NULL, type_obj, SCM_ARG1, "gtk-object-new");

  SCM_DEFER_INTS;
  obj = gtk_object_new (info->header.type, NULL);
  scm_obj = sgtk_wrap_gtkobj (obj);
  args = sgtk_build_args (info, &n_args, scm_args, scm_obj, "gtk-object-new");
  gtk_object_setv (obj, n_args, args);
  g_free (args);
  SCM_ALLOW_INTS;

  return scm_obj;
}

SCM
sgtk_gtk_object_set (SCM scm_obj, SCM scm_args)
{
  int n_args;
  sgtk_object_info *info;
  GtkArg *args;
  GtkObject *obj;

  SCM_ASSERT (GTKOBJP(scm_obj), scm_obj, SCM_ARG1, "gtk-object-set");
  n_args = scm_ilength (scm_args);
  SCM_ASSERT (n_args >= 0 && (n_args%2) == 0, scm_args,
	      SCM_ARG2, "gtk-object-set");
  n_args = n_args/2;

  obj = GTKOBJ_PROXY(scm_obj)->obj;
  info = sgtk_find_object_info_from_type (GTK_OBJECT_TYPE(obj));
  SCM_ASSERT (info != NULL, scm_obj, SCM_ARG1, "gtk-object-set");
  
  SCM_DEFER_INTS;
  args = sgtk_build_args (info, &n_args, scm_args, scm_obj, "gtk-object-set");
  gtk_object_setv (obj, n_args, args);
  g_free (args);
  SCM_ALLOW_INTS;

  return SCM_UNSPECIFIED;
}

SCM
sgtk_gtk_object_get (SCM scm_obj, SCM argsym)
{
  GtkObject *obj;
  sgtk_object_info *info;
  char *name;
  GtkArg arg;

  SCM_ASSERT (GTKOBJP(scm_obj), scm_obj, SCM_ARG1, "gtk-object-get");
  SCM_ASSERT (SCM_NIMP(argsym) &&
	      (SCM_KEYWORDP(argsym) || SCM_SYMBOLP(argsym)), argsym,
	      SCM_ARG2, "gtk-object-get");

  obj = GTKOBJ_PROXY(scm_obj)->obj;
  info = sgtk_find_object_info_from_type (GTK_OBJECT_TYPE(obj));
  SCM_ASSERT (info != NULL, scm_obj, SCM_ARG1, "gtk-object-get");

  if (SCM_SYMBOLP(argsym))
    name = SCM_CHARS(argsym);
  else
    name = SCM_CHARS(SCM_KEYWORDSYM(argsym))+1;
  sgtk_find_arg_info (&arg, info, name);

  SCM_DEFER_INTS;
  if (arg.type != GTK_TYPE_INVALID)
    gtk_object_getv (obj, 1, &arg);
  SCM_ALLOW_INTS;

  if (arg.type == GTK_TYPE_INVALID)
    return SCM_BOOL_F;
  else
    return sgtk_arg2scm (&arg, 1);
}



/* Creating new object classes */

GtkType
gtk_class_new (GtkType parent_type, gchar *name)
{
  GtkTypeInfo info = { 0 };
  GtkTypeInfo parent_info;

  if (!gtk_type_get_info (parent_type, &parent_info))
    return GTK_TYPE_INVALID;

  info.type_name = name;
  info.object_size = parent_info.object_size;
  info.class_size = parent_info.class_size;
  info.class_init_func = NULL;
  info.object_init_func = NULL;
#if GTK_MAJOR_VERSION > 1 || GTK_MINOR_VERSION > 0
  info.base_class_init_func = NULL;
#endif

  return gtk_type_unique (parent_type, &info);
}

guint
gtk_signal_new_generic (const gchar     *name,
			GtkSignalRunType signal_flags,
			GtkType          type,
			GtkType          return_type,
			guint            nparams,
			GtkType         *params)
{
  guint signal_id;

  if (GTK_FUNDAMENTAL_TYPE (type) != GTK_TYPE_OBJECT)
    return 0;

  signal_id = gtk_signal_newv (name, signal_flags, type,
			       0, NULL,
			       return_type, nparams, params);
  if (signal_id > 0)
    gtk_object_class_add_signals (gtk_type_class (type),
				  &signal_id, 1);

  return signal_id;
}

void
sgtk_signal_emit (GtkObject *obj, char *name, SCM scm_args)
{
#ifdef GTK_2_0
  GSignalQuery info;
#else
  GtkSignalQuery *info;
#endif
  guint signal_id, i;
  guint n_params;
  GtkArg *args;

  signal_id = gtk_signal_lookup (name, GTK_OBJECT_TYPE (obj));
#ifdef GTK_2_0
  if (signal_id != 0)
    {
      g_signal_query (signal_id, &info);
      signal_id = info.signal_id;
    }
#endif
  if (signal_id == 0)
    {
      SCM_ALLOW_INTS;
      scm_misc_error ("gtk-signal-emit",
#ifdef HAVE_SCM_SIMPLE_FORMAT
		      "no such signal: ~S",
#else
		      "no such signal: %S",
#endif
		      scm_cons (scm_makfrom0str (name), SCM_EOL));
    }

#ifdef GTK_2_0
  n_params = info.n_params;
#else
  info = gtk_signal_query (signal_id);
  n_params = info->nparams;
#endif
  if (scm_ilength (scm_args) != n_params)
    {
#ifndef GTK_2_0
      g_free (info);
#endif
      SCM_ALLOW_INTS;
      scm_misc_error ("gtk-signal-emit", "wrong number of signal arguments",
		      SCM_EOL);
    }

  args = g_new (GtkArg, n_params+1);
  i = 0;
  while (SCM_NIMP (scm_args))
    {
      args[i].name = NULL;
#ifdef GTK_2_0
      args[i].type = info.param_types[i];
#else
      args[i].type = info->params[i];
#endif

      if (!sgtk_valid_arg (&args[i], SCM_CAR (scm_args)))
	{
	  SCM throw_args =
	    SCM_LIST2 (scm_makfrom0str (gtk_type_name (args[i].type)),
		       SCM_CAR (scm_args));
	  g_free (args);
	  SCM_ALLOW_INTS;
	  scm_misc_error ("gtk-signal-emit",
#ifdef HAVE_SCM_SIMPLE_FORMAT
			  "wrong type for ~A: ~S",
#else
			  "wrong type for %s: %S",
#endif
			  throw_args);
	}

      sgtk_scm2arg (&args[i], SCM_CAR(scm_args), SCM_BOOL_T);
      i++;
      scm_args = SCM_CDR (scm_args);
    }
  args[i].type = GTK_TYPE_NONE;

  gtk_signal_emitv (obj, signal_id, args);

  g_free (args);
#ifndef GTK_2_0
  g_free (info);
#endif
}



/* Initialization */

static int standalone_p = 1;

void
sgtk_set_standalone (int flag)
{
  standalone_p = flag;
}

int
sgtk_is_standalone ()
{
  return standalone_p;
}

SCM
sgtk_standalone_p ()
{
  return standalone_p? SCM_BOOL_T : SCM_BOOL_F;
}

void
sgtk_register_glue (char *name, void (*func)(void))
{
  static char modprefix[] = "gtk %static-initfuncs% ";
  char *full_name;

  full_name = malloc (strlen (name) + sizeof (modprefix) + 1);
  if (full_name == NULL)
    return;

  strcpy (full_name, modprefix);
  strcat (full_name, name);
  scm_register_module_xxx (full_name, func);
}

SCM_SYMBOL (sym_top_repl, "top-repl");
SCM_SYMBOL (sym_quit, "quit");
SCM_SYMBOL (sym_use_modules, "use-modules");
SCM_SYMBOL (sym_gtk, "gtk");
SCM_SYMBOL (sym_repl, "repl");
SCM_SYMBOL (sym_gtk_repl, "gtk-repl");

#if GTK_MAJOR_VERSION == 1 && GTK_MINOR_VERSION == 2
SCM_SYMBOL (sym_gtk_version, "gtk-1.2");
#elif GTK_MAJOR_VERSION == 1 && GTK_MINOR_VERSION == 3
SCM_SYMBOL (sym_gtk_version, "gtk-1.3");
#else
#error can only deal with gtk-1.2 and gtk-1.3
#endif

static void
sgtk_init_substrate (void)
{	
  
  tc16_gtkobj_marker_hook = scm_make_smob_type ("gtkobj_marker_hook", sizeof(sgtk_object_proxy));
  scm_set_smob_mark (tc16_gtkobj_marker_hook, gtkobj_marker_hook);
  scm_set_smob_print (tc16_gtkobj_marker_hook, gtkobj_marker_hook_print);
    
  tc16_gtkobj = scm_make_smob_type ("gtkobj", sizeof(sgtk_object_proxy));
  scm_set_smob_mark (tc16_gtkobj, gtkobj_mark);
  scm_set_smob_free (tc16_gtkobj, gtkobj_free);
  scm_set_smob_print (tc16_gtkobj, gtkobj_print);
  
  tc16_boxed = scm_make_smob_type ("gtkboxed", sizeof(sgtk_boxed_info));
  scm_set_smob_mark (tc16_boxed, scm_mark0);
  scm_set_smob_free (tc16_boxed, boxed_free);
  scm_set_smob_print (tc16_boxed, boxed_print);
   
  tc16_gtktype = scm_make_smob_type ("gtktype", sizeof(sgtk_type_info));
  scm_set_smob_mark (tc16_gtktype, scm_mark0);
  scm_set_smob_free (tc16_gtktype, scm_free0);
  scm_set_smob_print (tc16_gtktype, gtktype_print);
  scm_set_smob_equalp (tc16_gtktype, gtktype_equalp);
  
  tc16_illobj = scm_make_smob_type ("gtkillobj", sizeof(GtkType));
  scm_set_smob_mark (tc16_illobj, scm_mark0);
  scm_set_smob_free (tc16_illobj, scm_free0);
  scm_set_smob_print (tc16_illobj, illobj_print);
  
  global_protects = NULL;
  sgtk_protshell_chunk = g_mem_chunk_create (sgtk_protshell, 128,
					     G_ALLOC_AND_FREE);
  install_marker_hook ();

  callback_trampoline = scm_permanent_object (scm_cons (SCM_BOOL_F, SCM_EOL));

  sgtk_init_threads ();

#ifndef SCM_MAGIC_SNARFER
#ifndef MKDEP
#include "guile-gtk.x"
#endif /* MKDEP */
#endif /* SCM_MAGIC_SNARFER */
}

static int sgtk_inited = 0;

void
sgtk_init_with_args (int *argcp, char ***argvp)
{
  if (sgtk_inited)
    return;

  /* XXX - Initialize Gtk only once.  We assume that Gtk has already
     been initialized when Gdk has.  That is not completely correct,
     but the best I can do. */

  if (gdk_display == NULL)
    {
      gtk_set_locale ();
      gtk_init (argcp, argvp);
    }
  sgtk_init_substrate ();
  sgtk_inited = 1;
}

static char*
xstrdup (char *str)
{
  if (str)
    {
      char *newstr = scm_must_malloc (strlen(str)+1, "strdup");
      strcpy (newstr, str);
      return newstr;
    }
  else
    return NULL;
}

static void
make_argv (SCM list, int *argc, char ***argv)
{
  static char *argv_storage[1] = { "guile-gtk" };

  int c = scm_ilength (list), i;
  char **v;

  *argv = argv_storage;
  *argc = 1;

  if (c < 0)
    return;

  v = (char **)scm_must_malloc ((c+1) * sizeof(char**), "make-argv");
  for (i = 0; i < c; i++, list = SCM_CDR (list))
    {
      if (SCM_IMP (SCM_CAR (list)) || SCM_NSTRINGP (SCM_CAR (list)))
	{
	  scm_must_free ((char *)v);
	  return;
	}
      v[i] = xstrdup (SCM_CHARS (SCM_CAR (list)));
    }
  v[c] = NULL;
  
  *argv = v;
  *argc = c;
}

void
sgtk_init ()
{
  int argc;
  char **argv;

  make_argv (scm_program_arguments (), &argc, &argv);
  sgtk_init_with_args (&argc, &argv);
  scm_set_program_arguments (argc, argv, NULL);
}

static SCM
hack_compiled_switches (SCM script)
{
  SCM last_action;

  script = scm_reverse_x (script, SCM_UNDEFINED);
  last_action = SCM_CAR (script);
  SCM_SETCAR (script, SCM_LIST2 (sym_use_modules,
				 SCM_LIST2 (sym_gtk_version, sym_gtk)));
  script = scm_cons (SCM_LIST2 (sym_use_modules,
				SCM_LIST2 (sym_gtk, sym_repl)),
		     script);
  
  if (SCM_CAR (last_action) == sym_top_repl)
    {
      script = scm_cons (SCM_LIST1 (sym_gtk_repl), script);
      sgtk_set_standalone (0);
    }
  else if (SCM_CAR (last_action) != sym_quit)
    {
      fprintf (stderr, "guile-gtk: unknown action in startup script\n");
      scm_display (last_action, SCM_UNDEFINED);
      scm_newline (SCM_UNDEFINED);
      exit (1);
    }

  return scm_reverse_x (script, SCM_UNDEFINED);
}

void
sgtk_shell (int argc, char **argv)
{
  SCM script;

  sgtk_init_with_args (&argc, &argv);

  /* If present, add SCSH-style meta-arguments from the top of the
     script file to the argument vector.  See the SCSH manual: "The
     meta argument" for more details.  */
  {
    char **new_argv = scm_get_meta_args (argc, argv);

    if (new_argv)
      {
	argv = new_argv;
	argc = scm_count_argv (new_argv);
      }
  }

  script = hack_compiled_switches (scm_compile_shell_switches (argc, argv));
#ifdef SCM_EVAL_X_TWO_ARGS
  scm_eval_x (script, scm_the_root_module ());
#else
  scm_eval_x (script);
#endif
  exit (0);
}
