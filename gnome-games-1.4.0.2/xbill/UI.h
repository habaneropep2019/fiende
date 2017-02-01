#ifndef X11_UI_H
#define X11_UI_H

#include <gnome.h>
#include "Picture.h"
#include "MCursor.h"

class UI {
	guint timer;
	int playing;
	Picture icon;
	void get_coords (gint *x, gint *y);
	MCursor defaultcursor, downcursor;
	GdkFont *font;
	GdkGC *stdgc, *whitegc;
public:
	UI() {playing=0; timer=0;}
	GdkWindow *display;
	GdkWindow *window, *rootwindow;
	GdkColormap *colormap;
	int depth;
	GdkColor white, black;
	GdkPixmap *offscreen;

	void restart_timer();
	void kill_timer();

	void pause_game();
	void resume_game();

	void initialize(int argc, char **argv,
			const struct poptOption *options);
	void make_mainwin();
	void make_windows();
	void popup_dialog (int dialog);

	void set_cursor (int cursor);
	void load_cursors();
	void graph_init();
	void clear();
	void refresh();
	void draw (Picture picture, int x, int y);
	void draw_centered (Picture picture);
	void draw_line (int x1, int y1, int x2, int y2);
	void draw_str (char *str, int x, int y);

	void set_pausebutton (int action);
	void MainLoop();

	void update_scorebox(int level, int score);
	void update_hsbox(char *str);
};

#endif
