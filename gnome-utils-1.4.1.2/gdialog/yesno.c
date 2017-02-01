/*
 *  yesno.c -- implements the yes/no box
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

/* Prototypes */

int dialog_yesno_with_default(const char *title, const char *prompt, 
			      int height, int width, int yesno_default);

static void callback_yn(GtkWidget *w, gint button, gpointer *unused)
{
	/* yes = 0 no = 1 */
	exit(button);
}

static void callback_err(GtkWidget *w, gpointer *unused)
{
	exit(-1);
}

void dialog_yesno(const char *title, const char *prompt, int height, int width)
{
    dialog_yesno_with_default(title, prompt, height, width, 1);
}

/*
 * Display a dialog box with two buttons - Yes and No
 */
int dialog_yesno_with_default(const char *title, const char *prompt, int height, int width, int yesno_default)
{
	int i, x, y, key = 0, button = 0;
	WINDOW *dialog;


	if (gnome_mode) {
		GtkWidget *w = gnome_dialog_new(title, 	
			GNOME_STOCK_BUTTON_YES, GNOME_STOCK_BUTTON_NO, NULL);
		GtkWidget *hbox;
		GtkWidget *vbox;

                gnome_dialog_set_default(GNOME_DIALOG(w), !yesno_default);
                
		gnome_dialog_set_close(GNOME_DIALOG(w), TRUE);
		gtk_window_set_title(GTK_WINDOW(w), title);
		
		hbox = gtk_hbox_new(FALSE, 0);
		vbox = gtk_vbox_new(FALSE, 0);

                if(width != 0)
                {
                    label_autowrap(vbox, prompt, width);
		}
                else
                {
                    GtkWidget *t = gtk_label_new(prompt);
                    gtk_box_pack_start(GTK_BOX(vbox), t, TRUE, TRUE, 0);
                    gtk_misc_set_alignment(GTK_MISC(t), -1, 0);
                }
                
		gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

		gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(w)->vbox), 
			hbox,
			TRUE, TRUE, GNOME_PAD);
		gtk_window_set_position(GTK_WINDOW(w), GTK_WIN_POS_CENTER);
		
		gtk_signal_connect(GTK_OBJECT(w), "clicked", GTK_SIGNAL_FUNC(callback_yn), NULL);
		gtk_signal_connect(GTK_OBJECT(w), "destroy", GTK_SIGNAL_FUNC(callback_err), NULL);
		gtk_widget_show_all(w);
		gtk_main();
		return -1;
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
	wattrset(dialog, border_attr);
	waddch(dialog, ACS_LTEE);
	for (i = 0; i < width - 2; i++)
		waddch(dialog, ACS_HLINE);
	wattrset(dialog, dialog_attr);
	waddch(dialog, ACS_RTEE);
	wmove(dialog, height - 2, 1);
	for (i = 0; i < width - 2; i++)
		waddch(dialog, ' ');

	if (title != NULL) {
		wattrset(dialog, title_attr);
		wmove(dialog, 0, (width - strlen(title)) / 2 - 1);
		waddch(dialog, ' ');
		waddstr(dialog, title);
		waddch(dialog, ' ');
	}
	wattrset(dialog, dialog_attr);
	print_autowrap(dialog, prompt, width - 2, 1, 3);

	x = width / 2 - 10;
	y = height - 2;
	print_button(dialog, "  No  ", y, x + 13, !yesno_default);
	print_button(dialog, " Yes ", y, x, yesno_default);
	wmove(dialog, y,
              (yesno_default ? x : x + 13));
        button = !yesno_default;
	wrefresh(dialog);

	while (key != ESC) {
		key = mouse_wgetch(dialog);
		switch (key) {
		case 'Y':
		case 'y':
			delwin(dialog);
			return 0;
		case 'N':
		case 'n':
			delwin(dialog);
			return 1;

		case M_EVENT + 'y':	/* mouse enter... */
		case M_EVENT + 'n':	/* use the code for toggling */
			button = (key == M_EVENT + 'y');

		case TAB:
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
			if (!button) {
				button = 1;	/* "No" button selected */
				print_button(dialog, " Yes ", y, x, FALSE);
				print_button(dialog, "  No  ", y, x + 13, TRUE);
			} else {
				button = 0;	/* "Yes" button selected */
				print_button(dialog, "  No  ", y, x + 13, FALSE);
				print_button(dialog, " Yes ", y, x, TRUE);
			}
			wrefresh(dialog);
			break;
		case ' ':
		case '\n':
			delwin(dialog);
			return button;
		case ESC:
			break;
		}
	}

	delwin(dialog);
	return -1;		/* ESC pressed */
}
