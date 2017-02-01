/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Authors: Jaka Mocnik  <jaka.mocnik@kiss.uni-lj.si>
 */

#ifndef __PREFS_H__
#define __PREFS_H__

#include <gtk/gtkwidget.h>

#include "gtkgs.h"
#include "ggvwindow.h"

/* command lines of the external programmes: */
extern gchar *gs_cmd;           /* ghostscript */
extern gchar *gs_scan_pdf_cmd;  /* ghostscript and command line options
				 * needed to scan a pdf file
				 * printf format; 2 char* arguments: pdf file, dsc file
				 */
extern gchar *gs_ungzip_cmd;    /* uncompress stdin to stdout via gzip */
extern gchar *gs_unbzip2_cmd;   /* uncompress stdin to stdout via bzip2 */
extern gchar *gs_print_cmd;     /* print command: printf format, char* argument */

extern gboolean gs_panel;       /* panel visible */
extern gboolean gs_toolbar;     /* toolbar visible */
extern gboolean gs_menubar;     /* menubar visible */
extern gboolean gs_auto_jump;   /* to top of page */
extern gint gs_default_magnification; /* Default magnification for ggv */


/* We are not going to use this any more. Instead we use the
   gtk_gs_get, and gtk_gs_set... functions */

/* 
extern gint gs_def_media;
extern gboolean gs_override_media;
extern gboolean gs_antialiased;
extern gboolean gs_watch_doc;
*/

extern gboolean gs_save_geometry; /* Save the current geometry for next session */
extern gint ggv_default_width, file_sel_width;
extern gint ggv_default_height, file_sel_height;

void load_prefs (gchar *);
void save_prefs (gchar *);
void set_prefs  (ggv_window *);
void synchonize_menu_toggles (ggv_window *ggv);


#define MAX_RECENT 6

#define DEFAULT_WINDOW_WIDTH  640
#define DEFAULT_WINDOW_HEIGHT 480

#define DEFAULT_FILE_SEL_WIDTH   320
#define DEFAULT_FILE_SEL_HEIGHT  256

#endif
