#include <config.h>
#include <guile-gtk.h>

#ifndef HAVE_SCM_DONE_MALLOC

/* Announce that there has been some malloc done that will be freed
 * during gc.  A typical use is for a smob that uses some malloced
 * memory but can not get it from scm_must_malloc (for whatever
 * reason).  When a new object of this smob is created you call
 * scm_done_malloc with the size of the object.  When your smob free
 * function is called, be sure to include this size in the return
 * value. */

#define SCM_MTRIGGER_HYSTERESIS 10000

void
scm_done_malloc (size)
     long size;
{
  scm_mallocated += size;

  if (scm_mallocated > scm_mtrigger)
    {
      scm_igc ("foreign mallocs");
      if (scm_mallocated > scm_mtrigger - SCM_MTRIGGER_HYSTERESIS)
	{
	  if (scm_mallocated > scm_mtrigger)
	    scm_mtrigger = scm_mallocated + scm_mallocated / 2;
	  else
	    scm_mtrigger += scm_mtrigger / 2;
	}
    }
}

#endif

#ifndef HAVE_SCM_INTERNAL_CWDR

/* Simulate cwdr with catch.  It is not the same, but only
 * continuations will skrew us, I think. */

SCM scm_internal_cwdr (scm_catch_body_t body,
		       void *body_data,
		       scm_catch_handler_t handler,
		       void *handler_data,
		       SCM_STACKITEM *stack_start)
{
  return scm_internal_catch (SCM_BOOL_T,
			     body, body_data,
			     handler, handler_data);
}

#endif

#ifndef HAVE_SCM_PUTS

void
scm_puts (char *str, SCM port)
{
  scm_gen_puts (scm_regular_string, str, port);
}

#endif

