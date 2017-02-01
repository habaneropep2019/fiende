#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gtk/gtkrc.h>
#include <gtk/gtkthemes.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

/**************************************************************************
* GTK Metal Theme
*
* Version 0.9, Oct 2, 1998
*
* Copyright 1998: Randy Gordon, Integrand Systems
*                 http://www.integrand.com
*                 mailto://randy@integrand.com
*
* License: GPL (Gnu Public License)
*
*
**************************************************************************/

#define DEBUG 0

extern GtkStyleClass metal_default_class;
extern GtkStyleClass metal_special_class;
extern GdkFont     *default_font;
extern GSList      *unattached_styles;

extern GdkGC *metal_light_gray_gc;
extern GdkGC *metal_mid_gray_gc;
extern GdkGC *metal_dark_gray_gc;


