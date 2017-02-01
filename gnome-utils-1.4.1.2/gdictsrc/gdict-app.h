#ifndef __GDICT_APP_H_
#define __GDICT_APP_H_

/* $Id: gdict-app.h,v 1.8 2000/05/19 00:18:59 hovinen Exp $ */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim@cs.cmu.edu>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict main window
 *
 */

#include "dict.h"

#ifdef HAVE_GNOME_PRINT
#  include <libgnomeprint/gnome-print.h>
#endif /* HAVE_GNOME_PRINT */

#include "gdict-defbox.h"
#include "gdict-speller.h"
#include "gdict-pref-dialog.h"

extern GtkWidget *gdict_app;
extern GtkWidget *gdict_appbar;
extern GtkWidget *word_entry;
extern GDictDefbox *defbox;
extern GDictSpeller *speller;
extern GtkWidget *pref_dialog;

#ifdef HAVE_GNOME_PRINT
extern GnomePrinter *gdict_printer;
#endif /* HAVE_GNOME_PRINT */

extern dict_context_t *context;

gint gdict_init_context (void);
void gdict_open_speller (void);
gint gdict_spell (gchar *text, gboolean pattern);
void gdict_app_clear (void);
void gdict_app_do_lookup (gchar *text);
void gdict_app_show_preferences (void);
void gdict_not_online (void);
GtkWidget *gdict_app_create (void);

#endif /* __GDICT_APP_H_ */
