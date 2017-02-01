#ifndef __GDICT_DEFBOX_H_
#define __GDICT_DEFBOX_H_

/* $Id: gdict-defbox.h,v 1.2 2000/05/14 19:48:58 jirka Exp $ */

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

#include "dict.h"

#define GDICT_DEFBOX(obj)         GTK_CHECK_CAST (obj, gdict_defbox_get_type (), GDictDefbox)
#define GDICT_DEFBOX_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gdict_defbox_get_type (), GDictDefboxClass)
#define IS_GDICT_DEFBOX(obj)      GTK_CHECK_TYPE (obj, gdict_defbox_get_type ())

typedef struct _GDictDefbox        GDictDefbox;
typedef struct _GDictDefboxClass   GDictDefboxClass;

struct _GDictDefbox {
    GtkText         text;
    
    dict_context_t *context;
    dict_command_t *def_cmd;
    gchar          *database;
};

struct _GDictDefboxClass {
    GtkTextClass    parent_class;
    
    void (*word_lookup_start) (GDictDefbox *);
    void (*word_lookup_done)  (GDictDefbox *);
    void (*word_not_found)    (GDictDefbox *);
    void (*substr_not_found)  (GDictDefbox *);
    void (*socket_error)      (GDictDefbox *, gchar *);
};

guint      gdict_defbox_get_type   (void);

GtkWidget *gdict_defbox_new        (void);

void       gdict_defbox_destroy    (GDictDefbox *defbox);

gint       gdict_defbox_lookup     (GDictDefbox *defbox, gchar *text);
void       gdict_defbox_clear      (GDictDefbox *defbox);
void       gdict_defbox_find       (GDictDefbox *defbox, gchar *text,
                                    gboolean start);
void       gdict_defbox_reset      (GDictDefbox *defbox,
                                    dict_context_t *context);

#ifdef HAVE_GNOME_PRINT
void       gdict_defbox_print      (GDictDefbox *defbox);
#endif

gchar     *gdict_defbox_get_word   (GDictDefbox *defbox);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GDICT_DEFBOX_H_ */
