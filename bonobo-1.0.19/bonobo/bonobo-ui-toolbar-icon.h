/*  -*- Mode: C; c-set-style: linux; indent-tabs-mode: nil; c-basic-offset: 8 -*-

   Copyright (C) 1999, 2000 Red Hat, Inc.
    
   Authors: Havoc Pennington, Jonathan Blandford
*/

#ifndef BONOBO_UI_TOOLBAR_ICON_H
#define BONOBO_UI_TOOLBAR_ICON_H

#include <gtk/gtkmisc.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libgnome/gnome-defs.h"

BEGIN_GNOME_DECLS

typedef enum {
        /* update struct when adding enum values */
	BONOBO_UI_TOOLBAR_ICON_SIMPLE, /* No alpha blending */
	BONOBO_UI_TOOLBAR_ICON_COLOR   /* */
} BonoboUIToolbarIconDrawMode;

#define BONOBO_TYPE_UI_TOOLBAR_ICON            (bonobo_ui_toolbar_icon_get_type ())
#define BONOBO_UI_TOOLBAR_ICON(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR_ICON, BonoboUIToolbarIcon))
#define BONOBO_UI_TOOLBAR_ICON_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR_ICON, BonoboUIToolbarIconClass))
#define BONOBO_IS_UI_TOOLBAR_ICON(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_ICON))
#define BONOBO_IS_UI_TOOLBAR_ICON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BONOBO_TYPE_UI_TOOLBAR_ICON))
#define BONOBO_UI_TOOLBAR_ICON_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), BONOBO_TYPE_UI_TOOLBAR_ICON, BonoboUIToolbarIconClass))


typedef struct {
	GtkMisc misc;

        /* NOTE. In the old BonoboUIToolbarIcon, _lots_ of people were using BonoboUIToolbarIcon to
	 * load images, sucking out the pixmap field, and then not using the
	 * BonoboUIToolbarIcon as a widget at all. IF YOU DO THIS I WILL PERSONALLY
	 * KICK YOUR ASS. Use gdk_pixbuf_new_from_file(). Thank you.
	 * These are PRIVATE FIELDS which I will gratuitously change just to
	 * break your broken code.
	 *                          -  hp + jrb + quartic + Jesse Ventura + GW Bush 
	 */

	GdkPixbuf *provided_image;

        struct {
                GdkPixbuf *pixbuf;
                GdkBitmap *mask;
		gfloat saturation;
		gboolean pixelate;
        } provided[5]; /* the five states */

	GdkPixbuf *generated_scaled_image;
	GdkBitmap *generated_scaled_mask;
        
        struct {
                GdkPixbuf *pixbuf;
                GdkBitmap *mask;
        } generated[5]; /* the five states */
        
	gint width, height;
	gint alpha_threshold;

	BonoboUIToolbarIconDrawMode mode : 2;
} BonoboUIToolbarIcon;


typedef struct {
	GtkMiscClass parent_class;
} BonoboUIToolbarIconClass;

guint  bonobo_ui_toolbar_icon_get_type  (void) G_GNUC_CONST;

GtkWidget *bonobo_ui_toolbar_icon_new                      (void);
GtkWidget *bonobo_ui_toolbar_icon_new_from_file            (const gchar  *filename);
GtkWidget *bonobo_ui_toolbar_icon_new_from_file_at_size    (const gchar  *filename,
                                                            gint          width,
                                                            gint          height);
GtkWidget *bonobo_ui_toolbar_icon_new_from_xpm_d           (const gchar **xpm_data);
GtkWidget *bonobo_ui_toolbar_icon_new_from_xpm_d_at_size   (const gchar **xpm_data,
                                                            gint          width,
                                                            gint          height);
GtkWidget *bonobo_ui_toolbar_icon_new_from_pixbuf          (GdkPixbuf    *pixbuf);
GtkWidget *bonobo_ui_toolbar_icon_new_from_pixbuf_at_size  (GdkPixbuf    *pixbuf,
                                                            gint          width,
                                                            gint          height);

void       bonobo_ui_toolbar_icon_set_pixbuf_size  (BonoboUIToolbarIcon *gpixmap,
                                                    gint                 width,
                                                    gint                 height);
void       bonobo_ui_toolbar_icon_get_pixbuf_size  (BonoboUIToolbarIcon *gpixmap,
                                                    gint                *width,
                                                    gint                *height);

void       bonobo_ui_toolbar_icon_set_pixbuf       (BonoboUIToolbarIcon *gpixmap,
                                                    GdkPixbuf           *pixbuf);
GdkPixbuf *bonobo_ui_toolbar_icon_get_pixbuf       (BonoboUIToolbarIcon *gpixmap);
   
/* Sets the individual states, instead of generating them. */
void  bonobo_ui_toolbar_icon_set_pixbuf_at_state  (BonoboUIToolbarIcon *gpixmap,
                                                   GtkStateType         state,
                                                   GdkPixbuf           *pixbuf,
                                                   GdkBitmap           *mask);
void  bonobo_ui_toolbar_icon_set_state_pixbufs    (BonoboUIToolbarIcon *gpixmap,
                                                   GdkPixbuf           *pixbufs[5],
                                                   GdkBitmap           *masks[5]);
void  bonobo_ui_toolbar_icon_set_draw_vals        (BonoboUIToolbarIcon *gpixmap,
                                                   GtkStateType         state,
                                                   gfloat               saturation,
                                                   gboolean             pixelate);
void  bonobo_ui_toolbar_icon_get_draw_vals        (BonoboUIToolbarIcon *gpixmap,
                                                   GtkStateType         state,
                                                   gfloat              *saturation,
                                                   gboolean            *pixelate);

void                         bonobo_ui_toolbar_icon_set_draw_mode            (BonoboUIToolbarIcon          *gpixmap,
                                                                              BonoboUIToolbarIconDrawMode   mode);
BonoboUIToolbarIconDrawMode  bonobo_ui_toolbar_icon_get_draw_mode            (BonoboUIToolbarIcon          *gpixmap);

void  bonobo_ui_toolbar_icon_clear                (BonoboUIToolbarIcon *gpixmap);
void  bonobo_ui_toolbar_icon_set_alpha_threshold  (BonoboUIToolbarIcon *gpixmap,
                                                   gint                 alpha_threshold);
gint  bonobo_ui_toolbar_icon_get_alpha_threshold  (BonoboUIToolbarIcon *gpixmap);


END_GNOME_DECLS

#endif /* __BONOBO_UI_TOOLBAR_ICON_H__ */
