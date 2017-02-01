#ifndef __GDICT_SPELLER_H_
#define __GDICT_SPELLER_H_

/* $Id: gdict-speller.h,v 1.1 2000/04/06 00:46:16 hovinen Exp $ */

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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtk/gtk.h>
#include <gnome.h>

#include "dict.h"

#define GDICT_SPELLER(obj)         GTK_CHECK_CAST (obj, gdict_speller_get_type (), GDictSpeller)
#define GDICT_SPELLER_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gdict_speller_get_type (), GDictSpellerClass)
#define IS_GDICT_SPELLER(obj)      GTK_CHECK_TYPE (obj, gdict_speller_get_type ())

typedef struct _GDictSpeller        GDictSpeller;
typedef struct _GDictSpellerClass   GDictSpellerClass;

struct _GDictSpeller {
    GnomeDialog     dialog;
    
    dict_context_t *context;
    dict_command_t *get_strat_cmd;
    dict_command_t *spell_cmd;
    gchar          *database;
    gchar          *strat;
    
    GtkTable       *table;
    GtkEntry       *word_entry;
    GtkOptionMenu  *strat_sel;
    GtkMenu        *strat_list;
    guint           strat_idx;
    GtkCList       *word_sel;
    
    gchar          *current_word;
};

struct _GDictSpellerClass {
    GnomeDialogClass  parent_class;
    
    void (*word_lookup_start) (GDictSpeller *);
    void (*word_lookup_done)  (GDictSpeller *);
    void (*word_not_found)    (GDictSpeller *);
    void (*socket_error)      (GDictSpeller *, gchar *);
};

guint      gdict_speller_get_type    (void);

GtkWidget *gdict_speller_new         (dict_context_t *context);
void       gdict_speller_destroy     (GDictSpeller *speller);

gint       gdict_speller_lookup      (GDictSpeller *speller, gchar *text);
void       gdict_speller_clear       (GDictSpeller *speller);
void       gdict_speller_reset       (GDictSpeller *speller,
                                      dict_context_t *context);

gchar     *gdict_speller_get_word    (GDictSpeller *speller);

void       gdict_speller_reset_strat (GDictSpeller *speller);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GDICT_SPELLER_H_ */
