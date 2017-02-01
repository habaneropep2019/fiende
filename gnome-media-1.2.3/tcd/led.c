#include <gtk/gtk.h>
#include "led.h"
#include "led.xpm"

static GdkPixmap *led_pixmap;

void
led_init(GtkWidget * w)
{
	GdkColor black;

	gdk_color_black(gtk_widget_get_colormap(w), &black);
	led_pixmap = gdk_pixmap_create_from_xpm_d(w->window, NULL,
						  &black, led);
}

void
led_done()
{
	gdk_pixmap_unref(led_pixmap);
}

void
led_create_widget(GtkWidget * window, GtkWidget ** time, GtkWidget ** track)
{
	*time = gtk_pixmap_new(gdk_pixmap_new(window->window,
					      LED_WIDTH, LED_HEIGHT + 2,
					      -1), NULL);
	*track = gtk_pixmap_new(gdk_pixmap_new(window->window,
					       DIGIT_WIDTH * 2 + 2,
					       LED_HEIGHT + 2,
					       -1), NULL);
}

void led_stop_time(GdkPixmap *p, GtkWidget *target, int x, int y)
{
	GtkStyle *style;

	style = gtk_widget_get_style(target);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, x+1, y+1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, x+(DIGIT_WIDTH + 1), y+1, DIGIT_WIDTH, -1);
	/* : */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH,
			0, x+(2 * DIGIT_WIDTH + 2), y+1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, x+(3 * DIGIT_WIDTH - 1), y+1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, x+(4 * DIGIT_WIDTH - 1), y+1, DIGIT_WIDTH, -1);
}

void led_draw_track(GdkPixmap *p, GtkWidget *target, int x, int y, int trackno)
{
	GtkStyle *style;
	style = gtk_widget_get_style(target);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (trackno / 10) * DIGIT_WIDTH,
			0, x+1, y+1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (trackno % 10) * DIGIT_WIDTH,
			0, x+(DIGIT_WIDTH + 1), y+1, DIGIT_WIDTH, -1);
}

void led_stop_track(GdkPixmap *p, GtkWidget *target, int x, int y)
{
	GtkStyle *style;
	style = gtk_widget_get_style(target);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, x+1, y+1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, x+(DIGIT_WIDTH + 1), y+1, DIGIT_WIDTH, -1);
}

void led_draw_time(GdkPixmap *p, GtkWidget *window,
	int x, int y, int min, int sec )
{
	GtkStyle *style;

	style = gtk_widget_get_style(window);
	/* minute */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (min / 10) * DIGIT_WIDTH,
			0, x+1, y+1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (min % 10) * DIGIT_WIDTH,
			0, x+(DIGIT_WIDTH + 1),y+1, DIGIT_WIDTH, -1);
	/* : */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH,
			0, x+(2 * DIGIT_WIDTH + 2), y+1, DIGIT_WIDTH, -1);
	/* second */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (sec / 10) * DIGIT_WIDTH,
			0, x+(3 * DIGIT_WIDTH - 1), y+1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (sec % 10) * DIGIT_WIDTH,
			0, x+(4 * DIGIT_WIDTH - 1), y+1, DIGIT_WIDTH, -1);
}
