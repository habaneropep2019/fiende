#ifndef __GDICT_PREF_DIALOG_H_
#define __GDICT_PREF_DIALOG_H_

/* $Id: gdict-pref-dialog.h,v 1.1 2000/04/06 00:46:16 hovinen Exp $ */

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
#include "gdict-pref.h"

#define GDICT_PREF_DIALOG(obj)         GTK_CHECK_CAST (obj, gdict_pref_dialog_get_type (), GDictPrefDialog)
#define GDICT_PREF_DIALOG_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gdict_pref_dialog_get_type (), GDictPrefDialogClass)
#define IS_GDICT_PREF_DIALOG(obj)      GTK_CHECK_TYPE (obj, gdict_pref_dialog_get_type ())

typedef struct _GDictPrefDialog        GDictPrefDialog;
typedef struct _GDictPrefDialogClass   GDictPrefDialogClass;

struct _GDictPrefDialog {
    GnomeDialog       dialog;
    
    GtkTable         *table;
    GtkEntry         *server_entry;
    GtkEntry         *port_entry;
    GtkCheckButton   *smart_lookup_btn;
    GtkCheckButton   *applet_handle_btn;
    GtkOptionMenu    *db_sel;
    GtkMenu          *db_list;
    GtkOptionMenu    *strat_sel;
    GtkMenu          *strat_list;
    
    GnomeFontPicker  *font_pickers[NUM_TYPEFACES];
    GnomeColorPicker *color_pickers[NUM_TYPEFACES];
    
    dict_context_t   *context;
    dict_command_t   *get_db_cmd;
    dict_command_t   *get_strat_cmd;
    
    gchar            *database;
    guint             database_idx;
    gchar            *dfl_strat;
    guint             dfl_strat_idx;
};

struct _GDictPrefDialogClass {
    GnomeDialogClass parent_class;
    
    void (*apply_changes) (GDictPrefDialog *);
    void (*socket_error)  (GDictPrefDialog *, gchar *);
};

guint      gdict_pref_dialog_get_type   (void);

GtkWidget *gdict_pref_dialog_new        (dict_context_t *context);
void gdict_pref_dialog_destroy          (GDictPrefDialog *dialog);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GDICT_PREF_DIALOG_H_ */
