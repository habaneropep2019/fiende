/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 - 2001 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 *          Jaka Mocnik <jaka@gnu.org>
 */

#include <config.h>
#include <gnome.h>
#include <ctype.h>
#include <libgnome/gnome-history.h>

#include "gtkscrollpane.h"
#include "gtkchecklist.h"
#include "gtkgs.h"
#include "crop.h"
#include "prefs.h"
#include "stock/foot.xpm"
#include "stock/foot_white.xpm"
#include "gsmessage.h"
#include "ggvutils.h"
#include "ggvwindow.h"
#include "callbacks.h"

/* This sizes are good for the worst case: laptops :) */
enum {
        GEOMETRY_KEY = -1,
        NO_SIDE_BAR_KEY = -2,
        NO_MENU_BAR_KEY = -3,
        FULL_SCREEN_KEY = -4,
	NO_TOOL_BAR_KEY = -5,
	SPARTAN_KEY = -6,
};

#define MENU_DATA_PAPER_OFFSET 15

GdkCursor *pan_cursor = NULL;

GList *window_list = NULL;
GList *recent_files_list;
int active_ggv = 0;

ggv_prefs prefs_dialog = { NULL };

static GdkPixmap *page_marker_pixmap;
static GdkBitmap *page_marker_mask;
static GdkPixmap *page_marker_white_pixmap;
static GdkBitmap *page_marker_white_mask;

static char    *geometry = 0;
static gboolean nosidebar = FALSE;
static gint     nomenubar = FALSE;
static gboolean notoolbar = FALSE;
static gboolean fullscreen = FALSE;

/* Magnification steps for menu options */

/* The last number 10000, is just an arbitrary value never to be used */
gint ZoomMenuMagnificationSteps[MENU_ZOOM_SIZE] = {
        0, -12,-8,-4,-2,
        2  , 4, 8,0,
};

static char *ZoomMagnLabels[MENU_ZOOM_SIZE] = {
        "100%",
        "12%",
        "25%",
        "50%",
        "70%",
        "144%",
        "200%",
        "400%",
        N_("Other"),
};

void default_status_bar_message(ggv_window *ggv, gchar *message);

static void
free_wwd(window_with_data *wwd)
{
        g_free(wwd->data);
        g_free(wwd);
}



static int
get_number(const char **geometry)
{
	int value = 0;
	int mult  = 1;
	
	if(**geometry == '-'){
		mult = -1;
		(*geometry)++;
	}
	while(**geometry && isdigit(**geometry)){
		value = value * 10 + (**geometry - '0');
		(*geometry)++;
	}
	return value * mult;
}

static gboolean
ggv_parse_geometry(const gchar *geometry, 
                    gint *xpos, gint *ypos, 
                    gboolean *xneg, gboolean *yneg,
                    gint *width, 
                    gint *height)
{
	int subtract;

	g_return_val_if_fail(xpos != NULL, FALSE);
	g_return_val_if_fail(ypos != NULL, FALSE);
	g_return_val_if_fail(width != NULL, FALSE);
	g_return_val_if_fail(height != NULL, FALSE);
	g_return_val_if_fail(yneg != NULL, FALSE);
	g_return_val_if_fail(xneg != NULL, FALSE);
	
	*xpos = *ypos = *width = *height = -1;
        *xneg = FALSE; *yneg = FALSE;

	if(!geometry)
		return FALSE;

	if(*geometry == '=')
		geometry++;
	if(!*geometry)
		return FALSE;
	if(isdigit(*geometry))
		*width = get_number(&geometry);
	if(!*geometry)
		return TRUE;
	if(*geometry == 'x' || *geometry == 'X'){
		geometry++;
		*height = get_number(&geometry);
	}
	if(!*geometry)
		return 1;
	if(*geometry == '+'){
		subtract = 0;
		geometry++;
	} else if(*geometry == '-'){
		subtract = gdk_screen_width();
		geometry++;
	} else
		return FALSE;
	*xpos = get_number(&geometry);
	if(subtract) {
		*xpos = subtract - *xpos;
                *xneg = TRUE;
        }
	if(!*geometry)
		return TRUE;
	if(*geometry == '+'){
		subtract = 0;
		geometry++;
	} else if(*geometry == '-'){
		subtract = gdk_screen_height();
		geometry++;
	} else
		return FALSE;
	*ypos = get_number(&geometry);
	if(subtract) {
		*ypos = subtract - *ypos;
                *yneg = TRUE;
        }
	return TRUE;
}

static gchar *
escape_underlines(gchar *str)
{
	char *buf;
	char *p;

	buf = g_new(char, 2 * strlen(str) + 1);
	
	for(p = buf; *str; str++) {
		if(*str == '_')
			*p++ = '_';

		*p++ = *str;
	}

	*p = '\0';

	return buf;
}

void
recent_update_menus(ggv_window *ggv, GList *recent_files)
{
	GnomeUIInfo *menu;
	gchar *path, *recent_data;
	int i;
        window_with_data *wwd;
        char *escape;
       
       /*	
        if(!ggv->show_menus)
                return;
*/
	if(ggv->num_recent > 0)
                gnome_app_remove_menu_range(GNOME_APP(ggv->main_window), 
                                             "_File/", 4, ggv->num_recent + 1);

        ggv->num_recent = 0;

	if(recent_files == NULL)
		return;

	/* insert a separator at the beginning */
	menu = g_malloc0(2 * sizeof(GnomeUIInfo));
	menu->type = GNOME_APP_UI_SEPARATOR;

	(menu + 1)->type = GNOME_APP_UI_ENDOFINFO;
	gnome_app_insert_menus(GNOME_APP(ggv->main_window),
                                "_File/<Separator>", menu);

	for(i = g_list_length(recent_files) - 1; i >= 0;  i--)
	{
                recent_data = (gchar *)g_list_nth_data(recent_files, i);
                wwd = g_malloc0(sizeof(window_with_data));

                escape = escape_underlines(recent_data);
		menu->label =  g_strdup_printf("_%i. %s", i+1, escape);
                g_free(escape);

		menu->type = GNOME_APP_UI_ITEM;
		menu->hint = NULL;
                
		menu->moreinfo = (gpointer) recent_callback;

                wwd->data = g_strdup(recent_data);
                wwd->ggv = ggv;

		menu->user_data = wwd;
		menu->unused_data = NULL;
		menu->pixmap_type = 0;
		menu->pixmap_info = NULL;
		menu->accelerator_key = 0;

		gnome_app_insert_menus(GNOME_APP(ggv->main_window),
                                        "_File/<Separator>", menu);

                g_free(menu->label);

                gtk_object_set_data_full(GTK_OBJECT(menu->widget), "wwd",
                                         wwd, (GtkDestroyNotify)free_wwd);
	}
        g_free(menu);
	ggv->num_recent = g_list_length(recent_files);
}

void
recent_update(void)
{
	GList *dirlist = NULL;
        GList *filelist = NULL;
	GList *gnome_recent_list;
        GList *ggv_node;
	GnomeHistoryEntry histentry;
	char *filename;
	int i, j;
        int nrecent = 0;

	filelist = NULL;
	gnome_recent_list = gnome_history_get_recently_used();

        if(recent_files_list) {
                g_list_foreach(recent_files_list, (GFunc)g_free, NULL);
                g_list_free(recent_files_list);
        }

	if(g_list_length(gnome_recent_list) > 0) {
                for(i = g_list_length(gnome_recent_list) - 1; i >= 0; i--) {
                        histentry = g_list_nth_data(gnome_recent_list, i);
			if(strcmp("ggv", histentry->creator) == 0) {
				/* This is to make sure you don't have more than one
				   file of the same name in the recent list
				*/
				if(g_list_length(filelist) > 0) {
					for(j = g_list_length(filelist) - 1; j >= 0; j--) {
						if(strcmp(histentry->filename, g_list_nth_data(filelist, j)) == 0) {
                                                        g_free(g_list_nth_data(filelist, j));
							filelist = g_list_remove(filelist, g_list_nth_data(filelist, j));
						}
					}
				}
                                
				filename = g_strdup(histentry->filename);
				filelist = g_list_append(filelist, filename);
                                
                                /* For recent-directories, not yet fully implemented...
                                   end_path = strrchr(histentry->filename, '/');
                                   if(end_path)
                                   {
                                   for(i = 0; i < strlen(histentry->filename); i++)
                                   if((histentry->filename + i) == end_path)
                                   break;
                                   directory = g_malloc0(i + 2);
                                   strcat(directory, histentry->filename, i);
                                   }
                                */
				if(++nrecent == MAX_RECENT)
					break;
			}
		}
	}

	gnome_history_free_recently_used_list(gnome_recent_list);

        recent_files_list = filelist;

        ggv_node = window_list;
        while(ggv_node) {
                recent_update_menus((ggv_window *)ggv_node->data,
                                    recent_files_list);
                ggv_node = ggv_node->next;
        }
}

void
recent_add(char *filename)
{
        gnome_history_recently_used(filename, "application/postscript", "ggv", "PS");
}

void display_current_doc_info(ggv_window *ggv)
{
        char *fname = GTK_GS(ggv->gs)->gs_filename;
        GtkGS* gs = GTK_GS(ggv->gs);
        struct document *doc = gs->doc;
        char *sz_orientation; 
        char *temp;
        
        if (ggv == NULL || doc == NULL)
                return;

        switch(doc->orientation) {
        case ATEND: 
                sz_orientation = _("At end");
                break;
        case NONE: 
                sz_orientation = _("Unspecified");
                break;
        case PORTRAIT: 
                sz_orientation = _("Portrait");
                break;
        case LANDSCAPE: 
                sz_orientation = _("Landscape");
                break;
        default: 
                sz_orientation = _("Unknown");
                break;
        }

        if(doc->epsf) {
                temp = _("Encapsulated PostScript");
        } else if(gs->structured_doc) {
                temp = _("Structured PostScript");
        } else {
                temp = _("Unstructured PostScript");
        }
        

        default_status_bar_message(ggv, 
                                   g_strdup_printf(_(
                                                   "Filename: %s %s. "
                                                   "Magnification %3d%%. "
                                                   "Document Orientation: %s. "
                                                   "Override Media: %s"),
                                                   fname, temp,
                                                   (int)rint(gs->zoom_factor * 100),
                                                   sz_orientation,
                                                   gs->override_media?_("Yes"):_("No")
                                                   ));
}


/* Useful functions */
gboolean
load_gs(ggv_window *ggv, char *fname)
{
	gchar *flash, *title;
	const gchar *ctitle;
        gint page;
        gboolean is_structured, reload_ok;
	GtkGS *gs;
        GtkCList *pagelist;

	gs = GTK_GS(ggv->gs);
        pagelist = GTK_CLIST(ggv->pagelist);


	if(GTK_GS(ggv->gs)->loaded && (strcmp(fname, gs->gs_filename) == 0)) {
		/* Reload the same file (probably because it's changed
		 * Clear message window and reload the last page
                 */
                init_gs_status(ggv, NULL);
                page = gs->current_page;
	} else {
                /* set up message window */
                title = g_strdup_printf(_("gs warnings for %s"), fname);
		init_gs_status(ggv, title);
		g_free(title);
                /* start with first page */
                page = 0;
	}

        flash = g_strdup_printf(_("Opening %s..."), fname);
        flash_message(ggv, flash);

	if(!gtk_gs_load(gs, fname)) {
                gtk_clist_clear(pagelist);
                gtk_window_set_title(GTK_WINDOW(ggv->main_window), "GGv");
                gtk_widget_set_sensitive(ggv->zoomoutbutton, FALSE);
                gtk_widget_set_sensitive(ggv->zoominbutton, FALSE);
                gtk_widget_set_sensitive(ggv->zoomoutpu, FALSE);
                gtk_widget_set_sensitive(ggv->zoominpu, FALSE);
                gtk_widget_set_sensitive(ggv->recenteritem, FALSE);
                gtk_widget_set_sensitive(ggv->recenterpu, FALSE);
                gtk_widget_set_sensitive(ggv->printitem, FALSE);
                gtk_widget_set_sensitive(ggv->printmarkeditem, FALSE);
                gtk_widget_set_sensitive(ggv->saveitem, FALSE);
                gtk_widget_set_sensitive(ggv->toggleakt, FALSE);
                gtk_widget_set_sensitive(ggv->toggleall, FALSE);
                gtk_widget_set_sensitive(ggv->toggleeven, FALSE);
                gtk_widget_set_sensitive(ggv->toggleodd, FALSE);
                gtk_widget_set_sensitive(ggv->clearall, FALSE);
                gtk_widget_set_sensitive(ggv->toggleakt_pu, FALSE);
                gtk_widget_set_sensitive(ggv->toggleall_pu, FALSE);
                gtk_widget_set_sensitive(ggv->toggleeven_pu, FALSE);
                gtk_widget_set_sensitive(ggv->toggleodd_pu, FALSE);
                gtk_widget_set_sensitive(ggv->clearall_pu, FALSE);

		gtk_widget_set_sensitive(ggv->toggleevenbutton, FALSE);
		gtk_widget_set_sensitive(ggv->toggleoddbutton, FALSE);
		gtk_widget_set_sensitive(ggv->toggleallbutton, FALSE);
		gtk_widget_set_sensitive(ggv->clearallbutton, FALSE);

		gtk_widget_set_sensitive(ggv->printbutton, FALSE);
		gtk_widget_set_sensitive(ggv->savebutton, FALSE);

		/* All errors while loading are reported in the status window
                 * so just show a short hint in the status line */
                flash = g_strdup_printf(_("Unable to load %s."), fname);
		flash_message(ggv, flash);

                return FALSE;
        }
        fflush(NULL);
        gtk_gs_set_pagemedia(gs, -1, 0);
        fflush(NULL);

	ctitle = gtk_gs_document_title(gs);
        if(!ctitle)
 		ctitle = fname;

	title = g_strjoin("", "GGv: ", ctitle, NULL);
        
	gtk_window_set_title(GTK_WINDOW(ggv->main_window), title);
	g_free(title);

        /* FIXME:
           Resize window if necessary, we 
           must verify whether automatic resizing is allowed (add option to
           menu on whether to auto resize 
           finally, at most it should be as large as the screen
           
           We also have to zoom the document to the specific zoom factor
           according to the menu. 

           gtk_gs_set_zoom(GTK_GS(ggv->gs), ggv->zoom_value); 

           So far, the user can't make the window smaller. I don't 
           know how to fix it.

        */

	/*
        gtk_widget_set_usize(gs, gs->width, gs->height);
	*/

	is_structured = gs->structured_doc;
	reload_ok = !gs->reading_from_pipe;

        if(ggv->pagelist) {
                gtk_clist_freeze(pagelist);
                gtk_clist_clear(pagelist);
                if(is_structured) {
			int n = gtk_gs_document_numpages(gs);
                        if(n > 0) {
				int i;
                                for(i = 1; i <= n ; i++) {
                                        gtk_check_list_append_row (GTK_CHECK_LIST (pagelist),
                                                                   gtk_gs_document_page_label (gs, i),
                                                                   FALSE, NULL);
                                }
                        }
                }
                gtk_clist_thaw(pagelist);
        }
        goto_page(ggv, page);
        gtk_widget_set_sensitive(ggv->zoomoutbutton, reload_ok);
        gtk_widget_set_sensitive(ggv->zoominbutton, reload_ok);
        gtk_widget_set_sensitive(ggv->zoomoutpu, reload_ok);
        gtk_widget_set_sensitive(ggv->zoominpu, reload_ok);
        gtk_widget_set_sensitive(ggv->printitem, reload_ok);
        gtk_widget_set_sensitive(ggv->recenteritem, reload_ok);
        gtk_widget_set_sensitive(ggv->recenterpu, reload_ok);
        gtk_widget_set_sensitive(ggv->printmarkeditem, is_structured);
        gtk_widget_set_sensitive(ggv->reloaditem, reload_ok);
        gtk_widget_set_sensitive(ggv->saveitem, is_structured && (!GTK_GS_IS_PDF(gs)));
	gtk_widget_set_sensitive(ggv->reloadbutton, reload_ok);
	gtk_widget_set_sensitive(ggv->printbutton, reload_ok);
	gtk_widget_set_sensitive(ggv->savebutton, is_structured && (!GTK_GS_IS_PDF(gs)));

        set_page_sensitivities(ggv, page);

        gtk_widget_set_sensitive(ggv->toggleakt, is_structured);
        gtk_widget_set_sensitive(ggv->toggleall, is_structured);
        gtk_widget_set_sensitive(ggv->toggleeven, is_structured);
        gtk_widget_set_sensitive(ggv->toggleodd, is_structured);
        gtk_widget_set_sensitive(ggv->clearall, is_structured);
        gtk_widget_set_sensitive(ggv->toggleakt_pu, is_structured);
        gtk_widget_set_sensitive(ggv->toggleall_pu, is_structured);
        gtk_widget_set_sensitive(ggv->toggleeven_pu, is_structured);
        gtk_widget_set_sensitive(ggv->toggleodd_pu, is_structured);
        gtk_widget_set_sensitive(ggv->clearall_pu, is_structured);

	gtk_widget_set_sensitive(ggv->toggleevenbutton, is_structured);
	gtk_widget_set_sensitive(ggv->toggleoddbutton, is_structured);
	gtk_widget_set_sensitive(ggv->toggleallbutton, is_structured);
	gtk_widget_set_sensitive(ggv->clearallbutton, is_structured);

        display_current_doc_info(ggv);

        flash = g_strdup_printf(_("Successfully loaded %s."), fname);
        flash_message(ggv, flash);

	if(!gs->reading_from_pipe)
                /* Ideally it should include the absolute path. It
                   does not, currently */
                recent_add(fname);

	if(gs->watch_doc) {
		if(gs->timer_tag)
                        gtk_timeout_remove(gs->timer_tag);
		gs->timer_tag = gtk_timeout_add(GGV_WATCH_INTERVAL,
						 timer_callback, ggv);
	}

        return TRUE;
}

void save_marked_pages(ggv_window *ggv, gchar *file)
{
        gchar *error_msg, *flash;
        FILE *file_handle;

        if((file_handle = fopen(file, "w")) != NULL) {
                GtkGS *gs = GTK_GS(ggv->gs);
		if(GTK_GS_IS_PDF(gs)) {
			/* Save marked pages of the dsc file. The saved
			 * file is useless without the pdf file, but can
			 * be used to print the marked pages.
			 * Printing only works with gs, not with a 'real'
			 * postscript printer.
			 * FIXME: Can we produce a real pdf file with ps2pdf?
			 */
			pscopydoc(file_handle, gs->gs_filename_dsc,
                                  gs->doc, gs->pages_marked);
                }
		else {
                       	/* Use uncompressed file if necessary */
                        pscopydoc(file_handle, GTK_GS_GET_PS_FILE(gs), gs->doc, gs->pages_marked);
                }
                flash = g_strdup_printf(_("Successfully saved marked pages to file %s."), file);
                flash_message(ggv, flash);
        } else {
                error_msg = g_strdup_printf(_("Unable to create %s."), file);
                error_message(ggv,error_msg);
        }
}

void set_page_sensitivities(ggv_window *ggv, int page)
{
        gboolean prev = FALSE, next = FALSE;

        if(page > 0)
                prev = TRUE;
        if(!GTK_GS(ggv->gs)->structured_doc ||(GTK_GS(ggv->gs)->doc == NULL) ||
          (page < GTK_GS(ggv->gs)->doc->numpages - 1))
                next = TRUE;
        gtk_widget_set_sensitive(ggv->nextbutton, next);
        gtk_widget_set_sensitive(ggv->prevbutton, prev);
        gtk_widget_set_sensitive(ggv->nextitem, next);
        gtk_widget_set_sensitive(ggv->previtem, prev);
        gtk_widget_set_sensitive(ggv->nextpu, next);
        gtk_widget_set_sensitive(ggv->prevpu, prev);
        gtk_widget_set_sensitive(ggv->firstpu, prev);
        gtk_widget_set_sensitive(ggv->firstitem, prev);
        gtk_widget_set_sensitive(ggv->lastpu, next && GTK_GS(ggv->gs)->structured_doc);
        gtk_widget_set_sensitive(ggv->lastitem, next && GTK_GS(ggv->gs)->structured_doc);
}        

void goto_page(ggv_window *ggv, int page)
{
        if(gtk_gs_goto_page( GTK_GS(ggv->gs), page) &&
           gs_auto_jump) {
                gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                         GTK_SCROLLPANE_GOTOEDGE_LOWER, 
                                         GTK_SCROLLPANE_GOTOEDGE_LOWER);
         }

        if(GTK_GS(ggv->gs)->structured_doc) {
                page = GTK_GS(ggv->gs)->current_page; /* Make sure */
                set_page_sensitivities(ggv, page);
                if(ggv->pagelist) {
                        gtk_clist_select_row(GTK_CLIST(ggv->pagelist), page,1);
                        if( !gtk_clist_row_is_visible(GTK_CLIST(ggv->pagelist), page)) {
                                gtk_clist_moveto(GTK_CLIST(ggv->pagelist), GTK_GS(ggv->gs)->current_page, 0, 0.5, 0.5);
                        }
                }
        }
}

/* Convenience function to load a pixmap and mask from xpm data */
static void
create_pixmap(char **data, GdkPixmap **pixmap, GdkBitmap **mask)
{
	GdkImlibImage *im;

	im = gdk_imlib_create_image_from_xpm_data(data);
	gdk_imlib_render(im, im->rgb_width, im->rgb_height);
	*pixmap = gdk_imlib_copy_image(im);
	*mask = gdk_imlib_copy_mask(im);
	gdk_imlib_destroy_image(im);
}


void create_popup_menus(ggv_window *ggv)
{
        /* the below #defines are supposed to be synced with actual indexes
           of the corresponding items in the popupMenu[] array! */
        GnomeUIInfo popupMenu[] = {
#define PU_NEXT_PAGE_INDEX 0
                {GNOME_APP_UI_ITEM, N_("_Next Page"), NULL, next_page_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD, 0, 0, NULL},
#define PU_PREV_PAGE_INDEX 1
                {GNOME_APP_UI_ITEM, N_("_Previous Page"), NULL, previous_page_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BACK, 0, 0, NULL},
#define PU_FIRST_PAGE_INDEX 2
                {GNOME_APP_UI_ITEM, N_("_First Page"), NULL, first_page_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FIRST, 0, 0, NULL},
#define PU_LAST_PAGE_INDEX 3
                {GNOME_APP_UI_ITEM, N_("_Last Page"), NULL, last_page_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_LAST, 0, 0, NULL},
#define PU_ZOOM_IN_INDEX 4
                {GNOME_APP_UI_ITEM, N_("Zoom _In"), NULL, zoomin_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, GNOME_STOCK_MENU_FORWARD, '+', GDK_CONTROL_MASK, NULL},
#define PU_ZOOM_OUT_INDEX 5
                {GNOME_APP_UI_ITEM, N_("Zoom _Out"), NULL, zoomout_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, GNOME_STOCK_MENU_BACK, '-', GDK_CONTROL_MASK, NULL},
#define PU_RECENTER_PAGE_INDEX 6
                {GNOME_APP_UI_ITEM, N_("_Recenter Page"), NULL, recenter_page_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SEPARATOR},
#define PU_PAGE_MENUS_INDEX 8
                {GNOME_APP_UI_ITEM, N_("Toggle _current page"), NULL, toggle_current_page_callback, NULL ,NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("Toggle _all pages"), NULL, toggle_all_pages_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("Toggle o_dd pages"), NULL, toggle_odd_pages_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("Toggle _even pages"), NULL, toggle_even_pages_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("_Unmark all pages"), NULL, unmark_all_pages_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SEPARATOR},
#define PU_SHOW_MENUS_INDEX 14
                {GNOME_APP_UI_TOGGLEITEM, N_("Show _Menus"), NULL, show_menubar_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 'm', GDK_CONTROL_MASK, NULL},
#define PU_SHOW_PANEL_INDEX 15
                {GNOME_APP_UI_TOGGLEITEM, N_("Show _Side Panel"), NULL, hide_panel_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 's', GDK_CONTROL_MASK, NULL},
#define PU_SHOW_TOOLBAR_INDEX 16
                {GNOME_APP_UI_TOGGLEITEM, N_("Show _Toolbar"), NULL, show_toolbar_callback, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 't', GDK_CONTROL_MASK, NULL},		 
                {GNOME_APP_UI_ENDOFINFO}
        };
                
        ggv->popup_menu = gnome_popup_menu_new(popupMenu);
        ggv->nextpu = popupMenu[PU_NEXT_PAGE_INDEX].widget;
        ggv->prevpu = popupMenu[PU_PREV_PAGE_INDEX].widget;
        ggv->firstpu = popupMenu[PU_FIRST_PAGE_INDEX].widget;
        ggv->lastpu = popupMenu[PU_LAST_PAGE_INDEX].widget;
        ggv->recenterpu = popupMenu[PU_RECENTER_PAGE_INDEX].widget;
        ggv->zoominpu = popupMenu[PU_ZOOM_IN_INDEX].widget;
        ggv->zoomoutpu = popupMenu[PU_ZOOM_OUT_INDEX].widget;

        ggv->menus_vis_pu = popupMenu[PU_SHOW_MENUS_INDEX].widget;
        ggv->panel_vis_pu = popupMenu[PU_SHOW_PANEL_INDEX].widget;
	ggv->toolbar_vis_pu = popupMenu[PU_SHOW_TOOLBAR_INDEX].widget;
        ggv->toggleakt_pu = popupMenu[PU_PAGE_MENUS_INDEX].widget;
        ggv->toggleall_pu = popupMenu[PU_PAGE_MENUS_INDEX+1].widget;
        ggv->toggleeven_pu = popupMenu[PU_PAGE_MENUS_INDEX+2].widget;
        ggv->toggleodd_pu = popupMenu[PU_PAGE_MENUS_INDEX+3].widget;
        ggv->clearall_pu = popupMenu[PU_PAGE_MENUS_INDEX+4].widget;

        gtk_widget_set_sensitive(GTK_WIDGET(ggv->prevpu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->nextpu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->firstpu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->lastpu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->zoominpu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->zoomoutpu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->recenterpu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->toggleakt_pu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->toggleall_pu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->toggleeven_pu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->toggleodd_pu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(ggv->clearall_pu), FALSE);
}
      
void create_menus(ggv_window *retval, window_with_data *temp)
{
        int i;

/* Use temporaries for non-compile-time constants. */
#define TMPVAL(i,orig)          ((void *) (0xC0DE0000 | (i)))
#define SETVAL(i,val,orig)	g_assert((val) == TMPVAL(i,orig)); val = orig
#define LASTTMPVAL 88
       
        GnomeUIInfo orientationMenu2[] = {
                GNOMEUIINFO_ITEM_DATA(N_("_Portrait"), N_("Portrait orientation"), orientation_callback,  TMPVAL(0,&temp[0]), NULL),
                GNOMEUIINFO_ITEM_DATA(N_("_Landscape"), N_("Landscape orientation"), orientation_callback, TMPVAL(1,&temp[1]), NULL),
                GNOMEUIINFO_ITEM_DATA(N_("_Upside Down"), N_("Upside down orientation"), orientation_callback, TMPVAL(2,&temp[2]), NULL),
                GNOMEUIINFO_ITEM_DATA(N_("_Seascape"), N_("Seascape orientation"), orientation_callback, TMPVAL(3,&temp[3]), NULL),
                GNOMEUIINFO_END
        };
        
        GnomeUIInfo orientationMenu[] = {
                {GNOME_APP_UI_TOGGLEITEM, N_("_Override Document Orientation"), N_("Override the orientation of the document."), 
                 override_orientation_callback, 
                 TMPVAL(4,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SEPARATOR},
                GNOMEUIINFO_RADIOLIST(TMPVAL(5,orientationMenu2)),
                GNOMEUIINFO_END
        };
        
#if 0
        GnomeUIInfo zoomMenu2[] = {
                GNOMEUIINFO_RADIOITEM_DATA("1:1", NULL, zoom_callback,  &temp[9], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("1:10" , NULL, zoom_callback, &temp[4] , NULL),
                GNOMEUIINFO_RADIOITEM_DATA("1:8" , NULL, zoom_callback, &temp[5], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("1:4", NULL, zoom_callback, &temp[6], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("1:2", NULL, zoom_callback, &temp[7], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("1:1.4", NULL, zoom_callback, &temp[8], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("1.4:1", NULL, zoom_callback, &temp[10] , NULL),
                GNOMEUIINFO_RADIOITEM_DATA("2:1", NULL, zoom_callback, &temp[11], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("4:1", NULL, zoom_callback, &temp[12], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("8:1", NULL, zoom_callback, &temp[13], NULL),
                GNOMEUIINFO_RADIOITEM_DATA("10:1", NULL, zoom_callback, &temp[14], NULL),
                GNOMEUIINFO_END,
        };
#else
        /* Lets try a percentage. The ratio does not seem to be not clear to everybuddy */ 
        GnomeUIInfo zoomMenu2[] = {
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(6,ZoomMagnLabels[0]), NULL, zoom_callback, TMPVAL(7,&temp[4]) , NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(8,ZoomMagnLabels[1]), NULL, zoom_callback, TMPVAL(9,&temp[5]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(10,ZoomMagnLabels[2]), NULL, zoom_callback, TMPVAL(11,&temp[6]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(12,ZoomMagnLabels[3]), NULL, zoom_callback, TMPVAL(13,&temp[7]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(14,ZoomMagnLabels[4]), NULL, zoom_callback, TMPVAL(15,&temp[8]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(16,ZoomMagnLabels[5]), NULL, zoom_callback, TMPVAL(17,&temp[9]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(18,ZoomMagnLabels[6]), NULL, zoom_callback, TMPVAL(19,&temp[10]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(20,ZoomMagnLabels[7]), NULL, zoom_callback, TMPVAL(21,&temp[11]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(22,ZoomMagnLabels[8]), NULL, NULL, TMPVAL(23,&temp[14]), NULL),
                GNOMEUIINFO_END,
        };
#endif


        GnomeUIInfo paperMenu2[] = {
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(24,papersizes[0].name), NULL, paper_callback,  
                                           TMPVAL(25,&temp[MENU_DATA_PAPER_OFFSET]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(26,papersizes[1].name), NULL, paper_callback,  
                                           TMPVAL(27,&temp[MENU_DATA_PAPER_OFFSET+1]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(28,papersizes[2].name), NULL, paper_callback,  
                                           TMPVAL(29,&temp[MENU_DATA_PAPER_OFFSET+2]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(30,papersizes[3].name), NULL, paper_callback,  
                                           TMPVAL(31,&temp[MENU_DATA_PAPER_OFFSET+3]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(32,papersizes[4].name), NULL, paper_callback,  
                                           TMPVAL(33,&temp[MENU_DATA_PAPER_OFFSET+4]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(34,papersizes[5].name), NULL, paper_callback,  
                                           TMPVAL(35,&temp[MENU_DATA_PAPER_OFFSET+5]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(36,papersizes[6].name), NULL, paper_callback,  
                                           TMPVAL(37,&temp[MENU_DATA_PAPER_OFFSET+6]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(38,papersizes[7].name), NULL, paper_callback,  
                                           TMPVAL(39,&temp[MENU_DATA_PAPER_OFFSET+7]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(40,papersizes[8].name), NULL, paper_callback,  
                                           TMPVAL(41,&temp[MENU_DATA_PAPER_OFFSET+8]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(42,papersizes[9].name), NULL, paper_callback,  
                                           TMPVAL(43,&temp[MENU_DATA_PAPER_OFFSET+9]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(44,papersizes[10].name), NULL, paper_callback,  
                                           TMPVAL(45,&temp[MENU_DATA_PAPER_OFFSET+10]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(46,papersizes[11].name), NULL, paper_callback,  
                                           TMPVAL(47,&temp[MENU_DATA_PAPER_OFFSET+11]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(48,papersizes[12].name), NULL, paper_callback,  
                                           TMPVAL(49,&temp[MENU_DATA_PAPER_OFFSET+12]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(50,papersizes[13].name), NULL, paper_callback,  
                                           TMPVAL(51,&temp[MENU_DATA_PAPER_OFFSET+13]), NULL),
                GNOMEUIINFO_RADIOITEM_DATA(TMPVAL(52,papersizes[14].name), NULL, paper_callback,  
                                           TMPVAL(53,&temp[MENU_DATA_PAPER_OFFSET+14]), NULL),
                GNOMEUIINFO_END,
        };
        
        GnomeUIInfo paperMenu[] = {
                {GNOME_APP_UI_TOGGLEITEM, N_("_Override Document Size"), N_("Override the size of page to use."), 
                 override_paper_callback, TMPVAL(54,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SEPARATOR},
                GNOMEUIINFO_RADIOLIST(TMPVAL(55,paperMenu2)),
                GNOMEUIINFO_END,
        };

        GnomeUIInfo zoomMenu[] = {
                GNOMEUIINFO_RADIOLIST(TMPVAL(56,zoomMenu2)),
                GNOMEUIINFO_END,
        };

        GnomeUIInfo settingsMenu[] = {
                GNOMEUIINFO_MENU_PREFERENCES_ITEM(preferences_callback, TMPVAL(57,retval)),
                {GNOME_APP_UI_SEPARATOR},
                {GNOME_APP_UI_TOGGLEITEM, N_("Show _Menus"), N_("Toggle menu visibility"), 
                 show_menubar_callback, TMPVAL(58,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_TOGGLEITEM, N_("Show _Side Panel"), N_("Toggle side panel visibility"), 
                 hide_panel_callback, TMPVAL(59,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	 	{GNOME_APP_UI_TOGGLEITEM, N_("Show _Toolbar"), N_("Toggle toolbar visibility"), 
                 show_toolbar_callback, TMPVAL(60,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},								  
                GNOMEUIINFO_END
        };

        GnomeUIInfo fileMenu[] = {
                {GNOME_APP_UI_ITEM, N_("_New Window"), N_("Open a new window"), new_callback, TMPVAL(61,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 'n', GDK_CONTROL_MASK, NULL},
                {GNOME_APP_UI_ITEM, N_("_Open..."), N_("Load a PostScript document"), open_callback, TMPVAL(62,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN, 'o', GDK_CONTROL_MASK, NULL},
                {GNOME_APP_UI_ITEM, N_("_Reload"), N_("Reload current document from disk"), reload_callback, TMPVAL(63,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_REFRESH, 'r', GDK_CONTROL_MASK, NULL},
                {GNOME_APP_UI_SEPARATOR},
                {GNOME_APP_UI_ITEM, N_("_Print"), N_("Print the document"), print_callback, TMPVAL(64,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PRINT, 'p', GDK_CONTROL_MASK, NULL },
                {GNOME_APP_UI_ITEM, N_("Prin_t marked pages"), N_("Print marked pages"), print_marked_pages_callback, TMPVAL(65,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PRINT, 0, 0, NULL },
                {GNOME_APP_UI_ITEM, N_("Save _marked pages..."), N_("Save marked pages to a new file"), save_callback, TMPVAL(66,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE, 0, 0, NULL},
                {GNOME_APP_UI_SEPARATOR},
                {GNOME_APP_UI_ITEM, N_("_Close"), N_("Close this window"), close_callback, TMPVAL(67,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CLOSE, 'w', GDK_CONTROL_MASK, NULL },
                {GNOME_APP_UI_SEPARATOR},
                {GNOME_APP_UI_ITEM, N_("E_xit"), N_("Exit GGv"), exit_callback, TMPVAL(68,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'q', GDK_CONTROL_MASK, NULL },
                {GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL, NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL}
        };

        GnomeUIInfo documentMenu[] = {
                {GNOME_APP_UI_ITEM, N_("_Next Page"), N_("Show next page"), next_page_callback, TMPVAL(69,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("_Previous Page"), N_("Show previous page"), previous_page_callback, TMPVAL(70,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BACK, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("_First Page"), N_("Show first page"), first_page_callback, TMPVAL(87,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FIRST, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("_Last Page"), N_("Show last page"), last_page_callback, TMPVAL(88,retval), NULL,
                 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_LAST, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("_Recenter Page"), N_("Center page"), recenter_page_callback, TMPVAL(71,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SEPARATOR},
                {GNOME_APP_UI_ITEM, N_("Toggle _current page"), NULL, toggle_current_page_callback, TMPVAL(72,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("Toggle _all pages"), NULL, toggle_all_pages_callback, TMPVAL(73,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("Toggle o_dd pages"), NULL, toggle_odd_pages_callback, TMPVAL(74,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("Toggle _even pages"), NULL, toggle_even_pages_callback, TMPVAL(75,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ITEM, N_("_Unmark all pages"), NULL, unmark_all_pages_callback, TMPVAL(76,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SEPARATOR},
                {GNOME_APP_UI_TOGGLEITEM, N_("Antialiasin_g"), N_("Toggle antialiasing"), antialiasing_callback, TMPVAL(77,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_TOGGLEITEM, N_("_Watch File"), N_("Updates display if file changes."), watch_file_callback, TMPVAL(78,retval), NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SUBTREE, N_("Orien_tation"), NULL, TMPVAL(79,orientationMenu), NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SUBTREE, N_("_Zoom"), NULL, TMPVAL(80,zoomMenu), NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_SUBTREE, N_("Page _Size"), NULL, TMPVAL(81,paperMenu), NULL, NULL,
                 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
                {GNOME_APP_UI_ENDOFINFO}
        };
        
        GnomeUIInfo helpMenu[] = {
                GNOMEUIINFO_HELP("ggv"),
                GNOMEUIINFO_MENU_ABOUT_ITEM(about_callback, TMPVAL(82,retval)),
                GNOMEUIINFO_END
        };
        
        GnomeUIInfo mainMenu[] = {
                GNOMEUIINFO_MENU_FILE_TREE(TMPVAL(83,fileMenu)),
                GNOMEUIINFO_SUBTREE(N_("_Document"), TMPVAL(84,documentMenu)),
                GNOMEUIINFO_MENU_SETTINGS_TREE(TMPVAL(85,settingsMenu)),
                GNOMEUIINFO_MENU_HELP_TREE(TMPVAL(86,helpMenu)),
                GNOMEUIINFO_END
        };

	/* Initialize pseudo-constan fields. */
        SETVAL(0,orientationMenu2[0].user_data,&temp[0]);
        SETVAL(1,orientationMenu2[1].user_data,&temp[1]);
        SETVAL(2,orientationMenu2[2].user_data,&temp[2]);
        SETVAL(3,orientationMenu2[3].user_data,&temp[3]);
        SETVAL(4,orientationMenu[0].user_data,retval);
        SETVAL(5,orientationMenu[2].moreinfo,orientationMenu2);
        SETVAL(6,zoomMenu2[0].label,ZoomMagnLabels[0]);
        SETVAL(7,zoomMenu2[0].user_data,&temp[4]);
        SETVAL(8,zoomMenu2[1].label,ZoomMagnLabels[1]);
        SETVAL(9,zoomMenu2[1].user_data,&temp[5]);
        SETVAL(10,zoomMenu2[2].label,ZoomMagnLabels[2]);
        SETVAL(11,zoomMenu2[2].user_data,&temp[6]);
        SETVAL(12,zoomMenu2[3].label,ZoomMagnLabels[3]);
        SETVAL(13,zoomMenu2[3].user_data,&temp[7]);
        SETVAL(14,zoomMenu2[4].label,ZoomMagnLabels[4]);
        SETVAL(15,zoomMenu2[4].user_data,&temp[8]);
        SETVAL(16,zoomMenu2[5].label,ZoomMagnLabels[5]);
        SETVAL(17,zoomMenu2[5].user_data,&temp[9]);
        SETVAL(18,zoomMenu2[6].label,ZoomMagnLabels[6]);
        SETVAL(19,zoomMenu2[6].user_data,&temp[10]);
        SETVAL(20,zoomMenu2[7].label,ZoomMagnLabels[7]);
        SETVAL(21,zoomMenu2[7].user_data,&temp[11]);
        SETVAL(22,zoomMenu2[8].label,ZoomMagnLabels[8]);
        SETVAL(23,zoomMenu2[8].user_data,&temp[14]);
        SETVAL(24,paperMenu2[0].label,papersizes[0].name);
        SETVAL(25,paperMenu2[0].user_data,&temp[MENU_DATA_PAPER_OFFSET]);
        SETVAL(26,paperMenu2[1].label,papersizes[1].name);
        SETVAL(27,paperMenu2[1].user_data,&temp[MENU_DATA_PAPER_OFFSET+1]);
        SETVAL(28,paperMenu2[2].label,papersizes[2].name);
        SETVAL(29,paperMenu2[2].user_data,&temp[MENU_DATA_PAPER_OFFSET+2]);
        SETVAL(30,paperMenu2[3].label,papersizes[3].name);
        SETVAL(31,paperMenu2[3].user_data,&temp[MENU_DATA_PAPER_OFFSET+3]);
        SETVAL(32,paperMenu2[4].label,papersizes[4].name);
        SETVAL(33,paperMenu2[4].user_data,&temp[MENU_DATA_PAPER_OFFSET+4]);
        SETVAL(34,paperMenu2[5].label,papersizes[5].name);
        SETVAL(35,paperMenu2[5].user_data,&temp[MENU_DATA_PAPER_OFFSET+5]);
        SETVAL(36,paperMenu2[6].label,papersizes[6].name);
        SETVAL(37,paperMenu2[6].user_data,&temp[MENU_DATA_PAPER_OFFSET+6]);
        SETVAL(38,paperMenu2[7].label,papersizes[7].name);
        SETVAL(39,paperMenu2[7].user_data,&temp[MENU_DATA_PAPER_OFFSET+7]);
        SETVAL(40,paperMenu2[8].label,papersizes[8].name);
        SETVAL(41,paperMenu2[8].user_data,&temp[MENU_DATA_PAPER_OFFSET+8]);
        SETVAL(42,paperMenu2[9].label,papersizes[9].name);
        SETVAL(43,paperMenu2[9].user_data,&temp[MENU_DATA_PAPER_OFFSET+9]);
        SETVAL(44,paperMenu2[10].label,papersizes[10].name);
        SETVAL(45,paperMenu2[10].user_data,&temp[MENU_DATA_PAPER_OFFSET+10]);
        SETVAL(46,paperMenu2[11].label,papersizes[11].name);
        SETVAL(47,paperMenu2[11].user_data,&temp[MENU_DATA_PAPER_OFFSET+11]);
        SETVAL(48,paperMenu2[12].label,papersizes[12].name);
        SETVAL(49,paperMenu2[12].user_data,&temp[MENU_DATA_PAPER_OFFSET+12]);
        SETVAL(50,paperMenu2[13].label,papersizes[13].name);
        SETVAL(51,paperMenu2[13].user_data,&temp[MENU_DATA_PAPER_OFFSET+13]);
        SETVAL(52,paperMenu2[14].label,papersizes[14].name);
        SETVAL(53,paperMenu2[14].user_data,&temp[MENU_DATA_PAPER_OFFSET+14]);
        SETVAL(54,paperMenu[0].user_data,retval);
        SETVAL(55,paperMenu[2].moreinfo,paperMenu2);
        SETVAL(56,zoomMenu[0].moreinfo,zoomMenu2);
        SETVAL(57,settingsMenu[0].user_data,retval);
        SETVAL(58,settingsMenu[2].user_data,retval);
        SETVAL(59,settingsMenu[3].user_data,retval);
        SETVAL(60,settingsMenu[4].user_data,retval);
        SETVAL(61,fileMenu[0].user_data,retval);
        SETVAL(62,fileMenu[1].user_data,retval);
        SETVAL(63,fileMenu[2].user_data,retval);
        SETVAL(64,fileMenu[4].user_data,retval);
        SETVAL(65,fileMenu[5].user_data,retval);
        SETVAL(66,fileMenu[6].user_data,retval);
        SETVAL(67,fileMenu[8].user_data,retval);
        SETVAL(68,fileMenu[10].user_data,retval);
        SETVAL(69,documentMenu[0].user_data,retval);
        SETVAL(70,documentMenu[1].user_data,retval);
        SETVAL(87,documentMenu[2].user_data,retval);
        SETVAL(88,documentMenu[3].user_data,retval);
        SETVAL(71,documentMenu[4].user_data,retval);
        SETVAL(72,documentMenu[6].user_data,retval);
        SETVAL(73,documentMenu[7].user_data,retval);
        SETVAL(74,documentMenu[8].user_data,retval);
        SETVAL(75,documentMenu[9].user_data,retval);
        SETVAL(76,documentMenu[10].user_data,retval);
        SETVAL(77,documentMenu[12].user_data,retval);
        SETVAL(78,documentMenu[13].user_data,retval);
        SETVAL(79,documentMenu[14].moreinfo,orientationMenu);
        SETVAL(80,documentMenu[15].moreinfo,zoomMenu);
        SETVAL(81,documentMenu[16].moreinfo,paperMenu);
        SETVAL(82,helpMenu[1].user_data,retval);
        SETVAL(83,mainMenu[0].moreinfo,fileMenu);
        SETVAL(84,mainMenu[1].moreinfo,documentMenu);
        SETVAL(85,mainMenu[2].moreinfo,settingsMenu);
        SETVAL(86,mainMenu[3].moreinfo,helpMenu);
#undef TMPVAL
#undef SETVAL
#undef LASTTMPVAL

        /* We set up the main window... */
        gnome_app_create_menus(GNOME_APP(retval->main_window), mainMenu);

	gnome_app_install_menu_hints(GNOME_APP(retval->main_window),
                                     mainMenu);

        retval->nextitem = documentMenu[0].widget;
        retval->previtem = documentMenu[1].widget;
        retval->firstitem = documentMenu[2].widget;
        retval->lastitem = documentMenu[3].widget;
        retval->recenteritem = documentMenu[4].widget;
        retval->reloaditem = fileMenu[2].widget;
        retval->printitem = fileMenu[4].widget;
        retval->printmarkeditem = fileMenu[5].widget;
        retval->saveitem = fileMenu[6].widget;
        retval->menus_vis = settingsMenu[2].widget;
        retval->panel_vis = settingsMenu[3].widget;
	retval->toolbar_vis = settingsMenu[4].widget;
        retval->toggleakt = documentMenu[6].widget;
        retval->toggleall = documentMenu[7].widget;
        retval->toggleeven = documentMenu[8].widget;
        retval->toggleodd = documentMenu[9].widget;
        retval->clearall = documentMenu[10].widget;
        retval->antialiased_menu = documentMenu[12].widget;
        retval->watchdoc_menu = documentMenu[13].widget;
        retval->override_media_menu = paperMenu[0].widget;
        retval->override_orientation_menu = orientationMenu[0].widget;

        for (i=0;i<MENU_ZOOM_SIZE;i++) {
                retval->zoom_menu[i] = zoomMenu2[i].widget;
        }

        for (i=0;i<PAPER_SIZE_COUNT;i++) {
                retval->paper_menu[i] = paperMenu2[i].widget;
        }
        for (i=0;i<MENU_ORIENTATION_SIZE;i++) {
                retval->orientation_menu[i] = orientationMenu2[i].widget;
        }

        gtk_widget_set_sensitive(GTK_WIDGET(retval->previtem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->nextitem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->firstitem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->lastitem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->recenteritem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->printitem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->printmarkeditem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->reloaditem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->saveitem), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->toggleakt), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->toggleall), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->toggleeven), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->toggleodd), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->clearall), FALSE);

        gtk_widget_set_sensitive(zoomMenu2[MENU_ZOOM_SIZE-1].widget, FALSE);

        /*
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(retval->zoom_menu[2]),
                                                       TRUE);
        */

        if(!recent_files_list)
                recent_update();
        recent_update_menus(retval, recent_files_list);
}


void create_toolbar(ggv_window *retval)
{
	GnomeUIInfo toolbar [] = {
		GNOMEUIINFO_ITEM_STOCK (N_("New"), N_("Create a new window"), new_callback, GNOME_STOCK_PIXMAP_NEW),
		GNOMEUIINFO_ITEM_STOCK (N_("Open"), N_("Load a PostScript document"), open_callback, GNOME_STOCK_PIXMAP_OPEN),
		GNOMEUIINFO_ITEM_STOCK (N_("Reload"), N_("Reload current document from disk"), reload_callback, GNOME_STOCK_PIXMAP_REFRESH),

		GNOMEUIINFO_SEPARATOR,

		GNOMEUIINFO_ITEM_STOCK (N_("Print marked"), N_("Print marked pages"), print_marked_pages_callback, GNOME_STOCK_PIXMAP_PRINT),
		GNOMEUIINFO_ITEM_STOCK (N_("Save marked"), N_("Save marked pages to a new file"), save_callback, GNOME_STOCK_PIXMAP_SAVE),
  		
		GNOMEUIINFO_SEPARATOR,

		GNOMEUIINFO_ITEM_STOCK (N_("Preferences"), N_("Configure the application"), preferences_callback, GNOME_STOCK_PIXMAP_PREFERENCES),

		GNOMEUIINFO_END
	};

	gnome_app_create_toolbar_with_data(GNOME_APP(retval->main_window), toolbar, retval);

	retval->reloadbutton = toolbar[2].widget;
	retval->printbutton = toolbar[4].widget;
	retval->savebutton = toolbar[5].widget;

	gtk_widget_set_sensitive(GTK_WIDGET(retval->reloadbutton), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(retval->printbutton), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(retval->savebutton), FALSE);
}

void create_sidebar(ggv_window *retval)
{
        GtkWidget *pic;
        GtkWidget *npbhbox;
        GtkWidget *zbhbox;
	GtkWidget *bhbox;

	bhbox = gtk_hbox_new(TRUE, 0);

	retval->toggleallbutton = gtk_button_new();
	pic = gnome_stock_pixmap_widget_new(retval->toggleallbutton, STOCK_TOGGLE_ALL);
	gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->toggleallbutton), pic);

	retval->toggleoddbutton = gtk_button_new();
	pic = gnome_stock_pixmap_widget_new(retval->toggleallbutton, STOCK_TOGGLE_ODD);
	gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->toggleoddbutton), pic);
	
	retval->toggleevenbutton = gtk_button_new();
	pic = gnome_stock_pixmap_widget_new(retval->toggleallbutton, STOCK_TOGGLE_EVEN);
	gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->toggleevenbutton), pic);

	retval->clearallbutton = gtk_button_new();
	pic = gnome_stock_pixmap_widget_new(retval->toggleallbutton, STOCK_CLEAR_ALL);
	gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->clearallbutton), pic);

	gtk_signal_connect(GTK_OBJECT(retval->toggleallbutton), "clicked", GTK_SIGNAL_FUNC(toggle_all_pages_callback), retval);
	gtk_signal_connect(GTK_OBJECT(retval->toggleoddbutton), "clicked", GTK_SIGNAL_FUNC(toggle_odd_pages_callback), retval);
	gtk_signal_connect(GTK_OBJECT(retval->toggleevenbutton), "clicked", GTK_SIGNAL_FUNC(toggle_even_pages_callback), retval);
	gtk_signal_connect(GTK_OBJECT(retval->clearallbutton), "clicked", GTK_SIGNAL_FUNC(unmark_all_pages_callback), retval);

	gtk_container_add(GTK_CONTAINER(bhbox), retval->toggleallbutton);
	gtk_container_add(GTK_CONTAINER(bhbox), retval->toggleoddbutton);
	gtk_container_add(GTK_CONTAINER(bhbox), retval->toggleevenbutton);
	gtk_container_add(GTK_CONTAINER(bhbox), retval->clearallbutton);

	gtk_widget_set_sensitive(GTK_WIDGET(retval->toggleallbutton),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(retval->toggleoddbutton),FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->toggleevenbutton),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(retval->clearallbutton),FALSE);

	gtk_button_set_relief(GTK_BUTTON(retval->toggleallbutton),GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(retval->toggleoddbutton),GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(retval->toggleevenbutton), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(retval->clearallbutton),GTK_RELIEF_NONE);

	ggv_set_tooltip(retval->toggleallbutton, _("Toggle all pages"));
	ggv_set_tooltip(retval->toggleoddbutton, _("Toggle odd pages"));
	ggv_set_tooltip(retval->toggleevenbutton, _("Toggle even pages"));
	ggv_set_tooltip(retval->clearallbutton, _("Unmark all pages"));

        gtk_widget_show(retval->toggleallbutton);
	gtk_widget_show(retval->toggleoddbutton);
	gtk_widget_show(retval->toggleevenbutton);
 	gtk_widget_show(retval->clearallbutton);

	gtk_widget_show(bhbox);
        
        retval->scrollpane = gtk_scrollpane_new(GTK_ADJUSTMENT(retval->hadj), GTK_ADJUSTMENT(retval->vadj), 1.0);
        gtk_signal_connect(GTK_OBJECT(retval->scrollpane), "middle_clicked", GTK_SIGNAL_FUNC(scrollpane_middle_click_callback), retval);
        gtk_signal_connect(GTK_OBJECT(retval->scrollpane), "right_clicked", GTK_SIGNAL_FUNC(scrollpane_right_click_callback), retval);
        gtk_widget_show(retval->scrollpane);
        
        npbhbox = gtk_hbox_new(TRUE, 0);
        retval->nextbutton = gtk_button_new();
        retval->prevbutton = gtk_button_new();

        pic = gnome_stock_pixmap_widget_new(retval->prevbutton, GNOME_STOCK_PIXMAP_BACK);
        gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->prevbutton), pic);
        pic = gnome_stock_pixmap_widget_new(retval->nextbutton, GNOME_STOCK_PIXMAP_FORWARD);
        gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->nextbutton), pic);
        gtk_signal_connect(GTK_OBJECT(retval->nextbutton), "clicked", GTK_SIGNAL_FUNC(next_page_callback), retval);
        gtk_signal_connect(GTK_OBJECT(retval->prevbutton), "clicked", GTK_SIGNAL_FUNC(previous_page_callback), retval);
        gtk_container_add(GTK_CONTAINER(npbhbox), retval->prevbutton);
        gtk_container_add(GTK_CONTAINER(npbhbox), retval->nextbutton);

        gtk_widget_set_sensitive(GTK_WIDGET(retval->prevbutton), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(retval->nextbutton), FALSE);

	gtk_button_set_relief(GTK_BUTTON(retval->prevbutton), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(retval->nextbutton), GTK_RELIEF_NONE);

	ggv_set_tooltip(retval->prevbutton, _("Previous page"));
	ggv_set_tooltip(retval->nextbutton, _("Next page"));

        gtk_widget_show(retval->prevbutton);
        gtk_widget_show(retval->nextbutton);
        gtk_widget_show(npbhbox);
        
        zbhbox = gtk_hbox_new(TRUE, 0);
        retval->zoominbutton = gtk_button_new(); 
        retval->zoomoutbutton = gtk_button_new();
                 
	pic = gnome_stock_pixmap_widget_new(retval->zoomoutbutton, STOCK_ZOOM_OUT);
        gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->zoomoutbutton), pic);
       
       	pic = gnome_stock_pixmap_widget_new(retval->zoominbutton, STOCK_ZOOM_IN);
        gtk_widget_show(pic);
        gtk_container_add(GTK_CONTAINER(retval->zoominbutton), pic);
        
	gtk_signal_connect(GTK_OBJECT(retval->zoominbutton), "clicked", GTK_SIGNAL_FUNC(zoomin_callback), retval);
        gtk_signal_connect(GTK_OBJECT(retval->zoomoutbutton), "clicked", GTK_SIGNAL_FUNC(zoomout_callback), retval);
        gtk_container_add(GTK_CONTAINER(zbhbox), retval->zoomoutbutton);
        gtk_container_add(GTK_CONTAINER(zbhbox), retval->zoominbutton);

	gtk_button_set_relief(GTK_BUTTON(retval->zoomoutbutton), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(retval->zoominbutton), GTK_RELIEF_NONE);

	ggv_set_tooltip(retval->zoomoutbutton, _("Zoom out"));
	ggv_set_tooltip(retval->zoominbutton, _("Zoom in"));

        gtk_widget_set_sensitive(GTK_WIDGET(retval->zoomoutbutton), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(retval->zoominbutton), FALSE);
		
	gtk_widget_show(retval->zoomoutbutton);
        gtk_widget_show(retval->zoominbutton);
        gtk_widget_show(zbhbox);

	retval->pagelistscroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(retval->pagelistscroll),
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(retval->pagelistscroll),
					GTK_CORNER_BOTTOM_RIGHT);

        retval->pagelist = gtk_check_list_new ();
	gtk_widget_set_usize(retval->pagelist, 80, 100);

        /* gtk_clist_set_column_width(GTK_CLIST(retval->pagelist), 0, 18);
           gtk_clist_set_column_width(GTK_CLIST(retval->pagelist), 1, 20);*/
	gtk_clist_set_selection_mode(GTK_CLIST(retval->pagelist), GTK_SELECTION_BROWSE);
        /*	gtk_clist_set_column_justification(GTK_CLIST(retval->pagelist), 0, GTK_JUSTIFY_LEFT);*/
	gtk_clist_set_column_justification(GTK_CLIST(retval->pagelist), 1, GTK_JUSTIFY_RIGHT);

        gtk_clist_set_shadow_type(GTK_CLIST(retval->pagelist),GTK_SHADOW_ETCHED_IN);

        gtk_signal_connect(GTK_OBJECT(retval->pagelist), "select_row", GTK_SIGNAL_FUNC(select_page_callback),retval);
        gtk_signal_connect(GTK_OBJECT(retval->pagelist), "toggled", GTK_SIGNAL_FUNC(page_toggled_callback),retval);

	gtk_container_add(GTK_CONTAINER(retval->pagelistscroll), retval->pagelist);
	gtk_widget_show(retval->pagelist);
	gtk_widget_show(retval->pagelistscroll);

        gtk_box_pack_start(GTK_BOX(retval->sidebar), retval->scrollpane, FALSE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(retval->sidebar), zbhbox, FALSE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(retval->sidebar), npbhbox, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(retval->sidebar), bhbox, FALSE, TRUE, 0);
	     
        gtk_container_add(GTK_CONTAINER(retval->sidebar), retval->pagelistscroll);
        gtk_widget_show(retval->sidebar); 
}

/* main functions */
ggv_window *create_ggv_window()
{
        /* DnD stuff */
	static GtkTargetEntry drop_types [] = {
		{ "text/uri-list", 0, TARGET_URI_LIST}
	};
	static gint n_drop_types = sizeof(drop_types) / sizeof(drop_types[0]);

        gboolean bxneg, byneg;
        gint i;
        GtkWidget *hbox;
        /* GtkWidget *vbox; */
        GtkWidget *gsframe;

        ggv_window *retval = g_new0(ggv_window, 1);

        if(nomenubar)
                retval->show_menus = FALSE;
        else
                retval->show_menus = gs_menubar;

	if(nosidebar)
                retval->show_panel = FALSE;
        else
                retval->show_panel = gs_panel;

	if(notoolbar)
                retval->show_toolbar = FALSE;
        else
                retval->show_toolbar = gs_toolbar;

        retval->pane_auto_jump = gs_auto_jump;
        retval->pan = FALSE;

        /* create the main window. */
        retval->main_window = gnome_app_new("ggv", "GGv:");

        if(geometry){
                int xpos, ypos;
                int temp_ggv_default_width;
                int temp_ggv_default_height;
         
                ggv_parse_geometry(geometry, &xpos, &ypos, &bxneg, &byneg, &temp_ggv_default_width, &temp_ggv_default_height);
                /*
                g_print("Geometry %d %d %d %d\n", xpos, ypos, 
                        temp_ggv_default_width, temp_ggv_default_height);
                */
                if(xpos != -1 && ypos != -1) {
                        if(bxneg) xpos -= temp_ggv_default_width;
                        /* FIXME we need to know the vertical size of
                           the current window bar */
                        if(byneg) ypos -= temp_ggv_default_height + 18;
                        gtk_widget_set_uposition(GTK_WIDGET(retval->main_window), xpos, ypos);
                }
                if(temp_ggv_default_width != -1 && temp_ggv_default_height != -1) {
                        ggv_default_width = temp_ggv_default_width;
                        ggv_default_height = temp_ggv_default_height;
                }
                /* Only the first window gets --geometry treatment for now */
                geometry = NULL;
        }
        if(fullscreen) {
                gtk_widget_set_uposition(GTK_WIDGET(retval->main_window), 0, 0);
                /* FIXME we need to know the vertical size of
                   the current window bar */
                ggv_default_height = gdk_screen_height() - 18;
                ggv_default_width = gdk_screen_width();

        }

        /* Make sure that the window is _never_ bigger than the screen */
        if(gdk_screen_height() < ggv_default_height)
                ggv_default_height = gdk_screen_height();
        
        if(gdk_screen_width() < ggv_default_width)
                ggv_default_width = gdk_screen_width();

        gtk_window_set_default_size(GTK_WINDOW(retval->main_window), 
                                    ggv_default_width, ggv_default_height);
        gtk_window_set_policy(GTK_WINDOW(retval->main_window), TRUE, TRUE, FALSE);
        retval->cd = NULL;
        /* make it a drop zone for files */
	gtk_drag_dest_set(GTK_WIDGET(retval->main_window),
	                   GTK_DEST_DEFAULT_MOTION |
			   GTK_DEST_DEFAULT_HIGHLIGHT |
			   GTK_DEST_DEFAULT_DROP,
			   drop_types, n_drop_types,
			   GDK_ACTION_COPY);
        gtk_signal_connect(GTK_OBJECT(retval->main_window), "drag_data_received",
			    GTK_SIGNAL_FUNC(drop_callback), retval);
        /* take care of its deletion */
	gtk_signal_connect(GTK_OBJECT(retval->main_window), "delete_event",
                            GTK_SIGNAL_FUNC(delete_callback), retval);

        /* Set up the callback values for the orientation */
        for(i = 0; i < MENU_DATA_SIZE; i++)
                retval->menudata[i].ggv = retval;

        retval->menudata[0].data = GINT_TO_POINTER(GTK_GS_ORIENTATION_PORTRAIT);
        retval->menudata[1].data = GINT_TO_POINTER(GTK_GS_ORIENTATION_LANDSCAPE);
        retval->menudata[2].data = GINT_TO_POINTER(GTK_GS_ORIENTATION_UPSIDEDOWN);
        retval->menudata[3].data = GINT_TO_POINTER(GTK_GS_ORIENTATION_SEASCAPE);

	/* We are lying a little here - the magstep does not give
	 * these ratios exactly but who's going to know ;)
	 */
        for (i=0;i<MENU_ZOOM_SIZE;i++) {
                retval->menudata[4+i].data = GINT_TO_POINTER(ZoomMenuMagnificationSteps[i]);
        }
     
        /*
        retval->menudata[5].data = GINT_TO_POINTER(-13);
        retval->menudata[6].data = GINT_TO_POINTER(-12);
        retval->menudata[7].data = GINT_TO_POINTER(-8); 
        retval->menudata[8].data = GINT_TO_POINTER(-4); 
        retval->menudata[9].data = GINT_TO_POINTER(-2); 
        retval->menudata[10].data = GINT_TO_POINTER(0);
        retval->menudata[11].data = GINT_TO_POINTER(2);
        retval->menudata[12].data = GINT_TO_POINTER(4);
        retval->menudata[13].data = GINT_TO_POINTER(8);
        retval->menudata[14].data = GINT_TO_POINTER(12);
        retval->menudata[15].data = GINT_TO_POINTER(13);
        */

        /* Fill the data for page size */
        for(i = 0; i< PAPER_SIZE_COUNT; i++) {
                retval->menudata[MENU_DATA_PAPER_OFFSET+i].data = GINT_TO_POINTER(i);
        }

        /* Statusbar */
        retval->appbar = gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_USER);
        gtk_widget_show(retval->appbar);
        gnome_app_set_statusbar(GNOME_APP(retval->main_window), retval->appbar);

        /* Menus */
	create_menus(retval, retval->menudata);
	create_popup_menus(retval);

	/* Toolbar */
	create_toolbar(retval);

        /* We set up the layout */
        hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        /* vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL); */

        retval->hadj = gtk_adjustment_new(0.1, 0.0, 1.0, 1.0, 1.0, 0.5);
        retval->vadj = gtk_adjustment_new(0.1, 0.0, 1.0, 1.0, 1.0, 0.5);

        /* we set up the post script display */
        gsframe = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(gsframe), GTK_SHADOW_IN);

        retval->gs = gtk_gs_new(GTK_ADJUSTMENT(retval->hadj),
                                GTK_ADJUSTMENT(retval->vadj));
        gtk_widget_set_events(retval->gs, 
                              GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
        set_gs_prefs(retval);
        gtk_container_add(GTK_CONTAINER(gsframe), retval->gs);
        gtk_widget_show(retval->gs);
        gtk_widget_show(gsframe);

        /* we prepare for the crop data */
        retval->cd = g_malloc(sizeof(crop_data));

        /* We set up the sidebar */
        retval->zoom_magstep = 0; /* 1 : 1 */

        retval->sidebar = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	
       	create_sidebar(retval);

        if(!retval->show_panel)
	{
		gtk_widget_hide (retval->sidebar);
		gtk_widget_queue_resize (retval->main_window);
	}			
        
        /* We layout the page */
        gtk_box_pack_start(GTK_BOX(hbox), retval->sidebar, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), gsframe, TRUE, TRUE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
        gtk_widget_show(hbox);
        /* gtk_widget_show(vbox); */

	gnome_app_set_contents(GNOME_APP(retval->main_window), hbox);
        gtk_signal_connect(GTK_OBJECT(retval->gs), "button_press_event",
                            GTK_SIGNAL_FUNC(button_press_callback), retval);
        gtk_signal_connect(GTK_OBJECT(retval->gs), "button_release_event",
                            GTK_SIGNAL_FUNC(button_release_callback), retval);
        gtk_signal_connect(GTK_OBJECT(retval->gs), "motion_notify_event",
                            GTK_SIGNAL_FUNC(motion_callback), retval);
        gtk_signal_connect(GTK_OBJECT(retval->gs), "interpreter_message",
                            GTK_SIGNAL_FUNC(interpreter_message_callback),
                            retval);
        gtk_signal_connect(GTK_OBJECT(retval->main_window), "key_press_event",
                            GTK_SIGNAL_FUNC(key_pressed_event_callback), retval);

        retval->file_sel = NULL;
        retval->save_file_sel = NULL;
        retval->pane_auto_jump =  gs_auto_jump;

        if(retval->show_menus) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(retval->menus_vis),
                                        retval->show_menus);
                synchronize_user_interface(retval);

        }
	
	if(retval->show_toolbar) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(retval->toolbar_vis),
                                        retval->show_menus);
	}

        if(retval->show_panel)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(retval->panel_vis),
                                                retval->show_panel);

        return retval;
}

void default_status_bar_message(ggv_window *ggv, gchar *message)
{
        if(message) {
                gnome_appbar_set_default((GnomeAppBar*)ggv->appbar, message);
                g_free(message);
        }
}

void flash_message(ggv_window *ggv, gchar *flash)
{
        if(flash) {
                gnome_app_flash(GNOME_APP(ggv->main_window), flash);
                g_free(flash);
        }
}

void error_message(ggv_window *ggv, gchar *errormsg)
{
        if(errormsg) {
                gnome_app_error(GNOME_APP(ggv->main_window), errormsg);
                g_free(errormsg);
        }
}

ggv_window *open_window(gchar *filename, gint x, gint y, gint w, gint h)
{
        ggv_window *ggv;

        if(w != -1 && h != -1) {
                ggv_default_width = w;
                ggv_default_height = h;
        }

        ggv = create_ggv_window();
        active_ggv++;
        window_list = g_list_append(window_list, ggv);

        if(x != -1 && y != -1)
                gtk_widget_set_uposition(ggv->main_window, x, y);

        gtk_widget_show(ggv->main_window);

	if(!ggv->show_menus)
	{
		gtk_widget_hide((GNOME_APP(ggv->main_window))->menubar->parent);
		gtk_widget_queue_resize (ggv->main_window);
	}
	
	if(!ggv->show_toolbar)
	{
		GnomeDockItem* dock_item = gnome_app_get_dock_item_by_name(GNOME_APP(ggv->main_window), 
				GNOME_APP_TOOLBAR_NAME);
		gtk_widget_hide (GTK_WIDGET(dock_item));
		gtk_widget_queue_resize (ggv->main_window);
	}

	if(filename)
                load_gs(ggv, filename);

        return ggv;
}

void close_window(ggv_window *ggv)
{
        /* do we want to save changes??? */
	window_list = g_list_remove(window_list, ggv);
	if(ggv->status)
		gtk_widget_destroy(GTK_WIDGET(ggv->status));
	gtk_widget_destroy(GTK_WIDGET(ggv->main_window));
        g_free(ggv);

        active_ggv--;

	if(active_ggv == 0)
		gtk_main_quit();
}

void open_prefs_dialog(ggv_prefs *pd)
{
        GtkWidget *table, *label, *menu;
        gint i;

        if(pd->pbox)
                return;

        pd->pbox = GNOME_PROPERTY_BOX(gnome_property_box_new());
	gtk_signal_connect(GTK_OBJECT(pd->pbox), "destroy",
                           GTK_SIGNAL_FUNC(prefs_destroy_callback), pd);
	gtk_signal_connect(GTK_OBJECT(pd->pbox), "apply",
                           GTK_SIGNAL_FUNC(prefs_apply_callback), pd);
	gtk_signal_connect(GTK_OBJECT(pd->pbox), "help",
                           GTK_SIGNAL_FUNC(prefs_help_callback), pd);

        gtk_window_set_title(GTK_WINDOW(GNOME_PROPERTY_BOX(pd->pbox)), 
                              _("GGV Preferences"));

        /* Document page */
        table = gtk_table_new(2, 7, FALSE);

        /* zoom choice menu */
        label = gtk_label_new(_("Default Zoom"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);

        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 0, 1,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);

        gtk_widget_show(label);
        pd->zoom = gtk_option_menu_new();
        gtk_widget_show(pd->zoom);
        menu = gtk_menu_new();

        /* We only go as far a as MENU_ZOOM_SIZE-1 because the last option
           is "other" */
        for(i = 0; i < MENU_ZOOM_SIZE-1; i++) {
                pd->zoom_choice[i] = gtk_menu_item_new_with_label(ZoomMagnLabels[i]);
                gtk_widget_show(pd->zoom_choice[i]);
                gtk_menu_append(GTK_MENU(menu), pd->zoom_choice[i]);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(pd->zoom), menu);
        /* We have to find with option to activate. The default magnification 
           should match one of the elements of ZoomMenuMagnificationSteps */
        for (i = 0; i < MENU_ZOOM_SIZE; i++) {
                if (ZoomMenuMagnificationSteps[i] == gs_default_magnification) {
                        gtk_option_menu_set_history(GTK_OPTION_MENU(pd->zoom), i);
                }
        }

        for(i = 0; i< MENU_ZOOM_SIZE-1; i++)
                gtk_signal_connect(GTK_OBJECT(pd->zoom_choice[i]),
                                   "activate", GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
        gtk_table_attach(GTK_TABLE(table), pd->zoom,
                         1, 2, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);

        /* media choice menu */
        label = gtk_label_new(_("Fallback media size"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 1, 2,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        pd->media = gtk_option_menu_new();
        gtk_widget_show(pd->media);
        menu = gtk_menu_new();
        for(i = 0; papersizes[i].name != NULL; i++) {
                pd->media_choice[i] = gtk_menu_item_new_with_label(papersizes[i].name);
                gtk_widget_show(pd->media_choice[i]);
                gtk_menu_append(GTK_MENU(menu), pd->media_choice[i]);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(pd->media), menu);
        i = gtk_gs_get_default_page_media();
        gtk_option_menu_set_history(GTK_OPTION_MENU(pd->media), i);
        for(i = 0; papersizes[i].name != NULL; i++)
                gtk_signal_connect(GTK_OBJECT(pd->media_choice[i]),
                                   "activate", GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
        gtk_table_attach(GTK_TABLE(table), pd->media,
                         1, 2, 1, 2,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);

        /* override media size */
        pd->override_media = gtk_check_button_new_with_label(_("Override media size"));
        gtk_table_attach(GTK_TABLE(table), pd->override_media,
                         0, 2, 2, 3,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->override_media), 
                                     gtk_gs_get_default_override_media());
        gtk_signal_connect(GTK_OBJECT(pd->override_media), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->override_media);

        /* orientation choice menu */
        label = gtk_label_new(_("Fallback media orientation"));
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 3, 4,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);

        pd->orientation = gtk_option_menu_new();
        gtk_widget_show(pd->orientation);
        menu = gtk_menu_new();
        for(i = 0; i < MENU_ORIENTATION_SIZE; i++) {
                pd->orientation_choice[i] = gtk_menu_item_new_with_label(OrientationLabels[i]);
                gtk_widget_show(pd->orientation_choice[i]);
                gtk_menu_append(GTK_MENU(menu), pd->orientation_choice[i]);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(pd->orientation), menu);

        i = gtk_gs_get_default_orientation();

        gtk_option_menu_set_history(GTK_OPTION_MENU(pd->orientation), i);

        for(i = 0; i< MENU_ORIENTATION_SIZE; i++) 
                gtk_signal_connect(GTK_OBJECT(pd->orientation_choice[i]),
                                   "activate", GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
        gtk_table_attach(GTK_TABLE(table), pd->orientation,
                         1, 2, 3, 4,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);

        /* override orientation media */
        pd->override_orientation = gtk_check_button_new_with_label(_("Override document orientation"));
        gtk_table_attach(GTK_TABLE(table), pd->override_orientation,
                         0, 2, 4, 5,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->override_orientation), 
                                     gtk_gs_get_default_override_orientation());
        gtk_signal_connect(GTK_OBJECT(pd->override_orientation), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->override_orientation);

        /* antialiasing */
        pd->aa = gtk_check_button_new_with_label(_("Antialiasing"));
        gtk_table_attach(GTK_TABLE(table), pd->aa,
                         0, 2, 5, 6,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->aa), 
                                     gtk_gs_get_default_antialiased());

        gtk_signal_connect(GTK_OBJECT(pd->aa), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->aa);

        /* respect EOF */
        pd->respect_eof = gtk_check_button_new_with_label(_("Respect EOF comments"));
        gtk_table_attach(GTK_TABLE(table), pd->respect_eof,
                         0, 2, 6, 7,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->respect_eof), 
                                     gtk_gs_get_default_respect_eof());
        gtk_signal_connect(GTK_OBJECT(pd->respect_eof), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->respect_eof);

        /* watch file */
        pd->watch = gtk_check_button_new_with_label(_("Watch file"));
        gtk_table_attach(GTK_TABLE(table), pd->watch,
                         0, 2, 7, 8,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->watch), 
                                     gtk_gs_get_default_watch_doc());
        gtk_signal_connect(GTK_OBJECT(pd->watch), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->watch);

        label = gtk_label_new(_("Document"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gnome_property_box_append_page(pd->pbox, table, label);

        /* Layout page */
        table = gtk_table_new(3, 1, FALSE);

        /* show toolbar */
        pd->tbar = gtk_check_button_new_with_label(_("Show side panel by default"));
        gtk_table_attach(GTK_TABLE(table), pd->tbar,
                         0, 1, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->tbar), gs_panel);
        gtk_signal_connect(GTK_OBJECT(pd->tbar), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->tbar);
        /* show menubar */
        pd->mbar = gtk_check_button_new_with_label(_("Show menubar by default"));
        gtk_table_attach(GTK_TABLE(table), pd->mbar,
                         0, 1, 1, 2,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->mbar), gs_menubar);
        gtk_signal_connect(GTK_OBJECT(pd->mbar), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->mbar);
	/* show toolbar */
        pd->toolbar = gtk_check_button_new_with_label(_("Show toolbar by default"));
        gtk_table_attach(GTK_TABLE(table), pd->toolbar,
                         0, 1, 2, 3,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->toolbar), gs_toolbar);
        gtk_signal_connect(GTK_OBJECT(pd->toolbar), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->toolbar);

        /* save geometry */
        pd->savegeo = gtk_check_button_new_with_label(_("Save geometry"));
        gtk_table_attach(GTK_TABLE(table), pd->savegeo,
                         0, 1, 3, 4,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->savegeo), gs_save_geometry);
        gtk_signal_connect(GTK_OBJECT(pd->savegeo), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->savegeo);
        /* auto jump to beginning of page */
        pd->auto_jump= gtk_check_button_new_with_label(_("Jump to beginning of page when using keyboard"));
        gtk_table_attach(GTK_TABLE(table), pd->auto_jump,
                         0, 1, 4, 5,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pd->auto_jump), gs_auto_jump);
        gtk_signal_connect(GTK_OBJECT(pd->auto_jump), "clicked",
                           prefs_changed_callback, pd);
        gtk_widget_show(pd->auto_jump);


        label = gtk_label_new(_("Layout"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gnome_property_box_append_page(pd->pbox, table, label);

        /* GhostScript page */
        table = gtk_table_new(2, 4, FALSE);

        /* interpreter */
        label = gtk_label_new(_("Interpreter"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 0, 1,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        pd->gs = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(pd->gs), gs_cmd);
	gtk_entry_set_position(GTK_ENTRY(pd->gs), 0);
        gtk_table_attach(GTK_TABLE(table), pd->gs,
                         1, 2, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	gtk_signal_connect(GTK_OBJECT(pd->gs), "activate",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
	gtk_signal_connect(GTK_OBJECT(pd->gs), "changed",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
        gtk_widget_show(pd->gs);
        /* scan PDF command */
        label = gtk_label_new(_("Scan PDF"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 1, 2,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        pd->scan_pdf = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(pd->scan_pdf), gs_scan_pdf_cmd);
	gtk_entry_set_position(GTK_ENTRY(pd->scan_pdf), 0);
        gtk_table_attach(GTK_TABLE(table), pd->scan_pdf,
                         1, 2, 1, 2,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	gtk_signal_connect(GTK_OBJECT(pd->scan_pdf), "activate",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
	gtk_signal_connect(GTK_OBJECT(pd->scan_pdf), "changed",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
        gtk_widget_show(pd->scan_pdf);

	/* unzip command: gzip */
        label = gtk_label_new(_("Gzip"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 2, 3,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        pd->unzip = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(pd->unzip), gs_ungzip_cmd);
	gtk_entry_set_position(GTK_ENTRY(pd->unzip), 0);
        gtk_table_attach(GTK_TABLE(table), pd->unzip,
                         1, 2, 2, 3,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	gtk_signal_connect(GTK_OBJECT(pd->unzip), "activate",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
	gtk_signal_connect(GTK_OBJECT(pd->unzip), "changed",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
	gtk_widget_show(pd->unzip);

	/* unzip command: bzip2 */
        label = gtk_label_new(_("Bzip2"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 3, 4,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        pd->unbzip2 = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(pd->unbzip2), gs_unbzip2_cmd);
	gtk_entry_set_position(GTK_ENTRY(pd->unbzip2), 0);
        gtk_table_attach(GTK_TABLE(table), pd->unbzip2,
                         1, 2, 3, 4,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	gtk_signal_connect(GTK_OBJECT(pd->unbzip2), "activate",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
	gtk_signal_connect(GTK_OBJECT(pd->unbzip2), "changed",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
	gtk_widget_show(pd->unbzip2);

        label = gtk_label_new(_("Ghostscript"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gnome_property_box_append_page(pd->pbox, table, label);

        /* Printing page */
        table = gtk_table_new(1, 2, FALSE);

        /* print command */
        label = gtk_label_new(_("Print command"));
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 0, 1,
                         GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        pd->print = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(pd->print), gs_print_cmd);
	gtk_entry_set_position(GTK_ENTRY(pd->print), 0);
        gtk_table_attach(GTK_TABLE(table), pd->print,
                         1, 2, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	gtk_signal_connect(GTK_OBJECT(pd->print), "activate",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
	gtk_signal_connect(GTK_OBJECT(pd->print), "changed",
                           GTK_SIGNAL_FUNC(prefs_changed_callback), pd);
        gtk_widget_show(pd->print);

        label = gtk_label_new(_("Printing"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gnome_property_box_append_page(pd->pbox, table, label);

	gtk_window_position(GTK_WINDOW(pd->pbox), GTK_WIN_POS_MOUSE);
	
	gtk_widget_show(GTK_WIDGET(pd->pbox));
}

void zoom_to(ggv_window *ggv, gint magstep) 
{
        gfloat z;
        
        if (ggv->zoom_magstep == magstep) {
                return;
        }
        
        z = ggv_compute_zoom(magstep);

	if((z <= 2.0/100.0) || (z >= 1000.0/100.0)) 
	{
		/* To avoid problem if zoom factor < 2% or > 1000% */
		return;
	}
	else
	{	
		ggv->zoom_magstep = magstep;
	}

        /* Return if no active doc */
        if(!GTK_GS(ggv->gs)->loaded) {
                GTK_GS(ggv->gs)->zoom_factor = z;
                return;
        }
        
        gtk_gs_set_zoom(GTK_GS(ggv->gs), z);
        gtk_gs_goto_page(GTK_GS(ggv->gs), GTK_GS(ggv->gs)->current_page);        
        display_current_doc_info(ggv);
}



gboolean set_corresponding_zoom_menu_option(ggv_window *ggv, int magstep)
{
        int i;
	GnomeUIInfo *menu;
        window_with_data *wwd;

        /* This function is used to set the menu zoom radio button to
          the corresponding magnification. 

        */

        /* I know this is inefficient, but it guarantees that if the magnification
           is different from any in the menu, then none is going to be ON */

        for  (i=0; i<MENU_ZOOM_SIZE-1; i++) {
                if (ZoomMenuMagnificationSteps[i] == magstep) {
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->zoom_menu[i]),
                                                       TRUE);
                        return TRUE;
                }
        }
        
#if 0
        /*Well, I was trying to dynamically create a radio button for the
          new mag. but I was not able to get it to be a radio button :( */

        wwd = g_malloc0(sizeof(window_with_data));

        gnome_app_remove_menu_range(GNOME_APP(ggv->main_window), 
                                    "_Document/_Zoom/", 
                                    MENU_ZOOM_SIZE-1, 1);

	menu = g_malloc0(2 * sizeof(GnomeUIInfo));

	(menu + 1)->type = GNOME_APP_UI_ENDOFINFO;

        menu->label =  g_strdup_printf("xyz");
        menu->type = GNOME_APP_UI_ITEM;
        menu->hint = NULL;
                
        menu->moreinfo = (gpointer) zoom_callback;

        wwd->data = GINT_TO_POINTER(magstep);
        wwd->ggv = ggv;

        menu->user_data = wwd;
        menu->unused_data = NULL;
        menu->pixmap_type = 0;
        menu->pixmap_info = NULL;
        menu->accelerator_key = 0;

	gnome_app_insert_menus(GNOME_APP(ggv->main_window),
                                "_Document/_Zoom/400%", menu);

        ggv->zoom_menu[MENU_ZOOM_SIZE-1] = menu->widget;

#else
        /* Select the "other" option, which is not user selectable */

        /* its callback  is NULL, so it should have no side effect */
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->zoom_menu[MENU_ZOOM_SIZE-1]),
                                       TRUE);
#endif

        return FALSE;
}

void synchronize_user_interface(ggv_window *ggv)
{
        g_assert(ggv != NULL);

        if(ggv->show_menus) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->antialiased_menu),
                                               GTK_GS(ggv->gs)->antialiased);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->watchdoc_menu),
                                               GTK_GS(ggv->gs)->watch_doc);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->override_media_menu),
                                               GTK_GS(ggv->gs)->override_media);
                set_corresponding_zoom_menu_option(ggv, ggv->zoom_magstep);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->paper_menu[GTK_GS(ggv->gs)->default_page_media]), 
                                               TRUE);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->orientation_menu[GTK_GS(ggv->gs)->fallback_orientation]), 
                                               TRUE);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->watchdoc_menu),
                                               GTK_GS(ggv->gs)->watch_doc);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->override_orientation_menu),
                                               GTK_GS(ggv->gs)->override_orientation);
        }
}


void set_gs_prefs(ggv_window *ggv)
{
        GtkGS *gs = GTK_GS(ggv->gs);
        gboolean redisplay;

	redisplay =(ggv->gs && GTK_GS(ggv->gs)->loaded &&
		    ((gs->antialiased != gtk_gs_get_default_antialiased()) ||
		     (gs->override_media != gtk_gs_get_default_override_media()) ||
                     (gtk_gs_get_orientation(gs) != gtk_gs_get_default_orientation()) ||
		     (gtk_gs_get_default_override_media() && 
                      (gs->default_page_media != gtk_gs_get_default_page_media())) &&
                     (gs->respect_eof != gtk_gs_get_default_respect_eof())
                     ));
        set_prefs(ggv);
        synchronize_user_interface(ggv);
        if(redisplay)
                reload_callback(NULL, ggv);
}

void apply_gs_prefs(GList *windows)
{
        while(windows) {
                set_gs_prefs((ggv_window *)windows->data);
                windows = windows->next;
        }
}

static void
parse_an_arg(poptContext state,
           enum poptCallbackReason reason,
           const struct poptOption *opt,
           const char *arg, void *data)
{
        struct terminal_config *cfg = data;
 
        int key = opt->val;
 
        switch(key) {
        case GEOMETRY_KEY:
                geometry =(char *)arg;
                break;
        case NO_SIDE_BAR_KEY:
                nosidebar = TRUE;
                break;
        case NO_MENU_BAR_KEY:
                nomenubar = TRUE;
                break;
        case FULL_SCREEN_KEY:
                fullscreen = TRUE;
                break;
	case NO_TOOL_BAR_KEY:
                notoolbar = TRUE;
                break;
	case SPARTAN_KEY:
		nosidebar = TRUE;
		notoolbar = TRUE;
		break;
	
        }
}

struct poptOption options_callback [] = {
        { NULL, '\0', POPT_ARG_CALLBACK, parse_an_arg, 0},
        
        { "geometry", '\0', POPT_ARG_STRING, NULL, GEOMETRY_KEY,
          N_("Specifies the geometry for the main window"), N_("GEOMETRY")},

      	{ "spartan", '\0', POPT_ARG_NONE, NULL, SPARTAN_KEY,
	  N_("Do not use sidebar and toolbar (deprecated)"), N_("SPARTAN") },

	{ "nosidebar", '\0', POPT_ARG_NONE, NULL, NO_SIDE_BAR_KEY,
	  N_("Do not use sidebar"), N_("NOSIDEBAR") },

	{ "nomenubar", '\0', POPT_ARG_NONE, NULL, NO_MENU_BAR_KEY,
	  N_("Do not use a menu bar"), N_("NOMENUBAR") },

	{ "notoolbar", '\0', POPT_ARG_NONE, NULL, NO_TOOL_BAR_KEY,
	  N_("Do not use a tool bar"), N_("NOTOOLBAR") },
 
	{ "fullscreen", '\0', POPT_ARG_NONE, NULL, FULL_SCREEN_KEY,
	  N_("Use the whole screen for the main window"), N_("FULLSCREEN") },
        
        { NULL, '\0', 0, NULL, 0}
};


/* search for filename, return new allocated copy of filename or NULL
 * if no file was found
 * search for: "-"(means read from pipe), filename, filename+".ps",
 * ".pdf", ".ps.gz" and ".pdf.gz"
 */
static char *ggv_filename(const char *filename)
{
        char *newname;
        int l;

	if(filename == NULL)
                return NULL;

        if((strcmp(filename,"-") == 0) || ggv_file_readable(filename))
                return g_strdup(filename);

        l = strlen(filename)+8;
        newname = g_malloc(l);

        g_snprintf(newname, l, "%s.ps", filename);
        if(ggv_file_readable(newname))
                return newname;

        g_snprintf(newname, l, "%s.pdf", filename);
        if(ggv_file_readable(newname))
                return newname;

        g_snprintf(newname, l, "%s.ps.gz", filename);
        if(ggv_file_readable(newname))
                return newname;

        g_snprintf(newname, l, "%s.pdf.gz", filename);
        if(ggv_file_readable(newname))
                return newname;

	g_free(newname);

        return NULL;
}

static void restore_session()
{
        int docs, i;
        gint w, h, x, y, page;
        gfloat zoom, horval, vertval;
        gchar *docdesc, *docinfo, *infoptr;
        ggv_window *ggv;
        GtkWidget *wd;

        docs = gnome_config_get_int("Session/Documents=0");
        i = 1;
        while(i <= docs) {
                docdesc = g_strdup_printf("Session/Document%d", i);
                docinfo = gnome_config_get_string(docdesc);
                if(docinfo) {
                        infoptr = docinfo;
                        while(*infoptr != '\0' && *infoptr != ':')
                                infoptr++;
                        if(*infoptr == '\0')
                                continue;
                        *infoptr = '\0';
                        if(sscanf(infoptr + 1, "%d,%d,%d,%d/%d/%f/%f/%f",
                                  &x, &y, &w, &h, &page, &zoom,
                                  &horval, &vertval) != 8) {
                                x = y = w = h = -1;
                                page = 0;
                                zoom = 1.0;
                                horval = vertval = 0.0;
                        }
                        ggv = open_window(NULL, x, y, w, h);
                        gtk_gs_set_zoom(GTK_GS(ggv->gs), zoom);
                        load_gs(ggv, docinfo);
                        goto_page(ggv, page);
                        gtk_adjustment_set_value(GTK_GS(ggv->gs)->hadj, horval);
                        gtk_adjustment_set_value(GTK_GS(ggv->gs)->vadj, vertval);
 
                }
                g_free(docdesc);
                i++;
        }
}

static int save_session(GnomeClient        *client,
                        gint                phase,
                        GnomeRestartStyle   save_style,
                        gint                shutdown,
                        GnomeInteractStyle  interact_style,
                        gint                fast,
                        gpointer            client_data) {
	gchar *prefix= gnome_client_get_config_prefix(client);
	gchar *argv[]= { "rm", "-r", NULL };
	gint docs, w, h, x, y, i;
        gchar *docdesc, *docinfo, *fname;
        GList *doc_node;
        ggv_window *ggv;

	/* Save the state using gnome-config stuff. */
	gnome_config_push_prefix(prefix);
        docs = g_list_length(window_list);
	gnome_config_set_int("Session/Documents", docs);
        doc_node = window_list;
        i = 1;
        while(doc_node) {
                docdesc = g_strdup_printf("Session/Document%d", i);
                ggv = (ggv_window *)(doc_node->data);
                fname = GTK_GS(ggv->gs)->gs_filename;
		gdk_window_get_geometry(ggv->main_window->window,
                                        &x, &y, &w, &h, NULL);
		gdk_window_get_origin(((ggv_window *)doc_node->data)->main_window->window, &x, &y);
                docinfo = g_strdup_printf("%s:%d,%d,%d,%d/%d/%f/%f/%f",
                                          fname, x, y, w, h,
                                          GTK_GS(ggv->gs)->current_page,
                                          GTK_GS(ggv->gs)->zoom_factor,
                                          GTK_GS(ggv->gs)->hadj->value,
                                          GTK_GS(ggv->gs)->vadj->value);
                gnome_config_set_string(docdesc, docinfo);
                g_free(docinfo);
                g_free(docdesc);
                doc_node = doc_node->next;
                i++;
        }
	gnome_config_pop_prefix();
	gnome_config_sync();
	
	argv[2] = gnome_config_get_real_path(prefix);
	gnome_client_set_discard_command(client, 3, argv);
	
	argv[0] = (char*) client_data;
	gnome_client_set_clone_command(client, 1, argv);
	gnome_client_set_restart_command(client, 1, argv);
	
	return TRUE;
}

gint client_die(GnomeClient *client, gpointer client_data)
{
	gtk_exit (0);
	
	return FALSE;
}

/* and we finally start the main prog. */
int main(int argc, char *argv[])
{
	GnomeClient *client;
	poptContext ctx;
	gchar **startups;
        gchar *fname;
        ggv_window *ggv;

        bindtextdomain(PACKAGE, GNOMELOCALEDIR);
        textdomain(PACKAGE);

	gnome_init_with_popt_table("gnome-ghostview", 
                                   VERSION, argc, argv, options_callback, 
                                   0, &ctx);

        load_prefs("/ggv/");

	create_pixmap(foot_xpm, &page_marker_pixmap, &page_marker_mask);
	create_pixmap(foot_white_xpm, &page_marker_white_pixmap, &page_marker_white_mask);

        client = gnome_master_client();
	gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
                            GTK_SIGNAL_FUNC(save_session), (gpointer)argv[0]);
	gtk_signal_connect (GTK_OBJECT (client), "die",
                            GTK_SIGNAL_FUNC(client_die), NULL);

	ggv_stock_init();
	
        /* restore state from previous session */
        if(gnome_client_get_flags(client) & GNOME_CLIENT_RESTORED) {
		gnome_config_push_prefix(gnome_client_get_config_prefix(client));
		restore_session();
		gnome_config_pop_prefix();
	}
        else {
                startups = (char **) poptGetArgs(ctx);
                if(startups) {
                        while((fname = *startups++)) {
                                if((fname = ggv_filename(fname))) {
                                        open_window(fname, -1, -1, -1, -1);
                                        g_free(fname);
                                }
                        }
                }
                poptFreeContext(ctx);
                if(window_list == NULL) {
                        if(isatty(fileno(stdin)))
                                open_window(NULL, -1, -1, -1, -1);
                        else
                                open_window("-", -1, -1, -1, -1);
                }
        }

	gtk_main();

        save_prefs("/ggv/");

        return 0;
}

