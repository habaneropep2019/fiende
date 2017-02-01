#ifndef GNOME_EXCLUDE_DEPRECATED
#ifndef __GTK_XMHTML_H__
#define __GTK_XMHTML_H__

#include <gdk/gdk.h>
#include <gtk/gtkobject.h>

#ifndef GTK_XMHTML_LIBRARY
#ifndef WITH_GTK
# define WITH_GTK
#endif

#include <gtk-xmhtml/toolkit.h>
#include <gtk-xmhtml/XmHTML.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_XMHTML(obj)          GTK_CHECK_CAST (obj, gtk_xmhtml_get_type (), GtkXmHTML)
#define GTK_XMHTML_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_xmhtml_get_type (), GtkXmHTMLClass)
#define GTK_IS_XMHTML(obj)       GTK_CHECK_TYPE (obj, gtk_xmhtml_get_type ())

/* For compatibility and consistency functions */
#define XmIsHTML(obj)             GTK_IS_XMHTML(obj)
#define XmHTML(obj)               GTK_XMHTML(obj)

typedef struct _GtkXmHTML       GtkXmHTML;
typedef struct _GtkXmHTMLClass  GtkXmHTMLClass;

#ifndef GTK_XMHTML_LIBRARY
#include <gtk-xmhtml/XmHTMLP.h>
#endif

struct _GtkXmHTML
{
	GtkContainer widget;

	XmHTMLPart html;

	/* Scrollbar adjustements */
	GtkObject *vsba;
	GtkObject *hsba;

	/* managing our code */
	int frozen;
	int parse_needed;
	int reformat_needed;
	int redraw_needed;
	int free_images_needed;
	int layout_needed;

	int initialized;
	GList *children;

	/* These replace Motif's fields */

	gulong  background_pixel;
	gulong  foreground_pixel;
	GdkGC  *bottom_shadow_gc;
	GdkGC  *top_shadow_gc;
	GdkGC  *highlight_gc;
	gulong  highlight_color;
};

struct _GtkXmHTMLClass
{
	GtkContainerClass parent_class;

	void (*activate)        (GtkXmHTML *, void *);
	void (*rearm)           (GtkXmHTML *, void *);
	void (*anchor_track)    (GtkXmHTML *, void *);
	void (*frame)           (GtkXmHTML *, void *);
	void (*form)            (GtkXmHTML *, void *);
	void (*input)           (GtkXmHTML *, void *);
	void (*link)            (GtkXmHTML *, void *);
	void (*motion)          (GtkXmHTML *, void *);
	void (*imagemap)        (GtkXmHTML *, void *);
	void (*document)        (GtkXmHTML *, void *);
	void (*focus)           (GtkXmHTML *, void *);
	void (*losing_focus)    (GtkXmHTML *, void *);
	void (*motion_track)    (GtkXmHTML *, void *);
	void (*html_event)      (GtkXmHTML *, void *); /* HTML 4.0 event */
	int  (*anchor_visited)  (GtkXmHTML *, char *, void *);
};

GtkWidget *gtk_xmhtml_new         	   	  (void);
void gtk_xmhtml_freeze       	   	     	  (GtkXmHTML *html);
void gtk_xmhtml_thaw         	   	     	  (GtkXmHTML *html);
void gtk_xmhtml_source       	   	     	  (GtkXmHTML *html,
						   char *source);
void gtk_xmhtml_set_string_direction  	     	  (GtkXmHTML *html,
						   int direction);
void gtk_xmhtml_set_alignment         	     	  (GtkXmHTML *html,
						   int alignment);
void gtk_xmhtml_outline               	     	  (GtkXmHTML *html,
					   	   int flag);
void gtk_xmhtml_set_font_familty                  (GtkXmHTML *html,
						   char *family,
						   char *sizes);
void gtk_xmhtml_set_font_familty_fixed            (GtkXmHTML *html,
						   char *family,
						   char *sizes);
void gtk_xmhtml_set_font_charset                  (GtkXmHTML *html,
						   char *charset);
void gtk_xmhtml_set_allow_body_colors             (GtkXmHTML *html,
						   int enable);
void gtk_xmhtml_set_hilight_on_enter              (GtkXmHTML *html,
						   int flag);
void gtk_xmhtml_set_anchor_underline_type         (GtkXmHTML *html,
						   int underline_type);
void gtk_xmhtml_set_anchor_visited_underline_type (GtkXmHTML *html,
						   int underline_type);
void gtk_xmhtml_set_anchor_target_underline_type  (GtkXmHTML *html,
						   int underline_type);
void gtk_xmhtml_set_allow_color_switching         (GtkXmHTML *html,
						   int flag);
void gtk_xmhtml_set_dithering                     (GtkXmHTML *html,
						   XmHTMLDitherType flag);
void gtk_xmhtml_set_allow_font_switching          (GtkXmHTML *html,
						   int flag);
void gtk_xmhtml_set_max_image_colors              (GtkXmHTML *html,
						   int max_colors);
void gtk_xmhtml_set_allow_images                  (GtkXmHTML *html,
						   int flag);
void gtk_xmhtml_set_plc_intervals                 (GtkXmHTML *html, 
						   int min_delay,
						   int max_delay,
						   int def_delay);
void gtk_xmhtml_set_def_body_image_url            (GtkXmHTML *html,
						   char *url);
void gtk_xmhtml_set_anchor_buttons                (GtkXmHTML *html,
						   int flag);
void gtk_xmhtml_set_anchor_cursor                 (GtkXmHTML *html,
						   GdkCursor * cursor,
						   int flag);
void gtk_xmhtml_set_topline                       (GtkXmHTML *html,
						   int line);
int gtk_xmhtml_get_topline                        (GtkXmHTML *html);
void gtk_xmhtml_set_freeze_animations             (GtkXmHTML *html,
						   int flag);
char *gtk_xmhtml_get_source                       (GtkXmHTML *html);
void gtk_xmhtml_set_screen_gamma                  (GtkXmHTML *html,
						   float     gamma);
void gtk_xmhtml_set_image_procs                   (GtkXmHTML         *html,
						   XmImageProc       image_proc,
						   XmImageGifProc    gif_proc,
						   XmHTMLGetDataProc get_data,
						   XmHTMLEndDataProc end_data);
void gtk_xmhtml_set_event_proc                    (GtkXmHTML         *html,
						   XmHTMLEventProc   event_proc);
void gtk_xmhtml_set_perfect_colors                (GtkXmHTML *html, int flag);
void gtk_xmhtml_set_uncompress_command            (GtkXmHTML *html, char *cmd);
void gtk_xmhtml_set_strict_checking               (GtkXmHTML *html, int flag);
void gtk_xmhtml_set_bad_html_warnings             (GtkXmHTML *html, int flag);
void gtk_xmhtml_set_allow_form_coloring           (GtkXmHTML *html, int flag);
void gtk_xmhtml_set_imagemap_draw                 (GtkXmHTML *html, int flag);
void gtk_xmhtml_set_mime_type                     (GtkXmHTML *html, char *mime_type);
void gtk_xmhtml_set_alpha_processing              (GtkXmHTML *html, int flag);
void gtk_xmhtml_set_rgb_conv_mode                 (GtkXmHTML *html, int val);

void Toolkit_Draw_Shadows(XmHTMLWidget w, TGC top_shadow, TGC bottom_shadow,
			  gint xs, gint ys, gint xe, gint ye, gint xxx, gint shadow_type);


/* These ones are used internally: */
void  gtk_xmhtml_set_geometry (GtkWidget *widget, int x, int y, int width, int height);
guint gtk_xmhtml_get_type (void);
void  gtk_xmhtml_manage (GtkContainer *container, GtkWidget *widget);

enum {
	GTK_ANCHOR_NOLINE,
	GTK_ANCHOR_SINGLE_LINE,
	GTK_ANCHOR_DOUBLE_LINE,
	GTK_ANCHOR_DASHED_LINE,
	GTK_ANCHOR_DOUBLE_DASHED_LINE
};

/* For compatibility with the Motif sources */
typedef struct {
	int reason;
	GdkEvent *event;
} gtk_xmhtml_callback_info;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
#endif /* GNOME_EXCLUDE_DEPRECATED */
