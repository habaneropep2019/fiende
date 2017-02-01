/*
 *  util.c
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

#ifndef NO_COLOR_CURSES
/* use colors by default? */
bool use_colors = USE_COLORS;
/* shadow dialog boxes by default?
   Note that 'use_shadow' implies 'use_colors' */
bool use_shadow = USE_SHADOW;
#endif

const char *backtitle = NULL;

const char *dialog_result;

/* 
 * Attribute values, default is for mono display
 */
chtype attributes[] =
{
	A_NORMAL,		/* screen_attr */
	A_NORMAL,		/* shadow_attr */
	A_REVERSE,		/* dialog_attr */
	A_REVERSE,		/* title_attr */
	A_REVERSE,		/* border_attr */
	A_BOLD,			/* button_active_attr */
	A_DIM,			/* button_inactive_attr */
	A_UNDERLINE,		/* button_key_active_attr */
	A_UNDERLINE,		/* button_key_inactive_attr */
	A_NORMAL,		/* button_label_active_attr */
	A_NORMAL,		/* button_label_inactive_attr */
	A_REVERSE,		/* inputbox_attr */
	A_REVERSE,		/* inputbox_border_attr */
	A_REVERSE,		/* searchbox_attr */
	A_REVERSE,		/* searchbox_title_attr */
	A_REVERSE,		/* searchbox_border_attr */
	A_REVERSE,		/* position_indicator_attr */
	A_REVERSE,		/* menubox_attr */
	A_REVERSE,		/* menubox_border_attr */
	A_REVERSE,		/* item_attr */
	A_NORMAL,		/* item_selected_attr */
	A_REVERSE,		/* tag_attr */
	A_REVERSE,		/* tag_selected_attr */
	A_NORMAL,		/* tag_key_attr */
	A_BOLD,			/* tag_key_selected_attr */
	A_REVERSE,		/* check_attr */
	A_REVERSE,		/* check_selected_attr */
	A_REVERSE,		/* uarrow_attr */
	A_REVERSE		/* darrow_attr */
};

#ifndef NO_COLOR_CURSES
#include "colors.h"

/*
 * Table of color values
 */
int color_table[][3] =
{
	{SCREEN_FG, SCREEN_BG, SCREEN_HL},
	{SHADOW_FG, SHADOW_BG, SHADOW_HL},
	{DIALOG_FG, DIALOG_BG, DIALOG_HL},
	{TITLE_FG, TITLE_BG, TITLE_HL},
	{BORDER_FG, BORDER_BG, BORDER_HL},
	{BUTTON_ACTIVE_FG, BUTTON_ACTIVE_BG, BUTTON_ACTIVE_HL},
	{BUTTON_INACTIVE_FG, BUTTON_INACTIVE_BG, BUTTON_INACTIVE_HL},
      {BUTTON_KEY_ACTIVE_FG, BUTTON_KEY_ACTIVE_BG, BUTTON_KEY_ACTIVE_HL},
{BUTTON_KEY_INACTIVE_FG, BUTTON_KEY_INACTIVE_BG, BUTTON_KEY_INACTIVE_HL},
{BUTTON_LABEL_ACTIVE_FG, BUTTON_LABEL_ACTIVE_BG, BUTTON_LABEL_ACTIVE_HL},
	{BUTTON_LABEL_INACTIVE_FG, BUTTON_LABEL_INACTIVE_BG,
	 BUTTON_LABEL_INACTIVE_HL},
	{INPUTBOX_FG, INPUTBOX_BG, INPUTBOX_HL},
	{INPUTBOX_BORDER_FG, INPUTBOX_BORDER_BG, INPUTBOX_BORDER_HL},
	{SEARCHBOX_FG, SEARCHBOX_BG, SEARCHBOX_HL},
	{SEARCHBOX_TITLE_FG, SEARCHBOX_TITLE_BG, SEARCHBOX_TITLE_HL},
	{SEARCHBOX_BORDER_FG, SEARCHBOX_BORDER_BG, SEARCHBOX_BORDER_HL},
   {POSITION_INDICATOR_FG, POSITION_INDICATOR_BG, POSITION_INDICATOR_HL},
	{MENUBOX_FG, MENUBOX_BG, MENUBOX_HL},
	{MENUBOX_BORDER_FG, MENUBOX_BORDER_BG, MENUBOX_BORDER_HL},
	{ITEM_FG, ITEM_BG, ITEM_HL},
	{ITEM_SELECTED_FG, ITEM_SELECTED_BG, ITEM_SELECTED_HL},
	{TAG_FG, TAG_BG, TAG_HL},
	{TAG_SELECTED_FG, TAG_SELECTED_BG, TAG_SELECTED_HL},
	{TAG_KEY_FG, TAG_KEY_BG, TAG_KEY_HL},
	{TAG_KEY_SELECTED_FG, TAG_KEY_SELECTED_BG, TAG_KEY_SELECTED_HL},
	{CHECK_FG, CHECK_BG, CHECK_HL},
	{CHECK_SELECTED_FG, CHECK_SELECTED_BG, CHECK_SELECTED_HL},
	{UARROW_FG, UARROW_BG, UARROW_HL},
	{DARROW_FG, DARROW_BG, DARROW_HL},
};				/* color_table */
#endif

/*
 * Set window to attribute 'attr'
 */
void attr_clear(WINDOW * win, int height, int width, chtype attr)
{
	int i, j;

	wattrset(win, attr);
	for (i = 0; i < height; i++) {
		wmove(win, i, 0);
		for (j = 0; j < width; j++)
			waddch(win, ' ');
	}
	touchwin(win);
}

void dialog_clear(void)
{
	attr_clear(stdscr, LINES, COLS, screen_attr);

#ifndef CFG_SMALLEST
	/* Display background title if it exists ... - SLH */
	if (backtitle != NULL) {
		int i;

		wattrset(stdscr, screen_attr);
		wmove(stdscr, 0, 1);
		waddstr(stdscr, backtitle);
		wmove(stdscr, 1, 1);
		for (i = 1; i < COLS - 1; i++)
			waddch(stdscr, ACS_HLINE);
	}
#endif

	wnoutrefresh(stdscr);
}

/*
 * Do some initialization for dialog
 */
void init_dialog(void)
{
	if (gnome_mode == 0) {
#ifdef WITH_GPM
		mouse_open();
#endif
#ifdef HAVE_RC_FILE
#ifndef NO_COLOR_CURSES
		if (parse_rc() == -1)	/* Read the configuration file */
			exit(-1);
#endif
#endif

		initscr();	/* Init curses */
		keypad(stdscr, TRUE);
		cbreak();
		noecho();

#ifndef NO_COLOR_CURSES
		if (use_colors || use_shadow)	/* Set up colors */
			color_setup();
#endif

		dialog_clear();
	}
}

#ifndef NO_COLOR_CURSES
/*
 * Setup for color display
 */
void color_setup(void)
{
	int i;

	if (has_colors()) {	/* Terminal supports color? */
		start_color();

		/* Initialize color pairs */
		for (i = 0; i < ATTRIBUTE_COUNT; i++)
			init_pair(i + 1, color_table[i][0], color_table[i][1]);

		/* Setup color attributes */
		for (i = 0; i < ATTRIBUTE_COUNT; i++)
			attributes[i] = C_ATTR(color_table[i][2], i + 1);
	}
}
#endif

/*
 * End using dialog functions.
 */
void end_dialog(void)
{
#ifdef WITH_GPM
	mouse_close();
#endif
	if(!gnome_mode)
	  endwin();
}


/*
 * Print a string of text in a window, automatically wrap around to the
 * next line if the string is too long to fit on one line. Note that the
 * string may contain "\n" to represent a newline character or the real
 * newline '\n', but in that case, auto wrap around will be disabled.
 */
void print_autowrap(WINDOW * win, const char *prompt, int width, int y, int x)
{
	int first = 1, cur_x, cur_y;
	char tempstr[MAX_LEN + 1], *word, *tempptr, *tempptr1;

	strcpy(tempstr, prompt);
	if ((strstr(tempstr, "\\n") != NULL) || (strchr(tempstr, '\n') != NULL)) {
		/* Prompt contains "\n" or '\n' */
		word = tempstr;
		cur_y = y;
		wmove(win, cur_y, x);
		while (1) {
			tempptr = strstr(word, "\\n");
			tempptr1 = strchr(word, '\n');
			if (tempptr == NULL && tempptr1 == NULL)
				break;
			else if (tempptr == NULL) {	/* No more "\n" */
				tempptr = tempptr1;
				tempptr[0] = '\0';
			} else if (tempptr1 == NULL) {	/* No more '\n' */
				tempptr[0] = '\0';
				tempptr++;
			} else {	/* Prompt contains both "\n" and '\n' */
				if (strlen(tempptr) - 2 < strlen(tempptr1) - 1) {
					tempptr = tempptr1;
					tempptr[0] = '\0';
				} else {
					tempptr[0] = '\0';
					tempptr++;
				}
			}

			waddstr(win, word);
			word = tempptr + 1;
			wmove(win, ++cur_y, x);
		}
		waddstr(win, word);
	} else if (strlen(tempstr) <= width - x * 2) {	/* If prompt is short */
		wmove(win, y, (width - strlen(tempstr)) / 2);
		waddstr(win, tempstr);
	} else {
		cur_x = x;
		cur_y = y;
		/* Print prompt word by word, wrap around if necessary */
		while ((word = strtok(first ? tempstr : NULL, " ")) != NULL) {
			if (first)	/* First iteration */
				first = 0;
			if (cur_x + strlen(word) > width) {
				cur_y++;	/* wrap to next line */
				cur_x = x;
			}
			wmove(win, cur_y, cur_x);
			waddstr(win, word);
			getyx(win, cur_y, cur_x);
			cur_x++;
		}
	}
}

static char *unquote_nl(const char *input)
{
	char *t=g_malloc(strlen(input)+1);
	char *p=t;
	while(*input)
	{
		if(*input=='\\' && input[1]=='n')
		{
			*p++='\n';
			input+=2;
		}
		else
			*p++=*(input++);
	}
	*p=0;
	return t;
}

void label_autowrap(GtkWidget *vbox, const char *input, int w)
{
	char buf[512];
	char word[512];
	char *bp, *wp;
	int wlen;
	int blen;
	int pspace;
	const char *orig_input;

	if(w>511)
		w=511;

	input=unquote_nl(input);
	/* Save this so we can free it later */
	orig_input = input;

	bp=buf;
	wp=word;
	wlen=0;
	blen=0;
	pspace=0;
	
	while(*input)
	{
		while(*input && isspace(*input))
			input++;
		
		while(!isspace(*input) && *input &&  wlen < w)
		{
			*wp++=*(input++);
			wlen++;
		}
		*wp=0;
		/* We have a word */
		if(wlen+blen >=w)
		{
			GtkWidget *t;
			*bp=0;
			bp=buf;
			while(*bp && isspace(*bp))
				bp++;
			
			t=gtk_label_new(buf);
			gtk_box_pack_start(GTK_BOX(vbox), t, TRUE, TRUE, 0);
			gtk_misc_set_alignment(GTK_MISC(t), -1, 0);
			blen=0;
			bp=buf;
		}
		*wp=0;
		if(pspace)
		{
			*bp++=' ';
			blen++;
			pspace=0;
		}
		strcpy(bp, word);
		bp+=wlen;
		blen+=wlen;
		wlen=0;
		wp=word;
		if(*bp=='\n')
		{
			GtkWidget *t;
			*bp=0;
			bp=buf;
			while(*bp && isspace(*bp))
				bp++;
			t=gtk_label_new(buf);
			gtk_box_pack_start(GTK_BOX(vbox), t, TRUE, TRUE, 0);
			gtk_misc_set_alignment(GTK_MISC(t), -1, 0);
			blen=0;
			bp=buf;
		}
		else
			pspace=1;
	}

	g_free((gpointer)orig_input); /* allocated by unquote_nl() */

	if(blen)
	{
		GtkWidget *t;
		*bp=0;
		bp=buf;
		while(*bp && isspace(*bp))
			bp++;
		t=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(vbox), t, TRUE, TRUE, 0);
			gtk_misc_set_alignment(GTK_MISC(t), -1, 0);
	}
}


/*
 * Print a button
 */
void print_button(WINDOW * win, const char *label, int y, int x, int selected)
{
	int i, temp;

	wmove(win, y, x);
	wattrset(win, selected ? button_active_attr : button_inactive_attr);
	waddstr(win, "<");
	temp = strspn(label, " ");
#ifdef WITH_GPM
	mouse_mkbutton(y, x, strlen(label) + 2, tolower(label[temp]));
#endif
	label += temp;
	wattrset(win, selected ? button_label_active_attr
		 : button_label_inactive_attr);
	for (i = 0; i < temp; i++)
		waddch(win, ' ');
	wattrset(win, selected ? button_key_active_attr
		 : button_key_inactive_attr);
	waddch(win, label[0]);
	wattrset(win, selected ? button_label_active_attr
		 : button_label_inactive_attr);
	waddstr(win, label + 1);
	wattrset(win, selected ? button_active_attr : button_inactive_attr);
	waddstr(win, ">");
	wmove(win, y, x + temp + 1);
}

/*
 * Draw a rectangular box with line drawing characters
 */
void draw_box(WINDOW * win, int y, int x, int height, int width,
	      chtype box, chtype border)
{
	int i, j;

	wattrset(win, 0);
	for (i = 0; i < height; i++) {
		wmove(win, y + i, x);
		for (j = 0; j < width; j++)
			if (!i && !j)
				waddch(win, border | ACS_ULCORNER);
			else if (i == height - 1 && !j)
				waddch(win, border | ACS_LLCORNER);
			else if (!i && j == width - 1)
				waddch(win, box | ACS_URCORNER);
			else if (i == height - 1 && j == width - 1)
				waddch(win, box | ACS_LRCORNER);
			else if (!i)
				waddch(win, border | ACS_HLINE);
			else if (i == height - 1)
				waddch(win, box | ACS_HLINE);
			else if (!j)
				waddch(win, border | ACS_VLINE);
			else if (j == width - 1)
				waddch(win, box | ACS_VLINE);
			else
				waddch(win, box | ' ');
	}
}

#ifndef NO_COLOR_CURSES
/*
 * Draw shadows along the right and bottom edge to give a more 3D look
 * to the boxes
 */
void draw_shadow(WINDOW * win, int y, int x, int height, int width)
{
	int i;

	if (has_colors()) {	/* Whether terminal supports color? */
		wattrset(win, shadow_attr);
		wmove(win, y + height, x + 2);
		for (i = 0; i < width; i++)
			waddch(win, winch(win) & A_CHARTEXT);
		for (i = y + 1; i < y + height + 1; i++) {
			wmove(win, i, x + width);
			waddch(win, winch(win) & A_CHARTEXT);
			waddch(win, winch(win) & A_CHARTEXT);
		}
		wnoutrefresh(win);
	}
}
#endif
