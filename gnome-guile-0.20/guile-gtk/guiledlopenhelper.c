/*
 * Copyright (C) 1997, 1998, 1999 Marius Vollmer
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

#include <libguile.h>

/* XXX - the HAVE_DLOPEN leaks from libguile/scmconfig.h. */

#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#endif

SCM
sgtk_dlopen (SCM name, SCM fullname)
{
  void *handle;

  SCM_ASSERT (SCM_NIMP(name) && SCM_STRINGP(name), name,
	      SCM_ARG1, "%sgtk-dlopen");
  SCM_COERCE_SUBSTR (name);

  SCM_ASSERT (SCM_NIMP(fullname) && SCM_STRINGP(fullname), fullname,
	      SCM_ARG2, "%sgtk-dlopen");
  SCM_COERCE_SUBSTR (fullname);

  SCM_DEFER_INTS;
#ifdef HAVE_DLOPEN
  /* BSD-like systems don't have RTLD_GLOBAL.
   *
   * [FIXME]: If RTLD_GLOBAL is not defined, we assume that dlopen ()
   *          requires a full pathname and can't do the LD_LIBRARY_PATH
   *          search for us.
   *
   *          This is the case on BSD-like systems (FreeBSD), but we should
   *          use a better check for it.
   *
   * Oct 24 1998, Martin.
   */
#ifdef RTLD_GLOBAL
#if (defined __NetBSD__)
  handle = dlopen (SCM_CHARS (fullname), RTLD_LAZY|RTLD_GLOBAL);
#else
  handle = dlopen (SCM_CHARS (name), RTLD_LAZY|RTLD_GLOBAL);
#endif
#else
  handle = dlopen (SCM_CHARS (fullname), RTLD_LAZY);
#endif
  if (handle == NULL)
    fprintf (stderr, "dlopen: %s\n", dlerror ());
#else
  handle = NULL;
#endif
  SCM_ALLOW_INTS;
  
  return handle? scm_ulong2num ((unsigned long)handle) : SCM_BOOL_F;
}

SCM
sgtk_dlinit (SCM sym, SCM lib)
{
  void *handle, (*func)();

  SCM_ASSERT (SCM_NIMP(sym) && SCM_STRINGP(sym), sym,
	      SCM_ARG1, "%sgtk-dlinit");
  handle = (void *)scm_num2ulong (lib, (char *)SCM_ARG2, "%sgtk-dlinit");
  SCM_COERCE_SUBSTR (sym);
  
  SCM_DEFER_INTS;
#ifdef HAVE_DLOPEN
#if (defined __NetBSD__)
  do {
    char *symbol = scm_must_malloc (strlen (SCM_CHARS (sym)) + 2, "dlsym");

    sprintf (symbol, "_%s", SCM_CHARS (sym));
    func = (void (*)())dlsym (handle, symbol);
    scm_must_free (symbol);
  } while (0);
#else
  func = (void (*)())dlsym (handle, SCM_CHARS (sym));
#endif
  if (func)
    func ();
  else
    fprintf (stderr, "dlsym (%s): %s\n", SCM_CHARS (sym), dlerror ());
#endif
  SCM_ALLOW_INTS;
  
  return SCM_UNDEFINED;
}

void
sgtk_dlopenhelper_init ()
{
  scm_make_gsubr ("%sgtk-dlopen", 2, 0, 0, sgtk_dlopen);
  scm_make_gsubr ("%sgtk-dlinit", 2, 0, 0, sgtk_dlinit);
}
