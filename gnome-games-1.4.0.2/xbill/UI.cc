#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "objects.h"
#include "x11.h"
#include "Strings.h"

#include <libgnomeui/gnome-window-icon.h>
GtkWidget *toplevel, *menubar, *field;
GtkWidget *scorebox, *endgamebox;
GtkWidget *warpbox, *quitbox, *newgamebox, *pausebox;
GtkWidget *aboutbox, *rulesbox, *storybox;

/**************************/
/* Timer control routines */
/**************************/

void UI::restart_timer() {
	timer = gtk_timeout_add(250, (GtkFunction)timer_eh, NULL); /* 250 ms */
}

void UI::kill_timer() {
	if (timer) {
		gtk_timeout_remove(timer);
		timer = 0;
	}
}

void UI::pause_game() {
	if (timer) playing = 1;
	kill_timer();
}

void UI::resume_game() {
	if (playing && !timer) restart_timer();
	playing = 0;
}

/*******************/
/* Window routines */
/*******************/

void UI::initialize(int argc, char **argv, const struct poptOption *options) {
        gnome_init_with_popt_table(PACKAGE, VERSION, argc, argv,
				   options, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/xbill.png");
	toplevel = gnome_app_new("gnome-xbill", "Gnome xBill");
	gtk_signal_connect(GTK_OBJECT(toplevel), "destroy",
			   GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
	gtk_widget_realize(toplevel);
	display = toplevel->window;
}

void UI::make_mainwin() {
        GtkStyle *style;

	CreateMenuBar(toplevel);

	field = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(field), game.scrwidth,
			      game.scrheight);

	gnome_app_set_contents(GNOME_APP(toplevel), field);
	gtk_widget_show(field);

	gtk_widget_ensure_style(field);
	style = field->style;
	colormap = style->colormap;
	white.red = style->white.red; white.green = style->white.green;
	white.blue = style->white.blue; white.pixel = style->white.pixel;
	black.red = style->black.red; black.green = style->black.green;
	black.blue = style->black.blue; black.pixel = style->black.pixel;

	gtk_widget_set_events(field, gtk_widget_get_events(field) |
			             GDK_BUTTON_PRESS_MASK |
			             GDK_BUTTON_RELEASE_MASK |
			             GDK_LEAVE_NOTIFY_MASK |
			             GDK_ENTER_NOTIFY_MASK |
			             GDK_EXPOSURE_MASK);

	gtk_signal_connect(GTK_OBJECT(field), "button_press_event",
			   GTK_SIGNAL_FUNC(button_press_eh), NULL);
	gtk_signal_connect(GTK_OBJECT(field), "button_release_event",
			   GTK_SIGNAL_FUNC(button_release_eh), NULL);
	gtk_signal_connect(GTK_OBJECT(field), "leave_notify_event",
			   GTK_SIGNAL_FUNC(leave_window_eh), NULL);
	gtk_signal_connect(GTK_OBJECT(field), "enter_notify_event",
			   GTK_SIGNAL_FUNC(enter_window_eh), NULL);
	gtk_signal_connect(GTK_OBJECT(field), "expose_event",
			   GTK_SIGNAL_FUNC(redraw_window_eh), NULL);
	
	gtk_widget_realize(field);
	window = field->window;

	gtk_window_set_policy(GTK_WINDOW(toplevel), FALSE, FALSE, FALSE);
        gtk_widget_show(toplevel);
}

void UI::make_windows() {
	Picture about;

	icon.load("icon");

	gdk_window_set_icon(toplevel->window, NULL, icon.pix, NULL);
	/* XXXXX */
	newgamebox = CreateDialog ("New Game", OK|CANCEL, NULL,
		newgamestr, (char *)NULL, GTK_SIGNAL_FUNC(new_game_cb));
	pausebox = CreateDialog ("Pause Game", OK, icon.pix,
		pausestr, "Continue", NULL);
	quitbox = CreateDialog ("Quit", OK|CANCEL, NULL,
		quitstr, (char *)NULL, GTK_SIGNAL_FUNC(quit_game_cb));
	warpbox = CreateEnterText ("Warp To Level", warpstr,
		GTK_SIGNAL_FUNC(warp_apply));
	about.load("about");

	aboutbox = CreatePixmapBox("About", about.pix,
		"Ported to GNOME by James Henstridge <james@daa.com.au>");
	rulesbox = CreatePixmapBox("Rules", NULL, rulesstr);
	storybox = CreatePixmapBox("Story", NULL, storystr);

	scorebox = CreateDialog ("Score", OK, NULL, "", (char *)NULL, NULL);
	endgamebox = CreateDialog ("Endgame", OK, NULL,
		endgamestr, "Nuts!", NULL);
}

void UI::popup_dialog (int dialog) {
	GtkWidget *w;
	switch (dialog) {
		case game.SCORE: w = scorebox; break;
		case game.ENDGAME: w = endgamebox; break;
	}
	popup (NULL, &w);
}

/*********************/
/* Graphics routines */
/*********************/

void UI::set_cursor(int cursor) {
	switch (cursor) {
	case game.BUCKETC:
	        gdk_window_set_cursor(window, bucket.cursor.cursor); break;
	case game.DOWNC:
		gdk_window_set_cursor(window, downcursor.cursor); break;
	case game.DEFAULTC:
		gdk_window_set_cursor(window, defaultcursor.cursor); break;
	default:
		gdk_window_set_cursor(window, OS.cursor[cursor].cursor);
	}
}

void UI::load_cursors() {
	defaultcursor.load("hand_up", defaultcursor.SEP_MASK);
	gdk_window_set_cursor(window, defaultcursor.cursor);
	downcursor.load("hand_down", downcursor.SEP_MASK);
}

void UI::graph_init() {
	GdkGCValues gcval;
	GdkGCValuesMask gcmask;

        gcmask = GDK_GC_EXPOSURES;
        gcval.graphics_exposures = FALSE;
        stdgc = gdk_gc_new_with_values(display, &gcval, gcmask);
	gdk_gc_set_line_attributes(stdgc, 3, GDK_LINE_SOLID, GDK_CAP_ROUND,
				   GDK_JOIN_MITER);
	gdk_gc_set_background(stdgc, &white);
	gdk_gc_set_foreground(stdgc, &black);

        whitegc = gdk_gc_new_with_values(display, &gcval, gcmask);
        gdk_gc_set_background(whitegc, &white);
        gdk_gc_set_foreground(whitegc, &white);

	gtk_widget_ensure_style(field);
	gtk_widget_realize(field);
	font = field->style->font;
	offscreen = gdk_pixmap_new(field->window, game.scrwidth, game.scrheight,
				   -1);
}

void UI::clear() {
        gdk_draw_rectangle(offscreen, whitegc, TRUE, 0, 0,
			   game.scrwidth, game.scrheight);
}

void UI::refresh() {
        gdk_draw_pixmap(window, stdgc, offscreen, 0,0, 0,0,
			game.scrwidth, game.scrheight);
}

void UI::draw (Picture pict, int x, int y) {
        gdk_gc_set_clip_origin(pict.gc, x, y);
	gdk_draw_pixmap(offscreen, pict.gc, pict.pix, 0,0, x,y,
			pict.width, pict.height);
}

void UI::draw_centered (Picture pict) {
	draw (pict, (game.scrwidth - pict.width)/2,
		(game.scrheight - pict.height)/2);
}

void UI::draw_line(int x1, int y1, int x2, int y2) {
        gdk_draw_line(offscreen, stdgc, x1,y1, x2,y2);
}

void UI::draw_str(char *str, int x, int y) {
        gdk_draw_string(offscreen, font, stdgc, x,y, str);
}


/******************/
/* Other routines */
/******************/

void UI::set_pausebutton (int action) {
        extern GtkWidget *pausebutton;
	if (pausebutton) gtk_widget_set_sensitive(pausebutton, action);
}


void UI::MainLoop() {
        gtk_main();
}
