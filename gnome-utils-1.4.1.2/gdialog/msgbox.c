/*
 *  msgbox.c -- implements the message box and info box
 *
 *  AUTHOR: Savio Lam (lam836@cs.cuhk.hk)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "dialog.h"

static void cancelled(GtkWidget *w, gpointer *d)
{
	exit(-1);
}

static void okayed(GtkWidget *w, int button, gpointer *d)
{
	exit(0);
}


/*
 * Display a message box. Program will pause and display an "OK" button
 * if the parameter 'pause' is non-zero.
 */
int dialog_msgbox(const char *title, const char *prompt, int height, int width,
		  int pause)
{
	int i, x, y, key = 0;
	WINDOW *dialog;


	if (gnome_mode) {
		GtkWidget *w = gnome_dialog_new(title,
					    GNOME_STOCK_BUTTON_OK, NULL);
		GtkWidget *hbox;
		GtkWidget *vbox;

		gnome_dialog_set_close(GNOME_DIALOG(w), TRUE);
		gtk_window_set_title(GTK_WINDOW(w), title);

		hbox = gtk_hbox_new(FALSE, 0);
		vbox = gtk_vbox_new(FALSE, 0);


		label_autowrap(vbox, prompt, width);

		gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

		gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(w)->vbox),
				   hbox,
				   TRUE, TRUE, GNOME_PAD);
		gtk_window_set_position(GTK_WINDOW(w), GTK_WIN_POS_CENTER);
		gtk_signal_connect(GTK_OBJECT(w), "destroy",
			GTK_SIGNAL_FUNC(cancelled), NULL);
		gtk_signal_connect(GTK_OBJECT(w), "clicked",
			GTK_SIGNAL_FUNC(okayed), NULL);
		gtk_widget_show_all(w);
		gtk_main();
		return 0;
	}
	/* center dialog box on screen */
	x = (COLS - width) / 2;
	y = (LINES - height) / 2;

#ifndef NO_COLOR_CURSES
	if (use_shadow)
		draw_shadow(stdscr, y, x, height, width);
#endif
	dialog = newwin(height, width, y, x);
#ifdef WITH_GPM
	mouse_setbase(x, y);
#endif
	keypad(dialog, TRUE);

	draw_box(dialog, 0, 0, height, width, dialog_attr, border_attr);

	if (title != NULL) {
		wattrset(dialog, title_attr);
		wmove(dialog, 0, (width - strlen(title)) / 2 - 1);
		waddch(dialog, ' ');
		waddstr(dialog, title);
		waddch(dialog, ' ');
	}
	wattrset(dialog, dialog_attr);
	print_autowrap(dialog, prompt, width - 2, 1, 2);

	if (pause) {
		wattrset(dialog, border_attr);
		wmove(dialog, height - 3, 0);
		waddch(dialog, ACS_LTEE);
		for (i = 0; i < width - 2; i++)
			waddch(dialog, ACS_HLINE);
		wattrset(dialog, dialog_attr);
		waddch(dialog, ACS_RTEE);
		wmove(dialog, height - 2, 1);
		for (i = 0; i < width - 2; i++)
			waddch(dialog, ' ');
#ifdef WITH_GPM
		mouse_mkbutton(height - 2, width / 2 - 4, 6, '\n');
#endif
		print_button(dialog, "  OK  ",
			     height - 2, width / 2 - 4, TRUE);

		wrefresh(dialog);
		while (key != ESC && key != '\n' && key != ' ')
			key = mouse_wgetch(dialog);
	} else {
		key = '\n';
		wrefresh(dialog);
	}

	delwin(dialog);
	return key == ESC ? -1 : 0;
}
