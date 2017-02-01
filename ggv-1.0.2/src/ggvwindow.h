/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#ifndef __GGV_WINDOW_H__
#define __GGV_WINDOW_H__

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include "crop.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  size of the array used to specify to specify data */
#define MENU_DATA_SIZE 40
#define MENU_ZOOM_SIZE 9 /* Number of zoom options  */
#define MENU_ORIENTATION_SIZE 4

enum {
        TOGGLE_CURRENT_PAGE = -1,
        TOGGLE_EVEN_PAGES = -2,
        TOGGLE_ODD_PAGES = -3,
        TOGGLE_ALL_PAGES = -4,
        UNMARK_ALL_PAGES = -5,
};

enum {
        TARGET_URI_LIST,
};

typedef struct _window_with_data window_with_data;
typedef struct _ggv_window ggv_window;
typedef struct _ggv_prefs ggv_prefs;

/* global variables */
extern GdkCursor *pan_cursor;
extern GList     *window_list;
extern int       active_ggv;
extern ggv_prefs prefs_dialog;
extern GList     *recent_files_list;
extern gint      ZoomMenuMagnificationSteps[];
extern char      *OrientationLabels[];

/* prototypes */
gboolean load_gs(ggv_window *ggv, char *fname);
ggv_window *create_ggv_window(void);
void create_sidebar(ggv_window *retval);
void create_menus(ggv_window *retval, window_with_data *temp);
void create_toolbar(ggv_window *retval);
void create_popup_menus(ggv_window *ggv);
ggv_window *open_window(gchar *filename, gint x, gint y, gint w, gint h);
void close_window(ggv_window *ggv);
void set_gs_prefs(ggv_window *ggv);
void apply_gs_prefs(GList *windows);
void open_prefs_dialog(ggv_prefs *pd);
void goto_page(ggv_window *ggv, int page);
void set_page_sensitivities(ggv_window *ggv, int page);
void flash_message(ggv_window *ggv, gchar *flash);
void error_message(ggv_window *ggv, gchar *errormsg);
void save_marked_pages(ggv_window *ggv, gchar *file);
void synchronize_user_interface(ggv_window *ggv);

struct _window_with_data {
        ggv_window *ggv;
        gpointer data;
};

struct _ggv_window {
	GtkWidget *main_window;         /* main window */
	GtkWidget *status, *gs_text;    /* status window */
	GtkWidget *pagelistscroll;
        GtkWidget *pagelist;
        GtkWidget *scrollpane;
        GtkWidget *popup_menu;
        GtkWidget *sidebar;
        GtkWidget *file_sel;
        GtkWidget *save_file_sel;
        GtkWidget *appbar;
        GtkObject *hadj, *vadj;
        GtkWidget *gs;
        gboolean show_menus, show_panel, show_toolbar;
        gboolean loaded;
        gboolean pan;
        gboolean pane_auto_jump;	/* ...to top of new page */
        gdouble prev_x, prev_y;
        gchar *startup_file;
        crop_data *cd;
        gint zoom_magstep;
        window_with_data menudata[MENU_DATA_SIZE];

        /* store some pointers to widgets from GnomeUIInfo trees here
           as we need them for setting sensitivity and stuff like that */
        GtkWidget *firstitem, *firstpu;  /* first-page menu item and popup menu item */
        GtkWidget *lastitem, *lastpu;  /* last-page menu item and popup menu item */
        GtkWidget *nextbutton, *nextitem, *nextpu;  /* next button, menu item and popup menu item */
        GtkWidget *prevbutton, *previtem, *prevpu;  /* prev ... */
        GtkWidget *printitem, *printmarkeditem, *reloaditem;          /* menu items... */
	GtkWidget *printbutton, *reloadbutton, *savebutton;      /* toolbar buttons */
        GtkWidget *saveitem;
        GtkWidget *recenteritem, *recenterpu;
        GtkWidget *zoominbutton, *zoominpu;                    /* zoom buttons */
        GtkWidget *zoomoutbutton, *zoomoutpu;
        GtkWidget *panel_vis, *menus_vis, *toolbar_vis;  /* menu/side panel/toolbar visibility check items in menus */
        GtkWidget *watchdoc_menu;
        GtkWidget *antialiased_menu;
        GtkWidget *override_media_menu;
        GtkWidget *override_orientation_menu;
        GtkWidget *zoom_menu[MENU_ZOOM_SIZE];
        GtkWidget *paper_menu[PAPER_SIZE_COUNT];
        GtkWidget *orientation_menu[MENU_ORIENTATION_SIZE];

        GtkWidget *panel_vis_pu, *menus_vis_pu, *toolbar_vis_pu; /* and in popup menu */
 	GtkWidget *toggleakt, *toggleall,          		 /* (un)mark pages */
 	          *toggleeven, *toggleodd, *clearall,
 	          *toggleakt_pu, *toggleall_pu,
 	          *toggleeven_pu, *toggleodd_pu, *clearall_pu,
		  /**/
		  *toggleallbutton, *clearallbutton,
		  *toggleevenbutton, *toggleoddbutton;
		  /**/
        guint num_recent;                           /* Recent menu has been intantiated? so we can
                                                       remove it before we add new entries */
};

struct _ggv_prefs
{
        GnomePropertyBox *pbox;
        GtkWidget *gs, *scan_pdf, *unzip, *unbzip2, *print;   /* entries */
        GtkWidget *media, *zoom, *orientation;                /* option menu */
        GtkWidget *watch, *aa, *override_media, *respect_eof; /* checkbuttons */
        GtkWidget *override_orientation;
        GtkWidget *tbar, *mbar, *toolbar, *savegeo,*auto_jump;/* checkbuttons */
        GtkWidget *media_choice[PAPER_SIZE_COUNT];            /* paper items */
        GtkWidget *zoom_choice[MENU_ZOOM_SIZE];               /* zoom items */
        GtkWidget *orientation_choice[MENU_ORIENTATION_SIZE]; /* orientation items */
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
