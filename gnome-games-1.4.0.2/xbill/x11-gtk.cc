#include "objects.h"
#include "x11.h"

#include <gtk/gtk.h>

extern GtkWidget *toplevel, *menubar, *field;
extern GtkWidget *aboutbox, *rulesbox, *storybox;
extern GtkWidget *warpbox, *quitbox, *newgamebox, *pausebox;
extern GtkWidget *scorebox, *endgamebox;

static void display_scores();
#ifndef GNOMEUIINFO_ITEM_STOCK_DATA
#define GNOMEUIINFO_ITEM_STOCK_DATA(label, tip, cb, data, xpm) \
                   {GNOME_APP_UI_ITEM, label, tip, cb, data, NULL, \
                    GNOME_APP_PIXMAP_STOCK, xpm, 0, (GdkModifierType)0, NULL}
#endif
#define GNOMEUIINFO_ITEM_NONE_DATA(label, tip, cb, data) \
                   {GNOME_APP_UI_ITEM, label, tip, cb, data, NULL, \
	            GNOME_APP_PIXMAP_NONE, NULL, 0, (GdkModifierType)0, NULL}
static GnomeUIInfo game_menu[] = {
  GNOMEUIINFO_MENU_NEW_GAME_ITEM((void *) popup, &newgamebox),
  GNOMEUIINFO_MENU_PAUSE_GAME_ITEM((void *) popup, &pausebox),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_ITEM_STOCK_DATA("Warp to level...", NULL, (void *) popup, &warpbox,
			      GNOME_STOCK_MENU_FORWARD),
  GNOMEUIINFO_MENU_SCORES_ITEM((void *) display_scores, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_EXIT_ITEM((void *) popup, &quitbox),
  GNOMEUIINFO_END
};
  
static GnomeUIInfo help_menu[] = {
  GNOMEUIINFO_ITEM_NONE_DATA("Story of xBill", NULL, (void *) popup, &storybox),
  GNOMEUIINFO_ITEM_NONE_DATA("Rules", NULL, (void *) popup, &rulesbox),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_ABOUT_ITEM((void *) popup, &aboutbox),
  GNOMEUIINFO_END
};
static GnomeUIInfo menus[] = {
  GNOMEUIINFO_MENU_GAME_TREE(game_menu),
  GNOMEUIINFO_MENU_HELP_TREE(help_menu),
  GNOMEUIINFO_END
};
GtkWidget *pausebutton = NULL;

void CreateMenuBar(GtkWidget *app) {
	gnome_app_create_menus(GNOME_APP(app), menus);
	pausebutton = game_menu[1].widget;
}

static gint  popdown(GtkWidget *w) {
  gtk_main_quit();
  return TRUE; /* don't kill window */
}

GtkWidget *CreatePixmapBox(char *title, GdkPixmap *pixmap, const char *text) {
        GtkWidget *win, *hbox, *vbox, *wid;

	win = gnome_dialog_new(title, GNOME_STOCK_BUTTON_OK, NULL);
	gtk_signal_connect(GTK_OBJECT(win), "close",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	gtk_signal_connect(GTK_OBJECT(win), "clicked",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	vbox = GNOME_DIALOG(win)->vbox;

	wid = gtk_pixmap_new(game.logo.pix, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
	gtk_widget_show(wid);

	if (pixmap) {
	        wid = gtk_pixmap_new(pixmap, NULL);
		gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
		gtk_widget_show(wid);
	}
	if (text) {
	        wid = gtk_label_new(text);
		gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
		gtk_widget_show(wid);
	}

	return win;
}

void warp_apply(GtkWidget *b, GtkEntry *entry) {
        game.warp_to_level(atoi(gtk_entry_get_text(entry)));
}

GtkWidget *CreateEnterText (char *title, const char *text,
			    GtkSignalFunc callback) {
        GtkWidget *win, *vbox, *entry, *wid;

	win = gnome_dialog_new(title, GNOME_STOCK_BUTTON_OK,
			       GNOME_STOCK_BUTTON_CANCEL, NULL);
	gnome_dialog_set_default(GNOME_DIALOG(win), 0);
	gtk_signal_connect(GTK_OBJECT(win), "close",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	gtk_signal_connect(GTK_OBJECT(win), "clicked",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	vbox = GNOME_DIALOG(win)->vbox;

	wid = gtk_label_new(text);
	gtk_box_pack_start(GTK_BOX(vbox), wid, FALSE, TRUE, 0);
	gtk_widget_show(wid);

	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);
	gtk_widget_show(entry);

	gnome_dialog_button_connect(GNOME_DIALOG(win), 0, callback, entry);
	return win;
}

GtkWidget *CreateDialog (char *title, int buttonmask, GdkPixmap *icon,
			 const char *text, const char *buttonlabel,
			 GtkSignalFunc callback) {
	GtkWidget *win, *hbox, *wid, *button;
	char *ttext= (char*)malloc(strlen(text)+5);

	win = gnome_dialog_new(title, NULL);
	gtk_signal_connect(GTK_OBJECT(win), "close",
			   GTK_SIGNAL_FUNC(popdown), NULL);
	gtk_signal_connect(GTK_OBJECT(win), "clicked",
			   GTK_SIGNAL_FUNC(popdown), NULL);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(win)->vbox), hbox,
			   TRUE, TRUE, 0);
	gtk_widget_show(hbox);

	if (icon) {
	        wid = gtk_pixmap_new(icon, NULL);
		gtk_box_pack_start(GTK_BOX(hbox), wid, FALSE, TRUE, 0);
		gtk_widget_show(wid);
	}

	strcpy(ttext, text);
	if (strlen(ttext)<12) strcat(ttext, "     ");
	wid = gtk_label_new(ttext);
	free (ttext);
	gtk_object_set_user_data(GTK_OBJECT(win), wid);
	gtk_box_pack_start(GTK_BOX(hbox), wid, FALSE, TRUE, 0);
	gtk_widget_show(wid);

	if (buttonmask&OK) {
	        gnome_dialog_append_buttons(GNOME_DIALOG(win),
					    GNOME_STOCK_BUTTON_OK, NULL);
		if (callback)
		        gnome_dialog_button_connect(GNOME_DIALOG(win), 0,
					   callback, NULL);
	}
	if (buttonmask&CANCEL) {
	        gnome_dialog_append_buttons(GNOME_DIALOG(win),
					    GNOME_STOCK_BUTTON_CANCEL, NULL);
	}
	gnome_dialog_set_default(GNOME_DIALOG(win), 0);
	return win;
}

void UI::update_hsbox(char *str) {
        /* this is no longer needed (because of gnome_scores) */
}

void UI::update_scorebox(int level, int score) {
        GtkLabel *label;
	char str[40];

	label = GTK_LABEL(gtk_object_get_user_data(GTK_OBJECT(scorebox)));
	sprintf (str, "After Level %d:     \nYour score: %d", level, score);
	gtk_label_set(label, str);
}

void show_scores(int pos) {
  gnome_scores_display("Gnome xBill Scores", "gnome-xbill", NULL, pos);
}

static void display_scores() {
  show_scores(0);
}

/**********************/
/* Callback functions */
/**********************/

void new_game_cb () {
	game.start(1);
}

void quit_game_cb () {
	game.quit();
}

/*void get_coords (guint *x, guint *y) {
	XWindowAttributes wattr;
	Window junk;
	int rx, ry;
	XGetWindowAttributes (ui.display, ui.window, &wattr);
	XTranslateCoordinates (ui.display, ui.window, wattr.root,
		-wattr.border_width, -wattr.border_width, &rx, &ry, &junk);
	*x=rx+20;
	*y=ry+40;
}
*/

void popup (GtkWidget *mi, GtkWidget **box) {
  ui.pause_game();
  gtk_widget_show(*box);
  gtk_grab_add(*box);
  gtk_main();
  gtk_grab_remove(*box);
  gtk_widget_hide(*box);
  ui.resume_game();
}

/******************/
/* Event handlers */
/******************/
void leave_window_eh(GtkWidget *w, GdkEventCrossing *event) {
	ui.pause_game();
}

void enter_window_eh(GtkWidget *w, GdkEventCrossing *event) {
	ui.resume_game();
}

void redraw_window_eh(GtkWidget *w, GdkEventExpose *event) {
	ui.refresh();
}

void button_press_eh(GtkWidget *w, GdkEventButton *event) {
	game.button_press((int)event->x, (int)event->y);
}

void button_release_eh(GtkWidget *w, GdkEventButton *event) {
	game.button_release((int)event->x, (int)event->y);
}

gint timer_eh() {
	game.update();
	return TRUE;
}

