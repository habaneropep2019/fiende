#include <math.h>
#include <gtk/gtk.h>
#include "metal_theme.h"

#define DETAIL(xx)   ((detail) && (!strcmp(xx, detail)))

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

/* internal functions */
static void         draw_hline(GtkStyle * style,
			       GdkWindow * window,
			       GtkStateType state_type,
			       GdkRectangle * area,
			       GtkWidget * widget,
			       gchar * detail,
			       gint x1,
			       gint x2,
			       gint y);
static void         draw_vline(GtkStyle * style,
			       GdkWindow * window,
			       GtkStateType state_type,
			       GdkRectangle * area,
			       GtkWidget * widget,
			       gchar * detail,
			       gint y1,
			       gint y2,
			       gint x);
static void         draw_shadow(GtkStyle * style,
				GdkWindow * window,
				GtkStateType state_type,
				GtkShadowType shadow_type,
				GdkRectangle * area,
				GtkWidget * widget,
				gchar * detail,
				gint x,
				gint y,
				gint width,
				gint height);

static void         draw_polygon(GtkStyle * style,
				 GdkWindow * window,
				 GtkStateType state_type,
				 GtkShadowType shadow_type,
				 GdkRectangle * area,
				 GtkWidget * widget,
				 gchar * detail,
				 GdkPoint * point,
				 gint npoints,
				 gint fill);
static void         draw_arrow(GtkStyle * style,
			       GdkWindow * window,
			       GtkStateType state_type,
			       GtkShadowType shadow_type,
			       GdkRectangle * area,
			       GtkWidget * widget,
			       gchar * detail,
			       GtkArrowType arrow_type,
			       gint fill,
			       gint x,
			       gint y,
			       gint width,
			       gint height);
static void         draw_diamond(GtkStyle * style,
				 GdkWindow * window,
				 GtkStateType state_type,
				 GtkShadowType shadow_type,
				 GdkRectangle * area,
				 GtkWidget * widget,
				 gchar * detail,
				 gint x,
				 gint y,
				 gint width,
				 gint height);
static void         draw_oval(GtkStyle * style,
			      GdkWindow * window,
			      GtkStateType state_type,
			      GtkShadowType shadow_type,
			      GdkRectangle * area,
			      GtkWidget * widget,
			      gchar * detail,
			      gint x,
			      gint y,
			      gint width,
			      gint height);
static void         draw_string(GtkStyle * style,
				GdkWindow * window,
				GtkStateType state_type,
				GdkRectangle * area,
				GtkWidget * widget,
				gchar * detail,
				gint x,
				gint y,
				const gchar * string);
static void         draw_box(GtkStyle * style,
			     GdkWindow * window,
			     GtkStateType state_type,
			     GtkShadowType shadow_type,
			     GdkRectangle * area,
			     GtkWidget * widget,
			     gchar * detail,
			     gint x,
			     gint y,
			     gint width,
			     gint height);
static void         draw_flat_box(GtkStyle * style,
				  GdkWindow * window,
				  GtkStateType state_type,
				  GtkShadowType shadow_type,
				  GdkRectangle * area,
				  GtkWidget * widget,
				  gchar * detail,
				  gint x,
				  gint y,
				  gint width,
				  gint height);
static void         draw_check(GtkStyle * style,
			       GdkWindow * window,
			       GtkStateType state_type,
			       GtkShadowType shadow_type,
			       GdkRectangle * area,
			       GtkWidget * widget,
			       gchar * detail,
			       gint x,
			       gint y,
			       gint width,
			       gint height);
static void         draw_option(GtkStyle * style,
				GdkWindow * window,
				GtkStateType state_type,
				GtkShadowType shadow_type,
				GdkRectangle * area,
				GtkWidget * widget,
				gchar * detail,
				gint x,
				gint y,
				gint width,
				gint height);
static void         draw_cross(GtkStyle * style,
			       GdkWindow * window,
			       GtkStateType state_type,
			       GtkShadowType shadow_type,
			       GdkRectangle * area,
			       GtkWidget * widget,
			       gchar * detail,
			       gint x,
			       gint y,
			       gint width,
			       gint height);
static void         draw_ramp(GtkStyle * style,
			      GdkWindow * window,
			      GtkStateType state_type,
			      GtkShadowType shadow_type,
			      GdkRectangle * area,
			      GtkWidget * widget,
			      gchar * detail,
			      GtkArrowType arrow_type,
			      gint x,
			      gint y,
			      gint width,
			      gint height);
static void         draw_tab(GtkStyle * style,
			     GdkWindow * window,
			     GtkStateType state_type,
			     GtkShadowType shadow_type,
			     GdkRectangle * area,
			     GtkWidget * widget,
			     gchar * detail,
			     gint x,
			     gint y,
			     gint width,
			     gint height);
static void         draw_shadow_gap(GtkStyle * style,
				    GdkWindow * window,
				    GtkStateType state_type,
				    GtkShadowType shadow_type,
				    GdkRectangle * area,
				    GtkWidget * widget,
				    gchar * detail,
				    gint x,
				    gint y,
				    gint width,
				    gint height,
				    GtkPositionType gap_side,
				    gint gap_x,
				    gint gap_width);
static void         draw_box_gap(GtkStyle * style,
				 GdkWindow * window,
				 GtkStateType state_type,
				 GtkShadowType shadow_type,
				 GdkRectangle * area,
				 GtkWidget * widget,
				 gchar * detail,
				 gint x,
				 gint y,
				 gint width,
				 gint height,
				 GtkPositionType gap_side,
				 gint gap_x,
				 gint gap_width);
static void         draw_extension(GtkStyle * style,
				   GdkWindow * window,
				   GtkStateType state_type,
				   GtkShadowType shadow_type,
				   GdkRectangle * area,
				   GtkWidget * widget,
				   gchar * detail,
				   gint x,
				   gint y,
				   gint width,
				   gint height,
				   GtkPositionType gap_side);
static void         draw_focus(GtkStyle * style,
			       GdkWindow * window,
			       GdkRectangle * area,
			       GtkWidget * widget,
			       gchar * detail,
			       gint x,
			       gint y,
			       gint width,
			       gint height);
static void         draw_slider(GtkStyle * style,
				GdkWindow * window,
				GtkStateType state_type,
				GtkShadowType shadow_type,
				GdkRectangle * area,
				GtkWidget * widget,
				gchar * detail,
				gint x,
				gint y,
				gint width,
				gint height,
				GtkOrientation orientation);
static void         draw_handle(GtkStyle * style,
				GdkWindow * window,
				GtkStateType state_type,
				GtkShadowType shadow_type,
				GdkRectangle * area,
				GtkWidget * widget,
				gchar * detail,
				gint x,
				gint y,
				gint width,
				gint height,
				GtkOrientation orientation);

static int          fuzzy_match(int i,
				int j,
				int fudge);

static void
metal_arrow(GdkWindow *window, 
            GtkWidget *widget, 
            GdkGC *gc, 
            GtkArrowType arrow_type,
            gint x, 
            gint y, 
            gint width, 
            gint height);
static void
metal_scrollbar_trough(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);
static void
metal_scrollbar_slider(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);
static void
metal_scale_trough(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);
static void
metal_scale_slider(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height,
              GtkOrientation orientation);
static void
metal_menu(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);
static void
metal_menu_item(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);
static void
metal_notebook(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);
static void
metal_tab(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);
static void
get_tab_status(GtkNotebook *notebook,
               int x, int y, int *position, int *selected);
static void
metal_button(GtkStyle * style,
              GdkWindow * window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle * area,
              GtkWidget * widget,
              gchar * detail,
              gint x,
              gint y,
              gint width,
              gint height);

/* internal data structs */

GtkStyleClass       metal_default_class =
{
  2,
  2,
  draw_hline,
  draw_vline,
  draw_shadow,
  draw_polygon,
  draw_arrow,
  draw_diamond,
  draw_oval,
  draw_string,
  draw_box,
  draw_flat_box,
  draw_check,
  draw_option,
  draw_cross,
  draw_ramp,
  draw_tab,
  draw_shadow_gap,
  draw_box_gap,
  draw_extension,
  draw_focus,
  draw_slider,
  draw_handle
};

GtkStyleClass       metal_special_class =
{
  0,
  0,
  draw_hline,
  draw_vline,
  draw_shadow,
  draw_polygon,
  draw_arrow,
  draw_diamond,
  draw_oval,
  draw_string,
  draw_box,
  draw_flat_box,
  draw_check,
  draw_option,
  draw_cross,
  draw_ramp,
  draw_tab,
  draw_shadow_gap,
  draw_box_gap,
  draw_extension,
  draw_focus,
  draw_slider,
  draw_handle
};

static void
draw_hline(GtkStyle * style,
	   GdkWindow * window,
	   GtkStateType state_type,
	   GdkRectangle * area,
	   GtkWidget * widget,
	   gchar * detail,
	   gint x1,
	   gint x2,
	   gint y)
{
   gint thickness_light;
   gint thickness_dark;
   gint i;
   GdkGC *lightgc, *darkgc;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

   thickness_light = style->klass->ythickness / 2;
   thickness_dark = style->klass->ythickness - thickness_light;

   lightgc = style->light_gc[state_type];
   darkgc  = style->dark_gc[state_type];

   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
   }

   for (i = 0; i < thickness_dark; i++) {
      gdk_draw_line(window, lightgc, x2 - i - 1, y + i, x2, y + i);
      gdk_draw_line(window, darkgc, x1, y + i, x2 - i - 1, y + i);
   }

   y += thickness_dark;
   for (i = 0; i < thickness_light; i++) {
      gdk_draw_line(window, darkgc, x1, y + i, x1 + thickness_light - i - 1, y + i);
      gdk_draw_line(window, lightgc, x1 + thickness_light - i - 1, y + i, x2, y + i);
   }

   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
   }
}
/**************************************************************************/
static void
draw_vline(GtkStyle * style,
	   GdkWindow * window,
	   GtkStateType state_type,
	   GdkRectangle * area,
	   GtkWidget * widget,
	   gchar * detail,
	   gint y1,
	   gint y2,
	   gint x)
{
   gint thickness_light;
   gint thickness_dark;
   gint i;
   GdkGC *lightgc, *darkgc;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

   thickness_light = style->klass->xthickness / 2;
   thickness_dark = style->klass->xthickness - thickness_light;

   lightgc = style->light_gc[state_type];
   darkgc  = style->dark_gc[state_type];

   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
   }

   for (i = 0; i < thickness_dark; i++) {
      gdk_draw_line(window, lightgc, x + i, y2 - i - 1, x + i, y2);
      gdk_draw_line(window, darkgc, x + i, y1, x + i, y2 - i - 1);
   }

   x += thickness_dark;

   for (i = 0; i < thickness_light; i++) {
      gdk_draw_line(window, darkgc, x + i, y1, x + i, y1 + thickness_light - i);
      gdk_draw_line(window, lightgc, x + i, y1 + thickness_light - i, x + i, y2);
   }

   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
   }
}
/**************************************************************************/
static void
draw_shadow(GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    gchar * detail,
	    gint x,
	    gint y,
	    gint width,
	    gint height)
{
   GdkGC              *gc1, *gc2, *gc3, *gc4;
   gint                thickness_light;
   gint                thickness_dark;
   gint                i;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

/* return; */
#if DEBUG
  printf("draw_shadow: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);
#endif

   if (shadow_type == GTK_SHADOW_NONE)
     return;

   if ((width == -1) && (height == -1)) gdk_window_get_size(window, &width, &height);
   else if (width == -1)                gdk_window_get_size(window, &width, NULL);
   else if (height == -1)               gdk_window_get_size(window, NULL, &height);

   /* Override shadow-type for Metal button */
   if (DETAIL("button") || DETAIL("buttondefault")) 
      shadow_type = GTK_SHADOW_ETCHED_IN;
   if (DETAIL("optionmenu"))
      shadow_type = GTK_SHADOW_ETCHED_IN;
   if (DETAIL("handlebox_bin"))
      shadow_type = GTK_SHADOW_ETCHED_IN;

   /* Short-circuit some metal styles for now */
   if (DETAIL("frame")) {
      gc1 = style->dark_gc[state_type];
      if (area) gdk_gc_set_clip_rectangle(gc1, area);
      gdk_draw_rectangle(window, gc1, FALSE, x, y, width-1, height-1);
      if (area) gdk_gc_set_clip_rectangle(gc1, NULL);
      return; /* tbd */
   }
   if (DETAIL("optionmenutab")) {
      gc1 = style->black_gc;
      if (area) gdk_gc_set_clip_rectangle(gc1, area);
      gdk_draw_line(window, gc1, x, y, x+10, y);
      gdk_draw_line(window, gc1, x+1, y+1, x+9, y+1);
      gdk_draw_line(window, gc1, x+2, y+2, x+8, y+2);
      gdk_draw_line(window, gc1, x+3, y+3, x+7, y+3);
      gdk_draw_line(window, gc1, x+4, y+4, x+6, y+4);
      gdk_draw_line(window, gc1, x+5, y+5, x+5, y+4);
      if (area) gdk_gc_set_clip_rectangle(gc1, NULL);
      return; /* tbd */
   }

   switch (shadow_type) {
   case GTK_SHADOW_NONE:
     /* Handled above */
   case GTK_SHADOW_IN:
   case GTK_SHADOW_ETCHED_IN:
      gc1 = style->light_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->black_gc;
      gc4 = style->bg_gc[state_type];
      break;
   case GTK_SHADOW_OUT:
   case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->dark_gc[state_type];
      gc2 = style->light_gc[state_type];
      gc3 = style->black_gc;
      gc4 = style->bg_gc[state_type];
      break;
   }

   if (area) {
      gdk_gc_set_clip_rectangle(gc1, area);
      gdk_gc_set_clip_rectangle(gc2, area);
      gdk_gc_set_clip_rectangle(gc3, area);
      gdk_gc_set_clip_rectangle(gc4, area);
   }

   switch (shadow_type) {
   case GTK_SHADOW_NONE:
      break;
   case GTK_SHADOW_IN:
      gdk_draw_line(window, gc1,
		    x, y + height - 1, x + width - 1, y + height - 1);
      gdk_draw_line(window, gc1,
		    x + width - 1, y, x + width - 1, y + height - 1);

      gdk_draw_line(window, gc4,
		    x + 1, y + height - 2, x + width - 2, y + height - 2);
      gdk_draw_line(window, gc4,
		    x + width - 2, y + 1, x + width - 2, y + height - 2);

      gdk_draw_line(window, gc3,
		    x + 1, y + 1, x + width - 2, y + 1);
      gdk_draw_line(window, gc3,
		    x + 1, y + 1, x + 1, y + height - 2);

      gdk_draw_line(window, gc2,
		    x, y, x + width - 1, y);
      gdk_draw_line(window, gc2,
		    x, y, x, y + height - 1);
      break;

   case GTK_SHADOW_OUT:
      gdk_draw_line(window, gc1,
		    x + 1, y + height - 2, x + width - 2, y + height - 2);
      gdk_draw_line(window, gc1,
		    x + width - 2, y + 1, x + width - 2, y + height - 2);

      gdk_draw_line(window, gc2,
		    x, y, x + width - 1, y);
      gdk_draw_line(window, gc2,
		    x, y, x, y + height - 1);

      gdk_draw_line(window, gc4,
		    x + 1, y + 1, x + width - 2, y + 1);
      gdk_draw_line(window, gc4,
		    x + 1, y + 1, x + 1, y + height - 2);

      gdk_draw_line(window, gc3,
		    x, y + height - 1, x + width - 1, y + height - 1);
      gdk_draw_line(window, gc3,
		    x + width - 1, y, x + width - 1, y + height - 1);
      break;
   case GTK_SHADOW_ETCHED_IN:
   case GTK_SHADOW_ETCHED_OUT:
      thickness_light = 1;
      thickness_dark = 1;

      for (i = 0; i < thickness_dark; i++)
	{
	  gdk_draw_line(window, gc1,
			x + i,
			y + height - i - 1,
			x + width - i - 1,
			y + height - i - 1);
	  gdk_draw_line(window, gc1,
			x + width - i - 1,
			y + i,
			x + width - i - 1,
			y + height - i - 1);

	  gdk_draw_line(window, gc2,
			x + i,
			y + i,
			x + width - i - 2,
			y + i);
	  gdk_draw_line(window, gc2,
			x + i,
			y + i,
			x + i,
			y + height - i - 2);
	}

      for (i = 0; i < thickness_light; i++) {
	  gdk_draw_line(window, gc1,
			x + thickness_dark + i,
			y + thickness_dark + i,
			x + width - thickness_dark - i - 1,
			y + thickness_dark + i);
	  gdk_draw_line(window, gc1,
			x + thickness_dark + i,
			y + thickness_dark + i,
			x + thickness_dark + i,
			y + height - thickness_dark - i - 1);

	  gdk_draw_line(window, gc2,
			x + thickness_dark + i,
			y + height - thickness_light - i - 1,
			x + width - thickness_light - 1,
			y + height - thickness_light - i - 1);
	  gdk_draw_line(window, gc2,
			x + width - thickness_light - i - 1,
			y + thickness_dark + i,
			x + width - thickness_light - i - 1,
			y + height - thickness_light - 1);
	}
      break;
   }

   if (area) {
      gdk_gc_set_clip_rectangle(gc1, NULL);
      gdk_gc_set_clip_rectangle(gc2, NULL);
      gdk_gc_set_clip_rectangle(gc3, NULL);
      gdk_gc_set_clip_rectangle(gc4, NULL);
   }
}
/**************************************************************************/
static void
draw_polygon(GtkStyle * style,
	     GdkWindow * window,
	     GtkStateType state_type,
	     GtkShadowType shadow_type,
	     GdkRectangle * area,
	     GtkWidget * widget,
	     gchar * detail,
	     GdkPoint * points,
	     gint npoints,
	     gint fill)
{
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif /* M_PI */
#ifndef M_PI_4
#define M_PI_4  0.78539816339744830962
#endif /* M_PI_4 */

  static const gdouble pi_over_4 = M_PI_4;
  static const gdouble pi_3_over_4 = M_PI_4 * 3;

  GdkGC              *gc1;
  GdkGC              *gc2;
  GdkGC              *gc3;
  GdkGC              *gc4;
  gdouble             angle;
  gint                xadjust;
  gint                yadjust;
  gint                i;

  g_return_if_fail(style != NULL);
  g_return_if_fail(window != NULL);
  g_return_if_fail(points != NULL);

#if DEBUG
  printf("draw_polygon: %p %p %s\n", widget, window, detail);
#endif

  switch (shadow_type) {
    case GTK_SHADOW_IN:
      gc1 = style->bg_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->light_gc[state_type];
      gc4 = style->black_gc;
      break;
    case GTK_SHADOW_ETCHED_IN:
      gc1 = style->light_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->dark_gc[state_type];
      gc4 = style->light_gc[state_type];
      break;
    case GTK_SHADOW_OUT:
      gc1 = style->dark_gc[state_type];
      gc2 = style->light_gc[state_type];
      gc3 = style->black_gc;
      gc4 = style->bg_gc[state_type];
      break;
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->dark_gc[state_type];
      gc2 = style->light_gc[state_type];
      gc3 = style->light_gc[state_type];
      gc4 = style->dark_gc[state_type];
      break;
    default:
      return;
    }

  if (area) {
      gdk_gc_set_clip_rectangle(gc1, area);
      gdk_gc_set_clip_rectangle(gc2, area);
      gdk_gc_set_clip_rectangle(gc3, area);
      gdk_gc_set_clip_rectangle(gc4, area);
    }

  if (fill) gdk_draw_polygon(window, style->bg_gc[state_type], TRUE, points, npoints);

  npoints--;

  for (i = 0; i < npoints; i++) {
      if ((points[i].x == points[i + 1].x) &&
	  (points[i].y == points[i + 1].y)) {
	  angle = 0;
	}
      else {
	  angle = atan2(points[i + 1].y - points[i].y,
			points[i + 1].x - points[i].x);
	}

      if ((angle > -pi_3_over_4) && (angle < pi_over_4)) {
	  if (angle > -pi_over_4) {
	      xadjust = 0;
	      yadjust = 1;
	    }
	  else {
	      xadjust = 1;
	      yadjust = 0;
	    }

	  gdk_draw_line(window, gc1,
			points[i].x - xadjust, points[i].y - yadjust,
			points[i + 1].x - xadjust, points[i + 1].y - yadjust);
	  gdk_draw_line(window, gc3,
			points[i].x, points[i].y,
			points[i + 1].x, points[i + 1].y);
	}
      else {
	  if ((angle < -pi_3_over_4) || (angle > pi_3_over_4)) {
	      xadjust = 0;
	      yadjust = 1;
	    }
	  else {
	      xadjust = 1;
	      yadjust = 0;
	    }

	  gdk_draw_line(window, gc4,
			points[i].x + xadjust, points[i].y + yadjust,
			points[i + 1].x + xadjust, points[i + 1].y + yadjust);
	  gdk_draw_line(window, gc2,
			points[i].x, points[i].y,
			points[i + 1].x, points[i + 1].y);
	}
    }
  if (area) {
      gdk_gc_set_clip_rectangle(gc1, NULL);
      gdk_gc_set_clip_rectangle(gc2, NULL);
      gdk_gc_set_clip_rectangle(gc3, NULL);
      gdk_gc_set_clip_rectangle(gc4, NULL);
    }
}
/**************************************************************************/
static void
draw_arrow(GtkStyle * style,
	   GdkWindow * window,
	   GtkStateType state_type,
	   GtkShadowType shadow_type,
	   GdkRectangle * area,
	   GtkWidget * widget,
	   gchar * detail,
	   GtkArrowType arrow_type,
	   gint fill,
	   gint x,
	   gint y,
	   gint width,
	   gint height)
{
   GdkGC              *gc;
   gint                half_width;
   gint                half_height;
   gint                xthik, ythik;
   GdkPoint            points[3];
   gchar               border = 1;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

   if ((width == -1) && (height == -1)) gdk_window_get_size(window, &width, &height);
   else if (width == -1)                gdk_window_get_size(window, &width, NULL);
   else if (height == -1)               gdk_window_get_size(window, NULL, &height);

   xthik = style->klass->xthickness;
   ythik = style->klass->ythickness;

   gc = style->black_gc;

   if (DETAIL("menuitem")) {
      border = 0;
      gc = style->fg_gc[state_type];
   }

   if (area) gdk_gc_set_clip_rectangle(gc, area);

   if ((fill) && (border)) {
      draw_box(style, window, state_type, GTK_SHADOW_ETCHED_IN, area, widget, detail,
               x, y, width, height);
   }

   metal_arrow(window, widget, gc, arrow_type, x, y, width, height);

   if (area) gdk_gc_set_clip_rectangle(gc, NULL);
}
/**************************************************************************/
static void
metal_arrow(GdkWindow *window, GtkWidget *widget, GdkGC *gc, GtkArrowType arrow_type,
                   gint x, gint y, gint width, gint height)
{
   int base, span, xoffset, yoffset;
   int i;

   switch (arrow_type) {
   case GTK_ARROW_UP:
      base    = width/2;
      if (base%2 == 0) base++;
      xoffset = (width-base)/2;
      span = base/2 + 1;
      yoffset = (height+span)/2-1;
      for (i=0; i<span; i++) {
         gdk_draw_line(window, gc, x+xoffset+i, y+yoffset-i, 
                                   x+xoffset+base-1-i, y+yoffset-i);
      }
      break;
   case GTK_ARROW_DOWN:
      base    = width/2;
      if (base%2 == 0) base++;
      xoffset = (width-base)/2;
      span = base/2 + 1;
      yoffset = (height-span)/2;
      for (i=0; i<span; i++) {
         gdk_draw_line(window, gc, x+xoffset+i, y+yoffset+i, 
                                   x+xoffset+base-1-i, y+yoffset+i);
      }
      break;
   case GTK_ARROW_RIGHT:
      if (GTK_CHECK_TYPE(widget, gtk_menu_item_get_type())) {
         base = 7;
      }
      else {
         base = height/2;
      }
      if (base%2 == 0) base++;
      yoffset = (height-base)/2;
      span = base/2 + 1;
      xoffset = (width-span)/2;
      for (i=0; i<span; i++) {
         gdk_draw_line(window, gc, x+xoffset+i, y+yoffset+i, 
                                   x+xoffset+i, y+yoffset+base-1-i);
      }
      break;
   case GTK_ARROW_LEFT:
      base    = height/2;
      if (base%2 == 0) base++;
      yoffset = (height-base)/2;
      span = base/2 + 1;
      xoffset = (width+span)/2-1;
      for (i=0; i<span; i++) {
         gdk_draw_line(window, gc, x+xoffset-i, y+yoffset+i, 
                                   x+xoffset-i, y+yoffset+base-1-i);
      }
      break;
   }
}
/**************************************************************************/
static void
draw_diamond(GtkStyle * style,
	     GdkWindow * window,
	     GtkStateType state_type,
	     GtkShadowType shadow_type,
	     GdkRectangle * area,
	     GtkWidget * widget,
	     gchar * detail,
	     gint x,
	     gint y,
	     gint width,
	     gint height)
{
   gint                half_width;
   gint                half_height;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

   if ((width == -1) && (height == -1)) gdk_window_get_size(window, &width, &height);
   else if (width == -1)                gdk_window_get_size(window, &width, NULL);
   else if (height == -1)               gdk_window_get_size(window, NULL, &height);

   half_width = width / 2;
   half_height = height / 2;

   if (area) {
      gdk_gc_set_clip_rectangle(style->light_gc[state_type], area);
      gdk_gc_set_clip_rectangle(style->bg_gc[state_type], area);
      gdk_gc_set_clip_rectangle(style->dark_gc[state_type], area);
      gdk_gc_set_clip_rectangle(style->black_gc, area);
   }

   switch (shadow_type) {
   case GTK_SHADOW_IN:
      gdk_draw_line(window, style->bg_gc[state_type],
		    x + 2, y + half_height,
		    x + half_width, y + height - 2);
      gdk_draw_line(window, style->bg_gc[state_type],
		    x + half_width, y + height - 2,
		    x + width - 2, y + half_height);
      gdk_draw_line(window, style->light_gc[state_type],
		    x + 1, y + half_height,
		    x + half_width, y + height - 1);
      gdk_draw_line(window, style->light_gc[state_type],
		    x + half_width, y + height - 1,
		    x + width - 1, y + half_height);
      gdk_draw_line(window, style->light_gc[state_type],
		    x, y + half_height,
		    x + half_width, y + height);
      gdk_draw_line(window, style->light_gc[state_type],
		    x + half_width, y + height,
		    x + width, y + half_height);

      gdk_draw_line(window, style->black_gc,
		    x + 2, y + half_height,
		    x + half_width, y + 2);
      gdk_draw_line(window, style->black_gc,
		    x + half_width, y + 2,
		    x + width - 2, y + half_height);
      gdk_draw_line(window, style->dark_gc[state_type],
		    x + 1, y + half_height,
		    x + half_width, y + 1);
      gdk_draw_line(window, style->dark_gc[state_type],
		    x + half_width, y + 1,
		    x + width - 1, y + half_height);
      gdk_draw_line(window, style->dark_gc[state_type],
		    x, y + half_height,
		    x + half_width, y);
      gdk_draw_line(window, style->dark_gc[state_type],
		    x + half_width, y,
		    x + width, y + half_height);
      break;
   case GTK_SHADOW_OUT:
      gdk_draw_line(window, style->dark_gc[state_type],
		    x + 2, y + half_height,
		    x + half_width, y + height - 2);
      gdk_draw_line(window, style->dark_gc[state_type],
		    x + half_width, y + height - 2,
		    x + width - 2, y + half_height);
      gdk_draw_line(window, style->dark_gc[state_type],
		    x + 1, y + half_height,
		    x + half_width, y + height - 1);
      gdk_draw_line(window, style->dark_gc[state_type],
		    x + half_width, y + height - 1,
		    x + width - 1, y + half_height);
      gdk_draw_line(window, style->black_gc,
		    x, y + half_height,
		    x + half_width, y + height);
      gdk_draw_line(window, style->black_gc,
		    x + half_width, y + height,
		    x + width, y + half_height);

      gdk_draw_line(window, style->bg_gc[state_type],
		    x + 2, y + half_height,
		    x + half_width, y + 2);
      gdk_draw_line(window, style->bg_gc[state_type],
		    x + half_width, y + 2,
		    x + width - 2, y + half_height);
      gdk_draw_line(window, style->light_gc[state_type],
		    x + 1, y + half_height,
		    x + half_width, y + 1);
      gdk_draw_line(window, style->light_gc[state_type],
		    x + half_width, y + 1,
		    x + width - 1, y + half_height);
      gdk_draw_line(window, style->light_gc[state_type],
		    x, y + half_height,
		    x + half_width, y);
      gdk_draw_line(window, style->light_gc[state_type],
		    x + half_width, y,
		    x + width, y + half_height);
      break;
   default:
      break;
   }

   if (area) {
      gdk_gc_set_clip_rectangle(style->light_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle(style->bg_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle(style->dark_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle(style->black_gc, NULL);
   }
}
/**************************************************************************/
static void
draw_oval(GtkStyle * style,
	  GdkWindow * window,
	  GtkStateType state_type,
	  GtkShadowType shadow_type,
	  GdkRectangle * area,
	  GtkWidget * widget,
	  gchar * detail,
	  gint x,
	  gint y,
	  gint width,
	  gint height)
{
   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);
}
/**************************************************************************/
static void
draw_string(GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    gchar * detail,
	    gint x,
	    gint y,
	    const gchar * string)
{
   GdkGC *fggc, *whitegc, *midgc;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

#if DEBUG
  printf("draw_string: %p %p %s %i %i\n", widget, window, detail, x, y);
#endif

   if (DETAIL("label")) {
      fggc    = style->black_gc;
      whitegc = style->white_gc;
      midgc   = metal_mid_gray_gc;
   }
   else {
      fggc    = style->fg_gc[state_type];
      whitegc = style->white_gc;
      midgc   = metal_mid_gray_gc;
   }

   if (area) {
      gdk_gc_set_clip_rectangle(fggc,    area);
      gdk_gc_set_clip_rectangle(whitegc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
   }

   if (state_type == GTK_STATE_INSENSITIVE) {
      gdk_draw_string(window, style->font, whitegc, x + 1, y + 1, string);
      gdk_draw_string(window, style->font, midgc,   x, y, string);
   }
   else {
      gdk_draw_string(window, style->font, fggc, x, y, string);
   }

   if (area) {
      gdk_gc_set_clip_rectangle(fggc,    NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
   }
}

/**************************************************************************/
static void
draw_box(GtkStyle * style,
	 GdkWindow * window,
	 GtkStateType state_type,
	 GtkShadowType shadow_type,
	 GdkRectangle * area,
	 GtkWidget * widget,
	 gchar * detail,
	 gint x,
	 gint y,
	 gint width,
	 gint height)
{
   gint                xthik;
   gint                ythik;

   GtkWidget *parent;
   GtkWidget *child;
   GtkRange  *range;
   GList     *children;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

   if ((width == -1) && (height == -1)) gdk_window_get_size(window, &width, &height);
   else if (width == -1)                gdk_window_get_size(window, &width, NULL);
   else if (height == -1)               gdk_window_get_size(window, NULL, &height);

#if DEBUG
  printf("draw_box: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);
#endif

   /* ===================================================================== */

   if (widget && DETAIL("trough")) {
      GdkPixmap          *pm;
      gint                depth;

      if (GTK_IS_PROGRESS_BAR (widget)) {
	  if (area) gdk_gc_set_clip_rectangle(style->light_gc[GTK_STATE_NORMAL], area);
	  gdk_draw_rectangle(window, style->light_gc[GTK_STATE_NORMAL],
			     TRUE, x, y, width, height);
	  if (area) gdk_gc_set_clip_rectangle(style->light_gc[GTK_STATE_NORMAL], NULL);
	  gtk_paint_shadow(style, window, state_type, shadow_type, area, widget, detail,
			   x, y, width, height);
      }
      else if (GTK_IS_SCROLLBAR (widget))  {
           metal_scrollbar_trough(style, window, state_type, shadow_type,
                       area, widget, detail, x, y, width, height);
      }
      else if (GTK_IS_SCALE (widget))  {
           metal_scale_trough(style, window, state_type, shadow_type,
                       area, widget, detail, x, y, width, height);
      }
      else {
/*
	  xthik = style->klass->xthickness;
	  ythik = style->klass->ythickness;

	  gdk_window_get_geometry(window, NULL, NULL, NULL, NULL, &depth);
	  pm = gdk_pixmap_new(window, 2, 2, depth);

	  gdk_draw_point(pm, style->bg_gc[GTK_STATE_NORMAL], 0, 0);
	  gdk_draw_point(pm, style->bg_gc[GTK_STATE_NORMAL], 1, 1);
	  gdk_draw_point(pm, style->light_gc[GTK_STATE_NORMAL], 1, 0);
	  gdk_draw_point(pm, style->light_gc[GTK_STATE_NORMAL], 0, 1);
	  gdk_window_set_back_pixmap(window, pm, FALSE);
	  gdk_window_clear(window);

	  gdk_pixmap_unref(pm);
*/
      }
   }
   else if (DETAIL("menu")) {
      metal_menu(style, window, state_type, shadow_type,
                 area, widget, detail, x, y, width, height);
   }
   else if (DETAIL("menuitem")) {
      metal_menu_item(style, window, state_type, shadow_type,
                      area, widget, detail, x, y, width, height);
   }
   else if (DETAIL("bar")) {
      if (area) gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_SELECTED], area);
      gdk_draw_rectangle(window, style->bg_gc[GTK_STATE_SELECTED],
			 TRUE, x + 1, y + 1, width - 2, height - 2);
      if (area) gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_SELECTED], NULL);
   }
   else if (DETAIL("menubar")) {
      if (area) gdk_gc_set_clip_rectangle(style->bg_gc[state_type], area);
      gdk_draw_rectangle(window, style->bg_gc[state_type], TRUE,
			 x, y, width, height);
      if (area) gdk_gc_set_clip_rectangle(style->bg_gc[state_type], NULL);
   }
   else if (DETAIL("slider")) {
      if (GTK_CHECK_TYPE(widget, gtk_hscrollbar_get_type()) ||
          GTK_CHECK_TYPE(widget, gtk_vscrollbar_get_type()))  {
         metal_scrollbar_slider(style, window, state_type, shadow_type,
                                area, widget, detail, x, y, width, height);
      }
/*
      else if (GTK_CHECK_TYPE(widget, gtk_hscale_get_type()) ||
               GTK_CHECK_TYPE(widget, gtk_vscale_get_type()))  {
         metal_scale_slider(style, window, state_type, shadow_type,
                            area, widget, detail, x, y, width, height);
      }
*/
   }
   else if (DETAIL("notebook")) {
      metal_notebook(style, window, state_type, shadow_type,
                     area, widget, detail, x, y, width, height);
   }
   else if (DETAIL("tab")) {
      metal_tab(style, window, state_type, shadow_type,
                area, widget, detail, x, y, width, height);
   }
   else if (DETAIL("button") || DETAIL("togglebutton") || DETAIL("buttondefault")) {
      metal_button(style, window, state_type, shadow_type,
                   area, widget, detail, x, y, width, height);
   }
   else {
      if ((!style->bg_pixmap[state_type]) ||
          (gdk_window_get_type(window) == GDK_WINDOW_PIXMAP)) {
          if (area) gdk_gc_set_clip_rectangle(style->bg_gc[state_type], area);
          gdk_draw_rectangle(window, style->bg_gc[state_type], TRUE,
                            x, y, width, height);
          if (area) gdk_gc_set_clip_rectangle(style->bg_gc[state_type], NULL);
      }
      else {
	gtk_style_apply_default_pixmap(style, window, state_type, area, x, y, width, height);
      }
      gtk_paint_shadow(style, window, state_type, shadow_type, area, widget, detail,
		       x, y, width, height);
   }
}
/**************************************************************************/
static void 
metal_scrollbar_trough(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GdkGC *lightgc, *midgc, *darkgc, *whitegc;

   /* Get colors */
   lightgc = metal_light_gray_gc;
   midgc   = metal_mid_gray_gc;
   darkgc  = metal_dark_gray_gc;
   whitegc = style->white_gc;

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
      gdk_gc_set_clip_rectangle(whitegc, area);
   }

   /* Draw backgound */
   gdk_draw_rectangle(window, lightgc, TRUE, x, y, width, height);

   /* Draw border */
   gdk_draw_rectangle(window, darkgc,  FALSE, x, y, width-2, height-2);
   gdk_draw_rectangle(window, whitegc, FALSE, x+1, y+1, width-2, height-2);

   /* Draw inset shadow */
   if (GTK_CHECK_TYPE(widget, gtk_hscrollbar_get_type())) {
      gdk_draw_line(window, midgc, x+1, y+1, x+width-2, y+1);
   }
   else {
      gdk_draw_line(window, midgc, x+1, y+1, x+1, y+height-2);
   }

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
   }
}
/**************************************************************************/
static void 
metal_scrollbar_slider(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GdkPixmap          *pm;
   gint               depth;
   GdkRectangle       texturearea;
   GdkGC *fillgc;
   GdkGCValues values;
   GdkGC *lightgc, *midgc, *darkgc, *whitegc;
   int w, h;

   /* Get colors */
   if (state_type == GTK_STATE_PRELIGHT) {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc   = style->bg_gc[GTK_STATE_PRELIGHT];
      darkgc  = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
   }
   else {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc   = style->bg_gc[GTK_STATE_SELECTED];
      darkgc  = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
   }

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
      gdk_gc_set_clip_rectangle(whitegc, area);
   }

   /* Draw backgound */
   gdk_draw_rectangle(window, midgc, TRUE, x, y, width, height);

   /* Draw border */
   gdk_draw_rectangle(window, lightgc, FALSE, x+1, y+1, x+width-2, y+height-2);
   gdk_draw_rectangle(window, darkgc, FALSE, x+0, y+0, x+width-2, y+height-2);
   if (GTK_CHECK_TYPE(widget, gtk_hscrollbar_get_type())) {
      gdk_draw_line(window, whitegc, x+0, y+height-1, x+width-1, y+height-1);
      gdk_draw_line(window, midgc, x+width-1, y+1, x+width-1, y+height-2);
   }
   else {
      gdk_draw_line(window, whitegc, x+width-1, y+0, x+width-1, y+height-1);
      gdk_draw_line(window, midgc, x+0, y+height-1, x+width-2, y+height-1);
   }

   /* Draw textured surface */
   gdk_window_get_geometry(window, NULL, NULL, NULL, NULL, &depth);
   pm = gdk_pixmap_new(window, 4, 4, depth);

   gdk_draw_rectangle(pm, midgc, TRUE, 0, 0, 4, 4);
   if (state_type == GTK_STATE_PRELIGHT) {
      gdk_draw_point(pm, darkgc,     0, 0);
      gdk_draw_point(pm, whitegc,    1, 1);
      gdk_draw_point(pm, darkgc,     2, 2);
      gdk_draw_point(pm, whitegc,    3, 3);
   }
   else {
      gdk_draw_point(pm, darkgc,     0, 0);
      gdk_draw_point(pm, lightgc,    1, 1);
      gdk_draw_point(pm, darkgc,     2, 2);
      gdk_draw_point(pm, lightgc,    3, 3);
   }

   values.fill = GDK_TILED;
   values.ts_x_origin = 5;
   values.ts_y_origin = 3;
   fillgc = gdk_gc_new_with_values(window, &values, 
                    GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);
   if (area) gdk_gc_set_clip_rectangle(fillgc, area);
   gdk_gc_set_tile(fillgc, pm);
   if (GTK_CHECK_TYPE(widget, gtk_hscrollbar_get_type())) {
      w = width&1?width-11:width-10;
      h = height&1?height-7:height-8;
      gdk_draw_rectangle(window, fillgc, TRUE, x+5, y+3, w, h);
   }
   else {
      w = width&1?width-7:width-8;
      h = height&1?height-11:height-10;
      gdk_draw_rectangle(window, fillgc, TRUE, x+3, y+5, w, h);
   }
   gdk_gc_unref(fillgc);
   gdk_pixmap_unref(pm);

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
   }
}
/**************************************************************************/
static void 
metal_scale_trough(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GdkGC *lightgc, *midgc, *darkgc, *whitegc;

   /* Get colors */
   lightgc = metal_light_gray_gc;
   midgc   = style->bg_gc[GTK_STATE_SELECTED];
   darkgc  = metal_dark_gray_gc;
   whitegc = style->white_gc;

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
      gdk_gc_set_clip_rectangle(whitegc, area);
   }

   if (GTK_CHECK_TYPE(widget, gtk_hscale_get_type())) {
      /* Draw backgound */
      gdk_draw_rectangle(window, midgc, TRUE, x, y+4, width-2, 9);

      /* Draw border */
      gdk_draw_rectangle(window, darkgc,  FALSE, x, y+4, width-2, 7);
      gdk_draw_rectangle(window, whitegc, FALSE, x+1, y+5, width-2, 7);
   }
   else {
      /* Draw backgound */
      gdk_draw_rectangle(window, midgc, TRUE, x+4, y, 9, height-2);

      /* Draw border */
      gdk_draw_rectangle(window, darkgc,  FALSE, x+4, y, 7, height-2);
      gdk_draw_rectangle(window, whitegc, FALSE, x+5, y+1, 7, height-2);
   }

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
   }
}
/**************************************************************************/
static void 
metal_scale_slider(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height,
	      GtkOrientation orientation)
{
   GdkPixmap          *pm;
   gint               depth;
   GdkRectangle       texturearea;
   GdkGC *fillgc;
   GdkGCValues values;
   GdkGC *lightgc, *midgc, *darkgc, *whitegc, *blackgc;
   int w, h;
   int xx, yy;
   GdkPoint points[5];

   /* Get colors */
   if (state_type == GTK_STATE_PRELIGHT) {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc   = style->bg_gc[GTK_STATE_PRELIGHT];
      darkgc  = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
      blackgc = style->black_gc;
   }
   else {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc   = style->bg_gc[GTK_STATE_SELECTED];
      darkgc  = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
      blackgc = style->black_gc;
   }

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
      gdk_gc_set_clip_rectangle(whitegc, area);
      gdk_gc_set_clip_rectangle(blackgc, area);
      gdk_gc_set_clip_rectangle(metal_light_gray_gc, area);
   }

#if 1
   /* Draw backgound */
   gdk_draw_rectangle(window, midgc, TRUE, x, y, width, height);

   /* Draw border */
   gdk_draw_rectangle(window, lightgc, FALSE, x+1, y+1, x+width-2, y+height-2);
   gdk_draw_rectangle(window, darkgc, FALSE, x+0, y+0, x+width-2, y+height-2);
   if (GTK_CHECK_TYPE(widget, gtk_hscale_get_type())) {
      gdk_draw_line(window, whitegc, x+0, y+height-1, x+width-1, y+height-1);
      gdk_draw_line(window, midgc, x+width-1, y+1, x+width-1, y+height-2);
   }
   else {
      gdk_draw_line(window, whitegc, x+width-1, y+0, x+width-1, y+height-1);
      gdk_draw_line(window, midgc, x+0, y+height-1, x+width-2, y+height-1);
   }

   /* Draw textured surface */
   gdk_window_get_geometry(window, NULL, NULL, NULL, NULL, &depth);
   pm = gdk_pixmap_new(window, 4, 4, depth);

   gdk_draw_rectangle(pm, midgc, TRUE, 0, 0, 4, 4);
   if (state_type == GTK_STATE_PRELIGHT) {
      gdk_draw_point(pm, darkgc,     0, 0);
      gdk_draw_point(pm, whitegc,    1, 1);
      gdk_draw_point(pm, darkgc,     2, 2);
      gdk_draw_point(pm, whitegc,    3, 3);
   }
   else {
      gdk_draw_point(pm, darkgc,     0, 0);
      gdk_draw_point(pm, lightgc,    1, 1);
      gdk_draw_point(pm, darkgc,     2, 2);
      gdk_draw_point(pm, lightgc,    3, 3);
   }

   values.fill = GDK_TILED;
   values.ts_x_origin = 5;
   values.ts_y_origin = 3;
   fillgc = gdk_gc_new_with_values(window, &values, 
                    GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);
   if (area) gdk_gc_set_clip_rectangle(fillgc, area);
   gdk_gc_set_tile(fillgc, pm);
   if (GTK_CHECK_TYPE(widget, gtk_hscale_get_type())) {
      w = width&1?width-11:width-10;
      h = height&1?height-7:height-8;
      gdk_draw_rectangle(window, fillgc, TRUE, x+5, y+3, w, h);
   }
   else {
      w = width&1?width-7:width-8;
      h = height&1?height-11:height-10;
      gdk_draw_rectangle(window, fillgc, TRUE, x+3, y+5, w, h);
   }
   gdk_gc_unref(fillgc);
   gdk_pixmap_unref(pm);

   /* Draw middle line */
   if (GTK_CHECK_TYPE(widget, gtk_hscale_get_type())) {
      if (state_type == GTK_STATE_PRELIGHT) {
         gdk_draw_line(window, darkgc,  x+width/2,   y+2, x+width/2,   y+height-4);
         gdk_draw_line(window, whitegc, x+width/2+1, y+2, x+width/2+1, y+height-4);
      }
      else {
         gdk_draw_line(window, darkgc,  x+width/2,   y+2, x+width/2,   y+height-4);
         gdk_draw_line(window, lightgc, x+width/2+1, y+2, x+width/2+1, y+height-4);
      }
   }
   else {
      if (state_type == GTK_STATE_PRELIGHT) {
         gdk_draw_line(window, darkgc,  x+2, y+height/2,   x+width-4, y+height/2);
         gdk_draw_line(window, whitegc, x+2, y+height/2+1, x+width-4, y+height/2+1);
      }
      else {
         gdk_draw_line(window, darkgc,  x+2, y+height/2,   x+width-4, y+height/2);
         gdk_draw_line(window, lightgc, x+2, y+height/2+1, x+width-4, y+height/2+1);
      }
   }

#else
   /* The following code draws the sliders more faithfully to the 
      JL&F spec, but I think it looks bad. */

   /* Draw backgound */
   gdk_draw_rectangle(window, lightgc, TRUE, x, y, width-1, height-1);

   /* Draw textured surface */
   gdk_window_get_geometry(window, NULL, NULL, NULL, NULL, &depth);
   pm = gdk_pixmap_new(window, 4, 4, depth);

   gdk_draw_rectangle(pm, midgc, TRUE, 0, 0, 4, 4);
   if (state_type == GTK_STATE_PRELIGHT) {
      gdk_draw_point(pm, darkgc,     0, 0);
      gdk_draw_point(pm, whitegc,    1, 1);
      gdk_draw_point(pm, darkgc,     2, 2);
      gdk_draw_point(pm, whitegc,    3, 3);
   }
   else {
      gdk_draw_point(pm, darkgc,     0, 0);
      gdk_draw_point(pm, lightgc,    1, 1);
      gdk_draw_point(pm, darkgc,     2, 2);
      gdk_draw_point(pm, lightgc,    3, 3);
   }

   values.fill = GDK_TILED;
   values.ts_x_origin = 5;
   values.ts_y_origin = 3;
   fillgc = gdk_gc_new_with_values(window, &values, 
                    GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);
   if (area) gdk_gc_set_clip_rectangle(fillgc, area);
   gdk_gc_set_tile(fillgc, pm);
   w = width-4;
   h = height-4;
   gdk_draw_rectangle(window, fillgc, TRUE, x+2, y+2, w, h);
   gdk_gc_unref(fillgc);
   gdk_pixmap_unref(pm);

   if (GTK_CHECK_TYPE(widget, gtk_hscale_get_type())) {
      /* Draw border */
      points[0].x = x;
      points[0].y = y;
      points[1].x = x+14;
      points[1].y = y;
      points[2].x = x+14;
      points[2].y = y+7;
      points[3].x = x+7;
      points[3].y = y+14;
      points[4].x = x;
      points[4].y = y+7;
      gdk_draw_polygon(window, blackgc, FALSE, points, 5);
      points[0].x = x+1;
      points[0].y = y+1;
      points[1].x = x+13;
      points[1].y = y+1;
      points[2].x = x+13;
      points[2].y = y+7;
      points[3].x = x+7;
      points[3].y = y+13;
      points[4].x = x+1;
      points[4].y = y+7;
      gdk_draw_polygon(window, lightgc, FALSE, points, 5);
   
      /* Fix bottom corners */
      points[0].x = x;
      points[0].y = y+14;
      points[1].x = x;
      points[1].y = y+8;
      points[2].x = x+6;
      points[2].y = y+14;
      gdk_draw_polygon(window, metal_light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon(window, metal_light_gray_gc, TRUE,  points, 3);
      points[0].x = x+14;
      points[0].y = y+14;
      points[1].x = x+14;
      points[1].y = y+8;
      points[2].x = x+8;
      points[2].y = y+14;
      gdk_draw_polygon(window,   metal_light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon(window,   metal_light_gray_gc, TRUE,  points, 3);
      gdk_draw_rectangle(window, metal_light_gray_gc, TRUE,  x, y+15, width, height-15);
   }
   else {
      /* Draw border */
      points[0].x = x;
      points[0].y = y+7;
      points[1].x = x+7;
      points[1].y = y;
      points[2].x = x+14;
      points[2].y = y;
      points[3].x = x+14;
      points[3].y = y+14;
      points[4].x = x+7;
      points[4].y = y+14;
      gdk_draw_polygon(window, blackgc, FALSE, points, 5);

      points[0].x = x+1;
      points[0].y = y+7;
      points[1].x = x+7;
      points[1].y = y+1;
      points[2].x = x+13;
      points[2].y = y+1;
      points[3].x = x+13;
      points[3].y = y+13;
      points[4].x = x+7;
      points[4].y = y+13;
      gdk_draw_polygon(window, lightgc, FALSE, points, 5);
   
      /* Fix corners */
      points[0].x = x;
      points[0].y = y;
      points[1].x = x+6;
      points[1].y = y;
      points[2].x = x;
      points[2].y = y+6;
      gdk_draw_polygon(window, metal_light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon(window, metal_light_gray_gc, TRUE,  points, 3);
      points[0].x = x;
      points[0].y = y+8;
      points[1].x = x;
      points[1].y = y+14;
      points[2].x = x+6;
      points[2].y = y+14;
      gdk_draw_polygon(window,   metal_light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon(window,   metal_light_gray_gc, TRUE,  points, 3);
/*      gdk_draw_rectangle(window, metal_light_gray_gc, TRUE,  x, y+15, width, height-15); */
   }
#endif

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
      gdk_gc_set_clip_rectangle(blackgc, NULL);
      gdk_gc_set_clip_rectangle(metal_light_gray_gc, NULL);
   }
}
/**************************************************************************/
static void 
metal_menu(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GdkGC *midgc, *whitegc;

   midgc   = style->bg_gc[GTK_STATE_SELECTED];
   whitegc = style->white_gc;

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(whitegc, area);
   }

   gdk_draw_rectangle(window, whitegc, FALSE, x+1, y+1, width-2, height-2);
   gdk_draw_rectangle(window, midgc,   FALSE, x, y, width-1, height-1);

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(whitegc, area);
   }
}
/**************************************************************************/
static void 
metal_menu_item(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_SELECTED], area);
      gdk_gc_set_clip_rectangle(style->dark_gc[GTK_STATE_SELECTED], area);
      gdk_gc_set_clip_rectangle(style->light_gc[GTK_STATE_SELECTED], area);
   }

   gdk_draw_rectangle(window, style->bg_gc[GTK_STATE_SELECTED], TRUE, x, y, width, height);
   gdk_draw_line(window, style->dark_gc[GTK_STATE_SELECTED], x, y, x + width, y);
   gdk_draw_line(window, style->light_gc[GTK_STATE_SELECTED], x, y + height -1, x + width, y + height - 1);

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_SELECTED], NULL);
      gdk_gc_set_clip_rectangle(style->dark_gc[GTK_STATE_SELECTED], NULL);
      gdk_gc_set_clip_rectangle(style->light_gc[GTK_STATE_SELECTED], NULL);
   }
}
/**************************************************************************/
static void 
metal_notebook(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GdkPixmap          *pm;
   gint               depth;
   GdkRectangle       texturearea;
   GdkGC *fillgc;
   GdkGCValues values;
   GdkGC *lightgc, *midgc, *darkgc, *whitegc;
   GdkPoint points[5];
   int w, h;

   /* Get colors */
   if (state_type == GTK_STATE_PRELIGHT) {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc   = style->bg_gc[GTK_STATE_SELECTED];
      darkgc  = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
   }
   else {
      lightgc = metal_light_gray_gc;
      midgc   = metal_mid_gray_gc;
      darkgc  = metal_dark_gray_gc;
      whitegc = style->white_gc;
   }

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
      gdk_gc_set_clip_rectangle(whitegc, area);
   }

   /* Draw backgound */
   gdk_draw_rectangle(window, lightgc, TRUE, x, y, width, height);

   /* Draw border */
   gdk_draw_rectangle(window, darkgc, FALSE, x, y, width-2, height-2);
   gdk_draw_rectangle(window, style->white_gc, FALSE, x+1, y+1, width-2, height-2);

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
   }
}
/**************************************************************************/
static void 
metal_tab(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GtkNotebook        *notebook;
   GdkPixmap          *pm;
   gint               depth;
   GdkRectangle       texturearea;
   GdkGC *fillgc;
   GdkGCValues values;
   GdkGC *lightgc, *midgc, *darkgc, *whitegc, *bggc;
   GdkPoint points[5];
   int w, h;
   int orientation, position, selected;

   notebook = GTK_NOTEBOOK(widget);
   orientation = notebook->tab_pos;
   get_tab_status(notebook, x, y, &position, &selected);

   /* Get colors */
   if (state_type == GTK_STATE_PRELIGHT) {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc   = style->bg_gc[GTK_STATE_SELECTED];
      darkgc  = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
      bggc    = style->bg_gc[GTK_STATE_NORMAL];
   }
   else {
      lightgc = metal_light_gray_gc;
      midgc   = metal_mid_gray_gc;
      darkgc  = metal_dark_gray_gc;
      whitegc = style->white_gc;
      bggc    = metal_light_gray_gc;
   }

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
      gdk_gc_set_clip_rectangle(whitegc, area);
      gdk_gc_set_clip_rectangle(bggc,    area);
   }

   /* Fill area */
   gdk_draw_rectangle(window, bggc, TRUE, x+0, y+0, width, height);

   switch (orientation) {
   case GTK_POS_TOP:
      /* Draw background */
      points[0].x = x+2;
      points[0].y = y+height;
      points[1].x = x+2;
      points[1].y = y+6;
      points[2].x = x+6;
      points[2].y = y+2;
      points[3].x = x+width-1;
      points[3].y = y+2;
      points[4].x = x+width-1;
      points[4].y = y+height;
      if (selected) gdk_draw_polygon(window, lightgc, TRUE, points, 5);
      else          gdk_draw_polygon(window, midgc,   TRUE, points, 5); 

      /* Draw border */
      if (position == 0) gdk_draw_line(window, darkgc, x+0, y+6, x+0, y+height+1);
      else if (selected) gdk_draw_line(window, darkgc, x+0, y+6, x+0, y+height-1);
      gdk_draw_line(window, darkgc, x+0, y+6, x+6, y+0);
      gdk_draw_line(window, darkgc, x+6, y+0, x+width-2, y+0);
      gdk_draw_line(window, darkgc, x+width-1, y+1, x+width-1, y+height-1);

      if (position == 0) gdk_draw_line(window, whitegc, x+1, y+6, x+1, y+height+1);
      else               gdk_draw_line(window, whitegc, x+1, y+6, x+1, y+height-1);
      gdk_draw_line(window, whitegc, x+1, y+6, x+6, y+1);
      gdk_draw_line(window, whitegc, x+6, y+1, x+width-2, y+1);
      break;
   case GTK_POS_LEFT:
      /* Draw background */
      points[0].x = x+2;
      points[0].y = y+height;
      points[1].x = x+2;
      points[1].y = y+6;
      points[2].x = x+6;
      points[2].y = y+2;
      points[3].x = x+width-1;
      points[3].y = y+2;
      points[4].x = x+width-1;
      points[4].y = y+height;
      if (selected) gdk_draw_polygon(window, lightgc, TRUE, points, 5);
      else          gdk_draw_polygon(window, midgc,   TRUE, points, 5); 

      /* Draw border */
      gdk_draw_line(window, darkgc, x+0, y+6, x+0, y+height-1);
      gdk_draw_line(window, darkgc, x+0, y+6, x+6, y+0);
      if (position == 0) gdk_draw_line(window, darkgc, x+6, y+0, x+width+1, y+0);
      else               gdk_draw_line(window, darkgc, x+6, y+0, x+width-1, y+0);
      gdk_draw_line(window, darkgc, x+0, y+height-1, x+width-1, y+height-1);

      gdk_draw_line(window, whitegc, x+1, y+6, x+6, y+1);
      if (position == 0) gdk_draw_line(window, whitegc, x+6, y+1, x+width+1, y+1);
      else               gdk_draw_line(window, whitegc, x+6, y+1, x+width-1, y+1);
      break;
   case GTK_POS_RIGHT:
      /* Draw background */
      points[0].x = x+width-2;
      points[0].y = y+height-1;
      points[1].x = x+width-2;
      points[1].y = y+6;
      points[2].x = x+width-6;
      points[2].y = y+2;
      points[3].x = x-1;
      points[3].y = y+2;
      points[4].x = x-1;
      points[4].y = y+height-1;
      if (selected) gdk_draw_polygon(window, lightgc, TRUE, points, 5);
      else          gdk_draw_polygon(window, midgc,   TRUE, points, 5); 

      /* Draw border */
      gdk_draw_line(window, darkgc, x+width-1, y+6, x+width-1, y+height-1);
      gdk_draw_line(window, darkgc, x+width-1, y+6, x+width-7, y+0);
      if (position == 0) gdk_draw_line(window, darkgc, x-2, y+0, x+width-7, y+0);
      else               gdk_draw_line(window, darkgc, x-1, y+0, x+width-7, y+0);
      gdk_draw_line(window, darkgc, x-1, y+height-1, x+width-1, y+height-1);

      gdk_draw_line(window, whitegc, x+width-2, y+6, x+width-7, y+1);
      if (position == 0) gdk_draw_line(window, whitegc, x+width-7, y+1, x-2, y+1);
      else               gdk_draw_line(window, whitegc, x+width-7, y+1, x-1, y+1);
      break;
   case GTK_POS_BOTTOM:
      /* Draw background */
      points[0].x = x+2;
      points[0].y = y+0;
      points[1].x = x+2;
      points[1].y = y+height-6;
      points[2].x = x+6;
      points[2].y = y+height-2;
      points[3].x = x+width-1;
      points[3].y = y+height-2;
      points[4].x = x+width-1;
      points[4].y = y+0;
      if (selected) gdk_draw_polygon(window, lightgc, TRUE, points, 5);
      else          gdk_draw_polygon(window, midgc,   TRUE, points, 5); 

      /* Draw border */
      if (position == 0) gdk_draw_line(window, darkgc, x+0, y+height-6, x+0, y-2);
      else if (selected) gdk_draw_line(window, darkgc, x+0, y+height-6, x+0, y-1);
      gdk_draw_line(window, darkgc, x+0, y+height-6, x+6, y+height);
      gdk_draw_line(window, darkgc, x+5, y+height-1, x+width-2, y+height-1);
      gdk_draw_line(window, darkgc, x+width-1, y+height-1, x+width-1, y-1);

      if (position == 0) gdk_draw_line(window, whitegc, x+1, y+height-6, x+1, y-2);
      else               gdk_draw_line(window, whitegc, x+1, y+height-6, x+1, y-1);
      gdk_draw_line(window, whitegc, x+1, y+height-6, x+5, y+height-2);
      break;
   }

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
      gdk_gc_set_clip_rectangle(bggc,    NULL);
   }
}
/**************************************************************************/
static void
get_tab_status(GtkNotebook *notebook, 
               int x, int y, int *position, int *selected)
{
   GtkNotebookPage *page;
   GtkWidget *label;
   GList *list;
   int pos = 0;
   int sel = FALSE;
   int border;
   
   border = GTK_CONTAINER(notebook)->border_width;

   /* Find tab in notebook based on (x,y) position
      Matches within 5 pixels, what a hack */
   list = notebook->children;
   while (list) {
      page = list->data;
      label = page->tab_label;
      if (fuzzy_match(x, label->allocation.x, 5) && 
          fuzzy_match(y, label->allocation.y, 5)) {
         if (page == notebook->cur_page) sel = TRUE;
         break;
      }
      list = list->next;
      pos++;
   }
   *position = pos;
   *selected = sel;
}
static int
fuzzy_match(int i, int j, int fudge)
{
   if (i>j) return(i-j <= fudge);
   else     return(j-i <= fudge);
}
/**************************************************************************/
static void 
metal_button(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GdkGC *bggc, *lightgc, *darkgc, *selgc;
   GtkToggleButton *togglebutton;

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_PRELIGHT], area);
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_SELECTED], area);
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_NORMAL], area);
      gdk_gc_set_clip_rectangle(style->bg_gc[state_type], area);
      gdk_gc_set_clip_rectangle(style->light_gc[state_type], area);
      gdk_gc_set_clip_rectangle(style->dark_gc[state_type], area);
   }

   /* Hack: Fix mangled co-ordinates if the widget has focus */
   if (GTK_WIDGET_HAS_FOCUS(widget)) {
      x -= 1;
      y -= 1;
      width  += 2;
      height += 2;
   }

   if (GTK_CHECK_TYPE(widget, gtk_toggle_button_get_type())) {
/*   if (DETAIL("togglebutton")) { */
      togglebutton = GTK_TOGGLE_BUTTON(widget);
      if (state_type == GTK_STATE_PRELIGHT) {
         gdk_draw_rectangle(window, style->bg_gc[GTK_STATE_PRELIGHT], TRUE,  x, y, width, height);
      }
      else if (togglebutton->active) {
         gdk_draw_rectangle(window, style->bg_gc[GTK_STATE_SELECTED], TRUE,  x, y, width, height);
      }
      else {
         gdk_draw_rectangle(window, style->bg_gc[GTK_STATE_NORMAL], TRUE,  x, y, width, height);
      }
   }
   else {
      gdk_draw_rectangle(window, style->bg_gc[state_type],    TRUE,  x, y, width, height);
   }

   gdk_draw_rectangle(window, style->dark_gc[state_type],  FALSE, x, y, width-2, height-2);
   gdk_draw_rectangle(window, style->light_gc[state_type], FALSE, x+1, y+1, width-2, height-2);

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_PRELIGHT], NULL);
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_SELECTED], NULL);
      gdk_gc_set_clip_rectangle(style->bg_gc[GTK_STATE_NORMAL], NULL);
      gdk_gc_set_clip_rectangle(style->bg_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle(style->light_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle(style->dark_gc[state_type], NULL);
   }
}
/**************************************************************************/
static void
draw_flat_box(GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
   GdkGC              *gc1, *gc2;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);


#if DEBUG
  printf("draw_flat_box: %p %s %i %i %i %i\n", detail, detail, x, y, width, height);
#endif

/*return; */

   if ((width == -1) && (height == -1)) gdk_window_get_size(window, &width, &height);
   else if (width == -1)                gdk_window_get_size(window, &width, NULL);
   else if (height == -1)               gdk_window_get_size(window, NULL, &height);


   if (DETAIL("selected")) {
      gc1 = style->bg_gc[GTK_STATE_SELECTED];
      gc2 = style->black_gc;
   }
   if (DETAIL("text") && (state_type == GTK_STATE_SELECTED)) {
      gc1 = style->bg_gc[GTK_STATE_SELECTED];
      gc2 = style->black_gc;
   }
   else if (DETAIL("viewportbin")) {
      gc1 = style->bg_gc[GTK_STATE_NORMAL];
      gc2 = style->black_gc;
   }
   else {
      gc1 = style->bg_gc[state_type];
      gc2 = style->black_gc;
   }

   if (area) {
      gdk_gc_set_clip_rectangle(gc1, area);
      gdk_gc_set_clip_rectangle(gc2, area);
   }

   if ((!style->bg_pixmap[state_type]) || (gc1 != style->bg_gc[state_type]) ||
       (gdk_window_get_type(window) == GDK_WINDOW_PIXMAP)) {
      gdk_draw_rectangle(window, gc1, TRUE, x, y, width, height);
      if (DETAIL("tooltip")) {
         gdk_draw_rectangle(window, gc2, FALSE, x, y, width - 1, height - 1);
      }
   }
   else {
      gtk_style_apply_default_pixmap(style, window, state_type, area, x, y, width, height);
      if (DETAIL("tooltip")) {
         gdk_draw_rectangle(window, gc2, FALSE, x, y, width - 1, height - 1);
      }
   }

   if (area) {
      gdk_gc_set_clip_rectangle(gc1, NULL);
      gdk_gc_set_clip_rectangle(gc2, NULL);
   }
}
/**************************************************************************/
static void
draw_check(GtkStyle * style,
	   GdkWindow * window,
	   GtkStateType state_type,
	   GtkShadowType shadow_type,
	   GdkRectangle * area,
	   GtkWidget * widget,
	   gchar * detail,
	   gint x,
	   gint y,
	   gint width,
	   gint height)
{
  GdkGC              *gc1, *gc2, *gc3, *gc4;
  gint                xx, yy;

  /* Fixed size only */

#if DEBUG
  printf("draw_check: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);
#endif

   gc1 = style->black_gc;
   gc2 = style->bg_gc[GTK_STATE_NORMAL];
   gc3 = style->dark_gc[state_type];
   gc4 = style->light_gc[state_type];

   if (area) {
      gdk_gc_set_clip_rectangle(gc1, area);
      gdk_gc_set_clip_rectangle(gc2, area);
      gdk_gc_set_clip_rectangle(gc3, area);
      gdk_gc_set_clip_rectangle(gc4, area);
   }

   /* Draw box */
   if (GTK_CHECK_TYPE(widget, gtk_menu_item_get_type())) {
      gdk_draw_rectangle(window, gc3, FALSE, x-2, y-2, 8, 8);
      gdk_draw_rectangle(window, gc4, FALSE, x-1, y-1, 8, 8);

      if (shadow_type == GTK_SHADOW_IN) {
         gdk_draw_line(window, gc1, x+1, y+0, x+1, y+4);
         gdk_draw_line(window, gc1, x+2, y+0, x+2, y+4);
         gdk_draw_line(window, gc1, x+3, y+3, x+7, y-1);
         gdk_draw_line(window, gc1, x+3, y+2, x+7, y-2);
      }
   }
   else {
      gdk_draw_rectangle(window, gc2, TRUE, x, y, width, height);

      gdk_draw_rectangle(window, gc3, FALSE, x-2, y-2, 11, 11);
      gdk_draw_rectangle(window, gc4, FALSE, x-1, y-1, 11, 11);

      if (shadow_type == GTK_SHADOW_IN) {
         gdk_draw_line(window, gc1, x+1, y+3, x+1, y+7);
         gdk_draw_line(window, gc1, x+2, y+3, x+2, y+7);
         gdk_draw_line(window, gc1, x+3, y+6, x+7, y+2);
         gdk_draw_line(window, gc1, x+3, y+5, x+7, y+1);
      }
   }

   if (area) {
      gdk_gc_set_clip_rectangle(gc1, NULL);
      gdk_gc_set_clip_rectangle(gc2, NULL);
      gdk_gc_set_clip_rectangle(gc3, NULL);
      gdk_gc_set_clip_rectangle(gc4, NULL);
   }
}
/**************************************************************************/
static void
draw_option(GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    gchar * detail,
	    gint x,
	    gint y,
	    gint width,
	    gint height)
{
   GdkGC              *gc0;
   GdkGC              *gc1;
   GdkGC              *gc2;
   GdkGC              *gc3;
   GdkGC              *gc4;

   x -= 1;
   y -= 1;
   width += 2;
   height += 2;

   gc0 = style->white_gc;
   gc1 = style->light_gc[GTK_STATE_NORMAL];
   gc2 = style->bg_gc[GTK_STATE_NORMAL];
   gc3 = style->dark_gc[GTK_STATE_NORMAL];
   gc4 = style->black_gc;

   if (area) {
      gdk_gc_set_clip_rectangle(gc0, area);
      gdk_gc_set_clip_rectangle(gc1, area);
      gdk_gc_set_clip_rectangle(gc2, area);
      gdk_gc_set_clip_rectangle(gc3, area);
      gdk_gc_set_clip_rectangle(gc4, area);
   }

   /* Draw radio button, metal-stle
      There is probably a better way to do this
      with pixmaps. Fix later. */

   if (GTK_CHECK_TYPE(widget, gtk_menu_item_get_type())) {
      /* dark */
      gdk_draw_line(window, gc3, x+2, y, x+6, y);
      gdk_draw_line(window, gc3, x+1, y+1, x+1, y+1);
      gdk_draw_line(window, gc3, x+7, y+1, x+7, y+1);
      gdk_draw_line(window, gc3, x+2, y+8, x+2, y+8);
      gdk_draw_line(window, gc3, x+7, y+7, x+7, y+7);
      gdk_draw_line(window, gc3, x+2, y+8, x+6, y+8);
      gdk_draw_line(window, gc3, x, y+2, x, y+6);
      gdk_draw_line(window, gc3, x+8, y+2, x+8, y+6);
 
      /* white */
      gdk_draw_line(window, gc0, x+3, y+1, x+6, y+1);
      gdk_draw_line(window, gc0, x+8, y+1, x+8, y+1);
      gdk_draw_line(window, gc0, x+2, y+2, x+2, y+2);
      gdk_draw_line(window, gc0, x+1, y+3, x+1, y+6);
      gdk_draw_line(window, gc0, x+9, y+2, x+9, y+7);
      gdk_draw_line(window, gc0, x+1, y+8, x+1, y+8);
      gdk_draw_line(window, gc0, x+8, y+8, x+8, y+8);
      gdk_draw_line(window, gc0, x+2, y+9, x+7, y+9);

      if (shadow_type == GTK_SHADOW_IN) {
         gdk_draw_rectangle(window, gc4, TRUE, x+2, y+3, 5, 3);
         gdk_draw_rectangle(window, gc4, TRUE, x+3, y+2, 3, 5);
      }
   }
   else {
      /* background */
      gdk_draw_rectangle(window, gc2, TRUE, x, y, width, height);

      /* dark */
      gdk_draw_line(window, gc3, x+4, y, x+7, y);
      gdk_draw_line(window, gc3, x+2, y+1, x+3, y+1);
      gdk_draw_line(window, gc3, x+8, y+1, x+9, y+1);
      gdk_draw_line(window, gc3, x+2, y+10, x+3, y+10);
      gdk_draw_line(window, gc3, x+8, y+10, x+9, y+10);
      gdk_draw_line(window, gc3, x+4, y+11, x+7, y+11);


      gdk_draw_line(window, gc3, x, y+4, x, y+7);
      gdk_draw_line(window, gc3, x+1, y+2, x+1, y+3);
      gdk_draw_line(window, gc3, x+1, y+8, x+1, y+9);
      gdk_draw_line(window, gc3, x+10, y+2, x+10, y+3);
      gdk_draw_line(window, gc3, x+10, y+8, x+10, y+9);
      gdk_draw_line(window, gc3, x+11, y+4, x+11, y+7);
 
      /* white */
      gdk_draw_line(window, gc0, x+4, y+1, x+7, y+1);
      gdk_draw_line(window, gc0, x+2, y+2, x+3, y+2);
      gdk_draw_line(window, gc0, x+8, y+2, x+9, y+2);
      gdk_draw_line(window, gc0, x+2, y+11, x+3, y+11);
      gdk_draw_line(window, gc0, x+8, y+11, x+9, y+11);
      gdk_draw_line(window, gc0, x+4, y+12, x+7, y+12);


      gdk_draw_line(window, gc0, x+1, y+4, x+1, y+7);
      gdk_draw_line(window, gc0, x+2, y+2, x+2, y+3);
      gdk_draw_line(window, gc0, x+2, y+8, x+2, y+9);
      gdk_draw_line(window, gc0, x+11, y+2, x+11, y+3);
      gdk_draw_line(window, gc0, x+11, y+8, x+11, y+9);
      gdk_draw_line(window, gc0, x+12, y+4, x+12, y+7);
      gdk_draw_point(window, gc0, x+10, y+1);
      gdk_draw_point(window, gc0, x+10, y+10);

      if (shadow_type == GTK_SHADOW_IN) {
         gdk_draw_rectangle(window, gc4, TRUE, x+3, y+4, 6, 4);
         gdk_draw_rectangle(window, gc4, TRUE, x+4, y+3, 4, 6);
      }
   }

   if (area) {
      gdk_gc_set_clip_rectangle(gc0, NULL);
      gdk_gc_set_clip_rectangle(gc1, NULL);
      gdk_gc_set_clip_rectangle(gc2, NULL);
      gdk_gc_set_clip_rectangle(gc3, NULL);
      gdk_gc_set_clip_rectangle(gc4, NULL);
   }
}
/**************************************************************************/
static void
draw_cross(GtkStyle * style,
	   GdkWindow * window,
	   GtkStateType state_type,
	   GtkShadowType shadow_type,
	   GdkRectangle * area,
	   GtkWidget * widget,
	   gchar * detail,
	   gint x,
	   gint y,
	   gint width,
	   gint height)
{
   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);
}

/**************************************************************************/
static void
draw_ramp(GtkStyle * style,
	  GdkWindow * window,
	  GtkStateType state_type,
	  GtkShadowType shadow_type,
	  GdkRectangle * area,
	  GtkWidget * widget,
	  gchar * detail,
	  GtkArrowType arrow_type,
	  gint x,
	  gint y,
	  gint width,
	  gint height)
{
   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);
}

/**************************************************************************/
static void
draw_tab(GtkStyle * style,
	 GdkWindow * window,
	 GtkStateType state_type,
	 GtkShadowType shadow_type,
	 GdkRectangle * area,
	 GtkWidget * widget,
	 gchar * detail,
	 gint x,
	 gint y,
	 gint width,
	 gint height)
{
   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

#if DEBUG
  printf("draw_tab: %p %s %i %i\n", detail, detail, width, height);
#endif

   gtk_paint_box(style, window, state_type, shadow_type, area, widget, detail,
                 x, y, width, height);
}

/**************************************************************************/
static void
draw_shadow_gap(GtkStyle * style,
		GdkWindow * window,
		GtkStateType state_type,
		GtkShadowType shadow_type,
		GdkRectangle * area,
		GtkWidget * widget,
		gchar * detail,
		gint x,
		gint y,
		gint width,
		gint height,
		GtkPositionType gap_side,
		gint gap_x,
		gint gap_width)
{
   GdkRectangle        rect;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

#if DEBUG
  printf("draw_shadow_gap: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);
#endif

   gtk_paint_shadow(style, window, state_type, shadow_type, area, widget, detail,
		   x, y, width, height);

   switch (gap_side)
     {
     case GTK_POS_TOP:
       rect.x = x + gap_x;
       rect.y = y;
       rect.width = gap_width;
       rect.height = 2;
       break;
     case GTK_POS_BOTTOM:
       rect.x = x + gap_x;
       rect.y = y + height - 2;
       rect.width = gap_width;
       rect.height = 2;
       break;
     case GTK_POS_LEFT:
       rect.x = x;
       rect.y = y + gap_x;
       rect.width = 2;
       rect.height = gap_width;
       break;
     case GTK_POS_RIGHT:
       rect.x = x + width - 2;
       rect.y = y + gap_x;
       rect.width = 2;
       rect.height = gap_width;
       break;
     }

   gtk_style_apply_default_pixmap(style, window, state_type, area,
                                  rect.x, rect.y, rect.width, rect.height);
}
/**************************************************************************/
static void
draw_box_gap(GtkStyle * style,
	     GdkWindow * window,
	     GtkStateType state_type,
	     GtkShadowType shadow_type,
	     GdkRectangle * area,
	     GtkWidget * widget,
	     gchar * detail,
	     gint x,
	     gint y,
	     gint width,
	     gint height,
	     GtkPositionType gap_side,
	     gint gap_x,
	     gint gap_width)
{
   GdkRectangle        rect;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

#if DEBUG
  printf("draw_box_gap: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);
#endif

   gtk_paint_box(style, window, state_type, shadow_type, area, widget, detail,
                 x, y, width, height);

   switch (gap_side)
     {
     case GTK_POS_TOP:
       rect.x = x + gap_x;
       rect.y = y;
       rect.width = gap_width;
       rect.height = 2;
       break;
     case GTK_POS_BOTTOM:
       rect.x = x + gap_x;
       rect.y = y + height - 2;
       rect.width = gap_width;
       rect.height = 2;
       break;
     case GTK_POS_LEFT:
       rect.x = x;
       rect.y = y + gap_x;
       rect.width = 2;
       rect.height = gap_width;
       break;
     case GTK_POS_RIGHT:
      rect.x = x + width - 2;
      rect.y = y + gap_x;
      rect.width = 2;
      rect.height = gap_width;
      break;
     }

   gtk_style_apply_default_pixmap(style, window, state_type, area,
                                  rect.x, rect.y, rect.width, rect.height);
}
/**************************************************************************/
static void
draw_extension(GtkStyle * style,
	       GdkWindow * window,
	       GtkStateType state_type,
	       GtkShadowType shadow_type,
	       GdkRectangle * area,
	       GtkWidget * widget,
	       gchar * detail,
	       gint x,
	       gint y,
	       gint width,
	       gint height,
	       GtkPositionType gap_side)
{
   GdkRectangle        rect;

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

#if DEBUG
  printf("draw_extension: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);
#endif

   gtk_paint_box(style, window, state_type, shadow_type, area, widget, detail,
                 x, y, width, height);

   switch (gap_side)
     {
     case GTK_POS_TOP:
       rect.x = x + style->klass->xthickness;
       rect.y = y;
       rect.width = width - style->klass->xthickness * 2;
       rect.height = style->klass->ythickness;
       break;
     case GTK_POS_BOTTOM:
       rect.x = x + style->klass->xthickness;
       rect.y = y + height - style->klass->ythickness;
       rect.width = width - style->klass->xthickness * 2;
       rect.height = style->klass->ythickness;
       break;
     case GTK_POS_LEFT:
       rect.x = x;
       rect.y = y + style->klass->ythickness;
       rect.width = style->klass->xthickness;
       rect.height = height - style->klass->ythickness * 2;
     case GTK_POS_RIGHT:
       rect.x = x + width - style->klass->xthickness;
       rect.y = y + style->klass->ythickness;
       rect.width = style->klass->xthickness;
       rect.height = height - style->klass->ythickness * 2;
     }
   
   gtk_style_apply_default_pixmap(style, window, state_type, area,
                                  rect.x, rect.y, rect.width, rect.height);
}
/**************************************************************************/
static void
draw_focus(GtkStyle * style,
	   GdkWindow * window,
	   GdkRectangle * area,
	   GtkWidget * widget,
	   gchar * detail,
	   gint x,
	   gint y,
	   gint width,
	   gint height)
{
   GdkGC *focusgc;

#if DEBUG
  printf("draw_focus: %p %p %s %i %i %i %i\n", widget, window, detail, x, y, width, height);
#endif

/*  return; TBD */

   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

   if ((width == -1) && (height == -1)) gdk_window_get_size(window, &width, &height);
   else if (width == -1)                gdk_window_get_size(window, &width, NULL);
   else if (height == -1)               gdk_window_get_size(window, NULL, &height);

   focusgc = style->bg_gc[GTK_STATE_SELECTED];

   if (area) {
      gdk_gc_set_clip_rectangle(focusgc, area);
   }

   if (DETAIL("button") || DETAIL("togglebutton") || 
       DETAIL("buttondefault") || DETAIL("tab")) {
      gdk_draw_rectangle(window, focusgc, FALSE, x+2, y+2, width-4, height-4);
   }
   else if (DETAIL("checkbutton") || DETAIL("radiobutton")) {
      gdk_draw_rectangle(window, focusgc, FALSE, x+15, y+2, width-17, height-4);
   }

   if (area) {
      gdk_gc_set_clip_rectangle(focusgc, NULL);
   }
}
/**************************************************************************/
static void
draw_slider(GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    gchar * detail,
	    gint x,
	    gint y,
	    gint width,
	    gint height,
	    GtkOrientation orientation)
{
   g_return_if_fail(style != NULL);
   g_return_if_fail(window != NULL);

   if ((width == -1) && (height == -1)) gdk_window_get_size(window, &width, &height);
   else if (width == -1)                gdk_window_get_size(window, &width, NULL);
   else if (height == -1)               gdk_window_get_size(window, NULL, &height);

   metal_scale_slider(style, window, state_type, shadow_type,
                      area, widget, detail, x, y, width, height, orientation);
}
/**************************************************************************/
static void
draw_handle(GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    gchar * detail,
	    gint x,
	    gint y,
	    gint width,
	    gint height,
	    GtkOrientation orientation)
{
   GdkPixmap          *pm;
   gint               depth;
   GdkRectangle       texturearea;
   GdkGC *fillgc;
   GdkGCValues values;
   GdkGC *lightgc, *midgc, *darkgc, *whitegc, *blackgc;

   /* Get colors */
   if (state_type == GTK_STATE_PRELIGHT) {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc   = style->bg_gc[GTK_STATE_SELECTED];
      darkgc  = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
      blackgc = style->black_gc;
   }
   else {
      lightgc = metal_light_gray_gc;
      midgc   = metal_mid_gray_gc;
      darkgc  = metal_dark_gray_gc;
      whitegc = style->white_gc;
      blackgc = style->black_gc;
   }

   /* Draw textured surface */
   gdk_window_get_geometry(window, NULL, NULL, NULL, NULL, &depth);

   pm = gdk_pixmap_new(window, 8, 3, depth);

   gdk_draw_rectangle(pm, lightgc, TRUE, 0, 0, 8, 3);
   gdk_draw_point(pm, whitegc,   3, 0);
   gdk_draw_point(pm, whitegc,   0, 1);
   gdk_draw_point(pm, blackgc,   4, 1);
   gdk_draw_point(pm, blackgc,   1, 2);

   values.fill = GDK_TILED;
   values.ts_x_origin = 2;/*5; */
   values.ts_y_origin = 2;/*3; */
   fillgc = gdk_gc_new_with_values(window, &values, 
                    GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);

   /* Set Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, area);
      gdk_gc_set_clip_rectangle(midgc,   area);
      gdk_gc_set_clip_rectangle(darkgc,  area);
      gdk_gc_set_clip_rectangle(whitegc, area);
      gdk_gc_set_clip_rectangle(blackgc, area);
   }

   /* Draw backgound */
   gdk_draw_rectangle(window, lightgc, TRUE, x, y, width, height);

   /* Draw border */
   gdk_draw_rectangle(window, whitegc, FALSE, x+1, y+1, width-2, height-2);
   gdk_draw_rectangle(window, darkgc,  FALSE, x+0, y+0, width-2, height-2);

   if (area) gdk_gc_set_clip_rectangle(fillgc, area);
   gdk_gc_set_tile(fillgc, pm);
   gdk_draw_rectangle(window, fillgc, TRUE, x+2, y+2, width-4, height-4);
  
   gdk_gc_unref(fillgc);
   gdk_pixmap_unref(pm);

   /* Reset Clip Region */
   if (area) {
      gdk_gc_set_clip_rectangle(lightgc, NULL);
      gdk_gc_set_clip_rectangle(midgc,   NULL);
      gdk_gc_set_clip_rectangle(darkgc,  NULL);
      gdk_gc_set_clip_rectangle(whitegc, NULL);
      gdk_gc_set_clip_rectangle(blackgc, NULL);
   }
}
