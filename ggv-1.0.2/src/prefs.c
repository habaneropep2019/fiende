/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Authors: Jaka Mocnik  <jaka@gnu.org>
 */

#include <config.h>
#include <gnome.h>

#include "ps.h"
#include "prefs.h"
#include "gtkgs.h"
#include "ggvwindow.h"
#include "callbacks.h"
#include "ggvutils.h"

gchar *gs_cmd = NULL;
gchar *gs_scan_pdf_cmd = NULL;
gchar *gs_ungzip_cmd = NULL;
gchar *gs_unbzip2_cmd = NULL;
gchar *gs_print_cmd = NULL;

gboolean gs_save_geometry;
gint ggv_default_width, file_sel_width;
gint ggv_default_height, file_sel_height;

gboolean gs_panel, gs_menubar, gs_toolbar;
gboolean gs_auto_jump;
gint gs_default_magnification; /* Default magnification for ggv */

char *OrientationLabels[MENU_ORIENTATION_SIZE+1] = {
        "Portrait",
        "Landscape",
        "Upside Down",
        "Seascape",
        NULL,
};


static gint
get_media_index(gchar *string, struct documentmedia *media)
{
        guint idx = 0;

        while(media[idx].name != NULL) {
                if(strcmp(media[idx].name, string) == 0)
                        return idx;
                idx++;
        }

        return -1;
}

static gint
get_orientation_index(gchar *string)
{
        guint idx = 0;

        while(OrientationLabels[idx] != NULL) {
                if(strcmp(OrientationLabels[idx], string) == 0)
                        return idx;
                idx++;
        }

        return -1;
}

#include <math.h>
gfloat compute_zoom(gint s)
{
        return pow(1.2, s);
}

void
load_prefs(gchar *prefix)
{
        gchar *s;
        gint i;

        gnome_config_push_prefix(prefix);
        if((s = gnome_config_get_string("GhostScript/Interpreter=" GS_PATH))) {
                g_free(gs_cmd);
                gs_cmd = s;
        }
        if((s = gnome_config_get_string("GhostScript/ScanPDF=" GS_PATH " -dNODISPLAY -dQUIET -sPDFname=\"%s\" -sDSCname=\"%s\" pdf2dsc.ps -c quit"))) {
                g_free(gs_scan_pdf_cmd);
                gs_scan_pdf_cmd = s;
        }
        if((s = gnome_config_get_string("GhostScript/Ungzip=gzip -cd"))) {
                g_free(gs_ungzip_cmd);
                gs_ungzip_cmd = s;
        }
        if((s = gnome_config_get_string("GhostScript/Unbzip2=bzip2 -cd"))) {
                g_free(gs_unbzip2_cmd);
                gs_unbzip2_cmd = s;
        }
        if((s = gnome_config_get_string("Printing/Command=" LPR_PATH " %s"))) {
                g_free(gs_print_cmd);
                gs_print_cmd = s;
        }

        /* Read gtkgs widget defaults */
        gs_default_magnification = gnome_config_get_int("Document/Magnification=0");
        gtk_gs_set_default_zoom_factor(ggv_compute_zoom(gs_default_magnification));

        gtk_gs_set_default_override_media(gnome_config_get_bool("Document/OverrideDocumentMedia=false"));
        gtk_gs_set_default_antialiased(gnome_config_get_bool("Document/Antialiasing=false"));
        gtk_gs_set_default_watch_doc(gnome_config_get_bool("Document/Watch=false"));
        gtk_gs_set_default_respect_eof(gnome_config_get_bool("Document/RespectEOF=false"));
        gtk_gs_set_default_override_orientation(gnome_config_get_bool("Document/OverrideOrientation=false"));
        gtk_gs_set_default_override_orientation(gnome_config_get_bool("Document/OverrideOrientation=false"));

        if((s = gnome_config_get_string("Document/Orientation=Portrait"))) {
                i = get_orientation_index(s);
                g_free(s);
                if (i == -1) 
                        i = get_orientation_index("Portrait");
                gtk_gs_set_default_orientation(i);

        }

        if((s = gnome_config_get_string("Document/Media=A4"))) {
                i = get_media_index(s, papersizes);
                g_free(s);
                if(i == -1)
                        i = get_media_index("A4", papersizes);

                /*
                g_print("default media = %d\n", i);
                */
                gtk_gs_set_default_page_media(i);
        }

        /* read ggv widget defaults */
        gs_panel = gnome_config_get_bool("Layout/ShowPanel=true");
	gs_toolbar = gnome_config_get_bool("Layout/ShowToolbar=true");
        gs_menubar = gnome_config_get_bool("Layout/ShowMenubar=true");
        gs_auto_jump = gnome_config_get_bool("Layout/AutoJump=true");

        /* Get geometry */
        gs_save_geometry = gnome_config_get_bool("Layout/SaveGeometry=false");
        if((ggv_default_width = gnome_config_get_int("Layout/SaveGeometryX")) == 0)
                ggv_default_width = DEFAULT_WINDOW_WIDTH;
        if((ggv_default_height = gnome_config_get_int("Layout/SaveGeometryY")) == 0)
                ggv_default_height = DEFAULT_WINDOW_HEIGHT;
        if((file_sel_width = gnome_config_get_int("Layout/FileSelWidth")) == 0)
                file_sel_width = DEFAULT_FILE_SEL_WIDTH;
        if((file_sel_height = gnome_config_get_int("Layout/FileSelHeight")) == 0)
                file_sel_height = DEFAULT_FILE_SEL_WIDTH;

        gnome_config_pop_prefix();
}

void
save_prefs(gchar *prefix)
{
        gnome_config_push_prefix(prefix);
        gnome_config_set_string("GhostScript/Interpreter", gs_cmd);
        gnome_config_set_string("GhostScript/ScanPDF", gs_scan_pdf_cmd);
        gnome_config_set_string("GhostScript/Ungzip", gs_ungzip_cmd);
        gnome_config_set_string("GhostScript/Unbzip2", gs_unbzip2_cmd);

        gnome_config_set_string("Printing/Command", gs_print_cmd);

        gnome_config_set_int("Document/Magnification", gs_default_magnification);
        gnome_config_set_string("Document/Media", papersizes[gtk_gs_get_default_page_media()].name);
        gnome_config_set_bool("Document/OverrideDocumentMedia", gtk_gs_get_default_override_media());
        gnome_config_set_bool("Document/Antialiasing", gtk_gs_get_default_antialiased());
        gnome_config_set_bool("Document/OverrideOrientation", 
                              gtk_gs_get_default_override_orientation());
        gnome_config_set_string("Document/Orientation", 
                                OrientationLabels[gtk_gs_get_default_orientation()]);
        gnome_config_set_bool("Document/Watch", gtk_gs_get_default_watch_doc());
        gnome_config_set_bool("Document/RespectEOF", gtk_gs_get_default_respect_eof());

        gnome_config_set_bool("Layout/ShowPanel", gs_panel);
	gnome_config_set_bool("Layout/ShowToolbar", gs_toolbar);
        gnome_config_set_bool("Layout/ShowMenubar", gs_menubar);
        gnome_config_set_bool("Layout/AutoJump", gs_auto_jump);
        gnome_config_set_bool("Layout/SaveGeometry", gs_save_geometry);
        if(gs_save_geometry) {
                gnome_config_set_int("Layout/SaveGeometryX", ggv_default_width);
                gnome_config_set_int("Layout/SaveGeometryY", ggv_default_height);
                gnome_config_set_int("Layout/FileSelWidth", file_sel_width);
                gnome_config_set_int("Layout/FileSelHeight", file_sel_height);
        }

        gnome_config_pop_prefix();

	gnome_config_sync();
}

void
set_prefs(ggv_window *ggv)
{
        GtkGS *gs = GTK_GS(ggv->gs);
        
        gs->override_media = gtk_gs_get_default_override_media();
        gs->default_page_media = gtk_gs_get_default_page_media();
        gs->antialiased = gtk_gs_get_default_antialiased();
        gs->watch_doc = gtk_gs_get_default_watch_doc();
        gs->fallback_orientation = gtk_gs_get_default_orientation();
        gs->respect_eof = gtk_gs_get_default_respect_eof();
        gtk_gs_set_override_orientation (gs, gtk_gs_get_default_override_orientation());
}

