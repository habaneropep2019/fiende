#include "raleigh.h"

#include <math.h>
#include <gtk/gtk.h>

static GtkStyleClass *parent_class;

typedef enum {
  CHECK_AA,
  CHECK_BASE,
  CHECK_BLACK,
  CHECK_DARK,
  CHECK_LIGHT,
  CHECK_MID,
  CHECK_TEXT,
  RADIO_BASE,
  RADIO_BLACK,
  RADIO_DARK,
  RADIO_LIGHT,
  RADIO_MID,
  RADIO_TEXT
} Part;

#define PART_SIZE 13

static char check_aa_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x58,0x00,0xa0,
 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char check_base_bits[] = {
 0x00,0x00,0x00,0x00,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,
 0x07,0xfc,0x07,0xfc,0x07,0xfc,0x07,0x00,0x00,0x00,0x00};
static char check_black_bits[] = {
 0x00,0x00,0xfe,0x0f,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,
 0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x00,0x00};
static char check_dark_bits[] = {
 0xff,0x1f,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00};
static char check_light_bits[] = {
 0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,
 0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0xfe,0x1f};
static char check_mid_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,
 0x08,0x00,0x08,0x00,0x08,0x00,0x08,0xfc,0x0f,0x00,0x00};
static char check_text_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x03,0x80,0x01,0x80,0x00,0xd8,
 0x00,0x60,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char radio_base_bits[] = {
 0x00,0x00,0x00,0x00,0xf0,0x01,0xf8,0x03,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,
 0x07,0xfc,0x07,0xf8,0x03,0xf0,0x01,0x00,0x00,0x00,0x00};
static char radio_black_bits[] = {
 0x00,0x00,0xf0,0x01,0x08,0x02,0x04,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,
 0x00,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char radio_dark_bits[] = {
 0xf0,0x01,0x08,0x02,0x04,0x04,0x02,0x04,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x01,0x00,0x02,0x00,0x0c,0x00,0x00,0x00,0x00,0x00};
static char radio_light_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x00,0x10,0x00,0x10,0x00,
 0x10,0x00,0x10,0x00,0x08,0x00,0x04,0x08,0x02,0xf0,0x01};
static char radio_mid_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00,0x08,0x00,
 0x08,0x00,0x08,0x00,0x04,0x00,0x02,0xf0,0x01,0x00,0x00};
static char radio_text_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x00,0xf0,0x01,0xf0,0x01,0xf0,
 0x01,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static struct {
  char      *bits;
  GdkBitmap *bmap;
} parts[] = {
  { check_aa_bits, NULL },
  { check_base_bits, NULL },
  { check_black_bits, NULL },
  { check_dark_bits, NULL },
  { check_light_bits, NULL },
  { check_mid_bits, NULL },
  { check_text_bits, NULL },
  { radio_base_bits, NULL },
  { radio_black_bits, NULL },
  { radio_dark_bits, NULL },
  { radio_light_bits, NULL },
  { radio_mid_bits, NULL },
  { radio_text_bits, NULL }
};

static void
draw_part (GdkDrawable  *drawable,
	   GdkGC        *gc,
	   GdkRectangle *area,
	   gint          x,
	   gint          y,
	   Part          part)
{
  if (area)
    gdk_gc_set_clip_rectangle (gc, area);
  
  if (!parts[part].bmap)
      parts[part].bmap = gdk_bitmap_create_from_data (drawable,
						      parts[part].bits,
						      PART_SIZE, PART_SIZE);

  gdk_gc_set_ts_origin (gc, x, y);
  gdk_gc_set_stipple (gc, parts[part].bmap);
  gdk_gc_set_fill (gc, GDK_STIPPLED);

  gdk_draw_rectangle (drawable, gc, TRUE, x, y, PART_SIZE, PART_SIZE);

  gdk_gc_set_fill (gc, GDK_SOLID);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
draw_check(GtkStyle      *style,
	   GdkWindow     *window,
	   GtkStateType   state,
	   GtkShadowType  shadow,
	   GdkRectangle  *area,
	   GtkWidget     *widget,
	   gchar         *detail,
	   gint           x,
	   gint           y,
	   gint           width,
	   gint           height)
{
  RaleighEngineData *engine_data = style->engine_data;

  x -= (1 + PART_SIZE - width) / 2;
  y -= (1 + PART_SIZE - height) / 2;
      
  if (strcmp (detail, "check") == 0)	/* Menu item */
    {
      if (shadow == GTK_SHADOW_IN)
	{
	  draw_part (window, style->black_gc, area, x, y, CHECK_TEXT);
	  draw_part (window, style->dark_gc[state], area, x, y, CHECK_AA);
	}
    }
  else
    {
      draw_part (window, style->black_gc, area, x, y, CHECK_BLACK);
      draw_part (window, style->dark_gc[state], area, x, y, CHECK_DARK);
      draw_part (window, style->mid_gc[state], area, x, y, CHECK_MID);
      draw_part (window, style->light_gc[state], area, x, y, CHECK_LIGHT);
      draw_part (window, style->base_gc[state], area, x, y, CHECK_BASE);
      
      if (shadow == GTK_SHADOW_IN)
	{
	  draw_part (window, style->text_gc[state], area, x, y, CHECK_TEXT);
	  draw_part (window, engine_data->aa_gc[state], area, x, y, CHECK_AA);
	}
    }
}

static void
draw_option(GtkStyle      *style,
	    GdkWindow     *window,
	    GtkStateType   state,
	    GtkShadowType  shadow,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    gchar         *detail,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  x -= (1 + PART_SIZE - width) / 2;
  y -= (1 + PART_SIZE - height) / 2;
      
  if (strcmp (detail, "option") == 0)	/* Menu item */
    {
      if (shadow == GTK_SHADOW_IN)
	draw_part (window, style->black_gc, area, x, y, RADIO_TEXT);
    }
  else
    {
      draw_part (window, style->black_gc, area, x, y, RADIO_BLACK);
      draw_part (window, style->dark_gc[state], area, x, y, RADIO_DARK);
      draw_part (window, style->mid_gc[state], area, x, y, RADIO_MID);
      draw_part (window, style->light_gc[state], area, x, y, RADIO_LIGHT);
      draw_part (window, style->base_gc[state], area, x, y, RADIO_BASE);
      
      if (shadow == GTK_SHADOW_IN)
	draw_part (window, style->text_gc[state], area, x, y, RADIO_TEXT);
    }
}

static void
draw_varrow (GdkWindow     *window,
	     GdkGC         *gc,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkArrowType   arrow_type,
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height)
{
  gint steps, extra;
  gint y_start, y_increment;
  gint i;

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);
  
  width = width + width % 2 - 1;	/* Force odd */
  
  steps = 1 + width / 2;

  extra = height - steps;

  if (arrow_type == GTK_ARROW_DOWN)
    {
      y_start = y;
      y_increment = 1;
    }
  else
    {
      y_start = y + height - 1;
      y_increment = -1;
    }

  for (i = 0; i < extra; i++)
    {
      gdk_draw_line (window, gc,
		     x,              y_start + i * y_increment,
		     x + width - 1,  y_start + i * y_increment);
    }
  for (; i < height; i++)
    {
      gdk_draw_line (window, gc,
		     x + (i - extra),              y_start + i * y_increment,
		     x + width - (i - extra) - 1,  y_start + i * y_increment);
    }
  

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
draw_harrow (GdkWindow     *window,
	     GdkGC         *gc,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkArrowType   arrow_type,
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height)
{
  gint steps, extra;
  gint x_start, x_increment;
  gint i;

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);
  
  height = height + height % 2 - 1;	/* Force odd */
  
  steps = 1 + height / 2;

  extra = width - steps;

  if (arrow_type == GTK_ARROW_RIGHT)
    {
      x_start = x;
      x_increment = 1;
    }
  else
    {
      x_start = x + width - 1;
      x_increment = -1;
    }

  for (i = 0; i < extra; i++)
    {
      gdk_draw_line (window, gc,
		     x_start + i * x_increment, y,
		     x_start + i * x_increment, y + height - 1);
    }
  for (; i < width; i++)
    {
      gdk_draw_line (window, gc,
		     x_start + i * x_increment, y + (i - extra),
		     x_start + i * x_increment, y + height - (i - extra) - 1);
    }
  

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
draw_arrow (GtkStyle      *style,
	    GdkWindow     *window,
	    GtkStateType   state,
	    GtkShadowType  shadow,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    gchar         *detail,
	    GtkArrowType   arrow_type,
	    gboolean       fill,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  if ((width == -1) && (height == -1))
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);
  
  if (detail && strcmp (detail, "spinbutton") == 0)
    {
      x += (width - 7) / 2;

      if (arrow_type == GTK_ARROW_UP)
	y += (height - 4) / 2;
      else
	y += (1 + height - 4) / 2;

      draw_varrow (window, style->black_gc, shadow, area, arrow_type,
		   x, y, 7, 4);
    }
  else if (detail && strcmp (detail, "vscrollbar") == 0)
    {
      parent_class->draw_box (style, window, state, shadow, area,
			      widget, detail, x, y, width, height);
      
      x += (width - 7) / 2;
      y += (height - 5) / 2;

      draw_varrow (window, style->black_gc, shadow, area, arrow_type,
		   x, y, 7, 5);
      
    }
  else if (detail && strcmp (detail, "hscrollbar") == 0)
    {
      parent_class->draw_box (style, window, state, shadow, area,
			      widget, detail, x, y, width, height);
      
      y += (height - 7) / 2;
      x += (width - 5) / 2;

      draw_harrow (window, style->black_gc, shadow, area, arrow_type,
		   x, y, 5, 7);
    }
  else
    {
      if (arrow_type == GTK_ARROW_UP || arrow_type == GTK_ARROW_DOWN)
	{
	  x += (width - 7) / 2;
	  y += (height - 5) / 2;
	  
	  draw_varrow (window, style->black_gc, shadow, area, arrow_type,
		       x, y, 7, 5);
	}
      else
	{
	  x += (width - 5) / 2;
	  y += (height - 7) / 2;
	  
	  draw_harrow (window, style->black_gc, shadow, area, arrow_type,
		       x, y, 5, 7);
	}
    }
}

void
draw_thin_shadow (GtkStyle      *style,
		  GdkWindow     *window,
		  GtkStateType   state,
		  GdkRectangle  *area,
		  gint           x,
		  gint           y,
		  gint           width,
		  gint           height)
{
  GdkGC *gc1, *gc2;
  
  if ((width == -1) && (height == -1))
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);
  
  gc1 = style->light_gc[state];
  gc2 = style->dark_gc[state];
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
    }
  
  gdk_draw_line (window, gc1,
		 x, y + height - 1, x + width - 1, y + height - 1);
  gdk_draw_line (window, gc1,
		 x + width - 1, y,  x + width - 1, y + height - 1);
      
  gdk_draw_line (window, gc2,
		 x, y, x + width - 1, y);
  gdk_draw_line (window, gc2,
		 x, y, x, y + height - 1);

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
    }
}

void
draw_spin_entry_shadow (GtkStyle      *style,
			GdkWindow     *window,
			GtkStateType   state,
			GdkRectangle  *area,
			gint           x,
			gint           y,
			gint           width,
			gint           height)
{
  gint window_width, window_height;
  gboolean focus_inset;

  gdk_window_get_size (window, &window_width, &window_height);

  if (width == -1)
    width = window_width;
  if (height == 1)
    height = window_height;

  focus_inset = (width < window_width && height < window_height);
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[state], area);
      gdk_gc_set_clip_rectangle (style->black_gc, area);
      gdk_gc_set_clip_rectangle (style->bg_gc[state], area);
      gdk_gc_set_clip_rectangle (style->base_gc[state], area);
    }

  gdk_draw_line (window, style->light_gc[state],
		 x, y + height - 1, x + width - 1, y + height - 1);

  gdk_draw_line (window, 
		 style->base_gc[state],
		 x + width - 1,  y + 1, x + width - 1,  y + height - 3);
      
  if (!focus_inset)
    {
      gdk_draw_line (window, style->bg_gc[state],
		     x + 1, y + height - 2, x + width - 1, y + height - 2);
      gdk_draw_line (window, 
		     style->base_gc[state],
		     x + width - 2, y + 1, x + width - 2, y + height - 3);
  
      gdk_draw_line (window, style->black_gc,
		     x + 1, y + 1, x + width - 1, y + 1);
      gdk_draw_line (window, style->black_gc,
		     x + 1, y + 1, x + 1, y + height - 2);
    }
      
  gdk_draw_line (window, style->dark_gc[state],
		 x, y, x + width - 1, y);
  gdk_draw_line (window, style->dark_gc[state],
		 x, y, x, y + height - 1);

  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[state], NULL);
      gdk_gc_set_clip_rectangle (style->black_gc, NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[state], NULL);
      gdk_gc_set_clip_rectangle (style->base_gc[state], NULL);
    }
}

static void
draw_spinbutton_shadow (GtkStyle      *style,
			GdkWindow     *window,
			GtkStateType   state,
			GdkRectangle  *area,
			gint           x,
			gint           y,
			gint           width,
			gint           height)
{
  gint y_middle = y + height / 2;
  
  if ((width == -1) && (height == -1))
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->black_gc, area);
      gdk_gc_set_clip_rectangle (style->bg_gc[state], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[state], area);
      gdk_gc_set_clip_rectangle (style->light_gc[state], area);
    }
  
  gdk_draw_line (window, style->black_gc,
		 x, y + 2, x, y + height - 3);
  gdk_draw_line (window, style->black_gc,
		 x, y + 1, x + width - 2, y + 1);
  gdk_draw_line (window, style->black_gc,
		 x + width - 2, y + 2, x + width - 2, y + height - 3);
  
  gdk_draw_line (window, style->bg_gc[state],
		 x, y + height - 2, x + width - 2, y + height - 2);

  gdk_draw_line (window, style->dark_gc[state],
		 x, y, x + width - 1, y);
  gdk_draw_line (window, style->dark_gc[state],
		 x + 1, y_middle - 1, x + width - 3, y_middle - 1);
  gdk_draw_line (window, style->dark_gc[state],
		 x + 1, y + height - 3, x + width - 3, y + height - 3);

  gdk_draw_line (window, style->light_gc[state],
		 x + 1, y + 2, x + width - 3, y + 2);
  gdk_draw_line (window, style->light_gc[state],
		 x + 1, y_middle, x + width - 3, y_middle);
  gdk_draw_line (window, style->light_gc[state],
		 x + width - 1, y + 1, x + width - 1, y + height - 1);
  gdk_draw_line (window, style->light_gc[state],
		 x, y + height - 1, x + width - 2, y + height - 1);
      
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->black_gc, NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[state], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[state], NULL);
      gdk_gc_set_clip_rectangle (style->light_gc[state], NULL);
    }
}

static void
draw_shadow (GtkStyle      *style,
		  GdkWindow     *window,
		  GtkStateType   state,
		  GtkShadowType  shadow,
		  GdkRectangle  *area,
		  GtkWidget     *widget,
		  gchar         *detail,
		  gint           x,
		  gint           y,
		  gint           width,
		  gint           height)
{
  if (shadow == GTK_SHADOW_IN)
    {
      if (detail &&
	  (strcmp (detail, "buttondefault") == 0 ||
	   strcmp (detail, "trough") == 0))
	{
	  draw_thin_shadow (style, window, state, area,
			    x, y, width, height);
	  return;
	}
      else if (widget && GTK_IS_SPIN_BUTTON (widget) &&
	       detail && strcmp (detail, "entry") == 0)
	{
	  draw_spin_entry_shadow (style, window, state, area,
				  x, y, width, height);
	  return;
	}
      else if (widget && GTK_IS_SPIN_BUTTON (widget) &&
	       detail && strcmp (detail, "spinbutton") == 0)
	{
	  draw_spinbutton_shadow (style, window, state,
				  area, x, y, width, height);
	  return;
	}
    }
  
  parent_class->draw_shadow (style, window, state,
			     shadow, area, widget, detail,
			     x, y, width, height);
}

static void 
draw_focus (GtkStyle      *style,
	    GdkWindow     *window,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    gchar         *detail,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  if (!(detail && widget &&
	GTK_IS_SPIN_BUTTON (widget) && strcmp (detail, "entry") == 0))
    parent_class->draw_focus (style, window, area, widget, detail,
			      x, y, width, height);
  else
    {
      if (width == -1 && height == -1)
	{
	  gdk_window_get_size (window, &width, &height);
	  width -= 1;
	  height -= 1;
	}
      else if (width == -1)
	{
	  gdk_window_get_size (window, &width, NULL);
	  width -= 1;
	}
      else if (height == -1)
	{
	  gdk_window_get_size (window, NULL, &height);
	  height -= 1;
	}
      
      if (area)
	gdk_gc_set_clip_rectangle (style->black_gc, area);

      gdk_draw_line (window, style->black_gc,
		     x, y, x + width, y);
      gdk_draw_line (window, style->black_gc,
		     x, y + 1, x, y + height);
      gdk_draw_line (window, style->black_gc,
		     x, y + height, x + width, y + height);
      gdk_draw_point (window, style->black_gc, x + width, y + 1);
      gdk_draw_point (window, style->black_gc, x + width, y + height - 1);
      
      if (area)
	gdk_gc_set_clip_rectangle (style->black_gc, NULL);
    }
}

static void 
draw_box (GtkStyle      *style,
	  GdkWindow     *window,
	  GtkStateType   state_type,
	  GtkShadowType  shadow_type,
	  GdkRectangle  *area,
	  GtkWidget     *widget,
	  gchar         *detail,
	  gint           x,
	  gint           y,
	  gint           width,
	  gint           height)
{
  parent_class->draw_box (style, window, state_type, shadow_type, area,
			  widget, detail, x, y, width, height);

  if (detail && strcmp (detail, "optionmenu") == 0)
    {
      if ((width == -1) && (height == -1))
	gdk_window_get_size (window, &width, &height);
      else if (width == -1)
	gdk_window_get_size (window, &width, NULL);
      else if (height == -1)
	gdk_window_get_size (window, NULL, &height);
  
      parent_class->draw_vline (style, window, state_type, area, widget,
				detail,
				y + style->klass->ythickness + 1,
				y + height - style->klass->ythickness - 3,
				x + width - (OPTION_INDICATOR_WIDTH + OPTION_INDICATOR_LEFT_SPACING + OPTION_INDICATOR_RIGHT_SPACING) - style->klass->xthickness);
    }
}

static void
draw_tab (GtkStyle      *style,
	  GdkWindow     *window,
	  GtkStateType   state,
	  GtkShadowType  shadow,
	  GdkRectangle  *area,
	  GtkWidget     *widget,
	  gchar         *detail,
	  gint           x,
	  gint           y,
	  gint           width,
	  gint           height)
{
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  x += (width - OPTION_INDICATOR_WIDTH) / 2;
  y += (height - 13) / 2 - 1;

  draw_varrow (window, style->black_gc, shadow, area, GTK_ARROW_UP,
	       x, y, OPTION_INDICATOR_WIDTH, 5);
  draw_varrow (window, style->black_gc, shadow, area, GTK_ARROW_DOWN,
	       x, y + 8, OPTION_INDICATOR_WIDTH, 5);
}

void 
raleigh_initialize_style (GtkStyleClass *klass,
			  GtkStyleClass *parent)
{
  parent_class = parent;	/* Save for later use */

  *klass = *parent;

  klass->draw_arrow = draw_arrow;
  klass->draw_box = draw_box;
  klass->draw_check = draw_check;
  klass->draw_focus = draw_focus;
  klass->draw_option = draw_option;
  klass->draw_shadow = draw_shadow;
  klass->draw_tab = draw_tab;
}
