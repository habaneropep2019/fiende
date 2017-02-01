
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <config.h>
#include <gnome.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "dummy.h"

extern GnomeMDI *mdi;
extern gboolean gtop_is_running;

extern GnomeUIInfo fileMenu [];

extern void gtop_mdi_init (void);
extern void gtop_mdi_start (gboolean);
extern void gtop_quit (void);

extern void gtop_init_libgtop (void);
extern gint gtop_libgtop_is_summary_supported (gint);

extern void gtop_check_old_config (void);

#endif
