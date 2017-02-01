/*  zvtterm.c - Zed's Virtual Terminal
 *  Copyright (C) 1998  Michael Zucchi
 *
 *  The zvtterm widget.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <config.h>

/* please use K&R style indenting if you make changes.
   2, 4, or 8 character tabs are fine - MPZ */

/* need for 'gcc -ansi -pedantic' under GNU/Linux */
#ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE 1
#endif
#include <sys/types.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkselection.h>
#include <gtk/gtkwindow.h>

#include "zvtterm.h"

#include <gdk/gdkx.h>
#include <gdk_imlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>

/* define to 'x' to enable copious debug output */
#define d(x)

/* default font */
#ifndef ZVT_MB
#define DEFAULT_FONT "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso8859-1"
#else
#define DEFAULT_FONT "-misc-fixed-medium-r-normal--14-*-*-*-*-*-*-1"
#endif

#define PADDING 2

/* forward declararations */
static void zvt_term_init (ZvtTerm *term);
static void zvt_term_class_init (ZvtTermClass *class);
static void zvt_term_init (ZvtTerm *term);
static void zvt_term_destroy (GtkObject *object);
static void zvt_term_realize (GtkWidget *widget);
static void zvt_term_unrealize (GtkWidget *widget);
static void zvt_term_map (GtkWidget *widget);
static void zvt_term_unmap (GtkWidget *widget);
static void zvt_term_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void zvt_term_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint zvt_term_expose (GtkWidget *widget, GdkEventExpose *event);
static void zvt_term_draw (GtkWidget *widget, GdkRectangle *area);
static gint zvt_term_button_press (GtkWidget *widget, GdkEventButton *event);
static gint zvt_term_button_release (GtkWidget *widget, GdkEventButton *event);
static gint zvt_term_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gint zvt_term_key_press (GtkWidget *widget, GdkEventKey *event);
static gint zvt_term_focus_in (GtkWidget *widget, GdkEventFocus *event);
static gint zvt_term_focus_out (GtkWidget *widget, GdkEventFocus *event);
static void zvt_term_selection_received (GtkWidget *widget, 
					 GtkSelectionData *selection_data, 
					 guint time);
static gint zvt_term_selection_clear (GtkWidget *widget, GdkEventSelection *event);
static void zvt_term_selection_get (GtkWidget *widget,
				    GtkSelectionData *selection_data_ptr, 
				    guint info, 
				    guint time);
static void zvt_term_child_died (ZvtTerm *term);
static void zvt_term_title_changed (ZvtTerm *term, VTTITLE_TYPE type, char *str);
static void zvt_term_title_changed_raise (void *user_data, VTTITLE_TYPE type, char *str);
static void zvt_term_dtterm_seq (void *term);
static void zvt_term_dtterm_seq_event (ZvtTerm *term);
static gint zvt_term_cursor_blink (gpointer data);
static void zvt_term_scrollbar_moved (GtkAdjustment *adj, GtkWidget *widget);
static void zvt_term_readdata (gpointer data, gint fd, GdkInputCondition condition);
static void zvt_term_readmsg (gpointer data, gint fd, GdkInputCondition condition);
static void zvt_term_fix_scrollbar (ZvtTerm *term);
static void vtx_unrender_selection (struct _vtx *vx);
static void zvt_term_scroll (ZvtTerm *term, int n);
static void zvt_term_scroll_by_lines (ZvtTerm *term, int n);
static int vt_cursor_state(void *user_data, int state);
static void zvt_term_writemore (gpointer data, gint fd, GdkInputCondition condition);
static void zvt_term_updated(ZvtTerm *term, int mode);
static void clone_col(unsigned short **dest, unsigned short *from);

/* callbacks from update layer */
void vt_draw_text(void *user_data, struct vt_line *line, int row, int col, int len, int attr);
void vt_scroll_area(void *user_data, int firstrow, int count, int offset, int fill);

/* transparent terminal prototypes */
static Window     get_desktop_window (Window the_window);
static Pixmap     get_pixmap_prop (Window the_window, char *prop_id);
static GdkPixmap *load_pixmap_back (char *file, int shaded);
static GdkPixmap *create_shaded_pixmap (Pixmap p, int x, int y, int w, int h);
static gint       safe_strcmp(gchar *a, gchar *b);

/* load the "current" background, from file or from system */
static void       load_background (ZvtTerm *term);

#ifdef ZVT_IM_ON_THE_SPOT
static void zvt_im_preedit_set_spot(ZvtTerm *term, int col, int row, int offx, int offy);
static void zvt_im_preedit_set_foreground(ZvtTerm *term, GdkColor *color);
static void zvt_im_preedit_set_background(ZvtTerm *term, GdkColor *color);
static void zvt_im_preedit_set_font(ZvtTerm *term, GdkFont *font);
#endif

/* static data */

/* The first 16 values are the ansi colors, the last
 * two are the default foreground and default background
 */
static gushort default_red[] = 
{0x0000,0xaaaa,0x0000,0xaaaa,0x0000,0xaaaa,0x0000,0xaaaa,
 0x5555,0xffff,0x5555,0xffff,0x5555,0xffff,0x5555,0xffff,
 0x0000, 0xffff};

static gushort default_grn[] = 
{0x0000,0x0000,0xaaaa,0x5555,0x0000,0x0000,0xaaaa,0xaaaa,
 0x5555,0x5555,0xffff,0xffff,0x5555,0x5555,0xffff,0xffff,
 0x0000, 0xffff};

static gushort default_blu[] = 
{0x0000,0x0000,0x0000,0x0000,0xaaaa,0xaaaa,0xaaaa,0xaaaa,
 0x5555,0x5555,0x5555,0x5555,0xffff,0xffff,0xffff,0xffff,
 0x0000, 0xffff};


/* GTK signals */
enum 
{
  CHILD_DIED,
  TITLE_CHANGED,
  DTTERM_SEQ,
  LAST_SIGNAL
};
static guint term_signals[LAST_SIGNAL] = { 0 };

/* values for selection info */
enum {
  TARGET_STRING,
  TARGET_UTF8,
  TARGET_TEXT,
  TARGET_COMPOUND_TEXT
};

/* GTK parent class */
static GtkWidgetClass *parent_class = NULL;


guint
zvt_term_get_type (void)
{
  static guint term_type = 0;
  
  if (!term_type) {
      GtkTypeInfo term_info = {
	"ZvtTerm",
	sizeof (ZvtTerm),
	sizeof (ZvtTermClass),
	(GtkClassInitFunc) zvt_term_class_init,
	(GtkObjectInitFunc) zvt_term_init,
	(GtkArgSetFunc) NULL,
	(GtkArgGetFunc) NULL,
      };
      
      term_type = gtk_type_unique (gtk_widget_get_type (), &term_info);
    }

  return term_type;
}


static void
zvt_term_class_init (ZvtTermClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  ZvtTermClass *term_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  term_class = (ZvtTermClass*) class;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  term_signals[CHILD_DIED] =
    gtk_signal_new ("child_died",
                    GTK_RUN_FIRST,
                    object_class->type,
                    GTK_SIGNAL_OFFSET (ZvtTermClass, child_died),
                    gtk_signal_default_marshaller,
                    GTK_TYPE_NONE, 0);

  term_signals[TITLE_CHANGED] =
    gtk_signal_new ("title_changed",
                    GTK_RUN_FIRST,
                    object_class->type,
                    GTK_SIGNAL_OFFSET (ZvtTermClass, child_died),
                    gtk_marshal_NONE__INT_POINTER,
                    GTK_TYPE_NONE, 2,
		    GTK_TYPE_INT,
		    GTK_TYPE_STRING);

  term_signals[DTTERM_SEQ] =
    gtk_signal_new ("dtterm_seq",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (ZvtTermClass, child_died),
		    gtk_signal_default_marshaller,
		    GTK_TYPE_NONE, 0);

  gtk_object_class_add_signals (object_class, term_signals, LAST_SIGNAL);

  object_class->destroy = zvt_term_destroy;

  widget_class->realize = zvt_term_realize;
  widget_class->unrealize = zvt_term_unrealize;
  widget_class->map = zvt_term_map;
  widget_class->unmap = zvt_term_unmap;
  widget_class->draw = zvt_term_draw;
  widget_class->expose_event = zvt_term_expose;
  widget_class->focus_in_event = zvt_term_focus_in;
  widget_class->focus_out_event = zvt_term_focus_out;
  widget_class->size_request = zvt_term_size_request;
  widget_class->size_allocate = zvt_term_size_allocate;
  widget_class->key_press_event = zvt_term_key_press;
  widget_class->button_press_event = zvt_term_button_press;
  widget_class->button_release_event = zvt_term_button_release;
  widget_class->motion_notify_event = zvt_term_motion_notify;

  widget_class->selection_clear_event = zvt_term_selection_clear;
  widget_class->selection_received = zvt_term_selection_received;
  widget_class->selection_get = zvt_term_selection_get;

  term_class->child_died = zvt_term_child_died;
  term_class->title_changed = zvt_term_title_changed;
}


static void
zvt_term_init (ZvtTerm *term)
{
  struct _zvtprivate *zp;
  static const GtkTargetEntry targets[] = {
    { "STRING", 0, TARGET_STRING },
#ifdef ZVT_UTF
    { "UTF-8",  0, TARGET_UTF8 }, 
#endif
#ifdef ZVT_MB
    { "TEXT",   0, TARGET_TEXT }, 
    { "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT }
#endif
  };
  static const gint n_targets = sizeof(targets) / sizeof(targets[0]);

  GTK_WIDGET_SET_FLAGS (term, GTK_CAN_FOCUS);

  /* create and configure callbacks for the teminal back-end */
  term->vx = vtx_new (80, 24, term);
  term->vx->vt.ring_my_bell = zvt_term_bell;
  term->vx->vt.change_my_name = zvt_term_title_changed_raise;
  term->vx->vt.dtterm_seq     = zvt_term_dtterm_seq;

  /* set rendering callbacks */
  term->vx->draw_text = vt_draw_text;
  term->vx->scroll_area = vt_scroll_area;
  term->vx->cursor_state = vt_cursor_state;

  term->shadow_type = GTK_SHADOW_NONE;
  term->term_window = NULL;
  term->cursor_bar = NULL;
  term->cursor_dot = NULL;
  term->cursor_current = NULL;
  term->font = NULL;
  term->font_bold = NULL;
  term->scroll_gc = NULL;
  term->fore_gc = NULL;
  term->back_gc = NULL;
  term->fore_last = 0;
  term->back_last = 0;
  term->color_ctx = 0;
  term->ic = NULL;

  /* background pixmap */
  term->pixmap_filename = NULL;
  term->background.pix = NULL;
  term->background.x = 0;
  term->background.y = 0;
  term->background.w = 0;
  term->background.h = 0;

  /* grid sizing */
  term->grid_width = term->vx->vt.width;
  term->grid_height = term->vx->vt.height;

  /* input handlers */
  term->input_id = -1;
  term->msg_id = -1;
  term->timeout_id = -1;

  /* bitfield flags */
  term->cursor_on = 0;
  term->cursor_filled = 0;
  term->cursor_blink_state = 0;
  term->scroll_on_keystroke = 0;
  term->scroll_on_output = 0;
  term->blink_enabled = 1;
  term->ic = NULL;
  term->in_expose = 0;
  term->transparent = 0;
  term->shaded = 0;

  /* private data - set before calling functions */
  zp = g_malloc0(sizeof(*zp));
  if (zp) {
    zp->scrollselect_id = -1;
    zp->text_expand = 0;
    zp->text_expandlen = 0;
    zp->scroll_position = 0;
    zp->fonttype=0;
    zp->default_char=0;
    zp->bold_save = 0;
    zp->paste_id = -1;
    zp->paste = 0;
    zp->queue_red= 0;
    zp->queue_green = 0;
    zp->queue_blue = 0;
  }
  gtk_object_set_data(GTK_OBJECT (term), "_zvtprivate", zp);

  /* charwidth, charheight set here */
  zvt_term_set_font_name (term, DEFAULT_FONT);

  /* scrollback position adjustment */
  term->adjustment =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 1.0, 1.0, 1.0, 1.0));
  gtk_object_ref (GTK_OBJECT (term->adjustment));
  gtk_object_sink (GTK_OBJECT (term->adjustment));

  gtk_signal_connect (
      GTK_OBJECT (term->adjustment),
      "value_changed",
      GTK_SIGNAL_FUNC (zvt_term_scrollbar_moved),
      term);

  gtk_signal_connect (
      GTK_OBJECT (term),
      "dtterm_seq",
      GTK_SIGNAL_FUNC (zvt_term_dtterm_seq_event),
      NULL);

  /* selection received */
  gtk_selection_add_targets (GTK_WIDGET (term),
			     GDK_SELECTION_PRIMARY,
			     targets, n_targets);
}

/**
 * zvt_term_set_blink:
 * @term: A &ZvtTerm widget.
 * @state: The blinking state.  If %TRUE, the cursor will blink.
 *
 * Use this to control the way the cursor is displayed (blinking/solid)
 */
void
zvt_term_set_blink (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);                     
  g_return_if_fail (ZVT_IS_TERM (term));

  if (!(term->blink_enabled ^ (state?1:0)))
    return;

  if (term->blink_enabled) {
    if (term->timeout_id != -1){
      gtk_timeout_remove (term->timeout_id);
      term->timeout_id = -1;
    }
    
    if (GTK_WIDGET_REALIZED (term))
      vt_cursor_state (GTK_WIDGET (term), 1);
    
    term->blink_enabled = 0;
  } else {
    term->timeout_id = gtk_timeout_add (500, zvt_term_cursor_blink, term);
    term->blink_enabled = 1;
  }
}

/**
 * zvt_term_set_scroll_on_keystroke:
 * @term: A &ZvtTerm widget.
 * @state: Desired state.
 * 
 * If @state is %TRUE, forces the terminal to jump out of the
 * scrollback buffer whenever a keypress is received.
 **/
void 
zvt_term_set_scroll_on_keystroke(ZvtTerm *term, int state)
{
  term->scroll_on_keystroke = (state != 0);
}

/**
 * zvt_term_set_scroll_on_output:
 * @term: A &ZvtTerm widget.
 * @state: Desired state.
 *
 * If @state is %TRUE, forces the terminal to scroll on output
 * being generated by a child process or by zvt_term_feed().
 */
void 
zvt_term_set_scroll_on_output   (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);                     
  g_return_if_fail (ZVT_IS_TERM (term));

  term->scroll_on_output = (state != 0);
}

/**
 * zvt_term_set_wordclass:
 * @term: A &ZvtTerm widget.
 * @class: A string of characters to consider a "word" character.
 *
 * Sets the list of characters (character class) that are considered
 * part of a word, when selecting by word.  The @class is defined
 * the same was as a regular expression character class (as normally
 * defined using []'s, but without those included).  A leading or trailing
 * hypen (-) is used to include a hyphen in the character class.
 *
 * Passing a %NULL @class restores the default behaviour of alphanumerics
 * plus "_"  (i.e. "A-Za-z0-9_").
 */
void zvt_term_set_wordclass(ZvtTerm *term, unsigned char *class)
{
  g_return_if_fail (term != NULL);                     
  g_return_if_fail (ZVT_IS_TERM (term));

  vt_set_wordclass(term->vx, class);
}

static void
term_force_size(ZvtTerm *term)
{
  if (GTK_WIDGET_REALIZED (term)) {

    /* ok, if we update the window hints ourselves, then we
       use that to implicitly resize the window, otherwise
       we do it explicitly */
    /* in stable, do not implement auto_hint, but after merge, do */
    gtk_widget_queue_resize (GTK_WIDGET (term));

    d(printf("forcing term size to %d, %d\n", term->grid_width, term->grid_height));
    d(printf("                   = %d, %d\n",
	     (term->grid_width * term->charwidth) + 
	     (GTK_WIDGET(term)->style->klass->xthickness * 2) + PADDING,
	     (term->grid_height * term->charheight) + 
	     (GTK_WIDGET(term)->style->klass->ythickness * 2)));
  }
}

/**
 * zvt_term_new_with_size:
 * @cols: Number of columns required.
 * @rows: Number of rows required.
 *
 * Creates a new ZVT Terminal widget of the given character dimentions.
 * If the encompassing widget is resizable, then this size may change
 * afterwards, but should be correct at realisation time.
 *
 * Return Value: A pointer to a &ZvtTerm widget is returned, or %NULL
 * on error.
 */
GtkWidget*
zvt_term_new_with_size (int cols, int rows)
{
  ZvtTerm *term;
  term = gtk_type_new (zvt_term_get_type ());

  /* fudge the pixel size, not (really) used anyway */
  vt_resize (&term->vx->vt, cols, rows, cols*8, rows*8);

  term->grid_width = cols;
  term->grid_height = rows;

  return GTK_WIDGET (term);
}


/**
 * zvt_term_new:
 *
 * Creates a new ZVT Terminal widget.  By default the terminal will be
 * setup as 80 colmns x 24 rows, but it will size automatically to its
 * encompassing widget, and may be smaller or larger upon realisation.
 *
 * Return Value: A pointer to a &ZvtTerm widget is returned, or %NULL
 * on error.
 */
GtkWidget*
zvt_term_new (void)
{
  ZvtTerm *term;
  term = gtk_type_new (zvt_term_get_type ());
  return GTK_WIDGET (term);
}


static void
zvt_term_destroy (GtkObject *object)
{
  ZvtTerm *term;
  struct _zvtprivate *zp;

  g_return_if_fail (object != NULL);
  g_return_if_fail (ZVT_IS_TERM (object));

  term = ZVT_TERM (object);
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  if (term->timeout_id != -1)
    gtk_timeout_remove(term->timeout_id);

  zvt_term_closepty (term);
  vtx_destroy (term->vx);

  if (term->font) {
    gdk_font_unref (term->font);
    term->font = NULL;
  }

  if (term->font_bold) {
    gdk_font_unref (term->font_bold);
    term->font_bold = NULL;
  }

  /* release the adjustment */
  if (term->adjustment) {
    gtk_signal_disconnect_by_data (GTK_OBJECT (term->adjustment), term);
    gtk_object_unref (GTK_OBJECT (term->adjustment));
    term->adjustment = NULL;
  }

  if (term->ic) {
    gdk_ic_destroy(term->ic);
    term->ic = NULL;
  }

  /* release private data */
  if (zp) {
    if (zp->text_expand)
      g_free(zp->text_expand);
    if (zp->bold_save)
      gdk_pixmap_unref(zp->bold_save);
    if (zp->paste)
      g_free(zp->paste);
    if (zp->paste_id != -1)
      gdk_input_remove(zp->paste_id);

    if (zp->queue_red)
	    g_free(zp->queue_red);
    if (zp->queue_green)
	    g_free(zp->queue_green);
    if (zp->queue_blue)
	    g_free(zp->queue_blue);
    
    g_free(zp);
    gtk_object_set_data (GTK_OBJECT (term), "_zvtprivate", 0);
  }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/**
 * zvt_term_reset:
 * @term: A &ZvtTerm widget.
 * @hard: If %TRUE, then perform a HARD reset.
 * 
 * Performs a complete reset on the terminal.  Resets all
 * attributes, and if @hard is %TRUE, also clears the screen.
 **/
void
zvt_term_reset (ZvtTerm *term, int hard)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  vt_cursor_state (term, 0);
  vt_reset_terminal(&term->vx->vt, hard);
  vt_update (term->vx, UPDATE_CHANGES);
  vt_cursor_state (term, 1);
  zvt_term_updated(term, 2);
}

static void
clone_col(unsigned short **dest, unsigned short *from)
{
  if (*dest)
    g_free(*dest);
  if (from) {
    *dest = g_malloc(18 * sizeof(unsigned short));
    memcpy(*dest, from, 18 * sizeof(unsigned short));
  } else {
    *dest = 0;
  }
}

/**
 * zvt_term_set_color_scheme:
 * @term: A &ZvtTerm widget.
 * @red:  pointer to a gushort array of 18 elements with red values.
 * @grn:  pointer to a gushort array of 18 elements with green values.
 * @blu:  pointer to a gushort array of 18 elements with blue values.
 *
 * This function sets the colour palette for the terminal @term.  Each
 * pointer points to a gushort array of 18 elements.  White is 0xffff
 * in all elements.
 *
 * The elements 0 trough 15 are the first 16 colours for the terminal,
 * with element 16 and 17 the default foreground and background colour
 * respectively.
 */
void
zvt_term_set_color_scheme (ZvtTerm *term, gushort *red, gushort *grn, gushort *blu)
{
  int  nallocated;
  GdkColor c;
  struct _zvtprivate *zp;
  
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (red != NULL);
  g_return_if_fail (grn != NULL);
  g_return_if_fail (blu != NULL);

  zp = _ZVT_PRIVATE(term);

  if (term->color_ctx == NULL) {
    clone_col(&zp->queue_red, red);
    clone_col(&zp->queue_green, grn);
    clone_col(&zp->queue_blue, blu);
    return;
  }

  memset (term->colors, 0, sizeof (term->colors));
  nallocated = 0;
  gdk_color_context_get_pixels (term->color_ctx, red, grn, blu, 18, 
				term->colors, &nallocated);
  c.pixel = term->colors [17];
#if 1
  gdk_window_set_background (GTK_WIDGET (term)->window, &c);
  gdk_window_clear (GTK_WIDGET (term)->window);
#endif

  /* always clear up any old queued values */
  clone_col(&zp->queue_red, 0);
  clone_col(&zp->queue_green, 0);
  clone_col(&zp->queue_blue, 0);
#ifdef ZVT_IM_ON_THE_SPOT
  zvt_im_preedit_set_background(term, &c);
  c.pixel = term->colors [16];
  zvt_im_preedit_set_foreground(term, &c);
#endif /* ZVT_IM_ON_THE_SPOT */
}

/**
 * zvt_term_set_default_color_scheme:
 * @term: A &ZvtTerm widget.
 *
 * Resets the color values to the default color scheme.
 */
void
zvt_term_set_default_color_scheme (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  
  zvt_term_set_color_scheme (term, default_red, default_grn, default_blu);
}

/**
 * zvt_term_set_size:
 * @term: A &ZvtTerm widget.
 * @width: Width of terminal, in columns.
 * @height: Height of terminal, in rows.
 * 
 * Causes the terminal to attempt to resize to the absolute character
 * size of @width rows by @height columns.
 **/
void
zvt_term_set_size (ZvtTerm *term, guint width, guint height)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  term->grid_width = width;
  term->grid_height = height;
  term_force_size(term);
}

static void
zvt_term_realize (GtkWidget *widget)
{
  ZvtTerm *term;
  GdkWindowAttr attributes;
  GdkPixmap *cursor_dot_pm;
  gint attributes_mask;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  term = ZVT_TERM (widget);
  zp = _ZVT_PRIVATE(term);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width - (2 * widget->style->klass->xthickness) - PADDING;
  attributes.height = widget->allocation.height - (2 * widget->style->klass->ythickness);
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) |
    GDK_EXPOSURE_MASK | 
    GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_MOTION_MASK |
    GDK_POINTER_MOTION_MASK | 
    GDK_BUTTON_RELEASE_MASK | 
    GDK_KEY_PRESS_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  /* main window */
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  widget->style = gtk_style_attach (widget->style, widget->window);
  gdk_window_set_user_data (widget->window, widget);
#if 1
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
#else
  gdk_window_set_back_pixmap(widget->window, 0, 1);
#endif

  /* this should never have been created *sigh* */
  /* keep for compatability */
  term->term_window = widget->window;

  /* create pixmaps for this window */
  cursor_dot_pm = 
    gdk_pixmap_create_from_data(widget->window,
				"\0", 1, 1, 1,
				&widget->style->fg[GTK_STATE_ACTIVE],
				&widget->style->bg[GTK_STATE_ACTIVE]);

  /* Get I beam cursor, and also create a blank one based on the blank image */
  term->cursor_bar = gdk_cursor_new(GDK_XTERM);
  term->cursor_dot = 
    gdk_cursor_new_from_pixmap(cursor_dot_pm, cursor_dot_pm,
			       &widget->style->fg[GTK_STATE_ACTIVE],
			       &widget->style->bg[GTK_STATE_ACTIVE],
			       0, 0);
  gdk_window_set_cursor(widget->window, term->cursor_bar);
  gdk_pixmap_unref (cursor_dot_pm);
  zp->cursor_hand = gdk_cursor_new(GDK_HAND2);
  term->cursor_current = term->cursor_bar;

  /* setup scrolling gc */
  term->scroll_gc = gdk_gc_new (widget->window);
  gdk_gc_set_exposures (term->scroll_gc, TRUE);

  /* Colors */
  term->fore_gc = gdk_gc_new (widget->window);
  term->back_gc = gdk_gc_new (widget->window);
  term->color_ctx = 
      gdk_color_context_new (gtk_widget_get_visual (GTK_WIDGET (term)),
			     gtk_widget_get_colormap (GTK_WIDGET (term)));

  /* Allocate default or requested colour set */
  if (zp->queue_red != NULL && zp->queue_green != NULL && zp->queue_blue != NULL) {
    zvt_term_set_color_scheme(term, zp->queue_red, zp->queue_green, zp->queue_blue);
  } else {
    zvt_term_set_default_color_scheme (term);
  }
  
  /* set the initial colours */
  term->back_last = -1;
  term->fore_last = -1;

  /* and the initial size ... */
  term_force_size(term);

  /* input context */
#ifdef ZVT_IM_ON_THE_SPOT
  zvt_term_set_open_im (term, True);
#else
  if (gdk_im_ready () && !term->ic) {
    GdkICAttr attr;
    
    /* FIXME: do we have any window yet? */
    attr.style = GDK_IM_PREEDIT_NOTHING | GDK_IM_STATUS_NOTHING;
    attr.client_window = attr.focus_window = widget->window;
    term->ic = gdk_ic_new(&attr, GDK_IC_ALL_REQ);
    
    if (!term->ic) {
      g_warning("Can't create input context.");
    }
  }
#endif
}

static void
zvt_term_unrealize (GtkWidget *widget)
{
  ZvtTerm *term;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  /* free resources */
  gdk_cursor_destroy (term->cursor_bar);
  term->cursor_bar = NULL;

  gdk_cursor_destroy (term->cursor_dot);
  term->cursor_dot = NULL;

  gdk_cursor_destroy (zp->cursor_hand);
  zp->cursor_hand = NULL;

  term->cursor_current = NULL;

  gdk_color_context_free (term->color_ctx);
  term->color_ctx = NULL;

  gdk_gc_destroy (term->scroll_gc);
  term->scroll_gc = NULL;

  gdk_gc_destroy (term->back_gc);
  term->back_gc = NULL;

  gdk_gc_destroy (term->fore_gc);
  term->fore_gc = NULL;

  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (*GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}


static void
zvt_term_map (GtkWidget *widget)
{
  ZvtTerm *term;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);

  if (!GTK_WIDGET_MAPPED (widget)) {
    GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
    
    gdk_window_show (widget->window);

    /* XXX: is this right? --JMP */
    if (!GTK_WIDGET_HAS_FOCUS (widget))
      gtk_widget_grab_focus (widget);
  }
}


static void
zvt_term_unmap (GtkWidget *widget)
{
  ZvtTerm *term;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);

  if (GTK_WIDGET_MAPPED (widget)) {
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);
    
    gdk_window_hide (widget->window);
  }
}


static gint
zvt_term_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
  ZvtTerm *term;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

  vt_cursor_state(term, 0);
  term->cursor_filled = 1;
  vt_cursor_state(term, 1);

  /* setup blinking cursor */
  if (term->blink_enabled && term->timeout_id == -1)
    term->timeout_id = gtk_timeout_add (500, zvt_term_cursor_blink, term);

  if (term->ic)
    gdk_im_begin (term->ic, widget->window);

  return FALSE;
}


static gint
zvt_term_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
  ZvtTerm *term;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

  vt_cursor_state(term, 0);
  term->cursor_filled = 0;
  vt_cursor_state(term, 1);

  /* setup blinking cursor */
  if (term->blink_enabled && term->timeout_id != -1) {
    gtk_timeout_remove(term->timeout_id);
    term->timeout_id = -1;
  }
  
  if (term->ic)
    gdk_im_end ();

  return FALSE;
}

/*
 Ok, this is a bit weird, we basically keep returning
 the initial size, until we no longer need it (this is
 during realization), then we just return 1x1 always,
 so that the window can grow or shrink so that set_usize
 always works.
*/
static void 
zvt_term_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  ZvtTerm *term;
  guint grid_width, grid_height;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (requisition != NULL);

  term = ZVT_TERM (widget);

  grid_width = term->grid_width;
  grid_height = term->grid_height;
  
  if (grid_width <= 0)
    grid_width = 8;
  
  if (grid_height <= 0)
    grid_height = 2;

  requisition->width = (grid_width * term->charwidth) + 
    (widget->style->klass->xthickness * 2) + PADDING;
  requisition->height = (grid_height * term->charheight) + 
    (widget->style->klass->ythickness * 2);

  /* debug ouput */
  d( printf("zvt_term_size_request x=%d y=%d\n", grid_width, grid_height) );
  d( printf("   requestion size w=%d, h=%d\n", requisition->width, requisition->height) );
}

static void
zvt_term_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  gint term_width, term_height;
  guint grid_width, grid_height;
  ZvtTerm *term;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;

  if (GTK_WIDGET_REALIZED (widget)) {
      term = ZVT_TERM (widget);
      zp = _ZVT_PRIVATE(term);

      d( printf("zvt_term_size_allocate old grid x=%d y=%d\n", 
		term->vx->vt.width,
		term->vx->vt.height) );
      
      d(printf("old size w=%d h=%d\n", widget->allocation.width, widget->allocation.height));
      d(printf("new allocation w=%d h=%d\n", allocation->width, allocation->height));

      gdk_window_move_resize (widget->window,
			      allocation->x,
			      allocation->y,
			      allocation->width,
			      allocation->height);

      term_width = allocation->width - (2 * widget->style->klass->xthickness) - PADDING;
      term_height = allocation->height - (2 * widget->style->klass->ythickness);

      /* resize the virtual terminal buffer, if its size has changed, minimal size is 1x1 */
      grid_width = MAX(term_width / term->charwidth, 1);
      grid_height = MAX(term_height / term->charheight, 1);
      if (grid_width != term->charwidth
	  || grid_height != term->charheight) {

	/* turn off the selection */
	term->vx->selstartx = term->vx->selendx;
	term->vx->selstarty = term->vx->selendy;
	term->vx->selected = 0;

	vt_resize (&term->vx->vt, grid_width, grid_height, term_width, term_height);
	vt_update (term->vx, UPDATE_REFRESH|UPDATE_SCROLLBACK);
	
	d(printf("zvt_term_size_allocate grid calc x=%d y=%d\n", 
		 grid_width,
		 grid_height) );
      }

      /* resize the scrollbar */
      zvt_term_fix_scrollbar (term);
      zvt_term_updated(term, 2);

      d( printf("zvt_term_size_allocate new grid x=%d y=%d\n", 
		term->vx->vt.width,
		term->vx->vt.height) );
  }
}


static void 
zvt_term_draw (GtkWidget *widget, GdkRectangle *area)
{
  gint width, height;
  ZvtTerm *term;
  int fill;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (area != NULL);

  term = ZVT_TERM (widget);

  d( printf("zvt_term_draw xx=%d y=%d, w=%d, h=%d\n", 
	   area->x, area->y, area->width, area->height));

  if (GTK_WIDGET_DRAWABLE (widget)) {
    d( printf("zvt_term_draw is drawable\n") );

    /* make sure we've got the right background, incase some stupid style changed it */
    if (term->transparent==0 && term->pixmap_filename==0) {
      GdkColor c;
      c.pixel = term->colors [17];
      gdk_window_set_background (GTK_WIDGET (term)->window, &c);
    }
    
    term->in_expose = 1;
    
    gdk_window_get_size (widget->window, &width, &height);

    /* always load the background */
    load_background(term);

    if(term->transparent || term->pixmap_filename) {
      gdk_draw_rectangle (widget->window,
			  term->back_gc, 1,
			  0, 0,
			  widget->allocation.width,
			  widget->allocation.height);
    }
    fill=17;

    /* assume the screen is filled with background? */
    vt_update_rect (term->vx, fill, 0, 0,
		    width / term->charwidth,
		    height / term->charheight);
    
    term->in_expose = 0;

    /* draw shadow/border */
    if (term->shadow_type != GTK_SHADOW_NONE)
      gtk_draw_shadow (widget->style, widget->window,
		       GTK_STATE_NORMAL, term->shadow_type, 0, 0, 
		       widget->allocation.width,
		       widget->allocation.height);
  }	  
}


static gint
zvt_term_expose (GtkWidget      *widget,
		 GdkEventExpose *event)
{
  ZvtTerm *term;
  int offx, offy;
  int fill;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  offx = widget->style->klass->xthickness + PADDING;
  offy = widget->style->klass->ythickness;

  d(printf("exposed!  (%d,%d) %dx%d\n",
	  event->area.x, 
	  event->area.y,
	  event->area.width, 
	  event->area.height));

  if (GTK_WIDGET_DRAWABLE (widget)) {
    term = ZVT_TERM (widget);
    
    /* FIXME: may update 1 more line/char than needed */
    term->in_expose = 1;

    /*
      this could probably do what draw does, but it looks a bit slower if you do.
     */
    if(term->transparent || term->pixmap_filename) {
      gdk_draw_rectangle (widget->window,
			  term->back_gc, 1,
			  event->area.x, 
			  event->area.y,
			  event->area.width, 
			  event->area.height);
    }
    fill = 17;

    vt_update_rect (term->vx, fill,
		    (event->area.x-offx) / term->charwidth,
		    (event->area.y-offy) / term->charheight,
		    (event->area.x + event->area.width) / term->charwidth+1,
		    (event->area.y + event->area.height) / term->charheight+1);

    term->in_expose = 0;

    if (term->shadow_type != GTK_SHADOW_NONE)
      gtk_draw_shadow (widget->style, widget->window,
		       GTK_STATE_NORMAL, term->shadow_type, 0, 0, 
		       widget->allocation.width,
		       widget->allocation.height); 
#if 0
    /* DEBUG to know what was exposed */
    {
      GdkColor pen;
      pen.pixel = term->colors [(term->fore_last + 1)&7];
      gdk_gc_set_foreground (term->fore_gc, &pen);
      term->fore_last = (term->fore_last + 1)&7;
      gdk_draw_rectangle (widget->window,
			  term->fore_gc, 0,
			  event->area.x, 
			  event->area.y,
			  event->area.width, 
			  event->area.height);
    }
#endif
  }

  return FALSE;
}


/**
 * zvt_term_show_pointer:
 * @term: A &ZvtTerm widget.
 *
 * Show the default I beam pointer.
 */
void
zvt_term_show_pointer (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);

  if (term->cursor_current == term->cursor_dot) {
    gdk_window_set_cursor(GTK_WIDGET(term)->window, term->cursor_bar);
    term->cursor_current = term->cursor_bar;
  }
}

static void zvt_term_set_pointer(ZvtTerm *term, GdkCursor *c)
{
  if (term->cursor_current != c) {
    gdk_window_set_cursor(GTK_WIDGET(term)->window, c);
    term->cursor_current = c;
  }
}

/**
 * zvt_term_show_pointer:
 * @term: A &ZvtTerm widget.
 *
 * Hide the pointer.  In reality the pointer is changed to a
 * single-pixel black dot.
 */
void
zvt_term_hide_pointer (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);

  if (term->cursor_current != term->cursor_dot) {
    gdk_window_set_cursor(GTK_WIDGET(term)->window, term->cursor_dot);
    term->cursor_current = term->cursor_dot;
  }
}

/**
 * zvt_term_set_scrollback:
 * @term: A &ZvtTerm widget.
 * @lines: Number of lines desired for the scrollback buffer.
 *
 * Set the maximum number of scrollback lines for the widget @term to
 * @lines lines.
 */
void
zvt_term_set_scrollback (ZvtTerm *term, int lines)
{
  g_return_if_fail (term != NULL);

  vt_scrollback_set (&term->vx->vt, lines);
  zvt_term_fix_scrollbar (term);
}


/* Load a set of fonts into the terminal.
 * These fonts should be the same size, otherwise it could get messy ...
i to * if font_bold is NULL, then the font is emboldened manually (overstrike)
 */
static void
zvt_term_set_fonts_internal(ZvtTerm *term, GdkFont *font, GdkFont *font_bold)
{
  struct _zvtprivate *zp;

  /* ignore no-font setting */
  if (font==NULL)
    return;

  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  /* get the font/fontset size, and set the font type */
  switch (font->type) {
  case GDK_FONT_FONT: {
    XFontStruct *xfont;
    xfont = (XFontStruct *)GDK_FONT_XFONT(font);
    term->charwidth = xfont->max_bounds.width;
    term->charheight = font->ascent + font->descent;
    if ((xfont->min_byte1 == 0) && (xfont->max_byte1 == 0))
      zp->fonttype = ZVT_FONT_1BYTE;
    else
      zp->fonttype = ZVT_FONT_2BYTE;
    break;
  }
  case GDK_FONT_FONTSET: {
    XFontSet fontset = (XFontSet) ((GdkFontPrivate *)font)->xfont;
    XFontSetExtents *extents = XExtentsOfFontSet(fontset);
#ifdef ZVT_MB /* This is look bug..., isn't it? */
    term->charwidth = gdk_string_width (font, "M");
#else
    term->charwidth = extents->max_logical_extent.width;
#endif
    term->charheight = extents->max_logical_extent.height;
    zp->fonttype = ZVT_FONT_FONTSET;
  }
  }
  d(printf("fonttype = %d\n", zp->fonttype));

  /* set the desired size, and force a resize */
  term->grid_width = term->vx->vt.width;
  term->grid_height = term->vx->vt.height;
  term_force_size(term);

  if (term->font)
    gdk_font_unref (term->font);
  term->font = font;
#ifdef ZVT_IM_ON_THE_SPOT
  zvt_im_preedit_set_font(term, font);
#endif

  if (term->font_bold)
    gdk_font_unref (term->font_bold);
  term->font_bold = font_bold;

  /* setup bold_save pixmap, for when drawing bold, we need to save
     what was at the end of the text, so we can restore it.  Affects
     tiny fonts only - what a mess */
  if (zp && term->font_bold==0) {
    int depth;

    if (zp->bold_save)
      gdk_pixmap_unref(zp->bold_save);
    gdk_window_get_geometry(GTK_WIDGET(term)->window,NULL,NULL,NULL,NULL,&depth);

    zp->bold_save = gdk_pixmap_new(GTK_WIDGET(term)->window,
				   1, term->charheight, depth);
  }

  /*gtk_widget_queue_resize (GTK_WIDGET (term));*/
}

/**
 * zvt_term_set_fonts:
 * @term: A &ZvtTerm widget.
 * @font: Font used for regular text.
 * @font_bold: Font used for bold text.  May be null, in which case the bold
 * font is rendered by over-striking.
 *
 * Load a set of fonts into the terminal.
 * 
 * These fonts should be the same size, otherwise it could get messy ...
 */
void
zvt_term_set_fonts (ZvtTerm *term, GdkFont *font, GdkFont *font_bold)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (font != NULL);

  zvt_term_set_fonts_internal (term, font, font_bold);

  gdk_font_ref (font);
  if (font_bold)
      gdk_font_ref (font_bold);
}

/**
 * zvt_term_set_font_name:
 * @term: A &ZvtTerm widget.
 * @name: A full X11 font name string.
 *
 * Set a font by name only.  If font aliases such as 'fixed' or
 * '10x20' are passed to this function, then both the bold and
 * non-bold font will be identical.  In colour mode bold fonts are
 * always the top 8 colour scheme entries, and so bold is still
 * rendered.
 *
 * Tries to calculate bold font name from the base name.  This only
 * works with fonts where the names are alike.
 */
void
zvt_term_set_font_name (ZvtTerm *term, char *name)
{
  int count;
  char c, *ptr, *rest;
  GString *newname, *outname;
  GdkFont *font, *font_bold;

  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (name != NULL);

  newname = g_string_new (name);
  outname = g_string_new ("");

  rest = 0;
  ptr = newname->str;

  for (count = 0; (c = *ptr++);) {
    if (c == '-') {
      count++;
      
      d(printf("scanning (%c) (%d)\n", c, count));
      d(printf("newname = %s ptr = %s\n", newname->str, ptr));
      
      switch (count) {
      case 3:
	ptr[-1] = 0;
	break;
	
      case 5:
	rest = ptr - 1;
	break;
      }
    }
  }

  if (rest) {
    g_string_sprintf (outname, "%s-medium-r%s", newname->str, rest);
#ifndef ZVT_MB
    font = gdk_font_load (outname->str);
#else
    font = gdk_fontset_load (outname->str);
#endif
    d( printf("loading normal font %s\n", outname->str) );
    
    g_string_sprintf (outname, "%s-bold-r%s", newname->str, rest); 
#ifndef ZVT_MB
    font_bold = gdk_font_load (outname->str);
#else
    font_bold = gdk_fontset_load (outname->str);
#endif
    d( printf("loading bold font %s\n", outname->str) );
    
    zvt_term_set_fonts_internal (term, font, font_bold);
  } else {
#ifndef ZVT_MB
    font = gdk_font_load (name);
#else
    font = gdk_fontset_load (name);
#endif
    zvt_term_set_fonts_internal (term, font, 0);
  }

  g_string_free (newname, TRUE);
  g_string_free (outname, TRUE);
}


/*
  Called when something has changed, size of window or scrollback.

  Fixes the adjustment and notifies the system.
*/
static void 
zvt_term_fix_scrollbar (ZvtTerm *term)
{
  GTK_ADJUSTMENT(term->adjustment)->upper = 
    term->vx->vt.scrollbacklines + term->vx->vt.height - 1;

  GTK_ADJUSTMENT(term->adjustment)->value = 
    term->vx->vt.scrollbackoffset + term->vx->vt.scrollbacklines;

  GTK_ADJUSTMENT(term->adjustment)->page_increment = 
    term->vx->vt.height - 1;

  GTK_ADJUSTMENT(term->adjustment)->page_size =
    term->vx->vt.height - 1;

  gtk_signal_emit_by_name (GTK_OBJECT (term->adjustment), "changed");
}

/* keep calling with incrementing 'type' until you get a valid return,
   or nothing
   returns true if a paste was actually requested. */
static int
request_paste (GtkWidget *widget, int type, gint32 time)
{
  GdkAtom string_atom;
#ifdef ZVT_UTF
#ifdef ZVT_MB
  char *types[] = {"UTF-8", "COMPOUND_TEXT"};
#else
  char *types[] = {"UTF-8", "STRING"};
#endif /* ZVT_MB */
  int index;
  struct _zvtprivate *zp = _ZVT_PRIVATE(widget);

  if ( ((ZvtTerm *)widget)->vx->vt.coding != ZVT_CODE_UTF8)
    index = type+1;
  else
    index = type;

  if (index>=sizeof(types)/sizeof(types[0])) {
    zp->lastselectiontype = -1;
    return 0;
  }

  string_atom = gdk_atom_intern (types[index], FALSE);
  zp->lastselectiontype = type;

  d(printf(" %s atom = %d\n", types[index], (int)string_atom));
#else
  /* Get the atom corresonding to the target "STRING" */
#ifdef ZVT_MB
  string_atom = gdk_atom_intern ("COMPOUND_TEXT", FALSE);
#else
  string_atom = gdk_atom_intern ("STRING", FALSE);
#endif /* ZVT_MB */
#endif

  if (string_atom == GDK_NONE) {
    g_warning("WARNING: Could not get string atom\n");
  }

  /* And request the "STRING" target for the primary selection */
    gtk_selection_convert (widget, GDK_SELECTION_PRIMARY, string_atom,
			   time);
  return 1;
}

/*
  perhaps most of the button press stuff could be shifted
  to the update file.  as for the report_button function
  shifted to the vt file ?

*/

static gint
zvt_term_button_press (GtkWidget      *widget,
		       GdkEventButton *event)
{
  gint x,y;
  GdkModifierType mask;
  struct _vtx *vx;
  ZvtTerm *term;
  struct _zvtprivate *zp;

  d(printf("button pressed\n"));

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  zvt_term_show_pointer (term);

  gdk_window_get_pointer(widget->window, &x, &y, &mask);
  x = ( x - ( widget->style->klass->xthickness + PADDING ) ) / term->charwidth;
  y = ( y - widget->style->klass->ythickness ) / term->charheight + vx->vt.scrollbackoffset;

  if (zp && zp->scrollselect_id!=-1) {
    gtk_timeout_remove(zp->scrollselect_id);
    zp->scrollselect_id=-1;
  }

  /* Shift is an overwrite key for the reporting of the buttons */
  if (!(event->state & GDK_SHIFT_MASK))
    if (vt_report_button(&vx->vt, 1, event->button, event->state, x, y)) 
      return FALSE;

  /* ignore all control-clicks' at this level */
  if (event->state & GDK_CONTROL_MASK) {
    return FALSE;
  }
    
  switch(event->button) {
  case 1:			/* left button */

    /* set selection type, and from which end we are selecting */
    switch(event->type) {
    case GDK_BUTTON_PRESS:
      vx->selectiontype = VT_SELTYPE_CHAR|VT_SELTYPE_BYSTART;
      break;
    case GDK_2BUTTON_PRESS:
      vx->selectiontype = VT_SELTYPE_WORD|VT_SELTYPE_BYSTART|VT_SELTYPE_MOVED;
      break;
    case GDK_3BUTTON_PRESS:
      vx->selectiontype = VT_SELTYPE_LINE|VT_SELTYPE_BYSTART|VT_SELTYPE_MOVED;
      break;
    default:
      break;
    }
    
    /* reset selection */
    vx->selstartx = x;
    vx->selstarty = y;
    vx->selendx = x;
    vx->selendy = y;
    
    /* reset 'drawn' screen (to avoid mis-refreshes) */
    if (!vx->selected) {
      vx->selstartxold = x;
      vx->selstartyold = y;
      vx->selendxold = x;
      vx->selendyold = y;
      vx->selected =1;
    }

    if (event->type != GDK_BUTTON_PRESS) {
      vt_fix_selection(vx);	/* handles by line/by word select update */
    }

    /* either draw (double/triple click) or undraw (single click) selection */
    vt_draw_selection(vx);

    d( printf("selection starting %d %d\n", x, y) );

    gtk_grab_add (widget);
    gdk_pointer_grab (widget->window, FALSE,
		      GDK_BUTTON_RELEASE_MASK |
		      GDK_BUTTON_MOTION_MASK |
		      GDK_POINTER_MOTION_HINT_MASK,
		      NULL, NULL, 0);

    /* 'block' input while we're selecting text */
    if (term->input_id!=-1) {
      gdk_input_remove(term->input_id);
      term->input_id=-1;
    }
    break;

  case 2:			/* middle button - paste */
    if (event->type == GDK_BUTTON_PRESS)
      request_paste (widget, 0, event->time);
    break;

  case 3:			/* right button - select extend? */

    if (vx->selected) {
      int midpos;
      int mypos;

      switch(event->type) {
      case GDK_BUTTON_PRESS:
	vx->selectiontype = VT_SELTYPE_CHAR|VT_SELTYPE_MOVED;
	break;
      case GDK_2BUTTON_PRESS:
	vx->selectiontype = VT_SELTYPE_WORD|VT_SELTYPE_MOVED;
	break;
      case GDK_3BUTTON_PRESS:
	vx->selectiontype = VT_SELTYPE_LINE|VT_SELTYPE_MOVED;
	break;
      default:
	break;
      }

      midpos = ((vx->selstarty+vx->selendy)/2)*vx->vt.width + (vx->selendx+vx->selstartx)/2;
      mypos = y*vx->vt.width + x;
      if (mypos < midpos) {
	vx->selstarty=y;
	vx->selstartx=x;
	vx->selectiontype |= VT_SELTYPE_BYEND;
      } else {
	vx->selendy=y;
	vx->selendx=x;
	vx->selectiontype |= VT_SELTYPE_BYSTART;
      }

      vt_fix_selection(vx);
      vt_draw_selection(vx);
      
      gtk_grab_add (widget);
      gdk_pointer_grab (widget->window, FALSE,
			GDK_BUTTON_RELEASE_MASK |
			GDK_BUTTON_MOTION_MASK |
			GDK_POINTER_MOTION_HINT_MASK,
			NULL, NULL, 0);
      /* 'block' input while we're selecting text */
      if (term->input_id!=-1) {
	gdk_input_remove(term->input_id);
	term->input_id=-1;
      }
    }
    break;

  case 4:
    if (event->type == GDK_BUTTON_PRESS)
      zvt_term_scroll_by_lines (term, -12);
    break;

  case 5:
    if (event->type == GDK_BUTTON_PRESS)
      zvt_term_scroll_by_lines (term, 12);
    break;
  }
  return FALSE;
}

static gint
zvt_term_button_release (GtkWidget      *widget,
			 GdkEventButton *event)
{
  ZvtTerm *term;
  gint x, y;
  GdkModifierType mask;
  struct _vtx *vx;
  struct _zvtprivate *zp;

  d(printf("button released\n"));

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  gdk_window_get_pointer (widget->window, &x, &y, &mask);
  x = ( x - ( widget->style->klass->xthickness + PADDING ) ) / term->charwidth;
  y = ( y - widget->style->klass->ythickness ) / term->charheight + vx->vt.scrollbackoffset;

  /* reset scrolling selection timer if it is enabled */
  if (zp && zp->scrollselect_id!=-1) {
    gtk_timeout_remove(zp->scrollselect_id);
    zp->scrollselect_id=-1;
  }

  /* ignore wheel mice buttons (4 and 5) */
  /* otherwise they affect the selection */
  if (event->button == 4 || event->button == 5)
    return FALSE;
  
  if (vx->selectiontype == VT_SELTYPE_NONE) {
    /* report mouse to terminal */
    if (!(event->state & GDK_SHIFT_MASK))
      if (vt_report_button(&vx->vt, 0, event->button, event->state, x, y))
	return FALSE;
    
    /* ignore all control-clicks' at this level */
    if (event->state & GDK_CONTROL_MASK) {
      return FALSE;
    }
  }

  if (vx->selectiontype & VT_SELTYPE_BYSTART) {
    vx->selendx = x;
    vx->selendy = y;
  } else {
    vx->selstartx = x;
    vx->selstarty = y;
  }

  switch(event->button) {
  case 1:
  case 3:
    d(printf("select from (%d,%d) to (%d,%d)\n", vx->selstartx, vx->selstarty,
	     vx->selendx, vx->selendy));

    gtk_grab_remove (widget);
    gdk_pointer_ungrab (0);

    /* re-enable input */
    if (term->input_id==-1 && term->vx->vt.childfd !=-1) {
      term->input_id =
	gdk_input_add(term->vx->vt.childfd, GDK_INPUT_READ, zvt_term_readdata, term);
    }

    if (vx->selectiontype & VT_SELTYPE_MOVED) {
      vt_fix_selection(vx);
      vt_draw_selection(vx);
          
      /* get the selection as 32 bit utf */
      vt_get_selection(vx, 4, 0);
      
      gtk_selection_owner_set (widget,
			       GDK_SELECTION_PRIMARY,
			       event->time);
    }

    vx->selectiontype = VT_SELTYPE_NONE; /* 'turn off' selecting */

  }

  return FALSE;
}

static void
zvt_term_scroll_by_lines (ZvtTerm *term, int n)
{
  gfloat new_value = 0;

  if (n > 0)
    new_value = MIN (term->adjustment->value + n, term->adjustment->upper - n);
  else if (n < 0)
    new_value = MAX (term->adjustment->value + n, term->adjustment->lower);
  else
    return;

  gtk_adjustment_set_value (term->adjustment, new_value);
}

static gint zvt_selectscroll(gpointer data)
{
  ZvtTerm *term;
  GtkWidget *widget;
  struct _zvtprivate *zp;

  widget = data;
  term = ZVT_TERM (widget);
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  if (zp) {
    zvt_term_scroll_by_lines(term, zp->scrollselect_dir);
  }
  return TRUE;
}

/*
  mouse motion notify.
  only gets called for the first motion?  why?
*/

static gint
zvt_term_motion_notify (GtkWidget      *widget,
			GdkEventMotion *event)
{
  struct _vtx *vx;
  gint x, y;
  ZvtTerm *term;
  struct _zvtprivate *zp;
  struct vt_match *m;
  GdkModifierType mask;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  gdk_window_get_pointer (event->window, &x, &y, &mask);

  x = ( x - ( widget->style->klass->xthickness + PADDING ) ) / term->charwidth;
  y = ( y - widget->style->klass->ythickness ) / term->charheight;

  if (vx->selectiontype != VT_SELTYPE_NONE){
    
    /* move end of selection, and draw it ... */
    if (vx->selectiontype & VT_SELTYPE_BYSTART) {
      vx->selendx = x;
      vx->selendy = y + vx->vt.scrollbackoffset;
    } else {
      vx->selstartx = x;
      vx->selstarty = y + vx->vt.scrollbackoffset;
    }

    vx->selectiontype |= VT_SELTYPE_MOVED;
    
    vt_fix_selection(vx);
    vt_draw_selection(vx);

    if (zp) {
      if (zp->scrollselect_id!=-1) {
	gtk_timeout_remove(zp->scrollselect_id);
	zp->scrollselect_id = -1;
      }
      
      if (y<0 || y>vx->vt.height) {
	if (y<0)
	  zp->scrollselect_dir = -1;
	else
	  zp->scrollselect_dir = 1;
	zp->scrollselect_id = gtk_timeout_add(100, zvt_selectscroll, term);
      }
    }
  } else {
    /* check for magic matches, if we havent already for this lot of output ... */
    if (term->vx->magic_matched==0)
      vt_getmatches(term->vx);

    /* check for actual matches? */
    m = vt_match_check(vx, x, y);
    vt_match_highlight(vx, m);
    if (m) {
      zvt_term_set_pointer(term, zp->cursor_hand);
    } else {
      zvt_term_set_pointer(term, term->cursor_bar);
    }
  }
  /* otherwise, just a mouse event */
  /* make sure the pointer is visible! */
  zvt_term_show_pointer(term);

  return FALSE;
}

/*
 * zvt_term_selection_clear: [internal]
 * @term: A &ZvtTerm widget.
 * @event: Event that triggered the selection claim.
 *
 * Called when another application claims the selection.
 */
gint
zvt_term_selection_clear (GtkWidget *widget, GdkEventSelection *event)
{
  struct _vtx *vx;
  ZvtTerm *term;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  /* Let the selection handling code know that the selection
   * has been changed, since we've overriden the default handler */
  if (!gtk_selection_clear (widget, event))
    return FALSE;

  term = ZVT_TERM (widget);
  vx = term->vx;

  vtx_unrender_selection(vx);
  vt_clear_selection(vx);
  return TRUE;
}

#ifdef ZVT_UTF

/* perform data-conversion on the selection */
static char *
zvt_term_convert_selection(ZvtTerm *term, int type, int *outlen)
{
  char *out;
  int i;
  uint32 c;

  switch (type) {
  default:

#ifdef ZVT_MB
  case TARGET_COMPOUND_TEXT:
  case TARGET_TEXT:
#endif
  case TARGET_STRING: {               /* this is ascii/isolatin1 */
    unsigned char *o;
    d(printf("converting selection to ISOLATIN1\n"));
    out = g_malloc(term->vx->selection_size+1);
    o = out;
    for(i=0;i<term->vx->selection_size;i++) {
      c = term->vx->selection_data[i];
      o[i] = c<0x100?c:'?';
    }
    *outlen = term->vx->selection_size;
    break;
  }
  case TARGET_UTF8: {         /* this is utf-8, basically a local implementation of wcstombs() */
    unsigned char *o;
    unsigned int len=0;
    d(printf("converting selection to UTF-8\n"));
    for(i=0;i<term->vx->selection_size;i++) {
      c = term->vx->selection_data[i];
      if (c<0x80)
	len+=1;
      else if (c<0x800)
	len+=2;
      else if (c<0x10000)
	len+=3;
      else if (c<0x200000)
	len+=4;
      else if (c<0x4000000)
	len+=5;
      else
	len+=6;
    }

    out = g_malloc(len);
    o = out;
    *outlen = len;

    for(i=0;i<term->vx->selection_size;i++) {
      c = term->vx->selection_data[i];
      if (c<0x80)
	*o++=c;
      else if (c<0x800) {
	*o++=(c>>6)|0xc0;
	*o++=(c&0x3f)|0x80;
      } else if (c<0x10000) {
	*o++=(c>>12)|0xe0;
	*o++=((c>>6)&0x3f)|0x80;
	*o++=(c&0x3f)|0x80;
      } else if (c<0x200000) {
	*o++=(c>>18)|0xf0;
	*o++=((c>>12)&0x3f)|0x80;
	*o++=((c>>6)&0x3f)|0x80;
	*o++=(c&0x3f)|0x80;
      } else if (c<0x4000000) {
	*o++=(c>>24)|0xf8;
	*o++=((c>>18)&0x3f)|0x80;
	*o++=((c>>12)&0x3f)|0x80;
	*o++=((c>>6)&0x3f)|0x80;
	*o++=(c&0x3f)|0x80;
      } else
	/* these wont happen */
	;
    }
    d(printf("len = %d, but length\n", len);
    {
      unsigned char *p = out;
      printf("in utf: ");
      while(p<o) {
	printf("%02x", *p++);
      }
      printf("\n");
    });
    break;
  }
  }

  return out;
}
#endif

/* supply the current selection to the caller */
static void
zvt_term_selection_get (GtkWidget        *widget, 
			GtkSelectionData *selection_data_ptr,
			guint             info,
			guint             time)
{
  struct _vtx *vx;
  ZvtTerm *term;
#ifdef ZVT_UTF
  char *converted;
  int len;
  GdkAtom atom;
#endif

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (selection_data_ptr != NULL);

  term = ZVT_TERM (widget);
  vx = term->vx;

#ifdef ZVT_MB
  if (info == TARGET_COMPOUND_TEXT||info == TARGET_TEXT) {
      GdkAtom encoding;
      gint    format;
      guchar *str, *new_str;
      gint    new_len;
#ifdef ZVT_UTF
      str = zvt_term_convert_selection(term, info, &len);
#else
      int len = vx->selection_size;
      str = (guchar*)vx->selection_data;
#endif
      str[len] = '\0';
      gdk_string_to_compound_text (str, &encoding, &format, &new_str, &new_len);
      gtk_selection_data_set (selection_data_ptr, encoding, format,
                            new_str, new_len);
      gdk_free_compound_text (new_str);
#if ZVT_UTF
      g_free(str);
#endif
      return;
  }
#endif /* ZVT_MB */

#ifdef ZVT_UTF
  /* convert selection based on info */
  /* the selection is actually stored in 32 bit chars */
  if (info==TARGET_UTF8)
    atom = gdk_atom_intern ("UTF-8", FALSE);
  else
    atom = GDK_SELECTION_TYPE_STRING;

  converted = zvt_term_convert_selection(term, info, &len);
  gtk_selection_data_set (selection_data_ptr, atom,
			  8, converted, len);
  g_free(converted);
#else
  gtk_selection_data_set (selection_data_ptr, GDK_SELECTION_TYPE_STRING,
			  8, vx->selection_data, vx->selection_size);
#endif
}

/* receive a selection */
/* Signal handler called when the selections owner returns the data */
static void
zvt_term_selection_received (GtkWidget *widget, GtkSelectionData *selection_data, 
			     guint time)
{
  struct _vtx *vx;
  ZvtTerm *term;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (selection_data != NULL);

  term = ZVT_TERM (widget);
  vx = term->vx;

  d(printf("got selection from system\n"));

  /* **** IMPORTANT **** Check to see if retrieval succeeded  */
  /* If we have more selection types we can ask for, try the next one,
     until there are none left */
  if (selection_data->length < 0) {
    struct _zvtprivate *zp = _ZVT_PRIVATE(widget);

    /* now, try again with next selection type */
    if (request_paste(widget, zp->lastselectiontype+1, time)==0)
      g_print ("Selection retrieval failed\n");
    return;
  }

  /* we will get a selection type of atom(UTF-8) for utf text,
     perhaps that needs to do something different if the terminal
     isn't actually in utf8 mode? */

  /* Make sure we got the data in the expected form */
  if (selection_data->type != GDK_SELECTION_TYPE_STRING
      && selection_data->type != gdk_atom_intern("COMPOUND_TEXT", FALSE)
      && selection_data->type != gdk_atom_intern("UTF-8", FALSE)) {
    g_print ("Selection \"STRING\" was not returned as strings!\n");
    return;
  }

  /* paste selection into window! */
  if (selection_data->length)
    {
      int i;
      char *ctmp = selection_data->data;
      gint length = selection_data->length;

      if (selection_data->type == gdk_atom_intern("COMPOUND_TEXT",FALSE)) {
        gchar **list;
        gint    count;

        count = gdk_text_property_to_text_list (selection_data->type,
                                                selection_data->format,
                                                selection_data->data,
                                                selection_data->length,
                                                &list);
        if (count > 0) {
            gint n;
            length = 0;
            for (n=0; n<count; n++)  {
                ctmp = list[n];
                length = strlen (list[n]);
                for(i = 0; i < length; i++)
                    if(ctmp[i] == '\n') ctmp[i] = '\r';

                if (term->scroll_on_keystroke)
                    zvt_term_scroll (term, 0);
                vt_writechild(&vx->vt, ctmp, length);
            }
            gdk_free_text_list (list);
        }
      } else  {
        for (i = 0; i < length; i++)
            if(ctmp[i] == '\n') ctmp[i] = '\r';

        if (term->scroll_on_keystroke)
            zvt_term_scroll (term, 0);
        vt_writechild(&vx->vt, ctmp, length);
      }
    }
}  

/**
 * zvt_term_writechild:
 * @term: A &ZvtTerm widget.
 * @data: Data to write.
 * @len: Length of data to write.
 *
 * Writes @len bytes of data, starting from @data to the subordinate
 * child process.  If the child is unable to handle all of the data
 * at once, then it will return, and asynchronously continue to feed
 * the data to the child.
 *
 * Return Value: The number of bytes written initially.
 */
int
zvt_term_writechild(ZvtTerm *term, char *data, int len)
{
  int length;
  struct _zvtprivate *zp;

  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  /* if we are already pasting something, make sure we append ... */
  if (zp->paste_id == -1)
    length = vt_writechild(&term->vx->vt, data, len);
  else
    length = 0;

  d(printf("wrote %d bytes of %d bytes\n", length, len));

  if (length>=0 && length < len) {

    if (zp->paste_id == -1) {
      zp->paste = g_malloc(len-length);
      zp->paste_offset = 0;
      zp->paste_len = len-length;
      memcpy(zp->paste, data+length, len-length);
      zp->paste_id =
	gdk_input_add(term->vx->vt.keyfd, GDK_INPUT_WRITE, zvt_term_writemore, term);
    } else {
      /* could also drop off offset, but its rarely likely to happen? */
      zp->paste = g_realloc(zp->paste, zp->paste_len + len - length);
      memcpy(zp->paste + zp->paste_len, data+length, len-length);
      zp->paste_len += (len - length);
    }
  }
  return length;
}

#ifdef ZVT_IM_ON_THE_SPOT
/**
 * zvt_term_set_open_im:
 * @term: A &ZvtTerm widget.
 * @state: if True, open IM, else close.
 **/
void
zvt_term_set_open_im (ZvtTerm *term, int state)
{
  if(!state)
    {
      if (term->ic) 
	{
	  gdk_ic_destroy(term->ic);
	  term->ic = NULL;
	}
      return;
    }

  if (gdk_im_ready () && !term->ic)
    {
      gint       width, height;
      GdkICAttr  attr;
      GdkColormap *colormap;
      GdkICAttributesType attrmask = GDK_IC_ALL_REQ;
      GdkIMStyle style;
      GdkIMStyle supported_style = GDK_IM_PREEDIT_NONE |
				   GDK_IM_PREEDIT_NOTHING |
			           GDK_IM_PREEDIT_POSITION |
			           GDK_IM_STATUS_NONE |
				   GDK_IM_STATUS_NOTHING;
      
      if (GTK_WIDGET (term)->style &&
	  GTK_WIDGET (term)->style->font->type != GDK_FONT_FONTSET)
	supported_style &= ~GDK_IM_PREEDIT_POSITION;

      attr.style = style = gdk_im_decide_style (supported_style);
      attr.client_window = attr.focus_window = term->term_window;

      if ((colormap = gtk_widget_get_colormap (GTK_WIDGET (term)))
	  != gtk_widget_get_default_colormap ())
	{
	  attrmask |= GDK_IC_PREEDIT_COLORMAP;
	  attr.preedit_colormap = colormap;
	}

      switch (style & GDK_IM_PREEDIT_MASK)
	{
	  case GDK_IM_PREEDIT_POSITION:
	    if (term->font && term->font->type != GDK_FONT_FONTSET)
	      {
		g_warning ("over-the-spot style requires fontset");
		break;
	      }
#if 0
	    gdk_window_get_size (term->term_window, &width, &height);
#else
	    width  = term->vx->vt.width* term->charwidth;
	    height = term->vx->vt.height* term->charheight;
#endif
	    attrmask |= GDK_IC_PREEDIT_POSITION_REQ|GDK_IC_PREEDIT_FONTSET;
	    attr.spot_location.x = 0;
	    attr.spot_location.y = 0;
	    attr.preedit_area.x = 0;
	    attr.preedit_area.y = 0;
	    attr.preedit_area.width = width;
	    attr.preedit_area.height = height;
	    attr.preedit_fontset = term->font;
	    break;
	}

      term->ic = gdk_ic_new(&attr, attrmask);

      if (!term->ic) 
	{
	  g_warning("Can't create input context.");
	}
    }
}


static void
zvt_im_preedit_set_spot(ZvtTerm *term, int col, int row, int offx, int offy)
{
  if (term->ic && 
      (gdk_ic_get_style (term->ic) & GDK_IM_PREEDIT_POSITION))
    {
      GdkICAttr          attr;
      attr.spot_location.x = col * term->charwidth + offx;
      attr.spot_location.y = row * term->charheight
	  + term->font->ascent + offy;
      gdk_ic_set_attr (term->ic, &attr, GDK_IC_SPOT_LOCATION);
    }
}

static void
zvt_im_preedit_set_foreground(ZvtTerm *term, GdkColor *color)
{
  if (term->ic && 
      (gdk_ic_get_style (term->ic) & GDK_IM_PREEDIT_POSITION))
    {
      GdkICAttr       attr;
      attr.preedit_foreground = *color;
      gdk_ic_set_attr (term->ic, &attr, GDK_IC_PREEDIT_FOREGROUND);
    }
}

static void
zvt_im_preedit_set_background(ZvtTerm *term, GdkColor *color)
{
  if (term->ic && 
      (gdk_ic_get_style (term->ic) & GDK_IM_PREEDIT_POSITION))
    {
      GdkICAttr       attr;
      attr.preedit_background = *color;
      gdk_ic_set_attr (term->ic, &attr, GDK_IC_PREEDIT_BACKGROUND);
    }
}


static void
zvt_im_preedit_set_font(ZvtTerm *term, GdkFont *font)
{
  if (term->ic && 
      (gdk_ic_get_style (term->ic) & GDK_IM_PREEDIT_POSITION))
    {
      GdkICAttr       attr;
      if (font && font->type != GDK_FONT_FONTSET)
	  g_warning ("over-the-spot style requires fontset");
      attr.preedit_fontset = font;
      gdk_ic_set_attr (term->ic, &attr, GDK_IC_PREEDIT_FONTSET);
    }
}
#endif /* ZVT_IM_ON_THE_SPOT */

static void
zvt_term_writemore (gpointer data, gint fd, GdkInputCondition condition)
{
  ZvtTerm *term = (ZvtTerm *)data;
  struct _zvtprivate *zp;
  int length;

  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");
  length = vt_writechild(&term->vx->vt, zp->paste + zp->paste_offset, zp->paste_len);

  d(printf("zvt_writemore(): written %d of %d bytes\n", length, zp->paste_len));

  if (length == -1 || length==zp->paste_len) {
    if (length == -1) {
      g_warning("Write failed to child process\n");
    }
    g_free(zp->paste);
    zp->paste = 0;
    gdk_input_remove(zp->paste_id);
    zp->paste_id = -1;
  } else {
    zp->paste_offset += length;
    zp->paste_len -= length;
  }
}


static gint
zvt_term_cursor_blink(gpointer data)
{
  ZvtTerm *term;
  GtkWidget *widget;

  widget = data;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);

  term = ZVT_TERM (widget);

  term->cursor_blink_state ^= 1;
  vt_cursor_state(data, term->cursor_blink_state);

  return TRUE;
}

/* called by everything when the screen display
   might have updated.
   if mode=1, then the data updated
   if mode=2, then the display updated (scrollbar moved)
*/
static void
zvt_term_updated(ZvtTerm *term, int mode)
{
  /* whenever we update, clear the match table and indicator */
  if (term->vx->magic_matched)
    vt_free_match_blocks(term->vx);
}

/**
 * zvt_term_dtterm_seq_event:
 * @term:   An initialised &ZvtTerm widget
 *          the term->vx->vt.arg.num.intargs member should be an
 *          &int pointing to an array consisting of the 'Ps' parameters
 *          from the CSI sequence (translated into ints, that is)
 * @report: integer. (The 1st Ps from the CSI Ps [; Ps [; Ps ]] t sequence)
 *
 * handle a dtterm escape sequence: generate a report
 * and write it back to the child's pty if necessary.
 **/
static void
zvt_term_dtterm_seq_event (ZvtTerm *term)
{
#define DTTERM_2ITEM_REPORT() \
  g_snprintf(rbuf, bufs, "%s%d;%d;%dt", CSI, (seqn - 10), ps[0], ps[1])
#define DTTERM_TITLE_REPORT(c) \
  g_snprintf(rbuf, bufs, "%s%c%s%s", OSC, c, ZVT_GTK_WINDOW(term)->title, ST)

  static const char * CSI = "\x1b[";
  static const char * OSC = "\x1b]";
  static const char * ST  = "\x1b\\";
  
  char rbuf[128];
  const int bufs = sizeof(rbuf); 

  int   ps[2] = { 0, 0 }; /* 'Ps' parameters from/for dtterm sequence */
  int   seqn  = *(term->vx->vt.arg.num.intargs);
  int * arg   = term->vx->vt.arg.num.intargs + 1;
  
  d(printf("signal received and handed over\n"));

  memset(rbuf, 0, sizeof(rbuf));

  switch( seqn ) {
#if GTK_MAJOR_VERSION < 2    
  case 1:
    /* de-iconify */
    {
      GdkWindow *g = ZVT_GDK_TOPWIN(term);
      gdk_window_show (g);
      gdk_window_raise(g);
    }
    break;
  case 2:
    /* iconify */
    {
      GdkWindow *g = ZVT_GDK_TOPWIN(term);
      Window     w = GDK_WINDOW_XWINDOW (g);
      Display   *d = GDK_WINDOW_XDISPLAY(g);
      XWindowAttributes xwa;
      XGetWindowAttributes(d, w, &xwa);
      XIconifyWindow(d, w, XScreenNumberOfScreen(xwa.screen));
    }
    break;
#else
  case 1:
    gtk_deiconify_window(ZVT_GTK_WINDOW(term));
    break;    
  case 2:
    gtk_iconify_window(ZVT_GTK_WINDOW(term));
    break;
#endif
    break;
  case 3:
    /* move to x, y */
    ps[0] = *arg++;
    ps[1] = *arg;
    gdk_window_move(ZVT_GDK_TOPWIN(term), ps[0], ps[1]);
    break;
  case 4:
    /* resize to h x w pixels (actually nearest possible char x char size) */
    /* as this is what xterm appears to do. */
    ps[0] = *arg++ / term->charheight;
    ps[1] = *arg   / term->charwidth;
    zvt_term_set_size(term, ps[1], ps[0]);
    break;
  case 5:
    gdk_window_raise(ZVT_GDK_TOPWIN(term));
    break;
  case 6:
    gdk_window_lower(ZVT_GDK_TOPWIN(term));
    break;
  case 7:
    /* refresh */
    vt_update(term->vx, UPDATE_REFRESH);
    break;
  case 8:
    /* resize to h x w chars */
    ps[0] = *arg++;
    ps[1] = *arg;
    zvt_term_set_size(term, ps[1], ps[0]);
    break;
  case 9:
    /* (*arg == 0) ? unmaximise : maximise */
    break;
  case 11:
    d(printf("determine if term is iconified, echo appropriate sequence\n"));
    /* IMPLEMENTME : iconifiedp ? (CSI 2 t) : (CSI 1 t) */
    break;
  case 13:
    /* report window position (pixels x,y) */
    gdk_window_get_position(ZVT_GDK_TOPWIN(term), &ps[0], &ps[1]);
    DTTERM_2ITEM_REPORT();
    break;
  case 14:
    /* report window sixe (pixels h,w) */
    gdk_window_get_size(ZVT_GDK_WINDOW(term), &ps[1], &ps[0]);
    DTTERM_2ITEM_REPORT();
    break;
  case 18:
    /* report window size (chars h,w) */
    ps[1] = term->vx->vt.width;
    ps[0] = term->vx->vt.height;
    DTTERM_2ITEM_REPORT();
    break;
  case 19:
    /* report screen size (chars h,w) */
    ps[1] = gdk_screen_width()  / term->charwidth;
    ps[0] = gdk_screen_height() / term->charheight;
    DTTERM_2ITEM_REPORT();
    break;
  case 20:
    /* report icon label */
    DTTERM_TITLE_REPORT('L');
    break;
  case 21:
    /* report window title */
    DTTERM_TITLE_REPORT('l');
    break;
  default:
    /* DECSLPP - resize to #seqn lines: */
    if(seqn >= 24)
      zvt_term_set_size(term, term->vx->vt.width, seqn);
    else
      d(printf("Unknown dtterm sequence %d\n", seqn));
  }

  /* have we collected a report? if so, write it back out... */
  if(rbuf[0]) {
    rbuf[sizeof(rbuf) - 1] = 0;
    vt_writechild( &(term->vx->vt), rbuf, strlen(rbuf) );
  }
}

/*
 * Callback for when the adjustment changes - i.e., the scrollbar
 * moves.
 */
static void 
zvt_term_scrollbar_moved (GtkAdjustment *adj, GtkWidget *widget)
{
  int line;
  ZvtTerm *term;
  struct _vtx *vx;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  line = term->vx->vt.scrollbacklines - (int)adj->value;

  /* needed for floating point errors in slider code */
  if (line < 0)
    line = 0;

  term->vx->vt.scrollbackoffset = -line;

  d(printf("zvt_term_scrollbar_moved adj->value=%f\n", adj->value));

  /* will redraw if scrollbar moved */
  vt_update (term->vx, UPDATE_SCROLLBACK);

  /* for scrolling selection */
  if (zp && zp->scrollselect_id != -1) {
    int x,y;

    if (zp->scrollselect_dir>0) {
      x = vx->vt.width-1;
      y = vx->vt.height-1;
    } else {
      x = 0;
      y = 0;
    }

    if (vx->selectiontype & VT_SELTYPE_BYSTART) {
      vx->selendx = x;
      vx->selendy = y + vx->vt.scrollbackoffset;
    } else {
      vx->selstartx = x;
      vx->selstarty = y + vx->vt.scrollbackoffset;
    }

    vt_fix_selection(vx);
    vt_draw_selection(vx);
  }

  zvt_term_updated(term, 2);
}


/**
 * zvt_term_forkpty:
 * @term: A &ZvtTerm widget.
 * @do_uwtmp_log: If %TRUE, then log the session in wtmp(4) and utmp(4).
 *
 * Fork a child process, with a master controlling terminal.
 */
int
zvt_term_forkpty (ZvtTerm *term, int do_uwtmp_log)
{
  int pid;

  g_return_val_if_fail (term != NULL, -1);
  g_return_val_if_fail (ZVT_IS_TERM (term), -1);

  /* cannot fork twice! */
  if (term->input_id != -1)
    return -1;

  pid = vt_forkpty (&term->vx->vt, do_uwtmp_log);
  if (pid > 0) {
    term->input_id = 
      gdk_input_add(term->vx->vt.childfd, GDK_INPUT_READ, zvt_term_readdata, term);
    term->msg_id =
      gdk_input_add(term->vx->vt.msgfd, GDK_INPUT_READ, zvt_term_readmsg, term);
  }

  return pid;
}

/**
 * zvt_term_killchild:
 * @term: A &ZvtTerm widget.
 * @signal: A signal number.
 *
 * Send the signal @signal to the child process.  Note that a child
 * process must have first been started using zvt_term_forkpty().
 *
 * Return Value: See kill(2).
 * See Also: signal(5).
 */
int
zvt_term_killchild (ZvtTerm *term, int signal)
{
  g_return_val_if_fail (term != NULL, -1);
  g_return_val_if_fail (ZVT_IS_TERM (term), -1);

  return vt_killchild (&term->vx->vt, signal);
}

/**
 * zvt_term_closepty:
 * @term: A &ZvtTerm widget.
 *
 * Close master pty to the child process.  It is upto the child to
 * recognise its pty has been closed, and to exit appropriately.
 *
 * Note that a child process must have first been started using
 * zvt_term_forkpty().
 *
 * Return Value: See close(2).
 */
int
zvt_term_closepty (ZvtTerm *term)
{
  g_return_val_if_fail (term != NULL, -1);
  g_return_val_if_fail (ZVT_IS_TERM (term), -1);

  if (term->input_id != -1) {
    gdk_input_remove (term->input_id);
    term->input_id = -1;
  }

  if (term->msg_id != -1) {
    gdk_input_remove (term->msg_id);
    term->msg_id = -1;
  }

  return vt_closepty (&term->vx->vt);
}

static void
zvt_term_scroll (ZvtTerm *term, int n)
{
  gfloat new_value = 0;
  
  if (n) {
    new_value = term->adjustment->value + (n * term->adjustment->page_size);
  } else if (new_value == (term->adjustment->upper - term->adjustment->page_size)) {
    return;
  } else {
    new_value = term->adjustment->upper - term->adjustment->page_size;
  }

  gtk_adjustment_set_value (
      term->adjustment,
      n > 0 ? MIN(new_value, term->adjustment->upper- term->adjustment->page_size) :
      MAX(new_value, term->adjustment->lower));
}

/*
 * Keyboard input callback
 */

/* remapping table for function keys 5-20 */
static unsigned char f1_f20_remap[] =
   {11,12,13,14,15,17,18,19,20,21,23,24,25,26,28,29,31,32,33,34};

static gint
zvt_term_key_press (GtkWidget *widget, GdkEventKey *event)
{
  char buffer[128];
  char *p=buffer;
  struct _vtx *vx;
  ZvtTerm *term;
  int handled;
  char *cursor;
  int appl_keypad;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;

  appl_keypad = (vx->vt.mode & VTMODE_APP_KEYPAD) != 0;

  zvt_term_hide_pointer(term);
  
  d(printf("keyval = %04x state = %x\n", event->keyval, event->state));
  handled = TRUE;
  switch (event->keyval) {
  case GDK_BackSpace:
    if (event->state & GDK_MOD1_MASK)
      *p++ = '\033';

    if (term->swap_del_key)
      *p++ = '\177';
    else
      *p++ = 8;
    break;
  case GDK_KP_Right:
  case GDK_Right:
    cursor ="C";
    goto do_cursor;
  case GDK_KP_Left:
  case GDK_Left:
    cursor = "D";
    goto do_cursor;
  case GDK_KP_Up:
  case GDK_Up:
    cursor = "A";
    goto do_cursor;
  case GDK_KP_Down:
  case GDK_Down:
    cursor = "B";
    do_cursor:
    if (vx->vt.mode & VTMODE_APP_CURSOR)
      p+=sprintf (p, "\033O%s", cursor);
    else
      p+=sprintf (p, "\033[%s", cursor);
    break;
  case GDK_KP_Insert:
  case GDK_Insert:
    if ((event->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK){
        request_paste (widget, 0, event->time);
    } else {
      p+=sprintf (p, "\033[2~");
    }
    break;
  case GDK_Delete:
    if (term->del_is_del){
      if (event->state & GDK_MOD1_MASK)
	*p++ = '\033';
      if (term->swap_del_key)
	*p++ = 8;
      else
	*p++ = '\177';
    } else {
      p+=sprintf (p, "\033[3~");
    }
    break;
  case GDK_KP_Delete:
    p+=sprintf (p, "\033[3~");
    break;
  case GDK_KP_Home:
  case GDK_Home:
    p+=sprintf (p, "\033[1~");
    break;
  case GDK_KP_End:
  case GDK_End:
    p+=sprintf (p, "\033[4~");
    break;
  case GDK_KP_Page_Up:
  case GDK_Page_Up:
    if (event->state & GDK_SHIFT_MASK){
      zvt_term_scroll (term, -1);
    } else
      p+=sprintf (p, "\033[5~");
    break;
  case GDK_KP_Page_Down:
  case GDK_Page_Down:
    if (event->state & GDK_SHIFT_MASK){
      zvt_term_scroll (term, 1);
    } else
      p+=sprintf (p, "\033[6~");
    break;

  case GDK_KP_F1:  case GDK_KP_F2:  case GDK_KP_F3:  case GDK_KP_F4:
    if (event->state & GDK_SHIFT_MASK){
       p+=sprintf (p, "\033[%d~", f1_f20_remap[10+event->keyval-GDK_KP_F1]);
    } else
       p+=sprintf (p, "\033[%d~", f1_f20_remap[event->keyval-GDK_KP_F1]);
    break;

  case GDK_F1:  case GDK_F2:  case GDK_F3:  case GDK_F4:
    if (event->state & GDK_SHIFT_MASK){
       p+=sprintf (p, "\033[%d~", f1_f20_remap[10+event->keyval-GDK_F1]);
    } else
       p+=sprintf (p, "\033O%c", 'P' + (event->keyval-GDK_F1));
    break;
      
  case GDK_F5:  case GDK_F6:  case GDK_F7:  case GDK_F8:
  case GDK_F9:  case GDK_F10:  case GDK_F11:  case GDK_F12:
  case GDK_F13:  case GDK_F14:  case GDK_F15:  case GDK_F16:
  case GDK_F17:  case GDK_F18:  case GDK_F19:  case GDK_F20:
    if ( (event->state & GDK_SHIFT_MASK) &&
         ((event->keyval-GDK_F1) < 10) ){
       p+=sprintf (p, "\033[%d~", f1_f20_remap[10+event->keyval-GDK_F1]);
    } else
       p+=sprintf (p, "\033[%d~", f1_f20_remap[event->keyval-GDK_F1]);
    break;

  case GDK_KP_0:  case GDK_KP_1:  case GDK_KP_2:  case GDK_KP_3:
  case GDK_KP_4:  case GDK_KP_5:  case GDK_KP_6:  case GDK_KP_7:
  case GDK_KP_8:  case GDK_KP_9:
    if (appl_keypad) {
      p+=sprintf (p, "\033O%c", 'p' + (event->keyval - GDK_KP_0));
    } else {
      *p++ = '0' + (event->keyval - GDK_KP_0);
    }
    break;
  case GDK_KP_Enter:
    if (appl_keypad) {
      p+=sprintf (p, "\033OM");
    } else {
      *p++ = '\r';
    }
    break;
  case GDK_KP_Add:
    if (appl_keypad) {
      p+=sprintf (p, "\033Ok");
    } else {
      *p++ = '+';
    }
    break;
  case GDK_KP_Subtract:
    if (appl_keypad) {
      p+=sprintf (p, "\033Om");
    } else {
      *p++ = '-';
    }
    break;
  case GDK_KP_Multiply:
    if (appl_keypad) {
      p+=sprintf (p, "\033Oj");
    } else {
      *p++ = '*';
    }
    break;
  case GDK_KP_Divide:
    if (appl_keypad) {
      p+=sprintf (p, "\033Oo");
    } else {
      *p++ = '/';
    }
    break;
  case GDK_KP_Separator:  /* aka KP Comma */
    if (appl_keypad) {
      p+=sprintf (p, "\033Ol");
    } else {
      *p++ = '-';
    }
    break;
  case GDK_KP_Decimal:
    if (appl_keypad) {
      p+=sprintf (p, "\033On");
    } else {
      *p++ = '.';
    }
    break;

  case GDK_KP_Begin:
    /* ? middle key of keypad */
  case GDK_Print:
  case GDK_Scroll_Lock:
  case GDK_Pause:
    /* control keys */
  case GDK_Shift_Lock:
  case GDK_Num_Lock:
  case GDK_Caps_Lock:
    /* ignore - for now FIXME: do something here*/
    break;
  case GDK_Control_L:
  case GDK_Control_R:
    break;
  case GDK_Shift_L:
  case GDK_Shift_R:
    break;
  case GDK_Alt_L:
  case GDK_Alt_R:
  case GDK_Meta_L:
  case GDK_Meta_R:
    break;
  case GDK_Mode_switch:
  case GDK_Multi_key:
    break;
  case GDK_ISO_Left_Tab:
    *p++ = gdk_keyval_from_name("Tab");
    break;
  case GDK_Tab:
    *p++ = event->keyval;
    break;
  case GDK_Menu:
    p+=sprintf (p, "\033[29~");
    break;
  case ' ':
    /* maps single characters to correct control and alt versions */
    if (event->state & GDK_CONTROL_MASK)
      *p++=event->keyval & 0x1f;
    else if (event->state & GDK_MOD1_MASK)
      *p++=event->keyval + 0x80; /* this works for space at least */
    else
      *p++=event->keyval;
    break;
  default:
      if (event->length > 0){
	if (event->state & (GDK_MOD1_MASK | GDK_MOD4_MASK)){
	   *p++ = '\033';
        }
	memcpy(p, event->string, event->length*sizeof(char));
	p += event->length;
      } else {
	handled = FALSE;
      }
      d(printf ("[%s,%d,%d]\n", event->string, event->length, handled));
  }
  if (handled && p>buffer) {
    vt_writechild(&vx->vt, buffer, (p-buffer));
    if (term->scroll_on_keystroke) zvt_term_scroll (term, 0);
  }

  return handled;
}

/* dummy default signal handler */
static void
zvt_term_child_died (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  /* perhaps we should do something here? */
}

/* dummy default signal handler for title_changed */
static void
zvt_term_title_changed (ZvtTerm *term, VTTITLE_TYPE type, char *str)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  /* perhaps we should do something here? */
  gtk_window_set_title(ZVT_GTK_WINDOW(term), str);
}

/**
 * zvt_term_dtterm_seq:
 * @term: An initialised &ZvtTerm.
 * @report: integer (The 1st Ps from the CSI Ps [; Ps [; Ps ]] t sequence)
 *
 * Catches a request for a report and dispatches it to the connected
 * signal handler (default: zvt_term_dtterm_seq_event)
 **/
static void
zvt_term_dtterm_seq (void *term)
{
  ZvtTerm *TERM = term;

  g_return_if_fail (TERM != NULL);
  g_return_if_fail (ZVT_IS_TERM (TERM));

  gtk_signal_emit(GTK_OBJECT(TERM), term_signals[DTTERM_SEQ]);
}

/**
 * zvt_term_match_add:
 * @term: An initialised &ZvtTerm.
 * @regex: A regular expression to match.  It should be concise
 * enough so that it doesn't match whole lines.
 * @highlight_mask: Mask of bits used to set the attributes used
 * to highlight the match as the mouse moves over it.
 * @data: User data.
 * 
 * Add a new auto-match regular expression.  The
 * zvt_term_match_check() function can be used to check for matches
 * using screen coordinates.
 * 
 * Each regular expression @regex will be matched against each
 * line in the visible buffer.
 *
 * The @highlight_mask is taken from the VTATTR_* bits, as defined
 * in vt.h.  These include VTATTR_BOLD, VTATTR_UNDERLINE, etc.
 * Colours may also be set by including the colour index in the
 * appropriate bit position.  Colours and attributes may be combined.
 *
 * e.g. to set foreground colour 2, and background colour 5, use
 * highlight_mask = (2<<VTATTR_FORECOLOURB)|(5<<VTATTR_BACKCOLOURB).
 *
 * Return value: Returns -1 when the regular expression is invalid
 * and cannot be compiled (see regcomp(3c)).  Otherwise returns 0.
 **/
int
zvt_term_match_add(ZvtTerm *term, char *regex, uint32 highlight_mask, void *data)
{
  struct vt_magic_match *m;
  struct _vtx *vx = term->vx;

  m = g_malloc0(sizeof(*m));
  if (regcomp(&m->preg, regex, REG_EXTENDED)==0) {
    m->regex = g_strdup(regex);
    vt_list_addtail(&vx->magic_list, (struct vt_listnode *)m);
    m->user_data = data;
    m->highlight_mask = highlight_mask & VTATTR_MASK;
  } else {
    g_free(m);
    return -1;
  }
  return 0;
}


/**
 * zvt_term_match_clear:
 * @term: An initialised &ZvtTerm.
 * @regex: A regular expression to remove, or %NULL to remove
 * all match strings.
 * 
 * Remove a specific match string, or all match strings
 * from the terminal @term.
 **/
void
zvt_term_match_clear(ZvtTerm *term, char *regex)
{
  vt_match_clear(term->vx, regex);
}

/**
 * zvt_term_match_check:
 * @term: An initialised &ZvtTerm.
 * @x: X coordinate, in character coordinates.
 * @y: Y coordinate to check, in character coordinates.
 * @user_data_ptr: A pointer to a location to hold the user-data
 * associated with this match.  If NULL, then this is ignored.
 * 
 * Check for a match at a given character location.
 *
 * Return Values: Returns the string matched.  If @user_data_ptr is
 * non-NULL, then it is set to the user_data associated with this
 * match type.  The return value is only guaranteed valid until the next
 * iteration of the gtk main loop.
 **/
char *
zvt_term_match_check(ZvtTerm *term, int x, int y, void **user_data_ptr)
{
  struct vt_match *m;
  m = vt_match_check(term->vx, x, y);
  if (m) {
    if (user_data_ptr)
      *user_data_ptr = m->match->user_data;
    return m->matchstr;
  }
  return 0;
}

/* raise the title_changed signal */
static void
zvt_term_title_changed_raise (void *user_data, VTTITLE_TYPE type, char *str)
{
  ZvtTerm *term = user_data;
  
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  gtk_signal_emit(GTK_OBJECT(term), term_signals[TITLE_CHANGED], type, str);
}

static void
vtx_unrender_selection (struct _vtx *vx)
{
  /* need to 'un-render' selected area */
  if (vx->selected)
    {
      vx->selstartx = vx->selendx;
      vx->selstarty = vx->selendy;
      vt_draw_selection(vx);	/* un-render selection */
      vx->selected = 0;
    }
}

/*
 * this callback is called when data is ready on the child's file descriptor
 *
 * Read all data waiting on the file descriptor, updating the virtual
 * terminal buffer, until there is no more data to read, and then render it.
 *
 * NOTE: this may impact on slow machines, but it may not also ...!
 */
static void
zvt_term_readdata (gpointer data, gint fd, GdkInputCondition condition)
{
  gboolean update;
  gchar buffer[4096];
  gint count, saveerrno;
  struct _vtx *vx;
  ZvtTerm *term;

  d( printf("zvt_term_readdata\n") );

  term = (ZvtTerm *) data;

  if (term->input_id == -1)
    return;

  update = FALSE;
  vx = term->vx;
  vtx_unrender_selection (vx);
  saveerrno = EAGAIN;

  vt_cursor_state (term, 0);
  vt_match_highlight(term->vx, 0);
  while ( (saveerrno == EAGAIN) && (count = read (fd, buffer, 4096)) > 0)  {
    update = TRUE;
    saveerrno = errno;
    vt_parse_vt (&vx->vt, buffer, count);
  }

  if (update) {
    if (GTK_WIDGET_DRAWABLE (term)) {
      d( printf("zvt_term_readdata: update from read\n") );
      vt_update (vx, UPDATE_CHANGES);
    } else {
      d( printf("zvt_term_readdata: suspending update -- not drawable\n") );
    }
  } else {
    saveerrno = errno;
  }

  /* *always* turn the cursor back on */
  vt_cursor_state (term, 1);
  
  /* fix scroll bar */
  if (term->scroll_on_output) {
      zvt_term_scroll (term, 0);
  }

  /* flush all X events - this is really necessary to stop X queuing up
   * lots of screen updates and reducing interactivity on a busy terminal
   */
  gdk_flush ();

  /* read failed? oh well, that's life -- we handle dead children via
   * SIGCHLD
   */
  zvt_term_fix_scrollbar (term);

  zvt_term_updated(term, 2);
}

/**
 * zvt_term_feed:
 * @term: A &ZvtTerm widget.
 * @text: The text to feed.
 * @len:  The text length.
 *
 * This makes the terminal emulator process the stream of
 * characters in @text for @len bytes.  The text is interpreted
 * by the terminal emulator as if it were generated by a child
 * process.
 *
 * This is used by code that needs a terminal emulator, but
 * does not use a child process.
 */
void
zvt_term_feed (ZvtTerm *term, char *text, int len)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (text != NULL);
  
  vt_cursor_state (term, 0);
  vt_match_highlight(term->vx, 0);
  vtx_unrender_selection (term->vx);
  vt_parse_vt (&term->vx->vt, text, len);
  vt_update (term->vx, UPDATE_CHANGES);
  
  /* *always* turn the cursor back on */
  vt_cursor_state (term, 1);
  
  /* fix scroll bar */
  if (term->scroll_on_output)
    zvt_term_scroll (term, 0);
  
  /* flush all X events - this is really necessary to stop X queuing up
   * lots of screen updates and reducing interactivity on a busy terminal
   */
  gdk_flush ();
  
  /* read failed? oh well, that's life -- we handle dead children via
   *  SIGCHLD
   */
  zvt_term_fix_scrollbar (term);

  zvt_term_updated(term, 1);
}

static void
zvt_term_readmsg (gpointer data, gint fd, GdkInputCondition condition)
{
  ZvtTerm *term = (ZvtTerm *)data;

  /* I suppose I should bother reading the message from the fd, but
   * it doesn't seem worth the trouble <shrug>
   */
  if (term->input_id != -1) {
    gdk_input_remove (term->input_id);
    term->input_id=-1;
  }

  zvt_term_closepty (term);

  /* signal application FIXME: include error/non error code */
  gtk_signal_emit (GTK_OBJECT(term), term_signals[CHILD_DIED]);
}

/**
 * zvt_term_set_background:
 * @terminal: A &ZvtTerm widget.
 * @pixmap_file: file containing the pixmap image
 * @transparent: true if we want to run in transparent mode
 * @flags: A bitmask of background options:
 *   ZVT_BACKGROUND_SHADED, shade the transparency pixmap.
 *   ZVT_BACKGROUND_SCROLL, allow smart scrolling of the pixmap,
 *   ignored if transparency is requested.
 *
 * Sets the background of the @terminal.  If @pixmap_file and
 * @transparent are %NULL and %FALSE, then a standard filled background
 * is set.
 */
void
zvt_term_set_background (ZvtTerm *terminal, char *pixmap_file, 
			 int transparent, int flags)
{
#if 0
  if (!(zvt_term_get_capabilities (terminal) & ZVT_TERM_PIXMAP_SUPPORT))  
    return; 
#endif

  /* if there no changes return */
  if(!((flags & ZVT_BACKGROUND_SHADED) == terminal->shaded &&
       safe_strcmp(pixmap_file,terminal->pixmap_filename)==0 &&
       transparent == terminal->transparent)) {
  
    if (terminal->background.pix) {
      gdk_pixmap_unref(terminal->background.pix);
      terminal->background.pix = NULL;
    }

    terminal->transparent = transparent;
    terminal->shaded = (flags & ZVT_BACKGROUND_SHADED) != 0;
    g_free (terminal->pixmap_filename);
    
    if (pixmap_file)
      terminal->pixmap_filename = g_strdup (pixmap_file);
    else
      terminal->pixmap_filename = NULL;

    /* load it, set it in the gc's */
    load_background (terminal);
  }

  /* determine the scroll-mode to use */
  if (transparent)
    terminal->vx->scroll_type=VT_SCROLL_NEVER;
  else if (terminal->pixmap_filename) {
    if (flags & ZVT_BACKGROUND_SCROLL)
      terminal->vx->scroll_type=VT_SCROLL_NEVER;
    else
      terminal->vx->scroll_type=VT_SCROLL_NEVER;
  } else
    terminal->vx->scroll_type=VT_SCROLL_NEVER;
}

/*
 * callback rendering functions called by vt_update, etc
 */
static int
vt_cursor_state(void *user_data, int state)
{
  ZvtTerm *term;
  GtkWidget *widget;
  int old_state;

  widget = user_data;

  g_return_val_if_fail (widget != NULL, 0);
  g_return_val_if_fail (ZVT_IS_TERM (widget), 0);

  term = ZVT_TERM (widget);
  old_state = term->cursor_on;

  /* only call vt_draw_cursor if the state has changed */
  if (old_state ^ state) {
    if (GTK_WIDGET_DRAWABLE (widget))	{
      if(term->cursor_filled || !state)
        vt_draw_cursor(term->vx, state);
      else {
        vt_draw_cursor(term->vx, FALSE);
        if (term->vx->vt.scrollbackold == 0 &&
	    term->vx->vt.cursorx < term->vx->vt.width) {
	  int offx = widget->style->klass->xthickness + PADDING;
	  int offy = widget->style->klass->ythickness;
	  gdk_draw_rectangle (widget->window,
			      term->fore_gc, 0,
			      offx + term->vx->vt.cursorx *
			        term->charwidth + 1,
			      offy + term->vx->vt.cursory *
			        term->charheight + 1,
			      term->charwidth - 2,
			      term->charheight - 2);
	}
      }
      term->cursor_on = state;
    }
  }
  return old_state;
}

void
vt_draw_text(void *user_data, struct vt_line *line, int row, int col, int len, int attr)
{
  GdkFont *f;
  struct _vtx *vx;
  ZvtTerm *term;
  GtkWidget *widget;
  int fore, back, or;
  GdkColor pen;
  GdkGC *fgc, *bgc;
  int overstrike=0;
  int dofill=0;
  int offx, offy, x, y;
  int i;
  uint32 c;
  struct _zvtprivate *zp=0;

  GdkFontPrivate *font_private;
  GdkWindowPrivate *drawable_private;
  GdkGCPrivate *gc_private;
  
  widget = user_data;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  if (!GTK_WIDGET_DRAWABLE (widget))
    return;

  term = ZVT_TERM (widget);

  vx = term->vx;

  if (len+col>vx->vt.width)
    len = vx->vt.width-col;

  /* rendering offsetx */
  x = col * term->charwidth;
  y = row * term->charheight + term->font->ascent;
  offx = widget->style->klass->xthickness + PADDING;
  offy = widget->style->klass->ythickness;
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  if (attr & VTATTR_BOLD)  {
    or = 8;
    f = term->font_bold;
    if (f==NULL) {
      f = term->font;
      overstrike = 1;
      if (zp && zp->bold_save) {
	gdk_draw_pixmap(zp->bold_save, term->fore_gc, GTK_WIDGET(term)->window,
			x + offx + len*term->charwidth, offy + row*term->charheight,
			0, 0, 1, term->charheight);
      }
    }
  } else  {
    or = 0;
    f = term->font;
  }

  fore = (attr & VTATTR_FORECOLOURM) >> VTATTR_FORECOLOURB;
  back = (attr & VTATTR_BACKCOLOURM) >> VTATTR_BACKCOLOURB;

  if (fore < 8)
    fore |= or;

  /* set the right colour in the appropriate gc */
  fgc = term->fore_gc;
  bgc = term->back_gc;

  /* for reverse, swap colours's */
  if (attr & VTATTR_REVERSE) {
    int tmp;
    tmp = fore;fore=back;back=tmp;
  }

  if (term->back_last != back) {
    pen.pixel = term->colors [back];
    gdk_gc_set_background (fgc, &pen);
    term->back_last = back;
  }
  
  if (term->fore_last != fore) {
    pen.pixel = term->colors [fore];
    gdk_gc_set_foreground (fgc, &pen);
    term->fore_last = fore;
  }

  /* optimise: dont 'clear' background if not in 
   * expose, and background colour == window colour
   * this may look a bit weird, it really does need to 
   * be this way - so dont touch!
   *
   * if the terminal is transparent we must redraw 
   * all the time as we can't optimize in that case
   *
   * This needs a re-visit due to recent changes for pixmap
   * support.
   */
#if 0
  printf("f=%d:%d ", fore,back);
  printf("e= %d ", term->in_expose);
  printf("b= %d ", vx->back_match);
  printf("t= %d ", term->transparent);
#endif

#if 1

  if (term->in_expose || vx->back_match == 0) {
    if ((term->transparent || term->pixmap_filename)
	&& back==17) {
      gdk_draw_rectangle (widget->window,
			  bgc, 1,
			  offx + col * term->charwidth,
			  offy + row * term->charheight,
			  len * term->charwidth,
			  term->charheight);
      d(printf("done fill "));
    } else {
      dofill=1;
      d(printf("do fill "));
    }
  }

#else

  if (term->in_expose || vx->back_match == 0) {
    if ((term->transparent || term->pixmap_filename)
	&& ( term->in_expose==0 && !vx->back_match )) {
      /*	&& !term->in_expose && back >= 17) {*/
      gdk_draw_rectangle (widget->window,
			  bgc, 1,
			  offx + col * term->charwidth,
			  offy + row * term->charheight,
			  len * term->charwidth,
			  term->charheight);
      d(printf("draw pixmap\n"));
    } else if ( (term->in_expose && back < 17)
		|| ( term->in_expose==0 && !vx->back_match )) {
      dofill=1;
      /* if we can get away with it, use XDrawImageString() to do
	 the background fill! */
      d(printf("fill via image\n"));
    } else {
      d(printf("do nothing\n"));
    }
  } else {
    d(printf("not clearing background in_expose = %d, back_match=%d\n", 
	     term->in_expose, vx->back_match));
    d(printf("txt = '%.*s'\n", len, text));
  }
#endif

  font_private = (GdkFontPrivate*) f;
  drawable_private = (GdkWindowPrivate *)widget->window;
  gc_private = (GdkGCPrivate *)fgc;

  /* make sure we have the space to expand the text */
  if (zp->text_expand==0 || zp->text_expandlen<len) {
    zp->text_expand = g_realloc(zp->text_expand, len*sizeof(uint32));
    zp->text_expandlen = len;
  }

  /* convert text in input into the format suitable for output ... */
  switch (zp->fonttype) {
  case ZVT_FONT_1BYTE: {		/* simple single-byte font */
    char *expand = zp->text_expand;
    XFontStruct *xfont;

    for(i=0;i<len;i++) {
      c=VT_ASCII(line->data[i+col]);
      if (c>=256)
	c='?';
      expand[i]=c&0xff;
    }

    d(printf(" '%.*s'\n", len, expand));

    /* render characters, with fill if we can */
    xfont = (XFontStruct *) font_private->xfont;
    XSetFont(drawable_private->xdisplay, gc_private->xgc, xfont->fid);
    if (dofill) {
      XDrawImageString(drawable_private->xdisplay, drawable_private->xwindow,
		       gc_private->xgc, offx + x, offy + y, expand, len);
    } else {
      XDrawString(drawable_private->xdisplay, drawable_private->xwindow,
		  gc_private->xgc, offx + x, offy + y, expand, len);
    }
    if (overstrike)
      XDrawString(drawable_private->xdisplay, drawable_private->xwindow,
		  gc_private->xgc, offx + x + 1, offy + y, expand, len);
  }
  break;
  case ZVT_FONT_2BYTE: {
    XChar2b *expand16 = zp->text_expand;
    XFontStruct *xfont;

    /* this needs to check for valid chars? */
    for (i=0;i<len;i++) {
      c=VT_ASCII(line->data[i+col]);
      expand16[i].byte2=c&0xff;
      expand16[i].byte1=(c>>8)&0xff;
      /*printf("(%04x)", c);*/
    }
    /*    printf("\n");*/

    /* render 2-byte characters, with fill if we can */
    xfont = (XFontStruct *) font_private->xfont;
    XSetFont(drawable_private->xdisplay, gc_private->xgc, xfont->fid);
    if (dofill) {
      XDrawImageString16(drawable_private->xdisplay, drawable_private->xwindow,
			 gc_private->xgc, offx + x, offy + y, expand16, len);
    } else {
      XDrawString16(drawable_private->xdisplay, drawable_private->xwindow,
		    gc_private->xgc, offx + x, offy + y, expand16, len);
    }
    if (overstrike)
      XDrawString16(drawable_private->xdisplay, drawable_private->xwindow,
		    gc_private->xgc, offx + x + 1, offy + y, expand16, len);
  }
  break;
  /* this is limited to 65535 characters! */
  case ZVT_FONT_FONTSET: {
    char *expand = zp->text_expand;
    XFontSet fontset = (XFontSet) font_private->xfont;

    for (i=0;i<len;i++) {
      expand[i] = VT_ASCII(line->data[i+col]) & 0xff;
    }

    /* render wide characters, with fill if we can */
    if (dofill) {
      XmbDrawImageString(drawable_private->xdisplay, drawable_private->xwindow,
                       fontset, gc_private->xgc, offx + x, offy + y, expand, len);
    } else {
      XmbDrawString(drawable_private->xdisplay, drawable_private->xwindow,
                    fontset, gc_private->xgc, offx + x, offy + y, expand, len);
    }
    if (overstrike)
      XmbDrawString(drawable_private->xdisplay, drawable_private->xwindow,
                  fontset, gc_private->xgc, offx + x + 1, offy + y, expand, len);
  }
  }

  /* check for underline */
  if (attr&VTATTR_UNDERLINE) {
    gdk_draw_line(widget->window, fgc,
		  offx + x,
		  offy + y,
		  offx + (col + len) * term->charwidth - 1,
		  offy + y);
  }
  
  if (overstrike && zp && zp->bold_save) {
    gdk_draw_pixmap(GTK_WIDGET(term)->window,
		    term->fore_gc,
		    zp->bold_save,
		    0, 0,
		    x + offx + len*term->charwidth, offy + row*term->charheight,
		    1, term->charheight);
  }
#ifdef ZVT_IM_ON_THE_SPOT
  if (len <= MB_CUR_MAX)
      zvt_im_preedit_set_spot(term, col, row, offx, offy);
#endif
}





void
vt_scroll_area(void *user_data, int firstrow, int count, int offset, int fill)
{
  int width, offx, offy;
  ZvtTerm *term;
  GtkWidget *widget;
  GdkEvent *event;
  GdkGC *gc;
  struct _zvtprivate *zp;
  
  widget = user_data;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  if (!GTK_WIDGET_DRAWABLE (widget)) {
    return;
  }

  term = ZVT_TERM (widget);
  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  d(printf("scrolling %d rows from %d, by %d lines, fill=%d\n",
	  count,firstrow,offset,fill));

  width = term->charwidth * term->vx->vt.width;
  offx = widget->style->klass->xthickness + PADDING;
  offy = widget->style->klass->ythickness;

  /* "scroll" area */
  gdk_draw_pixmap(widget->window,
		  term->scroll_gc, /* must use this to generate expose events */
		  widget->window,
		  offx, offy+(firstrow+offset)*term->charheight,
		  offx, offy+firstrow*term->charheight,
		  width, count*term->charheight);

  /* clear the other part of the screen */
  /* check fill colour is right */
  if (fill >= 17) {
    gc=term->back_gc;
  } else {
    gc=term->fore_gc;
    if (term->fore_last != fill) {
      GdkColor pen;
      
      pen.pixel = term->colors[fill];
      gdk_gc_set_foreground (term->fore_gc, &pen);
      term->fore_last = fill;
    }
  }

  /* this does a 'scrolling' pixmap */
  if (term->transparent || term->pixmap_filename) {
    zp->scroll_position = (zp->scroll_position + offset*term->charheight) % term->background.h;
    gdk_gc_set_ts_origin (term->back_gc, 0, -zp->scroll_position);
  }

  /* fill the exposed area with blank */
  if (offset > 0) {
    gdk_draw_rectangle(widget->window,
		       gc,
		       1,
		       offx, offy+(firstrow+count)*term->charheight,
		       term->vx->vt.width*term->charwidth, offset*term->charheight);
  } else {
    gdk_draw_rectangle(widget->window,
		       gc,
		       1,
		       offx, offy+(firstrow+offset)*term->charheight,
		       term->vx->vt.width*term->charwidth, (-offset)*term->charheight);
  }

  /* fix up the screen display after a scroll - ensures all expose events handled
     before continuing. */
  while ((event = gdk_event_get_graphics_expose (widget->window)) != NULL) {
    gtk_widget_event (widget, event);
    if (event->expose.count == 0) {
      gdk_event_free (event);
      break;
    }
    gdk_event_free (event);
  }
}

/**
 * zvt_term_set_del_key_swap:
 * @term:   A &ZvtTerm widget.
 * @state:  If true it swaps the del/backspace definitions
 * 
 * Sets the mode for interpreting the DEL and Backspace keys.
 **/
void
zvt_term_set_del_key_swap (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  
  term->swap_del_key = state != 0;
}

/**
 * zvt_term_set_del_is_del:
 * @term:   A &ZvtTerm widget.
 * @state:  If true it uses DEL/^H for Delete key
 * 
 * Sets Delete code to DEL/^H or Esc[3~ sequences.
 **/
void
zvt_term_set_del_is_del (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  
  term->del_is_del = state != 0;
}

/*
 * zvt_term_bell:
 * @term: Terminal.
 *
 * Generate a terminal bell.  Currently this is just a beep.
 **/
void
zvt_term_bell(void *user_data)
{
  gdk_beep();
}


/**
 * zvt_term_set_bell:
 * @term: A &ZvtTerm widget.
 * @state: New bell state.
 * 
 * Enable or disable the terminal bell.  If @state is %TRUE, then the
 * bell is enabled.
 **/
void
zvt_term_set_bell(ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  if (state)
    term->vx->vt.ring_my_bell = zvt_term_bell;
  else
    term->vx->vt.ring_my_bell = 0;
}

/**
 * zvt_term_get_bell:
 * @term: A &ZvtTerm widget.
 * 
 * get the terminal bell state.  If the bell on then %TRUE is
 * returned, otherwise %FALSE.
 **/
gboolean
zvt_term_get_bell(ZvtTerm *term)
{
  g_return_val_if_fail (term != NULL, 0);
  g_return_val_if_fail (ZVT_IS_TERM (term), 0);

  return (term->vx->vt.ring_my_bell)?TRUE:FALSE;
}

void
zvt_term_set_shadow_type(ZvtTerm  *term, GtkShadowType type)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  term->shadow_type = type;

  if (GTK_WIDGET_VISIBLE (term))
    gtk_widget_queue_resize (GTK_WIDGET (term));
}

/**
 * zvt_term_get_capabilities:
 * @term: A &ZvtTerm widget.
 * 
 * Description: Gets the compiled in capabilities of the terminal widget.
 *
 * %ZVT_TERM_PIXMAP_SUPPORT; Pixmaps can be loaded into the background
 * using the background setting function.
 *
 * %ZVT_TERM_PIXMAPSCROLL_SUPPORT; The background scrolling flag of the
 * background setting function is honoured.
 *
 * %ZVT_TERM_EMBOLDEN_SUPPORT; Bold fonts are autogenerated, and can
 * be requested by setting the bold_font of the font setting function
 * to NULL.
 *
 * %ZVT_TERM_MATCH_SUPPORT; The zvt_term_add_match() functions exist,
 * and can be used to receive the match_clicked signal when the user
 * clicks on matching text.
 *
 * %ZVT_TERM_TRANSPARENCY_SUPPORT; A transparent background can be
 * requested on the current display.
 *
 * Returns: a bitmask of the capabilities
 **/
guint32
zvt_term_get_capabilities (ZvtTerm *term)
{
  Atom prop, prop2;
  guint32 out = ZVT_TERM_EMBOLDEN_SUPPORT|ZVT_TERM_PIXMAPSCROLL_SUPPORT|
    ZVT_TERM_MATCH_SUPPORT;

  /* pixmap and transparency support */
  if (gdk_imlib_get_visual () == gtk_widget_get_default_visual ())
    out |= ZVT_TERM_PIXMAP_SUPPORT;

  /* check if we really have transparency support - i think this works MPZ */
  prop = XInternAtom(GDK_DISPLAY(), "_XROOTPMAP_ID", True);
  prop2 = XInternAtom(GDK_DISPLAY(), "_XROOTCOLOR_PIXEL", True);  
  if (prop != None || prop2 != None)
    out |= ZVT_TERM_TRANSPARENCY_SUPPORT;

  return out;
}

/**
 * zvt_term_get_buffer:
 * @term: Valid &ZvtTerm widget.
 * @len: Placeholder to store the length of text selected.  May be
 *       %NULL in which case the value is not returned.
 * @type: Type of selection.  %VT_SELTYPE_LINE, select by line,
 *       %VT_SELTYPE_WORD, select by word, or %VT_SELTYPE_CHAR, select
 *       by character.
 * @sx: Start of selection, horizontal.
 * @sy: Start of selection, vertical.  0 is the top of the visible
 *      screen, <0 is scrollback lines, >0 is visible lines (upto the
 *      height of the window).
 * @ex: End of selection, horizontal.
 * @ey: End of selection, vertical, as above.
 * 
 * Convert the buffer memory into a contiguous array which may be
 * saved or processed.  Note that this is not gauranteed to match the
 * order of characters processed by the terminal, only the order in
 * which they were displayed.  Tabs will normally be preserved in
 * the output.
 *
 * All inputs are range-checked first, so it is possible to fudge
 * a full buffer grab.
 *
 * Examples:
 *  data = zvt_term_get_buffer(term, NULL, VT_SELTYPE_LINE,
 *       -term->vx->vt.scrollbackmax, 0,
 *        term->vx->vt.height, 0);
 *  or, as a rule -
 *  data = zvt_term_get_buffer(term, NULL, VT_SELTYPE_LINE,
 *       -10000, 0, 10000, 0);
 *
 * Will return the contents of the entire scrollback and on-screen
 * buffers, remembering that all inputs are range-checked first.
 *
 *  data = zvt_term_get_buffer(term, NULL, VT_SELTYPE_CHAR,
 *       0, 0, 5, 10);
 *
 * Will return the first 5 lines of the visible screen, and the 6th
 * line upto column 10.
 * 
 * Return value: A pointer to a %NUL terminated buffer containing the
 * raw text from the buffer.  If memory could not be allocated, then
 * returns %NULL.  Note that it is upto the caller to free the memory,
 * using g_free(3c).  If @len was supplied, then the length of data is
 * stored there.
 **/
char *
zvt_term_get_buffer(ZvtTerm *term, int *len, int type, int sx, int sy, int ex, int ey)
{
  struct _vtx *vx;
  int ssx, ssy, sex, sey, stype, slen;
  uint32 *sdata;
  char *data;

  g_return_val_if_fail (term != NULL, 0);
  g_return_val_if_fail (ZVT_IS_TERM (term), 0);

  vx = term->vx;

  /* this is a bit messy - we save the current selection state,
   * override it, 'select' the new text, then restore the old
   *  selection state but return the new ...
   *  api should be more separated.  vt_get_block() is a start on this.
   */

  ssx = vx->selstartx;
  ssy = vx->selstarty;
  sex = vx->selendx;
  sey = vx->selendy;
  sdata = vx->selection_data;
  slen = vx->selection_size;
  stype = vx->selectiontype;

  vx->selstartx = sx;
  vx->selstarty = sy;
  vx->selendx = ex;
  vx->selendy = ey;
  vx->selection_data = 0;
  vx->selectiontype = type & VT_SELTYPE_MASK;

  vt_fix_selection(vx);

  /* this always currently gets the data as bytes */
  data = vt_get_selection(vx, 1, len);
  
  vx->selstartx = ssx;
  vx->selstarty = ssy;
  vx->selendx = sex;
  vx->selendy = sey;
  vx->selection_data = sdata;
  vx->selection_size = slen;
  vx->selectiontype = stype;

  return data;
}

/******************************/
/**** TRANSPARENT TERMINAL ****/


/* kind of stolen from Eterm and needs heavy cleanup */
static Window desktop_window = None;

static Window
get_desktop_window (Window the_window)
{
  Atom prop, type, prop2;
  int format;
  unsigned long length, after;
  unsigned char *data;
  unsigned int nchildren;
  Window w, root, *children, parent;
  
  prop = XInternAtom(GDK_DISPLAY(), "_XROOTPMAP_ID", True);
  prop2 = XInternAtom(GDK_DISPLAY(), "_XROOTCOLOR_PIXEL", True);
  
  if (prop == None && prop2 == None)
    return None;
  
#ifdef WATCH_DESKTOP_OPTION
  if (Options & Opt_watchDesktop)  {
    if (TermWin.wm_parent != None)
      XSelectInput(GDK_DISPLAY(), TermWin.wm_parent, None);
    
    if (TermWin.wm_grandparent != None)
      XSelectInput(GDK_DISPLAY(), TermWin.wm_grandparent, None);
  }
#endif
  
  for (w = the_window; w; w = parent) {
    if ((XQueryTree(GDK_DISPLAY(), w, &root, &parent, &children, &nchildren)) == False) 
      return None;
      
    if (nchildren) 
      XFree(children);

#ifdef WATCH_DESKTOP_OPTION
    if (Options & Opt_watchDesktop) {
      if (w == TermWin.parent) {
	TermWin.wm_parent = parent;
	XSelectInput(GDK_DISPLAY(), TermWin.wm_parent, StructureNotifyMask);
      } else if (w == TermWin.wm_parent) {
	TermWin.wm_grandparent = parent;
	XSelectInput(GDK_DISPLAY(), TermWin.wm_grandparent, StructureNotifyMask);
      }
    }
#endif
    
    if (prop != None) {
      XGetWindowProperty(GDK_DISPLAY(), w, prop, 0L, 1L, False, AnyPropertyType,
			 &type, &format, &length, &after, &data);
    } else if (prop2 != None) {
      XGetWindowProperty(GDK_DISPLAY(), w, prop2, 0L, 1L, False, AnyPropertyType,
			 &type, &format, &length, &after, &data);
    } else  {
      continue;
    }
    
    if (type != None) {
      return (desktop_window = w);
    }
  }
  
  return (desktop_window = None);
}

static Pixmap
get_pixmap_prop (Window the_window, char *prop_id)
{
  Atom prop, type;
  int format;
  unsigned long length, after;
  unsigned char *data;
  
  /*this should be changed when desktop changes I guess*/
  if(desktop_window == None)
    desktop_window = get_desktop_window(the_window);
  if(desktop_window == None)
    desktop_window = GDK_ROOT_WINDOW();
  
  prop = XInternAtom(GDK_DISPLAY(), prop_id, True);
  
  if (prop == None)
    return None;
  
  XGetWindowProperty(GDK_DISPLAY(), desktop_window, prop, 0L, 1L, False,
		     AnyPropertyType, &type, &format, &length, &after,
		     &data);

  if (type == XA_PIXMAP)
    return *((Pixmap *)data);

  return None;
}

static GdkPixmap *
load_pixmap_back(char *file, int shaded)
{
  GdkPixmap *pix;
  GdkImlibColorModifier mod;
  GdkImlibImage *iim;
  
  if(!file)
    return NULL;
  
  iim = gdk_imlib_load_image(file);
  if(!iim)
    return NULL;
  mod.contrast=256;
  mod.gamma=256;
  if(shaded)
    mod.brightness=190;
  else
    mod.brightness=256;
  
  gdk_imlib_set_image_modifier(iim,&mod);
  gdk_imlib_render(iim, iim->rgb_width,iim->rgb_height);
  pix = gdk_imlib_move_image(iim);
  gdk_imlib_destroy_image(iim);
  
  return pix;
}

static GdkPixmap *
create_shaded_pixmap (Pixmap p, int x, int y, int w, int h)
{
  GdkPixmap *pix,*pp,*tmp;
  GdkImlibColorModifier mod;
  GdkImlibImage *iim;
  GdkGC *tgc;
  int width,height,depth;
  
  if (p == None)
    return NULL;

  pp = gdk_pixmap_foreign_new(p);
  gdk_window_get_geometry(pp,NULL,NULL,&width,&height,&depth);

  if (width<x+w || height<y+h || x<0 || y<0) {
    tgc = gdk_gc_new(pp);
    tmp = gdk_pixmap_new(pp,w,h,depth);
    
    gdk_gc_set_tile (tgc, pp);
    gdk_gc_set_fill (tgc, GDK_TILED);
    gdk_gc_set_ts_origin(tgc, -x, -y);
    gdk_draw_rectangle (tmp, tgc, TRUE, 0, 0, w, h);
    gdk_gc_destroy(tgc);
    
    iim = gdk_imlib_create_image_from_drawable(tmp,
					       NULL, 0, 0, w, h);
    gdk_pixmap_unref(tmp);
  } else {
    iim = gdk_imlib_create_image_from_drawable(pp,
					       NULL, x, y, w, h);
  }
  gdk_xid_table_remove (GDK_WINDOW_XWINDOW(pp));
  g_dataset_destroy (pp);
  g_free (pp);

  if(!iim)
    return NULL;

  mod.contrast = 256;
  mod.brightness = 190;
  mod.gamma = 256;
  gdk_imlib_set_image_modifier (iim, &mod);
  gdk_imlib_render (iim, iim->rgb_width, iim->rgb_height);
  pix = gdk_imlib_move_image (iim);
  gdk_imlib_destroy_image (iim);
  
  return pix;
}

/* sets the selected pixmap as our background fill */
static void
load_background (ZvtTerm *widget)
{
  int x,y;
  Window childret;
  ZvtTerm *term;
  GdkGC *bgc;
  Pixmap p;
  int width,height;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);
  bgc = term->back_gc;

  if (bgc == 0)
    return;

  if (term->pixmap_filename==0 && !term->transparent) {
    GdkColor pen;
    gdk_gc_set_fill (bgc, GDK_SOLID);
    pen.pixel = term->colors[17];
    gdk_gc_set_foreground (bgc, &pen);
    gdk_gc_set_background (term->fore_gc, &pen);
    if (term->background.pix) {
      gdk_pixmap_unref(term->background.pix);
      term->background.pix = 0;
    }
    return;
  }

  zp = gtk_object_get_data (GTK_OBJECT (term), "_zvtprivate");

  /* we will be doing a background pixmap, not transparency */
  if (term->pixmap_filename) {

    if (!term->background.pix) {
      term->background.pix =
	load_pixmap_back (term->pixmap_filename, term->shaded);
      /* must reset origin */
      gdk_gc_set_ts_origin(bgc,0, 0);
    }

    if (!term->background.pix && !term->transparent) {
      GdkColor pen;
      gdk_gc_set_fill (bgc, GDK_SOLID);
      pen.pixel = term->colors[17];
      gdk_gc_set_foreground (bgc, &pen);
      g_free (term->pixmap_filename);
      term->pixmap_filename = NULL;
      gdk_gc_set_fill (bgc, GDK_SOLID);
      return;
    } else if (term->background.pix) {
      GdkWindowPrivate *wp = (GdkWindowPrivate *)term->background.pix;

      zp->transpix = FALSE;

      term->background.w = wp->width;
      term->background.h = wp->height;
      term->background.x = wp->x;
      term->background.y = wp->y;

      d(printf("height = %d\n", wp->height));

      gdk_gc_set_tile (bgc, term->background.pix);
      gdk_gc_set_ts_origin (term->back_gc, 0, -zp->scroll_position);
      gdk_gc_set_fill (bgc, GDK_TILED);
      return;
    }
 
    /* if we can't load a pixmap and transparency is set,
     * default to the transparency setting
     */
  }
  
  p = get_pixmap_prop (GDK_WINDOW_XWINDOW(GTK_WIDGET(widget)->window), "_XROOTPMAP_ID");
  if (p == None)
    goto failure;
  
  XTranslateCoordinates (
      GDK_WINDOW_XDISPLAY (GTK_WIDGET(widget)->window),
      GDK_WINDOW_XWINDOW (GTK_WIDGET(widget)->window),
      GDK_ROOT_WINDOW (),
      0, 0,
      &x, &y,
      &childret);

  gdk_window_get_size(GTK_WIDGET(widget)->window, &width, &height);
  
  /* dont update if we're actually not visible? */
  if (x<-width || y<-height) {
    d(printf("oi, you can't see me, why ask me to redraw!?\n"));
    return;
  }

  if ((term->background.pix == NULL && term->shaded)
      || (zp->transpix == FALSE && !term->shaded) /* FIXFIX */
      || term->background.x != x
      || term->background.y != y
      || term->background.w != width
      || term->background.h != height) {

    term->background.x = x;
    term->background.y = y;
    term->background.w = width;
    term->background.h = height;

    /* free any pixmaps we already have */
    if (term->background.pix) {
      gdk_pixmap_unref(term->background.pix);
      term->background.pix = 0;
    }

    if (term->shaded) {

      /* In theory, gdk_imlib_create_image_from_drawable should
       * catch most errors, but it doesn't look 100% reliable
       */
      gdk_error_trap_push();
      term->background.pix = create_shaded_pixmap (p, x, y, width, height);
      if (gdk_error_trap_pop() || !term->background.pix)
	goto failure;
      
      gdk_gc_set_ts_origin (term->back_gc, 0, 0);
      gdk_gc_set_tile (bgc, term->background.pix);

      gdk_gc_set_fill (bgc, GDK_TILED);
    } else {
      d(printf("loading background at %d,%d\n", x, y));

      /* We unconditionally set this TRUE, instead of setting
       * it after the operation succeeds so we don't waste
       * too much effort trying to access unaccessable pixmaps
       */
      zp->transpix = TRUE;
      
      /*non-shaded is simple?*/
      /* Luckily XSetTile always flushes the GC Cache, so if
       * this and the following XSync() succeed, we are safe.
       */
      gdk_error_trap_push();
      XSetTile (GDK_DISPLAY(), GDK_GC_XGC (bgc), p);
      XSync (GDK_DISPLAY(), False);
      if (gdk_error_trap_pop())
	goto failure;

      gdk_gc_set_ts_origin (bgc,-x,-y);
      gdk_gc_set_fill (bgc, GDK_TILED);
    }
  } else {
    d(printf("background hasn't moved, leaving\n"));
  }
  return;
  
 failure:
  {
    GdkColor pen;
    term->transparent = 0;
    gdk_gc_set_fill (bgc, GDK_SOLID);
    pen.pixel = term->colors[17];
    gdk_gc_set_foreground (term->back_gc, &pen);
    return;
  }

}

static gint
safe_strcmp(gchar *a, gchar *b)
{
  if (!a && !b)
    return 0;

  if (a && b)
    return strcmp(a,b);

  return 1;
}
