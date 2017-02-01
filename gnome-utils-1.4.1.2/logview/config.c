/*  ----------------------------------------------------------------------

    Copyright (C) 1998  Cesar Miquel  (miquel@df.uba.ar)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    ---------------------------------------------------------------------- */


#include "config.h"
#include <gnome.h>
#include <gtk/gtk.h>
#include "logview.h"
#include <stdlib.h>

/* 
 * 	-------------------
 * 	     Defines
 * 	-------------------
 */

#define FIXED_12_FONT      _("-b&h-lucidatypewriter-medium-r-normal-*-12-*-*-*-*-*-*-*")
#define FIXED_12_BFONT     _("-b&h-lucidatypewriter-bold-r-normal-*-12-*-*-*-*-*-*-*")
#define FIXED_10_FONT      _("-b&h-lucidatypewriter-medium-r-normal-sans-10-*-*-*-*-*-*-*")
#define FIXED_10_BFONT     _("-b&h-lucidatypewriter-bold-r-normal-sans-10-*-*-*-*-*-*-*")

#define HELVETICA_14_FONT  _("-adobe-helvetica-medium-r-normal-*-14-*-*-*-*-*-*-*")
#define HELVETICA_14_BFONT _("-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*")
#define HELVETICA_12_FONT  _("-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*")
#define HELVETICA_12_BFONT _("-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*")
#define HELVETICA_10_FONT  _("-adobe-helvetica-medium-r-normal-*-10-*-*-*-*-*-*-*")
#define HELVETICA_10_BFONT _("-adobe-helvetica-bold-r-normal-*-10-*-*-*-*-*-*-*")

static GdkFont *
fontset_load (const char *font_name)
{
	GdkFont *font;

	font = gdk_fontset_load (font_name);
	if (font != NULL)
		return font;
	font = gdk_font_load ("fixed");
	if (font != NULL)
		return font;
	return gdk_font_load ("*");
}



/* ----------------------------------------------------------------------
   NAME:	CreateConfig
   DESCRIPTION:	Allocate memory for configuartion data and set default 
   		values.
   ---------------------------------------------------------------------- */

ConfigData *
CreateConfig(void)
{
  ConfigData *newcfg;
  GtkStyle  *cs;
  GdkColor black = {0, 0, 0, 0}, 
           green = {30000, 0, 50000, 0}, 
	   blue = {0, 0, 0, 65535}, 
	   blue1 = {0, 0, 0, 32768}, 
	   blue3 = {0, 7680, 36864, 65535}, 
	   gray25 = {0, 16384, 16384, 16384}, 
	   gray50 = {0, 32768, 32768, 32768}, 
	   gray75 = {0, 49152, 49152, 49152}, 
	   white = {0, 65535, 65535, 65535};


  newcfg = (ConfigData *)malloc (sizeof(ConfigData));
  if (newcfg == NULL)
    return (ConfigData *) NULL;

  /*  Init defaults */
  newcfg->vis = gdk_visual_get_system ();
  newcfg->cmap = gdk_colormap_get_system ();
  gdk_color_alloc (newcfg->cmap, &blue);
  gdk_color_alloc (newcfg->cmap, &blue1);
  gdk_color_alloc (newcfg->cmap, &blue3);
  gdk_color_alloc (newcfg->cmap, &gray25);
  gdk_color_alloc (newcfg->cmap, &gray75);
  gdk_color_alloc (newcfg->cmap, &gray50);
  gdk_color_alloc (newcfg->cmap, &green);
  gdk_color_alloc (newcfg->cmap, &white);
  gdk_color_alloc (newcfg->cmap, &black);

  newcfg->blue = blue;
  newcfg->blue1 = blue1;
  newcfg->blue3 = blue3;
  newcfg->gray25 = gray25;
  newcfg->gray50 = gray50;
  newcfg->gray75 = gray75;
  newcfg->green = green;
  newcfg->black = black;
  newcfg->white = white;

  /*  Set up fonts used */
  newcfg->headingb = fontset_load (HELVETICA_12_BFONT);
  newcfg->heading  = fontset_load (HELVETICA_12_FONT);
  newcfg->fixed    = fontset_load (FIXED_10_FONT);
  newcfg->fixedb   = fontset_load (FIXED_10_BFONT);
  newcfg->small    = fontset_load (HELVETICA_10_FONT);

  /*  Create styles */
  cs = newcfg->main_style = gtk_style_new ();
  
  /*   cs->bg[GTK_STATE_PRELIGHT].red   = 0; */
  /*   cs->bg[GTK_STATE_PRELIGHT].green = 0; */
  /*   cs->bg[GTK_STATE_PRELIGHT].blue  = 65535/2; */
  /*   cs->fg[GTK_STATE_PRELIGHT].red   = 65535; */
  /*   cs->fg[GTK_STATE_PRELIGHT].green = 65535; */
  /*   //  cs->fg[GTK_STATE_PRELIGHT].blue  = 65535; */
  /*   cs->bg[GTK_STATE_NORMAL].red   = (gushort) 65535.0 * 0.1; */
  /*   cs->bg[GTK_STATE_NORMAL].green = (gushort) 65535.0 * 0.15; */
  /*   cs->bg[GTK_STATE_NORMAL].blue  = (gushort) 65535.0 * 0.35; */
  /*   cs->fg[GTK_STATE_NORMAL].red   = (gushort) 65535; */
  /*   cs->fg[GTK_STATE_NORMAL].green = (gushort) 65535; */
  /*   cs->fg[GTK_STATE_NORMAL].blue  = (gushort) 65535; */
  /*   cs->fg[GTK_STATE_ACTIVE].red   = (gushort) 65535; */
  /*   cs->fg[GTK_STATE_ACTIVE].green = (gushort) 65535; */
  /*   cs->fg[GTK_STATE_ACTIVE].blue  = (gushort) 65535; */
  /*   cs->bg[GTK_STATE_ACTIVE].red   = (gushort) 65535.0 * 0.1; */
  /*   cs->bg[GTK_STATE_ACTIVE].green = (gushort) 65535.0 * 0.15; */
  /*   cs->bg[GTK_STATE_ACTIVE].blue  = (gushort) 65535.0 * 0.45; */
  /*   cs->white.red = 0; */
  /*   cs->white.green = 0; */
  /*   cs->white.blue = 0; */

  /*   for (i = 0; i < 5; i++) */
  /*   { */
  /*     cs->text[i] = cs->fg[i]; */
  /*     cs->base[i] = cs->white; */
  /*   } */

  gdk_font_unref (cs->font);
  newcfg->main_style->font = fontset_load (HELVETICA_10_FONT);

  /* Set default style */
#if 0
  gtk_rc_init();
  gtk_rc_add_widget_class_style (newcfg->main_style, "*");
#endif

  cs = newcfg->white_bg_style = gtk_style_new ();
  cs->bg[GTK_STATE_NORMAL].red   = (gushort) 65535;
  cs->bg[GTK_STATE_NORMAL].green = (gushort) 65535;
  cs->bg[GTK_STATE_NORMAL].blue  = (gushort) 65535;
  gdk_font_unref (cs->font);
  cs->font = fontset_load (HELVETICA_10_FONT);

  cs = newcfg->black_bg_style = gtk_style_new ();
  cs->bg[GTK_STATE_NORMAL].red   = (gushort) 0;
  cs->bg[GTK_STATE_NORMAL].green = (gushort) 0;
  cs->bg[GTK_STATE_NORMAL].blue  = (gushort) 0;
  cs->bg[GTK_STATE_NORMAL].pixel = black.pixel;
  gdk_font_unref (cs->font);
  cs->font = fontset_load (HELVETICA_10_FONT);

  /* Set paths */
  newcfg->regexp_db_path = NULL;
  newcfg->descript_db_path = NULL;
  newcfg->action_db_path = NULL;

  return (ConfigData *) newcfg;
}
