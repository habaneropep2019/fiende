#ifndef __GDICT_PREF_H_
#define __GDICT_PREF_H_

/* $Id: gdict-pref.h,v 1.4 2000/04/06 00:46:16 hovinen Exp $ */

/*
 *  Papadimitriou Spiros <spapadim@cs.cmu.edu>
 *  Mike Hughes <mfh@psilord.com>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 * 
 *  GDict preferences
 *
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtk/gtk.h>

#include "dict.h"

enum {
    TYPEFACE_HEADWORD,              /* Headword */
    TYPEFACE_NUMBER,                /* Sub-definition number */
    TYPEFACE_PRONUNCIATION,         /* Pronunciation */
    TYPEFACE_ETYMOLOGY,             /* Etymology */
    TYPEFACE_PART,                  /* Part of speech */
    TYPEFACE_EXAMPLE,               /* Example phrase */
    TYPEFACE_BODY,                  /* Definition body */
    TYPEFACE_XREF,                  /* Cross-reference */
    NUM_TYPEFACES
};

typedef struct _GDictTypeface {
    GdkFont   *font;
    gchar     *font_name;
    GdkColor  *color;
} GDictTypeface;

typedef struct _GDictPref {
    /* Server preferences */
    gchar        *server;
    gint          port;
    gboolean      smart;        /* Auto-detect patterns */
    gchar        *database;     /* Database to search for words and defs */
    gchar        *dfl_strat;    /* Default match strategy */

    /* Applet preferences (read only during startup) */
    gboolean      applet_handle;

    /* Font preferences */
    GDictTypeface typefaces[NUM_TYPEFACES];
} GDictPref;

extern GDictPref gdict_pref;

void gdict_pref_load (void);
void gdict_pref_save (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GDICT_PREF_H_ */
