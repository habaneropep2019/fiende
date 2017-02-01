
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   code taken from guname. 
 *
 *   Copyright (C) 1998 Havoc Pennington <hp@pobox.com> except marquee code.
 *   Copyright (C) 2000 Red Hat Inc, <jrb@redhat.com>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <gnome.h>
#include <unistd.h>
#include <string.h> /* memset */
#include <sys/utsname.h>
#include <errno.h>
#include "splash.h"

/* The basic information to display in the main dialog. */
typedef enum {
  si_OS,     
  si_distribution, /* Debian, Solaris, etc. */
  si_CPU_type,
  si_user,
  si_host,
  end_system_info
} system_info;

#define DEBIAN_STRING "Debian GNU/Linux"
#define MANDRAKE_STRING "Mandrake-Linux"
#define REDHAT_STRING "Red Hat Linux"
#define SUSE_STRING "SuSE Linux"

static const gchar * descriptions[] = {
  N_("Operating System:"),
  N_("Distribution Version:"),
  N_("Processor Type:"),
  N_("User Name:"),
  N_("Host Name:"),
};

static const gchar *info[end_system_info];

static void get_portable_info(void);
static void get_linux_info(void);
static void load_system_info(void);


void load_system_info()
{
  static gboolean first_time = TRUE;

  if (first_time) {
    /* Set all the pointers in the array to NULL */
    memset(&info, '\0', sizeof(info));
    first_time = FALSE;
  }
  else {
    /* Clear out the array. */
    int i = 0;
    while ( i < end_system_info ) {
      if (info[i] != NULL) g_free((gpointer)info[i]);
      info[i] = NULL;
      ++i;
    }
  }

  get_portable_info();

  get_linux_info();
}

static void get_portable_info()
{
  struct utsname buf;

  if ( uname(&buf) == -1 ) {
    g_error("uname() failed, %s\n", g_unix_error_string(errno));
  }

  info[si_OS] = g_strdup_printf("%s    version: %s", buf.sysname, buf.release);
  info[si_CPU_type] = g_strdup(buf.machine);
  info[si_host] = g_strdup(buf.nodename);
  /*  info[si_domain] = g_strdup(buf.domainname); */ /* man page wrong? */

  info[si_user] = g_strdup(g_get_user_name());

  /* Add more later */

}

static void get_linux_info()
{
  /* Identify distribution (really this could be compiled in) */
  if (g_file_exists("/etc/debian_version")) {
    FILE * f;
    gchar buf[20];

    f = fopen("/etc/debian_version", "r");
    if (f) { 
      fscanf(f, "%16s", buf);
      info[si_distribution] = g_strdup(buf);
      fclose(f);
    }
  } else if (g_file_exists("/etc/conectiva-release")) {
    FILE *f;
    gchar buf[80];

    f = fopen("/etc/conectiva-release", "r");
    if (f) { 
      fgets(buf, 79, f);
      info[si_distribution] = g_strdup(buf);
      fclose(f);
    } 
  } else if (g_file_exists("/etc/mandrake-release")) {  
    FILE *f;
    gchar buf[80];

    f = fopen("/etc/mandrake-release", "r");
    if (f) {
      fgets(buf, 79, f);
      info[si_distribution] = g_strdup(buf);
      fclose(f);
    }
  } else if (g_file_exists("/etc/SuSE-release")) {
    FILE *f;
    gchar buf[80];

    f = fopen("/etc/SuSE-release", "r");
    if (f) { 
      fgets(buf, 79, f);
      info[si_distribution] = g_strdup(buf);
      fclose(f);
    }

  } else if (g_file_exists("/etc/redhat-release")) {
    FILE *f;
    gchar buf[80];

    f = fopen("/etc/redhat-release", "r");
    if (f) {
      fgets(buf, 79, f);
/* drmike - this seems unecessary in RH 5.2 and later
      if (strchr(buf, ' '))
        info[si_distribution_version] = g_strdup(strchr(buf, ' ')+1);
      else
*/
        info[si_distribution] = g_strdup(buf);
      fclose(f);
    }
  } else if (g_file_exists("/etc/SuSE-release")) {
    FILE *f;
	gchar buf[80];

	f = fopen("/etc/SuSE-release", "r");
	if (f) { 
	  fgets(buf, 79, f);
	  info[si_distribution] = g_strdup(buf);
	  fclose(f);
	}
  }		
  else {
    /* This is about the only case where we'll put Unknown instead
       of simply not showing the info. */
    info[si_distribution] = g_strdup(_("Unknown"));
  }

  /* More to come. */

}


#define SPLASH_WIDTH 540
#define SPLASH_HEIGHT 456
#define SPLASH_FONT_BOLD "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1,-*-*-bold-r-normal-*-*-120-*-*-*-*-*-*"
#define SPLASH_FONT_REG "-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1,-*-*-medium-r-normal-*-*-120-*-*-*-*-*-*"
GtkWidget *
create_splash_screen (void)
{
        GtkWidget *retval;
        GdkImlibImage *image;
	gint i;
	gint max_width;
	gint max_height;
	GdkFont *font;
	gchar *image_path;

	load_system_info ();

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
        retval = gnome_canvas_new ();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	
        gtk_widget_set_usize (retval, SPLASH_WIDTH, SPLASH_HEIGHT);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (retval),
                                        0.0, 0.0,
                                        SPLASH_WIDTH, SPLASH_HEIGHT);


        gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (retval)),
                               gnome_canvas_rect_get_type (),
                               "x1", 0.0,
                               "y1", 0.0,
                               "x2", (gfloat) SPLASH_WIDTH,
                               "y2", (gfloat) SPLASH_HEIGHT,
			       "fill_color", "#FFFFFF",
                               NULL);

        gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (retval)),
                               gnome_canvas_rect_get_type (),
                               "x1", 0.0,
                               "y1", 49.0,
                               "x2", (gfloat) SPLASH_WIDTH,
                               "y2", 49.0,
			       "fill_color", "#000000",
                               NULL);

	image_path = gnome_pixmap_file ("ccsplash.png");
		
	image = gdk_imlib_load_image (image_path);
        gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (retval)),
                               gnome_canvas_image_get_type (),
                               "image", image,
                               "x", 1.0,
                               "y", 1.0,
                               "width", (gfloat) SPLASH_WIDTH,
                               "height", 48.0,
                               "anchor", GTK_ANCHOR_NORTH_WEST,
                               NULL);
        gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (retval)),
                               gnome_canvas_text_get_type (),
                               "text", _("Control Center"),
                               "justification", GTK_JUSTIFY_CENTER,
                               "fontset", "-adobe-helvetica-bold-r-normal-*-*-240-*-*-p-*-iso8859-1,-*-*-bold-r-normal-*-*-240-*-*-p-*-*",
                               "x", SPLASH_WIDTH/2.0,
                               "y", 100.0,
                               "anchor", GTK_ANCHOR_CENTER,
                               NULL);

	font = gdk_fontset_load (SPLASH_FONT_BOLD);
	max_width = 0;
	max_height = 0;
	for (i = 0; i < end_system_info; i++) {
		max_width = MAX (max_width, gdk_string_width (font, _(descriptions[i])));
		max_height = MAX (max_height, gdk_string_height (font, _(descriptions[i])));
	}
	gdk_font_unref (font);
	for (i = 0; i < end_system_info; i++) {
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (retval)),
				       gnome_canvas_text_get_type (),
				       "text", _(descriptions[i]),
				       "justification", GTK_JUSTIFY_CENTER,
				       "fontset", SPLASH_FONT_BOLD,
				       "x", 5.0,
				       "y", 150.0 + i * (max_height + 4.0),
				       "anchor", GTK_ANCHOR_NORTH_WEST,
				       NULL);
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (retval)),
				       gnome_canvas_text_get_type (),
				       "text", _(info[i]),
				       "justification", GTK_JUSTIFY_CENTER,
				       "fontset", SPLASH_FONT_REG,
				       "x", 5.0 + max_width + 5.0,
				       "y", 150.0 + i * (max_height + 4.0),
				       "anchor", GTK_ANCHOR_NORTH_WEST,
				       NULL);
	}
        gtk_widget_show_all (retval);
        return retval;
}
