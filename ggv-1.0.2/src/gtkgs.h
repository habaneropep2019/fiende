/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Ghostscript widget for GTK/GNOME
 * Copyright (C) 1998 the Free Software Foundation
 * Authors: Federico Mena (Quartic), Szekeres Istvan (Pista)
 */

#ifndef __GTK_GS_H__
#define __GTK_GS_H__

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <errno.h>
#include <signal.h>
#include "ps.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
        
#define GTK_GS(obj)         GTK_CHECK_CAST (obj, gtk_gs_get_type (), GtkGS)
#define GTK_GS_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_gs_get_type (), GtkGSClass)
#define GTK_IS_GS(obj)      GTK_CHECK_TYPE (obj, gtk_gs_get_type())

#define MAX_BUFSIZE 1024

#define GGV_WATCH_INTERVAL 1000
#define GGV_WATCH_TIMEOUT  2

typedef struct _GtkGS      GtkGS;
typedef struct _GtkGSClass GtkGSClass;

enum
{
        GTK_GS_ORIENTATION_NONE = -1,
        GTK_GS_ORIENTATION_PORTRAIT = 0,
        GTK_GS_ORIENTATION_SEASCAPE = 3,
        GTK_GS_ORIENTATION_UPSIDEDOWN = 2,
        GTK_GS_ORIENTATION_LANDSCAPE = 1,
};

struct _GtkGS
{
        GtkWidget widget;                    /* the main widget */
	GdkWindow *pstarget;                  /* the window passed to gv
                                              * it is a child of widget...
                                              */

	GdkPixmap *bpixmap;			/* Backing pixmap */
	int        use_bpixmap;

        long       message_window;  /* Used by ghostview to receive messages from app */
        
	GtkAdjustment* hadj;
	GtkAdjustment* vadj;

	int        disable_start;		/* Can the interpreter be started? */
	pid_t      interpreter_pid;		/* PID of interpreter, -1 if none  */
	int        interpreter_input;         /* stdin of interpreter            */
	int        interpreter_output;        /* stdout of interpreter           */
	int        interpreter_err;           /* stderr of interpreter           */
        guint      interpreter_input_id;
        guint      interpreter_output_id;
        guint      interpreter_error_id;

        gint        llx;
        gint        lly;
        gint        urx;
        gint        ury;
        gint        left_margin;
        gint        right_margin;
        gint        top_margin;
        gint        bottom_margin;
	gint        width;			/* Size of window at last setup()  */
	gint        height;
	gboolean    busy;			/* Is gs busy drawing? */
	gboolean    changed;			/* Anything changed since setup */
        gfloat      zoom_factor;
        gint        current_page;
        gboolean    structured_doc;
        gboolean    loaded;

        struct record_list *ps_input;
        gchar      *input_buffer_ptr;
        guint       bytes_left;
        guint       buffer_bytes_left;

        FILE       *gs_psfile;                /* the currently loaded FILE */
	gchar      *gs_filename;              /* the currently loaded filename */
	gchar      *gs_filename_dsc;          /* Used to browse PDF to PS */
	gchar      *gs_filename_unc;          /* Uncompressed file*/
        gchar      *input_buffer;
        gint        gs_scanstyle;
	time_t      mtime;
	gint        timer_tag;
        gboolean    send_filename_to_gs;     /* True if gs should read from file directly */
        gboolean    reading_from_pipe;        /* True if ggv is reading input from pipe */
        struct document *doc;

        /* User selected options... */
	gboolean    watch_doc;                /* Automatic reload if file changed */
        gboolean    antialiased;              /* Using antialiased display */
        gboolean    respect_eof;              /* respect EOF comments? */
        gint        default_page_media;
        gboolean    override_media;
        gfloat      xdpi, ydpi;
        gboolean    override_orientation;
        gint        fallback_orientation;  /* Orientation to use if override */
        gint        real_orientation;      /* Real orientation from the document */
        gint       *pages_marked;
};

struct _GtkGSClass
{
	GtkWidgetClass parent_class;
	GdkAtom        gs_atom;
	GdkAtom        gs_colors_atom;
	GdkAtom        next_atom;
	GdkAtom        page_atom;
	GdkAtom        done_atom;
	GdkAtom        string_atom;

        void (*interpreter_message)(GtkGS *, gchar *, gpointer);
};


/* structure to describe section of file to send to ghostscript */
struct record_list
{
        FILE *fp;
        long begin;
        guint len;
        gboolean seek_needed;
        gboolean close;
        struct record_list *next;
};

guint      gtk_gs_get_type (void);

GtkWidget *gtk_gs_new_from_file(GtkAdjustment *hadj, GtkAdjustment *vadj, gchar *fname);
GtkWidget *gtk_gs_new(GtkAdjustment *hadj, GtkAdjustment *vadj);
gboolean   gtk_gs_load (GtkGS *gs, const gchar *fname);

/* control functions */
void         gtk_gs_center_page(GtkGS *gs);
void         gtk_gs_scroll(GtkGS *gs, gint, gint);
void         gtk_gs_set_center(GtkGS *gs, gfloat x, gfloat y);
gboolean     gtk_gs_next_page(GtkGS *gs);
gboolean     gtk_gs_prev_page(GtkGS *gs);
gboolean     gtk_gs_goto_page(GtkGS *gs, gint);
gboolean     gtk_gs_set_pagemedia(GtkGS *gs, gint new_pagemedia, gint pageid);
gboolean     gtk_gs_set_orientation(GtkGS *gs, gint orientation);
void         gtk_gs_set_zoom(GtkGS *gs, gfloat zoom);
gint         gtk_gs_enable_interpreter(GtkGS *gs);
void         gtk_gs_disable_interpreter(GtkGS *gs);
gint         gtk_gs_get_orientation(GtkGS *gs);
const gchar *gtk_gs_document_title(GtkGS *widget);
guint        gtk_gs_document_numpages(GtkGS *widget);
const gchar *gtk_gs_document_page_label(GtkGS *widget, int page);
gint         gtk_gs_count_marked_pages(GtkGS *widget);

/* defaults accessors */
void     gtk_gs_set_default_page_media(gint iNewPageMedia); 
void     gtk_gs_set_default_override_media(gboolean bOverMedia);
void     gtk_gs_set_default_antialiased(gint iNewAntialiased);
void     gtk_gs_set_default_orientation(gint);
void     gtk_gs_set_default_override_orientation(gboolean bOverOrien);
void     gtk_gs_set_default_watch_doc(gint iNewWatchDoc);
gint     gtk_gs_get_default_page_media();
gboolean gtk_gs_get_default_override_media();
gboolean gtk_gs_get_default_antialiased();
gint     gtk_gs_get_default_orientation();
gboolean gtk_gs_get_default_override_orientation();
void     gtk_gs_set_default_zoom_factor(gfloat fZoom);
gfloat   gtk_gs_get_default_zoom_factor();
gint     gtk_gs_get_default_watch_doc() ;
void     gtk_gs_set_override_orientation (GtkGS *gs, gboolean bNewOverride);

#define GTK_GS_IS_COMPRESSED(gs)       (gs->gs_filename_unc != NULL)
#define GTK_GS_GET_PS_FILE(gs)         (GTK_GS_IS_COMPRESSED(gs) ? \
                                        gs->gs_filename_unc : \
                                        gs->gs_filename)
#define GTK_GS_IS_PDF(gs)              (gs->gs_filename_dsc != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_GS_H__ */
